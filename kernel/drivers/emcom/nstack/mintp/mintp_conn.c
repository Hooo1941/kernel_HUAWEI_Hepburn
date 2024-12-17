/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * License terms: GNU General Public License (GPL)
 * Author: songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#include <linux/of.h>
#include <linux/proc_fs.h>
#include <linux/udp.h>
#include <net/ip.h>
#include "mintp_congestion.h"
#include "mintp_timer.h"
#include "mintp_conn.h"

#define MTP_D2D_TIMEOUT (10 * MSEC_PER_SEC)

#define MTP_D2D_OFF 0
#define MTP_D2D_INIT 1
#define MTP_DEV_ON 2

struct mtp_dev_node {
	struct list_head	list;
	struct net_device       *dev;
	u16			d2d_enable_cnt;
	u8			state;
	atomic_t		bytes;
	long			bytes_jiffies; /* the time of start count send/recv bytes of the enabled devices */
};

struct mtp_hashinfo {
	spinlock_t		lock;
	u16			rand_port;
	struct hlist_head 	bind;
	struct hlist_nulls_head listening;
	struct hlist_nulls_head established;
	struct list_head 	dev_head;
};

static struct mtp_hashinfo mtp_hash;
static siphash_key_t syncookie_secret __read_mostly;
static struct delayed_work mtp_dev_worker;
struct mtp_mib __percpu *mtp_statistics;

#if defined(CONFIG_CONNECTIVITY_HI1105) || defined(CONFIG_CONNECTIVITY_HI1103)
#define MTP_D2D_ENABLE_CNT_MAX 1000
#define MTP_D2D_LOW_LATENCY_ENABLE 0x08
#define MTP_D2D_LOW_LATENCY_DISABLE 0x07
int32_t wal_ioctl_config_low_latency_hi1105(struct net_device *pst_net_dev, uint8_t uc_param);
extern uint32_t wlan_pm_disable_hi1105(void);
extern uint32_t wlan_pm_enable_hi1105(void);

static bool mtp_hi1106_check(void)
{
	int32_t ret;
	const char *subchip_type = NULL;
	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon,hi110x");
	if (!np) {
		mtp_info("of_find_compatible_node hisilicon,hi110x fail\n");
		return false;
	}

	ret = of_property_read_string(np, "hi110x,subchip_type", &subchip_type);
	if (ret || !subchip_type) {
		mtp_info("can't get hi110x,subchip_type\n");
		return ret;
	}

	if (strcmp(subchip_type, "bisheng"))
		return false;
	return true;
}
#endif

static void mtp_dev_disable_now(struct net_device *dev)
{
#if defined(CONFIG_CONNECTIVITY_HI1105) || defined(CONFIG_CONNECTIVITY_HI1103)
	int32_t ret;

	ret = wal_ioctl_config_low_latency_hi1105(dev, MTP_D2D_LOW_LATENCY_DISABLE);
	mtp_info("wal_ioctl_config_low_latency_hi1105 %s return %d\n", dev->name, ret);

	ret = wlan_pm_enable_hi1105();
	mtp_info("wlan_pm_enable_hi1105 return %d\n", ret);
#endif
}

static void mtp_dev_enable_now(struct net_device *dev)
{
#if defined(CONFIG_CONNECTIVITY_HI1105) || defined(CONFIG_CONNECTIVITY_HI1103)
	int32_t ret;

	ret = wal_ioctl_config_low_latency_hi1105(dev, MTP_D2D_LOW_LATENCY_ENABLE);
	mtp_info("wal_ioctl_config_low_latency_hi1105 %s return %d\n", dev->name, ret);

	ret = wlan_pm_disable_hi1105();
	mtp_info("wlan_pm_disable_hi1105 return %u\n", ret);
#endif
}

static void mtp_device_down_clean(struct list_head *clean_list)
{
	struct list_head *q, *n;
	struct sock *sk;
	struct mtp_sock *msk;
	list_for_each_safe(q, n, clean_list) {
		msk = list_entry(q, struct mtp_sock, net_down_node);
		list_del(&msk->net_down_node);
		sk = (struct sock *)msk;
		bh_lock_sock(sk);
		rcu_assign_pointer(msk->odev, NULL);
		msk->d2d = false;
		msk->dev_node = NULL;

		if (!sock_owned_by_user(sk)) {
			mtp_clear_xmit_timers(sk);
			__skb_queue_purge(&msk->reader_queue);
			mtp_write_queue_purge(sk);
			skb_rbtree_purge(&msk->out_of_order_queue);
			sk_stream_kill_queues(sk);
			if (!sock_flag(sk, SOCK_DEAD)) {
				mtp_info("sock is not dead\n");
				sk->sk_error_report(sk);
			} else {
				mtp_info("try dec sk ref %d\n", refcount_read(&sk->sk_refcnt));
				sock_put(sk);
			}
		}
		bh_unlock_sock(sk);
		sock_put(sk);
	}
}

