/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This module is to support wifi multilink feature.
 * Create: 2022-09-02
 */
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/if_vlan.h>
#include <linux/device.h>
#include <securec.h>
#include <linux/etherdevice.h>
#include "amax_multilink_tool.h"
#include "amax_multilink_stat.h"
#include "amax_multilink_main.h"
#include "amax_multilink_dev.h"
#include <huawei_platform/net/amax/amax_multilink_interface.h>
#include "dps_data_txrx.h"

#define BYTE_SHIFT 16

#define CLASSIFY_MODE_TCP_UDP 0
#define CLASSIFY_MODE_UDP 1
#define CLASSIFY_MODE_TCP 2
#define CLASSIFY_MODE_UDP_PORT 3
#define CLASSIFY_MODE_TCP_PORT 4

#define CLASSIFY_MARK 0x6a
#define AMAX_TCI_SHIFT 13
#define AMAX_CFI_LEN 12
#define ETH_MAC_TWICE 2
#define MAX_INTERVAL 600
#define GAME_FOREGROUND 2
#define NON_GAME_FOREGROUND 1
#define GAME_FOREGROUND_WAIT_TIME 30
#define NON_GAME_FOREGROUND_WAIT_TIME 200
#define THROUGHPUT_SAMPLE_NUM 10
#define AMAX_TIMER_STATS_PERIOD 10
#define AMAX_TIMER_PRINT_PERIOD 6000  // 1min打印一次tx rx信息
#define CONTIME_STAT 5 // contime divide by 32
#define MAX_THRESHOLD 65536
#define BYTE_TRANSFER 17
#define MAX_LATENCY_10S 10000 // 10s
#define MIN_5G_FREQUENCY 5000
#define MAX_5G_FREQUENCY 5999

#define CHECKSUM_COMPLETE 2
#define SELF_CURE_OOW_NUM_MAX 20

#define THROUGHPUT_DEFAULT 10
#define LATENCY_DEFAULT 50
#define PACKETNUMBER_DEFAULT 100

/* rx钩子 HOOKED说明转由amax线程提交协议栈 NOT_PROCESS说明直接走后续流程 */
#define HOOKED 0
#define NOT_PROCESS 1

#define BAND_2G_MIN_CHANNEL            1
#define BAND_2G_MAX_CHANNEL            14

#define IP_SEG_OFFSET            8

#ifdef CONFIG_CONNECTIVITY_HI1105
/* 通过vap结构体获取信道信息 */
typedef struct {
	uint8_t uc_chan_number;               /* 主20MHz信道号 */
	uint8_t en_band : 4,
			ext6g_band : 1,               /* 指示是否为6G频段标志 */
			resv : 3;
	uint8_t en_bandwidth;                 /* 带宽模式 */
	uint8_t uc_chan_idx;                  /* 信道索引号 */
} amax_mac_channel_stru;

typedef struct {
	uint8_t uc_vap_id;                    /* vap ID   */
	uint8_t uc_device_id;                 /* 设备ID   */
	uint8_t uc_chip_id;                   /* chip ID  */
	uint8_t en_vap_mode;                  /* vap模式  */
	uint32_t core_id;
	uint8_t auc_bssid[ETHER_ADDR_LEN];    /* BSSID，非，是mib中的auc_dot11StationID  */
	uint8_t en_vap_state;                 /* VAP状态 */
	uint8_t en_protocol;                  /* 工作的协议模式 */
	amax_mac_channel_stru st_channel;     /* vap所在的信道 */
} amax_mac_vap_stru;
#endif

static amax_sta_pair *g_sta_pair = NULL;
static const int8_t* g_dev_name_main = "wlan0";
static const int8_t* g_dev_name_assist = "wlan1";
static uint8_t g_magic_amax_sign[MAGIC_AMAX_LEN] = { 0x99, 0xab, 0xcd, 0x99 };
static uint8_t g_magic_tuples_sign[MAGIC_AMAX_LEN] = { 0x88, 0xab, 0xcd, 0x88 };
static struct hrtimer g_amax_timer;
static struct completion g_amax_rx_completion;
static struct task_struct *g_amax_rx_task = NULL;
static struct sk_buff_head g_skb_queue_array;
static uint8_t g_is_dev_info_get = false;
static uint8_t g_is_game_foreground = false;
static uint8_t g_dev_mac_addr_assist[ETHER_ADDR_LEN] = {0};
static uint8_t g_dev_mac_addr_main[ETHER_ADDR_LEN] = {0};
static uint8_t g_fn_main;
static uint8_t g_fn_assist;
static struct net_device *g_netdev_assist = NULL;
static struct net_device *g_netdev_main = NULL;
static cache_array_recv_handle *g_recv_array[TID_ARR_NUM] = {NULL};
static bool g_init_flag = false;
static bool g_amax_rx_exit_flag = false;
static bool g_amax_tx_timer_trigger_flag = false;
static bool g_amax_rx_timer_trigger_flag = false;
static bool g_amax_rx_data_trigger_flag = false;
static uint64_t g_amax_timetick;
static uint8_t g_window_reset_mask[TID_ARR_NUM];
static uint8_t g_amax_timer_switch = 0;
static bool g_pkt_proc_flag = false;
static uint8_t g_amax_throughput_index; // amax吞吐量数组下标
static uint8_t g_contime_index[FN_NUM]; // amax拥塞时延数组下标

/* tid映射表 */
static const uint8_t g_tid_arr_table[WLAN_TIDNO_BUTT] = {
	TID_BE_ARR_INDEX, TID_ARR_NUM, TID_NS_ARR_INDEX, TID_ARR_NUM,
	TID_ARR_NUM, TID_VI_ARR_INDEX, TID_VO_ARR_INDEX, TID_ARR_NUM
};
/* 多核锁 */
static spinlock_t g_tx_spinlock;
/* NAPI相关结构体 */
static struct napi_struct g_amax_napi;
static struct net_device amax_dummy_dev;
static struct station_info last_sinfo = { 0 };

/* variable parameter for debug */
int g_max_window_wait_time = 20;
int g_max_submit_wait_time = 200;
int g_submit_timeout = 1;
int g_wait_timeout = 0;
int g_enable_amax_uplink = 0;
int g_classify_len = 500;
int g_classify_mode = 0;
int g_udp_oow_submit = 0;
int g_duration_threshold = 100;
int g_stat_switch = 0;
int g_debug_window_switch = 1;
int g_gro_enable_switch = 1;
int g_amax_gro_enable_switch = 0;
int g_timer_stop_time = 3000;
int g_error_netbuf = 0;
int g_throughput_threshold = 10;
int g_latency_threshold = 50;
int g_pps_threshold = 200;
int g_send_mode_param = 0;
int g_same_segment_pkg_filter = 1;

/* variable parameter for debug */
module_param(g_max_window_wait_time, int, 0664);
module_param(g_max_submit_wait_time, int, 0664);
module_param(g_submit_timeout, int, 0664);
module_param(g_wait_timeout, int, 0664);
module_param(g_enable_amax_uplink, int, 0664);
module_param(g_classify_len, int, 0664);
module_param(g_classify_mode, int, 0664);
module_param(g_udp_oow_submit, int, 0664);
module_param(g_duration_threshold, int, 0664);
module_param(g_stat_switch, int, 0664);
module_param(g_debug_window_switch, int, 0664);
module_param(g_gro_enable_switch, int, 0664);
module_param(g_amax_gro_enable_switch, int, 0664);
module_param(g_timer_stop_time, int, 0664);
module_param(g_error_netbuf, int, 0664);
module_param(g_throughput_threshold, int, 0664);
module_param(g_latency_threshold, int, 0664);
module_param(g_pps_threshold, int, 0664);
module_param(g_send_mode_param, int, 0664);
module_param(g_same_segment_pkg_filter, int, 0664);

/* Fuction declaration */
static int32_t amax_init(void);
static void amax_exit(void);
static int32_t amax_rx_thread(void *arg);
static enum hrtimer_restart amax_timer_handler(struct hrtimer *timer);
static void amax_reset_single_recv_array(uint8_t tid);
static void amax_oow_process(struct sk_buff *skb, cache_array_recv_handle *recv_array);
static void amax_get_cur_llc_id(recv_frame *info);
static uint8_t amax_check_dup_pkt(struct sk_buff *new_nb, struct sk_buff *old_nb);
static void amax_debug_stat_window_delay(struct sk_buff *netbuf);
static int32_t amax_get_dev_info(void);
static int32_t amax_tx_frame_encapsulation(send_frame *info);
static void amax_set_llc_id(send_frame *info, uint8_t tid);
static void amax_tx_frame_dispatch(struct sk_buff *netbuf, send_frame *info, uint8_t tid);
static int32_t cache_array_recv_init(void);
static void cache_array_recv_exit(void);
static void amax_trigger_data_flag(bool cache_flag);
static void amax_rx_time_process(uint8_t tid);
static void amax_move_recv_windows(uint8_t tid);
static void amax_rx_frame_process(recv_frame *info, cache_array_recv_handle *recv_array_hdl);
static int32_t cache_recv_init(cache_array_recv_handle **recv_ahdl);
static int32_t amax_rx_frame_reassemble(struct sk_buff *net_buf, recv_frame *info);
static void amax_move_empty_recv_window(recv_frame *info, cache_array_recv_handle *recv_array_hdl);
static struct net_device *get_net_dev(uint8_t vdev_id);
static int32_t amax_outbound_frame_classify_core(struct sk_buff *netbuf);
static int32_t amax_encap_new_header(send_frame *info);
static int32_t amax_tx_frame_send(send_frame *info, struct net_device **net_dev);
static void amax_tx_send_copy_nb(struct sk_buff *netbuf);
static void amax_set_mac_addr(struct sk_buff *netbuf, uint8_t fn);
static uint32_t amax_tx_frame_send_fn_spec(struct sk_buff *netbuff, uint8_t fn, struct net_device **net_dev);
static PACKET_TYPE amax_outbound_frame_classify(struct sk_buff *netbuf, uint8_t *tid);
static void update_dev_info_get_flag(void);

