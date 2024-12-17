/*
  * haptic.h
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

#ifndef _HAPTIC_H_
#define _HAPTIC_H_

#include <linux/regmap.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <sound/control.h>
#include <sound/soc.h>

/*********************************************************
 *
 * Conditional Marco
 *
 *********************************************************/
#define SIH_PRIORITY_OPEN
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 4, 1)
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 1)
#define KERNEL_VERSION_49
#endif

#define SIH_HAPTIC_MAX_DEV_NUM				4
#define SIH_HAPTIC_MMAP_DEV_INDEX			0
#define SIH_HAPTIC_SEQUENCER_SIZE			8
#define SIH_HAPTIC_SEQUENCER_LOOP_SIZE			4
#define SIH_HAPTIC_MAX_GAIN				255
#define SIH_HAPTIC_GAIN_LIMIT				128
#define SIH_HAPTIC_REG_SEQLOOP_MAX			15
#define SIH_HAPTIC_SEQUENCER_GAIN_SIZE			4
#define SIH_HAPTIC_RAM_MAX_SIZE				(8192)
#define SIH_RAMDATA_RD_BUFFER_SIZE			(6144)
#define SIH_RAMDATA_WR_BUFFER_SIZE			(6144)
#define SIH_RAMDATA_WRITE_SIZE				(1024)
#define SIH_RAMDATA_READ_SIZE				(1024)
#define SIH_RAM_TIME_DELAU_INTERVAL_MS			0
#define SIH_PM_QOS_VALUE_VB				(400)
#define SIH_RTP_NAME_MAX				(64)
#define SIH_RTP_START_DEFAULT_THRES			0x05
#define SIH_TRIG_NUM					3
#define SIH_F0_PRE_VALUE				170
#define SIH_TRACKING_F0_PRE_VALUE			1700
#define SIH_DETECT_FIFO_SIZE				128
#define SIH_RAM_MAIN_LOOP_MAX_TIME			15
#define SIH_HAPTIC_COM_DRV_VIOL				0
#define SIH_HAPTIC_EXCEED_DRV_VAL			0
#define SIH_RTP_CYCLE_NUM				1024
#define SIH_RTP_FILE_MAX_NUM				10
#define SIH_RTP_ZERO_MAX_INTERVAL			20
#define SIH_RTP_FILE_FREQUENCY				170
#define SIH_HAPTIC_RESET_GPIO_INVALID			1
#define SIH_HAPTIC_RESET_GPIO_UNINVALID			0
#define SIH_HAPTIC_RESET_START_TIME			1000
#define SIH_HAPTIC_RESET_END_TIME			2000
#define SIH_HAPTIC_PWR_AWIT_START_TIME			3500
#define SIH_HAPTIC_PWR_AWIT_END_TIME			4000
#define SIH_F0_MAX_THRESHOLD				1800
#define SIH_F0_MIN_THRESHOLD				1600
#define SIH_F0_DETECT_TRY				2

#define HAPTIC_CHARGING_IS_CALIB			1
#define SIH_F0_FILE_PATH				"/data"
#define HAPTIC_F0_FILE_PATH_LEN				64


#define SIH_6887_DRV_VBOOST_MIN				6
#define SIH_6887_DRV_VBOOST_MAX				11
#define SIH_6887_DRV_VBOOST_COEFFICIENT			10

#define SIH_6164_DRV_VBOOST_MIN				6
#define SIH_6164_DRV_VBOOST_MAX				11
#define SIH_6164_DRV_VBOOST_COEFFICIENT			10

#ifdef TIMED_OUTPUT
#include <../../../drivers/staging/android/timed_output.h>
typedef struct timed_output_dev cdev_t;
#else
#include <../../../drivers/leds/leds.h>
typedef struct led_classdev cdev_t;
#endif

