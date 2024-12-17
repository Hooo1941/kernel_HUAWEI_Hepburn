/*
 * haptic_if.h
 *
 * code for vibrator
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef _HAPTIC_IF_H_
#define _HAPTIC_IF_H_
#include <linux/regmap.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <sound/control.h>
#include <sound/soc.h>

#include "cdv26xx_reg.h"

#define AAC_RICHTAP_SUPPORT
/* CHIPID */
#define CDV2625_CHIPID  0xc7

/* common macro */
#define CS_HAPTIC_DRIVER_VERSION            "v0.1.3"
#define CS_I2C_NAME                         "cs_haptic"
#define CS_HAPTIC_NAME                      "haptics"
#define CS_READ_CHIPID_RETRIES              (3)
#define CS_READ_CHIPID_RETRY_DELAY          (2)

#define CS_RAM_IDA                          0x3000
#define CS_RAM_IDB                          0x2000

#define CS_I2C_RETRY_TIMES                  3
#define CS_I2C_BYTE_ONE                     1
#define CS_MAX_SEQ_SIZE                     8
#define CS_T_NUM                            3
#define CS_RTP_NAME_MAX                     64

#define CS_VBAT_REFER                       (4200)
#define CS_VBAT_MIN                         (3000)
#define CS_VBAT_MAX                         (4500)
#define CS_SEQUENCER_LOOP_SIZE              (4)
#define CS_SEQUENCER_SIZE                   (8)
#define CS_DEFAULT_BOOST_VOL                0x4c

/* RTP single write data length. */
#define CDV2625_RTP_SINGLE_WRITE_LEN        512

/* RAM single write data length. */
#define CDV2625_RAM_SINGLE_WRITE_LEN        10
#define CS_RAMDATA_RD_BUFFER_SIZE           (1024)
#define CS_RAMLIB_BASEADDR_OFFSET           0x02
#define CS_RAMLIB_DATA_OFFSET               0x04

#define CS_READ_BIN_FLEXBALLY
#define CS_CHECK_QUALIFY

#define CS_RAM_WORK_DELAY_INTERVAL          (8000)
#define CS_BASE_ADDR                        0x0800

#define CHECK_RAM_DATA

#define F0_WAIT_TIME_MS                     500
#define DET_VDD_TIME_MS                     100
#define DET_RL_TIME_MS                      100
#define DET_OS_TIME_MS                      50
#define HAPTIC_GAIN_FOR_F0                  0x30
#define HAPTIC_BREAK_CYCLE                  0x05
#define HAPTIC_GAIN_FOR_F0                  0x30
#define HAPTIC_F0_OPEN_CYCLE                0x05
#define HAPTIC_F0_CHECK_TIMES               0x01
#define HAPTIC_LRA_CALIBRATION_TIMES        10
#define CS_HAPTIC_IOCTL_MAGIC               'h'

#define CS_LONG_HAPTIC_RUNNING              4253
#define CS_LONG_MAX_AMP_CFG                 9
#define CS_SHORT_MAX_AMP_CFG                6
#define ADJUST_GAIN_CAM_RESONANCES          112
#define MAX_LONG_HAPTIC_AMP                 1
#define THIRD_LONG_HAPTIC_AMP               3

#define HAPTIC_CHARGING_IS_CALIB            1
#define CS_SEL_BOOST_CFG_ON                 1
#define HAPTIC_CHARGING_CALIB_ID            123456
#define HAPTIC_NULL_WAVEFORM_ID             123451
#define HAPTIC_MIN_SPEC_CMD_ID              123450
#define HAPTIC_MAX_SPEC_CMD_ID              123460
#define HAPTIC_WAKE_LOCK_GAP                400

