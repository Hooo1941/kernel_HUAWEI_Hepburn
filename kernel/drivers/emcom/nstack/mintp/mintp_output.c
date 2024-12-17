/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * License terms: GNU General Public License (GPL)
 * Author: songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 *
 */
#include "mintp.h"
#include "mintp_congestion.h"
#include "mintp_timer.h"
#include "mintp_conn.h"
#include "mintp_output.h"

struct msq_tasklet {
	struct tasklet_struct	tasklet;
	struct list_head	head; /* queue of mtp sockets */
};
static struct msq_tasklet __percpu *msq_task;

void mtp_msq_handler(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	if (msk->lost_out > msk->retrans_out) {
		mtp_mstamp_refresh(msk);
		mtp_xmit_retransmit_queue(sk);
	}
	mtp_write_xmit(sk, 0, GFP_ATOMIC);
}

/*
 * One tasklet per cpu tries to send more skbs.
 * We run in tasklet context but need to disable irqs when
 * transferring msq->head because mtp_wfree() might
 * interrupt us (non NAPI drivers)
 */
static void mtp_tasklet_func(unsigned long data)
{
	struct msq_tasklet *msq = (struct msq_tasklet *)data;
	LIST_HEAD(list);
	unsigned long flags;
	struct list_head *q, *n;
	struct mtp_sock *msk;
	struct sock *sk;

	local_irq_save(flags);
	list_splice_init(&msq->head, &list);
	local_irq_restore(flags);

	list_for_each_safe(q, n, &list) {
		msk = list_entry(q, struct mtp_sock, msq_node);
		list_del(&msk->msq_node);

		sk = (struct sock *)msk;
		mtp_debug("try socket %u:%u\n", msk->src_port, msk->dst_port);
		smp_mb__before_atomic();
		clear_bit(MTP_MSQ_QUEUED, &sk->sk_tsq_flags);

		bh_lock_sock(sk);
		if (!sock_owned_by_user(sk)) {
			clear_bit(MTP_MSQ_DEFERRED, &sk->sk_tsq_flags);
			mtp_msq_handler(sk);
		} else {
			set_bit(TCP_TSQ_DEFERRED, &sk->sk_tsq_flags);
		}
		bh_unlock_sock(sk);
		sk_free(sk);
	}
}

int mtp_tasklet_init(void)
{
	int i;

	msq_task = alloc_percpu(struct msq_tasklet);
	if (!msq_task) {
		mtp_err("alloc_percpu fail\n");
		return -1;
	}

	for_each_possible_cpu(i) {
		struct msq_tasklet *msq = per_cpu_ptr(msq_task, i);

		INIT_LIST_HEAD(&msq->head);
		tasklet_init(&msq->tasklet,
			     mtp_tasklet_func,
			     (unsigned long)msq);
	}
	return 0;
}

void mtp_tasklet_destroy(void)
{
	int i;
	for_each_possible_cpu(i) {
		struct msq_tasklet *msq = per_cpu_ptr(msq_task, i);

		tasklet_kill(&msq->tasklet);
	}
	free_percpu(msq_task);
}

static bool mtp_small_queue_check(struct sock *sk, const struct sk_buff *skb,
				  unsigned int factor, struct net_device *dev)
{
	unsigned int limit;
	struct mtp_sock *msk = mtp_sk(sk);

	if ((msk->xmit == mtp_direct_xmit) && netif_xmit_stopped(netdev_get_tx_queue(dev, msk->tx_queue)) &&
		skb != mtp_write_queue_head(sk)) {
		msk->stats.out_dev_limit++;
		return true;
	}

	limit = mtp_limit_output_bytes;
	limit <<= factor;

	if (refcount_read(&sk->sk_wmem_alloc) > limit) {
		if (skb == sk->sk_write_queue.next ||
		    skb->prev == sk->sk_write_queue.next)
			return false;

		set_bit(MTP_MSQ_THROTTLED, &sk->sk_tsq_flags);
		smp_mb__after_atomic();
		if (refcount_read(&sk->sk_wmem_alloc) > limit) {
			msk->stats.out_msq_limit++;
			return true;
		}
	}
	return false;
}

static inline void mtp_event_ack_sent(struct sock *sk, u32 rcv_nxt)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (unlikely(rcv_nxt != msk->rcv_nxt))
		return;

	if (unlikely(msk->delack.ofo_ack > MTP_FASTRETRANS_THRESH))
		msk->delack.ofo_ack = MTP_FASTRETRANS_THRESH;

	msk->delack.compress_ack = 0;
	msk->rcv_wnd_free = 0;
	msk->delack.ack_pend = 0;
	msk->delack.quick = 0;
	msk->delack.enable = 0;
}

static void mtp_wfree(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;
	struct mtp_sock *msk = mtp_sk(sk);
	unsigned long flags, nval, oval;
	unsigned int limit;
	struct msq_tasklet *msq;
	bool empty;

	if (refcount_sub_and_test(skb->truesize - 1, &sk->sk_wmem_alloc))
		mtp_err("%u:%u sk_wmem_alloc should not 0\n", msk->src_port, msk->dst_port);

	limit = mtp_limit_output_bytes;
	if (refcount_read(&sk->sk_wmem_alloc) >= (limit >> 3))
		goto out;

#if !defined(CONFIG_HW_NSTACK_MINTP_MODULE) || defined(CONFIG_ARM64)
	if (refcount_read(&sk->sk_wmem_alloc) >= SKB_TRUESIZE(1) && this_cpu_ksoftirqd() == current)
		goto out;
#endif
	do {
		oval = READ_ONCE(sk->sk_tsq_flags);
		if (!(oval & MTPF_MSQ_THROTTLED) || (oval & MTPF_MSQ_QUEUED))
			goto out;

		nval = (oval & ~MTPF_MSQ_THROTTLED) | MTPF_MSQ_QUEUED | MTPF_MSQ_DEFERRED;
		nval = cmpxchg(&sk->sk_tsq_flags, oval, nval);
	} while (nval != oval);

	if (mtp_send_need_sched(msk)) {
		if (schedule_work_on(msk->cpu_id, &msk->worker))
			sock_hold(sk);
		goto out;
	}
	/* queue this socket to tasklet queue */
	local_irq_save(flags);
	msq = this_cpu_ptr(msq_task);
	empty = list_empty(&msq->head);
	list_add(&msk->msq_node, &msq->head);
	if (empty) {
		mtp_debug("tasklet_schedule msq\n");
		tasklet_schedule(&msq->tasklet);
	}
	local_irq_restore(flags);
	return;
out:
	sk_free(sk);
}

