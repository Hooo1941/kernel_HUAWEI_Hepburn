/* SPDX-License-Identifier: GPL-2.0 */
/*
 * rt9426.h
 *
 * driver for rt9426 battery fuel gauge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _RT9426_H_
#define _RT9426_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/coul/coul_calibration.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/hardware_ic/ground_loop_compensate.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_control.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif
#ifdef CONFIG_COUL_DRV
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/battery/battery_fault.h>
#include "rt9426_kirin.h"
#endif /* CONFIG_COUL_DRV */

#define RT9426_DRIVER_VER               0x0007

#define RT9426_UNSEAL_KEY               0x12345678
#define RT9426_CALIB_UNLOCK_KEY         5526789

#define RT9426_REG_DEVICE_ID            0x0E
#define RT9426_REG_CNTL                 0x00
#define RT9426_REG_RSVD_FLAG            0x02
#define RT9426_REG_CURR                 0x04
#define RT9426_REG_TEMP                 0x06
#define RT9426_REG_VBAT                 0x08
#define RT9426_REG_FLAG1                0x0A
#define RT9426_REG_FLAG2                0x0C
#define RT9426_REG_RM                   0x10
#define RT9426_REG_FCC                  0x12
#define RT9426_REG_AI                   0x14
#define RT9426_REG_TTE                  0x16
#define RT9426_REG_DUMMY                0x1E
#define RT9426_REG_VER                  0x20
#define RT9426_REG_VGCOMP12             0x24
#define RT9426_REG_VGCOMP34             0x26
#define RT9426_REG_INTT                 0x28
#define RT9426_REG_CYC                  0x2A
#define RT9426_REG_SOC                  0x2C
#define RT9426_REG_SOH                  0x2E
#define RT9426_REG_FLAG3                0x30
#define RT9426_REG_IRQ                  0x36
#define RT9426_REG_DSNCAP               0x3C
#define RT9426_REG_ADV                  0x3A
#define RT9426_REG_EXTREGCNTL           0x3E
#define RT9426_REG_SWINDOW1             0x40
#define RT9426_REG_SWINDOW2             0x42
#define RT9426_REG_SWINDOW3             0x44
#define RT9426_REG_SWINDOW4             0x46
#define RT9426_REG_SWINDOW5             0x48
#define RT9426_REG_SWINDOW6             0x4A
#define RT9426_REG_SWINDOW7             0x4C
#define RT9426_REG_SWINDOW8             0x4E
#define RT9426_REG_SWINDOW9             0x50
#define RT9426_REG_SWINDOW10            0x52
#define RT9426_REG_SWINDOW11            0x54
#define RT9426_REG_SWINDOW12            0x56
#define RT9426_REG_SWINDOW13            0x58
#define RT9426_REG_SWINDOW14            0x5A
#define RT9426_REG_SWINDOW15            0x5C
#define RT9426_REG_SWINDOW16            0x5E
#define RT9426_REG_OCV                  0x62
#define RT9426_REG_AV                   0x64
#define RT9426_REG_AT                   0x66
#define RT9426_REG_UN_FLT_SOC           0x70
#define RT9426_EXIT_SLP_CMD             0x7400
#define RT9426_ENTR_SLP_CMD             0x74AA

#define RT9426_TL_TABLE_IN_WINDOW4      5
#define RT9426_TL_TABLE_IN_WINDOW1      8
#define RT9426_OP_CONFIG_IN_WINDOW1     5
#ifdef CONFIG_DEBUG_FS
enum {
	RT9426FG_SOC_OFFSET_SIZE,
	RT9426FG_SOC_OFFSET_DATA,
	RT9426FG_PARAM_LOCK,
	RT9426FG_OFFSET_IP_ORDER,
	RT9426FG_FIND_OFFSET_TEST,
	RT9426FG_PARAM_CHECK,
	RT9426FG_DENTRY_NR,
};
#endif /* CONFIG_DEBUG_FS */

#define RT9426_BATPRES_MASK             0x0040
#define RT9426_RI_MASK                  0x0100
#define RT9426_BATEXIST_FLAG_MASK       0x8000
#define RT9426_USR_TBL_USED_MASK        0x0800
#define RT9426_CSCOMP1_OCV_MASK         0x0300
#define RT9426_UNSEAL_MASK              0x0003
#define RT9426_UNSEAL_STATUS            0x0001

#define RT9426_SMOOTH_POLL              20
#define RT9426_NORMAL_POLL              30
#define RT9426_SOCALRT_MASK             0x20
#define RT9426_SOCL_SHFT                0
#define RT9426_SOCL_MASK                0x1F
#define RT9426_SOCL_MAX                 32
#define RT9426_SOCL_MIN                 1

