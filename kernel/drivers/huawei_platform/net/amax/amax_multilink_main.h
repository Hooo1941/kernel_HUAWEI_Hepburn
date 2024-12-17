/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This module is to support wifi multilink feature.
 * Create: 2022-09-02
 */
#ifndef AMAX_MULTILINK_MAIN_H_
#define AMAX_MULTILINK_MAIN_H_

#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/if.h>
#include <linux/inet.h>
#include <linux/namei.h>
#include <linux/ip.h>
#include <linux/ieee80211.h>
#include <linux/ipv6.h>
#include <net/cfg80211.h>

#define MAGIC_AMAX_LEN 4
#define AMAX_RESET_PKT_NUM 5
#define AMAX_VALID_RESET_NUM 10
#define IP_FRAG_OFFSET_IS_ZERO 0x1fff
#define ETH_IS_MULTICAST(_a) (*(_a) & 0x01)
#define IS_IPV4_NS_TOS(tos) ((((tos) & 0xF0) == 0xF0) ? true : false)

#define MAX_LLC_SIZE 65536
#define MAX_WINDOW_SIZE 8192                  // 窗口最大长度
#define MAX_WINDOW_MASK (MAX_WINDOW_SIZE - 1) // 窗口最大长度
#define MAX_ITEM_NUM (MAX_WINDOW_SIZE * 2)    // 缓存数组最大个数
#define MAX_ITEM_MASK (MAX_ITEM_NUM - 1)      // 对16384求余的掩码，对应接收缓存数组的大小

#define AMAX_NAPI_WEIGHT 64
#define AMAX_FORCE_FN_5 1
#define AMAX_FORCE_FN_2 2
#define AMAX_FORCE_FN_DUAL 3

#define ETHER_TYPE_IP 0x0800
#define ETHER_TYPE_VLAN 0x8100
#define ETHER_TYPE_AMAX 0x9999
#define MAC_TCP_PROTOCAL 6
#define MAC_UDP_PROTOCAL 17
#define IPV4_ADDR_SIZE 4
#define ETHER_ADDR_LEN 6
#define MAC_HILINK_SRC_PORT 37443
#define MAC_DHCP_DEST_PORT1 67
#define MAC_DHCP_DEST_PORT2 68

#define SUCC 0
#define FAIL 1
#define FAILED (-1)

#define VLAN_ID 4094 // 可配置的VLAN ID取值范围为1～4094，0和4095协议保留。此处设置成4094来标识是LLC over IP封装后的帧
#define VLAN_ID_OVER_VLAN 4093
#define VLAN_ID_TCP 4091 // tcp通道单独一个vlan

#define INITIAL_LLC_ID 0xffff
#define CFI 0 // 经典格式指示符，设置为0

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define TIMEOUT_LIMIT 10

// 定时器调度周期，单位：ns
#define NETWORK_QOS_TIMER_PERIOD 10000000 // 1s: 1000000000 10000000000  100ms: 100000000 10ms: 10000000 1ms: 1000000
#define NETWORK_QOS_MIN_ACCURACY 1000000
#define NETWORK_QOS_TIMER_RATIO (NETWORK_QOS_TIMER_PERIOD / NETWORK_QOS_MIN_ACCURACY)
#define TICK_TO_TIME(x) ((x) * NETWORK_QOS_TIMER_RATIO)

/* proc系统维测打印 */
#define MAX_THREAD_NUM 8
#define MAX_FLUSH_REASON 3
#define MAX_GRO_FAIL_NUM 5
#define MAX_GRO_RET_NUM 6
#define MAX_DBG_NUM 10
#define CHECK_BYTE_CNT 4
#define MAX_CORE_CNT 8
#define MAX_EMPTY_HOLE_CNT 10
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3]
#define MACSTR "%02x:%02x:%02x:%02x:XX:XX"
#define LLC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3]
#define HOLE2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7], (a)[8], (a)[9]
#define CORE2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7]
#define SUB2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7], (a)[8], (a)[9]
#define GRO2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define FAIL2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4]
#define FLUSH2STR(a) (a)[0], (a)[1], (a)[2]