struct mtp_out_option {
	u8	options; /* bit field of OPTION_* */
	u8	num_sack_blocks; /* number of SACK blocks to include */
};

static u8 mtp_get_out_options(struct mtp_sock *msk, struct sk_buff *skb,
	u16 data_len, struct mtp_out_option *opts)
{
	u8 size = 0;
	u8 max_size;
	u8 avail_num_sacks;
	struct mtp_skb_cb *dcb = MTP_SKB_CB(skb);

	opts->options = 0;

	max_size = ALIGN(msk->mss - data_len, 4); /* ALIGN to 4 bytes */
	if (max_size > MTPOLEN_MAX)
		max_size = MTPOLEN_MAX;

	if (unlikely(dcb->type == MTPHDR_TYPE_HANDSHAKE)) {
		opts->options |= 1 << MTPOPT_MSS;
		size += MTPOLEN_MSS;
		opts->options |= 1 << MTPOPT_MSG_SIZE;
		size += MTPOLEN_MSG_SIZE;
	}

	if (unlikely(dcb->opt_flags)) {
		if (max_size >= (size + MTPOLEN_FLAGS)) {
			size += MTPOLEN_FLAGS;
			opts->options |= 1 << MTPOPT_FLAGS;
			msk->ca_recovery = 0;
		}
	}

	if (unlikely(msk->num_sacks)) {
		u8 avail_bytes = max_size - size;
		if (avail_bytes >= (MTPOLEN_SACK_BASE_ALIGNED + MTPOLEN_SACK_PERBLOCK)) {
			avail_num_sacks = (avail_bytes - MTPOLEN_SACK_BASE_ALIGNED) / MTPOLEN_SACK_PERBLOCK;
			avail_num_sacks = min(avail_num_sacks, msk->num_sacks);
			size += MTPOLEN_SACK_BASE_ALIGNED +
				avail_num_sacks * MTPOLEN_SACK_PERBLOCK;
			opts->options |= 1 << MTPOPT_SACK;
			opts->num_sack_blocks = avail_num_sacks;
		}
	}

	return size;
}

static void mtp_options_write(__be32 *ptr, struct mtp_sock *msk,
	const struct sk_buff *skb, const struct mtp_out_option *opts)
{
	struct mtp_skb_cb *dcb = MTP_SKB_CB(skb);

	if (opts->options & (1 << MTPOPT_MSS))
		*ptr++ = htonl((MTPOPT_MSS << MTP_OPT_OFFSET_24) |
			       (MTPOLEN_MSS << MTP_OPT_OFFSET_16) |
			       dcb->mss);

	if (opts->options & (1 << MTPOPT_MSG_SIZE))
		*ptr++ = htonl((MTPOPT_MSG_SIZE << MTP_OPT_OFFSET_24) |
			       (MTPOLEN_MSG_SIZE << MTP_OPT_OFFSET_16) |
			       dcb->max_msg_size);

	if (opts->options & (1 << MTPOPT_FLAGS))
		*ptr++ = htonl((MTPOPT_FLAGS  << MTP_OPT_OFFSET_24) |
			       (MTPOLEN_FLAGS << MTP_OPT_OFFSET_16) |
			       (dcb->opt_flags));

	if (opts->options & (1 << MTPOPT_SACK)) {
		struct mtp_sack_block *sp = msk->selective_acks;
		int this_sack;

		*ptr++ = htonl((MTPOPT_NOP  << MTP_OPT_OFFSET_24) |
			       (MTPOPT_NOP  << MTP_OPT_OFFSET_16) |
			       (MTPOPT_SACK <<  MTP_OPT_OFFSET_8) |
			       (MTPOLEN_SACK_BASE + (opts->num_sack_blocks *
						     MTPOLEN_SACK_PERBLOCK)));

		for (this_sack = 0; this_sack < opts->num_sack_blocks; ++this_sack) {
			*ptr++ = htonl(sp[this_sack].start_seq);
			*ptr++ = htonl(sp[this_sack].end_seq);
			mtp_debug("%u:%u send sack block %d %u~%u\n", msk->src_port, msk->dst_port, this_sack,
				sp[this_sack].start_seq, sp[this_sack].end_seq);
		}
	}
}

struct net_device *mtp_odev_get(struct mtp_sock *msk)
{
	struct net_device *dev;

	rcu_read_lock();
	dev = rcu_dereference(msk->odev);
	if (likely(dev && (dev->flags & IFF_UP)))
		dev_hold(dev);
	else
		dev = NULL;
	rcu_read_unlock();

	return dev;
}

static bool mtp_need_loss(void)
{
	static int last_loss = 0;

	if (g_mtp_loss > 0 && g_mtp_loss < MTP_LOSS_RATE_MAX) {
		unsigned int rand;
		get_random_bytes(&rand, sizeof(rand));
		if ((rand % MTP_LOSS_RATE_MAX) < g_mtp_loss || last_loss > 0) {
			if (last_loss <= 0) {
				last_loss = MTP_LOSS_PKT_NUM;
			} else {
				last_loss--;
			}
			return true;
		}
	}
	return false;
}

int mtp_direct_xmit(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	const struct net_device_ops *ops;

	local_bh_disable();
	dev_queue_xmit_nit(skb, dev);
	local_bh_enable();
	ops = dev->netdev_ops;
	return ops->ndo_start_xmit(skb, dev);
}

/* cache a packet in skb tmp list */
static inline void mtp_cache_pkt(struct mtp_sock *msk, struct sk_buff *skb)
{
	if (msk->tx_tmp_list == NULL) {
		msk->tx_tmp_list = skb;
		msk->tx_tmp_tail = skb;
	} else {
		CACHE_MTP_FRAME(msk->tx_tmp_tail, skb);
		msk->tx_tmp_tail = skb;
	}
	msk->tx_tmp_list_len++;

	return;
}