#define RT9426_RDY_MASK                 0x0080

#define RT9426_UNSEAL_PASS              0
#define RT9426_UNSEAL_FAIL              1
#define RT9426_PAGE_0                   0
#define RT9426_PAGE_1                   1
#define RT9426_PAGE_2                   2
#define RT9426_PAGE_3                   3
#define RT9426_PAGE_4                   4
#define RT9426_PAGE_5                   5
#define RT9426_PAGE_6                   6
#define RT9426_PAGE_7                   7
#define RT9426_PAGE_8                   8
#define RT9426_PAGE_9                   9
#define RT9426_PAGE_10                  10
#define RT9426_PAGE_11                  11

#define RT9426_SENSE_R_2P5_MOHM         25 /* 2.5 mohm */
#define RT9426_SENSE_R_5_MOHM           50 /* 5 mohm */
#define RT9426_SENSE_R_10_MOHM          100 /* 10 mohm */
#define RT9426_SENSE_R_20_MOHM          200 /* 20 mohm */

#define RT9426_RS_SEL_MASK              0xFF3F
#define RT9426_RS_SEL_REG_00            0x0000 /* 2.5 mohm */
#define RT9426_RS_SEL_REG_01            0x0040 /* 5 mohm */
#define RT9426_RS_SEL_REG_10            0x0080 /* 10 mohm */
#define RT9426_RS_SEL_REG_11            0x00C0 /* 20 mohm */

#define RT9426_BAT_TEMP_VAL             250
#define RT9426_BAT_VOLT_VAL             3800
#define RT9426_BAT_CURR_VAL             1000
#define RT9426_DESIGN_CAP_VAL           2000
#define RT9426_DESIGN_FCC_VAL           2000

#define RT9426_CHG_CURR_VAL             500
#define RT9426_LOWTEMP_T_THR            (-50) /* -5oC */
#define RT9426_LOWTEMP_V_THR            3750 /* 3750mV */
#define RT9426_LOWTEMP_EDV_THR          0x64 /* 0x64=100 => 100*5+2500=3000mV */

#define RT9426_READ_PAGE_CMD            0x6500
#define RT9426_WRITE_PAGE_CMD           0x6550
#define RT9426_EXT_READ_CMD_PAGE_5      0x6505
#define RT9426_EXT_WRITE_CMD_PAGE_5     0x6555

#define RT9426_OP_CONFIG_SIZE           5
#define RT9426_OP_CONFIG_0_DEFAULT_VAL  0x9480
#define RT9426_OP_CONFIG_1_DEFAULT_VAL  0x3241
#define RT9426_OP_CONFIG_2_DEFAULT_VAL  0x3EFF
#define RT9426_OP_CONFIG_3_DEFAULT_VAL  0x2000
#define RT9426_OP_CONFIG_4_DEFAULT_VAL  0x2A7F

#define RT9426_OCV_DATA_TOTAL_SIZE      80
#define RT9426_TL_DATA_TOTAL_SIZE       16
#define RT9426_OCV_ROW_SIZE             10
#define RT9426_TL_ROW_SIZE              2
#define RT9426_OCV_COL_SIZE             8
#define RT9426_BLOCK_PAGE_SIZE          8
#define RT9426_MAX_PARAMS               12
#define RT9426_WRITE_BUF_LEN            128
#define RT9426_SOC_OFFSET_SIZE          2
#define RT9426_SOC_OFFSET_DATA_SIZE     3
#define RT9426FG_OFFSET_IP_ORDER_SIZE   2
#define RT9426FG_FIND_OFFSET_TEST_SIZE  2
#define RT9426FG_PARAM_LOCK_SIZE        1
#define RT9426_DTSI_VER_SIZE            2
#define RT9426_DT_OFFSET_PARA_SIZE      3
#define RT9426_DT_EXTREG_PARA_SIZE      3
#define RT9426_OFFSET_INTERPLO_SIZE     2
#define RT9426_VOLT_SOURCE_NONE         0
#define RT9426_VOLT_SOURCE_VBAT         1
#define RT9426_VOLT_SOURCE_OCV          2
#define RT9426_VOLT_SOURCE_AV           3
#define RT9426_TEMP_SOURCE_NONE         0
#define RT9426_TEMP_SOURCE_TEMP         1
#define RT9426_TEMP_SOURCE_INIT         2
#define RT9426_TEMP_SOURCE_AT           3
#define RT9426_TEMP_ABR_LOW             (-400)
#define RT9426_TEMP_ABR_HIGH            800
#define RT9426_CURR_SOURCE_NONE         0
#define RT9426_CURR_SOURCE_CURR         1
#define RT9426_CURR_SOURCE_AI           2
#define RT9426_OTC_TTH_DEFAULT_VAL      0x0064
#define RT9426_OTD_ITH_DEFAULT_VAL      0x0064
#define RT9426_OTC_ITH_DEFAULT_VAL      0x0B5F
#define RT9426_OTD_DCHG_ITH_DEFAULT_VAL 0x0B5F
#define RT9426_UV_OV_DEFAULT_VAL        0x00FF
#define RT9426_CURR_DB_DEFAULT_VAL      0x0005
#define RT9426_FC_VTH_DEFAULT_VAL       0x0078
#define RT9426_FC_ITH_DEFAULT_VAL       0x000D
#define RT9426_FC_STH_DEFAULT_VAL       0x0046
#define RT9426_FD_VTH_DEFAULT_VAL       0x0091
#define RT9426_FD_SPD_VTH_DEFAULT_VAL   0x0005

