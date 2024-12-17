/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: liyouyong songqiubin
 *
 * mtp layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#ifndef __MINTP_H__
#define __MINTP_H__

#define pr_fmt(fmt) "[nstack_mtp]%s[%d] : " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/inetdevice.h>
#include <linux/printk.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/signal.h>

#include <net/net_namespace.h>
#include <net/inet_hashtables.h>
#include <net/protocol.h>
#include <net/addrconf.h>
#include <net/inet_common.h>
#include <net/inet_sock.h>
#include <net/arp.h>
#include <asm/unaligned.h>
#include <securec.h>

#define SOL_MTP 300
#ifndef IPPROTO_MINTP
#define IPPROTO_MINTP 200
#endif
#define MTP_TOS 1
#define MTP_KEEPIDLE 2
#define MTP_MAX_MSG_SIZE 3
#define MTP_D2D 4
#define MTP_STATS 5
#define MTP_CPU_ID 6
#define MTP_QDISC 7
#define MTP_CONG_CONFIG 8
#define MTP_SENDPAGE_MSG_SIZE 9

#define MTP_PORT_MAX 255
#define ETH_P_MTP 0xA85A
#define MTP_SEG_BURST 44

#define MTP_MIN_MSS 512
#define MTP_MSG_SHIFT 10

#define MTP_INIT_PACING_RATE (80 * 1000 * 1000)
#define MTP_PACING_SHIFT 10
#define MTP_SND_BUF_SIZE (8 * 1000 * 1000)
#define MTP_RWIN_THRESH_SHIF 2
#define ALIGIN_SIZE_4 4

#define MTP_CACHE 1
#define MTP_NOCACHE 0
#define MTP_BATCH_SENDING_MAX 24

#define MACFMT "%02x:%02x:%02x:%02x:**:**"
#define MACDATA(mac) mac[0], mac[1], mac[2], mac[3]

extern int sysctl_mtp_debug;

#ifdef MTP_DEBUG_FTRACE
#define mtp_printk(fmt, lvl, ...) trace_printk("[nstack_mtp]"fmt, ##__VA_ARGS__)
#else
#define mtp_printk(fmt, lvl, ...) printk(lvl pr_fmt(fmt), ##__VA_ARGS__)
#endif