/* send all packets in skb tmp list, only for test on 1105 */
static int mtp_xmit_skb_list(struct mtp_sock *msk, struct sk_buff *skb)
{
	struct sk_buff *skb_next;
	int i = 0;

	while (skb) {
		skb_next = GET_NEXT_MTP_FRAME(skb);
		RESET_MTP_FRAME_PTR(skb);
		msk->xmit(skb);
		skb = skb_next;
		i++;
	}
	mtp_debug("mtp send cache queue, len = %d\n", i);

	return 0;
}

static inline int mtp_xmit_cache(struct mtp_sock *msk)
{
	struct sk_buff *skb;

	if (msk->tx_tmp_list != NULL) {
		skb = msk->tx_tmp_list;
		msk->tx_tmp_list = NULL;
		msk->tx_tmp_tail = NULL;
		msk->tx_tmp_list_len = 0;

		/* test on 1105 can use: return mtp_xmit_skb_list(msk, skb); */
		return msk->xmit(skb);
	} else {
		return 0;
	}
}

static int mtp_xmit_skb_hi1106(struct mtp_sock *msk, struct sk_buff *skb)
{
	/* send packet at once */
	if (MTP_SKB_CB(skb)->need_cache == MTP_NOCACHE && msk->tx_tmp_list == NULL)
		return msk->xmit(skb);

	/* caches packet in the queue */
	mtp_cache_pkt(msk, skb);
	/* need send packets in the queue? */
	if ((MTP_SKB_CB(skb)->need_cache == MTP_NOCACHE) || (msk->tx_tmp_list_len >= MTP_BATCH_SENDING_MAX))
		return mtp_xmit_cache(msk);
	return 0;
}

static int mtp_xmit_skb(struct mtp_sock *msk, struct sk_buff *skb)
{
	struct net_device *dev;

	dev = skb->dev;
	if (unlikely(mtp_need_loss())) {
		kfree_skb(skb);
		return 0;
	}

	/* set NULL to skb->head */
	RESET_MTP_FRAME_PTR(skb);

	if (unlikely(((struct sock *)msk)->sk_state == TCP_SYN_SENT && mtp_mac_any(msk->dst_mac, ETH_ALEN))) {
		struct neighbour *neigh;
		u32 nexthop = (__force u32)msk->inet.inet_daddr;
		rcu_read_lock_bh();
		neigh = __ipv4_neigh_lookup_noref(dev, nexthop);
		if (!IS_ERR_OR_NULL(neigh)) {
			if (neigh->nud_state & NUD_VALID) {
				struct ethhdr *eth = eth_hdr(skb);
				(void)memcpy_s(msk->dst_mac, ETH_ALEN, neigh->ha, ETH_ALEN);
				(void)memcpy_s(eth->h_dest, ETH_ALEN, msk->dst_mac, ETH_ALEN);
				rcu_read_unlock_bh();
				goto xmit_direct;
			}
		} else {
			neigh = __neigh_create(&arp_tbl, &nexthop, dev, false);
		}

		if (!IS_ERR(neigh)) {
			int res;

			sock_confirm_neigh(skb, neigh);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
			res = neigh_output(neigh, skb);
#else
			res = neigh_output(neigh, skb, false);
#endif
			rcu_read_unlock_bh();
			return res;
		}
		rcu_read_unlock_bh();
		return 0;
	}

	if (msk->is_hi1106)
		return mtp_xmit_skb_hi1106(msk, skb);

xmit_direct:
	return msk->xmit(skb);
}

static int mtp_loopback_skb(struct sk_buff *skb)
{
	int ret;
	/* do not fool net_timestamp_check() with various clock bases */
	__skb_pull(skb, ETH_HLEN);
	skb->tstamp = 0;
	skb_orphan(skb);
	local_bh_disable();
	ret = netif_rx(skb);
	local_bh_enable();
	return ret;
}

static struct sk_buff *mtp_clone_skb_for_transmit(struct sk_buff *skb,
				int clone_it, gfp_t gfp_mask, struct mtp_sock *msk)
{
	struct sk_buff *oskb = NULL;

	if (clone_it) {
		oskb = skb;
		if (unlikely(skb_cloned(skb)))
			skb = pskb_copy(skb, gfp_mask);
		else
			skb = skb_clone(skb, gfp_mask);
		if (unlikely(!skb)) {
			mtp_err("mtp_transmit_skb clone_it failed\n");
			return NULL;
		}
	}
	if (oskb)
		mtp_skb_mstamp_refresh(msk, oskb);

	return skb;
}

static void mtp_set_mtph_ver_flag(struct mtphdr *mtph, struct mtp_sock *msk,
	struct mtp_skb_cb *dcb, struct sock *sk, u16 data_len)
{
	mtph->ver = 1;
	mtph->proto = 0;
	mtph->src = msk->src_port;
	mtph->dst = msk->dst_port;
	mtph->type = dcb->type;
	if (dcb->flags & MTPHDR_RTX)
		mtph->rtx = 1;
	if (data_len) {
		msk->compress_probe++;
		/* if send over 1/4 window */
		if (msk->compress_probe >= (min_t(u32, msk->snd_cwnd, msk->snd_wnd) >> 2))
			dcb->flags |= MTPHDR_PROBE;
	}
	if (dcb->flags & MTPHDR_PROBE) {
		msk->compress_probe = 0;
		mtph->probe = 1;
	}
	msk->rcv_wnd = mtp_full_win(sk) - mtp_block_win(sk);
	mtph->win = htons(msk->rcv_wnd);
	mtph->seq = htonl(dcb->seq);
}

static void mtp_set_mtph_for_transmit(struct sk_buff *skb,
				struct mtp_sock *msk, u32 rcv_nxt)
{
	struct mtp_skb_cb *dcb = NULL;
	struct mtphdr *mtph = NULL;
	struct sock *sk = skb->sk;
	struct mtp_out_option opts;
	u16 data_len;
	u8 optlen;