void mtp_device_down(struct net_device *dev)
{
	struct sock *sk;
	struct hlist_node *n;
	struct net_device *odev;
	struct mtp_dev_node *node, *tmp;
	struct list_head clean_list;

	INIT_LIST_HEAD(&clean_list);
	spin_lock_bh(&mtp_hash.lock);
	hlist_for_each_entry_safe(sk, n, &mtp_hash.bind, sk_bind_node) {
		struct mtp_sock *msk = (struct mtp_sock *)sk;
		rcu_read_lock();
		odev = rcu_dereference(msk->odev);
		rcu_read_unlock();
		if (odev == dev) {
			sk->sk_err = ENETDOWN;
			__sk_nulls_del_node_init_rcu(sk);
			__sk_del_bind_node(sk);
			msk->src_port = 0;
			mtp_set_state(sk, TCP_CLOSE);
			list_add_tail(&msk->net_down_node, &clean_list);
			sock_hold(sk);
		}
	}
	spin_unlock_bh(&mtp_hash.lock);

	mtp_device_down_clean(&clean_list);

	spin_lock_bh(&mtp_hash.lock);
	list_for_each_entry_safe(node, tmp, &mtp_hash.dev_head, list) {
		if (node->dev == dev) {
			/* don't need check node->on for now devices is down */
			list_del(&node->list);
			kfree(node);
			break;
		}
	}
	spin_unlock_bh(&mtp_hash.lock);
}

static void mtp_dev_work_process(struct work_struct *work)
{
	struct mtp_dev_node *node, *tmp;
	int work_on_cnt;
	(void)work;

restart:
	work_on_cnt = 0;
	spin_lock_bh(&mtp_hash.lock);
	list_for_each_entry_safe(node, tmp, &mtp_hash.dev_head, list) {
		struct net_device *dev = node->dev;
		u8 state = node->state;

		if (!node->d2d_enable_cnt) {
			list_del(&node->list);
			kfree(node);
			if (state == MTP_DEV_ON)
				goto disable;
			continue;
		}

		if (state == MTP_DEV_ON) {
			if (time_after(jiffies, node->bytes_jiffies + msecs_to_jiffies(MTP_D2D_TIMEOUT)) &&
			    !atomic_read(&node->bytes)) {
				node->state = MTP_D2D_OFF;
disable:
				spin_unlock_bh(&mtp_hash.lock);
				mtp_dev_disable_now(dev);
				/* the lock is released, the list may changed by other threads */
				goto restart;
			}
			work_on_cnt++;
			atomic_set(&node->bytes, 0);
			node->bytes_jiffies = jiffies;
		} else if (state == MTP_D2D_INIT) {
			node->state = MTP_DEV_ON;
			work_on_cnt++;
			atomic_set(&node->bytes, 0);
			node->bytes_jiffies = jiffies;
			spin_unlock_bh(&mtp_hash.lock);
			mtp_dev_enable_now(dev);
			/* the lock is released, the list may changed by other threads */
			goto restart;
		}
	}
	spin_unlock_bh(&mtp_hash.lock);

	if (work_on_cnt)
		mod_delayed_work(system_wq, &mtp_dev_worker, msecs_to_jiffies(MTP_D2D_TIMEOUT));
}

void mtp_hashinfo_init(void)
{
	spin_lock_init(&mtp_hash.lock);
	mtp_hash.rand_port = 1;
	INIT_HLIST_HEAD(&mtp_hash.bind);
	INIT_LIST_HEAD(&mtp_hash.dev_head);
	INIT_HLIST_NULLS_HEAD(&mtp_hash.listening, 0);
	INIT_HLIST_NULLS_HEAD(&mtp_hash.established, 0);
	INIT_DELAYED_WORK(&mtp_dev_worker, mtp_dev_work_process);
}

void mtp_hashinfo_exit(void)
{
	if (delayed_work_pending(&mtp_dev_worker))
		cancel_delayed_work(&mtp_dev_worker);
}

static bool mtp_port_used(unsigned char *mac, unsigned short snum)
{
	struct sock *sk;

	sk_for_each_bound(sk, &mtp_hash.bind) {
		struct mtp_sock *msk = (struct mtp_sock *)sk;
		if (msk->src_port == snum && (mtp_mac_any(msk->src_mac, ETH_ALEN) ||
			mtp_mac_any(mac, ETH_ALEN) ||
			!memcmp(msk->src_mac, mac, ETH_ALEN)))
			return true;
	}

	return false;
}

static struct net_device *dev_get_by_mac_rcu(const unsigned char *addr)
{
	struct net_device *dev;

	for_each_netdev_rcu(&init_net, dev) {
		if (memcmp(addr, dev->dev_addr, ETH_ALEN) == 0) {
			mtp_debug("dev_get_by_addr "MACFMT", index: %d\n", MACDATA(addr), dev->ifindex);
			return dev;
		}
	}

	return NULL;
}

int dev_get_mac_by_inetaddr(__be32 ifa_local, unsigned char *mac)
{
	struct net_device *dev;
	struct in_device *ind = NULL;
	struct in_ifaddr *ina = NULL;

	rcu_read_lock();
	for_each_netdev_rcu(&init_net, dev) {
		ind = in_dev_get(dev);
		if (!ind)
			continue;

		for (ina = (struct in_ifaddr *)ind->ifa_list; ina != NULL; ina = ina->ifa_next) {
			if (ina->ifa_address == ifa_local) {
				(void)memcpy_s(mac, ETH_ALEN, dev->dev_addr, ETH_ALEN);
				in_dev_put(ind);
				rcu_read_unlock();
				return 0;
			}
		}

		in_dev_put(ind);
	}

	rcu_read_unlock();
	return -1;
}

struct net_device *dev_get_by_mac(unsigned char *addr)
{
	struct net_device *dev;

	rcu_read_lock();
	dev = dev_get_by_mac_rcu(addr);
	if (dev && (dev->flags & IFF_UP))
		dev_hold(dev);
	else
		dev = NULL;
	rcu_read_unlock();
	return dev;
}