#define MAX_WRITE_BUF_LEN				16
#define HAPTIC_CHARGING_CALIB_ID			123456
#define HAPTIC_NULL_WAVEFORM_ID				123451
#define HAPTIC_MIN_SPEC_CMD_ID				123450
#define HAPTIC_MAX_SPEC_CMD_ID				123460
#define SI_SEL_BOOST_CFG_ON				1
#define SI_LONG_HAPTIC_RUNNING				4253
#define HAPTIC_WAKE_LOCK_GAP				400
#define RTP_WORK_HZ_12K					2
#define EFFECT_ID_MAX					100
#define EFFECT_MAX_SHORT_VIB_ID				10
#define EFFECT_SHORT_VIB_AVAIABLE			3
#define EFFECT_MAX_LONG_VIB_ID				20
#define LONG_VIB_BOOST_EFFECT_ID			4
#define LONG_VIB_EFFECT_ID				21
#define LONG_HAPTIC_RTP_MAX_ID				4999
#define LONG_HAPTIC_RTP_MIN_ID				1010
#define SHORT_HAPTIC_AMP_DIV_COFF			10
#define LONG_TIME_AMP_DIV_COFF				100
#define BASE_INDEX					31
#define SI_LONG_MAX_AMP_CFG				9
#define SI_SHORT_MAX_AMP_CFG				6
#define SHORT_HAPTIC_RAM_MAX_ID				309
#define SHORT_HAPTIC_RTP_MAX_ID				9999
#define HAPTIC_LM_LONG_VB_BASE_VAL			1000000
#define MAX_LONG_HAPTIC_AMP				1
#define ADJUST_GAIN_CAM_RESONANCES			112
#define THIRD_LONG_HAPTIC_AMP				3
#define AW_HAPTIC_IOCTL_MAGIC				'h'
#define AW_HAPTIC_SET_QUE_SEQ \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 1, struct aw_que_seq*)
#define AW_HAPTIC_SET_SEQ_LOOP \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 2, struct aw_seq_loop*)
#define AW_HAPTIC_PLAY_QUE_SEQ \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 3, unsigned int)
#define AW_HAPTIC_SET_BST_VOL \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 4, unsigned int)
#define AW_HAPTIC_SET_BST_PEAK_CUR \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 5, unsigned int)
#define AW_HAPTIC_SET_GAIN \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 6, unsigned int)
#define AW_HAPTIC_PLAY_REPEAT_SEQ \
	_IOWR(AW_HAPTIC_IOCTL_MAGIC, 7, unsigned int)
#define SI_RTP_NAME_MAX				(64)
/*********************************************************
 *
 * Common enumeration
 *
 *********************************************************/

enum si_haptic_read_write {
	SI_HAPTIC_CMD_READ_REG = 0,
	SI_HAPTIC_CMD_WRITE_REG = 1,
};

typedef enum sih_haptic_work_mode {
	SIH_HAPTIC_IDLE_MODE = 0,
	SIH_HAPTIC_RAM_MODE = 1,
	SIH_HAPTIC_RTP_MODE = 2,
	SIH_HAPTIC_TRIG_MODE = 3,
	SIH_HAPTIC_CONT_MODE = 4,
	SIH_HAPTIC_RAM_LOOP_MODE = 5,
} sih_haptic_work_mode_e;

typedef enum sih_haptic_state_status {
	SIH_HAPTIC_STANDBY_MODE = 0,
	SIH_HAPTIC_ACTIVE_MODE = 1,
} sih_haptic_state_status_e;

typedef enum sih_haptic_boost_mode {
	SIH_HAPTIC_BOOST_MODE_BYPASS = 0,
	SIH_HAPTIC_BOOST_MODE_ENABLE = 1,
} sih_haptic_boost_mode_e;

typedef enum sih_haptic_go_mode {
	SIH_HAPTIC_FO_TRACKING_GO = 0,
	SIH_HAPTIC_RAM_GO = 1,
	SIH_HAPTIC_RTP_GO = 2,
} sih_haptic_go_mode_e;

typedef enum sih_haptic_rw_type {
	SIH_BURST_WRITE = 0,
	SIH_BURST_READ = 1,
} sih_haptic_rw_type_e;

typedef enum sih_haptic_ram_vbat_comp_mode {
	SIH_RAM_VBAT_COMP_DISABLE = 0,
	SIH_RAM_VBAT_COMP_ENABLE = 1,
} sih_haptic_ram_vbat_comp_mode_e;

