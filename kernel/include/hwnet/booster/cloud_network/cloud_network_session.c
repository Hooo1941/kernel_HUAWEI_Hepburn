/*
 * cloud_network_session.c
 *
 * cloud network kernel module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "cloud_network_session.h"
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/gfp.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_seqadj.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <uapi/linux/netfilter/nf_nat.h>
#include <securec.h>

#include "cloud_network_utils.h"
#include "cloud_network_device.h"
#include "cloud_network.h"
#include "cloud_network_rx.h"
#include "cloud_network_option.h"
#include "cloud_network_param.h"

#define ALL_SESSION_PRINT_MS 5000
#define SESSION_PRINT_PACKET_TX_THRES 128
#define SESSION_PRINT_PACKET_RX_THRES 128

static void session_free(struct session *ss);

/* serialize hash session_clear_immediate and session_clear */
static DEFINE_MUTEX(session_mutex);
static DEFINE_SPINLOCK(session_locks_all_lock);
static __read_mostly bool session_locks_all;
seqcount_spinlock_t session_generation __read_mostly;
static __read_mostly struct kmem_cache *session_cache;
struct hlist_nulls_head *session_htable __read_mostly;
unsigned int session_htable_size __read_mostly;
__cacheline_aligned_in_smp spinlock_t session_htable_locks[SESSION_LOCKS_NUM];
static atomic_t session_htable_count;
static atomic_t all_session_count;
static unsigned int session_hash_rnd __read_mostly;
struct session_pcpu __percpu *session_pcpu_lists;

struct rb_root uid_session_data = RB_ROOT;
static seqlock_t uid_session_data_lock;
static __read_mostly struct kmem_cache *uid_session_cache;
static atomic_t uid_session_count;
static atomic_t all_uid_session_count;

static struct hrtimer all_session_print_timer;
static struct work_struct all_session_print_work;

static void free_uid_session_rcu(struct rcu_head *head)
{
	if (unlikely(head == NULL))
		return;

	atomic_dec(&uid_session_count);
	log_info("uid session free, uid %u count %d",
		(container_of(head, struct uid_session, rcu))->uid,
		atomic_read(&uid_session_count));
	kmem_cache_free(uid_session_cache, container_of(head, struct uid_session, rcu));
	return;
}

/* Incremental reference count on a uid_session */
void uid_session_get(struct uid_session *uid_ss)
{
	if (uid_ss)
		atomic_inc(&uid_ss->use);
	return;
}

/* decrement reference count on a uid_session */
void uid_session_put(struct uid_session *uid_ss)
{
	if (uid_ss && atomic_dec_and_test(&uid_ss->use))
		call_rcu(&uid_ss->rcu, free_uid_session_rcu);
	return;
}

/* Incremental reference count on a session */
void session_get(struct session *ss)
{
	if (ss)
		atomic_inc(&ss->use);
	return;
}

/* decrement reference count on a session */
void session_put(struct session *ss)
{
	if (ss && atomic_dec_and_test(&ss->use))
		session_free(ss);
	return;
}

static void uid_session_hash_table_clear(struct uid_session *uid_ss)
{
	if (!uid_ss)
		return;

	if (uid_ss->hash_table_size != UID_SESSION_NUM)
		return;

	for (int i = 0; i < uid_ss->hash_table_size; i++) {
		while (!hlist_empty(&(uid_ss->hash_table[i])))
			log_err("this session not delete from uid_session_hash_table, error");
	}
	return;
}

static u32 hash_session_raw(const struct nf_conntrack_tuple *tuple,
	const struct net *net, u32 ep_ro)
{
	unsigned int n;
	u32 seed;
	u32 val;

	if (unlikely(tuple == NULL || net == NULL))
		return 0;

	get_random_once(&session_hash_rnd, sizeof(session_hash_rnd));

	seed = session_hash_rnd ^ net_hash_mix(net);
	n = (sizeof(tuple->src) + sizeof(tuple->dst.u3)) / sizeof(u32);
	val = jhash2((u32 *)tuple, n, seed ^
			(((__force __u16)tuple->dst.u.all << 16) |
			tuple->dst.protonum));
	return reciprocal_scale(val, ep_ro);
}

/* Called with rcu_read_lock() or hash_table_locks[] held */
static struct session *uid_session_hash_table_find(struct uid_session *uid_ss,
	const struct nf_conntrack_tuple *tuple,
	struct net *net)
{
	struct session *ret = NULL;
	struct session *ss = NULL;
	unsigned int bucket;

	if (unlikely(uid_ss == NULL || tuple == NULL || net == NULL))
		return NULL;

	rcu_read_lock();
	bucket = hash_session_raw(tuple, net, uid_ss->hash_table_size);
	int i = 0;
	hlist_for_each_entry_rcu(ss, &(uid_ss->hash_table[bucket]), uid_hash) {
		i++;
		if (nf_ct_tuple_equal(
			tuple, &ss->session_hash[IP_CT_DIR_ORIGINAL].tuple) &&
			net_eq(net, read_pnet(&ss->session_net))) {
			ret = ss;
			break;
		}
	}
	rcu_read_unlock();
	return ret;
}

/* Input Parameter tuple must be IP_CT_DIR_ORIGINAL */
static void uid_session_hash_table_delete(
	struct uid_session *uid_ss,
	struct session *ss)
{
	unsigned int bucket;
	spinlock_t *lock;

	if (unlikely(uid_ss == NULL || ss == NULL))
		return;

	bucket = hash_session_raw(&(ss->session_hash[IP_CT_DIR_ORIGINAL].tuple),
		read_pnet(&ss->session_net), uid_ss->hash_table_size);
	lock = &(uid_ss->hash_table_locks[bucket]);
	spin_lock_bh(lock);

	hlist_del_rcu(&(ss->uid_hash));
	atomic_dec(&uid_ss->hash_table_count);
	spin_unlock_bh(lock);
	spin_lock_bh(&ss->lock);
	INIT_HLIST_NODE(&ss->uid_hash);
	uid_session_put(ss->uid_ss);
	ss->uid_ss = NULL;
	spin_unlock_bh(&ss->lock);
	return;
}

/* need Existence check by uid_session_hash_table_find func before called
   tuple.dir must is IP_CT_DIR_ORIGINAL */
static void uid_session_hash_table_insert(
	struct uid_session *uid_ss,
	const struct nf_conntrack_tuple *tuple,
	struct net *net,
	struct hlist_node *uid_hash)
{
	unsigned int bucket;
	spinlock_t *lock = NULL;

	if (unlikely(uid_ss == NULL || tuple == NULL || net == NULL ||
		uid_hash == NULL))
		return;

	bucket = hash_session_raw(tuple, net, uid_ss->hash_table_size);
	lock = &(uid_ss->hash_table_locks[bucket]);
	spin_lock_bh(lock);
	hlist_add_head_rcu(uid_hash,
			   &(uid_ss->hash_table[bucket]));
	atomic_inc(&uid_ss->hash_table_count);
	spin_unlock_bh(lock);
	return;
}

static struct uid_session *alloc_uid_session(const u32 uid)
{
	struct uid_session *ret = NULL;

	if (!uid_session_cache) {
		log_err("uid_session_cache is NULL, error");
		return NULL;
	}

	ret = kmem_cache_alloc(uid_session_cache, GFP_ATOMIC);
	if (!ret) {
		log_err("kmem_cache_alloc failed, error");
		return NULL;
	}

	spin_lock_init(&(ret->lock));
	atomic_set(&ret->use, 0);
	ret->uid = uid;
	ret->hash_table_size = UID_SESSION_NUM;
	ret->hash_table = nf_ct_alloc_hashtable(&(ret->hash_table_size), 0);
	ret->hash_table_size = UID_SESSION_NUM;
	atomic_set(&(ret->hash_table_count), 0);
	if (!ret->hash_table) {
		log_err("session hash table alloc failed, error");
		kmem_cache_free(uid_session_cache, ret);
		return NULL;
	}

	for (int i = 0; i < UID_SESSION_LOCK_NUM; i++)
		spin_lock_init(&(ret->hash_table_locks[i]));

	return ret;
}