#define MTP_QUEUE_UDP_PORT 5001
#define MTP_QUEUE_IPADDR 0x12345678
#define MTP_QUEUE_IP_VER 4
#define MTP_QUEUE_IP_HDL 5

static struct sk_buff *mtp_build_udp_skb(struct sock *sk)
{
	struct sk_buff *skb;
	struct udphdr *uh;
	struct iphdr *iph;
	struct ethhdr *eth;
	struct mtp_sock *msk = mtp_sk(sk);

	skb = alloc_skb(MAX_HEADER, sk->sk_allocation);
	if (unlikely(!skb)) {
		mtp_warn("alloc_skb failed\n");
		return NULL;
	}
	skb_reserve(skb, MAX_HEADER - sizeof(struct udphdr));
	skb_reset_transport_header(skb);

	uh = udp_hdr(skb);
	uh->source = MTP_QUEUE_UDP_PORT;
	uh->dest = MTP_QUEUE_UDP_PORT;
	uh->len = 0;
	uh->check = 0;

	skb_push(skb, sizeof(struct iphdr));
	skb_reset_network_header(skb);
	iph = ip_hdr(skb);
	iph->version  = MTP_QUEUE_IP_VER;
	iph->ihl      = MTP_QUEUE_IP_HDL;
	iph->tos      = msk->tos;
	iph->ttl      = 1;
	iph->daddr    = MTP_QUEUE_IPADDR;
	iph->saddr    = MTP_QUEUE_IPADDR;
	iph->protocol = IPPROTO_UDP;
	iph->frag_off = htons(IP_DF);
	iph->id = 0;

	skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);
	eth = eth_hdr(skb);
	(void)memcpy_s(eth->h_source, ETH_ALEN, msk->src_mac, ETH_ALEN);
	(void)memcpy_s(eth->h_dest, ETH_ALEN, msk->src_mac, ETH_ALEN);
	eth->h_proto = htons(ETH_P_IP);

	skb->priority = sk->sk_priority;
	skb->mark = sk->sk_mark;
	return skb;
}

/* now the tx queue for mintp skbs is same as udp skbs */
static int mtp_get_queue_id(struct sock *sk, struct net_device *dev)
{
	struct sk_buff *skb;
	const struct net_device_ops *ops = dev->netdev_ops;
	int queue_index = 0;

	if (dev->real_num_tx_queues <= 1)
		return 0;

	if (!ops->ndo_select_queue)
		return 0;

	skb = mtp_build_udp_skb(sk);
	if (!skb)
		return 0;

	queue_index = ops->ndo_select_queue(dev, skb, NULL);
	mtp_debug("ndo_select_queue %d\n", queue_index);
	__kfree_skb(skb);
	return queue_index;
}

static void mtp_setup_caps(struct sock *sk, struct net_device *dev)
{
	u32 max_segs = 1;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	sk_dst_set(sk, NULL);
	sk->sk_route_caps = dev->features;
	if (sk->sk_route_caps & NETIF_F_GSO)
		sk->sk_route_caps |= NETIF_F_GSO_SOFTWARE;
	sk->sk_route_caps &= ~sk->sk_route_nocaps;
	sk->sk_gso_max_segs = max_segs;
	msk->mss = dev->mtu - MTP_HLEN_MIN;
	msk->rcv_wnd_scale = (u16)__order_base_2(msk->mss);
	if (sysctl_mtp_queue >= 0 && sysctl_mtp_queue < dev->real_num_tx_queues)
		msk->tx_queue = sysctl_mtp_queue;
	else
		msk->tx_queue = mtp_get_queue_id(sk, dev);
	mtp_info("%s tx_queue is %d\n", dev->name, msk->tx_queue);

	msk->is_hi1106 = false;
#if defined(CONFIG_CONNECTIVITY_HI1105) || defined(CONFIG_CONNECTIVITY_HI1103)
	if (strnstr(dev->name, "wlan", IFNAMSIZ) ||
	    strnstr(dev->name, "p2p-p2p", IFNAMSIZ) ||
	    strnstr(dev->name, "chba", IFNAMSIZ)) {
		if (mtp_hi1106_check()) {
			msk->is_hi1106 = true;
			mtp_info("on BISHENG(hi1106) devices\n");
		}
	}
#endif
}

int mtp_d2d_enable(struct mtp_sock *msk)
{
#if defined(CONFIG_CONNECTIVITY_HI1105) || defined(CONFIG_CONNECTIVITY_HI1103)
	struct mtp_dev_node *node;
	struct net_device *dev;

	spin_lock_bh(&mtp_hash.lock);
	rcu_read_lock();
	dev = rcu_dereference(msk->odev);
	if (!dev || (!strnstr(dev->name, "p2p", IFNAMSIZ) && !strnstr(dev->name, "wlan", IFNAMSIZ))) {
		rcu_read_unlock();
		spin_unlock_bh(&mtp_hash.lock);
		return -ENOTSUPP;
	}
	rcu_read_unlock();

	list_for_each_entry(node, &mtp_hash.dev_head, list) {
		if (node->dev == dev) {
			if (node->d2d_enable_cnt >= MTP_D2D_ENABLE_CNT_MAX) {
				spin_unlock_bh(&mtp_hash.lock);
				return -EBUSY;
			}
			node->d2d_enable_cnt++;
			msk->dev_node = node;
			if (node->state == MTP_D2D_OFF) {
				node->state = MTP_D2D_INIT;
				mod_delayed_work(system_wq, &mtp_dev_worker, 0);
			}
			spin_unlock_bh(&mtp_hash.lock);
			return 0;
		}
	}

	node = kmalloc(sizeof(*node), GFP_ATOMIC);
	if (!node) {
		spin_unlock_bh(&mtp_hash.lock);
		return -ENOMEM;
	}

	msk->dev_node = node;
	node->dev = dev;
	node->d2d_enable_cnt = 1;
	atomic_set(&node->bytes, 0);
	node->state = MTP_D2D_INIT;
	list_add(&node->list, &mtp_hash.dev_head);
	spin_unlock_bh(&mtp_hash.lock);

	mod_delayed_work(system_wq, &mtp_dev_worker, 0);
	return 0;
#else
	return -ENOTSUPP;
#endif
}

