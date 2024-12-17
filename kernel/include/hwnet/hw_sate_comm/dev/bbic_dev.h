/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: This module is used to start the driver peripheral
 * and identify the peripheral chip type.
 * Author: lizhenzhou1@huawei.com
 * Create: 2022-11-26
 */

#ifndef __BBIC_DEV_H__
#define __BBIC_DEV_H__

#include <linux/platform_device.h>
#include <linux/tty.h>

#define DTS_BBIC_COMP_NAME "huawei_sate"
#define SIM0_IDX 0
#define SIM1_IDX 1

// BBIC chip type
enum {
	BBIC_CHIP_TYPE_HLCT,
	BBIC_CHIP_TYPE_WS,
};

enum {
	BBIC_LOAD_FROM_USB,
	BBIC_LOAD_FROM_UART,
	BBIC_BOOT_NORMAL
};

enum {
	BBIC_SET_SIM_SOC_MODMEM, // SIM switch to modem
	BBIC_SET_SIM_BBIC // SIM switch to BBIC
};

// BBIC sim io
enum {
	BBIC_SIM_VCC_1V8,
	BBIC_SIM_VCC_3V
};

// sleep gpio direction switch,0->initila status,1->all input
enum {
	BBIC_SLEEP_GPIO_SWITCH_TO_NORMAL,
	BBIC_SLEEP_GPIO_SWITCH_TO_INPUT
};

// sleep gpio status query, 0->all low,1->all high,2->other
enum {
	BBIC_SLEEP_GPIO_ALL_LOW_STATUS,
	BBIC_SLEEP_GPIO_ALL_HIGH_STATUS,
	BBIC_SLEEP_GPIO_INVALID_STATUS
};

struct ps_device {
	char *dev_node_name;
	struct platform_device *pm_pdev;
	struct ps_core_s *ps_core;
};

struct ps_core_s {
	void *ps_plat;
	struct platform_device *pm_pdev;
	struct ps_device *hlct_dev;
	struct device *dev;
	struct device_node *dev_node;

	int32_t ap_wakeup_bb;
	int32_t bb_wakeup_ap;

	int32_t bb_reset;

	/* mode0:0, mode3:0 -> USB; mode0:1, mode3:0 -> UART0; mode0:0, mode3:1 -> NORMAL(NAND FLASH) */
	int32_t bb_boot_mode0;
	int32_t bb_boot_mode3;

	int32_t usim0_card_ldo_vol; // sim0 ldo11 vcc
	int32_t usim1_card_ldo_vol; // sim1 ldo16 vcc

	int32_t usim0_switch;
	int32_t usim1_switch;
	int32_t usim0_ldo_num; // LDO53 1v8/3v
	int32_t usim0_ldo_vol_1v8;
	int32_t usim0_ldo_vol_3v;
	int32_t pwr_en; // GPIO_338_MCAM_BUCK
	int32_t core_ldo_num; // LDO1 HL : 1v1
	int32_t core_ldo_vol;
	int32_t ls_ldo_num; // LDO22 HL : 1v2
	int32_t ls_ldo_vol;
	int32_t usb_ls_pwr_en; // XGPIO_001
	int32_t usb_vbst_pwr_en;

	int32_t ws_vbst_5v_en; // GPIO_253
	int32_t ws_core_en; // XGPIO_000

	int32_t bds_bst_en_gpio; // GRL XGPIO003
	
	int32_t ant_vc1; // GPIO_351, this GPIO may inited in GG/PGY
	int32_t ant_vc2; // GPIO_332, this GPIO may inited in GG/PGY

	int32_t bds_mos_en;   // GPIO204
	int32_t buck_boost_volt; // buck boost volt

	int32_t ant_sw1_vc1; // GPIO_219, this GPIO may inited in GG/PGY
	int32_t vbuck3_vol; // vbuck3 for HL PGY ON 2.01V
	int32_t vbuck3_off; // vbuck3 for HL PGY OFF 1.95V

	int32_t vdd_buckboost_vol; // bbic_vdd_buckboost
};

int32_t hw_bbic_dev_init(void);
void hw_bbic_dev_exit(void);
int pwr_gpio_ctrl(bool status, bool is_need_set_bootmode);
void set_bbic_chip_type(int chip_type);
void set_bbic_pwr_status(bool sta);
bool get_bbic_pwr_status(void);
int32_t set_boot_mode(int boot_mode);
int32_t hw_set_usim_switch(int sim_slot, int gpio_status);
int32_t hw_set_sim_switch_modem_to_bbic(int sim_idx);
int32_t hw_set_sim_switch_bbic_to_modem(int sim_idx);
int32_t hw_bbic_sleep_switch(int status);
int32_t hw_bbic_sleep_gpio_status_query(void);
int32_t hw_sim_vcc_switch_proc(int vcc_idx);

#endif