void uid_session_tree_clear(struct rb_root *root)
{
	struct rb_node *p = NULL;

	if (unlikely(root == NULL))
		return;

	log_info("uid session tree clear");
	write_seqlock_bh(&uid_session_data_lock);
	if (RB_EMPTY_ROOT(root)) {
		write_sequnlock_bh(&uid_session_data_lock);
		return;
	}

	while (p = rb_first(root)) {
		struct uid_session *uid_ss = rb_entry(p, struct uid_session, node);
		log_info("uid_session uid %u erase", uid_ss->uid);
		if (unlikely(atomic_read(&(uid_ss->hash_table_count)) != 0)) {
			log_err("memory leak uid %u hash_table_count %d", uid_ss->uid,
				atomic_read(&(uid_ss->hash_table_count)));
			uid_session_hash_table_clear(uid_ss);
		}
		p = rb_next(p);
		rb_erase(&(uid_ss->node), root);
		uid_session_put(uid_ss);
	}
	uid_session_data = RB_ROOT;
	write_sequnlock_bh(&uid_session_data_lock);
	return;
}

/* Called with rcu_read_lock() or uid_session_data_lock held */
static struct uid_session *uid_session_lookup(struct rb_root *root,
	unsigned int read_seq, unsigned int seq, int *invalid,
	struct rb_node **parent_p, struct rb_node ***pp_p, const u32 uid)
{
	struct rb_node **pp = NULL;
	struct rb_node *parent = NULL;
	struct rb_node *next = NULL;
	struct uid_session *p = NULL;

	if (unlikely(root == NULL || invalid == NULL || parent_p == NULL ||
		pp_p == NULL))
		return NULL;

	pp = &(root->rb_node);
	while (1) {
		next = rcu_dereference_raw(*pp);
		if (!next)
			break;
		parent = next;
		p = rb_entry(parent, struct uid_session, node);
		if (uid == p->uid) {
			*invalid = 0;
			return p;
		}
		if (read_seq) {
			if (unlikely(read_seqretry(&uid_session_data_lock, seq)))
				break;
		}
		if (uid < p->uid)
			pp = &next->rb_left;
		else
			pp = &next->rb_right;
	}
	*invalid = 1;
	*parent_p = parent;
	*pp_p = pp;
	return NULL;
}

static struct uid_session *uid_session_search(struct rb_root *root,
	const u32 uid)
{
	struct uid_session *p = NULL;
	struct rb_node **pp = NULL;
	struct rb_node *parent = NULL;
	unsigned int seq;
	int invalid = 0;

	if (unlikely(root == NULL))
		return NULL;

	/* Attempt a lockless lookup first.
	 * Because of a concurrent writer, we might not find an existing entry.
	 */
	rcu_read_lock();
	seq = read_seqbegin(&uid_session_data_lock);
	p = uid_session_lookup(root, 1, seq, &invalid, &parent, &pp, uid);
	rcu_read_unlock();

	/* If find the uid session, return the uid session */
	if (p)
		return p;

	/* If no writer did a change and not create, we can return early. */
	if (!invalid)
		return NULL;

	/* retry an exact lookup, taking the lock before.
	 * At least, nodes should be hot in our cache.
	 */
	parent = NULL;
	write_seqlock_bh(&uid_session_data_lock);
	p = uid_session_lookup(root, 0, 0, &invalid, &parent, &pp, uid);
	write_sequnlock_bh(&uid_session_data_lock);
	return p;
}

/* need Existence check by uid_session_search function before called */
static struct uid_session *uid_session_insert(struct rb_root *root,
	const u32 uid)
{
	struct uid_session *cursor = NULL;
	struct uid_session *p = NULL;
	struct rb_node **pp = NULL;
	struct rb_node *parent = NULL;

	if (unlikely(root == NULL))
		return NULL;

	write_seqlock_bh(&uid_session_data_lock);
	pp = &(root->rb_node);
	parent = NULL;
	while (*pp) {
		parent = *pp;
		cursor = rb_entry(parent, struct uid_session, node);

		if (uid < cursor->uid) {
			pp = &(*pp)->rb_left;
		} else if (uid > cursor->uid) {
			pp = &(*pp)->rb_right;
		} else {
			write_sequnlock_bh(&uid_session_data_lock);
			return cursor;
		}
	}
	p = alloc_uid_session(uid);
	if (!p)
		return NULL;
	rb_link_node_rcu(&p->node, parent, pp);
	rb_insert_color(&p->node, root);
	atomic_inc(&uid_session_count);
	uid_session_get(p);
	log_info("uid session alloc, uid %u count %d", uid,
		atomic_read(&uid_session_count));
	write_sequnlock_bh(&uid_session_data_lock);
	return p;
}

static void uid_session_delete(struct rb_root *root,
	const u32 uid)
{
	struct uid_session *uid_ss = NULL;
	struct rb_node **cur = NULL;

	if (unlikely(root == NULL))
		return;

	write_seqlock_bh(&uid_session_data_lock);
	cur = &(root->rb_node);
	while (*cur != NULL) {
		uid_ss = rb_entry(*cur, struct uid_session, node);
		if (uid < uid_ss->uid) {
			cur = &((*cur)->rb_left);
		} else if (uid > uid_ss->uid) {
			cur = &((*cur)->rb_right);
		} else {
			rb_erase(&(uid_ss->node), root);
			uid_session_put(uid_ss);
			write_sequnlock_bh(&uid_session_data_lock);
			log_err("delete uid %d", uid);
			return;
		}
	}
	write_sequnlock_bh(&uid_session_data_lock);
	log_info("not find uid %d", uid);
	return;
}

static void uid_session_delete_hash_table_item(struct uid_session *uid_ss,
	struct session *ss)
{
	if (unlikely(uid_ss == NULL || ss == NULL))
		return;
	uid_session_hash_table_delete(uid_ss, ss);
	return;
}

/* must be called with rcu read lock held */
static inline void session_get_ht(struct hlist_nulls_head **hash,
	unsigned int *hsize)
{
	struct hlist_nulls_head *hptr = NULL;
	unsigned int sequence, hsz;

	if (unlikely(hash == NULL || hsize == NULL))
		return;

	do {
		sequence = read_seqcount_begin(&session_generation);
		hsz = session_htable_size;
		hptr = session_htable;
	} while (read_seqcount_retry(&session_generation, sequence));

	*hash = hptr;
	*hsize = hsz;
	return;
}

/* must be called with local_bh_disable */
static void session_add_to_dying_list(struct session *ss)
{
	struct session_pcpu *pcpu;

	if (unlikely(ss == NULL))
		return;

	/* add this session to the (per cpu) dying list */
	ss->cpu = smp_processor_id();
	pcpu = per_cpu_ptr(session_pcpu_lists, ss->cpu);

	spin_lock(&pcpu->lock);
	hlist_nulls_add_head(&ss->session_hash[IP_CT_DIR_ORIGINAL].hnnode,
		&pcpu->dying);
	spin_unlock(&pcpu->lock);
	return;
}