cache_array_recv_handle *get_recv_array_hdl_all(uint8_t tid)
{
	return g_recv_array[tid];
}

int get_gro_enable_switch(void)
{
	return g_gro_enable_switch;
}

int get_stat_switch(void)
{
	return g_stat_switch;
}

amax_sta_pair *get_sta_pair(void)
{
	return g_sta_pair;
}

uint8_t *get_magic_amax_sign(void)
{
	return g_magic_amax_sign;
}

static int amax_napi_poll(struct napi_struct *napi, int budget)
{
	amax_print(PRINT_ERROR, "This napi_poll should not be polled as we don't schedule it");

	return 0;
}

static void amax_napi_init(void)
{
	init_dummy_netdev(&amax_dummy_dev);
	netif_napi_add(&amax_dummy_dev, &g_amax_napi, amax_napi_poll, AMAX_NAPI_WEIGHT);
	napi_enable(&g_amax_napi);
}

static void amax_napi_deinit(void)
{
	napi_disable(&g_amax_napi);
	netif_napi_del(&g_amax_napi);
}

struct napi_struct *amax_napi_process(struct sk_buff *skb)
{
	uint8_t add_len;

	if (unlikely(skb == NULL)) {
		amax_print(PRINT_ERROR, "skb is null");
		return NULL;
	}

	add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
	skb_trim(skb, skb->len - add_len);
	debug_inc_napi_cnt();
	return &g_amax_napi;
}

static inline void set_mac_addr(uint8_t *puc_mac_addr1, uint32_t len1, const uint8_t *puc_mac_addr2, uint32_t len2)
{
	if (len1 >= ETHER_ADDR_LEN && len2 >= ETHER_ADDR_LEN) {
		puc_mac_addr1[0] = puc_mac_addr2[0]; /* 设置MAC地址0位 */
		puc_mac_addr1[1] = puc_mac_addr2[1]; /* 设置MAC地址1位 */
		puc_mac_addr1[2] = puc_mac_addr2[2]; /* 设置MAC地址2位 */
		puc_mac_addr1[3] = puc_mac_addr2[3]; /* 设置MAC地址3位 */
		puc_mac_addr1[4] = puc_mac_addr2[4]; /* 设置MAC地址4位 */
		puc_mac_addr1[5] = puc_mac_addr2[5]; /* 设置MAC地址5位 */
	}
}

/* 30s无下行报文，将定时器关闭 */
static void amax_timer_stat_proc(uint32_t *timer_cnt)
{
	if (*timer_cnt >= g_timer_stop_time) {
		*timer_cnt = 0;
		if (g_pkt_proc_flag == false)
			g_amax_timer_switch = 0;
		g_pkt_proc_flag = false;
	}
}

static void amax_flag_init(void)
{
	g_amax_rx_exit_flag = false;
	g_amax_tx_timer_trigger_flag = false;
	g_amax_rx_timer_trigger_flag = false;
	g_amax_rx_data_trigger_flag = false;
	return;
}