/* mtp_debug also use KERN_INFO for kenrel is not print level KERN_DEBUG */
#define mtp_debug(fmt, ...)	\
	do {	\
		if (unlikely(sysctl_mtp_debug >= LOGLEVEL_DEBUG))	\
			mtp_printk(fmt, KERN_INFO, ##__VA_ARGS__); \
	} while (0)

#define mtp_info(fmt, ...)	\
	do {	\
		if (likely(sysctl_mtp_debug >= LOGLEVEL_INFO))		\
			mtp_printk(fmt, KERN_INFO, ##__VA_ARGS__); \
	} while (0)

#define mtp_warn(fmt, ...)	\
	do {	\
		if (likely(sysctl_mtp_debug >= LOGLEVEL_WARNING))	\
			mtp_printk(fmt, KERN_WARNING, ##__VA_ARGS__); \
	} while (0)

#define mtp_err(fmt, ...)	\
	do {	\
		if (likely(sysctl_mtp_debug >= LOGLEVEL_ERR))		\
			mtp_printk(fmt, KERN_ERR, ##__VA_ARGS__); \
	} while (0)

#define MAX_MTP_HEADER	(64 + MAX_HEADER)

#define MTPHDR_TYPE_DEFAULT 0
#define MTPHDR_TYPE_SYN 1
#define MTPHDR_TYPE_HANDSHAKE 2
#define MTPHDR_TYPE_FIN 3
#define MTPHDR_TYPE_RST 4
#define MTPHDR_TYPE_MSG_END 5
#define MTPHDR_TYPE_MAX MTPHDR_TYPE_MSG_END

struct mtphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	hdl : 4,
		proto : 2,
		ver : 2;
	__u8	rsv : 3,
		probe : 1,
		rtx : 1,
		type : 3;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	ver : 2,
		proto : 2,
		hdl : 4;
	__u8	type : 3,
		rtx : 1,
		probe : 1,
		rsv : 3;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif

	__u8    src;
	__u8    dst;
	__be32  seq;
	__be32  ack_seq;
	__be16  len;
	__be16  win;
};


#define MTPHDR_RTX 0x10
#define MTPHDR_PROBE 0x08

#define MTP_HLEN_MIN	sizeof(struct mtphdr)
#define MTP_HL_GET(hdr) ((__u8)((hdr)->hdl << 2))
#define MTP_HL_SET(hdr, size) ((hdr)->hdl = ((size) >> 2))

#define MTPCB_SACKED_ACKED	0x01 /* SKB ACK'd by a SACK block */
#define MTPCB_SACKED_RETRANS	0x02 /* SKB retransmitted */
#define MTPCB_LOST		0x04 /* SKB is lost */
#define MTPCB_TAGBITS		0x07 /* All tag bits */
#define MTPCB_REPAIRED		0x10 /* SKB repaired (no skb_mstamp) */
#define MTPCB_EVER_RETRANS	0x80 /* Ever retransmitted frame */
#define MTPCB_RETRANS		(MTPCB_SACKED_RETRANS | MTPCB_EVER_RETRANS | \
				MTPCB_REPAIRED)
struct mtp_skb_cb {
	__u32		seq; /* Starting sequence number */
	__u32		end_seq; /* SEQ + FIN + SYN + datalen */
	__u32		ack_seq; /* Sequence number ACK'd */
	__u16		mss; /* mss */
	__u16		max_msg_size; /* msg size in the handshake */
	__u16		win; /* window size in packets */
	__u8		type; /* MTP header type. */
	__u8		flags; /* MTP header flags. */
	__u16		opt_flags; /* MTP header flags in MTPOPT_FLAGS option */
	__u8		hdlen; /* MTP header length. */
	__u8		sacked; /* State flags for SACK/FACK. */
	__u8		need_cache; /* cache packets for hi1106 batch sending */
};

#define MTP_SKB_CB(__skb) ((struct mtp_skb_cb *)&((__skb)->cb[0]))
#define MTP_FLAG_BYTE(mtph) ((((u_int8_t *)(mtph))[1]) & 0x1F)

struct mac_addr {
	unsigned char addr[ETH_ALEN];
	unsigned char pad[6]; /* the inet framework need size of addr >= 16 */
};

struct ip_addr {
	__be32	addr;
	unsigned char pad[8]; /* the inet framework need size of addr >= 16 */
};

#define MTP_ADDR_TYPE_MAC 0
#define MTP_ADDR_TYPE_IPV4 1
struct sockaddr_mtp {
	sa_family_t sa_family;
	u8 port;
	u8 type;
	union {
		struct mac_addr mac;
		struct ip_addr ip;
	};
};

#define MTPOPT_NOP		1 /* Padding */
#define MTPOPT_MSS		2 /* Segment size negotiating */
#define MTPOPT_FLAGS            3 /* Extended flags */
#define MTPOPT_MSG_SIZE         4 /* Msg size negotiating, the unit is 1K(1024bytes) */
#define MTPOPT_SACK             5 /* SACK Block */

#define MTPOLEN_SACK_BASE		2
#define MTPOLEN_SACK_BASE_ALIGNED	4
#define MTPOLEN_SACK_PERBLOCK		8
#define MTPOLEN_MSS			4
#define MTPOLEN_MSG_SIZE		4
#define MTPOLEN_FLAGS			4
#define MTPOPT_FLAG_REC		0x01

#define MTPOLEN_MAX (60 - MTP_HLEN_MIN)
#define MTP_NUM_SACKS 4
/* After receiving this amount of duplicate ACKs fast retransmit starts. */
#define MTP_FASTRETRANS_THRESH 3

struct mtp_sack_block_wire {
	__be32	start_seq;
	__be32	end_seq;
};

struct mtp_sack_block {
	u32	start_seq;
	u32	end_seq;
};

struct mtp_sock_stat {
	u32 out_segs;
	u32 out_msgs;
	u32 out_segs_unfulfilled;
	u32 out_segs_retrans;
	u32 out_app_limit;
	u32 out_rwnd_limit;
	u32 out_cwnd_limit;
	u32 out_dev_limit;
	u32 out_msq_limit;
	u32 out_dev_error;
	u32 in_acks;
	u32 cwnd_reduction;
	u32 cwnd_undo_reduction;
	u32 cwnd;
	u32 srtt;
};

struct mtp_tls_context;

#define MTP_CONG_AVOID_RATIO_MAX 16
struct mtp_cong_config {
	u32 init_cwnd; /* valid if positive */
	u32 cong_avoid_ratio; /* valid if positive */
};

struct mtp_sock {
	/* inet_connection_sock has to be the first member of mtp_sock */
	struct inet_sock inet;

	struct list_head msq_node;
	struct list_head delack_node;
	struct list_head net_down_node;

	/* udp_recvmsg try to use this before splicing sk_receive_queue */
	struct sk_buff_head    reader_queue;

	struct sk_buff *lost_skb_hint;
	struct sk_buff *retransmit_skb_hint;

	/* tx queue buffered for hi1106 */
	struct sk_buff *tx_tmp_list;
	struct sk_buff *tx_tmp_tail; /* the last node of the list */
	u32 tx_tmp_list_len;

	/* OOO segments go in this rbtree. Socket lock must be held. */
	struct rb_root	out_of_order_queue;
	struct sk_buff *ooo_last_skb; /* cache rb_last(out_of_order_queue) */
	int (*xmit)(struct sk_buff *skb);
	int (*segment)(struct sock *sk, struct msghdr *msg, long timeo,
			      int *seg, bool *process_backlog);
	int (*recv_queue)(struct sock *sk, struct msghdr *msg, size_t len, u16 *copied_skb);
	union {
		/* for mtp TCP_LISTEN sock */
		struct {
			spinlock_t	lock;
			struct list_head accept;
		} accept_queue;
		/* for mtp TCP_NEW_SYN_RECV sock */
		struct list_head req_node;
	};
	unsigned char src_mac[ETH_ALEN];
	unsigned char dst_mac[ETH_ALEN];
	__u8 src_port;
	__u8 dst_port;

	__u8	ca_recovery : 1, /* recv a valid retransmit skb */
		ca_loss : 1, /* detect a loss in recv skb MTPHDR_REC flag */
		ca_cwnd_limited : 1, /* forward progress limited by snd_cwnd? */
		ca_state : 5;
	__u8	tos;
	u16	snd_wnd; /* The remote perr's window size in pkts */
	u16	rcv_wnd; /* Current receiver window in pkts */
	u16	rcv_wnd_scale; /* the scale from sk_rcvbuf to rcv_wnd */
	u16	rcv_wnd_free; /* receiver window freed in pkts since last ack sent */

	u32	rcv_nxt;
	u32	copied_seq; /* Head of yet unread data */
	u32	snd_nxt;
	u32	snd_una;
	u32	write_seq;

	u32	snd_ssthresh;
	u32	snd_cwnd;
	u32	snd_cwnd_cnt;
	u32	packets_out;
	u32	retrans_out;
	u32	max_packets_out;
	u32	max_packets_seq;

	u32	tlp_high_seq;

	u64	cur_mstamp; /* timestamp in us */
	u64	slow_start_mstamp; /* timestamp of when slow start */
	u64	probe_rsp_mstamp; /* timestamp of when send a response to a probe */
	u32	compress_probe; /* segment sent since last probe request */
	u32	srtt_us;
	u32	mdev_us;
	u32	mdev_max_us;
	u32	rttvar_us;
	u32	rtt_seq;
	struct  minmax rtt_min;

	u32	keepalive_time;
	u8	keepalive_probes;
	s8	cpu_id;
	struct work_struct worker; /* used to wake up flood_qdisc on the Specified CPU */
	u32	rcv_tstamp;

	struct {
		struct timer_list timer;
		__u8	ack_pend : 1,
			quick : 1,
			enable : 1,
			res : 5;
		__u8              compress_ack;
		__u8		  ofo_ack; /* out-of-order packets received since rcv_nxt update */
		unsigned long	  timeout; /* Currently scheduled timeout */
		u32		  ofo_rcv_nxt;   /* the rcv_nxt when ofo_ack send */
	} delack;

	struct {
		struct timer_list timer;
		__u8	pend : 7,
			frto : 1;
		__u8 	retransmits;
		__u8 	backoff;
		__u8	probes;
		__u32	rto;
		unsigned long	timeout;
	} rtx;

	/* Information of the most recently (s)acked skb */
	struct mtp_rack {
		u64 mstamp;
		u32 rtt_us;
		u32 end_seq;
		u8 advanced;
		u8 reord;
	} rack;

	u32	lost_out; /* Lost packets */
	u32	sacked_out; /* SACK'd packets */
	u32	fackets_out; /* FACK'd packets */
	u32	prior_ssthresh; /* ssthresh saved at recovery start */
	u32	prior_cwnd; /* cwnd right before starting loss recovery */
	u32	prr_delivered; /* Number of newly delivered packets to
				 * receiver in Recovery. */
	u32	prr_out; /* Total number of pkts sent during Recovery. */
	u32	delivered; /* Total data packets delivered incl. rexmits */
	u32	high_seq; /* snd_nxt at onset of congestion */
	u32	undo_marker; /* snd_una upon a new recovery episode. */

	u8	num_sacks; /* Number of selective SACK blocks */
	u16	reordering; /* Packet reordering metric. */
	struct mtp_sack_block selective_acks[MTP_NUM_SACKS]; /* The SACKS themselves */
	struct mtp_sack_block recv_sack_cache[MTP_NUM_SACKS];
	struct sk_buff *highest_sack; /* skb just after the highest
				       * skb with SACKed bit set
				       * (validity guaranteed only if
				       * sacked_out > 0)
				       */
	int	lost_cnt_hint;
	int	tx_queue; /* qdisc tx queue index for mintp */
	u32	max_msg_size; /* mx msg size for input and output */
	u32	cur_rcv_msg_size; /* the current recv msg size in the reader_queue */
	u32	cur_snd_msg_size; /* the current send msg size in the write queue */
	u32	sendpage_msg_size; /* the msg size of mtp_sendpage, should smaller than max_msg_size/2 */
	u16	mss;
	bool	d2d;
	bool	is_loopback;
	bool	send_pend;
	bool	tls_recv_pend;
	bool	is_hi1106;
	void    *dev_node;
	struct mtp_tls_context *tls_ctx;
	struct net_device __rcu	*odev; /* output dev */
	struct mtp_sock_stat stats;
	struct mtp_cong_config cong_cfg;
};

static inline void mtp_state_store(struct sock *sk, int newstate)
{
	smp_store_release(&sk->sk_state, newstate);
}

static inline int mtp_state_load(const struct sock *sk)
{
	/* state change might impact lockless readers. */
	return smp_load_acquire(&sk->sk_state);
}

static inline struct mtp_sock *mtp_sk(const struct sock *sk)
{
	return (struct mtp_sock *)sk;
}

static inline struct mtphdr *mtp_hdr(const struct sk_buff *skb)
{
	return (struct mtphdr *)skb_transport_header(skb);
}

static inline struct sk_buff *mtp_write_queue_head(const struct sock *sk)
{
	return skb_peek(&sk->sk_write_queue);
}

static inline struct sk_buff *mtp_write_queue_tail(const struct sock *sk)
{
	return skb_peek_tail(&sk->sk_write_queue);
}

static inline struct sk_buff *mtp_write_queue_next(const struct sock *sk,
						   const struct sk_buff *skb)
{
	return skb_queue_next(&sk->sk_write_queue, skb);
}

static inline struct sk_buff *mtp_write_queue_prev(const struct sock *sk,
						   const struct sk_buff *skb)
{
	return skb_queue_prev(&sk->sk_write_queue, skb);
}

static inline void mtp_unlink_write_queue(struct sk_buff *skb, struct sock *sk)
{
	__skb_unlink(skb, &sk->sk_write_queue);
}

static inline void mtp_check_send_head(struct sock *sk, struct sk_buff *skb_unlinked)
{
	if (sk->sk_send_head == skb_unlinked)
		sk->sk_send_head = NULL;
}

#define mtp_for_write_queue(skb, sk)					\
	skb_queue_walk(&(sk)->sk_write_queue, skb)

#define mtp_for_write_queue_from(skb, sk)				\
	skb_queue_walk_from(&(sk)->sk_write_queue, skb)

#define mtp_for_write_queue_from_safe(skb, tmp, sk)			\
	skb_queue_walk_from_safe(&(sk)->sk_write_queue, skb, tmp)

static inline struct sk_buff *mtp_send_head(const struct sock *sk)
{
	return sk->sk_send_head;
}

static inline bool mtp_skb_is_last(const struct sock *sk,
				   const struct sk_buff *skb)
{
	return skb_queue_is_last(&sk->sk_write_queue, skb);
}

static inline bool mtp_skb_can_collapse_to(const struct sk_buff *skb)
{
	return likely(MTP_SKB_CB(skb)->type != MTPHDR_TYPE_MSG_END);
}

/*
 * The next routines deal with comparing 32 bit unsigned ints
 * and worry about wraparound (automatic with unsigned arithmetic).
 */

static inline bool before(__u32 seq1, __u32 seq2)
{
	return (__s32)(seq1 - seq2) < 0;
}
#define after(seq2, seq1) 	before(seq1, seq2)

/* is s2<=s1<=s3 ? */
static inline bool between(__u32 seq1, __u32 seq2, __u32 seq3)
{
	return seq3 - seq2 >= seq1 - seq2;
}

/* MTP uses 32bit jiffies to save some space. */
#define MTP_JIFFIERS32 ((u32)jiffies)

#define MTP_DEFAULT_KEEPALIVE_TIME (30 * 60 * HZ) /* 30 minutes */
#define MTP_KEEPALIVE_INTVL	((unsigned)(HZ / 2))
#define MAX_MTP_KEEPIDLE	32767
#define MTP_RTO_MAX	((unsigned)(20 * HZ))
#define MTP_RTO_MIN	((unsigned)(HZ / 5))

static inline u64 mtp_clock_ns(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	return ktime_get_ns();
#else
	return local_clock();
#endif
}

static inline u64 mtp_clock_us(void)
{
	return div_u64(mtp_clock_ns(), NSEC_PER_USEC);
}

/* provide the departure time in us unit */
static inline u64 mtp_skb_timestamp_us(const struct sk_buff *skb)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	return div_u64(skb->skb_mstamp_ns, NSEC_PER_USEC);
#else
	return skb->skb_mstamp;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
typedef char __user * sockptr_t;
#define copy_from_sockptr copy_from_user

static inline void mtp_skb_mstamp_refresh(struct mtp_sock *msk, struct sk_buff *skb)
{
	skb->skb_mstamp = msk->cur_mstamp;
}

static inline void mtp_wmem_free_skb(struct sock *sk, struct sk_buff *skb)
{
	sk_wmem_free_skb(sk, skb);
}
#else
static inline void mtp_skb_mstamp_refresh(struct mtp_sock *msk, struct sk_buff *skb)
{
	skb->skb_mstamp_ns = msk->cur_mstamp * NSEC_PER_USEC;
}

static inline void mtp_wmem_free_skb(struct sock *sk, struct sk_buff *skb)
{
	sk_wmem_queued_add(sk, -skb->truesize);
	sk_mem_uncharge(sk, skb->truesize);
	__kfree_skb(skb);
}
#endif

/* Refresh 1us clock of a TCP socket,
 * ensuring monotically increasing values.
 */
static inline void mtp_mstamp_refresh(struct mtp_sock *msk)
{
	u64 val = mtp_clock_us();
	if (val > msk->cur_mstamp)
		msk->cur_mstamp = val;
}

static inline u32 mtp_stamp_us_delta(u64 t1, u64 t0)
{
	return max_t(s64, t1 - t0, 0);
}

static inline u16 mtp_full_win(const struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	return (sk->sk_rcvbuf >> msk->rcv_wnd_scale);
}

static inline u16 mtp_block_win(const struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	return msk->reader_queue.qlen + sk->sk_receive_queue.qlen;
}

static inline bool mtp_send_need_sched(struct mtp_sock *msk)
{
	return (msk->cpu_id != -1) && (msk->cpu_id != smp_processor_id());
}

static inline void mtp_write_queue_purge(struct sock *sk)
{
	struct sk_buff *skb;
	while ((skb = __skb_dequeue(&sk->sk_write_queue)) != NULL)
		mtp_wmem_free_skb(sk, skb);
}

struct sk_buff *msk_walloc_skb(struct sock *sk, int size, gfp_t gfp);

#define MTP_DEFAULT_MSG_SIZE (1024000)

bool mtp_mac_any(unsigned char *mac, int len);

void mtp_done(struct sock *sk);
void mtp_set_state(struct sock *sk, int state);
void mtp_init_sock_base(struct sock *sk);
void mtp_skb_entail(struct sock *sk, struct sk_buff *skb);
extern int g_mtp_loss;
extern int sysctl_mtp_ack_timer;
extern int mtp_limit_output_bytes;
extern int sysctl_mtp_queue;

#endif /* __MINTP_H__ */
