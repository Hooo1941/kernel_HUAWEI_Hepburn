/*
 * cloud_network.h
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

#ifndef _CLOUD_NETWORK_H_
#define _CLOUD_NETWORK_H_

#include <linux/types.h>
#include <linux/if.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include "netlink_handle_boosterd.h"

#define SIGN_KEY_LEN 32
#define CLOUD_NETWORK_HON_VERSION 4
#define CLINET_ID_LEN 6
#define APP_ID_LEN 3
#define FOUR_BYTE 4

#define NUM_0 0
#define NUM_1 1
#define NUM_2 2
#define NUM_3 3
#define NUM_4 4
#define NUM_5 5
#define NUM_6 6
#define NUM_7 7
#define NUM_8 8
#define NUM_9 9
#define NUM_10 10
#define NUM_11 11
#define NUM_12 12
#define NUM_13 13
#define NUM_14 14
#define NUM_15 15
#define NUM_16 16
#define NUM_17 17
#define NUM_18 18
#define NUM_19 19
#define NUM_20 20
#define NUM_21 21
#define NUM_22 22
#define NUM_23 23
#define NUM_24 24
#define NUM_25 25
#define NUM_26 26
#define NUM_27 27
#define NUM_28 28
#define NUM_29 29
#define NUM_30 30
#define NUM_31 31
#define NUM_32 32
#define NUM_33 33
#define NUM_34 34
#define NUM_35 35
#define NUM_36 36
#define NUM_37 37
#define NUM_38 38
#define NUM_39 39
#define NUM_40 40
#define NUM_41 41
#define NUM_42 42
#define NUM_43 43
#define NUM_44 44
#define NUM_45 45
#define NUM_46 46
#define NUM_47 47
#define NUM_48 48

#define PATH_NUM_MAX 3
#define PATH_WIFI_1 0b0001
#define PATH_MODEM_1 0b0010
#define PATH_MODEM_2 0b0100

#define ROLLE_PRIMARY_WIFI 0
#define ROLLE_PRIMARY_MODEM 1
#define ROLLE_SECONDARY_MODEM 2
#define ROLLE_MAX 3

#define NETLINK_DATA_MSG_LEN 2048

enum loop_ret_type {
	LOOP_BREAK = 0,
	LOOP_CONTINUE = 1,
	LOOP_SUCCESS = 2,
	LOOP_TYPE_NUM_MAX
};

typedef enum {
	DETECT_PATH_BIT_ERROR = 0,
	DETECT_PATH_BIT_WIFI_1 = 1,
	DETECT_PATH_BIT_MODEM_1 = 2,
	DETECT_PATH_BIT_MODEM_2 = 4,
	DETECT_PATH_BIT_WIFI_2 = 8
} algorithm_path;

enum {
	DETECT_PATH_VAL_ERROR = -1,
	DETECT_PATH_VAL_MODEM_1 = 1,
	DETECT_PATH_VAL_MODEM_2 = 2,
	DETECT_PATH_VAL_WIFI_1 = 3,
	DETECT_PATH_VAL_WIFI_2 = 4
};

enum scene_type {
	SCENE_WIFI_MODEM = 0,
	SCENE_MOEM_MODEM = 1,
	SCENE_TYPE_MAX
};

enum receive_cmd_type {
	INIT = 0,
	DEINIT = 1,
	TX_RX_CTL = 2,
	CLOUD_PARAM_ALL_UPDATE = 3,
	CLOUD_PARAM_KEY_UPDATE = 4,
	CLOUD_PARAM_COMPASS_UPDATE = 5,
	CLOUD_PARAM_APP_UPDATE = 6,
	LINK_INFO_UPDATE = 7,
	IMS_MONITOR_START = 8,
	IMS_MONITOR_STOP = 9,
	COMPASS_AVAILABLE_MONITOR_START = 10,
	COMPASS_AVAILABLE_MONITOR_STOP = 11,
	RECEIVE_CMD_TYPE_NUM_MAX
};

enum report_event_type {
	INIT_RESULT = 0,
	COMPASS_ABNORMAL = 1,
	STARTED = 2,
	STOPED = 3,
	TUN_IP_CONFILCT = 4,
	TUN_CONFIG_INFO = 5,
	OPTION_INFORM_EXCEPTION = 6,
	PRIMARY_NETWORK_REPORT = 7,
	IMS_APP_ABNORMAL = 8,
	TX_RX_STAT = 9,
	GAIN_REPORT = 10,
	REPORT_EVENT_TYPE_NUM_MAX
};

struct netlink_msg_head {
	u32 type;
	u32 data_len; /* the length of data + sizeof(netlink_msg_head) */
	char data[0];
};