void mtp_d2d_disable(struct mtp_sock *msk)
{
	struct mtp_dev_node *node = (struct mtp_dev_node *)msk->dev_node;

	spin_lock_bh(&mtp_hash.lock);
	node->d2d_enable_cnt--;
	if (!node->d2d_enable_cnt)
		mod_delayed_work(system_wq, &mtp_dev_worker, 0);
	spin_unlock_bh(&mtp_hash.lock);
}

void mtp_dev_update_bytes(void *ptr, int bytes)
{
	struct mtp_dev_node *node = (struct mtp_dev_node *)ptr;

	if (!node)
		return;

	atomic_add(bytes, &node->bytes);

	if (node->state == MTP_D2D_OFF) {
		node->state = MTP_D2D_INIT;
		mod_delayed_work(system_wq, &mtp_dev_worker, 0);
	}
}

int mtp_get_port(struct sock *sk, unsigned short snum)
{
	int error;
	int i = 0;
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct net_device *dev;

	mtp_info("try to bind to port %u\n", snum);

	if (snum >= MTP_PORT_MAX)
		return -EINVAL;

	spin_lock_bh(&mtp_hash.lock);
	dev = dev_get_by_mac(msk->src_mac);
	if (!dev) {
		error = -EADDRNOTAVAIL;
		goto out;
	}

	mtp_setup_caps(sk, dev);
	sk->sk_bound_dev_if = dev->ifindex;
	rcu_assign_pointer(msk->odev, dev);
	dev_put(dev);
	if (snum) {
		if (mtp_port_used(msk->src_mac, snum)) {
			error = -EADDRINUSE;
		} else {
			msk->src_port = (__u8)snum;
			sk_add_bind_node(sk, &mtp_hash.bind);
			error = 0;
		}
	} else {
		while (mtp_port_used(msk->src_mac, mtp_hash.rand_port)) {
			mtp_hash.rand_port++;
			if (mtp_hash.rand_port >= MTP_PORT_MAX)
				mtp_hash.rand_port = 1;
			/* mtp spoort MTP_PORT_MAX connects, this help avoid endless loop */
			if (++i > MTP_PORT_MAX) {
				error = -EADDRINUSE;
				goto out;
			}
		}
		msk->src_port = mtp_hash.rand_port;
		sk_add_bind_node(sk, &mtp_hash.bind);
		error = 0;
	}
out:
	spin_unlock_bh(&mtp_hash.lock);

	return error;
}

void mtp_put_port(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	if (!msk->src_port)
		return;

	mtp_info("%u:%u try to release port\n", msk->src_port, msk->dst_port);
	spin_lock_bh(&mtp_hash.lock);
	if (!msk->src_port) {
		spin_unlock_bh(&mtp_hash.lock);
		return;
	}
	__sk_del_bind_node(sk);
	msk->src_port = 0;
	if (msk->d2d) {
		struct mtp_dev_node *node = (struct mtp_dev_node *)msk->dev_node;

		msk->d2d = false;
		msk->dev_node = NULL;
		if (node) {
			node->d2d_enable_cnt--;
			if (!node->d2d_enable_cnt)
				mod_delayed_work(system_wq, &mtp_dev_worker, 0);
		}
	}
	spin_unlock_bh(&mtp_hash.lock);
}

static void mtp_inherit_port(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	if (!msk->src_port)
		return;
	spin_lock_bh(&mtp_hash.lock);
	sk_add_bind_node(sk, &mtp_hash.bind);
	spin_unlock_bh(&mtp_hash.lock);
}

static bool mtp_skb_match_estab(struct mtp_sock *msk, const unsigned char *h_dest,
	const unsigned char *h_source, unsigned short dport, unsigned short sport)
{
	if (memcmp(msk->src_mac, h_dest, ETH_ALEN))
		return false;

	if (((struct sock *)msk)->sk_state != TCP_SYN_SENT || !mtp_mac_any(msk->dst_mac, ETH_ALEN)) {
		if (memcmp(msk->dst_mac, h_source, ETH_ALEN))
			return false;
	}

	if (msk->src_port != dport || msk->dst_port != sport)
		return false;

	return true;
}

int mtp_lib_hash(struct sock *sk)
{
	struct hlist_nulls_head *nulls_head = NULL;
	if (sk->sk_state == TCP_CLOSE)
		return -1;

	WARN_ON(!sk_unhashed(sk));
	spin_lock_bh(&mtp_hash.lock);
	if (sk->sk_state != TCP_LISTEN) {
		struct sock *osk;
		const struct hlist_nulls_node *node;
		sk_nulls_for_each_rcu(osk, node, &mtp_hash.established) {
			struct mtp_sock *msk = (struct mtp_sock *)sk;
			struct mtp_sock *omsk = (struct mtp_sock *)osk;

			if (unlikely(mtp_skb_match_estab(omsk, msk->src_mac, msk->dst_mac,
				msk->src_port, msk->dst_port))) {
				spin_unlock_bh(&mtp_hash.lock);
				return -1;
			}
		}
		nulls_head = &mtp_hash.established;
	} else {
		nulls_head = &mtp_hash.listening;
		sock_set_flag(sk, SOCK_RCU_FREE);
	}

	__sk_nulls_add_node_rcu(sk, nulls_head);
	spin_unlock_bh(&mtp_hash.lock);
	return 0;
}