/* must be called with local_bh_disable */
static void session_del_from_dying_list(struct session *ss)
{
	struct session_pcpu *pcpu;

	if (unlikely(ss == NULL))
		return;

	/* We has already overload first tuple to link into dying list */
	pcpu = per_cpu_ptr(session_pcpu_lists, ss->cpu);

	spin_lock(&pcpu->lock);
	BUG_ON(hlist_nulls_unhashed(&ss->session_hash[IP_CT_DIR_ORIGINAL].hnnode));
	hlist_nulls_del_rcu(&ss->session_hash[IP_CT_DIR_ORIGINAL].hnnode);
	spin_unlock(&pcpu->lock);
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_rx_interval_reset(struct session *ss)
{
	if (unlikely(!ss))
		return;

	(void)memset_s(ss->rx_interval, sizeof(ss->rx_interval),
		0x00, sizeof(ss->rx_interval));
	(void)memset_s(ss->rx_interval_primary, sizeof(ss->rx_interval_primary),
		0x00, sizeof(ss->rx_interval_primary));
	(void)memset_s(ss->rx_interval_secondary, sizeof(ss->rx_interval_secondary),
		0x00, sizeof(ss->rx_interval_secondary));
	return;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_per_reset(struct session *ss)
{
	if (unlikely(!ss))
		return;

	ss->per_start_rx_seq = 0;
	ss->per_end_rx_seq = 0;
	ss->per_packet_rx = 0;
	ss->per_start_rx_seq_primary = 0;
	ss->per_end_rx_seq_primary = 0;
	ss->per_packet_rx_primary = 0;
	ss->per_start_rx_seq_secondary = 0;
	ss->per_end_rx_seq_secondary = 0;
	ss->per_packet_rx_secondary = 0;
	return;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_tx_rx_reset(struct session *ss)
{
	if (unlikely(!ss))
		return;

	ss->packet_rx_multi_path = 0;
	ss->packet_rx_multi_path_gain = 0;
	ss->packet_tx = 0;
	ss->packet_tx_primary = 0;
	ss->packet_tx_secondary = 0;
	ss->packet_rx = 0;
	ss->packet_rx_primary = 0;
	ss->packet_rx_secondary = 0;
	ss->traffic_tx = 0;
	ss->traffic_tx_primary = 0;
	ss->traffic_tx_secondary = 0;
	ss->traffic_rx = 0;
	ss->traffic_rx_primary = 0;
	ss->traffic_rx_secondary = 0;
	ss->timestamp_tx = 0;
	ss->packet_rx_multi_path_last = 0;
	ss->packet_rx_multi_path_gain_last = 0;
	ss->packet_tx_last = 0;
	ss->packet_tx_primary_last = 0;
	ss->packet_tx_secondary_last = 0;
	ss->packet_rx_last = 0;
	ss->packet_rx_primary_last = 0;
	ss->packet_rx_secondary_last = 0;
	ss->traffic_tx_last = 0;
	ss->traffic_tx_primary_last = 0;
	ss->traffic_tx_secondary_last = 0;
	ss->traffic_rx_last = 0;
	ss->traffic_rx_primary_last = 0;
	ss->traffic_rx_secondary_last = 0;
	return;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_reset(struct session *ss)
{
	if (unlikely(!ss))
		return;

	session_statistics_tx_rx_reset(ss);
	session_statistics_per_reset(ss);

	ss->rx_timestamp = 0;
	ss->rx_timestamp_primary = 0;
	ss->rx_timestamp_secondary = 0;
	session_statistics_rx_interval_reset(ss);
	return;
}

static struct session *session_alloc(struct net *net,
	const struct nf_conntrack_tuple *orig,
	const struct nf_conntrack_tuple *repl, const struct sk_buff *skb,
	gfp_t gfp, u32 uid)
{
	struct session *ss = NULL;

	if (unlikely(!net || !orig || !repl || !skb))
		return NULL;

	if (unlikely(atomic_read(&session_htable_count) > SESSION_COUNT_MAX))
		return NULL;

	ss = kmem_cache_alloc(session_cache, gfp);
	if (ss == NULL)
		return NULL;

	atomic_set(&ss->use, 0);
	spin_lock_init(&ss->lock);
	spin_lock_bh(&ss->lock);
	all_window_reset(ss);
	ss->session_hash[IP_CT_DIR_ORIGINAL].tuple = *orig;
	ss->session_hash[IP_CT_DIR_ORIGINAL].hnnode.pprev = NULL;
	ss->session_hash[IP_CT_DIR_REPLY].tuple = *repl;
	ss->session_hash[IP_CT_DIR_REPLY].hnnode.pprev = NULL;
	INIT_HLIST_NODE(&ss->uid_hash);
	ss->uid_ss = NULL;
	write_pnet(&ss->session_net, net);
	ss->link_type = 0;
	ss->uid = uid;
	session_statistics_reset(ss);
	struct iphdr *iph = (struct iphdr *)skb->data;
	ss->saddr = iph->saddr;
	ss->daddr = iph->daddr;
	ss->protocol = iph->protocol;
	ss->id_tx = iph->id;

	if (iph->protocol == IPPROTO_TCP) {
		struct tcphdr *tcph = (struct tcphdr *)(skb->data + iph->ihl * 4);
		ss->source = tcph->source;
		ss->dest = tcph->dest;
	} else if (iph->protocol == IPPROTO_UDP) {
		struct udphdr *udph = (struct udphdr *)(skb->data + iph->ihl * 4);
		ss->source = udph->source;
		ss->dest = udph->dest;
	}
	atomic_set(&ss->send_seq, 0);
	spin_unlock_bh(&ss->lock);
	return ss;
}

static noinline struct nf_conntrack_tuple_hash *init_session(struct net *net,
	const struct nf_conntrack_tuple *tuple, const struct sk_buff *skb,
	u32 uid)
{
	struct session *ss = NULL;
	struct nf_conntrack_tuple repl_tuple;
	struct uid_session *uid_ss = NULL;

	if (unlikely(!net || !tuple || !skb))
		return NULL;

	if (!nf_ct_invert_tuple(&repl_tuple, tuple)) {
		log_err("Can't invert tuple");
		return NULL;
	}

	ss = session_alloc(net, tuple, &repl_tuple, skb, GFP_ATOMIC, uid);
	if (!ss) {
		log_err("session alloc failed");
		return NULL;
	}

	uid_ss = uid_session_search(&uid_session_data, uid);
	if (uid_ss == NULL)
		uid_ss = uid_session_insert(&uid_session_data, uid);
	if (likely(uid_ss != NULL)) {
		uid_session_get(uid_ss);
		ss->uid_ss = uid_ss;
		uid_session_hash_table_insert(uid_ss,
			&(ss->session_hash[IP_CT_DIR_ORIGINAL].tuple), dev_net(skb->dev),
			&(ss->uid_hash));
	}
	log_info("alloc session uid %d protocol %u"
		" all_session_count %d tuple %u %u %u", ss->uid, ss->protocol,
		atomic_read(&all_session_count), *((u32 *)tuple),
		*((u32 *)tuple + NUM_1), *((u32 *)tuple + NUM_2));

	return &ss->session_hash[IP_CT_DIR_ORIGINAL];
}

static void session_lock(spinlock_t *lock) __acquires(lock)
{
	if (unlikely(!lock))
		return;

	/* 1) Acquire the lock */
	spin_lock(lock);

	/* 2) read session_locks_all, with ACQUIRE semantics
	 * It pairs with the smp_store_release() in session_all_unlock()
	 */
	if (likely(smp_load_acquire(&session_locks_all) == false))
		return;

	/* fast path failed, unlock */
	spin_unlock(lock);

	/* Slow path 1) get global lock */
	spin_lock(&session_locks_all_lock);

	/* Slow path 2) get the lock we want */
	spin_lock(lock);

	/* Slow path 3) release the global lock */
	spin_unlock(&session_locks_all_lock);
	return;
}

static void session_double_unlock(unsigned int h1, unsigned int h2)
{
	h1 %= SESSION_LOCKS_NUM;
	h2 %= SESSION_LOCKS_NUM;
	spin_unlock(&session_htable_locks[h1]);
	if (h1 != h2)
		spin_unlock(&session_htable_locks[h2]);
	return;
}

static bool session_double_lock(unsigned int h1, unsigned int h2, unsigned int sequence)
{
	h1 %= SESSION_LOCKS_NUM;
	h2 %= SESSION_LOCKS_NUM;
	if (h1 <= h2) {
		session_lock(&session_htable_locks[h1]);
		if (h1 != h2)
			spin_lock_nested(&session_htable_locks[h2],
					 SINGLE_DEPTH_NESTING);
	} else {
		session_lock(&session_htable_locks[h2]);
		spin_lock_nested(&session_htable_locks[h1],
				 SINGLE_DEPTH_NESTING);
	}
	if (read_seqcount_retry(&session_generation, sequence)) {
		session_double_unlock(h1, h2);
		return true;
	}
	return false;
}

static void session_all_lock(void)
	__acquires(&session_locks_all_lock)
{
	int i;

	spin_lock(&session_locks_all_lock);

	session_locks_all = true;

	for (i = 0; i < SESSION_LOCKS_NUM; i++) {
		spin_lock(&session_htable_locks[i]);

		/* This spin_unlock provides the "release" to ensure that
		 * session_locks_all==true is visible to everyone that
		 * acquired spin_lock(&session_locks[]).
		 */
		spin_unlock(&session_htable_locks[i]);
	}
}

static void session_all_unlock(void)
	__releases(&session_locks_all_lock)
{
	/* All prior stores must be complete before we clear
	 * 'session_locks_all'. Otherwise session_lock()
	 * might observe the false value but not the entire
	 * critical section.
	 * It pairs with the smp_load_acquire() in session_lock()
	 */
	smp_store_release(&session_locks_all, false);
	spin_unlock(&session_locks_all_lock);
}

static void session_hash_insert(struct session *ss,
	unsigned int hash, unsigned int reply_hash)
{
	if (unlikely(!ss))
		return;

	hlist_nulls_add_head_rcu(&ss->session_hash[IP_CT_DIR_ORIGINAL].hnnode,
			   &session_htable[hash]);
	hlist_nulls_add_head_rcu(&ss->session_hash[IP_CT_DIR_REPLY].hnnode,
			   &session_htable[reply_hash]);
	atomic_inc(&session_htable_count);
	atomic_inc(&all_session_count);
	return;
}

static void session_free(struct session *ss)
{
	if (unlikely(!ss))
		return;

	struct nf_conntrack_tuple *tuple = &(ss->session_hash[IP_CT_DIR_ORIGINAL].tuple);

	/* A freed object has reference use == 0, that's
	 * the golden rule for SLAB_TYPESAFE_BY_RCU
	 */
	WARN_ON(atomic_read(&ss->use) != 0);

	if (likely(ss->uid_ss != NULL))
		uid_session_delete_hash_table_item(ss->uid_ss, ss);

	local_bh_disable();
	session_del_from_dying_list(ss);
	local_bh_enable();
	atomic_dec(&all_session_count);

	log_info("free session uid %d protocol %u"
		" all_session_count %d tuple %u %u %u", ss->uid, ss->protocol,
		atomic_read(&all_session_count), *((u32 *)tuple),
		*((u32 *)tuple + NUM_1), *((u32 *)tuple + NUM_2));

	kmem_cache_free(session_cache, ss);
	return;
}

static int session_stat_report(struct session *ss)
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_tx_rx_stat *msg_rept = NULL;

	if (unlikely(ss == NULL))
		return ERROR;

	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_tx_rx_stat), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_tx_rx_stat);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = TX_RX_STAT;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_tx_rx_stat);

	msg_rept = (struct rept_tx_rx_stat *)(msg_header->data);
	spin_lock_bh(&ss->lock);
	msg_rept->uid = ss->uid;
	msg_rept->packet_rx_multi_path = ss->packet_rx_multi_path;
	msg_rept->packet_rx_multi_path_gain = ss->packet_rx_multi_path_gain;
	msg_rept->traffic_tx = ss->traffic_tx;
	msg_rept->traffic_tx_primary = ss->traffic_tx_primary;
	msg_rept->traffic_tx_secondary = ss->traffic_tx_secondary;
	msg_rept->traffic_rx = ss->traffic_rx;
	msg_rept->traffic_rx_primary = ss->traffic_rx_primary;
	msg_rept->traffic_rx_secondary = ss->traffic_rx_secondary;
	spin_unlock_bh(&ss->lock);

	log_info("event report tx rx statistic uid %u sum rx %llu gain %llu "
	"size tx sum %llu size tx pri %llu size tx sec %llu size rx sum %llu "
	"size rx pri %llu size rx sec %llu", msg_rept->uid,
	msg_rept->packet_rx_multi_path, msg_rept->packet_rx_multi_path_gain,
	msg_rept->traffic_tx, msg_rept->traffic_tx_primary,
	msg_rept->traffic_tx_secondary, msg_rept->traffic_rx,
	msg_rept->traffic_rx_primary, msg_rept->traffic_rx_secondary);

	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return SUCCESS;
}

