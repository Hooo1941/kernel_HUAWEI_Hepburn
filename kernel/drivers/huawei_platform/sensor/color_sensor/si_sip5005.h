/*
 * si_sip5005.h
 *
 * code for SI sip5005 sensor
 *
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
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

#ifndef __SI_SIP5005__
#define __SI_SIP5005__
#include "color_sensor.h"

#ifndef SI_PLATFORM_H
#define SI_PLATFORM_H

/* Register map */
#define SIP5005_AL_CLK_CTRL                      0x01
#define SIP5005_LOW_POWER_CTRL                   0x02
#define SIP5005_ID_REG                           0x03
#define SIP5005_RST_CTRL                         0x05
#define SIP5005_INT_CTRL                         0x06
#define SIP5005_AL_CTRL0                         0x50
#define SIP5005_AL_CTRL1                         0x51
#define SIP5005_AL_CTRL2                         0x52
#define SIP5005_AL_INT_EN                        0x53
#define SIP5005_AL_DATA_LOW_TH_H                 0x54
#define SIP5005_AL_DATA_LOW_TH_L                 0x55
#define SIP5005_AL_DATA_HIGH_TH_H                0x56
#define SIP5005_AL_DATA_HIGH_TH_L                0x57
#define SIP5005_ALS_INTE_TIME_H                  0x58
#define SIP5005_ALS_INTE_TIME_M                  0x59
#define SIP5005_ALS_INTE_TIME_L                  0x5A
#define SIP5005_AL_PERIOD_STEP_H                 0x5B
#define SIP5005_AL_PERIOD_TIME                   0x5C
#define SIP5005_RST_NUM                          0x5D
#define SIP5005_AL_AZ_AUTO_NUM                   0x5E
#define SIP5005_AL_AZ_EN                         0x5F
#define SIP5005_AL_AZ_WAIT_TH                    0x60
#define SIP5005_AL_GAIN_1_0                      0x61
#define SIP5005_AL_GAIN_3_2                      0x62
#define SIP5005_AL_GAIN_5_4                      0x63
#define SIP5005_AL_IDAC_MODE                     0x65
#define SIP5005_AL_IDAC_MSB                      0x66
#define SIP5005_AL0_IDAC                         0x67
#define SIP5005_AL1_IDAC                         0x68
#define SIP5005_AL2_IDAC                         0x69
#define SIP5005_AL3_IDAC                         0x6A
#define SIP5005_AL4_IDAC                         0x6B
#define SIP5005_SW_MODE                          0x6F
#define SIP5005_FD_CTRL                          0x70
#define SIP5005_FD_INTE_TIME_H                   0x72
#define SIP5005_FD_INTE_TIME_L                   0x73
#define SIP5005_ALL_AL_INT                       0x82
#define SIP5005_ALL_FD_INT                       0x83
#define SIP5005_DATA_VALID                       0x84
#define SIP5005_FD_DATA                          0x85
#define SIP5005_AL0_DATA_H                       0x86
#define SIP5005_AL0_DATA_L                       0x87
#define SIP5005_AL1_DATA_H                       0x88
#define SIP5005_AL1_DATA_L                       0x89
#define SIP5005_AL2_DATA_H                       0x8A
#define SIP5005_AL2_DATA_L                       0x8B
#define SIP5005_AL3_DATA_H                       0x8C
#define SIP5005_AL3_DATA_L                       0x8D
#define SIP5005_AL4_DATA_H                       0x8E
#define SIP5005_AL4_DATA_L                       0x8F
#define SIP5005_AL5_DATA_H                       0x90
#define SIP5005_AL5_DATA_L                       0x91
#define SIP5005_AL6_DATA_H                       0x92
#define SIP5005_AL6_DATA_L                       0x93
#define SIP5005_AL7_DATA_H                       0x94
#define SIP5005_AL7_DATA_L                       0x95
#define SIP5005_AL8_DATA_H                       0x96
#define SIP5005_AL8_DATA_L                       0x97
#define SIP5005_AL9_DATA_H                       0x98
#define SIP5005_AL9_DATA_L                       0x99
#define SIP5005_AL10_DATA_H                      0x9A
#define SIP5005_AL10_DATA_L                      0x9B
#define SIP5005_AL11_DATA_H                      0x9C
#define SIP5005_AL11_DATA_L                      0x9D
#define SIP5005_AL12_DATA_H                      0x9E
#define SIP5005_AL12_DATA_L                      0x9F
#define SIP5005_FIFO_STATUS0                     0xA0
#define SIP5005_FIFO_STATUS1                     0xA1
#define SIP5005_FIFO_CFG2                        0xA3
#define SIP5005_TRIM_SEL0                        0xB0
#define SIP5005_TRIM7                            0xBA
#define SIP5005_OTPD0                            0xD0
#define SIP5005_OTPD4                            0xD4
#define SIP5005_AE_THRES_L                       0xE1
#define SIP5005_AF_THRES_L                       0xE3
#define SIP5005_RST_MODE                         0xE4