void mtp_lib_unhash(struct sock *sk)
{
	if (sk_unhashed(sk))
		return;

	spin_lock_bh(&mtp_hash.lock);
	__sk_nulls_del_node_init_rcu(sk);
	spin_unlock_bh(&mtp_hash.lock);
}

struct sock *mtp_lookup_skb(const struct sk_buff *skb, u16 dport, u16 sport)
{
	struct sock *sk;
	const struct ethhdr *eth = eth_hdr(skb);
	const struct hlist_nulls_node *node;

	if (!dport || !sport)
		return NULL;

ebegin:
	sk_nulls_for_each_rcu(sk, node, &mtp_hash.established) {
		struct mtp_sock *msk = (struct mtp_sock *)sk;
		if (likely(mtp_skb_match_estab(msk, eth->h_dest, eth->h_source,
			dport, sport))) {
			if (unlikely(!refcount_inc_not_zero(&sk->sk_refcnt)))
				return NULL;
			if (unlikely(!mtp_skb_match_estab(msk, eth->h_dest, eth->h_source,
				dport, sport))) {
				sock_put(sk);
				goto ebegin;
			}
			return sk;
		}
	}

	sk_nulls_for_each_rcu(sk, node, &mtp_hash.listening) {
		struct mtp_sock *msk = (struct mtp_sock *)sk;

		if (memcmp(msk->src_mac, eth->h_dest, ETH_ALEN))
			continue;
		if (msk->src_port != dport)
			continue;
		if (unlikely(!refcount_inc_not_zero(&sk->sk_refcnt)))
			return NULL;
		return sk;
	}

	return NULL;
}

static u32 mtp_cookie_hash(const unsigned char *smac,
		       const unsigned char *dmac,
		       __be16 sport, __be16 dport, u32 sseq)
{
	struct {
		u8 smac_align[ETH_ALEN + 2];
		u8 dmac_align[ETH_ALEN + 2];
		u32 sseq;
		__be16 sport;
		__be16 dport;
	} __aligned(SIPHASH_ALIGNMENT) combined;

	net_get_random_once(&syncookie_secret, sizeof(syncookie_secret));
	(void)memcpy_s(combined.smac_align, ETH_ALEN, smac, ETH_ALEN);
	combined.smac_align[6] = 0;
	combined.smac_align[7] = 0;
	(void)memcpy_s(combined.dmac_align, ETH_ALEN, dmac, ETH_ALEN);
	combined.dmac_align[6] = 0;
	combined.dmac_align[7] = 0;

	combined.sseq = sseq;
	combined.sport = sport;
	combined.dport = dport;
	return siphash(&combined, offsetofend(typeof(combined), dport),
		       &syncookie_secret);
}

u32 mtp_create_syn_cookie(const struct ethhdr *eth, const struct mtphdr *mtph)
{
	u32 count = mtp_cookie_time();
	return (mtp_cookie_hash(eth->h_source, eth->h_dest, mtph->src, mtph->dst,
		ntohl(mtph->seq)) + count);
}

bool mtp_check_syn_cookie(const struct ethhdr *eth, const struct mtphdr *mtph)
{
	u32 cookie = ntohl(mtph->ack_seq) - 1;
	u32 diff;
	u32 count = mtp_cookie_time();

	u32 seq = mtp_cookie_hash(eth->h_source, eth->h_dest, mtph->src,
				  mtph->dst, ntohl(mtph->seq)) + count;

	diff = seq - cookie;
	if (diff >= MTP_SYNCOOKIE_AGE)
		return false;

	return true;
}