#define RT9426_CALI_ENTR_CMD            0x0081
#define RT9426_CALI_EXIT_CMD            0x0080
#define RT9426_CURR_CONVERT_CMD         0x0009
#define RT9426_VOLT_CONVERT_CMD         0x008C
#define RT9426_CALI_MODE_MASK           0x1000
#define RT9426_SYS_TICK_ON_CMD          0xBBA1
#define RT9426_SYS_TICK_OFF_CMD         0xBBA0

#define RT9426_CALI_MODE_PASS           0
#define RT9426_CALI_MODE_FAIL           1

#define RT9426_SHDN_MASK                0x4000
#define RT9426_SHDN_ENTR_CMD            0x64AA
#define RT9426_SHDN_EXIT_CMD            0x6400

#define TA_IS_CONNECTED                 1
#define TA_IS_DISCONNECTED              0
#define RT9426_FD_TBL_IDX               4
#define RT9426_FD_DATA_IDX              10
#define RT9426_FD_BASE                  2500

#define RT9426_CURRENT_PRECISION        102
#define RT9426_CURRENT_PRECISION_BASE   100
#define RT9426_FFC_BUCK_PARA_A          2
#define RT9426_FFC_BUCK_PARA_B          0
#define RT9426_FFC_DC_PARA_A            2
#define RT9426_FFC_DC_PARA_B            0
#define RT9426_NON_FFC_PARA_A           10
#define RT9426_NON_FFC_PARA_B           50
#define RT9426_FC_DELTA_ITH_FOR_FFC_SPD 200
#define RT9426_FC_DELTA_ITH_FOR_NON_FFC_SPD 50
#define RT9426_FC_DELTA_ITH_FOR_SPD_TH  500

#define RT9426_HIGH_BYTE_MASK           0xFF00
#define RT9426_LOW_BYTE_MASK            0x00FF
#define RT9426_BYTE_BITS                8

#define RT9426_VTERM_INCREASE           50
#define RT9426_MAX_VTERM_SIZE           15
#define RT9426_REG_OCV_WRITE            0xCB00
#define RT9426_REG_OCV_COL_ADDRESS      0x0040
#define RT9426_OCV_TABLE_FIRST          0x0013
#define RT9426_OCV_TABLE_IN_OCV_COL     5

#define RT9426_T_V_ARRAY_LEN            27
#define RT9426_FC_LOW_CURR_VTH          0
#define RT9426_FC_LOW_CURR_CNT_TH       1
#define RT9426_FC_LOW_CURR_VDELTA       30
#define RT9426_FC_LOW_CURR_IDELTA       20
#define RT9426_ITERM_RATIO_BASE         100
/* FC_VTH = 3600mv + 5mv * x */
#define RT9426_FC_VTH(x)                ((x) * 5 + 3600)
#define RT9426_WRITE_TL_PASS            0
#define RT9426_WRITE_TL_FAIL            (-1)
#define RT9426_FCC_RATIO_BASE           100