// SIP5005_RST_CTRL	                           @ 0x05
#define OSC_EN                            (0x01 << 1)
#define SOFT_RST_N                        (0x01 << 0)

// SIP5005_INT_CTRL                             @0x06
#define INT_EN                            (0x01 << 2)
#define INT_CLEAR_MODE                    (0x01 << 0)

// SIP5005_AL_CTRL0 @0x50   ASL + FD
#define AL_EN                             (0x01 << 0)
#define AL0_EN                            (0x01 << 1)
#define AL1_EN                            (0x01 << 2)
#define AL2_EN                            (0x01 << 3)
#define AL3_EN                            (0x01 << 4)
#define AL4_EN                            (0x01 << 5)
#define FD_EN                             (0x01 << 6)
#define AL_EN_ALL                         (AL_EN | AL0_EN | AL1_EN | AL2_EN | AL3_EN | AL4_EN)

// SIP5005_ALS_CTRL1                            @0x51
#define ALS_PERIOD_EN                     (0x01 << 0)

// SIP5005_ALS_CTRL2                            @0x52
#define APERS_SHIFT                                 4
#define APERS_MASK              (0x0F << APERS_SHIFT)
#define ALS_TRIGGER_MODE                  (0x00 << 0)

// SIP5005_ALSINT_EN                            @0x53
#define ALS_CHANNEL_INT_EN                (0x0F << 4)  /* 0-12 select PD INT */
#define ALS_ERR_INT_EN                    (0x01 << 3)
#define ALS_ANS_SAT_INT_EN                (0x01 << 2)
#define ALS_DIG_SAT_INT_EN                (0x01 << 1)
#define ALS_INT_EN                        (0x01 << 0)
#define ALS_INT_MASK                             0x0F

// SIP5005_ALS_AZ_EN                            @0x5F
#define ALS_AZ_MANUAL_EN                  (0x01 << 1)
#define ALS_AZ_AUTO_EN                    (0x01 << 0)

// SIP5005_ALS_GAIN_1_0                         @0x61
#define ALS1_GAIN_MASK                    (0x0F << 4)
#define ALS0_GAIN_MASK                    (0x0F << 0)

// SIP5005_ALS_GAIN_3_2			                @0x62
#define ALS3_GAIN_MASK                    (0x0F << 4)
#define ALS2_GAIN_MASK                    (0x0F << 0)

// SIP5005_ALS_GAIN_5_4			                @0x63
#define ALS5_GAIN_MASK                    (0x0F << 4)
#define ALS4_GAIN_MASK                    (0x0F << 0)

// SIP5005_FD_CTRL				                @0x70
#define FINT_EN                           (0x01 << 1)

// SIP5005_ALL_AL_INT			                @0x82
#define ALS_ERR_INT                       (0x01 << 3)
#define ALS_ANS_SAT_INT                   (0x01 << 2)
#define ALS_DIG_SAT_INT                   (0x01 << 1)
#define ALS_INT                           (0x01 << 0)

