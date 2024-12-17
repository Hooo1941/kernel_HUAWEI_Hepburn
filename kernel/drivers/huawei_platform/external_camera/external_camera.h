#ifndef __EXTERNAL_CAMERA_H__
#define __EXTERNAL_CAMERA_H__

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/pm_wakeup.h>

#define SENSOR_NUM_MAX  10
#define SENSOR_NAME_LEN_MAX 30

enum cfg_type {
	PCIE2USB_POWER_UP,
	PCIE2USB_POWER_DOWN,
	PCIE2USB_ENUMERATE_DEVICE,
	ISP_POWER_UP,
	ISP_POWER_DOWN,
	SENSOR_POWER_UP,
	SENSOR_POWER_DOWN
};
struct cfg_data {
	enum cfg_type type;
	int parameter;
};

typedef struct sensor_info {
	int id;
	int position;
	char sensor_name[SENSOR_NAME_LEN_MAX];
}sensor_info_t;

typedef struct sensor_list {
	int num;
	sensor_info_t info[SENSOR_NUM_MAX];
}sensor_list_t;

#define EXCAM_IOCTL_CFG               	_IOW('C', 0, struct cfg_data)
#define EXCAM_IOCTL_GET_INFO          	_IOR('R', 1, struct sensor_list)
#define EXCAM_IOCTL_GET_FLASH_INVALID 	_IOR('R', 2, int)
#ifdef CONFIG_DKMD_DKSM
#define EXCAM_IOCTL_GET_SCREENSTATUS  	_IOR('R', 3, int)
#define EXCAM_IOCTL_CFG_DELAY_POWEROFF	_IOW('C', 4, int)
#endif

#endif