#define PRECISION_ENHANCE               5
#define RT9426_GAIN_DEFAULT_VAL         128
#define RT9426_GAIN_BASE_VAL            512
#define RT9426_COUL_DEFAULT_VAL         1000000
#define RT9426_TBATICAL_MIN_A           752000
#define RT9426_TBATICAL_MAX_A           1246000
#define RT9426_TBATCAL_MIN_A            752000
#define RT9426_TBATCAL_MAX_A            1246000
#define RT9426_TBATICAL_MIN_B           0
#define RT9426_TBATICAL_MAX_B           256000
#define RT9426_EXTEND_REG               0x78
#define RT9426_FULL_CAPCACITY           100
#define RT9426_CAPACITY_TH              7
#define LV_SMOOTH_V_TH                  3250   /* 3250mV */
#define LV_SMOOTH_S_TH                  4      /* 4% */
#define LV_SMOOTH_T_MIN                 100    /* 10oC */
#define LV_SMOOTH_T_MAX                 500    /* 50oC */
#define LV_SMOOTH_I_TH                  (-400) /* -400*2.5 = -1000 mA */
#define CC_THRESHOLD                    450
#define NODE_NAME_MAX                   20

#define RT9426_BATT_NAME_LEN             32
#define RT9426_COLD_TEMP                (-200)
#define RT9426_OVERHEAT_TEMP            600
#define RT9426_TEMP_UNKNOWN             (-400)

struct rt9426_fc_setting {
	int fc_th;
	int fc_ith;
	int fc_vth;
	int fc_ith_spd;
	int ffc_charge_flag;
};

struct rt9426_display_data {
	int temp;
	int vbat;
	int ibat;
	int avg_ibat;
	int rm;
	int soc;
	int fcc;
};

enum {
	RT9426_DISPLAY_TEMP = 0,
	RT9426_DISPLAY_VBAT,
	RT9426_DISPLAY_IBAT,
	RT9426_DISPLAY_AVG_IBAT,
	RT9426_DISPLAY_RM,
	RT9426_DISPLAY_SOC,
	RT9426_DISPLAY_DISIGN_FCC,
	RT9426_DISPLAY_FCC,
	RT9426_DISPLAY_TTE,
	RT9426_DISPLAY_DC,
};

enum {
	RT9426_IC_TYPE_MAIN = 0,
	RT9426_IC_TYPE_AUX,
	RT9426_IC_TYPE_MAX,
};

enum {
	RT9426_LR_SLOW = 0,
	RT9426_LR_FAST,
	RT9426_LR_MAX,
};

enum {
	BAT_CONNECT_1S1P = 0,
	BAT_CONNECT_1S2P,
	BAT_CONNECT_2S1P,
};

struct data_point {
	union {
		int x;
		int voltage;
		int soc;
	};
	union {
		int y;
		int temperature;
	};
	union {
		int z;
		int curr;
	};
	union {
		int w;
		int offset;
	};
};

struct extreg_data_point {
	union {
		int extreg_page;
	};
	union {
		int extreg_addr;
	};
	union {
		int extreg_data;
	};
};

struct extreg_update_table {
	struct extreg_data_point *extreg_update_data;
};

struct submask_condition {
	int x, y, z;
	int order_x, order_y, order_z;
	int xnr, ynr, znr;
	const struct data_point *mesh_src;
};

struct soc_offset_table {
	int soc_voltnr;
	int tempnr;
	struct data_point *soc_offset_data;
};

enum comp_offset_typs {
	FG_COMP = 0,
	SOC_OFFSET,
	EXTREG_UPDATE,
};

enum { /* temperature source table */
	RT9426_TEMP_FROM_AP,
	RT9426_TEMP_FROM_IC,
};

struct fg_ocv_table {
	int data[RT9426_OCV_COL_SIZE];
};

struct fg_tl_table {
	int data[RT9426_OCV_COL_SIZE];
};

struct rt9426_platform_data {
	int soc_offset_size[RT9426_SOC_OFFSET_SIZE];
	struct soc_offset_table soc_offset;
	int extreg_size;
	struct extreg_update_table extreg_update;
	int offset_interpolation_order[RT9426_OFFSET_INTERPLO_SIZE];
	struct fg_ocv_table ocv_table[RT9426_MAX_VTERM_SIZE * RT9426_OCV_ROW_SIZE];
	struct fg_tl_table tl_table[RT9426_MAX_VTERM_SIZE * RT9426_TL_ROW_SIZE];
	int ocv_vterm_size;
	char batt_name[RT9426_BATT_NAME_LEN];