#define SHORT_HAPTIC_RAM_MAX_ID             309
#define SHORT_HAPTIC_RTP_MAX_ID             9999
#define HAPTIC_LM_LONG_VB_BASE_VAL          1000000
#define SHORT_HAPTIC_AMP_DIV_COFF           10
#define LONG_TIME_AMP_DIV_COFF              100
#define BASE_INDEX                          31
#define LONG_VIB_EFFECT_ID                  21
#define LONG_VIB_BOOST_EFFECT_ID            4
#define MAX_WRITE_BUF_LEN                   16
#define LONG_HAPTIC_RTP_MAX_ID              4999
#define LONG_HAPTIC_RTP_MIN_ID              1010
#define CS_INTO_STANDBY_RETRIES             (40)
#define DURATION_NOTIFY_TIME_LIMIT          650
#define SHORT_RAM_DURATION_INIT             200
#define HAPTIC_GAIN_MAX_ADAPT_LEVELS        100
#define HAPTIC_MAX_GAIN_REG                 0x80
#define FINE_GRAINED_AMP_DIV_COFF           1000
#define FINE_GRAINED_RAM_ID_BASE_VAL        5000
#define FINE_GRAINED_RAM_BASE               5000000

#define BST_CONFIG                   0x43
#define BST_REF_CONFIG               0x20

/* cont mode */
#define CONT_BEMF_STARTUP_TIME       0x27
#define CONT_BEMF_TIME               0x70
#define CONT_BRK_GAIN                0x02
#define CONT_BRK_THRESHOLD           0x20
#define CONT_BEMF_TRACK_TIME         0x10
#define LONG_RAM_MAX_BRK_VOL         0x30
#define SPECIAL_RAM_MAX_BRK_VOL      0x7f
#define F0_CLOSE_CYCLE               0x03
#define LOW_POWER_CFG                0x20
#define BST_CFG                      0x43
#define NO_PAD_DEGLITCH              0x70

/* F0 cali */
#define F0_CALI_STEP       64
#define F0_CALI_OSC_BASE   12288
#define F0_CALI_FLOAT      10
#define F0_CALI_FACTOR00   192
#define F0_CALI_FACTOR01   185
#define F0_CALI_FACTOR02   171
#define F0_CALI_FACTOR03   162
#define F0_CALI_FACTOR04   149
#define F0_CALI_FACTOR05   146
#define F0_CALI_FACTOR06   138
#define F0_CALI_FACTOR07   131
#define F0_CALI_FACTOR08   125

#define PROTECT_ENABLE     1
#define PROTECT_DISABLE    0
#define PROTECT_SET_VOL    2

#define F0_CALI_INDEX_MIN      3
#define F0_CALI_INDEX_MAX      11
#define F0_CALI_BASE00_OFFSET  15
#define F0_CALI_BASE01_OFFSET  47
#define F0_CALI_BASE02_OFFSET  15
#define F0_CALI_BASE03_OFFSET  31
#define F0_CALI_BASE04_OFFSET  15
#define F0_CALI_BASE05_OFFSET  80
#define F0_CALI_BASE06_OFFSET  15
#define F0_CALI_BASE07_OFFSET  31
#define F0_CALI_BASE08_OFFSET  15

#define F0_CONV_FACTOR         3846154
#define RAM_BOOST_VOL_9V       0x58
#define RTP_BOOST_VOL_6_5V     0x30

#define CS_HAPTIC_IOCTL_MAGIC          'h'
#define CS_GLBRD_STATE_MASK            (15 << 0)
#define CS_STATE_STANDBY               (0x00)
#define CS_STATE_RTP                   (0x05)

#define SPECIAL_LONG_RAM_GAIN_COMP     0x70
#define MAX_GAIN                       0xff

#define HAPTIC_RAM_BRK_DISABLE_ID_1    27

#define MS_TIME_TO_NS_TIME                      (1000 * 1000)
#define RTP_LEN_TO_PLAY_TIME_COFF               12
#define MAX_TRANSIENT_RTP_LEN                   300
#define RTP_PLAY_END_STABLE_TIME                3

/* Conditional Marco */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 4, 1)
#define TIMED_OUTPUT
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 1)
#define KERNEL_OVER_4_19
#endif

#ifdef TIMED_OUTPUT
#include <../../../drivers/staging/android/timed_output.h>
typedef struct timed_output_dev cdev_t;
#else
typedef struct led_classdev cdev_t;
#endif

#ifdef KERNEL_OVER_4_19
typedef struct snd_soc_component aw_snd_soc_codec_t;
typedef struct snd_soc_component_driver aw_snd_soc_codec_driver_t;
#else
typedef struct snd_soc_codec aw_snd_soc_codec_t;
typedef struct snd_soc_codec_driver aw_snd_soc_codec_driver_t;
#endif