// SIP5005_ALL_FD_INT			                @0x83
#define FIFO_INT                          (0x01 << 1)
#define FLICKER_SAT_INT                   (0x01 << 0)

// SIP5005_DATA_VALID			                @0x84
#define ALS_DATA_VALID                    (0x01 << 0)

// SIP5005_FIFO_STATUS0			                @0xA0
#define FIFO_LVL_L_MASK                   (0xFF << 0)

// SIP5005_FIFO_STATUS1			                @0xA1
#define FIFO_LVL_H_MASK                   (0x01 << 0)
#define FIFO_FULL                         (0x01 << 4)
#define FIFO_EMPTY                        (0x01 << 3)
#define FIFO_ALMOST_FULL                  (0x01 << 2)
#define FIFO_ALMOST_EMPTY                 (0x01 << 1)

// SIP5005_FIFO_CFG2			                @0xA3
#define FIFO_CLEAR                        (0x01 << 4)
#define FIFO_INT_EN                       (0x01 << 5)

// SIP5005_AL_GAIN_1_0			                @0x61
#define AGAIN_SHIFT                                 0
#define AGAIN_MASK              (0x0F << AGAIN_SHIFT)
#define AGAIN_0_25X             (0x00 << AGAIN_SHIFT)
#define AGAIN_0_5X              (0x01 << AGAIN_SHIFT)
#define AGAIN_1X                (0x02 << AGAIN_SHIFT)
#define AGAIN_2X                (0x03 << AGAIN_SHIFT)
#define AGAIN_4X                (0x04 << AGAIN_SHIFT)
#define AGAIN_8X                (0x05 << AGAIN_SHIFT)
#define AGAIN_16X               (0x06 << AGAIN_SHIFT)
#define AGAIN_32X               (0x07 << AGAIN_SHIFT)
#define AGAIN_64X               (0x08 << AGAIN_SHIFT)
#define AGAIN_128X              (0x09 << AGAIN_SHIFT)
#define AGAIN_256X              (0x0A << AGAIN_SHIFT)
#define AGAIN_512X              (0x0B << AGAIN_SHIFT)
#define AGAIN_1024X             (0x0C << AGAIN_SHIFT)
#define AGAIN_2048X             (0x0D << AGAIN_SHIFT)

// SIP5005_AL_GAIN_5_4			                @0x63
#define FD_GAIN_0_25X           (0x00 << AGAIN_SHIFT)
#define FD_GAIN_0_5X            (0x01 << AGAIN_SHIFT)
#define FD_GAIN_1X              (0x02 << AGAIN_SHIFT)
#define FD_GAIN_2X              (0x03 << AGAIN_SHIFT)
#define FD_GAIN_4X              (0x04 << AGAIN_SHIFT)
#define FD_GAIN_8X              (0x05 << AGAIN_SHIFT)
#define FD_GAIN_16X             (0x06 << AGAIN_SHIFT)
#define FD_GAIN_32X             (0x07 << AGAIN_SHIFT)
#define FD_GAIN_64X             (0x08 << AGAIN_SHIFT)
#define FD_GAIN_128X            (0x09 << AGAIN_SHIFT)
#define FD_GAIN_256X            (0x0A << AGAIN_SHIFT)
#define FD_GAIN_512X            (0x0B << AGAIN_SHIFT)
#define FD_GAIN_1024X           (0x0C << AGAIN_SHIFT)
#define FD_GAIN_2048X           (0x0D << AGAIN_SHIFT)
#define FD_GAIN_4096X           (0x0E << AGAIN_SHIFT)

// Configration calculations
#define ATIME_DUTY_CYCLE                        2716

/* For ALS/Color auto gain setting */
#define MAX_ALS_VALUE                         0xFFFF
#define MIN_ALS_VALUE                              3
// Mark above register is follow SIP5005 chip define