static void amax_timer_init(void)
{
	ktime_t amax_kt;

	amax_kt = ktime_set(0, NETWORK_QOS_TIMER_PERIOD);
	hrtimer_init(&g_amax_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	g_amax_timer.function = amax_timer_handler;
	hrtimer_start(&g_amax_timer, amax_kt, HRTIMER_MODE_REL);
}

static void amax_timer_exit(void)
{
	hrtimer_cancel(&g_amax_timer);
}

static int32_t amax_sta_pair_init(void)
{
	g_sta_pair = (amax_sta_pair *)k_memalloc(sizeof(amax_sta_pair));
	if (g_sta_pair == NULL) {
		amax_print(PRINT_ERROR, "sta_pair init failed!\n");
		return FAIL;
	}
	memset_s(g_sta_pair, sizeof(amax_sta_pair), 0, sizeof(amax_sta_pair));
	return SUCC;
}

static void amax_sta_pair_exit(void)
{
	if (g_sta_pair != NULL) {
		kfree(g_sta_pair);
		g_sta_pair = NULL;
	}
}

static int32_t amax_init(void)
{
	/* 初始化标志位 */
	amax_flag_init();
	/* 初始化sta_pair */
	if (amax_sta_pair_init() != SUCC) {
		amax_print(PRINT_ERROR, "sta_pair initialize failed!\n");
		return FAIL;
	}
	/* 初始化统计量 */
	if (amax_stat_init() != SUCC) {
		amax_print(PRINT_ERROR, "amax stat initialize failed!\n");
		goto stat_error;
	}
	/* 初始化接收缓存数组 */
	if (cache_array_recv_init() != SUCC) {
		amax_print(PRINT_ERROR, "recv_array initialize failed!\n");
		goto cache_error;
	}
	/* 初始化发送打点统计 */
	if (amax_tx_dmd_stat_init() != SUCC) {
		amax_print(PRINT_ERROR, "amax tx dmd initialize failed!\n");
		goto dmd_stat_error;
	}
	/* 初始化异常打点统计 */
	if (amax_abnormal_stat_init() != SUCC) {
		amax_print(PRINT_ERROR, "amax abnormal stat initialize failed!\n");
		return FAIL;
	}
	amax_napi_init();
	/* 初始化skb_queue */
	skb_queue_head_init(&g_skb_queue_array);

	init_completion(&g_amax_rx_completion);
	spin_lock_init(&g_tx_spinlock);

	g_amax_rx_task = kthread_run(amax_rx_thread, NULL, "amax_rx_thread");
	if (g_amax_rx_task == NULL) {
		amax_print(PRINT_ERROR, "rx thread initialize failed!\n");
		goto thread_error;
	}
	/* 初始化高精度定时器 */
	amax_timer_init();

	amax_ioctl_set_threshold(THROUGHPUT_DEFAULT, LATENCY_DEFAULT, PACKETNUMBER_DEFAULT);
	/* 在初始化的时候就获取5G和2.4G的dev,避免开机首次连接时端管失效 */
	update_dev_info_get_flag();
	return SUCC;
thread_error:
	skb_queue_purge(&g_skb_queue_array);
	amax_napi_deinit();
	amax_tx_dmd_stat_exit();
dmd_stat_error:
	cache_array_recv_exit();
cache_error:
	amax_stat_exit();
stat_error:
	amax_sta_pair_exit();
	return FAIL;
}

static int32_t amax_try_init(void)
{
	if (amax_init() == SUCC) {
		g_init_flag = true;
		return SUCC;
	}
	amax_print(PRINT_ERROR, "init failed\n");
	return FAIL;
}

static void amax_exit(void)
{
	// 更新标志位
	g_amax_rx_exit_flag = true;

	// 唤醒线程
	complete(&g_amax_rx_completion);

	if (g_amax_rx_task != NULL) {
		kthread_stop(g_amax_rx_task);
		g_amax_rx_task = NULL;
	}

	amax_timer_exit();

	if (spin_is_locked(&g_tx_spinlock))
		spin_unlock_bh(&g_tx_spinlock);

	// 清除skb_queue array
	skb_queue_purge(&g_skb_queue_array);

	amax_napi_deinit();

	// 清除异常统计结构
	amax_abnormal_stat_exit();

	// 清除上行统计数组
	amax_tx_dmd_stat_exit();

	// 清除缓存数组
	cache_array_recv_exit();

	// 清除统计量
	amax_stat_exit();

	amax_sta_pair_exit();

	g_init_flag = false;
}

static void amax_trigger_data_flag(bool cache_flag)
{
	if (cache_flag) {
		g_amax_rx_data_trigger_flag = true;
		complete(&g_amax_rx_completion);
	}
}

static int32_t cache_recv_init(cache_array_recv_handle **recv_hdl)
{
	uint16_t i;
	uint8_t is_vmalloc = 0;
	*recv_hdl = (cache_array_recv_handle *)k_memalloc(sizeof(cache_array_recv_handle));
	if (unlikely(*recv_hdl == NULL)) {
		amax_print(PRINT_ERROR, "kalloc fail\n");
		*recv_hdl = (cache_array_recv_handle *)vmalloc(sizeof(cache_array_recv_handle));
		if (unlikely(*recv_hdl == NULL)) {
			amax_print(PRINT_ERROR, "vmalloc fail\n");
			return FAIL;
		}
		is_vmalloc = 1; // 1 means Memory allocated by vmalloc and later free by vfree
	}

	amax_print(PRINT_INFO, "cache_recv_init success %d\n", is_vmalloc);
	memset_s(*recv_hdl, sizeof(cache_array_recv_handle), 0, sizeof(cache_array_recv_handle));
	(*recv_hdl)->is_vmalloc = is_vmalloc;
	(*recv_hdl)->rx_lowedge = 0;
	(*recv_hdl)->real_lowedge = 0;
	(*recv_hdl)->rx_upedge = MAX_ITEM_NUM - 1;

	for (i = 0; i < MAX_ITEM_NUM; i++) {
		(*recv_hdl)->frame_array[i].llc_id = 0xffff;
		(*recv_hdl)->frame_array[i].recv_status = NOT_RECEIVED;
	}
	if (g_debug_window_switch)
		(*recv_hdl)->frame_array[(*recv_hdl)->rx_upedge].llc_id_16 = (1 << BYTE_SHIFT) - 1;

	return SUCC;
}

static int32_t cache_array_recv_init(void)
{
	uint8_t i;

	for (i = 0; i < TID_ARR_NUM; i++) {
		if (cache_recv_init(&g_recv_array[i]) != SUCC) {
			cache_array_recv_exit();
			return FAIL;
		}
	}

	return SUCC;
}

static void cache_array_recv_exit(void)
{
	uint8_t i;
	for (i = 0; i < TID_ARR_NUM; i++) {
		if (g_recv_array[i] == NULL) {
			continue;
		}

		if (g_recv_array[i]->is_vmalloc)
			vfree(g_recv_array[i]);
		else
			kfree(g_recv_array[i]);

		g_recv_array[i] = NULL;
	}
}

static void amax_timer_start(struct hrtimer *timer)
{
	ktime_t amax_kt;

	if (!g_amax_timer_switch) {
		g_amax_timer_switch = 1;
		amax_kt = ktime_set(0, NETWORK_QOS_TIMER_PERIOD);
		hrtimer_forward_now(timer, amax_kt);
		hrtimer_start(timer, amax_kt, HRTIMER_MODE_REL);
	}

	return;
}

static enum hrtimer_restart amax_timer_handler(struct hrtimer *timer)
{
	ktime_t amax_kt;

	g_amax_timetick++;
	g_amax_rx_timer_trigger_flag = true;
	g_amax_tx_timer_trigger_flag = true;
	complete(&g_amax_rx_completion);

	if (g_amax_rx_exit_flag == false) {
		if (g_amax_timer_switch) {
			amax_kt = ktime_set(0, NETWORK_QOS_TIMER_PERIOD);
			hrtimer_forward_now(timer, amax_kt);
			return HRTIMER_RESTART;
		} else {
			return HRTIMER_NORESTART;
		}
	} else {
		return HRTIMER_NORESTART;
	}
}

static uint32_t amax_valid_llc_header(llc_header *llc_hdr)
{
	/* tid非法，则直接退出 */
	if (llc_hdr->tid >= TID_ARR_NUM)
		return FAIL;

	/* group字段暂时没有使用, 默认为0 */
	if (llc_hdr->group != 0)
		return FAIL;

	/* 若不是reset包，则mask默认为0 */
	if (llc_hdr->reset == 0 && llc_hdr->flag.mask != 0)
		return FAIL;

	/* 若是reset包, 则包的llc_id不超过10 */
	if (llc_hdr->reset != 0 && llc_hdr->llc_id >= AMAX_VALID_RESET_NUM)
		return FAIL;
	return SUCC;
}

static uint32_t amax_valid_llc_header_reorder(struct sk_buff *skb)
{
	uint8_t *ptr = NULL;
	uint8_t *skb_tail = NULL;
	llc_header *llc_hdr = NULL;

	skb_tail = amax_get_skb_tail(skb);
	ptr = (uint8_t *)(skb_tail - MAGIC_AMAX_LEN);

	/* llc header非法，则直接退出 */
	llc_hdr = (llc_header *)(ptr - sizeof(llc_header));
	if (amax_valid_llc_header(llc_hdr) != SUCC)
		return FAIL;

	return SUCC;
}

PACKET_TYPE amax_inbound_frame_classify(struct sk_buff *pst_buf)
{
	uint8_t *ptr = NULL;
	uint8_t *skb_tail = NULL;

	if (unlikely(pst_buf == NULL)) {
		amax_print(PRINT_ERROR, "Error:pst_buf:%p\n", pst_buf);
		return PACKET_INVALID;
	}

	skb_tail = amax_get_skb_tail(pst_buf);
	ptr = (uint8_t *)(skb_tail - MAGIC_AMAX_LEN);
	/* 尾部不满足MAGIC封装格式, 则直接退出 */
	if (memcmp(ptr, g_magic_amax_sign, MAGIC_AMAX_LEN) != 0)
		return PACKET_INVALID;

	if (check_pkt_is_dps_data(pst_buf) == SUCC)
		return PACKET_DEVICE_PIPE_SYNERGY;

	if (amax_valid_llc_header_reorder(pst_buf) == SUCC)
		return PACKET_INBOUND;

	return PACKET_INVALID;
}

static void amax_reset_single_recv_array(uint8_t tid)
{
	uint16_t i;
	cache_array_recv_handle *recv_array = NULL;
	uint8_t is_vmalloc = 0;
	amax_print(PRINT_INFO, "amax_reset_single_recv_array");
	recv_array = g_recv_array[tid];
	is_vmalloc = recv_array->is_vmalloc; // backup mem alloc type
	memset_s(recv_array, sizeof(cache_array_recv_handle), 0, sizeof(cache_array_recv_handle));
	recv_array->is_vmalloc = is_vmalloc; // restore
	recv_array->rx_lowedge = 0;
	recv_array->real_lowedge = 0;
	recv_array->rx_upedge = MAX_ITEM_NUM - 1;
	recv_array->frame_array[recv_array->rx_upedge].llc_id_16 = (1 << BYTE_SHIFT) - 1;

	for (i = 0; i < MAX_ITEM_NUM; i++) {
		recv_array->frame_array[i].llc_id = 0xffff;
		recv_array->frame_array[i].recv_status = NOT_RECEIVED;
	}
	amax_reset_single_win_stat(tid);
}

static void amax_set_self_cure_flag(void)
{
	int32_t tid;
	for (tid = 0; tid < TID_ARR_NUM; tid++)
		g_recv_array[tid]->self_cure_flag = 1;
}

static void amax_self_cure_process(recv_frame *info, cache_array_recv_handle *recv_array_hdl)
{
	// 若聚合发包模式下，窗口外的丢包达到一定数量，则启动自愈模式
	if (recv_array_hdl->total_oow_frame_num < SELF_CURE_OOW_NUM_MAX &&
		recv_array_hdl->self_cure_flag == 0)
		return;

	if (recv_array_hdl->rx_lowedge == ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) {
		if (recv_array_hdl->self_cure_flag != 0) {
			/* 将当前窗口置为当前包的序号 */
			recv_array_hdl->real_lowedge = info->llc_id_16;
			recv_array_hdl->rx_lowedge = info->llc_id;
			recv_array_hdl->rx_upedge = ((info->llc_id + MAX_ITEM_NUM - 1) & MAX_ITEM_MASK);
			recv_array_hdl->frame_array[recv_array_hdl->rx_upedge].llc_id_16 = info->llc_id_16 - 1;

			recv_array_hdl->total_oow_frame_num = 0;
			recv_array_hdl->self_cure_flag = 0;
			window_debug_inc_self_cure_cnt(info->tid);
			amax_print(PRINT_INFO, "force move tid:%hhu window due to self_cure.\n", info->tid);
			return;
		}

		/* 设置各窗口自愈标志位 */
		amax_set_self_cure_flag();

		amax_abnormal_inc_self_cure_cnt();
		amax_print(PRINT_INFO, "force self_cure process due to tid:%hhu toow pks more than %u.\n",
			info->tid, SELF_CURE_OOW_NUM_MAX);
	}
}

static void amax_normal_move_windows(void)
{
	int32_t tid;
	for (tid = 0; tid < TID_ARR_NUM; tid++) {
		if (g_recv_array[tid]->rx_lowedge == ((g_recv_array[tid]->rx_upedge + 1) & MAX_ITEM_MASK)) {
			continue;
		}
		debug_inc_normal_move();
		window_debug_inc_normal_move(tid);
		amax_move_recv_windows(tid);
	}
}

static void amax_rx_pkt_process(void)
{
	int32_t ret = 0;
	recv_frame info = { 0 };
	struct sk_buff *net_buf = NULL;
	cache_array_recv_handle *recv_array_hdl = NULL;

	while (!skb_queue_empty(&g_skb_queue_array)) {
		net_buf = skb_dequeue(&g_skb_queue_array);
		if (likely(net_buf != NULL)) {
			// 有包到来，刷新标志位
			g_pkt_proc_flag = true;
			net_buf->dev = g_netdev_main;
			// 报文解封装
			memset_s(&info, sizeof(recv_frame), 0, sizeof(recv_frame));
			ret = amax_rx_frame_reassemble(net_buf, &info);
			if (ret != SUCC) {
				amax_print(PRINT_ERROR, "reassemble failed!\n");
				dev_kfree_skb_any(net_buf);
				net_buf = NULL;
				continue;
			}

			recv_array_hdl = g_recv_array[info.tid];
			if (recv_array_hdl == NULL) {
				dev_kfree_skb_any(net_buf);
				net_buf = NULL;
				continue;
			}

			amax_move_empty_recv_window(&info, recv_array_hdl);

			// 自愈处理
			amax_self_cure_process(&info, recv_array_hdl);

			// 报文缓存或丢弃
			amax_rx_frame_process(&info, recv_array_hdl);
		} else {
			amax_print(PRINT_ERROR, "skb_dequeue failed\n");
			break;
		}
	}

	amax_normal_move_windows();
}

static void amax_update_throughput_stats(void)
{
	uint8_t i;
	uint8_t index_5g = 0;
	uint8_t index_2g = 0;
	uint32_t throughput_5g_sum = 0;
	uint32_t throughput_2g_sum = 0;

	for (i = 0; i < THROUGHPUT_SAMPLE_NUM; i++) {
		index_5g = (amax_get_tx_5g_throughput(i) >
			amax_get_tx_5g_max_throughput()) ? i : index_5g;
		index_2g = (amax_get_tx_2g_throughput(i) >
			amax_get_tx_5g_max_throughput()) ? i : index_2g;
		amax_set_tx_5g_max_throughput(amax_get_tx_5g_throughput(index_5g));
		amax_set_tx_2g_max_throughput(amax_get_tx_2g_throughput(index_2g));
		throughput_5g_sum += amax_get_tx_5g_throughput(i);
		throughput_2g_sum += amax_get_tx_2g_throughput(i);
	}

	g_amax_throughput_index = (g_amax_throughput_index + 1) % THROUGHPUT_SAMPLE_NUM;
	amax_set_tx_5g_avg_throughput(throughput_5g_sum / THROUGHPUT_SAMPLE_NUM);
	amax_set_tx_2g_avg_throughput(throughput_2g_sum / THROUGHPUT_SAMPLE_NUM);
	amax_set_tx_5g_throughput(g_amax_throughput_index, 0);
	amax_set_tx_2g_throughput(g_amax_throughput_index, 0);

	return;
}

/* 每1min打印tx rx的收发包统计信息, 并清空统计 */
static void amax_print_tx_rx_info(void)
{
	amax_print_tx_stat_info();

	amax_print_rx_stat_info();

	amax_reset_tx_rx_stat_info();
}

static int32_t amax_rx_thread(void *arg)
{
	int32_t ret = 0;
	int32_t tid = 0;
	uint32_t amax_timer_counter = 0;
	uint32_t timer_counter = 0;

	while (!kthread_should_stop()) {
		if (g_amax_rx_exit_flag) {
			continue;
		}

		ret = wait_for_completion_interruptible(&g_amax_rx_completion);
		if (ret < 0) {
			amax_print(PRINT_ERROR, "g_amax_rx_completion terminated ret = %d\n", ret);
			break;
		}

		if (g_amax_rx_timer_trigger_flag == true) {
			amax_timer_counter++;
			timer_counter++;
			amax_timer_stat_proc(&amax_timer_counter);
			if (timer_counter % AMAX_TIMER_STATS_PERIOD == 0) {
				amax_update_throughput_stats();
				amax_update_tx_pps();
			}
			if (timer_counter % AMAX_TIMER_PRINT_PERIOD == 0)
				amax_print_tx_rx_info();
			for (tid = 0; tid < TID_ARR_NUM; tid++)
				amax_rx_time_process(tid);
		}

		if (g_amax_rx_data_trigger_flag == true || g_amax_rx_timer_trigger_flag == true) {
			g_amax_rx_data_trigger_flag = false;
			g_amax_rx_timer_trigger_flag = false;
			amax_rx_pkt_process();
		}
	}
	amax_print(PRINT_INFO, "rx_thread exit succ.\n");
	return SUCC;
}

static int32_t amax_rx_frame_reassemble(struct sk_buff *net_buf, recv_frame *info)
{
	uint8_t *skb_tail = NULL;
	uint16_t add_len = 0;
	llc_header *llc_hdr_ptr = NULL;

	if (unlikely(net_buf == NULL)) {
		amax_print(PRINT_ERROR, "net_buf is null! Return!\n");
		return FAIL;
	}

	add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
	skb_tail = amax_get_skb_tail(net_buf);

	llc_hdr_ptr = (llc_header *)(skb_tail - add_len);
	info->llc_id = (llc_hdr_ptr->llc_id & MAX_ITEM_MASK);
	info->llc_id_16 = llc_hdr_ptr->llc_id;
	info->tid = llc_hdr_ptr->tid; // tid在发送端做了映射
	info->fn = llc_hdr_ptr->fn;
	info->is_redundant = llc_hdr_ptr->is_redundant_mode;

	if (llc_hdr_ptr->reset == 1 && llc_hdr_ptr->flag.mask != g_window_reset_mask[info->tid]) {
		g_window_reset_mask[info->tid] = llc_hdr_ptr->flag.mask;
		g_recv_array[info->tid]->reset_flag = 1;
		debug_inc_reset_num();
		amax_reset_single_recv_array(info->tid);
		window_debug_inc_reset_num(info->tid);
	} else {
		g_recv_array[info->tid]->reset_flag = 0;
	}
	debug_inc_rx_num(info->fn, info->tid);

	if (net_buf->len > add_len)
		skb_trim(net_buf, net_buf->len - add_len);

	info->netbuf = net_buf;
	return SUCC;
}

static void window_fill_handle(recv_frame *info, cache_array_recv_handle *recv_array_hdl, recv_frame *recv_frame_array)
{
	if (recv_frame_array[info->llc_id].recv_status == NOT_RECEIVED) {
		recv_frame_array[info->llc_id].recv_status = RECEIVED;
		recv_frame_array[info->llc_id].recv_time = g_amax_timetick;
		recv_frame_array[info->llc_id].llc_id = info->llc_id;
		recv_frame_array[info->llc_id].llc_id_16 = info->llc_id_16;
		recv_frame_array[info->llc_id].tid = info->tid;
		recv_frame_array[info->llc_id].fn = info->fn;
		recv_frame_array[info->llc_id].netbuf = info->netbuf;

		recv_array_hdl->cache_num++;
		if (recv_array_hdl->cache_num > recv_array_hdl->max_cache_num)
			recv_array_hdl->max_cache_num = recv_array_hdl->cache_num;
		// 更新窗口上沿
		if ((int16_t)(info->llc_id_16 - recv_frame_array[recv_array_hdl->rx_upedge].llc_id_16) > 0)
			recv_array_hdl->rx_upedge = info->llc_id;
	} else {
		/* debug */
		if (amax_check_dup_pkt(info->netbuf, recv_frame_array[info->llc_id].netbuf) == 1) {
			recv_array_hdl->true_duplicate_num++;
		} else {
			recv_array_hdl->false_duplicate_num++;
		}
		if (info->netbuf != NULL) {
			dev_kfree_skb_any(info->netbuf);
			info->netbuf = NULL;
		}
		return;
	}
}

static void force_move_window(recv_frame *info, cache_array_recv_handle *recv_array_hdl)
{
	while ((uint16_t)(info->llc_id_16 - recv_array_hdl->real_lowedge) >= MAX_ITEM_NUM - 1) {
		amax_rx_time_process(info->tid);
		// 超时移动后窗口为空，直接将下沿移至当前LLC_ID
		if (recv_array_hdl->rx_lowedge == ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) {
			recv_array_hdl->real_lowedge = info->llc_id_16;
			recv_array_hdl->rx_lowedge = info->llc_id;
			recv_array_hdl->rx_upedge = ((info->llc_id + MAX_ITEM_NUM - 1) & MAX_ITEM_MASK);
			recv_array_hdl->frame_array[recv_array_hdl->rx_upedge].llc_id_16 = info->llc_id_16 - 1;
			debug_inc_empty_num();
			break;
		}
	}
}

static void amax_rx_frame_process(recv_frame *info, cache_array_recv_handle *recv_array_hdl)
{
	recv_frame *recv_frame_array = NULL;
	int32_t pre_win_wait_time = 0;
	int32_t pre_submit_wait_time = 0;

	if (info == NULL || recv_array_hdl == NULL) {
		amax_print(PRINT_ERROR, "input params error\n");
		return;
	}

	recv_frame_array = recv_array_hdl->frame_array;
	amax_get_cur_llc_id(info);

	/* 窗口外报文及回环处理 note: 取'='时，会出现空和满的歧义 */
	if ((uint16_t)(info->llc_id_16 - recv_array_hdl->real_lowedge) >= MAX_ITEM_NUM - 1) {
		/* 序号回环，强制提交窗口内报文 */
		if ((int16_t)(info->llc_id_16 - recv_array_hdl->real_lowedge) > 0) {
			pre_win_wait_time = g_max_window_wait_time;
			pre_submit_wait_time = g_max_submit_wait_time;
			/* 将超时时间置0，强制移动窗口 */
			g_max_window_wait_time = 0;
			g_max_submit_wait_time = 0;
			force_move_window(info, recv_array_hdl);
			g_max_window_wait_time = pre_win_wait_time;
			g_max_submit_wait_time = pre_submit_wait_time;
		} else {
			if (info->is_redundant == 0) {
				recv_array_hdl->out_of_window_frame_num++;
				recv_array_hdl->total_oow_frame_num++;
			}
			amax_oow_process(info->netbuf, recv_array_hdl);
			return;
		}
	}

	/* 窗口等待时间更新 */
	recv_array_hdl->wait_time = g_amax_timetick;
	window_fill_handle(info, recv_array_hdl, recv_frame_array);

	return;
}

static int32_t check_recv_array_valid(cache_array_recv_handle *recv_array_hdl)
{
	/* 缓存数组为空，直接退出 */
	if (recv_array_hdl == NULL || recv_array_hdl->rx_lowedge == ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK))
		return FAIL;
	return SUCC;
}