static void mtp_setup_newsk(struct sock *newsk, const struct sock *sk,
	const struct sk_buff *skb)
{
	const struct ethhdr *eth = eth_hdr(skb);
	const struct mtphdr *mtph = (struct mtphdr *)skb->data;
	u16 recv_mss = MTP_SKB_CB(skb)->mss;
	u32 recv_msg_size = MTP_SKB_CB(skb)->max_msg_size << MTP_MSG_SHIFT;
	struct mtp_sock *msk = mtp_sk(newsk);

	(void)memcpy_s(msk->src_mac, ETH_ALEN, eth->h_dest, ETH_ALEN);
	(void)memcpy_s(msk->dst_mac, ETH_ALEN, eth->h_source, ETH_ALEN);
	msk->src_port = mtph->dst;
	msk->dst_port = mtph->src;
	mtp_set_state(newsk, TCP_ESTABLISHED);
	INIT_LIST_HEAD(&msk->accept_queue.accept);
	if (msk->mss > recv_mss)
		msk->mss = recv_mss;
	if (msk->max_msg_size > recv_msg_size)
		msk->max_msg_size = recv_msg_size;
	msk->cur_rcv_msg_size = 0;
	msk->cur_snd_msg_size = 0;
	msk->sendpage_msg_size = 0;
	if (newsk->sk_sndbuf < (msk->max_msg_size << 1)) {
		newsk->sk_sndbuf = msk->max_msg_size << 1;
		newsk->sk_rcvbuf = newsk->sk_sndbuf;
	}
	newsk->sk_userlocks |= (SOCK_SNDBUF_LOCK | SOCK_RCVBUF_LOCK);
	mtp_debug("%u:%u max_msg_size %u sk_sndbuf %u\n", msk->src_port, msk->dst_port,
		msk->max_msg_size, newsk->sk_sndbuf);
	/* listeners have SOCK_RCU_FREE, not the children */
	sock_reset_flag(newsk, SOCK_RCU_FREE);
	mtp_init_sock_base(newsk);
	if (memcmp(msk->dst_mac, msk->src_mac, ETH_ALEN) == 0)
		msk->is_loopback = true;
	msk->snd_wnd = ntohs(mtph->win);
	msk->rcv_wnd_scale = (u16)__order_base_2(msk->mss);
	msk->rcv_wnd = mtp_full_win(sk);
	mtp_debug("%u:%u rcv_scale %u rcv_wnd %u\n", msk->src_port, msk->dst_port,
		msk->rcv_wnd_scale, msk->rcv_wnd);
	msk->rcv_nxt = ntohl(mtph->seq) + 1; /* What we want to receive next */
	WRITE_ONCE(msk->copied_seq, msk->rcv_nxt);
	msk->write_seq = ntohl(mtph->ack_seq); /* Tail(+1) of data held in mtp send buffer */
	msk->snd_nxt = msk->write_seq; /* Next sequence we send	 */
	msk->snd_una = msk->write_seq; /* First byte we want an ack for */
	msk->rcv_tstamp = MTP_JIFFIERS32;
	msk->slow_start_mstamp = msk->cur_mstamp;
}

struct sock *mtp_get_cookie_sock(struct sock *sk, const struct sk_buff *skb)
{
	struct sock *newsk;
	const struct ethhdr *eth = eth_hdr(skb);
	const struct mtphdr *mtph = mtp_hdr(skb);
	int err;

	if (mtp_check_syn_cookie(eth, mtph) != true) {
		mtp_err("server %u mtp_check_syn_cookie failed\n", mtp_sk(sk)->src_port);
		return NULL;
	}

	if (sk_acceptq_is_full(sk)) {
		__MTP_INC_STATS(MTP_MIB_ATTEMPTFAILS);
		mtp_err("server %u sk_acceptq_is_full\n", mtp_sk(sk)->src_port);
		return NULL;
	}

	newsk = sk_clone_lock(sk, GFP_ATOMIC);
	if (!newsk) {
		__MTP_INC_STATS(MTP_MIB_ATTEMPTFAILS);
		mtp_err("server %u sk_clone_lock failed\n", mtp_sk(sk)->src_port);
		return NULL;
	}