	dcb = MTP_SKB_CB(skb);
	data_len = dcb->end_seq - dcb->seq;
	if (dcb->type == MTPHDR_TYPE_SYN || dcb->type == MTPHDR_TYPE_HANDSHAKE || dcb->type == MTPHDR_TYPE_FIN)
		data_len -= 1;
	if (msk->ca_recovery) {
		mtp_debug("%u:%u set MTPOPT_FLAG_REC\n", msk->src_port, msk->dst_port);
		dcb->opt_flags |= MTPOPT_FLAG_REC;
	}

	optlen = mtp_get_out_options(msk, skb, data_len, &opts);
	skb_push(skb, MTP_HLEN_MIN + optlen);
	skb_reset_network_header(skb);

	mtph = (struct mtphdr *)skb_network_header(skb);
	(void)memset_s(mtph, sizeof(struct mtphdr), 0, sizeof(struct mtphdr));
	mtp_set_mtph_ver_flag(mtph, msk, dcb, sk, data_len);
	mtph->len = htons(data_len);
	mtph->ack_seq = htonl(rcv_nxt);
	if (dcb->type != MTPHDR_TYPE_SYN && dcb->type != MTPHDR_TYPE_RST)
		mtp_event_ack_sent(sk, rcv_nxt);

	if (optlen)
		mtp_options_write((__be32 *)(mtph + 1), msk, skb, &opts);
	MTP_HL_SET(mtph, MTP_HLEN_MIN + optlen);
	if (data_len) {
		msk->stats.out_segs++;
		if (data_len < msk->mss)
			msk->stats.out_segs_unfulfilled++;
		BUG_ON(sk->sk_state == TCP_LISTEN);
		skb->destructor = mtp_wfree;
	} else {
		if (sk->sk_state == TCP_SYN_SENT &&
		    (dcb->type == MTPHDR_TYPE_SYN || dcb->type != MTPHDR_TYPE_HANDSHAKE))
			skb->destructor = mtp_wfree;
	}

	skb_set_queue_mapping(skb, msk->tx_queue);
}

static int mtp_transmit_skb(struct sock *sk, struct sk_buff *skb,
				int clone_it, gfp_t gfp_mask, u32 rcv_nxt)
{
	struct ethhdr *eth;
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;

	skb = mtp_clone_skb_for_transmit(skb, clone_it, gfp_mask, msk);
	if (skb == NULL)
		return -ENOBUFS;
	mtp_skb_mstamp_refresh(msk, skb);
	skb_orphan(skb);
	skb->sk = sk;
	skb->destructor = sock_wfree;
	skb_set_hash_from_sk(skb, sk);
	refcount_add(skb->truesize, &sk->sk_wmem_alloc);

	// set mtp header
	mtp_set_mtph_for_transmit(skb, msk, rcv_nxt);

	// set mac header
	skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);
	eth = eth_hdr(skb);
	(void)memcpy_s(eth->h_source, ETH_ALEN, msk->src_mac, ETH_ALEN);
	(void)memcpy_s(eth->h_dest, ETH_ALEN, msk->dst_mac, ETH_ALEN);
	eth->h_proto = htons(ETH_P_MTP);
	skb->protocol = htons(ETH_P_MTP);
	skb->priority = sk->sk_priority;

	mtp_dev_update_bytes(msk->dev_node, skb->len);
	MTP_INC_STATS(MTP_MIB_OUTSEGS);
	if (msk->is_loopback)
		ret = mtp_loopback_skb(skb);
	else
		ret = mtp_xmit_skb(msk, skb);
	if (ret != 0)
		msk->stats.out_dev_error++;
	return ret;
}

int mtp_retransmit_skb(struct sock *sk, struct sk_buff *skb)
{
	int err;
	struct mtp_sock *msk = mtp_sk(sk);

	if (unlikely(skb_fclone_busy(sk, skb)))
		return -EBUSY;

	MTP_SKB_CB(skb)->flags |= MTPHDR_RTX;
	MTP_SKB_CB(skb)->need_cache = MTP_NOCACHE;
	err = mtp_transmit_skb(sk, skb, 1, GFP_ATOMIC, msk->rcv_nxt);
	if (err == 0) {
		MTP_SKB_CB(skb)->sacked |= MTPCB_RETRANS;
		msk->retrans_out += 1;
	}
	msk->stats.out_segs_retrans++;
	MTP_INC_STATS(MTP_MIB_RETRANSSEGS);
	return err;
}

static void mtp_write_queue(struct sock *sk, struct sk_buff *skb, struct net_device *dev)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *hole = NULL;
	u32 max_segs;

	max_segs = MTP_SEG_BURST;
	mtp_for_write_queue_from(skb, sk) {
		__u8 sacked;

		if (skb == mtp_send_head(sk))
			break;

		/* we could do better than to assign each time */
		if (!hole)
			msk->retransmit_skb_hint = skb;

		if (msk->snd_cwnd - mtp_packets_in_flight(msk) <= 0)
			break;

		sacked = MTP_SKB_CB(skb)->sacked;
		if (msk->retrans_out >= msk->lost_out) {
			break;
		} else if (!(sacked & MTPCB_LOST)) {
			if (!hole && !(sacked & (MTPCB_SACKED_RETRANS | MTPCB_SACKED_ACKED)))
				hole = skb;
			continue;
		}

		if (sacked & (MTPCB_SACKED_ACKED | MTPCB_SACKED_RETRANS))
			continue;

		if (mtp_small_queue_check(sk, skb, 1, dev)) {
			mtp_info("%u:%u mtp_small_queue_check fail\n", msk->src_port, msk->dst_port);
			break;
		}
		skb->dev = dev;
		if (mtp_retransmit_skb(sk, skb))
			break;

		if (mtp_in_cwnd_reduction(sk))
			msk->prr_out += 1;
		max_segs--;
		if (max_segs <= 0)
			break;

		if (skb == mtp_write_queue_head(sk)) {
			msk->rtx.pend = MTP_RTX_TIMER_RETRANS;
			msk->rtx.timeout = jiffies + msk->rtx.rto;
			mtp_info("%u:%u seq %u rto %u timeout %u\n", msk->src_port, msk->dst_port,
				MTP_SKB_CB(skb)->seq, msk->rtx.rto, msk->rtx.timeout);
			sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
		}
	}
}

