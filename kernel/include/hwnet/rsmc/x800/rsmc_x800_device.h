/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This module is used to start the driver peripheral
 * and identify the peripheral chip type. The operations on smc_comm
 * are adapted step by step on the Acousto-Electronic BeiDou short packet
 * chip platform. Peripheral startup is controlled by the compilation
 * macro switch, Other peripheral startups depend on this module.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-22
 */

#ifndef RSMC_X800_DEVICE_H
#define RSMC_X800_DEVICE_H

#include <linux/mutex.h>
#include <linux/semaphore.h>

#include "rsmc_msg_loop.h"

#define X800_IC_NAME "rsmc"
#define RSMC_X800_DEV_NODE_NAME "rsmc-x800"
#define RSMC_SYNC_HEAD_LEN 40
#define RSMC_DATA_MAX_LEN 1024
#define RSMC_REG_NUM 20
#define RSMC_RF_NUM 20

struct enable_msg {
	struct msg_head head;
	int status;
	int est_sec;
	int mode;
	int reg_num;
	int reg_value[RSMC_REG_NUM];
	int rf_num;
	int rf_value[RSMC_RF_NUM];
};

struct mode_set_msg {
	struct msg_head head;
	int mode;
};

struct fd_msg {
	struct msg_head head;
	int est_sec;
};

struct smc_cnf_msg {
	struct msg_head head;
	s32 result;
};

struct tx_data_msg {
	struct msg_head head;
	u32 freq_point;
	u32 power;
	u32 power_change;
	u32 freqency;
	u32 m_phs;
	u32 m2_phs;
	u32 code_nco;
	u32 sync_len;
	u32 info_len;
	u32 frame_len;
	u32 obw;
	u32 obw_work;
	u32 rf_tx_pre_on;
	u32 rf_tx_aft_on;
	u32 power_unload;
};

struct freq_off_est_entry {
	struct msg_head head;
	int enable;
	u32 frequency;
	u32 power;
	int duration;
};

struct single_cmd_entry {
	struct msg_head msg;
	u32 addr;
	u32 value;
	u32 target;
	u32 opara;
};

int smc_set_init(struct enable_msg *msg);
void send_msg_to_ctrl(struct msg_head *msg);
void send_soc_err(void);
bool enable_tx_ant(bool enable);
void enable_rx_ant(bool enable);
bool cmd_is_block(void);
msg_process *x800_device_reg(notify_event *fun);
void x800_device_unreg(int reason);
void channel_freq_offset_proc(struct freq_off_est_entry *msg);

#endif /* RSMC_X800_DEVICE_H */

