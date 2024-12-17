/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file is the interface file of other modules.
 * Author: linlixin2@huawei.com
 * Create: 2020-10-19
 */

#ifndef RSMC_MSG_LOOP_H
#define RSMC_MSG_LOOP_H

#include <linux/types.h>

/* Notification request issued by the upper layer is defined as: */
struct msg_head {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	u32 module;
};

/*
 * This enumeration type is the netlink command types issued by the JNI.
 * Mainly to maintain the channel with JNI.
 */
enum nl_cmd_type {
	NL_MSG_REG = 0,
	NL_MSG_REQ,
	NL_MSG_HEARTBEAT,
	NB_MSG_REQ_BUTT
};

/* down enumeration message after this enumeration value. */
enum msg_type_dn {
	CMD_DN_INIT_REQ = 0,
	CMD_DN_MODE_SET_REQ = 1,
	CMD_DN_FREQ_OFFSET_EST_REQ = 2,
	CMD_DN_TX_REQ = 3,
	CMD_DN_CHN_INIT_REQ = 4,
	CMD_DN_CHN_CLOSE_REQ = 5,
	CMD_DN_CHN_ACQ2TRK_REQ = 6,
	CMD_DN_CHN_TRK_ADJUST_REQ = 7,
	CMD_DN_SINGLE_CMD_REQ = 8,
	CMD_DN_FREQ_OFF_REQ = 9,
	CMD_DN_MSG_NUM
};

/* kernel internal enumeration message after this enumeration value */
enum msg_type_inter {
	CMD_INTER_INIT_REQ = 2000,
	CMD_INTER_HB_TIMER_REQ = 2001,
	CMD_INT_MSG_NUM,
};

/* up enumeration message after this enumeration value. */
enum msg_type_up {
	CMD_UP_INIT_CNF = 1000,
	CMD_UP_MODE_SET_CNF,
	CMD_UP_TX_CNF,
	CMD_UP_CHN_INIT_CNF,
	CMD_UP_CHN_CLOSE_CNF,
	CMD_UP_RX_IND,
	CMD_UP_ACQ_IND,
	CMD_UP_TRK_IND,
	CMD_UP_LOCAL_FD_IND,
	CMD_UP_FREQ_OFF_CNF,
	CMD_UP_SOC_ERR_IND,
	CMD_UP_MSG_NUM
};

/* slow Message send function */
typedef void notify_event(struct msg_head *msg);
typedef void msg_process(struct msg_head *req);
typedef msg_process* model_reg(notify_event *fun);
typedef void model_unreg(int reason);
/* fast Message send function */
void fast_notify_event(struct list_head *list);
void rsmc_clear_dn_msg_list(void);
void rsmc_start_heartbeat(void);
void rsmc_restart_heartbeat(void);
void rsmc_stop_heartbeat(void);
int rsmc_init_x800(void);
void rsmc_exit_x800(void);

#endif /* RSMC_MSG_LOOP_H */