typedef enum sih_haptic_cali_lra {
	SIH_WRITE_ZERO = 0,
	SIH_F0_CALI_LRA = 1,
	SIH_OSC_CALI_LRA = 2,
} sih_haptic_cali_lra_e;

typedef enum sih_haptic_pwm_sample_rpt {
	SIH_SAMPLE_RPT_ONE_TIME = 0,
	SIH_SAMPLE_RPT_TWO_TIME = 1,
	SIH_SAMPLE_RPT_FOUR_TIME = 3,
} sih_haptic_sample_rpt_e;

#ifdef SIH_PRIORITY_OPEN
typedef enum broadcast_priority {
	SIH_BROADCAST_PRI_MIN = 0,
	SIH_BROADCAST_PRI_LOW,
	SIH_BROADCAST_PRI_MID,
	SIH_BROADCAST_PRI_HIGH,
	SIH_BROADCAST_PRI_MAX,
} sih_broadcast_priority_e;
#endif

typedef struct chip_support_attr {
	uint8_t auto_pvdd : 1;
	uint8_t auto_brake : 1;
} sih_chip_support_attr_t;

typedef enum chip_attr_flag {
	SIH_CHIP_ATTR_DISABLE = 0,
	SIH_CHIP_ATTR_ENABLE = 1,
} sih_chip_attr_flag_e;

typedef enum rtp_play_mode {
	SIH_RTP_LOCAL_PLAY = 0,
	SIH_RTP_DATA_PLAY = 1,
} sih_rtp_play_mode_e;

typedef enum aw_haptic_activate_mode {
	SI_HAPTIC_ACTIVATE_RAM_MODE = 0,
	SI_HAPTIC_ACTIVATE_CONT_MODE = 1,
} sih_activate_mode_e;

typedef enum z_vib_mode_type {
	SHORT_VIB_RAM_MODE = 0,
	LONG_VIB_RAM_MODE = 1,
	RTP_VIB_MODE = 2,
	VIB_MODE_MAX,
} sih_z_vib_mode_e;

/*********************************************************
 *
 * Common vibrator mode
 *
 *********************************************************/
typedef struct sih_chip_reg {
	uint32_t rw_type;
	uint32_t reg_num;
	uint8_t *reg_addr;
} sih_chip_reg_t;

typedef struct sih_chip_inter_para {
	unsigned char chip_id;
	int state;
	unsigned char play_mode;            /* actual state */
	unsigned char action_mode;          /* work state */
	unsigned char ram_wave;             /* wave param */
	int gain;
	int level;                          /* gain level */
	bool auto_pvdd_en;
	bool low_power;
	int duration;                       /* work timing */
#ifdef SIH_PRIORITY_OPEN
	sih_broadcast_priority_e cur_pri;   /* current priority */
#endif
	uint32_t interval_us;
	uint32_t drv_vboost;
	ktime_t kcur_time;
	ktime_t kpre_time;
	sih_chip_support_attr_t support_attr;
} sih_chip_inter_para_t;

typedef struct sih_rtp_para {
	struct work_struct rtp_work;
	struct work_struct rtp_data_work;
	struct mutex rtp_lock;
	bool rtp_init;
	uint32_t rtp_cnt_offset;
	uint32_t rtp_cnt;
	uint32_t rtp_file_num;
	uint32_t rtp_start_thres;
	uint32_t rtp_pre_mode;
	struct haptic_container *rtp_cont;
} sih_rtp_para_t;

typedef struct sih_ram_para {
	unsigned char ram_init;
	struct delayed_work ram_delayed_work;
	uint8_t seq[SIH_HAPTIC_SEQUENCER_SIZE];
	uint8_t loop[SIH_HAPTIC_SEQUENCER_SIZE];
	bool wav_delay[SIH_HAPTIC_SEQUENCER_SIZE];
	uint8_t gain[SIH_HAPTIC_SEQUENCER_SIZE];
	uint32_t external_loop;
	int index;
	uint32_t len;
	uint32_t check_sum;
	uint32_t base_addr;
	uint8_t ram_num;
	uint8_t version;
	uint8_t ram_shift;
	uint8_t baseaddr_shift;
	uint8_t ram_vbat_comp;
} sih_ram_para_t;