/* LOG API */
#define log_err(format, ...) \
	pr_err("[cs_haptic][%04d]%s: " format "\n", __LINE__, __func__, ##__VA_ARGS__)

#define log_info(format, ...) \
	pr_info("[cs_haptic][%04d]%s: " format "\n", __LINE__, __func__, ##__VA_ARGS__)

#define log_dbg(format, ...) \
	pr_debug("[cs_haptic][%04d]%s: " format "\n", __LINE__, __func__, ##__VA_ARGS__)

/* richtap */
#ifdef AAC_RICHTAP_SUPPORT
enum {
	RICHTAP_UNKNOWN = -1,
	RICHTAP_CDV2625 = 0x09,
};

enum {
	MMAP_BUF_DATA_VALID = 0x55,
	MMAP_BUF_DATA_FINISHED = 0xAA,
	MMAP_BUF_DATA_INVALID = 0xFF,
};

#define RICHTAP_IOCTL_GROUP         0x52
#define RICHTAP_GET_HWINFO          _IO(RICHTAP_IOCTL_GROUP, 0x03)
#define RICHTAP_SET_FREQ            _IO(RICHTAP_IOCTL_GROUP, 0x04)
#define RICHTAP_SETTING_GAIN        _IO(RICHTAP_IOCTL_GROUP, 0x05)
#define RICHTAP_OFF_MODE            _IO(RICHTAP_IOCTL_GROUP, 0x06)
#define RICHTAP_TIMEOUT_MODE        _IO(RICHTAP_IOCTL_GROUP, 0x07)
#define RICHTAP_RAM_MODE            _IO(RICHTAP_IOCTL_GROUP, 0x08)
#define RICHTAP_RTP_MODE            _IO(RICHTAP_IOCTL_GROUP, 0x09)
#define RICHTAP_STREAM_MODE         _IO(RICHTAP_IOCTL_GROUP, 0x0A)
#define RICHTAP_UPDATE_RAM          _IO(RICHTAP_IOCTL_GROUP, 0x10)
#define RICHTAP_GET_F0              _IO(RICHTAP_IOCTL_GROUP, 0x11)
#define RICHTAP_STOP_MODE           _IO(RICHTAP_IOCTL_GROUP, 0x12)

#define RICHTAP_MMAP_BUF_SIZE       1000
#define RICHTAP_MMAP_PAGE_ORDER     2
#define RICHTAP_MMAP_BUF_SUM        16

#define HAPTIC_SR_MAX_WAIT_CNT      50
#define HAPTIC_RTP_WAKE_TIME        200

#pragma pack(4)
struct mmap_buf_format {
	uint8_t status;
	uint8_t bit;
	int16_t length;
	uint32_t reserve;
	struct mmap_buf_format *kernel_next;
	struct mmap_buf_format *user_next;
	uint8_t data[RICHTAP_MMAP_BUF_SIZE];
};
#pragma pack()
#endif

/* Enum Define */
enum cs_haptic_flags {
	CS_FLAG_NONR = 0,
	CS_FLAG_SKIP_INTERRUPTS = 1,
};

enum cs_haptic_work_mode {
	CS_STANDBY_MODE = 0,
	CS_RAM_MODE = 1,
	CS_RTP_MODE = 2,
	CS_TRIG_MODE = 3,
	CS_CONT_MODE = 4,
	CS_RAM_LOOP_MODE = 5,
};

enum cs_haptic_bst_mode {
	CS_BST_BYPASS_MODE = 0,
	CS_BST_BOOST_MODE = 1,
};
enum cs_haptic_cont_vbat_comp_mode {
	CS_CONT_VBAT_SW_COMP_MODE = 0,
	CS_CONT_VBAT_HW_COMP_MODE = 1,
};

enum cs_haptic_ram_vbat_comp_mode {
	CS_RAM_VBAT_COMP_DISABLE = 0,
	CS_RAM_VBAT_COMP_ENABLE = 1,
};

enum cs_haptic_f0_flag {
	CS_LRA_F0 = 0,
	CS_CALI_F0 = 1,
};

enum cs_haptic_pwm_mode {
	CS_PWM_48K = 0,
	CS_PWM_24K = 1,
	CS_PWM_12K = 2,
};

enum cs_haptic_play {
	CS_PLAY_NULL = 0,
	CS_PLAY_ENABLE = 1,
	CS_PLAY_STOP = 2,
	CS_PLAY_GAIN = 8,
};

enum cs_haptic_cmd {
	CS_CMD_NULL = 0,
	CS_CMD_ENABLE = 1,
	CS_CMD_HAPTIC = 0x0f,
	CS_CMD_TP = 0x10,
	CS_CMD_SYS = 0xf0,
	CS_CMD_STOP = 255,
};

enum cs_haptic_cali_lra {
	CS_F0_CALI_DEFAULT = 0,
	CS_F0_CALI_LRA = 1,
	CS_OSC_CALI_LRA = 2,
};

enum cs_haptic_rw_flag {
	CS_SEQ_WRITE = 0,
	CS_SEQ_READ = 1,
};

enum cs_haptic_tn {
	CS_T1,
	CS_T2,
	CS_T3,
};
enum z_vib_mode_type {
	SHORT_VIB_RAM_MODE = 0,
	LONG_VIB_RAM_MODE = 1,
	RTP_VIB_MODE = 2,
	VIB_MODE_MAX,
};
enum cs_haptic_read_write {
	CS_HAPTIC_CMD_READ_REG = 0,
	CS_HAPTIC_CMD_WRITE_REG = 1,
};
enum cs_haptic_activate_mode {
	CS_HAPTIC_ACTIVATE_RAM_MODE = 0,
	CS_HAPTIC_ACTIVATE_CONT_MODE = 1,
};
/* common struct */
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

struct fileops {
	unsigned char cmd;
	unsigned char reg;
	unsigned char ram_addrh;
	unsigned char ram_addrl;
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

struct haptic_ctr {
	uint8_t cnt;
	uint8_t cmd;
	uint8_t play;
	uint8_t loop;
	uint8_t gain;
	uint8_t wavseq;
	struct list_head list;
};

struct i2c_info {
	uint32_t flag;
	uint32_t reg_num;
	uint8_t *reg_data;
};

struct haptic_container {
	int len;
	uint8_t data[];
};

struct haptic_dts_info {
	uint8_t mode;
	uint8_t f0_cali_percent;
	uint8_t max_bst_vol;
	uint32_t f0_pre;
	int boost_fw;
	int boost_en;
	uint8_t duration_time[3];
	uint8_t d2s_gain;
	uint8_t brk_bst_md;
	uint8_t bst_vol_ram;
	uint8_t bst_vol_rtp;
	uint8_t bst_vol_default;
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
	uint8_t bstcfg[5];
	uint8_t prctmode[3];
	uint8_t sine_array[4];
	uint8_t trig_cfg[24];
	bool is_enabled_auto_bst;
	bool is_enabled_i2s;
	bool is_enabled_one_wire;
	bool is_enabled_lowpower_bst_config;
	bool is_enabled_lower_amp;
	bool is_enabled_richtap_core;
};

typedef struct cs_chip_inter_para {
	uint32_t interval_us;
	ktime_t kcur_time;
	ktime_t kpre_time;
} cs_chip_inter_para_t;

struct haptic {
	struct wakeup_source *ws;
	/* COMMON */
	uint8_t flags;
	uint8_t play_mode;
	uint8_t auto_boost;
	uint8_t activate_mode;
	uint8_t ram_vbat_comp;
	uint8_t rtp_routine_on;
	uint8_t seq[CS_MAX_SEQ_SIZE];
	uint8_t loop[CS_MAX_SEQ_SIZE];
	uint8_t f0_is_cali;
	int vmax;
	int gain;
	int state;
	int index;
	int chipid;
	int irq_gpio;
	int duration;
	int amplitude;
	int reset_gpio;
	int rtp_idx;
	bool rtp_init;
	bool ram_init;
	bool i2s_config;
	bool emergency_mode_flag;
	bool set_amplitude_flag;
	bool need_notify_duration;
	bool special_long_vib;
	uint32_t cust_boost_on;
	uint32_t f0;
	uint32_t lra;
	uint32_t vbat;
	uint32_t cont_f0;
	uint32_t rtp_cnt;
	uint32_t rtp_len;
	uint32_t gun_type;
	uint32_t bullet_nr;
	uint32_t f0_cali_data;
	uint32_t rtp_file_num;
	uint32_t ramload_fail_cnt;
	uint32_t timeval_flags;
	uint32_t osc_cali_data;
	uint32_t bst_enable;
	unsigned long microsecond;
	uint16_t ram_id;
	cdev_t vib_dev;

	ktime_t kend;
	ktime_t kstart;
	ktime_t kcurrent_time;
	ktime_t kpre_enter_time;

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
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrl_state;
	struct haptic_func *func;
	struct i2c_info i2c_info;
	struct trig trig[CS_T_NUM];
	enum z_vib_mode_type effect_mode;
	struct fileops fileops;
	cs_chip_inter_para_t chip_ipara;
#ifdef AAC_RICHTAP_SUPPORT
	uint8_t *rtp_ptr;
	struct mmap_buf_format *start_buf;
	struct mmap_buf_format *curr_buf;
	struct work_struct richtap_rtp_work;
	int16_t pos;
	atomic_t richtap_rtp_mode;
#endif
	atomic_t is_process_irq_work;
};

struct haptic_func {
	int (*get_f0)(struct haptic *);
	int (*get_irq_state)(struct haptic *);
	int (*check_qualify)(struct haptic *);
	int (*juge_rtp_going)(struct haptic *);
	int (*container_update)(struct haptic *, struct haptic_container *);
	void (*play_stop)(struct haptic *);
	void (*get_vbat)(struct haptic *);
	void (*cont_config)(struct haptic *);
	void (*read_f0)(struct haptic *);
	void (*play_go)(struct haptic *, bool);
	void (*ram_init)(struct haptic *, bool);
	void (*get_lra_resistance)(struct haptic *);
	void (*set_pwm)(struct haptic *, uint8_t);
	void (*set_gain)(struct haptic *, uint8_t);
	uint8_t (*get_gain)(struct haptic *);
	void (*play_mode)(struct haptic *, uint8_t);
	void (*set_bst_vol)(struct haptic *, uint8_t);
	void (*set_repeat_seq)(struct haptic *, uint8_t);
	void (*auto_bst_enable)(struct haptic *, uint8_t, uint8_t);
	void (*vbat_mode_config)(struct haptic *, uint8_t);
	void (*set_wav_seq)(struct haptic *, uint8_t, uint8_t);
	void (*set_wav_loop)(struct haptic *, uint8_t, uint8_t);
	void (*set_rtp_data)(struct haptic *, uint8_t *, uint32_t);
	void (*protect_config)(struct haptic *, uint8_t, uint8_t);
	void (*parse_dt)(struct device *, struct haptic *, struct device_node *);
	void (*irq_clear)(struct haptic *);
	void (*set_ram_addr)(struct haptic *);
	void (*misc_para_init)(struct haptic *);
	void (*interrupt_setup)(struct haptic *);
	void (*set_rtp_aei)(struct haptic *, bool);
	void (*upload_lra)(struct haptic *, uint32_t);
	void (*get_wav_seq)(struct haptic *, uint32_t);
	void (*get_first_wave_addr)(struct haptic *, uint8_t *);
	size_t (*get_wav_loop)(struct haptic *, char *);
	ssize_t (*get_ram_data)(struct haptic *, char *);
	ssize_t (*get_reg)(struct haptic *, ssize_t, char *);
	uint8_t (*get_prctmode)(struct haptic *);
	uint8_t (*get_glb_state)(struct haptic *);
	uint8_t (*rtp_get_fifo_afs)(struct haptic *);
	uint16_t (*get_trim_osc)(struct haptic *);
	void (*enable_gain)(struct haptic *, cs_enable_state_t);
	uint8_t (*rtp_get_fifo_afi)(struct haptic *);
	void (*fifo_reset)(struct haptic *);
#ifdef SND_SOC_CODEC
	int (*snd_soc_init)(struct device *);
#endif
};

/* extern */
struct haptic_func *cdv2625_func(void);
extern int cs_i2c_byte_read(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t * buf, uint32_t len);
extern int cs_i2c_byte_write(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t * buf, uint32_t len);
extern int cs_i2c_one_byte_write(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t data);
extern int cs_write_sram(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t *buf, uint32_t len);

#endif

