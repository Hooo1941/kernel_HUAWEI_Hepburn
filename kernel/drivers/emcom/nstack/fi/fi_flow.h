/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
* Description: This module is flow table for FI
* Author: songqiubin songqiubin@huawei.com
* Create: 2019-09-18
*/

#ifndef _FI_FLOW_H
#define _FI_FLOW_H

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/ktime.h>
#endif
#include <linux/spinlock_types.h>
#include <net/ipv6.h>

#define FI_APP_NAME_SIZ			64

#define FI_FLOW_LEN			88
#define FI_IPV4_OPTIONS_LEN		40
#define FI_EXT_OPTIONS_LEN		256
#define FI_TCP_OPTIONS_LEN		40
#define FI_PAYLOAD_SEG_LEN		350

#define FI_FLOW_TABLE_MASK	0xf
#define FI_FLOW_TABLE_SIZE	0x10
#define FI_FLOW_NODE_LIMIT	512
#define FI_FLOW_AGING_TIME	60000
#define FI_FLOW_PRINT_LOG_PERIOD 5000
#define FI_IFACE_PRINT_LOG_INTERVAL 10

#define FI_NETLINK_BUF_MAX_SIZE 2048
#define FI_NETLINK_REPORT_MAX_NUM 64

#define FI_FLOW_IDENTIFY_MAX		8
#define FI_FLOW_FILTER_STR_LEN		16
#define FI_FLOW_FILTER_STR_MAX		8
#define FI_DPORT_MAX		8
#define FI_PORT_ANY (-1)

#define FI_FLOW_PERIODIC_REPORT_STOP_UP (1 << 0)
#define FI_FLOW_PERIODIC_REPORT_STOP_DOWN (1 << 1)

#define FI_FLOW_PERIODIC_REPORT_STOP (FI_FLOW_PERIODIC_REPORT_STOP_UP | FI_FLOW_PERIODIC_REPORT_STOP_DOWN)

#define FI_FLOW_PKT_REPORT_STOP_UP (1 << 2)
#define FI_FLOW_PKT_REPORT_STOP_DOWN (1 << 3)

#define FI_FLOW_ALL_REPORT_STOP (FI_FLOW_PERIODIC_REPORT_STOP_UP | FI_FLOW_PERIODIC_REPORT_STOP_DOWN | \
	FI_FLOW_PKT_REPORT_STOP_UP | FI_FLOW_PKT_REPORT_STOP_DOWN)

#define FI_FLOW_PKT_REPORT_STOP (FI_FLOW_PKT_REPORT_STOP_UP | FI_FLOW_PKT_REPORT_STOP_DOWN)

#define FI_FLOW_PERIODIC_REPORT_NEED_UP (1 << 4)
#define FI_FLOW_PERIODIC_REPORT_NEED_DOWN (1 << 5)
#define FI_FLOW_PERIODIC_REPORT_NEED (FI_FLOW_PERIODIC_REPORT_NEED_UP | FI_FLOW_PERIODIC_REPORT_NEED_DOWN)

#define FI_FLOW_QOS_REPORT_NEED 1
#define FI_MAX_ALIGN_SIZE ((FI_NETLINK_BUF_MAX_SIZE / sizeof(struct fi_periodic_rpt_msg)) * sizeof(struct fi_periodic_rpt_msg))

enum {
	FLOW_ESTABLISHED = 1,
	FLOW_SYN_SENT,
	FLOW_SYN_RECV,
	FLOW_FINISH,
	FLOW_RESET = 0xff,
};

enum fi_app_status {
	FI_INVALID_APP_STATUS,
	FI_COLLECT_RUN,
	FI_COLLECT_STOP,
	FI_COLLECT_HALT,
	FI_MAX_APP_STATUS,
};