static int stoped_report(void)
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_stoped *msg_rept = NULL;

	log_info("event report data flow stoped");
	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_stoped), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_stoped);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = STOPED;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_stoped);

	msg_rept = (struct rept_stoped *)(msg_header->data);
	msg_rept->status = 0;

	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return SUCCESS;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_print_tx_rx(struct session *ss)
{
	if (unlikely(!ss))
		return;

	log_info("uid %u packet tx %llu packet rx %llu"
		" traffic tx %llu traffic rx %llu "
		" protocol %u rx_multi_path  %llu"
		" rx_multi_path_gain %llu packet_tx_primary %llu"
		" packet_tx_secondary %llu traffic_tx_primary %llu"
		" traffic_tx_secondary %llu packet_rx_primary %llu"
		" packet_rx_secondary %llu traffic_rx_primary %llu"
		" traffic_rx_secondary %llu", ss->uid, ss->packet_tx,
		ss->packet_rx, ss->traffic_tx, ss->traffic_rx,
		ss->protocol, ss->packet_rx_multi_path,
		ss->packet_rx_multi_path_gain,
		ss->packet_tx_primary, ss->packet_tx_secondary,
		ss->traffic_tx_primary, ss->traffic_tx_secondary,
		ss->packet_rx_primary, ss->packet_rx_secondary,
		ss->traffic_rx_primary, ss->traffic_rx_secondary);

	log_info("delta uid %u packet tx %llu packet rx %llu"
		" traffic tx %llu traffic rx %llu "
		" protocol %u rx_multi_path %llu"
		" rx_multi_path_gain %llu packet_tx_primary %llu"
		" packet_tx_secondary %llu traffic_tx_primary %llu"
		" traffic_tx_secondary %llu packet_rx_primary %llu"
		" packet_rx_secondary %llu traffic_rx_primary %llu"
		" traffic_rx_secondary %llu", ss->uid,
		ss->packet_tx - ss->packet_tx_last, ss->packet_rx - ss->packet_rx_last,
		ss->traffic_tx - ss->traffic_tx_last,
		ss->traffic_rx - ss->traffic_rx_last,
		ss->protocol, ss->packet_rx_multi_path - ss->packet_rx_multi_path_last,
		ss->packet_rx_multi_path_gain - ss->packet_rx_multi_path_gain_last,
		ss->packet_tx_primary - ss->packet_tx_primary_last,
		ss->packet_tx_secondary - ss->packet_tx_secondary_last,
		ss->traffic_tx_primary - ss->traffic_tx_primary_last,
		ss->traffic_tx_secondary - ss->traffic_tx_secondary_last,
		ss->packet_rx_primary - ss->packet_rx_primary_last,
		ss->packet_rx_secondary - ss->packet_rx_secondary_last,
		ss->traffic_rx_primary - ss->traffic_rx_primary_last,
		ss->traffic_rx_secondary - ss->traffic_rx_secondary_last);
	return;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_print_per(struct session *ss)
{
	if (unlikely(!ss))
		return;

	log_info("per uid %u"
		" protocol %u should rx %u actually rx %u"
		" primary should rx %u primary actually rx %u"
		" secondary should rx %u secondary actually rx %u", ss->uid,
		ss->protocol,
		ss->per_end_rx_seq - ss->per_start_rx_seq + 1, ss->per_packet_rx,
		ss->per_end_rx_seq_primary - ss->per_start_rx_seq_primary + 1,
		ss->per_packet_rx_primary,
		ss->per_end_rx_seq_secondary - ss->per_start_rx_seq_secondary + 1,
		ss->per_packet_rx_secondary);

	return;
}

static void session_statistics_print_rx_interval_each(struct session *ss, u32 *rx_interval)
{
	if (unlikely(!ss || !rx_interval))
		return;

	log_info("rx interval uid %u"
		" protocol %u"
		" %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u"
		" %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
		ss->uid, ss->protocol, *(rx_interval + NUM_0),
		*(rx_interval + NUM_1), *(rx_interval + NUM_2),
		*(rx_interval + NUM_3), *(rx_interval + NUM_4),
		*(rx_interval + NUM_5), *(rx_interval + NUM_6),
		*(rx_interval + NUM_7), *(rx_interval + NUM_8),
		*(rx_interval + NUM_9), *(rx_interval + NUM_10),
		*(rx_interval + NUM_11), *(rx_interval + NUM_12),
		*(rx_interval + NUM_13), *(rx_interval + NUM_14),
		*(rx_interval + NUM_15), *(rx_interval + NUM_16),
		*(rx_interval + NUM_17), *(rx_interval + NUM_18),
		*(rx_interval + NUM_19), *(rx_interval + NUM_20),
		*(rx_interval + NUM_21), *(rx_interval + NUM_22),
		*(rx_interval + NUM_23), *(rx_interval + NUM_24),
		*(rx_interval + NUM_25), *(rx_interval + NUM_26),
		*(rx_interval + NUM_27), *(rx_interval + NUM_28),
		*(rx_interval + NUM_29), *(rx_interval + NUM_30),
		*(rx_interval + NUM_31), *(rx_interval + NUM_32),
		*(rx_interval + NUM_33), *(rx_interval + NUM_34),
		*(rx_interval + NUM_35), *(rx_interval + NUM_36),
		*(rx_interval + NUM_37), *(rx_interval + NUM_38),
		*(rx_interval + NUM_39), *(rx_interval + NUM_40));
	return;
}


/* warning: this function need be used with spin_lock_bh(&ss->lock) */
static void session_statistics_print_rx_interval(struct session *ss)
{
	if (unlikely(!ss))
		return;

	session_statistics_print_rx_interval_each(ss, ss->rx_interval);
	session_statistics_print_rx_interval_each(ss, ss->rx_interval_primary);
	session_statistics_print_rx_interval_each(ss, ss->rx_interval_secondary);
	return;
}

static void session_statistics_print(struct session *ss)
{
	if (unlikely(!ss))
		return;

	spin_lock_bh(&ss->lock);
	if (ss->packet_tx > SESSION_PRINT_PACKET_TX_THRES ||
		ss->packet_rx > SESSION_PRINT_PACKET_RX_THRES) {
		session_statistics_print_tx_rx(ss);
		session_statistics_print_per(ss);
		session_statistics_print_rx_interval(ss);
	}

	ss->packet_rx_multi_path_last = ss->packet_rx_multi_path;
	ss->packet_rx_multi_path_gain_last = ss->packet_rx_multi_path_gain;
	ss->packet_tx_last = ss->packet_tx;
	ss->packet_tx_primary_last = ss->packet_tx_primary;
	ss->packet_tx_secondary_last = ss->packet_tx_secondary;
	ss->packet_rx_last = ss->packet_rx;
	ss->packet_rx_primary_last = ss->packet_rx_primary;
	ss->packet_rx_secondary_last = ss->packet_rx_secondary;
	ss->traffic_tx_last = ss->traffic_tx;
	ss->traffic_tx_primary_last = ss->traffic_tx_primary;
	ss->traffic_tx_secondary_last = ss->traffic_tx_secondary;
	ss->traffic_rx_last = ss->traffic_rx;
	ss->traffic_rx_primary_last = ss->traffic_rx_primary;
	ss->traffic_rx_secondary_last = ss->traffic_rx_secondary;
	session_statistics_per_reset(ss);
	session_statistics_rx_interval_reset(ss);
	spin_unlock_bh(&ss->lock);
	return;
}

static void session_delete_status_notify(struct session *ss)
{
	if (unlikely(!ss))
		return;

	send_session_synchronize_v4(4, ss->saddr, ss->daddr, ss->source, ss->dest, ss->protocol);
	session_stat_report(ss);
	session_statistics_print(ss);
	if (get_data_flow_status() && !cloud_network_started()) {
		set_data_flow_status_stop();
		stoped_report();
		stop_all_session_print();
		stop_gain_detection();
	}
	return;
}

static void session_clean_from_lists(struct session *ss)
{
	if (unlikely(!ss))
		return;

	hlist_nulls_del_rcu(&ss->session_hash[IP_CT_DIR_ORIGINAL].hnnode);
	hlist_nulls_del_rcu(&ss->session_hash[IP_CT_DIR_REPLY].hnnode);
	atomic_dec(&session_htable_count);
	return;
}

static void session_delete_from_lists(struct session *ss)
{
	unsigned int sequence;
	unsigned int hash;
	unsigned int reply_hash;

	if (unlikely(!ss))
		return;

	local_bh_disable();
	do {
		sequence = read_seqcount_begin(&session_generation);
		hash = hash_session_raw(&(ss->session_hash[IP_CT_DIR_ORIGINAL].tuple),
			read_pnet(&ss->session_net), session_htable_size);
		reply_hash = hash_session_raw(&(ss->session_hash[IP_CT_DIR_REPLY].tuple),
			read_pnet(&ss->session_net), session_htable_size);
	} while (session_double_lock(hash, reply_hash, sequence));

	session_clean_from_lists(ss);

	session_double_unlock(hash, reply_hash);

	session_add_to_dying_list(ss);
	local_bh_enable();
	session_put(ss);
	return;
}

static void session_delete(struct session *ss)
{
	if (unlikely(!ss))
		return;

	struct nf_conntrack_tuple *tuple = &(ss->session_hash[IP_CT_DIR_ORIGINAL].tuple);

	session_delete_from_lists(ss);
	session_delete_status_notify(ss);

	log_info("delete session uid %d protocol %u"
		" session_htable_count %d tuple %u %u %u", ss->uid, ss->protocol,
		atomic_read(&session_htable_count), *((u32 *)tuple),
		*((u32 *)tuple + NUM_1), *((u32 *)tuple + NUM_2));
	return;
}

/*
 * The input param tuple must be IP_CT_DIR_ORIGINAL dir
 */
void session_delete_by_tuple(struct nf_conntrack_tuple *tuple, struct net *net)
{
	struct nf_conntrack_tuple repl_tuple;
	struct session *ss = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;
	struct hlist_nulls_node *n = NULL;
	unsigned int sequence;
	u32 reply_hash;
	u32 hash;
	bool find = false;

	if (unlikely(!tuple || !net))
		return;

	if (!nf_ct_invert_tuple(&repl_tuple, tuple)) {
		log_err("Can't invert tuple");
		return;
	}

	local_bh_disable();
	do {
		sequence = read_seqcount_begin(&session_generation);
		hash = hash_session_raw(tuple, net, session_htable_size);
		reply_hash = hash_session_raw(&repl_tuple, net, session_htable_size);
	} while (session_double_lock(hash, reply_hash, sequence));

	hlist_nulls_for_each_entry_rcu(h, n, &session_htable[hash], hnnode) {
		ss = container_of(h, struct session, session_hash[h->tuple.dst.dir]);
		if (nf_ct_tuple_equal(tuple, &h->tuple) && net_eq(net, read_pnet(&ss->session_net))) {
			/* Incremental reference count on a conntrack */
			if (likely(atomic_inc_not_zero(&ss->use)))
				find = true;
			break;
		}
	}

	if (find == false) {
		session_double_unlock(hash, reply_hash);
		local_bh_enable();
		return;
	}

	session_clean_from_lists(ss);

	session_double_unlock(hash, reply_hash);

	session_add_to_dying_list(ss);
	local_bh_enable();
	session_put(ss);

	session_delete_status_notify(ss);

	log_info("delete session uid %d protocol %u"
		" session_htable_count %d tuple %u %u %u", ss->uid, ss->protocol,
		atomic_read(&session_htable_count), *((u32 *)tuple),
		*((u32 *)tuple + NUM_1), *((u32 *)tuple + NUM_2));

	session_put(ss);
	return;
}

/*
 * Need Existence check by is_session_existed or session_find_get before called
 * The return value(struct session *) must be session_put after use
 */
struct session *session_add_get(struct nf_conntrack_tuple *tuple,
	struct sk_buff *skb, u32 uid)
{
	struct session *ss = NULL;
	struct nf_conntrack_tuple_hash *original_h = NULL;
	unsigned int hash;
	unsigned int reply_hash;
	unsigned int sequence;

	if (unlikely(!tuple || !skb))
		return NULL;

	original_h = init_session(dev_net(skb->dev), tuple, skb, uid);
	if (original_h == NULL)
		return NULL;

	ss = container_of(original_h, struct session,
		session_hash[original_h->tuple.dst.dir]);

	local_bh_disable();
	do {
		sequence = read_seqcount_begin(&session_generation);
		hash = hash_session_raw(&(original_h->tuple), dev_net(skb->dev), session_htable_size);
		reply_hash = hash_session_raw(&(ss->session_hash[IP_CT_DIR_REPLY].tuple),
			dev_net(skb->dev), session_htable_size);
	} while (session_double_lock(hash, reply_hash, sequence));
	session_get(ss);
	session_hash_insert(ss, hash, reply_hash);
	session_double_unlock(hash, reply_hash);
	local_bh_enable();

	session_get(ss);
	log_info("add session uid %d protocol %u"
		" session_htable_count %d tuple %u %u %u", ss->uid, ss->protocol,
		atomic_read(&session_htable_count), *((u32 *)tuple),
		*((u32 *)tuple + NUM_1), *((u32 *)tuple + NUM_2));

	return ss;
}

static struct session *session_get_next_iterate(unsigned int *bucket)
{
	struct nf_conntrack_tuple_hash *h = NULL;
	struct session *ss = NULL;
	struct hlist_nulls_node *n = NULL;
	spinlock_t *lockp = NULL;

	if (unlikely(!bucket))
		return NULL;

	for (; *bucket < session_htable_size; (*bucket)++) {
		struct hlist_nulls_head *hslot = &session_htable[*bucket];

		if (hlist_nulls_empty(hslot))
			continue;

		lockp = &session_htable_locks[*bucket % SESSION_LOCKS_NUM];
		local_bh_disable();
		session_lock(lockp);
		hlist_nulls_for_each_entry(h, n, hslot, hnnode) {
			if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
				continue;
			/* All session objects are added to hash table twice, one
			 * for original direction tuple, once for the reply tuple.
			 * We only need to call the iterator once for each
			 * conntrack, so we just use the 'reply' direction
			 * tuple while iterating.
			 */
			ss = container_of(h, struct session,
				session_hash[h->tuple.dst.dir]);
			session_get(ss);
			break;
		}
		spin_unlock(lockp);
		local_bh_enable();
		if (ss != NULL)
			break;
		cond_resched();
	}
	return ss;
}

static void session_clear_immediate(void)
{
	struct nf_conntrack_tuple_hash *h = NULL;
	struct session *ss = NULL;

	mutex_lock(&session_mutex);
	local_bh_disable();
	session_all_lock();
	write_seqcount_begin(&session_generation);

	/* Lookups in session_htable might happen in parallel, which means we
	 * might get false negatives during connection lookup.
	 * though since that required taking the locks.
	 */
	for (int i = 0; i < session_htable_size; i++) {
		while (!hlist_nulls_empty(&session_htable[i])) {
			h = hlist_nulls_entry(session_htable[i].first,
						struct nf_conntrack_tuple_hash, hnnode);
			ss = container_of(h, struct session,
				session_hash[h->tuple.dst.dir]);
			session_get(ss);
			session_delete(ss);
			session_put(ss);
		}
	}
	write_seqcount_end(&session_generation);
	session_all_unlock();
	local_bh_enable();
	mutex_unlock(&session_mutex);
	return;
}

static void session_clear(void)
{
	unsigned int bucket = 0;
	struct session *ss = NULL;

	log_info("session clear");
	mutex_lock(&session_mutex);
	while ((ss = session_get_next_iterate(&bucket)) != NULL) {
		session_delete(ss);
		session_put(ss);
		cond_resched();
	}
	mutex_unlock(&session_mutex);
	return;
}

/*
 * Find and Incremental reference count on a session
 * It pairs with session_put for decrement reference count on the session
 */
static struct nf_conntrack_tuple_hash *__session_find_get(struct net *net,
	const struct nf_conntrack_tuple *tuple, u32 hash)
{
	struct nf_conntrack_tuple_hash *ret = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;
	struct session *ss = NULL;
	struct hlist_nulls_node *n = NULL;
	struct hlist_nulls_head *ss_hash = NULL;
	unsigned int hsize;

	if (unlikely(!net || !tuple))
		return NULL;

	rcu_read_lock();
	session_get_ht(&ss_hash, &hsize);

	hlist_nulls_for_each_entry_rcu(h, n, &session_htable[hash], hnnode) {
		ss = container_of(h, struct session, session_hash[h->tuple.dst.dir]);
		if (nf_ct_tuple_equal(tuple, &h->tuple) && net_eq(net, read_pnet(&ss->session_net))) {
			/* Incremental reference count on a conntrack */
			if (likely(atomic_inc_not_zero(&ss->use)))
				ret = h;
			break;
		}
	}
	rcu_read_unlock();
	return ret;
}

bool get_skb_tuple_reply(struct nf_conntrack_tuple *tuple,
	const struct sk_buff *skb)
{
	struct nf_conntrack_tuple inverse;
	u_int8_t l3num; /* Assume IPv4 protocol */
	errno_t err;

	if (unlikely(!tuple || !skb))
		return false;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		l3num = NFPROTO_IPV4;
		break;
	case htons(ETH_P_IPV6):
		l3num = NFPROTO_IPV6;
		break;
	default:
		return false;
	}
	/* Extract tuple from packet */
	if (!nf_ct_get_tuplepr(skb, skb_network_offset(skb), l3num,
		dev_net(skb->dev), tuple))
			return false;

	if (!nf_ct_invert_tuple(&inverse, tuple)) {
		log_err("nf_ct_invert_tuple");
		return false;
	}
	err = memcpy_s(tuple, sizeof(inverse), &inverse, sizeof(inverse));
	if (err != EOK)
		return false;

	return true;
}