#define SIP5005_CAL_RATIO                      10000
#define SIP5005_GAIN_SCALE                      1000
#define SIP5005_ITIME_DEFAULT                     50
#define SIP5005_CAL_ITIME_DEFAULT                250

#define SIP5005_ITIME_FAST                        15
#define SI_SIP5005_GAIN_OF_GOLDEN               2048

#define SI_SIP5005_FD_GAIN_OF_GOLDEN            2048
#define DEFAULT_ALS_GAIN_ID                        4
#define SI_REPORT_LOG_COUNT_NUM                   30

#define SIP5005_ITIME_FOR_FIRST_DATA               3
#define SIP5005_AGAIN_FOR_FIRST_DATA               6
#define ONE_BYTE_LENGTH                            8
#define INTEGER_BIT_SIZE                          32

#define GAIN_LEVEL_1                               1
#define GAIN_LEVEL_2                               4
#define GAIN_LEVEL_3                              16
#define GAIN_LEVEL_4                              64
#define GAIN_LEVEL_5                             256
#define GAIN_LEVEL_6                             512

#define FD_GAIN_LEVEL_1                            2
#define FD_GAIN_LEVEL_2                            8
#define FD_GAIN_LEVEL_3                           64
#define FD_GAIN_LEVEL_4                          256
#define FD_GAIN_LEVEL_5                         1024
#define FD_GAIN_LEVEL_6                         2048
#define FD_GAIN_LEVEL_7                         4096

#define ALS_GAIN_VALUE_1                           0  /* 0.5 */
#define ALS_GAIN_VALUE_2                           1
#define ALS_GAIN_VALUE_3                           2
#define ALS_GAIN_VALUE_4                           4
#define ALS_GAIN_VALUE_5                           8
#define ALS_GAIN_VALUE_6                          16
#define ALS_GAIN_VALUE_7                          32
#define ALS_GAIN_VALUE_8                          64
#define ALS_GAIN_VALUE_9                         128
#define ALS_GAIN_VALUE_10                        256
#define ALS_GAIN_VALUE_11                        512
#define ALS_GAIN_VALUE_12                       1024
#define ALS_GAIN_VALUE_13                       2048
#define ALS_GAIN_VALUE_14                       4096

#define FIFO_LEVEL_RATIO                           2
#define FD_POLLING_TIME                           22

#define DEFAULT_ASTEP                            499
#define DEFAULT_FD_GAIN                            4

#define SI_REPORT_DATA_LEN                        10
#define RGBAP_CALI_DATA_NV_NUM                   383
#define SI_SIP5005_FLOAT_TO_FIX                10000

#define SIP5005_FD_LOW_LEVEL                      30
#define SATURATION_CHECK_PCT                       8

#define SIDRIVER_ALS_ENABLE                        1
#define SIDRIVER_ALS_DISABLE                       0

#define SIDRIVER_FD_ENABLE                         1
#define SIDRIVER_FD_DISABLE                        0

#define REPORT_FIFO_LEN                          128

#define MAX_GAIN_ID                               12
#define MAX_AUTOGAIN_CHECK_CNT                    10
#define MIN_AUTOGAIN_CHECK                         9
#define MAX_AUTOGAIN_CHECK                        10
#define BUF_16_MAX_LEVEL5                          2
#define BUF_16_MAX_LEVEL4                          9
#define BUF_16_MAX_LEVEL3                         36
#define BUF_16_MAX_LEVEL2                        288
#define BUF_RATIO_8_16BIT                          2
#define ONE_BYTE                                   1
#define MS_2_US                                 1000
#define DOUBLE_BYTE                                2
#define MAX_SATURATION                         65535
#define MAX_ADC_COUNT                          65535

#define HIGH_TIMER_OFFSET                         80
#define LOW_TIMER_OFFSET                           2
#define AUTOZERO_TIMER_OFFSET                     12
#define ITIME_TIMER_OFFSET                        40
#define AUTO_GAIN_ITIME                            3