static int32_t check_timeout(cache_array_recv_handle *recv_array_hdl, recv_frame *recv_frame_info, uint8_t tid)
{
	uint64_t submit_wait_time = 0;
	uint64_t win_wait_time = 0;
	uint16_t ip_id = 0;

	if (g_submit_timeout) {
		submit_wait_time = TICK_TO_TIME(g_amax_timetick - recv_frame_info->recv_time);
		ip_id = get_ip_id(recv_frame_info->netbuf, 0);
		if (submit_wait_time >= g_max_submit_wait_time) {
			debug_inc_submit_timeout_cnt(tid);
			return SUCC;
		}
	}
	if (g_wait_timeout) {
		win_wait_time = TICK_TO_TIME(g_amax_timetick - recv_array_hdl->wait_time);
		ip_id = get_ip_id(recv_frame_info->netbuf, 0);
		if (win_wait_time >= g_max_window_wait_time) {
			debug_inc_win_wait_timeout_cnt(tid);
			return SUCC;
		}
	}

	return FAIL;
}

static void amax_rx_time_process(uint8_t tid)
{
	uint16_t cur_id;
	bool cache_flag = false;
	cache_array_recv_handle *recv_array_hdl = g_recv_array[tid];
	recv_frame *recv_frame_array = NULL;

	if (check_recv_array_valid(recv_array_hdl) != SUCC)
		return;

	recv_frame_array = recv_array_hdl->frame_array;
	/* 寻找首个缓存等待中的报文 */
	cur_id = recv_array_hdl->rx_lowedge;
	while (cur_id != ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) {
		if (recv_frame_array[cur_id].recv_status == RECEIVED) {
			cache_flag = true;
			break;
		}
		cur_id = ((cur_id + 1) & MAX_ITEM_MASK);
	}

	if (cache_flag == false) {
		amax_print(PRINT_DEBUG, "no cache frame, LowEdge: %d, LastId: %d\n", recv_array_hdl->rx_lowedge,
			recv_array_hdl->rx_upedge);
		return;
	}
	if (check_timeout(recv_array_hdl, &recv_frame_array[cur_id], tid) != SUCC)
		return;

	/* 统计因窗口超时移动导致的丢包，并更新窗口下沿 */
	amax_abnormal_inc_fdrop_num(recv_array_hdl, cur_id);

	recv_array_hdl->drop_pkts_num =
		recv_array_hdl->drop_pkts_num + ((cur_id + MAX_ITEM_NUM - recv_array_hdl->rx_lowedge) & MAX_ITEM_MASK);
	recv_array_hdl->rx_lowedge = cur_id;
	recv_array_hdl->real_lowedge = recv_frame_array[cur_id].llc_id_16;

	if (g_max_window_wait_time > 0) {
		recv_array_hdl->timeout_flag = 1;
	} else {
		recv_array_hdl->timeout_flag = 0;
	}

	amax_move_recv_windows(tid);

	/* 强制滑窗统计 */
	if (g_max_window_wait_time == 0) {
		debug_inc_update_move();
		window_debug_inc_update_move(tid);
		amax_abnormal_inc_update_move();
	} else {
		debug_inc_timeout_move();
		window_debug_inc_timeout_move(tid);
		amax_abnormal_inc_timeout_move();
	}

	return;
}