bool get_skb_tuple_original(struct nf_conntrack_tuple *tuple,
	const struct sk_buff *skb)
{
	int proto;

	if (unlikely(!tuple || !skb))
		return false;

	switch (skb_protocol(skb, true)) {
	case htons(ETH_P_IP):
		proto = NFPROTO_IPV4;
		break;
	case htons(ETH_P_IPV6):
		proto = NFPROTO_IPV6;
		break;
	default:
		return false;
	}

	if (!nf_ct_get_tuplepr(skb, skb_network_offset(skb), proto,
		dev_net(skb->dev), tuple))
		return false;

	return true;
}

/* find session existed or not */
bool is_session_existed(struct net *net,
	const struct nf_conntrack_tuple *tuple)
{
	u32 hash;
	struct session *ss = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;

	if (unlikely(!net || !tuple))
		return false;

	hash = hash_session_raw(tuple, net, session_htable_size);
	h = __session_find_get(net, tuple, hash);
	if (h != NULL) {
		ss = container_of(h, struct session, session_hash[h->tuple.dst.dir]);
		session_put(ss);
		return true;
	} else {
		return false;
	}
}

bool cloud_network_started(void)
{
	if (atomic_read(&session_htable_count) != 0)
		return true;
	else
		return false;
}

/*
 * find and Incremental reference count on a session
 * The return value(struct session *) must be session_put after use
 */