#define CALI_RGB_TIMER                           120
#define FIRST_FLK_TIMER                            6
#define MAX_BUFFER_SIZE                          256
#define BUFSIZ                                   512
#define PIXEL_MAX                                 39
#define PIXEL_RATIO                                2
#define FULL_BYTE                               0xFF
#define HIGH_HALF_BYTE                          0xF0
#define LOW_HALF_BYTE                           0x0F
#define HALF_BYTE_SHIFT                            4
#define ALS_CHANNELS                              13

#define ASTEP_LEN                                  2
#define DEFAULT_CALIB_SIP5005_GAIN_ID              9
#define LOW_AUTO_GAIN_VALUE                        3
#define AUTO_GAIN_DIVIDER                          2
#define LOWEST_GAIN_ID                             0
#define LOWEST_GAIN_VALUE                        500
#define SATURATION_LOW_PERCENT                    80
#define SATURATION_HIGH_PERCENT                  100
#define RAW_DATA_BYTE_NUM                         26
#define MAX_LOG_COUNT                             10

#define SIP5005_REGADDR_CHAIN_SMUX              0xDF
#define SIP5005_REGADDR_CHAINCMD                0xDC
#define CHAIN_CMD_SHIFT_ALS_6BITS               0x46
#define MAX_FIFO_LEVEL                           256
#define MAX_SMUX_WAIT_CYCLE                        5
#define CHIP_ID                                 0x0A

#define ITIME_DIVIDER                            720
#define ITIME_MULTIPLIER                        2000
#define ROUNDING_SCALAR                           10
#define ROUNDING_ADD                               5
#define IS_SATURATION                              1
#define SIP5005_CAL_RATIO                      10000
#define SIP5005_INTEGER_VAL                  	1000
#define SIP5005_ROUNDING_VAL                 1000000
#define SIP5005_HIGH_8_BIT                        16
#define SIP5005_MIDDLE_8_BIT                       8
#define SIP5005_MIDDLE_8_BIT_OFFSET           0x00FF
#define SIP5005_LOW_8_BIT_OFFSET            0x0000FF
#define SIP5005_ALS_ENABLE_TIME                    2
#define SIP5005_DISABLE_TIME                      10
#define SIP5005_IDAC_MSB_NUM                       6
#define SET_ENABLE_COMPLETE                        1
#define ALSPS_CFG_MAX_CHANNELS	                   5

enum channel_names {
	CHAN_F2 = 0,
	CHAN_FZ = 1,
	CHAN_F3 = 2,
	CHAN_F4 = 3,
	CHAN_FY = 4,
	CHAN_FX = 5,
	CHAN_F6 = 6,
	CHAN_F7 = 7,
	CHAN_NIR = 8,
	CHAN_VIS = 9
};

enum measurement_steps {
	STEP_FAST_AUTO_GAIN,
	STEP_FAST_MEASURE,
	STEP_DEFAULT_MEASURE
};

struct sip5005_flc_ctx_t {
	UINT32 fd_gain;
	UINT16 fd_time_ms;
	struct mutex lock;
	bool first_fd_inte;
};

struct sip5005_als_ctx_t {
	UINT16 saturation;
	UINT8 gain_id;
	UINT32 gain;
	UINT32 itime_ms;
	UINT16 astep;
	UINT8 atime;
	UINT8 status;
	UINT8 data_valid;
	UINT8 status2;
	UINT8 als_update;
	enum measurement_steps meas_steps;
	bool load_calibration;
	UINT32 data[SI_REPORT_DATA_LEN];
	UINT8 enabled;
};

struct sip5005_calib_ctx_t {
	uint32_t target[SI_REPORT_DATA_LEN];
	uint32_t ratio[SI_REPORT_DATA_LEN];
	uint8_t reserved[16];
};

