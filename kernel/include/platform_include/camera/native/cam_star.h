
/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: cam star module interfaces
 * Create: 2023-08-05
 */
#ifndef __HW_KERNEL_CAM_STAR_CFG_H__
#define __HW_KERNEL_CAM_STAR_CFG_H__
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

enum {
	HWSENSORMOTOR_DEVICE_NAME_SIZE = 32,
	SENSORMOTOR_V4L2_EVENT_TYPE = V4L2_EVENT_PRIVATE_START + 0x00070000,
	SENSORMOTOR_HIGH_PRIO_EVENT = 0x1500,
	SENSORMOTOR_MIDDLE_PRIO_EVENT = 0x2000,
	SENSORMOTOR_LOW_PRIO_EVENT = 0x3000,
};

struct sensormotor_info {
	char name[HWSENSORMOTOR_DEVICE_NAME_SIZE];
};

enum power_off_state_t {
	POWEROFF_TRY_MCU_STATE = 0,
	POWEROFF_DIR_WITHOUT_TRY,
};

struct sensormotor_move_info {
	uint16_t speed;
	union {
		uint16_t code;
		/* positive: move step forward
		* negative : move step backward
		*/
		int16_t step;
	};
};

enum calib_mode_t {
	NORMAL_MODE = 0,
	CALIB_MODE,
};

enum isr_err {
	MOVE_DONE = 0,
	NEW_LOG = 1,
	PERI_I2C_OFF = 2,
	MIDWAY_REVERSAL = 3,
	CALI_DATA_INVALID = 8,
	TMR_ABNORMAL = 9,
	EXTERNAL_PRESSION_DRAWING = 10,
	DRV_ABNORMAL = 11,
	MOVE_TIMEOUT = 12,
	TMR_DEVICE_ABSENT = 13,
	UNEXPECTED_RESET = 16,
	PERI_I2C_ERROR = 17,
	ADC_ABNORMAL = 18,
	E2PROM_ABNORMAL = 19,
};

struct star_event_t {
	uint32_t kind;
};

enum calib_data_from_t {
	CALIB_DATA_FROM_CDT = 1,
	CALIB_DATA_FROM_SUPPLIER = 2,
	CALIB_DATA_FROM_GOLDEN = 3,
};

#define HWSENSORMOTOR_IOCTL_GET_INFO \
	_IOR('A', BASE_VIDIOC_PRIVATE + 1, struct sensormotor_info)
#define HWSENSORMOTOR_IOCTL_DOWNLOAD_FW                         _IO('B', BASE_VIDIOC_PRIVATE + 2)
#define HWSENSORMOTOR_IOCTL_POWER_ON                            _IO('C', BASE_VIDIOC_PRIVATE + 3)
#define HWSENSORMOTOR_IOCTL_POWER_OFF \
	_IOW('D', BASE_VIDIOC_PRIVATE + 4, enum power_off_state_t)
#define HWSENSORMOTOR_IOCTL_MOVE \
	_IOWR('E', BASE_VIDIOC_PRIVATE + 5, struct sensormotor_move_info)
#define HWSENSORMOTOR_IOCTL_STOP                                _IO('F', BASE_VIDIOC_PRIVATE + 6)
#define HWSENSORMOTOR_IOCTL_GET_REAL_HEIGHT                     _IOWR('G', BASE_VIDIOC_PRIVATE + 7, uint16_t)
#define HWSENSORMOTOR_IOCTL_SET_CALIB_MODE                      _IOWR('H', BASE_VIDIOC_PRIVATE + 8, enum calib_mode_t)
/* before DOWNLOAD_FW to crc check */
#define HWSENSORMOTOR_IOCTL_MAINTENANCE_TAG                     _IO('I', BASE_VIDIOC_PRIVATE + 9)
/* after POWER_ON, before MOVE */
#define HWSENSORMOTOR_IOCTL_MAINTENANCE_DELTA                   _IOWR('J', BASE_VIDIOC_PRIVATE + 10, uint16_t)
/* especially for Maintenance outlets */
#define HWSENSORMOTOR_IOCTL_REPAIRED_TAG                        _IO('K', BASE_VIDIOC_PRIVATE + 11)

#endif