static void amax_move_recv_windows(uint8_t tid)
{
	uint16_t index = 0;
	cache_array_recv_handle *recv_array_hdl = NULL;
	recv_frame *recv_frame_array = NULL;

	recv_array_hdl = g_recv_array[tid];
	index = recv_array_hdl->rx_lowedge;
	recv_frame_array = recv_array_hdl->frame_array;

	// 遍历窗口
	while (index != ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) { // 窗口非空
		if (recv_frame_array[index].recv_status == RECEIVED) {
			// 提交报文至协议栈
			recv_array_hdl->cache_num--;
			amax_debug_stat_window_delay(recv_frame_array[index].netbuf);
			hmac_hook_driver_netif_rx(recv_frame_array[index].netbuf, recv_frame_array[index].netbuf->dev);
			/* debug */
			debug_inc_submit_num_total();
			window_debug_inc_submit_num_total(tid);

			if (recv_frame_array[index].fn == FN_2) {
				debug_inc_submit_num_2g();
				window_debug_inc_submit_num_2g(tid);
			} else if (recv_frame_array[index].fn == FN_5) {
				debug_inc_submit_num_5g();
				window_debug_inc_submit_num_5g(tid);
			}
			recv_frame_array[index].netbuf = NULL;
			recv_frame_array[index].recv_status = NOT_RECEIVED;
			recv_frame_array[index].recv_time = 0;
			recv_array_hdl->rx_lowedge = ((index + 1) & MAX_ITEM_MASK);
			recv_array_hdl->real_lowedge = recv_frame_array[index].llc_id_16 + 1;
		} else {
			break;
		}
		index = ((index + 1) & MAX_ITEM_MASK);
	}

	return;
}

static void amax_get_cur_llc_id(recv_frame *info)
{
	if (info->fn == FN_2) {
		g_recv_array[info->tid]->curr_llc_id_2g = info->llc_id_16;
	} else {
		g_recv_array[info->tid]->curr_llc_id_5g = info->llc_id_16;
	}
	return;
}

static uint8_t amax_check_dup_pkt(struct sk_buff *new_nb, struct sk_buff *old_nb)
{
	uint16_t new_ip_id;
	uint16_t old_ip_id;

	if (new_nb == NULL || old_nb == NULL || (new_nb->len != old_nb->len))
		return 0;

	new_ip_id = get_ip_id(new_nb, 0);
	old_ip_id = get_ip_id(old_nb, 0);

	return new_ip_id == old_ip_id ? 1 : 0;
}

static void amax_move_empty_recv_window(recv_frame *info, cache_array_recv_handle *recv_array_hdl)
{
	// 超时移动后窗口为空，直接将下沿移至当前LLC_ID
	if (recv_array_hdl->reset_flag != 0 || recv_array_hdl->timeout_flag == 0)
		return;

	if (recv_array_hdl->rx_lowedge == ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) {
		recv_array_hdl->real_lowedge = info->llc_id_16;
		recv_array_hdl->rx_lowedge = info->llc_id;
		recv_array_hdl->rx_upedge = ((info->llc_id + MAX_ITEM_NUM - 1) & MAX_ITEM_MASK);
		recv_array_hdl->frame_array[recv_array_hdl->rx_upedge].llc_id_16 = info->llc_id_16 - 1;
		recv_array_hdl->timeout_flag = 0;
		debug_inc_timeout_force_update_num();
	}
	return;
}

static void update_dev_info_get_flag(void)
{
	int32_t ret;

	ret = amax_get_dev_info();
	if (ret == SUCC) {
		amax_print(PRINT_INFO, "amax_get_dev_info succ.\n");
		g_is_dev_info_get = true;
	} else {
		amax_print(PRINT_ERROR, "amax_get_dev_info failed.\n");
		g_is_dev_info_get = false;
	}
}

/* 查询主路的频段 */
static uint32_t amax_get_main_dev_fn()
{
#ifdef CONFIG_CONNECTIVITY_HI1105
	uint8_t chan_number;
	amax_mac_vap_stru *pst_mac_vap = NULL;
	if (g_netdev_main == NULL) {
		amax_print(PRINT_ERROR, "g_netdev_main is NULL\n");
		goto exit;
	}
	pst_mac_vap = g_netdev_main->ml_priv;
	if (pst_mac_vap == NULL) {
		amax_print(PRINT_ERROR, "pst_mac_vap is NULL\n");
		goto exit;
	}
	chan_number = pst_mac_vap->st_channel.uc_chan_number;
	amax_print(PRINT_INFO, "chan_number of main wdev is %u\n", chan_number);
	if (chan_number >= BAND_2G_MIN_CHANNEL && chan_number <= BAND_2G_MAX_CHANNEL) {
		g_fn_main = FN_2;
		g_fn_assist = FN_5;
		return FN_2;
	}
exit:
#endif
	/* 默认5g为主链 */
	g_fn_main = FN_5;
	g_fn_assist = FN_2;
	return FN_5;
}

static int32_t amax_get_dev(void)
{
	g_netdev_main = dev_get_by_name(&init_net, g_dev_name_main);
	g_netdev_assist = dev_get_by_name(&init_net, g_dev_name_assist);
	if (g_netdev_main != NULL && g_netdev_assist != NULL)
		return SUCC;
	return FAIL;
}

static int32_t amax_get_dev_mac_addr(void)
{
	int32_t ret;
	if (g_netdev_main == NULL || g_netdev_assist == NULL)
		return FAIL;

	ret = memcpy_s(g_dev_mac_addr_main, ETHER_ADDR_LEN, g_netdev_main->dev_addr, ETHER_ADDR_LEN);
	if (ret != 0)
		return FAIL;
	ret = memcpy_s(g_dev_mac_addr_assist, ETHER_ADDR_LEN, g_netdev_assist->dev_addr, ETHER_ADDR_LEN);
	if (ret != 0)
		return FAIL;

	return SUCC;
}

static int32_t amax_get_dev_info(void)
{
	/* 获取主链路和辅链路的net dev */
	if (amax_get_dev() != SUCC)
		return FAIL;

	/* 获取主链路和辅链路的mac address */
	if (amax_get_dev_mac_addr() != SUCC)
		return FAIL;

	return SUCC;
}

static void amax_reset_tx_status(void)
{
	spin_lock_bh(&g_tx_spinlock);

	/* 以时间戳后8位作为本次multilink连接的标志 这个损失数据的强转是代码设计意图 */
	g_sta_pair->init_mask = (uint8_t)ktime_get();
	memset_s(g_sta_pair->llc_id, sizeof(g_sta_pair->llc_id), 0, sizeof(g_sta_pair->llc_id));
	memset_s(g_sta_pair->init_count, sizeof(g_sta_pair->init_count), 0, sizeof(g_sta_pair->init_count));
	amax_print_tx_rx_info();

	debug_inc_tx_reset_cnt();

	/* 更新全局net_dev结构体, 以及主辅链路的mac地址 */
	update_dev_info_get_flag();
	g_error_netbuf = 0;

	spin_unlock_bh(&g_tx_spinlock);
}