#define WLAN_IP_PRI_SHIFT 5 /* 获取ip头中tos字段 */

#define DEBUG_UDP_DST_PORT 7777
#define DEBUG_TCP_DST_PORT 6666
#define INITIAL_LLC_ID 0xffff

#define CONTIME_THRESHOLD_5G_INIT 20
#define CONLEN_THRESHOLD_5G_INIT 700
#define CONTIME_THRESHOLD_2G_INIT 20
#define CONLEN_THRESHOLD_2G_INIT 600

#define NETBUF_PROTOCOL(_netbuf) ((_netbuf)->protocol)

/* 报文类别枚举 */
typedef enum {
	PACKET_INVALID = 0, // 非业务报文
	PACKET_INBOUND = 1, // 接收的业务报文
	PACKET_OUTBOUND = 2, // 发送的业务报文
	PACKET_DEVICE_PIPE_SYNERGY = 3 // 端管协同管理类数据报文
} PACKET_TYPE;

/* 频段号 */
typedef enum {
	FN_2 = 0,
	FN_5 = 1,
	FN_NUM = 2,
	FN_DOUBLE = 3,
	FN_ADAPT,
	FN_MAIN,
	FN_INVALID
} FREQUENCY_NUMBER;

/* 主辅路枚举 */
typedef enum {
	ML_SLAVE = 0,
	ML_MASTER = 1,
	ML_ROLE_NUM = 2
} ML_ROLE_TYPE;

typedef enum {
	SINGLE = 0,     // 单频
	SINGLE_2G4 = 1, // 双频聚合
	DOUBLE_RED = 2, // 双频冗余
	SELF_ADAPT = 3  // 自适应
} WORK_MODE;

/* 接收状态 */
typedef enum {
	NOT_RECEIVED = 0,
	RECEIVED
} RECV_STATUS;

/* TID编号类别 */
typedef enum {
	WLAN_TIDNO_BEST_EFFORT = 0,            /* BE业务 */
	WLAN_TIDNO_BACKGROUND = 1,             /* BK业务 */
	WLAN_TIDNO_NETWORK_SLICING = 2,        /* 网络切片 */
	WLAN_TIDNO_ANT_TRAINING_LOW_PRIO = 3,  /* 智能天线低优先级训练帧 */
	WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO = 4, /* 智能天线高优先级训练帧 */
	WLAN_TIDNO_VIDEO = 5,                  /* VI业务 */
	WLAN_TIDNO_VOICE = 6,                  /* VO业务 */
	WLAN_TIDNO_BCAST = 7,                  /* 广播用户的广播或者组播报文 */
	WLAN_TIDNO_BUTT
} WLAN_TIDNO_ENUM;

typedef enum {
	TID_NS_ARR_INDEX = 0,
	TID_VO_ARR_INDEX = 1,
	TID_VI_ARR_INDEX = 2,
	TID_BE_ARR_INDEX = 3,
	TID_ARR_NUM = 4
} TID_ARR_ENUM;
/* sta相关信息 */
typedef struct {
	uint16_t llc_id[TID_ARR_NUM];
	uint8_t init_count[TID_ARR_NUM];
	uint8_t init_mask;
}amax_sta_pair;

/* LLC header */
typedef struct {
	uint16_t llc_id;        // 报文序号
	uint8_t tid : 3;        // 报文tid
	uint8_t fn : 1;         // 频段
	uint8_t reset : 1;      // 窗口重置标志位，为0时不适用后续8位
	uint8_t group : 2;      // 分核分组
	uint8_t is_redundant_mode : 1; // 是否冗余模式发送
	union {
		uint8_t test_resv : 1; // 保留
		uint8_t flag_resv : 7; // 保留
		uint8_t mask;          // 窗口重置标记，随机生成，用于标记同一次重置请求
	} flag;
} llc_header;