void mtp_xmit_retransmit_queue(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;
	struct net_device *dev;

	if (!msk->packets_out)
		return;

	dev = mtp_odev_get(msk);
	if (unlikely(!dev))
		return;
	if (msk->retransmit_skb_hint) {
		skb = msk->retransmit_skb_hint;
	} else {
		skb = mtp_write_queue_head(sk);
	}

	mtp_write_queue(sk, skb, dev);

	dev_put(dev);
}

void mtp_rearm_rto(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (!msk->packets_out) {
		msk->rtx.pend = 0;
	} else {
		u32 rto = msk->rtx.rto;
		/* Offset the time elapsed after installing regular RTO */
		if (msk->rtx.pend == MTP_RTX_TIMER_REO_TIMEOUT ||
		    msk->rtx.pend == MTP_RTX_TIMER_LOSS_PROBE) {
			s64 delta_us = mtp_rto_delta_us(sk);
			/* delta_us may not be positive if the socket is locked
			 * when the retrans timer fires and is rescheduled.
			 */
			rto = usecs_to_jiffies(max_t(int, delta_us, 1));
		}
		msk->rtx.pend = MTP_RTX_TIMER_RETRANS;
		msk->rtx.timeout = jiffies + rto;
		mtp_debug("%u:%u rto %u timeout %u\n", msk->src_port, msk->dst_port,
			msk->rtx.rto, msk->rtx.timeout);
		sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
	}
}

static void mtp_advance_send_head(struct sock *sk, const struct sk_buff *skb)
{
	if (mtp_skb_is_last(sk, skb))
		sk->sk_send_head = NULL;
	else
		sk->sk_send_head = mtp_write_queue_next(sk, skb);
}

static void mtp_event_new_data_sent(struct sock *sk, const struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	unsigned int prior_packets = msk->packets_out;

	mtp_advance_send_head(sk, skb);
	msk->snd_nxt = MTP_SKB_CB(skb)->end_seq;
	msk->packets_out += 1;
	if (!prior_packets || msk->rtx.pend == MTP_RTX_TIMER_LOSS_PROBE) {
		mtp_rearm_rto(sk);
		mtp_debug("%u:%u mtp_rearm_rto send_head %d\n", msk->src_port, msk->dst_port,
			sk->sk_send_head ? 1 : 0);
	}
}

static unsigned int mtp_cwnd_check(struct mtp_sock *msk)
{
	if (mtp_packets_in_flight(msk) >= msk->snd_cwnd) {
		msk->stats.out_cwnd_limit++;
		return 1;
	}

	if (msk->packets_out >= msk->snd_wnd) {
		msk->stats.out_rwnd_limit++;
		return 1;
	}
	return 0;
}

static void mtp_cwnd_validate(struct sock *sk, bool ca_cwnd_limited)
{
	struct mtp_sock *msk = mtp_sk(sk);

	/* Track the maximum number of outstanding packets in each
	 * window, and remember whether we were cwnd-limited then.
	 */
	if (!before(msk->snd_una, msk->max_packets_seq) ||
	    msk->packets_out > msk->max_packets_out ||
	    ca_cwnd_limited) {
		msk->max_packets_out = msk->packets_out;
		msk->max_packets_seq = msk->snd_nxt;
		msk->ca_cwnd_limited = ca_cwnd_limited;
		mtp_debug("%u:%u ca_cwnd_limited %d\n", msk->src_port, msk->dst_port,
			ca_cwnd_limited);
	}
}

static inline bool mtp_probe_time_start(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	return !msk->packets_out && mtp_send_head(sk) && !msk->rtx.pend;
}

static void mtp_init_tlp_timer(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	msk->rtx.pend = MTP_RTX_TIMER_PROBE;
	msk->rtx.probes = 0;
	msk->rtx.rto = min(mtp_get_rto(msk), MTP_RTO_MAX);
	msk->rtx.timeout = jiffies + msk->rtx.rto;
	mtp_debug("%u:%u MTP_RTX_TIMER_PROBE timer %u\n", msk->src_port, msk->dst_port, msk->rtx.rto);
	sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
}

/* This routine writes packets to the network.  It advances the
 * send_head.  This happens as incoming acks open up the remote
 * window for us.
 *
 * Send at most one packet when push_one > 0. Temporarily ignore
 * cwnd limit to force at most one packet out when push_one == 2.
 */
void mtp_write_xmit(struct sock *sk, int push_one, gfp_t gfp)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;
	unsigned int sent_pkts = 0;
	struct net_device *dev = mtp_odev_get(msk);

	mtp_debug("%u:%u mtp_write_xmit enter\n", msk->src_port, msk->dst_port);
	if (unlikely(!dev))
		return;

	mtp_mstamp_refresh(msk);

	while ((skb = mtp_send_head(sk))) {
		if (unlikely(mtp_cwnd_check(msk) && push_one != MTP_LOSS_PROB_THRES))
			break;
		if (test_bit(MTP_MSQ_DEFERRED, &sk->sk_tsq_flags))
			clear_bit(MTP_MSQ_DEFERRED, &sk->sk_tsq_flags);
		if (mtp_small_queue_check(sk, skb, 0, dev))
			break;
		/* Argh, we hit an empty skb(), presumably a thread
		 * is sleeping in sendmsg()/sk_stream_wait_memory().
		 * We do not want to send a pure-ack packet and have
		 * a strange looking rtx queue with empty packet(s).
		 */
		if (MTP_SKB_CB(skb)->end_seq == MTP_SKB_CB(skb)->seq)
			break;

		skb->dev = dev;
		MTP_SKB_CB(skb)->need_cache = (push_one ? MTP_NOCACHE : MTP_CACHE);
		if (unlikely(mtp_transmit_skb(sk, skb, 1, gfp, msk->rcv_nxt))) {
			mtp_info("%u:%u mtp_transmit_skb fail\n", msk->src_port, msk->dst_port);
			break;
		}
		mtp_event_new_data_sent(sk, skb);
		sent_pkts += 1;
		if (push_one)
			break;
	}

	/* transmit cached packets */
	if (!push_one && msk->is_hi1106)
		mtp_xmit_cache(msk);

	if (!mtp_send_head(sk) && !push_one)
		msk->stats.out_app_limit++;
	dev_put(dev);
	if (likely(sent_pkts)) {
		if (mtp_in_cwnd_reduction(sk))
			msk->prr_out += sent_pkts;
		/* Send one loss probe per tail loss episode. */
		if (push_one != MTP_LOSS_PROB_THRES)
			mtp_schedule_loss_probe(sk, false);

		mtp_cwnd_validate(sk, mtp_packets_in_flight(msk) >= msk->snd_cwnd);
		return;
	}

	if (mtp_probe_time_start(sk))
		mtp_init_tlp_timer(sk);
}