typedef struct sih_cont_para {
	uint8_t seq0_t_driver;
	uint8_t seq0_t_flush;
	uint8_t seq0_t_bemf;
	uint8_t seq1_t_driver;
	uint8_t seq1_t_flush;
	uint8_t seq1_t_bemf;
	uint8_t seq2_t_driver;
	uint8_t seq2_t_flush;
	uint8_t seq2_t_bemf;
	uint32_t a_smooth;
	uint32_t t_half_len;
	uint32_t t_half_num;
	uint32_t dst_ampli;
} sih_cont_para_t;

typedef struct sih_detect_para {
	uint32_t detect_time; // us
	bool trig_detect_en;
	bool ram_detect_en;
	bool rtp_detect_en;
	bool cont_detect_en;
	uint32_t detect_fifo_size;
	char *detect_fifo;
	uint32_t detect_drive_time;
	uint32_t *detect_bemf_value;
	uint32_t *detect_f0_value;
	uint32_t detect_f0;
	uint32_t tracking_f0;
	uint32_t detect_rl_rawdata;
	uint32_t detect_rl;
	uint32_t detect_rl_offset;
	uint32_t detect_vbat_rawdata;
	uint32_t detect_vbat;
	uint32_t cali_target_value;
} sih_detect_para_t;

typedef struct sih_brake_para {
	bool trig_brake_en;
	bool ram_brake_en;
	bool rtp_brake_en;
	bool cont_brake_en;
} sih_brake_para_t;

typedef struct sih_trig_para {
	uint8_t enable;
	uint8_t polar;
	uint8_t mode;
	uint8_t boost_bypass;
	uint8_t pose_id;
	uint8_t nege_id;
} sih_trig_para_t;

typedef struct sih_frame_core {
	cdev_t vib_dev;
} sih_frame_core_t;

typedef struct sih_chip_attr {
	int reset_gpio;
	int irq_gpio;
	unsigned int com_drv_viol;
	unsigned int exceed_drv_val;
} sih_chip_attr_t;

typedef struct sih_work_priority {
	sih_broadcast_priority_e ram;
	sih_broadcast_priority_e loopram;
	sih_broadcast_priority_e cont;
	sih_broadcast_priority_e trig;
	sih_broadcast_priority_e rtp;
} sih_work_priority_t;

typedef struct sih_osc_para {
	ktime_t kstart;
	ktime_t kend;
	uint32_t microsecond;
	uint32_t osc_data;
	uint32_t timeval_flag;
	uint32_t osc_rtp_len;
	uint32_t theory_time;
}sih_osc_para_t;

typedef struct sih_polar_cali_para {
	ktime_t kstart;
	ktime_t kend;
	uint32_t timeval_flag;
	uint32_t microsecond;
	uint32_t fifo_size;
	uint32_t bemf_value_pwm_0;
	uint32_t bemf_value_pwm_1;
	char *data_fifo;
	uint32_t *polar_bemf_value;
}sih_polar_cali_para_t;

typedef struct haptic_regmap {
	struct regmap *regmapping;
	const struct regmap_config *config;
} haptic_regmap_t;

typedef struct fileops {
	unsigned char cmd;
	unsigned char reg;
	unsigned char ram_addrh;
	unsigned char ram_addrl;
} sih_haptic_fileops_t;

typedef struct sih_haptic {
	struct i2c_client *i2c;
	struct device *dev;
	struct pinctrl *pinctrl;
	struct fileops fileops;
	struct pinctrl_state *pinctrl_state_active;
	struct mutex lock;
	struct hrtimer timer;
	struct work_struct ram_work;
	struct delayed_work delay_init_work;
	struct workqueue_struct *sih_wq;
	struct work_struct vibrator_work;
	ktime_t kcurrent_time;
	ktime_t kpre_enter_time;
	unsigned int interval_us;
	struct pm_qos_request pm_qos;
#ifdef SIH_PRIORITY_OPEN
	sih_work_priority_t register_pri;
#endif
	sih_osc_para_t osc_para;
	sih_chip_reg_t chip_reg;
	sih_chip_attr_t chip_attr;
	sih_chip_inter_para_t chip_ipara;
	sih_cont_para_t cont_para;
	sih_detect_para_t detect_para;
	sih_polar_cali_para_t polar_cali_para;
	sih_brake_para_t brake_para;
	sih_frame_core_t soft_frame;
	sih_ram_para_t ram_para;
	sih_rtp_para_t rtp_para;
	sih_trig_para_t trig_para[SIH_TRIG_NUM];
	struct haptic_func *hp_func;
	haptic_regmap_t regmapp;

	unsigned char f0_is_cali;
	uint8_t activate_mode;
	int duration;
	int index;
	sih_z_vib_mode_e effect_mode;
	uint32_t cust_boost_on;
	struct wakeup_source *ws;
	unsigned char wk_lock_flag;
	int amplitude;
	int rtp_idx;
	bool set_amplitude_flag;
} sih_haptic_t;