/* 发送缓存数组元素结构 */
typedef struct send_frame {
	struct sk_buff *netbuf; // 报文的指针
	uint16_t llc_id;        // 报文的LLC序号
	uint8_t tid;            // 子通道号：0——AC_BE 1——AC_BK 2——AC_VI 3——AC_VO
	uint8_t fn;             // 频段号： 0——2.4GHz; 1——5GHz；
} send_frame;

/* 业务信息 */
typedef struct _business_info {
	struct {
		uint8_t priority : 3;
		uint8_t event_flag : 1; // true代表增加，false代表删除
		uint8_t mode : 2;
		uint8_t reserved : 2;
	};
	uint8_t src_address[4]; // IPV4地址
	uint8_t dst_address[4]; // IPV4地址
	uint8_t protocol;
	uint16_t src_port;
	uint16_t dst_port;
} business_info;

// /* 接收缓存数组元素结构 */
typedef struct _recv_frame {
	struct sk_buff *netbuf; // 报文的指针
	uint64_t recv_time;     // 记录报文在缓存数组中滞留的时间，用于等待超时的判断
	uint16_t llc_id;        // 报文的LLC序号
	uint16_t llc_id_16;     // 16位LLC序号
	uint8_t tid;            // 报文发送的tid队列
	uint8_t fn;             // 报文的发送频段
	uint8_t recv_status;    // 接收状态： 0——NOT_RECEIVED; 1——RECEIVED_AND_FORWARD; 2——RECEIVED_AND_CACHED
	uint8_t is_redundant;   // 标记报文是否双发
} recv_frame;

/* 接收缓存数组句柄 流信息结构体, 包括五元组, 包数据，流统计信息 */
typedef struct _cache_array_recv_handle {
	recv_frame frame_array[MAX_ITEM_NUM]; // 缓存数组元素
	uint8_t reset_flag;
	uint8_t timeout_flag;
	uint8_t self_cure_flag;
	uint8_t is_vmalloc;
	uint64_t wait_time;

	uint16_t rx_lowedge;   // 接收窗口下沿
	uint16_t rx_upedge;    // 接收窗口上沿
	uint16_t real_lowedge; // 真实的下沿

	// 统计量
	uint16_t curr_llc_id_5g;
	uint16_t curr_llc_id_2g;
	uint16_t cache_num;
	uint16_t max_cache_num;
	uint64_t udp_oow_submit_num;
	uint64_t out_of_window_frame_num;
	uint64_t total_oow_frame_num;
	uint64_t true_duplicate_num;
	uint64_t false_duplicate_num;
	uint64_t drop_pkts_num;
	uint16_t empty_hole[MAX_EMPTY_HOLE_CNT];
	uint16_t empty_hole_16[MAX_EMPTY_HOLE_CNT];
} cache_array_recv_handle;

int32_t amax_tx_handle_hook(struct sk_buff *netbuf, struct net_device **net_dev);
int32_t amax_rx_handle_hook(struct sk_buff *netbuf);
int32_t amax_tx_complete_handle_hook(const struct sk_buff *netbuf);
cache_array_recv_handle *get_recv_array_hdl_all(uint8_t tid);

uint32_t amax_wifi_freq_get_by_name(int8_t *name, uint32_t name_len);
int amax_stack_fn(struct sk_buff **nbuf_list, uint32_t *num_list_elements, uint8_t vdev_id);

int get_gro_enable_switch(void);
int get_stat_switch(void);
amax_sta_pair *get_sta_pair(void);
uint8_t *get_magic_amax_sign(void);

PACKET_TYPE amax_inbound_frame_classify(struct sk_buff *pst_buf);

struct napi_struct *amax_napi_process(struct sk_buff *skb);

int amax_ignore_sinfo_reducepower(struct net_device *dev, struct station_info *sinfo);
void amax_store_station_info(struct net_device *dev, struct station_info *sinfo);

int amax_ioctl_reset_tx_status(uint8_t enable_amax);
int amax_ioctl_set_threshold(uint8_t throughput, uint8_t latency, uint8_t packetnumber);

void amax_module_init(void);
void amax_module_exit(void);
#endif
