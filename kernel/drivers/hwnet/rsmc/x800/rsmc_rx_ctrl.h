/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#ifndef RSMC_RX_CTRL_H
#define RSMC_RX_CTRL_H

#include <linux/list.h>
#include <linux/spi/spi.h>

#include "rsmc_msg_loop.h"

#define MAX_CHAN_NUM 8
#define CORR_TYPE_PILOT 0x00
#define CORR_TYPE_DATA 0x01

#define MODE_IDLE 0
#define MODE_ACQ 1
#define MODE_TRK 2
#define MODE_TRANS 3

#define CORR_P_ONLY 0
#define CORR_D_AND_P 1

#define CORR_DATA_ACQ_MS 125
#define FRM_PERIOD 125
#define CORR_DATA_TRACK_MS 10

#define PHASE_ADJUST_NO_NEED 0
#define PHASE_ADJUST_NEED 1
#define PHASE_ADJUST_ALREADY 2

#define ACQ_CORR_NUM 5
#define ACQ_FFT_NUM 125

struct complex {
	s16 real;
	s16 imag;
};

struct acquisition_msg {
	struct list_head list;
	struct msg_head mh;
	u8 chn_idx;
	u8 ms_idx;
	u16 rsv;
	struct complex corr_buf[ACQ_CORR_NUM][ACQ_FFT_NUM];
};

#define MAX_TRACK_LEN 20
struct track_msg {
	struct list_head list;
	struct msg_head mh;
	u8 chn_idx;
	u8 ms_idx;
	u8 len;
	u8 rsv;
	struct complex e_tap[MAX_TRACK_LEN];
	struct complex p_tap[MAX_TRACK_LEN];
	struct complex l_tap[MAX_TRACK_LEN];
	u32 clkdrift_1s;
	u32 clkdrift_100ms;
};

struct s2c_d_data_msg {
	struct list_head list;
	struct msg_head mh;
	u8 chn_idx;
	s32 data_inverted; /* to indicate if nav data bits should be inverted */
	s8 data[4100]; // 125 * 8 * 4 = 4000
};

struct channel_init_param {
	u32 chn_idx; // 0 ~ MAX_CHAN_NUM-1
	u32 ms_idx; // 0~124
	u32 g2_initphase;
	u32 carr_nco_word;
	u32 code_nco_word;
	u32 chnl_cnt_init; // unit 0.25 chip
	u32 timeout;
};

struct rx_init_msg {
	struct msg_head mh;
	u32 chn_num;
	struct channel_init_param chn_init_param[MAX_CHAN_NUM];
	s8 magic_code[FRM_PERIOD];
};

struct acq2track_msg {
	struct msg_head mh;
	u8 chn_idx;
	u8 nxt_mode;
	u16 rsv;
	s32 delay;
	u32 carr_nco_word;
	u32 code_nco_word;
	s32 ms_diff;
	s64 doppler;
	s64 dither;
	s64 code_doppler;
	s64 code_dither;
};

struct track_adjust_msg {
	struct msg_head mh;
	u8 chn_idx;
	u8 rsv1;
	u16 rsv2;
	s32 ms_diff;
};

void rx_mode_set(int mode);
void rx_init(struct rx_init_msg *msg);
bool tx_init(void);
void chan_close(struct rx_init_msg *msg);
void acq2track(struct acq2track_msg *msg);
void update_nco_word(u8 chn_idx);
void track_adjust(u8 chn_idx, u32 carr_nco_word, u32 code_nco_word);
bool rx_loop_ready(void);
void wait_loop_exit(bool is_tx);
void update_ms_idx(struct track_adjust_msg *msg);
void enter_nxt_status(void);
void cmd_read(u32 addr);
void cmd_write(u32 addr, u32 value);
void send_chn_init_rsp(s32 result);
void send_chn_close_rsp(s32 result);

typedef u32 spi_loop_cb(u32 value);
#ifdef CONFIG_HW_RSMC_QCOM
extern void set_spi_loop_cb(spi_loop_cb *cb, struct spi_controller *master);
#endif

#endif