enum fi_parse {
	FI_PAYLOADLEN_PARSE,
	FI_PAYLOAD_SEGMENT_PARSE,
	FI_ECHO_DELAY,
	FI_TCPSEQ_PARSE,
	FI_TCPACK_PARSE,
	FI_TCPFLAGS_PARSE,
	FI_TCPWINDOWSIZE_PARSE,
	FI_TCPOPTIONS_PARSE,
	FI_END_PARSE,
};

enum fi_dir {
	FI_DIR_UP,
	FI_DIR_DOWN,
	FI_DIR_BOTH,
};

typedef enum  {
	FI_FLOW_FAULT_NONE = 0,
	FI_FLOW_FAULT_CLOSE_BY_NW,
	FI_FLOW_FAULT_CLOSE_BY_APP,
} fi_flow_fault_type;

struct fi_flow_stat {
	int up_byte;
	int down_byte;
	int up_pkt_num;
	int up_pkt_byte;
	int down_pkt_num;
	int down_pkt_byte;
	int up_down_diff_time;
	int raw_up_pkt_num;
	int raw_up_pkt_byte;
	int raw_down_pkt_num;
	int raw_down_pkt_byte;
	unsigned char sock_state;
	u32 cwnd_left;
	u32 rtt;
	u32 total_retrans;
	char payload_seg[FI_DIR_BOTH][FI_PAYLOAD_SEG_LEN];
};

union fi_flow_rpt_ctrl {
	int	pkt_num;
	struct {
		unsigned long ts_start;
		unsigned long ts_end;
	};
};

struct fi_flow_msg {
	int32_t			uid;
	__be16			l3proto;
	__u8			l4proto;
	union {
		struct {
			uint32_t	ipv4_sip;
			uint32_t	ipv4_dip;
		};
#if IS_ENABLED(CONFIG_IPV6)
		struct {
			struct in6_addr	ipv6_sip;
			struct in6_addr	ipv6_dip;
		};
#endif
	};
	__be16			sport;
	__be16			dport;
	uint32_t		sk_mark;
	int				dev_if;
	char			dev[IFNAMSIZ];
	char			flow[FI_FLOW_LEN + 1];
	bool			new_flow;
	fi_flow_fault_type				flow_fault_type;
};

struct fi_flow_ctx {
	int up_pkt_num;
	int key_info_check_rst;
	uint32_t stat_stop;
	struct fi_flow_stat flow_stat;
	struct fi_flow_msg flow_msg;
	union fi_flow_rpt_ctrl periodic_ctrl[FI_DIR_BOTH];
	union fi_flow_rpt_ctrl pkt_ctrl[FI_DIR_BOTH];
	unsigned long last_up_timestamp;
	u32 last_total_retrans;
	int tcp_state;
};

/* FI basic feature config from emcomd xml (multipathFlow -> feature -> policy) */
typedef enum {
	FI_PROTO_NULL,
	FI_TCP,
	FI_UDP,
	FI_PROTO_ANY = -1,
} fi_l4_proto;

typedef enum {
	FI_RPT_OFF,
	FI_RPT_ALWAYS,
	FI_RPT_NUM_CTRL,
	FI_RPT_TIME_CTRL,
} fi_rpt_type;

typedef enum {
	FI_RPT_NULL,
	FI_RPT_BLOCK,
	FI_RPT_GO,
} fi_key_info_check_rst;

typedef struct {
	/* region of interest policy */
	fi_rpt_type report_type;
	int roi_start;
	int roi_stop;

	/* filter policy */
	uint32_t filter_pkt_size_start;
	uint32_t filter_pkt_size_stop;
	uint8_t filter_str[FI_FLOW_FILTER_STR_MAX][FI_FLOW_FILTER_STR_LEN];
	uint8_t filter_str_len[FI_FLOW_FILTER_STR_MAX];
	uint32_t filter_str_offset;

	/* feature policy */
	bool				opt_enable[FI_END_PARSE];
	uint32_t			seg_begin;
	uint32_t			seg_end;
}fi_rpt_cfg;