static struct net_device *get_net_dev(uint8_t vdev_id)
{
	if (vdev_id == g_fn_main) {
		return g_netdev_main;
	} else if (vdev_id == g_fn_assist) {
		return g_netdev_assist;
	}
	amax_print(PRINT_ERROR, "error: wrong input.\n");
	return NULL;
}

static int32_t classify_rule_apply(struct iphdr *p_ipheader, struct sk_buff *netbuf, uint16_t src_port,
	uint16_t dst_port)
{
	if (g_classify_mode == CLASSIFY_MODE_TCP_UDP) { // 过滤所有TCP/UDP报文
		return SUCC;
	} else if (g_classify_mode == CLASSIFY_MODE_UDP) { // 过滤所有UDP报文
		if (p_ipheader->protocol == MAC_UDP_PROTOCAL)
			return SUCC;
	} else if (g_classify_mode == CLASSIFY_MODE_TCP) { // 过滤所有TCP报文
		if (p_ipheader->protocol == MAC_TCP_PROTOCAL)
			return SUCC;
	}
	return FAIL;
}

static bool amax_is_ip_segment_same(struct iphdr *p_ipheader)
{
	uint32_t sa_ip_seg;
	uint32_t da_ip_seg;

	if (g_same_segment_pkg_filter == 0)
		return false;
	/* 获取源ip地址网段与目的ip地址网段 */
	sa_ip_seg = (p_ipheader->saddr << IP_SEG_OFFSET);
	da_ip_seg = (p_ipheader->daddr << IP_SEG_OFFSET);

	/* 过滤ip网段相同的tcp/udp报文 */
	if (sa_ip_seg == da_ip_seg)
		return true;
	return false;
}

static int32_t amax_outbound_frame_classify_core(struct sk_buff *netbuf)
{
	struct ethhdr *p_macheader = NULL;
	struct iphdr *p_ipheader = NULL;
	struct udphdr *p_udpheader = NULL;
	struct tcphdr *p_tcpheader = NULL;
	uint16_t src_port = 0;
	uint16_t dst_port = 0;

	p_macheader = (struct ethhdr *)(netbuf->data);
	p_ipheader = (struct iphdr *)(p_macheader + 1);

	// 只过滤TCP和UDP协议的报文
	if ((p_ipheader->protocol != MAC_TCP_PROTOCAL) && (p_ipheader->protocol != MAC_UDP_PROTOCAL))
		return FAIL;

	switch (p_ipheader->protocol) {
	case MAC_UDP_PROTOCAL:
		if (amax_is_ip_segment_same(p_ipheader)) {
			debug_inc_same_seg_num();
			return FAIL;
		}
		p_udpheader = (struct udphdr *)(p_ipheader + 1);
		src_port = p_udpheader->source;
		dst_port = p_udpheader->dest;
		break;
	case MAC_TCP_PROTOCAL:
		if (amax_is_ip_segment_same(p_ipheader)) {
			debug_inc_same_seg_num();
			return FAIL;
		}
		p_tcpheader = (struct tcphdr *)(p_ipheader + 1);
		src_port = p_tcpheader->source;
		dst_port = p_tcpheader->dest;
		break;
	default:
		src_port = 0;
		dst_port = 0;
		break;
	}

	return classify_rule_apply(p_ipheader, netbuf, src_port, dst_port);
}

static int32_t amax_tx_classify_unwanted_frame(struct sk_buff *netbuf)
{
	struct ethhdr *mac_header = NULL;
	struct iphdr *ip_header = NULL;

	mac_header = (struct ethhdr *)(netbuf->data);

	/* 不处理非线性报文 以及不带mark的报文 */
	if (netbuf->data_len != 0 || netbuf->mark != CLASSIFY_MARK)
		return SUCC;

	/* 不处理组播报文 */
	if (ETH_IS_MULTICAST(mac_header->h_dest))
		return SUCC;

	/* 不处理非ipv4报文, 以及经过amax封装的辅链发送的报文 */
	if (mac_header->h_proto != htons(ETHER_TYPE_IP) || eth_frame_is_amax_encaped(netbuf))
		return SUCC;

	ip_header = (struct iphdr *)(mac_header + 1);
	/* 不处理DHCP报文和hilink心跳包 */
	if (eth_frame_is_dhcp(ip_header) || eth_frame_is_hilink(ip_header))
		return SUCC;

	return FAIL;
}

static void amax_encap_tuples_sign(struct sk_buff *netbuf)
{
	uint32_t add_len = 0;
	llc_header *llc_hdr = NULL;
	uint8_t *magic_hdr = NULL;
	int32_t sec_ret;

	add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
	if (skb_tailroom(netbuf) < add_len) {
		debug_inc_tuples_tail_less_cnt();
		return;
	}
	if (netbuf->ip_summed == CHECKSUM_PARTIAL) {
		skb_checksum_help(netbuf);
		netbuf->ip_summed = CHECKSUM_COMPLETE;
	}
	llc_hdr = (llc_header *)skb_put(netbuf, sizeof(llc_header));
	memset_s(llc_hdr, sizeof(llc_header), 0, sizeof(llc_header));

	magic_hdr = (uint8_t *)skb_put(netbuf, MAGIC_AMAX_LEN);
	sec_ret = memcpy_s(magic_hdr, MAGIC_AMAX_LEN, g_magic_tuples_sign, MAGIC_AMAX_LEN);
	if (sec_ret != EOK) {
		amax_print(PRINT_ERROR, "memcpy_s error!\n");
		return;
	}
	debug_inc_tuples_encap_cnt();
	return;
}

static PACKET_TYPE amax_outbound_frame_classify(struct sk_buff *netbuf, uint8_t *tid)
{
	struct ethhdr *p_macheader = NULL;
	struct iphdr *p_ipheader = NULL;

	if (unlikely(netbuf == NULL || netbuf->data == NULL)) {
		amax_print(PRINT_ERROR, "NULL buf\n");
		return PACKET_INVALID;
	}

	p_macheader = (struct ethhdr *)(netbuf->data);
	p_ipheader = (struct iphdr *)(p_macheader + 1);
	/* 如果是大包，不满足双发条件，封装tuples_sign后，送回原流程处理 */
	if (netbuf->len > g_classify_len || eth_frame_is_fragmented(p_ipheader)) {
		amax_encap_tuples_sign(netbuf);
		return PACKET_INVALID;
	}

	*tid = amax_get_tid(p_ipheader);

	if (amax_check_tid_is_needed(*tid) == SUCC && amax_outbound_frame_classify_core(netbuf) == SUCC)
		return PACKET_OUTBOUND;

	return PACKET_INVALID;
}

static void amax_set_llc_id(send_frame *info, uint8_t tid)
{
	spin_lock_bh(&g_tx_spinlock);

	info->llc_id = g_sta_pair->llc_id[tid]++;

	spin_unlock_bh(&g_tx_spinlock);
}

static void amax_tx_frame_dispatch(struct sk_buff *netbuf, send_frame *info, uint8_t tid)
{
	uint32_t avg_main_throughput;
	uint32_t avg_assist_throughput;
	uint32_t avg_main_contime;
	uint32_t avg_assist_contime;
	uint32_t amax_tx_pps_avg;

	info->netbuf = netbuf;
	info->tid = tid;

	avg_main_throughput = amax_get_tx_avg_throughput(g_fn_main);
	avg_assist_throughput = amax_get_tx_avg_throughput(g_fn_assist);
	avg_main_contime = amax_get_tx_contime_sum(g_fn_main) >> CONTIME_STAT;
	avg_assist_contime = amax_get_tx_contime_sum(g_fn_assist) >> CONTIME_STAT;
	amax_tx_pps_avg = amax_get_tx_pps_avg(g_fn_main);

	if (g_send_mode_param == AMAX_FORCE_FN_5) {
		info->fn = FN_5;
	} else if (g_send_mode_param == AMAX_FORCE_FN_2) {
		info->fn = FN_2;
	} else if (g_send_mode_param == AMAX_FORCE_FN_DUAL) {
		info->fn = FN_DOUBLE;
	} else {
		if (g_is_game_foreground) {
			info->fn = FN_DOUBLE;
		// 10MBps = 10 ^ 17 / 10;
		} else if (avg_main_throughput < ((g_throughput_threshold << BYTE_TRANSFER) / 10) &&
			avg_main_contime > g_latency_threshold && amax_tx_pps_avg < g_pps_threshold) {
			info->fn = FN_DOUBLE;
		} else {
			info->fn = g_fn_main;
		}
	}
}

static int32_t amax_tx_frame_encapsulation(send_frame *info)
{
	uint32_t add_len = 0;
	int32_t ret;
	// 安全检查
	if (unlikely(info == NULL || info->netbuf == NULL)) {
		amax_print(PRINT_ERROR, "error: Empty Input!\n");
		return FAIL;
	}

	add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
	// 判断现有SKB的tailroom是否足够LLC头和MAGIC LEN
	if (skb_tailroom(info->netbuf) < add_len) {
		ret = pskb_expand_head(info->netbuf, 0, add_len, GFP_ATOMIC);
		if (unlikely(ret < 0)) {
			amax_print(PRINT_ERROR, "Realloc headroom failed\n");
			return FAIL;
		}
		if (info->netbuf == NULL || info->netbuf->data_len != 0) {
			amax_print(PRINT_WARNING, "after realloc. data_len is error\n");
			return FAIL;
		}
		debug_inc_expand_tail_cnt();
	}

	ret = amax_encap_new_header(info);
	if (unlikely(ret < 0)) {
		amax_print(PRINT_ERROR, "encap failed\n");
		return FAIL;
	}

	return SUCC;
}