#pragma pack(1)
struct compass_info {
	u32 compass_ip;
	u32 compass_port;
	u8 client_id[CLINET_ID_LEN];
	u8 app_id[APP_ID_LEN];
};
#pragma pack()

#pragma pack(1)
struct key_info {
	u8 hmac_id;
	u8 hmac_key[SIGN_KEY_LEN];
};
#pragma pack()

#pragma pack(1)
struct link_info {
	u8 pri_modem_permit;
	u8 sec_modem_permit;
	u8 pri_wifi_permit;
	u8 scene; /* 0 : WiFi + modem, 1: modem + modem */
};
#pragma pack()

struct app_collect {
	u32 data_len; /* the data[0] field length */
	char data[0];
};

struct app_info {
	u32 uid;
};

#pragma pack(1)
struct recv_init {
	struct compass_info compass;
	struct key_info key;
	struct link_info link;
	struct app_collect app;
};
#pragma pack()

struct recv_cloud_param_all {
	struct compass_info compass;
	struct key_info key;
	struct app_collect app;
};

struct recv_cloud_param_key {
	struct key_info key;
};

struct recv_cloud_param_compass {
	struct compass_info compass;
};

struct recv_cloud_param_app {
	struct app_collect app;
};

struct recv_cloud_param_link {
	struct link_info link;
};

struct tx_rx_ctl {
	u32 state;
};

struct recv_tx_rx_ctl {
	struct tx_rx_ctl tx_rx;
};

struct ims_monitor {
	u32 uid;
	u32 exception_timeout; /* exception timeout interval ms */
};

struct recv_ims_monitor_start {
	struct ims_monitor ims_monitor_param;
};

struct compass_available_monitor {
	u32 poll_timeout; /* poll timeout interval ms */
	u32 detect_interval; /* detect package tx interval ms */
	u32 exception_timeout; /* exception timeout interval ms */
	u32 exception_poll_timeout; /* when exception, poll timeout interval ms */
	u32 data_poll_timeout; /* when transmit data, poll timeout interval ms */
};

struct recv_compass_available_monitor_start {
	struct compass_available_monitor compass_monitor;
};

struct rept_init_result {
	u16 result;
	u16 reason;
};

struct rept_compass_status {
	u32 status; /* 0: normal 1: abnormal */
	u32 compass_ip;
};

struct rept_started {
	u32 status;
};

struct rept_stoped {
	u32 status;
};

struct rept_ip_conflict {
	u32 status;
	u32 conflict_type; /* 4 : ipv4 */
};

struct rept_tun_cfg_info {
	u32 status; /* 0 : down, 1 : up */
	u32 ip_address;
};

struct rept_primary_network {
	char primary_name[IFNAMSIZ];
	u8 length;
};

struct rept_ims_app_abnormal {
	u32 uid;
};

struct rept_gain_stat {
	u32 path;
	u32 gain;
};

#pragma pack(1)
struct rept_tx_rx_stat {
	u32 uid;
	u64 packet_rx_multi_path;
	u64 packet_rx_multi_path_gain;
	u64 traffic_tx;
	u64 traffic_tx_primary;
	u64 traffic_tx_secondary;
	u64 traffic_rx;
	u64 traffic_rx_primary;
	u64 traffic_rx_secondary;
};
#pragma pack()

void cloud_net_netlink_receive(const struct netlink_head *msg);
void cloud_net_register_report(netlink_event_report fn);
void netlink_report(const struct netlink_head *msg, enum exec_type type);
bool cloud_network_is_support(void);
#endif /* _CLOUD_NETWORK_H_ */