struct nf_conntrack_tuple_hash *session_find_get(struct net *net,
	const struct nf_conntrack_tuple *tuple)
{
	u32 hash;

	if (unlikely(!tuple || !net))
		return NULL;

	hash = hash_session_raw(tuple, net, session_htable_size);
	return __session_find_get(net, tuple, hash);
}

/*
 * for ip fragment packet, find and Incremental reference count on a session
 * The return value(struct session *) must be session_put after use
 */
struct session *fragment_tx_session_find_get(struct sk_buff* skb)
{
	unsigned int bucket = 0;
	struct session *ss = NULL;
	struct iphdr *iph = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;
	struct hlist_nulls_node *n = NULL;
	spinlock_t *lockp = NULL;

	if (unlikely(skb == NULL))
		return NULL;

	iph = ip_hdr(skb);
	for (; bucket < session_htable_size; bucket++) {
		struct hlist_nulls_head *hslot = &session_htable[bucket];

		if (hlist_nulls_empty(hslot))
			continue;

		lockp = &session_htable_locks[bucket % SESSION_LOCKS_NUM];
		local_bh_disable();
		session_lock(lockp);
		hlist_nulls_for_each_entry(h, n, hslot, hnnode) {
			if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
				continue;
			/* All session objects are added to hash table twice, one
			 * for original direction tuple, once for the reply tuple.
			 * We only need to call the iterator once for each
			 * conntrack, so we just use the 'reply' direction
			 * tuple while iterating.
			 */
			ss = container_of(h, struct session,
				session_hash[h->tuple.dst.dir]);
			if (ss->saddr == iph->saddr && ss->daddr == iph->daddr &&
				ss->protocol == iph->protocol && ss->id_tx == iph->id) {
				session_get(ss);
				spin_unlock(lockp);
				local_bh_enable();
				cond_resched();
				return ss;
			}
		}
		spin_unlock(lockp);
		local_bh_enable();
		cond_resched();
	}
	return NULL;
}