typedef struct haptic_container {
	int len;
	uint8_t data[];
} haptic_container_t;

typedef struct haptic_func {
	int (*match_func_list)(sih_haptic_t *);
	int (*get_interrupt_state)(sih_haptic_t *);
	int (*update_ram_config)(sih_haptic_t *, haptic_container_t *);
	void (*play_stop)(sih_haptic_t *);
	void (*get_vbat)(sih_haptic_t *);
	void (*update_cont_config)(sih_haptic_t *, uint8_t, uint32_t);
	void (*play_go)(sih_haptic_t *, bool);
	void (*ram_init)(sih_haptic_t *, bool);
	void (*get_lra_resistance)(sih_haptic_t *);
	void (*set_gain)(sih_haptic_t *, uint8_t);
	void (*set_play_mode)(sih_haptic_t *, uint8_t);
	void (*set_drv_bst_vol)(sih_haptic_t *, uint32_t);
	void (*set_repeat_seq)(sih_haptic_t *, uint8_t);
	void (*set_wav_seq)(sih_haptic_t *, uint8_t, uint8_t);
	void (*set_wav_internal_loop)(sih_haptic_t *, uint8_t, uint8_t);
	size_t (*write_rtp_data)(sih_haptic_t *, uint8_t *, uint32_t);
	void (*clear_interrupt_state)(sih_haptic_t *);
	void (*set_ram_addr)(sih_haptic_t *);
	void (*interrupt_state_init)(sih_haptic_t *);
	size_t (*get_trig_para)(sih_haptic_t *, char *);
	void (*set_rtp_aei)(sih_haptic_t *, bool);
	void (*upload_lra)(sih_haptic_t *, uint32_t);
	void (*get_wav_seq)(sih_haptic_t *, uint32_t);
	void (*get_first_wave_addr)(sih_haptic_t *, uint8_t *);
	size_t (*get_wav_internal_loop)(sih_haptic_t *, char *);
	ssize_t (*get_ram_data)(sih_haptic_t *, char *);
	ssize_t (*get_all_reg)(sih_haptic_t *, ssize_t, char *);
	uint8_t (*if_rtp_fifo_full)(sih_haptic_t *);
	void (*set_auto_pvdd)(sih_haptic_t *, bool);
	void (*set_wav_external_loop)(sih_haptic_t *, uint8_t);
	size_t (*get_wav_external_loop)(sih_haptic_t *, char *);
	void (*set_low_power_mode)(sih_haptic_t *, uint8_t);
	bool (*if_is_mode)(sih_haptic_t *, uint8_t);
	void (*set_trig_para)(sih_haptic_t *, uint32_t *);
	void (*set_ram_seq_gain)(sih_haptic_t *, uint8_t, uint8_t);
	size_t (*get_ram_seq_gain)(sih_haptic_t *, char *);
	void (*read_detect_fifo)(sih_haptic_t *);
	void (*init_chip_parameter)(sih_haptic_t *);
	void (*update_chip_state)(sih_haptic_t *);
	void (*f0_cali)(sih_haptic_t *);
	void (*trim_init)(sih_haptic_t *);
	void (*osc_trim_calibration)(sih_haptic_t *);
	void (*efuse_check)(sih_haptic_t *);
	int (*get_tracking_f0)(sih_haptic_t *);
} haptic_func_t;

/*********************************************************
 *
 * Function Call
 *
 *********************************************************/
#endif