bool mtp_schedule_loss_probe(struct sock *sk, bool advancing_rto)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 timeout, rto_delta_us;

	/* Schedule a loss probe in 2*RTT for SACK capable connections
	 * in Open state, that are either limited by cwnd or application.
	 */
	if (!msk->packets_out || msk->ca_state != MTP_CA_OPEN)
		return false;

	if ((msk->snd_cwnd > mtp_packets_in_flight(msk)) &&
	     mtp_send_head(sk))
		return false;

	/* Probe timeout is 2*rtt. Add minimum RTO to account
	 * for delayed ack when there's one outstanding packet. If no RTT
	 * sample is available then probe after MTP_TIMEOUT_INIT.
	 */
	if (msk->srtt_us) {
		timeout = usecs_to_jiffies(msk->srtt_us >> 2); /* Multiply by 4 */
		timeout += MTP_ATO_MIN;
	} else {
		timeout = MTP_TIMEOUT_INIT;
	}

	/* If the RTO formula yields an earlier time, then use that time. */
	rto_delta_us = advancing_rto ?
			jiffies_to_usecs(msk->rtx.rto) :
			mtp_rto_delta_us(sk);  /* How far in future is RTO? */
	if (rto_delta_us > 0)
		timeout = min_t(u32, timeout, usecs_to_jiffies(rto_delta_us));

	msk->rtx.pend = MTP_RTX_TIMER_LOSS_PROBE;
	msk->rtx.timeout = jiffies + timeout;
	mtp_debug("%u:%u TLP timeout %u\n", msk->src_port, msk->dst_port, timeout);
	sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
	return true;
}

/* When probe timeout (PTO) fires, try send a new segment if possible, else
 * retransmit the last segment.
 */
void mtp_send_loss_probe(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;
	struct net_device *dev;
	int pcount;
	int err;

	mtp_debug("%u:%u enter\n", msk->src_port, msk->dst_port);
	skb = mtp_send_head(sk);
	if (skb) {
		if (msk->packets_out >= msk->snd_wnd) {
			skb = mtp_write_queue_prev(sk, skb);
		} else {
			pcount = msk->packets_out;
			mtp_write_xmit(sk, MTP_LOSS_PROB_THRES, GFP_ATOMIC);
			if (msk->packets_out > pcount)
				goto probe_sent;
			goto rearm_timer;
		}
	} else {
		skb = mtp_write_queue_tail(sk);
		msk->stats.out_segs_retrans++;
	}

	if (unlikely(!skb)) {
		WARN_ON(msk->packets_out);
		msk->rtx.pend = 0;
		return;
	}

	/* At most one outstanding TLP retransmission. */
	if (msk->tlp_high_seq || skb_fclone_busy(sk, skb))
		goto rearm_timer;

	dev = mtp_odev_get(msk);
	if (unlikely(!dev))
		return;
	skb->dev = dev;
	err = mtp_transmit_skb(sk, skb, 1, GFP_ATOMIC, msk->rcv_nxt);
	dev_put(dev);
	if (err)
		goto rearm_timer;

	MTP_SKB_CB(skb)->sacked |= MTPCB_EVER_RETRANS;
	/* Record snd_nxt for loss detection. */
	msk->tlp_high_seq = msk->snd_nxt;

probe_sent:
	/* Reset s.t. mtp_rearm_rto will restart timer from now */
	msk->rtx.pend = 0;
rearm_timer:
	mtp_rearm_rto(sk);
}

static void mtp_init_nondata_skb(struct sk_buff *skb, u32 seq, u8 type, u8 flags)
{
	struct mtp_skb_cb *mcb = MTP_SKB_CB(skb);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->csum = 0;

	mcb->flags = flags;
	mcb->opt_flags = 0;
	mcb->sacked = 0;
	mcb->seq = seq;
	mcb->type = type;
	if (type == MTPHDR_TYPE_SYN || type == MTPHDR_TYPE_HANDSHAKE || type == MTPHDR_TYPE_FIN)
		seq++;
	mcb->end_seq = seq;
	mcb->need_cache = MTP_NOCACHE;
}

void mtp_send_ack(struct sock *sk, u8 probe)
{
	struct sk_buff *skb;
	struct mtp_sock *msk = mtp_sk(sk);
	struct net_device *dev;
	u8 flag = 0;

	mtp_debug("%u:%u send ack sk_state %u rcv_nxt %u\n", msk->src_port,
		msk->dst_port, sk->sk_state, msk->rcv_nxt);
	if (sk->sk_state == TCP_CLOSE)
		return;

	/* We are not putting this on the write queue, so
	 * mtp_transmit_skb() will set the ownership to this
	 * sock.
	 */
	skb = alloc_skb(MAX_MTP_HEADER, sk_gfp_mask(sk, GFP_ATOMIC | __GFP_NOWARN));
	if (unlikely(!skb)) {
		mtp_err("alloc_skb failed\n");
		mtp_send_delayed_ack(sk);
		return;
	}

	/* Reserve space for headers and prepare control bits. */
	skb_reserve(skb, MAX_MTP_HEADER);
	if (probe)
		flag |= MTPHDR_PROBE;
	mtp_init_nondata_skb(skb, msk->snd_nxt, MTPHDR_TYPE_DEFAULT, flag);

	dev = mtp_odev_get(msk);
	if (unlikely(!dev)) {
		kfree_skb(skb);
		return;
	}
	skb->dev = dev;
	/* Send it off, this clears delayed acks for us. */
	mtp_transmit_skb(sk, skb, 0, (__force gfp_t)0, msk->rcv_nxt);
	dev_put(dev);
}

