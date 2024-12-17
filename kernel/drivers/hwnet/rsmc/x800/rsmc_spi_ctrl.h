/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#ifndef RSMC_SPI_CTRL_H
#define RSMC_SPI_CTRL_H

#include <linux/timer.h>
#include <linux/timex.h>

#include "rsmc_msg_loop.h"
#include "rsmc_rx_ctrl.h"
#include "rsmc_spi_drv.h"

#define RF_HZ 2491750000
#define CODE_HZ 8160000
#define CORR_FIFO_MAX_CNT 1024

#define PPS_COUNT 8
#define OFFSET_8PPS 16000

#ifdef RSMC_TXCO_26M
#define REF_CLK 26000000
#define OFFSET0_1PPS 13000
#define OFFSET1_1PPS 0
#define OFFSET2_1PPS 26000
#define XMS_COUNT 41U

// NCO_CTRL is 165.19104984615384615384615384615
#define CARR_NCO_FW0 (-702134315) // ((IF_HZ * NCO_CTRL))
#define CODE_NCO_FW0 1347958967 // ((CODE_HZ * NCO_CTRL))
#define CARR_NCO_COEF 50781 // NCO_CTRL/(2 * PI * 4096) = 1 / 50781
#define CODE_NCO_ZOOM 1024
#define CODE_NCO_COEF 208000000 // NCO_CTRL/(2 ^ 35) = 1 / 208000000
#else
#define REF_CLK 38400000
#define OFFSET0_1PPS 8192
#define OFFSET1_1PPS 2000
#define OFFSET2_1PPS 24000
#define XMS_COUNT 61U

// NCO_CTRL is 111.84810666666666666666666666667
#define CARR_NCO_FW0 (-475403443) // ((IF_HZ * NCO_CTRL))
#define CODE_NCO_FW0 912680550 // ((CODE_HZ * NCO_CTRL))
#define CARR_NCO_COEF 75000 // NCO_CTRL/(2 * PI * 4096) = 1 / 150000
#define CODE_NCO_ZOOM 1024
#define CODE_NCO_COEF 307200000 // NCO_CTRL/(2 ^ 35) = 1 / 307200000
#endif

#define PPS_NO 0
#define PPS_BEGIN 1
#define PPS_END 2

// chip mode
#define MODE_RX 0
#define MODE_TX 1

// default version no
#define VERSION 0x091700

#define SYMBOL_PER_MS 32
#define SPI_WORD_DATA_LEN 24

#define REG68_RF_OPEN 0x09339F
#define REG68_RF_CLOSE 0x79339F

#define IDX_0 0
#define IDX_1 1
#define IDX_2 2
#define IDX_3 3

#define TIMES_2 2
#define TIMES_3 3
#define TIMES_10 10

enum x800_spi_addr {
	ADDR_00_SYS_CFG_0 = 0x00,
	ADDR_01_SYS_CFG_1 = 0x01,
	ADDR_02_SYS_CFG_2 = 0x02,
	ADDR_03_SYS_CFG_3 = 0x03,
	ADDR_04_SYS_CTRL = 0x04,
	ADDR_05_SYS_STATUS = 0x05,
	ADDR_06_TIMER_CFG = 0x06,
	ADDR_07_TIMER_PARAM = 0x07,
	ADDR_08_MOD_CTRL = 0x08,
	ADDR_09_MOD_CODE_NCO_FW_H = 0x09,
	ADDR_0A_MOD_CODE_NCO_FW_L = 0x0A,
	ADDR_0B_MOD_CODE_PHS = 0x0B,
	ADDR_0C_MOD_INFO_BIT_LEN = 0x0C,
	ADDR_0D_MOD_FIFO_DATA = 0x0D,
	ADDR_0E_CORR_FIFO = 0x0E,
	ADDR_0F_VERSION = 0x0F,
	ADDR_RF_60 = 0x60,
	ADDR_RF_62 = 0x62,
	ADDR_RF_68 = 0x68,
	ADDR_RF_6C = 0x6C,
	ADDR_RF_6D = 0x6D,
	ADDR_RF_70 = 0x70,
	ADDR_RF_71 = 0x71,
	ADDR_RF_72 = 0x72,
	ADDR_RF_73 = 0x73,
	ADDR_RF_7E = 0x7E,
	ADDR_RF_7F = 0x7F
};