	mtp_setup_newsk(newsk, sk, skb);
	__MTP_INC_STATS(MTP_MIB_PASSIVEOPENS);
	mtp_inherit_port(newsk);
	err = newsk->sk_prot->hash(newsk);
	if (unlikely(err)) {
		bh_unlock_sock(newsk);
		mtp_info("%u:%u hash failed\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port);
		sock_put(newsk);
		sock_set_flag(newsk, SOCK_DEAD);
		mtp_done(newsk);
		return NULL;
	}

	if (sock_flag(newsk, SOCK_KEEPOPEN))
		sk_reset_timer(newsk, &newsk->sk_timer, jiffies + mtp_sk(newsk)->keepalive_time);

	mtp_info("new socket %u:%u connected\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port);
	return newsk;
}

int mtp_mib_init(void)
{
	mtp_statistics = alloc_percpu(struct mtp_mib);
	if (!mtp_statistics)
		return -ENOMEM;

	MTP_ADD_STATS(MTP_MIB_RTOALGORITHM, 1);
	MTP_ADD_STATS(MTP_MIB_RTOMIN, MTP_RTO_MIN * MSEC_PER_SEC / HZ);
	MTP_ADD_STATS(MTP_MIB_RTOMAX, MTP_RTO_MAX * MSEC_PER_SEC / HZ);
	MTP_ADD_STATS(MTP_MIB_MAXCONN, -1);
	return 0;
}

void mtp_mib_exit(void)
{
	free_percpu(mtp_statistics);
}

#ifdef CONFIG_PROC_FS
enum mtp_seq_states {
	MTP_SEQ_STATE_LISTENING,
	MTP_SEQ_STATE_ESTABLISHED,
};

struct mtp_iter_state {
	struct seq_net_private	p;
	enum mtp_seq_states	state;
	int			offset;
	int			num;
	loff_t			last_pos;
};

static void mtp_seq_show_sock(struct sock *sk, struct seq_file *f, int i)
{
	int timer_active;
	unsigned long timer_expires;
	const struct mtp_sock *msk = mtp_sk(sk);
	int rx_queue;
	int state;
	const unsigned char *s = &msk->src_mac[0];
	const unsigned char *d = &msk->dst_mac[0];

	if (msk->rtx.pend) {
		timer_active	= msk->rtx.pend;
		timer_expires	= msk->rtx.timeout;
	} else if (timer_pending(&sk->sk_timer)) {
		timer_active	= MTP_KEEPALIVE_TIMER;
		timer_expires	= sk->sk_timer.expires;
	} else {
		timer_active = 0;
		timer_expires = jiffies;
	}

	state = mtp_state_load(sk);
	if (state == TCP_LISTEN)
		rx_queue = READ_ONCE(sk->sk_ack_backlog);
	else
		rx_queue = max_t(int, READ_ONCE(msk->rcv_nxt) - READ_ONCE(msk->copied_seq), 0);

	seq_printf(f, "%4d: %02X:%02X:%02X:%02X:%02X:%02X_%02X %02X:%02X:%02X:%02X:%02X:%02X_%02X "
			"%02X %08X:%08X %02X:%08lX %08X %5u %8d %lu %d %pK %lu %u %d %u %d",
		i, s[0], s[1], s[2], s[3], s[4], s[5], msk->src_port,
		d[0], d[1], d[2], d[3], d[4], d[5], msk->dst_port, state,
		READ_ONCE(msk->write_seq) - msk->snd_una,
		rx_queue,
		timer_active,
		jiffies_delta_to_clock_t(timer_expires - jiffies),
		msk->rtx.retransmits,
		from_kuid_munged(seq_user_ns(f), sock_i_uid(sk)),
		msk->rtx.probes,
		sock_i_ino(sk),
		refcount_read(&sk->sk_refcnt), sk,
		jiffies_to_clock_t(msk->rtx.rto),
		msk->ca_state,
		msk->cpu_id,
		msk->snd_cwnd,
		state == TCP_LISTEN ? 0 :
			(mtp_in_initial_slowstart(msk) ? -1 : msk->snd_ssthresh));
}

#define TMPSZ 150

static int mtp_seq_show(struct seq_file *seq, void *v)
{
	struct mtp_iter_state *st;
	seq_setwidth(seq, TMPSZ - 1);
	if (v == SEQ_START_TOKEN) {
		seq_puts(seq, "  sl  local_address rem_address   st tx_queue "
			   "rx_queue tr tm->when retrnsmt   uid  probes "
			   "inode ref sk rto ca_state cpu_id snd_cwnd snd_ssthresh");
		goto out;
	}
	st = seq->private;
	mtp_seq_show_sock(v, seq, st->num);
out:
	seq_pad(seq, '\n');
	return 0;
}

static void *mtp_listening_get_next(struct seq_file *seq, void *cur)
{
	struct mtp_iter_state *st = seq->private;
	struct sock *sk = cur;

	if (!sk) {
		spin_lock_bh(&mtp_hash.lock);
		sk = sk_nulls_head(&mtp_hash.listening);
		st->offset = 0;
	} else {
		++st->num;
		++st->offset;
		sk = sk_nulls_next(sk);
	}

	if (sk)
		return sk;
	spin_unlock_bh(&mtp_hash.lock);
	st->offset = 0;
	return NULL;
}

static void *mtp_listening_get_idx(struct seq_file *seq, loff_t *pos)
{
	struct mtp_iter_state *st = seq->private;
	void *rc;

	st->offset = 0;
	rc = mtp_listening_get_next(seq, NULL);

	while (rc && *pos) {
		rc = mtp_listening_get_next(seq, rc);
		--*pos;
	}
	return rc;
}

static void *mtp_established_get_first(struct seq_file *seq)
{
	struct mtp_iter_state *st = seq->private;
	struct sock *sk;

	st->offset = 0;
	spin_lock_bh(&mtp_hash.lock);
	sk = sk_nulls_head(&mtp_hash.established);
	if (sk)
		return sk;

	spin_unlock_bh(&mtp_hash.lock);
	return NULL;
}

static void *mtp_established_get_next(struct seq_file *seq, void *cur)
{
	struct sock *sk = cur;
	struct mtp_iter_state *st = seq->private;

	++st->num;
	++st->offset;
	sk = sk_nulls_next(sk);
	if (sk)
		return sk;

	spin_unlock_bh(&mtp_hash.lock);
	return NULL;
}

static void *mtp_established_get_idx(struct seq_file *seq, loff_t pos)
{
	void *rc = mtp_established_get_first(seq);
	while (rc && pos) {
		rc = mtp_established_get_next(seq, rc);
		--pos;
	}
	return rc;
}

static void *mtp_get_idx(struct seq_file *seq, loff_t pos)
{
	void *rc;
	struct mtp_iter_state *st = seq->private;

	st->state = MTP_SEQ_STATE_LISTENING;
	rc = mtp_listening_get_idx(seq, &pos);
	if (!rc) {
		st->state = MTP_SEQ_STATE_ESTABLISHED;
		rc = mtp_established_get_idx(seq, pos);
	}

	return rc;
}

static void *mtp_seek_last_pos(struct seq_file *seq)
{
	struct mtp_iter_state *st = seq->private;
	int offset = st->offset;
	int orig_num = st->num;
	void *rc = NULL;

	switch (st->state) {
	case MTP_SEQ_STATE_LISTENING:
		rc = mtp_listening_get_next(seq, NULL);
		while (offset-- && rc)
			rc = mtp_listening_get_next(seq, rc);
		if (rc)
			break;
		st->state = MTP_SEQ_STATE_ESTABLISHED;
		fallthrough;
	case MTP_SEQ_STATE_ESTABLISHED:
		rc = mtp_established_get_first(seq);
		while (offset-- && rc)
			rc = mtp_established_get_next(seq, rc);
	}

	st->num = orig_num;
	return rc;
}

static void *mtp_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct mtp_iter_state *st = seq->private;
	void *rc;

	if (*pos && *pos == st->last_pos) {
		rc = mtp_seek_last_pos(seq);
		if (rc)
			goto out;
	}

	st->state = MTP_SEQ_STATE_LISTENING;
	st->num = 0;
	st->offset = 0;
	rc = *pos ? mtp_get_idx(seq, *pos - 1) : SEQ_START_TOKEN;

out:
	st->last_pos = *pos;
	return rc;
}

static void *mtp_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct mtp_iter_state *st = seq->private;
	void *rc = NULL;