int mtp_send_syn(struct sock *sk)
{
	struct sk_buff *skb;
	struct mtp_sock *msk = mtp_sk(sk);
	struct net_device *dev;
	int err;

	mtp_debug("%u:%u send syn\n", msk->src_port, msk->dst_port);
	skb = msk_walloc_skb(sk, 0, sk->sk_allocation);
	if (unlikely(!skb))
		return -ENOBUFS;

	msk->write_seq = prandom_u32();
	mtp_debug("%u:%u write_seq %u\n", msk->src_port, msk->dst_port, msk->write_seq);
	msk->snd_una = msk->write_seq;
	msk->tlp_high_seq = msk->write_seq;
	msk->rcv_nxt = 0;
	mtp_init_nondata_skb(skb, msk->write_seq++, MTPHDR_TYPE_SYN, 0);
	msk->snd_nxt = msk->write_seq;
	skb->csum    = 0;
	__skb_header_release(skb);
	__skb_queue_tail(&sk->sk_write_queue, skb);
	sk->sk_wmem_queued += skb->truesize;
	sk_mem_charge(sk, skb->truesize);

	/* Queue it, remembering where we must start sending. */
	if (sk->sk_send_head == NULL)
		sk->sk_send_head = skb;

	mtp_mstamp_refresh(msk);
	dev = mtp_odev_get(msk);
	if (unlikely(!dev))
		return -ENODEV;

	skb->dev = dev;
	/* Send it off, this clears delayed acks for us. */
	err = mtp_transmit_skb(sk, skb, 1, sk->sk_allocation, msk->rcv_nxt);
	dev_put(dev);
	if (unlikely(err))
		return err;
	mtp_event_new_data_sent(sk, skb);
	msk->rtx.pend = MTP_RTX_TIMER_RETRANS;
	msk->rtx.timeout = jiffies + msk->rtx.rto;
	mtp_debug("%u:%u rto %u\n", msk->src_port, msk->dst_port, msk->rtx.rto);
	sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
	return 0;
}

void mtp_send_synack(struct sock *sk, u32 seq, u32 ack_seq)
{
	struct sk_buff *skb;
	struct mtp_sock *msk = mtp_sk(sk);
	struct net_device *dev;
	int clone_it;

	mtp_debug("%u:%u send synack\n", msk->src_port, msk->dst_port);
	if (sock_flag(sk, SOCK_DEAD)) {
		mtp_info("%u:%u SOCK_DEAD\n", msk->src_port, msk->dst_port);
		return;
	}

	if (mtp_state_load(sk) == TCP_SYN_SENT) {
		skb = mtp_write_queue_head(sk);
		clone_it = 1;
	} else {
		/* We are not putting this on the write queue, so
		 * mtp_transmit_skb() will set the ownership to this
		 * sock.
		 */
		skb = alloc_skb(MAX_MTP_HEADER, sk_gfp_mask(sk, GFP_ATOMIC | __GFP_NOWARN));
		if (unlikely(!skb)) {
			mtp_err("alloc_skb failed\n");
			return;
		}

		/* Reserve space for headers and prepare control bits. */
		skb_reserve(skb, MAX_MTP_HEADER);
		clone_it = 0;
	}
	mtp_init_nondata_skb(skb, seq, MTPHDR_TYPE_HANDSHAKE, 0);
	MTP_SKB_CB(skb)->mss = msk->mss;
	MTP_SKB_CB(skb)->max_msg_size = msk->max_msg_size >> MTP_MSG_SHIFT;

	dev = mtp_odev_get(msk);
	if (unlikely(!dev)) {
		if (!clone_it)
			kfree_skb(skb);
		return;
	}
	skb->dev = dev;
	/* Send it off, this clears delayed acks for us. */
	mtp_transmit_skb(sk, skb, clone_it, (__force gfp_t)0, ack_seq);
	dev_put(dev);

	if (clone_it) {
		msk->rtx.pend = MTP_RTX_TIMER_RETRANS;
		msk->rtx.timeout = jiffies + msk->rtx.rto;
		mtp_debug("rto %u timeout %u\n", msk->rtx.rto, msk->rtx.timeout);
		sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
	}
}

void mtp_send_fin(struct sock *sk, gfp_t priority)
{
	struct sk_buff *skb;
	struct sk_buff *tskb = mtp_write_queue_tail(sk);
	struct mtp_sock *msk = mtp_sk(sk);

	if (tskb && mtp_send_head(sk)) {
		MTP_SKB_CB(tskb)->type = MTPHDR_TYPE_FIN;
		MTP_SKB_CB(tskb)->end_seq++;
		msk->write_seq++;
	} else {
		skb = msk_walloc_skb(sk, 0, priority);
		if (unlikely(!skb))
			return;

		mtp_init_nondata_skb(skb, msk->write_seq++, MTPHDR_TYPE_FIN, 0);
		msk->snd_nxt = msk->write_seq;
		skb->csum    = 0;
		__skb_header_release(skb);
		__skb_queue_tail(&sk->sk_write_queue, skb);
		sk->sk_wmem_queued += skb->truesize;
		sk_mem_charge(sk, skb->truesize);

		/* Queue it, remembering where we must start sending. */
		if (sk->sk_send_head == NULL)
			sk->sk_send_head = skb;
	}
	mtp_write_xmit(sk, 0, priority);
}