static int32_t amax_encap_new_header(send_frame *info)
{
	struct sk_buff *netbuf = NULL;
	llc_header *llc_hdr = NULL;
	uint8_t *magic_hdr = NULL;
	int32_t sec_ret;

	// 安全检查
	if (unlikely(info->netbuf == NULL)) {
		amax_print(PRINT_ERROR, "Empty Input!\n");
		return FAIL;
	}

	netbuf = info->netbuf;
	// 填充LLC头
	llc_hdr = (llc_header *)skb_put(netbuf, sizeof(llc_header));
	(void)memset_s(llc_hdr, sizeof(llc_header), 0, sizeof(llc_header));
	llc_hdr->llc_id = info->llc_id;
	llc_hdr->tid = info->tid;
	llc_hdr->fn = g_fn_main;
	llc_hdr->is_redundant_mode = (info->fn == FN_DOUBLE);
	if (g_sta_pair->init_count[llc_hdr->tid] < AMAX_RESET_PKT_NUM) {
		g_sta_pair->init_count[llc_hdr->tid]++;
		llc_hdr->reset = 1;
		llc_hdr->flag.mask = g_sta_pair->init_mask;
	}

	// 填充MAGIC
	magic_hdr = (uint8_t *)skb_put(netbuf, MAGIC_AMAX_LEN);
	sec_ret = memcpy_s(magic_hdr, MAGIC_AMAX_LEN, g_magic_amax_sign, MAGIC_AMAX_LEN);
	if (sec_ret != EOK) {
		amax_print(PRINT_ERROR, "memcpy_s error!\n");
		return FAIL;
	}

	return SUCC;
}

static int32_t amax_tx_frame_send(send_frame *info, struct net_device **net_dev)
{
	int32_t ret = SUCC;
	struct sk_buff *netbuff = NULL;
	struct sk_buff *netbuf_assist = NULL;
	struct sk_buff *netbuf_main = NULL;

	netbuff = info->netbuf;
	if (info->fn == FN_DOUBLE) {
		// 主链路报文不需要额外修改，直接走后续流程发送
		netbuf_main = netbuff;
		netbuf_assist = skb_copy(netbuff, GFP_ATOMIC);
		if (netbuf_assist == NULL)
			return FAIL;
		amax_tx_send_copy_nb(netbuf_assist);
	} else if (info->fn == FN_5 || info->fn == FN_2) {
		ret = amax_tx_frame_send_fn_spec(netbuff, info->fn, net_dev);
		if (ret != SUCC)
			amax_print(PRINT_ERROR, "tx failed: fn: %d, llc_id: %d\n", info->fn, info->llc_id);
	}
	netbuff->mark = 0;
	return ret;
}

static uint32_t amax_tx_frame_send_fn_spec(struct sk_buff *netbuff, uint8_t fn, struct net_device **net_dev)
{
	llc_header *llc_hdr = NULL;
	uint16_t add_len;
	uint8_t *skb_tail = NULL;

	if (unlikely(netbuff == NULL || fn >= FN_NUM)) {
		amax_print(PRINT_ERROR, "error: invalid netbuf\n");
		return FAIL;
	}

	skb_tail = amax_get_skb_tail(netbuff);
	add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
	llc_hdr = (llc_header *)(skb_tail - add_len);
	llc_hdr->fn = fn;

	amax_set_mac_addr(netbuff, fn);
	*net_dev = get_net_dev(fn);
	netbuff->dev = get_net_dev(fn);

	return SUCC;
}

static void amax_tx_send_copy_nb(struct sk_buff *netbuf)
{
	llc_header *llc_hdr = NULL;
	uint16_t add_len;
	uint8_t *skb_tail = NULL;

	if (unlikely(netbuf == NULL)) {
		amax_print(PRINT_ERROR, "error: invalid netbuf\n");
		return;
	}

	skb_tail = amax_get_skb_tail(netbuf);
	add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
	llc_hdr = (llc_header *)(skb_tail - add_len);

	llc_hdr->fn = g_fn_assist;
	amax_set_mac_addr(netbuf, g_fn_assist);
	netbuf->dev = get_net_dev(g_fn_assist);
	netbuf->tstamp = ktime_get();

	netbuf->mark = 0;
	/* 调用原接口发送copy的报文 注意钩子的二次截获问题 */
	hmac_hook_driver_tx_send(netbuf, netbuf->dev);
}

static void amax_set_mac_addr(struct sk_buff *netbuf, uint8_t fn)
{
	struct ethhdr *p_macheader = NULL;

	p_macheader = (struct ethhdr *)(netbuf->data);
	if (fn == g_fn_main) {
		set_mac_addr(p_macheader->h_source, ETHER_ADDR_LEN, g_dev_mac_addr_main, ETHER_ADDR_LEN);
	} else if (fn == g_fn_assist) {
		set_mac_addr(p_macheader->h_source, ETHER_ADDR_LEN, g_dev_mac_addr_assist, ETHER_ADDR_LEN);
	} else {
		amax_print(PRINT_ERROR, "error: wrong fn %hhu\n", fn);
	}
	return;
}

// 统计throught和获取时间戳
static void amax_tx_stat_process(struct sk_buff *netbuf, uint8_t fn)
{
	uint32_t len;
	uint32_t throutput;

	len = netbuf->len;
	/* 初始化统计时间戳 */
	netbuf->tstamp = ktime_get();
	if (fn == FN_5) {
		throutput = amax_get_tx_5g_throughput(g_amax_throughput_index);
		amax_set_tx_5g_throughput(g_amax_throughput_index, throutput + len);
		amax_inc_tx_pps(FN_5);
		amax_tx_dmd_inc_throughput(FN_5, len);
		amax_tx_dmd_inc_packet(FN_5);
	} else if (fn == FN_2) {
		throutput = amax_get_tx_2g_throughput(g_amax_throughput_index);
		amax_set_tx_2g_throughput(g_amax_throughput_index, throutput + len);
		amax_inc_tx_pps(FN_2);
		amax_tx_dmd_inc_throughput(FN_2, len);
		amax_tx_dmd_inc_packet(FN_2);
	} else if (fn == FN_DOUBLE) {
		throutput = amax_get_tx_5g_throughput(g_amax_throughput_index);
		amax_set_tx_5g_throughput(g_amax_throughput_index, throutput + len);

		throutput = amax_get_tx_2g_throughput(g_amax_throughput_index);
		amax_set_tx_2g_throughput(g_amax_throughput_index, throutput + len);
		amax_inc_tx_pps(FN_5);
		amax_inc_tx_pps(FN_2);

		amax_tx_dmd_inc_throughput(FN_5, len);
		amax_tx_dmd_inc_throughput(FN_2, len);
		amax_tx_dmd_inc_packet(FN_5);
		amax_tx_dmd_inc_packet(FN_2);
	} else {
		amax_print(PRINT_ERROR, "error: wrong fn %hhu\n", fn);
	}

	return;
}

static void amax_update_pkt_congestion_status(uint32_t con_time, uint8_t fn)
{
	uint32_t *contime_array = NULL;
	uint32_t contime_sum;
	uint8_t cur_sample_index;

	contime_array = amax_get_tx_contime_array(fn);
	cur_sample_index = g_contime_index[fn] % CONTIME_SAMPLE_NUM;
	// 32个报文时延总和，减去最老的数据加上最新的数据
	contime_sum = amax_get_tx_contime_sum(fn);
	amax_set_tx_contime_sum(fn, contime_sum + con_time - contime_array[cur_sample_index]);
	contime_array[cur_sample_index] = con_time;
	g_contime_index[fn]++;

	return;
}

static uint8_t judge_vdev_id_by_dev(struct net_device *dev)
{
	if (dev == NULL)
		/* 驱动存在非数据报文，这类报文的net_dev为NULL，这里删除打印避免刷屏 */
		return FN_INVALID;

	if (dev == g_netdev_main) {
		return g_fn_main;
	} else if (dev == g_netdev_assist) {
		return g_fn_assist;
	}

	return FN_INVALID;
}

int32_t amax_tx_complete_handle_hook(const struct sk_buff *netbuf)
{
	ktime_t congestion_time;
	uint8_t *skb_tail = NULL;
	uint32_t contime_ms;
	uint16_t add_len;
	uint8_t vdev_id;
	llc_header *llc_hdr = NULL;

	if (netbuf == NULL) {
		amax_print(PRINT_ERROR, "Error: empty input.\n");
		return 0;
	}
	if (g_is_dev_info_get == true && eth_frame_is_amax_encaped(netbuf)) {
		vdev_id = judge_vdev_id_by_dev(netbuf->dev);
		if (vdev_id == FN_INVALID)
			return 0;
		// 拥塞时间 = 当前时间 - 发送时间（发送时记录）
		congestion_time = ktime_sub(ktime_get(), netbuf->tstamp);
		// 转换成ms
		contime_ms = (uint32_t)ktime_to_ms(congestion_time);
		if (netbuf->tstamp == 0 || contime_ms > MAX_LATENCY_10S) {
			g_error_netbuf++;
			return 0;
		}
		// 统计multilink包的吞吐
		if (vdev_id == g_fn_main) {
			amax_update_pkt_congestion_status(contime_ms, g_fn_main);
			add_len = sizeof(llc_header) + MAGIC_AMAX_LEN;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
			skb_tail = (uint8_t *)(netbuf->head + netbuf->tail);
#else
			skb_tail = (uint8_t *)netbuf->tail;
#endif
			llc_hdr = (llc_header *)(skb_tail - add_len);
			if (llc_hdr->is_redundant_mode)
				amax_tx_dmd_handle_new_contime(contime_ms, ML_MASTER);
		} else if (vdev_id == g_fn_assist) {
			amax_update_pkt_congestion_status(contime_ms, g_fn_assist);
			amax_tx_dmd_handle_new_contime(contime_ms, ML_SLAVE);
		} else {
			amax_print(PRINT_ERROR, "error: wrong vdev_id %hhu\n", vdev_id);
		}
	}

	return 0;
}