/*
 * for ip fragment packet, find and Incremental reference count on a session
 * The return value(struct session *) must be session_put after use
 */
struct session *fragment_rx_session_find_get(struct sk_buff* skb)
{
	unsigned int bucket = 0;
	struct session *ss = NULL;
	struct iphdr *iph = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;
	struct hlist_nulls_node *n = NULL;
	spinlock_t *lockp = NULL;

	if (unlikely(skb == NULL))
		return NULL;

	iph = ip_hdr(skb);
	for (; bucket < session_htable_size; bucket++) {
		struct hlist_nulls_head *hslot = &session_htable[bucket];

		if (hlist_nulls_empty(hslot))
			continue;

		lockp = &session_htable_locks[bucket % SESSION_LOCKS_NUM];
		local_bh_disable();
		session_lock(lockp);
		hlist_nulls_for_each_entry(h, n, hslot, hnnode) {
			if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
				continue;
			/* All session objects are added to hash table twice, one
			 * for original direction tuple, once for the reply tuple.
			 * We only need to call the iterator once for each
			 * conntrack, so we just use the 'reply' direction
			 * tuple while iterating.
			 */
			ss = container_of(h, struct session,
				session_hash[h->tuple.dst.dir]);
			if (ss->daddr == iph->saddr && ss->saddr == iph->daddr &&
				ss->protocol == iph->protocol && ss->id_rx == iph->id) {
				session_get(ss);
				spin_unlock(lockp);
				local_bh_enable();
				cond_resched();
				return ss;
			}
		}
		spin_unlock(lockp);
		local_bh_enable();
		cond_resched();
	}
	log_err("fragment packet rx session not found");
	return NULL;
}

/*
 * Need Existence check by is_session_existed or session_find_get before called
 * The return value(struct session *) must be session_put after use
 */
struct session *master_session_add_get(struct nf_conntrack_tuple *tuple,
	struct sk_buff *skb, u32 uid)
{
	if (unlikely(!tuple || !skb))
		return NULL;

	return session_add_get(tuple, skb, uid);
}

void session_update_tx(struct session *ss, struct sk_buff *skb)
{
	u64 timestamp = 0;
	struct iphdr *iph = NULL;

	if (unlikely(!ss || !skb))
		return;

	iph = ip_hdr(skb);
	timestamp = ktime_get_real_ns();

	spin_lock_bh(&(ss->lock));
	ss->packet_tx++;
	ss->traffic_tx += skb->len;
	ss->timestamp_tx = timestamp;
	ss->id_tx = iph->id;
	spin_unlock_bh(&(ss->lock));
	return;
}

void session_update_tx_dual(struct session *ss, struct sk_buff *skb, struct honhdr *honh)
{
	if (unlikely(!ss || !skb || !honh))
		return;

	spin_lock_bh(&ss->lock);
	if (honh->fid == DETECT_PATH_VAL_MODEM_1) { /* primary modem path */
		ss->packet_tx_primary++;
		ss->traffic_tx_primary += ntohs(honh->payload_length);
	}
	if (honh->fid == DETECT_PATH_VAL_MODEM_2) { /* secondary modem path */
		ss->packet_tx_secondary++;
		ss->traffic_tx_secondary += ntohs(honh->payload_length);
	}
	spin_unlock_bh(&ss->lock);
	return;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
void session_update_rx_dual(struct session *ss, struct sk_buff *skb,
	struct honhdr *honh, u64 timestamp)
{
	u64 id;

	if (unlikely(!ss || !skb || !honh))
		return;

	if (honh->fid == DETECT_PATH_VAL_MODEM_1) { /* primary modem path */
		ss->packet_rx_primary++;
		ss->traffic_rx_primary += skb->len;
		if (ss->per_start_rx_seq_primary == 0)
			ss->per_start_rx_seq_primary = ntohl(honh->seq);
		ss->per_end_rx_seq_primary = ntohl(honh->seq);
		ss->per_packet_rx_primary++;
		if (ss->rx_timestamp_primary != 0) {
			id = (timestamp - ss->rx_timestamp_primary) /
				(RX_INTERVAL_STEP * NSEC_PER_MSEC);
			if (id >= (RX_INTERVAL_NUM - 1))
				ss->rx_interval_primary[RX_INTERVAL_NUM - 1]++;
			else
				ss->rx_interval_primary[id]++;
		}
		ss->rx_timestamp_primary = timestamp;
	}
	if (honh->fid == DETECT_PATH_VAL_MODEM_2) { /* secondary modem path */
		ss->packet_rx_secondary++;
		ss->traffic_rx_secondary += skb->len;
		if (ss->per_start_rx_seq_secondary == 0)
			ss->per_start_rx_seq_secondary = ntohl(honh->seq);
		ss->per_end_rx_seq_secondary = ntohl(honh->seq);
		ss->per_packet_rx_secondary++;
		if (ss->rx_timestamp_secondary != 0) {
			id = (timestamp - ss->rx_timestamp_secondary) /
				(RX_INTERVAL_STEP * NSEC_PER_MSEC);
			if (id >= (RX_INTERVAL_NUM - 1))
				ss->rx_interval_secondary[RX_INTERVAL_NUM - 1]++;
			else
				ss->rx_interval_secondary[id]++;
		}
		ss->rx_timestamp_secondary = timestamp;
	}
	return;
}

/* warning: this function need be used with spin_lock_bh(&ss->lock) */
void session_update_rx(struct session *ss, struct sk_buff *skb,
	struct honhdr *honh, u64 timestamp)
{
	u64 id;
	struct iphdr *iph = NULL;
	u32 paths;

	if (unlikely(!ss || !skb || !honh))
		return;

	iph = ip_hdr(skb);

	if (ss->rx_timestamp != 0) {
		id = (timestamp - ss->rx_timestamp) /
			(RX_INTERVAL_STEP * NSEC_PER_MSEC);
		if (id >= (RX_INTERVAL_NUM - 1))
			ss->rx_interval[RX_INTERVAL_NUM - 1]++;
		else
			ss->rx_interval[id]++;
	}
	ss->rx_timestamp = timestamp;
	ss->packet_rx++;
	ss->traffic_rx += skb->len;
	ss->id_rx = iph->id;

	paths = get_link_path();
	if ((paths & (PATH_MODEM_1 | PATH_MODEM_2)) ==
		(PATH_MODEM_1 | PATH_MODEM_2)) { /* modem + modem scene */
		ss->packet_rx_multi_path++;
		if (honh->fid == DETECT_PATH_VAL_MODEM_2) /* secondary modem path */
			ss->packet_rx_multi_path_gain++;
	} else if ((paths & PATH_MODEM_1) != PATH_MODEM_1) {
		ss->rx_timestamp_primary = 0;
	} else if ((paths & PATH_MODEM_2) != PATH_MODEM_2) {
		ss->rx_timestamp_secondary = 0;
	}

	if (ss->per_start_rx_seq == 0)
		ss->per_start_rx_seq = ntohl(honh->seq);
	ss->per_end_rx_seq = ntohl(honh->seq);
	ss->per_packet_rx++;
	return;
}

void all_session_window_reset(void)
{
	unsigned int bucket = 0;
	struct session *ss = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;
	struct hlist_nulls_node *n = NULL;
	spinlock_t *lockp = NULL;

	for (; bucket < session_htable_size; bucket++) {
		struct hlist_nulls_head *hslot = &session_htable[bucket];

		if (hlist_nulls_empty(hslot))
			continue;

		lockp = &session_htable_locks[bucket % SESSION_LOCKS_NUM];
		local_bh_disable();
		session_lock(lockp);
		hlist_nulls_for_each_entry(h, n, hslot, hnnode) {
			if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
				continue;
			/* All session objects are added to hash table twice, one
			 * for original direction tuple, once for the reply tuple.
			 * We only need to call the iterator once for each
			 * conntrack, so we just use the 'reply' direction
			 * tuple while iterating.
			 */
			ss = container_of(h, struct session,
				session_hash[h->tuple.dst.dir]);
			spin_lock_bh(&ss->lock);
			all_window_reset(ss);
			spin_unlock_bh(&ss->lock);
		}
		spin_unlock(lockp);
		local_bh_enable();
		cond_resched();
	}
	return;
}

static void session_statistics_print_all(void)
{
	unsigned int bucket = 0;
	struct session *ss = NULL;
	struct nf_conntrack_tuple_hash *h = NULL;
	struct hlist_nulls_node *n = NULL;
	spinlock_t *lockp = NULL;

	for (; bucket < session_htable_size; bucket++) {
		struct hlist_nulls_head *hslot = &session_htable[bucket];

		if (hlist_nulls_empty(hslot))
			continue;

		lockp = &session_htable_locks[bucket % SESSION_LOCKS_NUM];
		local_bh_disable();
		session_lock(lockp);
		hlist_nulls_for_each_entry(h, n, hslot, hnnode) {
			if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
				continue;
			/* All session objects are added to hash table twice, one
			 * for original direction tuple, once for the reply tuple.
			 * We only need to call the iterator once for each
			 * conntrack, so we just use the 'reply' direction
			 * tuple while iterating.
			 */
			ss = container_of(h, struct session,
				session_hash[h->tuple.dst.dir]);
			session_statistics_print(ss);
		}
		spin_unlock(lockp);
		local_bh_enable();
		cond_resched();
	}
	log_info("session count is %d", atomic_read(&session_htable_count));
	return;
}

static void all_session_print_work_handle(struct work_struct *work)
{
	session_statistics_print_all();
	return;
}

static enum hrtimer_restart all_session_print_timer_func(struct hrtimer *timer)
{
	if (cloud_network_started())
		schedule_work(&all_session_print_work);
	hrtimer_forward_now(timer, ms_to_ktime(ALL_SESSION_PRINT_MS));
	return HRTIMER_RESTART;
}

/*
 * The input param tuple must be IP_CT_DIR_ORIGINAL dir
 */
void master_session_delete(struct nf_conntrack_tuple *tuple, struct net *net)
{
	if (unlikely(!tuple || !net))
		return;

	if (!is_compass_available())
		return;

	session_delete_by_tuple(tuple, net);
	return;
}

void start_all_session_print(void)
{
	if (!hrtimer_active(&all_session_print_timer))
	hrtimer_start(&all_session_print_timer,
		ms_to_ktime(ALL_SESSION_PRINT_MS), HRTIMER_MODE_REL);
	return;
}

void stop_all_session_print(void)
{
	hrtimer_cancel(&all_session_print_timer);
	cancel_work_sync(&(all_session_print_work));
	return;
}

static int ims_error_report(u32 uid)
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_ims_app_abnormal *msg_rept = NULL;

	log_info("event report ims app abnormal");
	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_ims_app_abnormal), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_ims_app_abnormal);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = IMS_APP_ABNORMAL;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_ims_app_abnormal);

	msg_rept = (struct rept_ims_app_abnormal *)(msg_header->data);
	msg_rept->uid = uid;

	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return SUCCESS;
}