	if (v == SEQ_START_TOKEN) {
		rc = mtp_get_idx(seq, 0);
		goto out;
	}

	switch (st->state) {
	case MTP_SEQ_STATE_LISTENING:
		rc = mtp_listening_get_next(seq, v);
		if (!rc) {
			st->state = MTP_SEQ_STATE_ESTABLISHED;
			st->offset = 0;
			rc = mtp_established_get_first(seq);
		}
		break;
	case MTP_SEQ_STATE_ESTABLISHED:
		rc = mtp_established_get_next(seq, v);
		break;
	}
out:
	++*pos;
	st->last_pos = *pos;
	return rc;
}

static void mtp_seq_stop(struct seq_file *seq, void *v)
{
	struct mtp_iter_state *st = seq->private;

	switch (st->state) {
	case MTP_SEQ_STATE_LISTENING:
		if (v != SEQ_START_TOKEN)
			spin_unlock_bh(&mtp_hash.lock);
		break;
	case MTP_SEQ_STATE_ESTABLISHED:
		if (v)
			spin_unlock_bh(&mtp_hash.lock);
		break;
	}
}

static const struct seq_operations mtp_seq_ops = {
	.show		= mtp_seq_show,
	.start		= mtp_seq_start,
	.next		= mtp_seq_next,
	.stop		= mtp_seq_stop,
};

struct mtp_seq_afinfo {
	sa_family_t family;
};

static struct mtp_seq_afinfo mtp_seq_afinfo = {
	.family	= AF_INET,
};

static const struct snmp_mib snmp_mtp_list[] = {
	SNMP_MIB_ITEM("RtoAlgorithm", MTP_MIB_RTOALGORITHM),
	SNMP_MIB_ITEM("RtoMin", MTP_MIB_RTOMIN),
	SNMP_MIB_ITEM("RtoMax", MTP_MIB_RTOMAX),
	SNMP_MIB_ITEM("MaxConn", MTP_MIB_MAXCONN),
	SNMP_MIB_ITEM("ActiveOpens", MTP_MIB_ACTIVEOPENS),
	SNMP_MIB_ITEM("PassiveOpens", MTP_MIB_PASSIVEOPENS),
	SNMP_MIB_ITEM("AttemptFails", MTP_MIB_ATTEMPTFAILS),
	SNMP_MIB_ITEM("EstabResets", MTP_MIB_ESTABRESETS),
	SNMP_MIB_ITEM("CurrEstab", MTP_MIB_CURRESTAB),
	SNMP_MIB_ITEM("InSegs", MTP_MIB_INSEGS),
	SNMP_MIB_ITEM("OutSegs", MTP_MIB_OUTSEGS),
	SNMP_MIB_ITEM("RetransSegs", MTP_MIB_RETRANSSEGS),
	SNMP_MIB_ITEM("InErrs", MTP_MIB_INERRS),
	SNMP_MIB_ITEM("OutRsts", MTP_MIB_OUTRSTS),
	SNMP_MIB_ITEM("TlsCurrTxSw", MTP_MIB_TLSCURRTXSW),
	SNMP_MIB_ITEM("TlsCurrRxSw", MTP_MIB_TLSCURRRXSW),
	SNMP_MIB_ITEM("TlsTxSw", MTP_MIB_TLSTXSW),
	SNMP_MIB_ITEM("TlsRxSw", MTP_MIB_TLSRXSW),
	SNMP_MIB_ITEM("TlsDecryptError", MTP_MIB_TLSDECRYPTERROR),
	SNMP_MIB_SENTINEL
};

static int mtp_stat_show(struct seq_file *seq, void *v)
{
	unsigned long buff[MTP_MIB_MAX];
	int i;

	(void)memset_s(buff, sizeof(buff), 0, sizeof(buff));
	snmp_get_cpu_field_batch(buff, snmp_mtp_list, mtp_statistics);
	for (i = 0; snmp_mtp_list[i].name; i++) {
		if (snmp_mtp_list[i].entry == MTP_MIB_MAXCONN)
			seq_printf(seq, "%s: %ld\n", snmp_mtp_list[i].name, buff[i]);
		else
			seq_printf(seq, "%s: %lu\n", snmp_mtp_list[i].name, buff[i]);
	}
	return 0;
}

static int mtp_proc_init_net(struct net *net)
{
	/* 0444 is -r--r--r-- */
	if (!proc_create_net_data("mintp", 0444, net->proc_net, &mtp_seq_ops,
			sizeof(struct mtp_iter_state), &mtp_seq_afinfo))
		return -ENOMEM;
	if (!proc_create_net_single("mintp_stat", 0444, net->proc_net, mtp_stat_show, NULL)) {
		remove_proc_entry("mintp", net->proc_net);
		return -ENOMEM;
	}
	return 0;
}

static void mtp_proc_exit_net(struct net *net)
{
	remove_proc_entry("mintp", net->proc_net);
	remove_proc_entry("mintp_stat", net->proc_net);
}

static struct pernet_operations mtp_net_ops = {
	.init = mtp_proc_init_net,
	.exit = mtp_proc_exit_net,
};

int mtp_proc_init(void)
{
	return register_pernet_subsys(&mtp_net_ops);
}

void mtp_proc_exit(void)
{
	unregister_pernet_subsys(&mtp_net_ops);
}
#endif /* CONFIG_PROC_FS */