typedef struct tag_fi_flow_identify_cfg {
	int			proto;
	int			dport[FI_DPORT_MAX];
	bool basic_rpt_en;
	fi_rpt_cfg periodic[FI_DIR_BOTH];
	fi_rpt_cfg pkt[FI_DIR_BOTH];
}fi_flow_identify_cfg;

struct fi_app_update_msg {
	int32_t				uid;
	char					app_name[FI_APP_NAME_SIZ + 1];
	enum fi_app_status		status;
	uint8_t service; // add for mproute 0:mpflow 1:mproute
	bool	qos_rpt_enable;
	fi_flow_identify_cfg 		flow_identify_cfg[FI_FLOW_IDENTIFY_MAX];
};

struct fi_app_info_node {
	struct fi_app_update_msg	msg;
	unsigned long		rand_num;
	spinlock_t			lock;
};

/**
 * struct fi_mstamp - multi resolution time stamps
 * @stamp_us: timestamp in us resolution
 * @stamp_jiffies: timestamp in jiffies
 */
struct fi_mstamp {
	u32	stamp_us;
	u32	stamp_jiffies;
};

struct fi_flow_bw {
	struct fi_mstamp first_rcv_time;
	struct fi_mstamp last_rcv_time;
	uint32_t burst_bytes;
	uint32_t total_bytes;
	uint32_t total_time;
};

struct fi_flow_qos {
	uint32_t rtt; /* us */
	uint32_t bw_est; /* bytes per second */
	uint32_t rcv_bytes;
};

struct fi_file_size {
	uint32_t size;
	uint64_t last_rcv_bytes;
};

typedef struct {
	struct fi_app_info_node *app_info;
	fi_flow_identify_cfg *flow_identify_cfg;
}fi_flow_cb;

struct fi_flow_node {
	struct list_head		list;
	union {
		struct {
			uint32_t		ipv4_saddr;
			uint32_t		ipv4_daddr;
		};
#if IS_ENABLED(CONFIG_IPV6)
		struct {
			struct in6_addr	ipv6_saddr;
			struct in6_addr	ipv6_daddr;
		};
#endif
	};
	__be16			sport;
	__be16			dport;
	uint32_t		updatetime;
	uint32_t		ip_flow_print_log_time;
	uint32_t		flow_qos_print_log_time;
	int32_t			uid;
	struct fi_flow_ctx flow_ctx;
	fi_flow_cb			flow_cb;
	struct sock *sk;
	struct fi_flow_qos	qos;
	struct fi_flow_bw	rcv_bw;
	struct fi_file_size file_size;
};

struct fi_flow_head {
	struct list_head		list;
};

struct fi_flow {
	struct fi_flow_head	flow_table[FI_FLOW_TABLE_SIZE];
	spinlock_t		flow_lock[FI_FLOW_TABLE_SIZE];
	atomic_t		node_num;
};

struct fi_timeval {
	long tv_sec;
	long tv_usec;
};

struct fi_pkt_msg {
	struct fi_flow_msg	flow_msg;
	struct {
		struct fi_timeval	tstamp;
		enum fi_dir 	dir;
		__be16			total_len;
		__be16			payload_len;
		__be16			report_len;
		uint32_t		tcp_seq;
		uint32_t		tcp_ack;
		unsigned char	tcp_flags;
		__be16			tcp_window;
		int 			up_down_diff_time;
		char			tcp_options[FI_TCP_OPTIONS_LEN];
		char			payload_seg[0];
	};
};

struct fi_periodic_rpt_msg {
	struct fi_flow_stat flow_stat;
	struct fi_flow_msg flow_msg;
};

struct fi_qos_rpt_msg {
	struct fi_flow_msg flow_msg;
	struct fi_flow_qos qos;
	uint32_t size;
};