void ims_check_send(u32 uid)
{
	if (!get_ims_status())
		return;

	if (!is_ims_uid(uid))
		return;

	if (is_ims_timeout(ktime_to_ms(ktime_get_real())))
		ims_error_report(uid);
}

void ims_check_recv(struct session *ss)
{
	if (unlikely(!ss))
		return;

	if (!get_ims_status())
		return;

	if (unlikely(ss == NULL)) {
		log_err("%s ss == NULL", __func__);
		return;
	}
	if (is_ims_uid(ss->uid))
		update_ims_recv_timestamp(ktime_to_ms(ktime_get_real()));
}

static int session_data_alloc(void)
{
	log_info("session data alloc");

	session_pcpu_lists = alloc_percpu(struct session_pcpu);
	if (session_pcpu_lists == NULL) {
		log_err("session cpu lists alloc failed, error");
		return ERROR;
	}

	atomic_set(&session_htable_count, 0);
	atomic_set(&all_session_count, 0);
	session_htable_size = SESSION_NUM;
	session_htable = nf_ct_alloc_hashtable(&session_htable_size, 1);
	if (unlikely(!session_htable)) {
		log_err("session hash table alloc failed, error");
		free_percpu(session_pcpu_lists);
		session_pcpu_lists = NULL;
		return ERROR;
	}

	session_cache = kmem_cache_create("cloud_network_session",
						sizeof(struct session),
						NFCT_INFOMASK + 1,
						SLAB_TYPESAFE_BY_RCU | SLAB_HWCACHE_ALIGN, NULL);
	if (unlikely(!session_cache)) {
		log_err("session cache create failed, error");
		kvfree(session_htable);
		session_htable = NULL;
		free_percpu(session_pcpu_lists);
		session_pcpu_lists = NULL;
		return ERROR;
	}

	uid_session_data = RB_ROOT;
	atomic_set(&uid_session_count, 0);
	atomic_set(&all_uid_session_count, 0);
	uid_session_cache = kmem_cache_create("cloud_network_uid_session",
						sizeof(struct uid_session),
						NFCT_INFOMASK + 1,
						SLAB_TYPESAFE_BY_RCU | SLAB_HWCACHE_ALIGN, NULL);
	if (unlikely(!uid_session_cache)) {
		log_err("uid session cache create failed, error");
		kvfree(session_htable);
		session_htable = NULL;
		kmem_cache_destroy(session_cache);
		session_cache = NULL;
		free_percpu(session_pcpu_lists);
		session_pcpu_lists = NULL;
		return ERROR;
	}

	return SUCCESS;
}

int cloud_network_session_init(void)
{
	int cpu;

	log_info("cloud network session module init");

	if (session_data_alloc() == ERROR) {
		log_err("session data alloc failed, error");
		return ERROR;
	}

	for_each_possible_cpu(cpu) {
		struct session_pcpu *pcpu = per_cpu_ptr(session_pcpu_lists, cpu);
		spin_lock_init(&pcpu->lock);
		INIT_HLIST_NULLS_HEAD(&pcpu->dying, SESSION_PCPU_LISTS_NULLS_VAL);
	}

	for (int i = 0; i < SESSION_LOCKS_NUM; i++)
		spin_lock_init(&session_htable_locks[i]);

	seqcount_spinlock_init(&session_generation,
			   &session_locks_all_lock);
	session_locks_all = false;
	hrtimer_init(&all_session_print_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	all_session_print_timer.function = all_session_print_timer_func;
	INIT_WORK(&all_session_print_work, all_session_print_work_handle);
	seqlock_init(&uid_session_data_lock);
	return SUCCESS;
}

void cloud_network_session_reset(void)
{
	log_info("cloud network session module exit");
	stop_all_session_print();
	session_clear();
	uid_session_tree_clear(&uid_session_data);
	return;
}

void cloud_network_session_exit(void)
{
	log_info("cloud network session module exit, htable %d session %d",
		atomic_read(&session_htable_count), atomic_read(&all_session_count));

	session_clear_immediate();
	uid_session_tree_clear(&uid_session_data);

	if (likely(session_htable != NULL)) {
		kvfree(session_htable);
		session_htable = NULL;
	}

	if (likely(session_cache != NULL)) {
		kmem_cache_destroy(session_cache);
		session_cache = NULL;
	}

	if (likely(session_cache != NULL)) {
		kmem_cache_destroy(session_cache);
		session_cache = NULL;
	}

	if (likely(uid_session_cache != NULL)) {
		kmem_cache_destroy(uid_session_cache);
		uid_session_cache = NULL;
	}

	if (likely(session_pcpu_lists != NULL)) {
		free_percpu(session_pcpu_lists);
		session_pcpu_lists = NULL;
	}
	return;
}
