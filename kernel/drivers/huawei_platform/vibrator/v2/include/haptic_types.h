/*
 * haptic_types.h
 *
 * code for vibrator
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#ifndef _HAPTIC_TYPES_H_
#define _HAPTIC_TYPES_H_

#include <linux/compiler_types.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/pm_wakeup.h>
#include <linux/regmap.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#define HAPTIC_READ_CHIPID_RETRIES		(5)
#define HAPTIC_READ_CHIPID_RETRY_DELAY		(2)
#define HAPTIC_SEQUENCER_SIZE			(8)

#define HAPTIC_SEQUENCER_LOOP_SIZE		(4)
#define HAPTIC_NAME_MAX (64)
#define HAPTIC_VBAT_REFER			(4200)
#define HAPTIC_VBAT_MIN				(3000)
#define HAPTIC_RAM_WORK_DELAY_INTERVAL		(8000)
#define HAPTIC_OSC_TRIM_PARAM			(50)
#define HAPTIC_OSC_CALI_ACCURACY		(24)
#define HAPTIC_OSC_CALI_MAX_LENGTH		(5100000)
#define HAPTIC_TRIG_NUM				(3)
#define HAPTIC_RAMDATA_RD_BUFFER_SIZE		(10)
#define HAPTIC_RAMDATA_WR_BUFFER_SIZE		(10)

#define HAPTIC_GLBRD_STATE_MASK			(15<<0)
#define HAPTIC_STATE_STANDBY			(0x00)
#define HAPTIC_STATE_RTP			(0x08)
#define HAPTIC_BIT_RESET			(0xAA)
#define HAPTIC_I2C_ADDR				(0x5A)

#define HAPTIC_CHECK_RAM_DATA
#define HAPTIC_READ_BIN_FLEXBALLY
#define HAPTIC_CHECK_QUALIFY
#define HAPTIC_CHARGING_IS_CALIB		1
#define HAPTIC_SEL_BOOST_CFG_ON			1
#define HAPTIC_CHARGING_CALIB_ID		123456
#define HAPTIC_NULL_WAVEFORM_ID			123451
#define HAPTIC_MIN_SPEC_CMD_ID			123450
#define HAPTIC_MAX_SPEC_CMD_ID			123460
#define HAPTIC_WAKE_LOCK_GAP			400

#define HAPTIC_F0_CALI_THER_H			(1700 + 150)
#define HAPTIC_F0_CALI_THER_L			(1700 - 150)

#define LONG_VIB_BOOST_EFFECT_ID                4
#define LONG_VIB_EFFECT_ID                      21
#define LONG_HAPTIC_RTP_MAX_ID                  4999
#define LONG_HAPTIC_RTP_MIN_ID                  1010

#define SHORT_HAPTIC_RAM_MAX_ID                 309
#define SHORT_HAPTIC_RTP_MAX_ID                 9999

#define SHORT_HAPTIC_AMP_DIV_COFF               10
#define LONG_TIME_AMP_DIV_COFF                  100
#define BASE_INDEX                              31

#define LONG_HAPTIC_RUNNING 			4253
#define LONG_MAX_AMP_CFG 			9
#define SHORT_MAX_AMP_CFG 			6

#define MAX_WRITE_BUF_LEN           		16

typedef struct led_classdev cdev_t;

enum haptic_read_write {
	HAPTIC_HAPTIC_CMD_READ_REG = 0,
	HAPTIC_HAPTIC_CMD_WRITE_REG = 1,
};

enum haptic_work_mode {
	HAPTIC_STANDBY_MODE = 0,
	HAPTIC_RAM_MODE = 1,
	HAPTIC_RTP_MODE = 2,
	HAPTIC_TRIG_MODE = 3,
	HAPTIC_CONT_MODE = 4,
	HAPTIC_RAM_LOOP_MODE = 5,
};

enum haptic_bst_mode {
	HAPTIC_BST_BYPASS_MODE = 0,
	HAPTIC_BST_BOOST_MODE = 1,
};

enum haptic_bst_pc {
	HAPTIC_BST_PC_L1 = 0,
	HAPTIC_BST_PC_L2 = 1,
};

enum haptic_activate_mode {
	HAPTIC_HAPTIC_ACTIVATE_RAM_MODE = 0,
	HAPTIC_HAPTIC_ACTIVATE_CONT_MODE = 1,
};

enum haptic_cont_vbat_comp_mode {
	HAPTIC_CONT_VBAT_SW_COMP_MODE = 0,
	HAPTIC_CONT_VBAT_HW_COMP_MODE = 1,
};

enum haptic_ram_vbat_comp_mode {
	HAPTIC_RAM_VBAT_COMP_DISABLE = 0,
	HAPTIC_RAM_VBAT_COMP_ENABLE = 1,
};

enum haptic_pwm_mode {
	HAPTIC_PWM_48K = 0,
	HAPTIC_PWM_24K = 1,
	HAPTIC_PWM_12K = 2,
};

enum haptic_cali_lra {
	HAPTIC_WRITE_ZERO = 0,
	HAPTIC_F0_CALI_LRA = 1,
	HAPTIC_OSC_CALI_LRA = 2,
};

enum haptic_rw_flag {
	HAPTIC_SEQ_WRITE = 0,
	HAPTIC_SEQ_READ = 1,
};

enum vib_mode_type {
	SHORT_VIB_RAM_MODE = 0,
	LONG_VIB_RAM_MODE = 1,
	RTP_VIB_MODE = 2,
	VIB_MODE_MAX,
};

/*********************************************************
 *
 * Struct Define
 *
 *********************************************************/