struct fi_pkt_parse {
	unsigned long rand_num;
	struct fi_pkt_msg msg;
	fi_flow_cb flow_cb;
	enum fi_dir dir;
	bool is_multi_frag;
	uint16_t raw_frag_num;
	uint16_t filter_frag_num;
	uint32_t parse_len;
	__be16 frag_payload_len;
	__be16 filter_frag_payload_len;
	char *payload;
	char *frag_payload;
	char *filter_frag_payload;
	bool is_malloc_buf;
};

struct fi_pkt_parse_f {
	enum fi_parse	opt_num;
	bool			(*opt_val_parse)(struct sk_buff *, struct fi_pkt_parse *);
};

struct report_local_var {
	int ret;
	int size;
	int total_left;
	int offset;
};

extern uint32_t fi_ipv4_flow_node_num(void);

extern void fi_ipv4_flow_lock(uint32_t index);

extern void fi_ipv4_flow_unlock(uint32_t index);

extern uint32_t fi_ipv4_flow_hash(struct fi_pkt_parse *pktinfo, int str_len, bool exchange);

extern struct fi_flow_head *fi_ipv4_flow_header(uint32_t index);

extern struct fi_flow_node *fi_ipv4_flow_find(const struct fi_flow_head *head,
			__be32 saddr, __u16 sport, __be32 daddr, __be16 dport);

extern struct fi_flow_node *fi_ipv4_flow_add(struct fi_flow_head *head, int32_t uid,
			struct fi_pkt_parse *pktinfo, struct sock *sk);

extern struct fi_flow_node *fi_ipv4_flow_get(struct fi_flow_head *head, int32_t uid,
			struct fi_pkt_parse *pktinfo, bool addflow, struct sock *sk);

extern void fi_ipv4_flow_age(void);

extern void fi_ipv4_flow_clear_node(struct fi_flow_node *node);

extern void fi_ipv4_flow_clear(void);

extern void fi_ipv4_flow_clear_by_uid(int32_t uid);

extern void fi_ipv4_flow_init(void);

#if IS_ENABLED(CONFIG_IPV6)
extern uint32_t fi_ipv6_flow_node_num(void);

extern void fi_ipv6_flow_lock(uint32_t index);

extern void fi_ipv6_flow_unlock(uint32_t index);

extern uint32_t fi_ipv6_flow_hash(struct fi_pkt_parse *pktinfo, int str_len, bool exchange);

extern struct fi_flow_head *fi_ipv6_flow_header(uint32_t index);

extern struct fi_flow_node *fi_ipv6_flow_find(const struct fi_flow_head *head,
			const struct in6_addr *saddr, __u16 sport,
			const struct in6_addr *daddr, __be16 dport);

extern struct fi_flow_node *fi_ipv6_flow_add(struct fi_flow_head *head, int32_t uid,
	struct fi_pkt_parse *pktinfo, struct sock *sk);

extern struct fi_flow_node *fi_ipv6_flow_get(struct fi_flow_head *head, int32_t uid,
	struct fi_pkt_parse *pktinfo, bool addflow, struct sock *sk);

extern void fi_ipv6_flow_age(void);

extern void fi_ipv6_flow_clear_node(struct fi_flow_node *node);

extern void fi_ipv6_flow_clear(void);

extern void fi_ipv6_flow_clear_by_uid(int32_t uid);

extern void fi_ipv6_flow_init(void);
#endif /* CONFIG_IPV6 */

void fi_flow_statistics(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo,
	enum fi_dir dir);

extern bool fi_payload_len_filter_check(__be16 payloadlen,
	uint32_t filter_size_start, uint32_t filter_size_stop);

extern bool fi_payload_str_filter_check(struct fi_pkt_parse *pktinfo, fi_rpt_cfg *rpt_cfg);

bool fi_pkt_report_check(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo, enum fi_dir dir);

void fi_flow_stat_update_report(void);

void fi_flow_qos_update_report(void);

static inline bool fi_flow_qos_rpt_enable(struct fi_flow_node *flow)
{
	return flow->flow_cb.app_info->msg.qos_rpt_enable;
}
#endif /* _FI_FLOW_H */