void mtp_send_reset(struct sock *sk, u32 seq, u32 ack_seq, gfp_t priority)
{
	struct sk_buff *skb;
	struct mtp_sock *msk = mtp_sk(sk);
	struct net_device *dev;

	mtp_debug("%u:%u send reset\n", msk->src_port, msk->dst_port);
	/* We are not putting this on the write queue, so
	 * mtp_transmit_skb() will set the ownership to this
	 * sock.
	 */
	skb = alloc_skb(MAX_MTP_HEADER, priority);
	if (unlikely(!skb)) {
		mtp_err("alloc_skb failed\n");
		return;
	}

	/* Reserve space for headers and prepare control bits. */
	skb_reserve(skb, MAX_MTP_HEADER);
	mtp_init_nondata_skb(skb, seq, MTPHDR_TYPE_RST, 0);

	dev = mtp_odev_get(msk);
	if (unlikely(!dev)) {
		kfree_skb(skb);
		return;
	}
	skb->dev = dev;
	/* Send it off, this clears delayed acks for us. */
	mtp_transmit_skb(sk, skb, 0, priority, ack_seq);
	dev_put(dev);
	MTP_INC_STATS(MTP_MIB_OUTRSTS);
}

static void mtp_set_rst_mtph(struct mtphdr *rst_mtph, struct mtphdr *mtph)
{
	u32 ack_seq;
	(void)memset_s(rst_mtph, sizeof(struct mtphdr), 0, sizeof(struct mtphdr));
	rst_mtph->ver = 1;
	rst_mtph->proto = 0;
	rst_mtph->src = mtph->dst;
	rst_mtph->dst = mtph->src;
	rst_mtph->seq = mtph->ack_seq;
	ack_seq = ntohl(mtph->seq) + ntohs(mtph->len);
	if (mtph->type == MTPHDR_TYPE_SYN || mtph->type == MTPHDR_TYPE_HANDSHAKE ||
	    mtph->type == MTPHDR_TYPE_FIN)
		ack_seq++;
	rst_mtph->ack_seq = htonl(ack_seq);
	rst_mtph->type = MTPHDR_TYPE_RST;
	MTP_HL_SET(rst_mtph, MTP_HLEN_MIN);
	rst_mtph->win = 0;
	rst_mtph->len = 0;
}

void mtp_reply_reset(struct sk_buff *skb)
{
	struct sk_buff *rst_skb;
	struct ethhdr *rst_eth;
	struct mtphdr *rst_mtph;
	struct mtphdr *mtph = (struct mtphdr *)skb_network_header(skb);
	struct ethhdr *eth = eth_hdr(skb);
	struct net_device *dev;
	const struct net_device_ops *ops;

	mtp_debug("reset peer %u:%u\n", mtph->src, mtph->dst);

	if (MTP_SKB_CB(skb)->type == MTPHDR_TYPE_RST)
		return;
	/* We are not putting this on the write queue, so
	 * mtp_transmit_skb() will set the ownership to this
	 * sock.
	 */
	rst_skb = alloc_skb(MAX_MTP_HEADER, GFP_ATOMIC | __GFP_NOWARN);
	if (unlikely(!rst_skb)) {
		mtp_err("alloc_skb failed\n");
		return;
	}

	/* set NULL to skb->head */
	RESET_MTP_FRAME_PTR(rst_skb);
	/* Reserve space for headers and prepare control bits. */
	skb_reserve(rst_skb, MAX_MTP_HEADER);
	skb_push(rst_skb, MTP_HLEN_MIN);
	skb_reset_network_header(rst_skb);

	rst_mtph = (struct mtphdr *)skb_network_header(rst_skb);
	mtp_set_rst_mtph(rst_mtph, mtph);

	// set mac header
	skb_push(rst_skb, ETH_HLEN);
	skb_reset_mac_header(rst_skb);
	rst_eth = eth_hdr(rst_skb);
	(void)memcpy_s(rst_eth->h_source, ETH_ALEN, eth->h_dest, ETH_ALEN);
	(void)memcpy_s(rst_eth->h_dest, ETH_ALEN, eth->h_source, ETH_ALEN);
	rst_eth->h_proto = htons(ETH_P_MTP);

	dev = skb->dev;
	if (dev == NULL) {
		mtp_err("dev is null\n");
		kfree_skb(rst_skb);
		return;
	}

	rst_skb->dev = dev;
	rst_skb->protocol = htons(ETH_P_MTP);

	if (memcmp(rst_eth->h_dest, dev->dev_addr, ETH_ALEN) == 0) {
		(void)mtp_loopback_skb(rst_skb);
	} else {
		ops = dev->netdev_ops;
		local_bh_disable();
		dev_queue_xmit_nit(rst_skb, dev);
		local_bh_enable();
		(void)ops->ndo_start_xmit(rst_skb, dev);
	}
	MTP_INC_STATS(MTP_MIB_OUTRSTS);
}

void mtp_send_delayed_ack(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (msk->delack.enable)
		return;

	msk->delack.timeout = jiffies + sysctl_mtp_ack_timer;
	msk->delack.enable = 1;
	sk_reset_timer(sk, &msk->delack.timer, msk->delack.timeout);
}

void mtp_release_cb(struct sock *sk)
{
	unsigned long flags, nflags;

	/* perform an atomic operation only if at least one flag is set */
	do {
		flags = sk->sk_tsq_flags;
		if (!(flags & MTP_DEFERRED_ALL))
			return;
		nflags = flags & ~MTP_DEFERRED_ALL;
	} while (cmpxchg(&sk->sk_tsq_flags, flags, nflags) != flags);
	mtp_debug("%u:%u flags %x\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, flags);

	if (flags & MTPF_MSQ_DEFERRED)
		mtp_msq_handler(sk);

	/* Here begins the tricky part :
	 * We are called from release_sock() with :
	 * 1) BH disabled
	 * 2) sk_lock.slock spinlock held
	 * 3) socket owned by us (sk->sk_lock.owned == 1)
	 *
	 * But following code is meant to be called from BH handlers,
	 * so we should keep BH disabled, but early release socket ownership
	 */
	sock_release_ownership(sk);

	if (flags & MTPF_WRITE_TIMER_DEFERRED) {
		mtp_write_timer_handler(sk);
		mtp_debug("MTPF_WRITE_TIMER_DEFERRED\n");
		__sock_put(sk);
	}

	if (flags & MTPF_DELACK_TIMER_DEFERRED) {
		mtp_delack_timer_handler(sk);
		mtp_debug("MTP_DELACK_TIMER_DEFERRED\n");
		__sock_put(sk);
	}
}