struct trig {
	uint8_t trig_brk;
	uint8_t trig_bst;
	uint8_t trig_level;
	uint8_t trig_polar;
	uint8_t pos_enable;
	uint8_t neg_enable;
	uint8_t pos_sequence;
	uint8_t neg_sequence;
};

struct haptic_ram {
	uint32_t len;
	uint32_t check_sum;
	uint32_t base_addr;
	uint8_t ram_num;
	uint8_t version;
	uint8_t ram_shift;
	uint8_t baseaddr_shift;
};

struct haptic_i2c_info {
	uint32_t flag;
	uint32_t reg_num;
	uint8_t *reg_data;
};

struct haptic_dts_info {
	/* COMMON */
	uint8_t mode;
	uint8_t max_bst_vol;
	uint8_t f0_cali_percent;
	uint8_t bst_vol_default;
	uint8_t duration_time[3];
	uint32_t f0_pre;
	int boost_fw;
	int boost_en;

	/* AW869XX AW8671X AW8692X */
	uint8_t d2s_gain;
	uint8_t brk_bst_md;
	uint8_t bst_vol_ram;
	uint8_t bst_vol_rtp;
	uint8_t cont_tset;
	uint8_t cont_drv1_lvl;
	uint8_t cont_drv2_lvl;
	uint8_t cont_wait_num;
	uint8_t cont_brk_time;
	uint8_t cont_bemf_set;
	uint8_t cont_brk_gain;
	uint8_t cont_drv1_time;
	uint8_t cont_drv2_time;
	uint8_t cont_drv_width;
	uint8_t cont_track_margin;
	uint8_t cont_bst_brk_gain;
	uint8_t trig_cfg[24];
	bool is_enabled_auto_bst;
	bool is_enabled_i2s;
	bool is_enabled_one_wire;
	bool is_enabled_lowpower_bst_config;
	bool is_enabled_richtap_core;
};

struct haptic_ops;
struct haptic {
	struct wakeup_source *ws;
	unsigned char wk_lock_flag;
	unsigned char f0_is_cali;

	struct regmap *regmap;

	/* AW869XX */
	bool i2s_config;
	bool rtp_inited;
	bool ram_inited;
	bool emergency_mode_flag;

	/* COMMON */
	uint8_t flags;
	uint8_t bst_pc;
	uint8_t play_mode;
	uint8_t auto_boost;
	uint8_t activate_mode;
	uint8_t ram_vbat_comp;
	uint8_t rtp_routine_on;
	uint8_t seq[HAPTIC_SEQUENCER_SIZE];
	uint8_t loop[HAPTIC_SEQUENCER_SIZE];

	int vmax;
	int gain;
	int rate;
	int width;
	int state;
	int index;
	int chipid;
	int irq_gpio;
	int duration;
	int amplitude;
	int reset_gpio;
	int rtp_idx;
	int bst_enable;
	int f0_val_err;

	uint32_t cust_boost_on;
	uint32_t f0;
	uint32_t lra;
	uint32_t vbat;
	uint32_t cont_f0;
	uint32_t rtp_cnt;
	uint32_t rtp_len;
	uint32_t f0_cali_data;
	uint32_t osc_cali_data;
	uint64_t microsecond;