int32_t amax_tx_handle_hook(struct sk_buff *netbuf, struct net_device **net_dev)
{
	uint8_t tid = 0;
	int32_t ret = SUCC;
	PACKET_TYPE packet_type = 0;
	send_frame info = { 0 };

	if ((net_dev == NULL) || (*net_dev != g_netdev_main && *net_dev != g_netdev_assist))
		return ret;

	if (netbuf->mark == CLASSIFY_MARK) {
		debug_inc_mark_num();
		amax_timer_start(&g_amax_timer);
	}

	/* 对特殊的报文不做处理：非ipv4包、组播包、DHCP包、hilink心跳报文、封装过的报文、不带mark报文 */
	if (amax_tx_classify_unwanted_frame(netbuf) == SUCC)
		return ret;

	if (g_enable_amax_uplink && g_is_dev_info_get == true)
		packet_type = amax_outbound_frame_classify(netbuf, &tid);

	if (packet_type == PACKET_OUTBOUND) {
		if (netbuf->ip_summed == CHECKSUM_PARTIAL) {
			skb_checksum_help(netbuf);
			netbuf->ip_summed = CHECKSUM_COMPLETE;
		}

		tid = g_tid_arr_table[tid];
		amax_set_llc_id(&info, tid);
		amax_tx_frame_dispatch(netbuf, &info, tid);
		ret = amax_tx_frame_encapsulation(&info);
		if (ret != SUCC) {
			debug_inc_pkt_fail(info.fn);
			return ret;
		}

		amax_tx_stat_process(info.netbuf, info.fn);

		ret = amax_tx_frame_send(&info, net_dev);
		if (ret == SUCC) {
			debug_inc_pkt_dc(info.fn);
		} else {
			debug_inc_pkt_fail(info.fn);
		}
	}

	return ret;
}

/* uplink develop */
int32_t amax_rx_handle_hook(struct sk_buff *netbuf)
{
	bool cache_flag = false;
	PACKET_TYPE packet_type = PACKET_INVALID;

	if (netbuf->dev != g_netdev_main && netbuf->dev != g_netdev_assist)
		return NOT_PROCESS;

	packet_type = amax_inbound_frame_classify(netbuf);
	// 截获带有amax封装的报文
	if (packet_type == PACKET_INBOUND) {
		// 该节点从skb链表中删除
		netbuf->tstamp = ktime_get();
		skb_queue_tail(&g_skb_queue_array, netbuf);
		cache_flag = true;
		amax_timer_start(&g_amax_timer);
		amax_trigger_data_flag(cache_flag);
		return HOOKED;
	} else if (packet_type == PACKET_DEVICE_PIPE_SYNERGY) {
		/* 端管数据报文需要上报到framework */
		dps_report_data_to_userspace(netbuf);
		return HOOKED;
	}
	return NOT_PROCESS;
}

static void amax_oow_process(struct sk_buff *skb, cache_array_recv_handle *recv_array)
{
	if (skb == NULL)
		return;

	if (g_udp_oow_submit == 0 || is_udp_pkt(skb) == false) {
		dev_kfree_skb_any(skb);
		return;
	}
	hmac_hook_driver_netif_rx(skb, skb->dev);
	recv_array->udp_oow_submit_num++;
}

static void amax_debug_stat_window_delay(struct sk_buff *netbuf)
{
	static uint64_t g_pre_submit_time;
	uint32_t submit_time_interval = 0;
	uint32_t stay_time = 0;

	if (g_stat_switch == 1) {
		stay_time = (uint32_t)ktime_to_ms(ktime_get() - netbuf->tstamp);
		if (stay_time >= g_duration_threshold)
			debug_inc_beyond_stay_cnt();
		debug_update_max_stay_time(stay_time);

		if (g_pre_submit_time == 0) {
			g_pre_submit_time = jiffies;
		} else {
			submit_time_interval = (uint32_t)jiffies_to_msecs(jiffies - g_pre_submit_time);
			g_pre_submit_time = jiffies;
			if (submit_time_interval <= MAX_INTERVAL)
				debug_update_max_win_interval(submit_time_interval);
		}
	}
}

int amax_ignore_sinfo_reducepower(struct net_device *dev, struct station_info *sinfo)
{
	int32_t ret;
	if (g_enable_amax_uplink && dev != NULL &&
		strncmp(dev->name, g_dev_name_assist, strlen(g_dev_name_assist)) == 0
		&& last_sinfo.filled != 0) {
		ret = memcpy_s(sinfo, sizeof(struct station_info), &last_sinfo, sizeof(struct station_info));
		if (ret != 0) {
			amax_print(PRINT_INFO, "memcpy_s error");
			return false;
		}
		return true;
	}
	return false;
}

void amax_store_station_info(struct net_device *dev, struct station_info *sinfo)
{
	int32_t ret;
	if (sinfo == NULL || dev == NULL || strncmp(dev->name, g_dev_name_assist, strlen(g_dev_name_assist)) != 0)
		return;

	ret = memcpy_s(&last_sinfo, sizeof(struct station_info), sinfo, sizeof(struct station_info));
	if (ret != 0)
		amax_print(PRINT_INFO, "memcpy_s error");
}

int amax_ioctl_reset_tx_status(uint8_t enable_amax)
{
	if (g_init_flag == false) {
		amax_print(PRINT_INFO, "multilink is not ready. exit ioctl process.\n");
		return 0;
	}
	amax_reset_tx_status();
	if (enable_amax == GAME_FOREGROUND) {
		amax_print(PRINT_INFO, "game app in foreground, receive amax enable msg");
		g_max_submit_wait_time = GAME_FOREGROUND_WAIT_TIME;
		if (!g_enable_amax_uplink) {
			g_enable_amax_uplink = 1;
			amax_print(PRINT_INFO, "g_enable_amax_uplink set to 1");
			amax_tx_dmd_set_main_freq(amax_get_main_dev_fn());
			amax_tx_dmd_handle_enable(ktime_get());
		}
	} else if (enable_amax == NON_GAME_FOREGROUND) {
		amax_print(PRINT_INFO, "normal app in froeground, receive amax enable msg");
		g_max_submit_wait_time = NON_GAME_FOREGROUND_WAIT_TIME;
		if (!g_enable_amax_uplink) {
			g_enable_amax_uplink = 1;
			amax_print(PRINT_INFO, "g_enable_amax_uplink set to 1");
			amax_tx_dmd_set_main_freq(amax_get_main_dev_fn());
			amax_tx_dmd_handle_enable(ktime_get());
		}
	} else {
		amax_print(PRINT_INFO, "receive amax disable msg");
		if (g_enable_amax_uplink) {
			g_enable_amax_uplink = 0;
			amax_tx_dmd_handle_disable(ktime_get());
			amax_print(PRINT_INFO, "g_enable_amax_uplink set to 0");
		}
	}
	amax_print(PRINT_INFO, "fn update succ. main:%hhu assist:%hhu\n", g_fn_main, g_fn_assist);
	return 0;
}

int amax_ioctl_set_threshold(uint8_t throughput, uint8_t latency, uint8_t packetnumber)
{
	if (g_init_flag == false) {
		amax_print(PRINT_INFO, "multilink is not ready. exit ioctl process.\n");
		return 0;
	}
	if (throughput > MAX_THRESHOLD || latency > MAX_THRESHOLD || packetnumber > MAX_THRESHOLD) {
		amax_print(PRINT_ERROR, "threshold large than maximum");
		return -1;
	} else {
		g_throughput_threshold = throughput;
		g_latency_threshold = latency;
		g_pps_threshold = packetnumber;
		amax_print(PRINT_INFO, "threshold set throughput to %d, latency to %d, packetnumber to %d",
			throughput, latency, packetnumber);
	}

	return 0;
}

void amax_module_init(void)
{
	if (g_init_flag) {
		amax_print(PRINT_ERROR, "avoid double init\n");
		return;
	}
	amax_print(PRINT_INFO, "enter amax_module_init\n");
	if (amax_try_init() == SUCC) {
		amax_hook_reg(amax_tx_handle_hook, amax_tx_complete_handle_hook, amax_rx_handle_hook);
		amax_ioctl_reg(amax_ioctl_reset_tx_status, amax_ioctl_set_threshold);
		dps_ioctl_reg(dps_ioctl_msg_proc);
	}
	return;
}

void amax_module_exit(void)
{
	if (!g_init_flag) {
		amax_print(PRINT_ERROR, "avoid double exit\n");
		return;
	}
	amax_print(PRINT_INFO, "enter amax_module_exit\n");
	amax_exit();
	dps_ioctl_unreg();
	amax_ioctl_unreg();
	amax_hook_unreg();
}

static int __init amax_mod_init(void)
{
	amax_module_init();
	amax_mod_reg(amax_module_init, amax_module_exit);
	return 0;
}

static void __exit amax_mod_exit(void)
{
	amax_module_exit();
	amax_mod_unreg();
}

#ifdef CONFIG_HW_WIFI_INSMOD_AMAX
module_init(amax_mod_init);
module_exit(amax_mod_exit);
MODULE_LICENSE("GPL");
#endif