	int fc_delta_ith_for_non_ffc_spd; /* setting for speed window */
	int fc_delta_ith_for_ffc_spd;
	int fc_delta_ith_for_spd_th;
	int ffc_buck_para_A;
	int ffc_buck_para_B;
	int ffc_dc_para_A;
	int ffc_dc_para_B;
	int non_ffc_para_A;
	int non_ffc_para_B;
	int iterm_ratio;
	int ffc_vterm; /* hex for register */
	int non_ffc_vterm; /* hex for register */
	int dynamic_change_iterm_en;
	int fc_low_curr_vdelta;
	int fc_low_curr_idelta;
	int fc_low_curr_cnt;
	int fc_low_curr_cnt_th;
	int fc_low_curr_vth[RT9426_MAX_VTERM_SIZE];
	int fc_low_curr_vth_non_ffc[RT9426_MAX_VTERM_SIZE];
	int fcc_lr_adjust_en;
	int fcc_lr_cycle_th;
	int fcc_lr_ratio_th;
	int fcc_lr_status;
	unsigned int cache_reg_temp_en;

	int boot_gpio;
	u32 intr_gpio;
	int chg_sts_gpio;
	int chg_inh_gpio;
	int chg_done_gpio;
	int design_capacity;
	int battery_type;
	u32 uv_ov_threshold;
	u32 us_threshold;
	u32 otc_tth;
	u32 otc_chg_ith;
	u32 otd_tth;
	u32 otd_dchg_ith;
	u32 curr_db;

	u32 dtsi_version[RT9426_DTSI_VER_SIZE];
	u32 op_config[RT9426_OP_CONFIG_SIZE];
	u32 fc_ith;
	u32 fc_sth;
	u32 fcc[RT9426_MAX_VTERM_SIZE];
	u32 fc_vth[RT9426_MAX_VTERM_SIZE];
	u32 non_ffc_fc_vth[RT9426_MAX_VTERM_SIZE];
	u32 fd_vth[RT9426_MAX_VTERM_SIZE];
	u32 fd_spd_vth[RT9426_MAX_VTERM_SIZE];
	u32 dnm_cfg_edv_disable;
	u32 vterm_th_arr[RT9426_MAX_VTERM_SIZE];
	u32 fc_vth_usb[RT9426_MAX_VTERM_SIZE];

	u32 temp_source;
	u32 volt_source;
	u32 curr_source;
	u32 ic_role;
	u32 disable_under_vol_irq;

	u16 ocv_checksum_ic;
	u16 ocv_checksum_dtsi;
	u32 rs_ic_setting; /* in unit of 0.01 Ohm */
	u32 rs_schematic; /* in unit of 0.01 Ohm */
	u32 rs_high_side;

	u32 rt_config_ver;
	bool force_use_aux_cali_para;
	int need_reset_version;
	int lowtemp_en;
	int lowtemp_edv_thr;
	int lowtemp_t_thr;

	struct glc_temp_comp_data glc_data;
	int ground_loop_comp_en;
	int ir_comp_en;
	int compensation_r;
	u32 platform_support;
	u32 connect_type;
	u32 vbat_max;
#ifdef CONFIG_COUL_DRV
	struct rt9426_id_volt_para id_volt_data[ID_VOLT_PARA_LEVEL];
#endif /* CONFIG_COUL_DRV */
	u32 adc_sample_type;
};

struct rt9426_cached_reg_data {
	int temp;
};

struct rt9426_chip {
	struct i2c_client *i2c;
	struct device *dev;
	struct rt9426_platform_data *pdata;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dir_dentry;
	struct dentry *file_dentries[RT9426FG_DENTRY_NR];
#endif /* CONFIG_DEBUG_FS */
	struct mutex var_lock;
	struct mutex update_lock;
	struct mutex retry_lock;
	struct notifier_block event_nb;
	struct notifier_block event_buck_nb;
	struct rt9426_cached_reg_data cached_reg_data;
	int alert_irq;
	int capacity;
	int soc_offset;
	u8 online : 1;
	int btemp;
	int bvolt;
	int bcurr;
	u16 ic_ver;
	u16 ocv_checksum_ic;
	u16 ocv_checksum_dtsi;
	bool calib_unlock;
	bool low_v_smooth_en;
	int rm;
	int avg_curr;
	int c_gain;
	int c_offset;
	int v_gain;
	int ocv_index;
	int lowtemp_edv;
	struct work_struct init_work;
#ifdef CONFIG_COUL_DRV
	int charge_status;
	struct chrg_para_lut *para_batt_data;
	struct power_supply *fg_psy;
#endif /* CONFIG_COUL_DRV */
};

int rt9426_get_display_data(struct rt9426_chip *chip, int index);
#ifdef CONFIG_COUL_DRV
struct rt9426_chip *rt9426_get_chip(int idx);
int rt9426_adapt_kirin_platform(struct rt9426_chip *chip);
#endif

#endif /* _RT9426_H_ */