	char rtp_name[HAPTIC_NAME_MAX];
	struct haptic_container *rtp;
	cdev_t vib_dev;
	struct device *dev;
	struct i2c_client *i2c;
	struct mutex lock;
	struct hrtimer timer;
	struct mutex rtp_lock;
	struct work_struct rtp_work;
	struct delayed_work ram_work;
	struct work_struct vibrator_work;
	struct haptic_ram ram;
	struct haptic_dts_info info;
	struct haptic_ops *hops;
	struct haptic_i2c_info i2c_info;
	struct trig trig[HAPTIC_TRIG_NUM];
	enum vib_mode_type effect_mode;
#ifdef AAC_RICHTAP_SUPPORT
	uint8_t *rtp_ptr;
	struct mmap_buf_format *start_buf;
	struct mmap_buf_format *curr_buf;
	struct work_struct richtap_rtp_work;
	int16_t pos;
	atomic_t richtap_rtp_mode;
#endif
};

struct haptic_container {
	int len;
	uint8_t data[];
};

struct haptic_ops {
	int (*read_chipid)(struct haptic *);
	int (*get_f0)(struct haptic *);
	int (*f0_cali)(struct haptic *haptic);
	int (*creat_node)(struct haptic *);
	int (*get_irq_state)(struct haptic *);
	int (*check_qualify)(struct haptic *);
	int (*juge_rtp_going)(struct haptic *);
	int (*container_update)(struct haptic *, struct haptic_container *);
	void (*play_stop)(struct haptic *);
	void (*get_vbat)(struct haptic *);
	void (*cont_config)(struct haptic *);
	void (*offset_cali)(struct haptic *);
	void (*read_f0)(struct haptic *);
	void (*play_go)(struct haptic *, bool);
	void (*ram_init)(struct haptic *, bool);
	void (*set_bst_peak_cur)(struct haptic *);
	void (*get_lra_resistance)(struct haptic *);
	void (*set_pwm)(struct haptic *, uint8_t);
	void (*set_gain)(struct haptic *, uint8_t);
	void (*enable_gain)(struct haptic *, bool);
	void (*play_mode)(struct haptic *, uint8_t);
	void (*set_bst_vol)(struct haptic *, uint8_t);
	void (*set_repeat_seq)(struct haptic *, uint8_t);
	void (*auto_bst_enable)(struct haptic *, uint8_t);
	void (*vbat_mode_config)(struct haptic *, uint8_t);
	void (*set_wav_seq)(struct haptic *, uint8_t, uint8_t);
	void (*set_wav_loop)(struct haptic *, uint8_t, uint8_t);
	void (*set_rtp_data)(struct haptic *, uint8_t *, uint32_t);
	void (*protect_config)(struct haptic *, uint8_t, uint8_t);
	void (*parse_dt)(struct haptic *, struct device_node *);
	void (*trig_init)(struct haptic *);
	void (*irq_clear)(struct haptic *);
	void (*set_ram_addr)(struct haptic *);
	void (*misc_para_init)(struct haptic *);
	void (*interrupt_setup)(struct haptic *);
	void (*set_rtp_aei)(struct haptic *, bool);
	void (*upload_lra)(struct haptic *, uint32_t);
	void (*get_wav_seq)(struct haptic *, uint32_t);
	void (*bst_mode_config)(struct haptic *, uint8_t);
	void (*get_first_wave_addr)(struct haptic *, uint8_t *);
	size_t (*get_wav_loop)(struct haptic *, char *);
	ssize_t (*get_ram_data)(struct haptic *, char *);
	ssize_t (*get_reg)(struct haptic *, ssize_t, char *);
	ssize_t (*set_reg)(struct haptic *, uint8_t, uint8_t);
	uint8_t (*get_prctmode)(struct haptic *);
	uint8_t (*get_trim_lra)(struct haptic *);
	uint8_t (*get_glb_state)(struct haptic *);
	uint8_t (*rtp_get_fifo_afs)(struct haptic *);
	uint8_t (*rtp_get_fifo_afi)(struct haptic *);
	const char *(*get_ram_name)(void);

	struct list_head list;
};

/*********************************************************
 *
 * Function Call
 *
 *********************************************************/
int register_haptic_ops(struct haptic_ops *hops);

struct haptic *dev_get_haptic(struct device *dev);

#endif