static inline u32 addr_10_chnl_en(u32 k)
{
	return (0x10 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_11_chnl_code_param(u32 k)
{
	return (0x11 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_12_chnl_cnt_init(u32 k)
{
	return (0x12 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_13_chnl_code_nco_fw(u32 k)
{
	return (0x13 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_14_chnl_code_nco_phs(u32 k)
{
	return (0x14 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_15_chnl_carr_nco_fw(u32 k)
{
	return (0x15 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_16_chnl_code_phs_lock(u32 k)
{
	return (0x16 + (k) * MAX_CHAN_NUM);
}

static inline u32 addr_17_chnl_code_cnt_lock(u32 k)
{
	return (0x17 + (k) * MAX_CHAN_NUM);
}

struct reg00_sys_cfg0_stru {
	u32 data_tx_mode_0 : 3;
	u32 data_tx_mode_1 : 3;
	u32 data_tx_mode_2 : 3;
	u32 data_tx_mode_3 : 3;
	u32 data_tx_mode_4 : 3;
	u32 data_tx_mode_5 : 3;
	u32 data_tx_mode_6 : 3;
	u32 data_tx_mode_7 : 3;
	u32 bit24_31 : 8;
};

struct reg01_sys_cfg1_stru {
	u32 corr_save_mode_0 : 1;
	u32 corr_save_mode_1 : 1;
	u32 corr_save_mode_2 : 1;
	u32 corr_save_mode_3 : 1;
	u32 corr_save_mode_4 : 1;
	u32 corr_save_mode_5 : 1;
	u32 corr_save_mode_6 : 1;
	u32 corr_save_mode_7 : 1;
	u32 pilot_tx_mode_0 : 1;
	u32 pilot_tx_mode_1 : 1;
	u32 pilot_tx_mode_2 : 1;
	u32 pilot_tx_mode_3 : 1;
	u32 pilot_tx_mode_4 : 1;
	u32 pilot_tx_mode_5 : 1;
	u32 pilot_tx_mode_6 : 1;
	u32 pilot_tx_mode_7 : 1;
	u32 bit16_23 : 8;
	u32 bit24_31 : 8;
};

struct reg02_sys_cfg2_stru {
	u32 mod_chip_num : 10;
	u32 mod_code_type : 2;
	u32 mod_ctrl_type : 1;
	u32 data_first : 1;
	u32 integral_64k : 1;
	u32 rf_on_polar : 1;
	u32 lna_on_polar : 1;
	u32 pa_on_polar : 1;
	u32 pps_polar : 1;
	u32 adc_edge : 1;
	u32 adc_convert_type : 2;
	u32 swap_iq : 2;
	u32 bit24_31 : 8;
};

struct reg03_sys_cfg3_stru {
	u32 m1_phs_init : 24;
	u32 bit24_31 : 8;
};

struct reg04_sys_ctrl_stru {
	u32 mod_start_loc : 1;
	u32 rst_n_mod_fifo : 1;
	u32 rst_n_corr_fifo : 1;
	u32 clear_corr_fifo_err : 1;
	u32 clear_mod_fifo_err : 1;
	u32 bit5_23 : 19;
	u32 bit24_31 : 8;
};

struct reg05_sys_status_stru {
	u32 mod_empty : 1;
	u32 mod_full : 1;
	u32 corr_empty : 1;
	u32 corr_full : 1;
	u32 mod_end : 1;
	u32 corr_err : 1;
	u32 mod_err : 1;
	u32 mod_cnt : 3;
	u32 corr_cnt : 10;
	u32 bit20_23 : 4;
	u32 bit24_31 : 8;
};

struct reg06_time_cfg_stru {
	u32 timer_pps_en : 1;
	u32 num_ppxms : 5;
	u32 timer_param : 15;
	u32 bit21_23 : 3;
	u32 bit24_31 : 8;
};

struct reg07_time_param_stru {
	u32 timer_ms : 5;
	u32 timer_xms : 8;
	u32 bit14_23 : 11;
	u32 bit24_31 : 8;
};

struct reg08_mod_ctrl_stru {
	u32 rf_on : 1;
	u32 lna_on : 1;
	u32 pa_on : 1;
	u32 bit3_7 : 5;
	u32 timer_param_pps : 15;
	u32 bit24_31 : 8;
};

struct reg09_mod_code_nco_h8_stru {
	u32 mod_code_nco_fw : 8;
	u32 mod_sync_bit_len : 16;
	u32 bit24_31 : 8;
};

struct reg0a_mod_code_nco_l24_stru {
	u32 mod_code_nco_fw : 24;
	u32 bit24_31 : 8;
};

struct reg0b_mod_code_mphs_stru {
	u32 mod_code_mphs : 24;
	u32 bit24_31 : 8;
};

struct reg0c_mod_info_bit_len_stru {
	u32 mod_info_bit_len : 20;
	u32 bit20_23 : 4;
	u32 bit24_31 : 8;
};

struct reg0d_mod_info_fifo_stru {
	u32 mod_fifo_din : 24;
	u32 bit24_31 : 8;
};

struct reg0e_corr_p_stru {
	u32 imag : 10;
	u32 real : 10;
	u32 flag : 1;
	u32 chn_idx : 3;
	u32 bit24_31 : 8;
};

struct reg0e_corr_d_stru {
	u32 data3 : 5;
	u32 data2 : 5;
	u32 data1 : 5;
	u32 data0 : 5;
	u32 flag : 1;
	u32 chn_idx : 3;
	u32 bit24_31 : 8;
};

struct reg0f_version_stru {
	u32 version : 24;
};

struct reg10_channel_enable_stru {
	u32 chnl_en : 1;
	u32 bit1_23 : 23;
	u32 bit24_31 : 8;
};

struct reg11_channel_code_para_stru {
	u32 phs_data : 10;
	u32 phs_pilot : 13;
	u32 bit23 : 1;
	u32 bit24_31 : 8;
};

struct reg12_channel_code_init_stru {
	u32 chnl_cnt_init : 13;
	u32 chnl_carr_nco_fw_h6 : 6;
	u32 bit19_23 : 5;
	u32 bit24_31 : 8;
};

struct reg13_channel_code_nco_stru {
	u32 chnl_code_nco_fw : 24;
	u32 bit24_31 : 8;
};

struct reg14_channel_code_nco_phs_stru {
	u32 phs_corr_ve : 2;
	u32 phs_corr_e : 2;
	u32 phs_corr_p : 2;
	u32 phs_corr_l : 2;
	u32 phs_corr_vl : 2;
	u32 chnl_code_nco_fw_h8 : 8;
	u32 bit18_23 : 6;
	u32 bit24_31 : 8;
};

struct reg15_channel_carr_nco_stru {
	u32 chnl_carr_nco_fw : 24;
	u32 bit24_31 : 8;
};

struct reg16_channel_code_phs_lock_stru {
	u32 chnl_code_phs_lock : 24;
	u32 bit24_31 : 8;
};

struct reg17_channel_code_cnt_lock_stru {
	u32 chnl_code_cnt_lock : 13;
	u32 bit13_bit23 : 11;
	u32 bit24_31 : 8;
};

struct reg6c_stru {
	u32 reserved_2_0 : 3;
	u32 afc_start : 1;
	u32 reserved_6_4 : 3;
	u32 rst_spi_sd : 1;
	u32 fil_div : 2;
	u32 na : 1;
	u32 fdiv : 11;
	u32 rdiv : 2;
	u32 bit24_31 : 8;
};

struct reg70_stru {
	u32 reserved_0 : 1;
	u32 to_div : 2;
	u32 reserved_6_3 : 4;
	u32 cp : 2;
	u32 cs_vco : 5;
	u32 reserved_23_14 : 10;
	u32 bit24_31 : 8;
};

struct reg71_stru {
	u32 reserved_1_0 : 2;
	u32 afc_en : 1;
	u32 reserved_23_3 : 21;
	u32 bit24_31 : 8;
};

struct reg72_stru {
	u32 reserved_2_0 : 3;
	u32 afc_start : 1;
	u32 reserved_6_4 : 3;
	u32 rst_spi_sd : 1;
	u32 fil_div : 2;
	u32 na : 1;
	u32 fdiv : 11;
	u32 rdiv : 2;
	u32 bit24_31 : 8;
};

struct reg7e_stru {
	u32 txlpf_latch : 1;
	u32 bpf_latch : 1;
	u32 bpf_c_1 : 7;
	u32 ifpgagc : 6;
	u32 ifmix_gain : 1;
	u32 rfmin_gain : 4;
	u32 lna_gain : 1;
	u32 rx1_agc_lock : 1;
	u32 afc_ok : 1;
	u32 ldx : 1;
	u32 bit24_31 : 8;
};

struct reg7f_stru {
	u32 version : 7;
	u32 vco_cs : 5;
	u32 pa_on : 1;
	u32 lna_on : 1;
	u32 rf_on : 1;
	u32 ldo_c_sts : 1;
	u32 ldo_rf_sts : 1;
	u32 txlpf_r_tune : 7;
	u32 bit24_31 : 8;
};

struct local_fd_msg {
	struct msg_head mh;
	u32 valid;
	s32 fd;
};

void set_chip_mode(int mode);
void update_tx_power(u32 power);
void update_tx_freq(u32 sdop);
void rf_init(u32 *rf_value, u32 num);
void chip_init(u32  *reg_value, u32 num);
void clk_fd_est(u32 seconds);
void stop_clk_fd_est(void);
void smc_set_value(u32 addr, u32 value);
void smc_get_value(u32 addr, u32 *value);
bool chip_is_ready_retry(void);
bool chip_is_ready(void);
void wait_chip_ready(void);
void adjust_agc(void);
int wait_8pps(bool fd_est);
int wait_1pps(bool fd_est);
void spi_ctrl_init(void);
void spi_ctrl_deinit(void);

#endif