enum sip5005_sensor_id_t {
	SI_UNKNOWN_DEVICE,
	SI_SIP5005_REV0,
	SI_SIP5005_LAST_DEVICE
};

enum tcs_rev_id_t {
	SI_UNKNOWN_REV,
	SI_SIP5005_REVID = 0x00
};

enum tcs_hal_algo_t {
	SI_UNSUPPORT_ALGO,
};

struct sip5005_reg_setting_t {
	UINT8 reg;
	UINT8 value;
};


struct sip5005_channel_idac {
	UINT8 ch_idac_high;
	UINT8 ch_idac_low;
};

struct sip5005_channel_scale {
	UINT8 value;
	INT32 scale;
};

static struct sip5005_channel_idac ch_idac_config[ALSPS_CFG_MAX_CHANNELS];
static struct sip5005_channel_scale ch_scale[SI_REPORT_DATA_LEN];

enum sip5005_power_mode_t {
	SI_SIP5005_FEATURE_ALS = (1 << 0),
	SI_SIP5005_FEATURE_LAST
};

struct sip5005_ctx {
	enum sip5005_sensor_id_t device_id;
	struct i2c_client *handle;
	struct sip5005_als_ctx_t alg_ctx;
	struct sip5005_flc_ctx_t flc_ctx;
	struct sip5005_calib_ctx_t cal_ctx;
};

static UINT8 const sip5005_ids[] = {
	CHIP_ID,  // sip5005 chip id val
};

static const struct sip5005_reg_setting_t sip5005_settings[] = {
	{ SIP5005_RST_CTRL, 0x03 },
	{ SIP5005_TRIM_SEL0, 0x0A },
	{ SIP5005_ALS_INTE_TIME_H, 0x00 },  /* 100ms */
	{ SIP5005_ALS_INTE_TIME_M, 0x8F },
	{ SIP5005_ALS_INTE_TIME_L, 0xD2 },
	{ SIP5005_AL_PERIOD_STEP_H, 0x08 },  /* 100.1ms */
	{ SIP5005_AL_PERIOD_TIME, 0x63 },
	{ SIP5005_AL_AZ_AUTO_NUM, 0xFF },
	{ SIP5005_AL_AZ_EN, 0xC1 },
	{ SIP5005_FD_INTE_TIME_H, 0x01 },  /* 1ms INTE_TIME */
	{ SIP5005_FD_INTE_TIME_L, 0x70 },
	{ SIP5005_AL_GAIN_1_0, (AGAIN_4X << 4) | AGAIN_4X },
	{ SIP5005_AL_GAIN_3_2, (AGAIN_4X << 4) | AGAIN_4X },
	{ SIP5005_AL_GAIN_5_4, (AGAIN_8X << 4) | AGAIN_4X },
	{ SIP5005_AL_CTRL2, (uint8_t)(((1) & 0x0F) << 4) },
	{ SIP5005_TRIM7, 0x06 },
	{ SIP5005_SW_MODE, 0xED },
	{ SIP5005_SW_MODE, 0x01 },
};

static UINT32 const sip5005_als_gain_lvl[] = {
	GAIN_LEVEL_1,
	GAIN_LEVEL_2,
	GAIN_LEVEL_3,
	GAIN_LEVEL_4,
	GAIN_LEVEL_5
};

static UINT16 const sip5005_als_gain[] = {
	ALS_GAIN_VALUE_1,  /* actual gain is 0.5 */
	ALS_GAIN_VALUE_2,
	ALS_GAIN_VALUE_3,
	ALS_GAIN_VALUE_4,
	ALS_GAIN_VALUE_5,
	ALS_GAIN_VALUE_6,
	ALS_GAIN_VALUE_7,
	ALS_GAIN_VALUE_8,
	ALS_GAIN_VALUE_9,
	ALS_GAIN_VALUE_10,
	ALS_GAIN_VALUE_11,
	ALS_GAIN_VALUE_12,
	ALS_GAIN_VALUE_13
};

#endif
#endif
