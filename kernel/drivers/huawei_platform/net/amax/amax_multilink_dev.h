/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: This module is to support wifi multilink ioctl.
 * Create: 2023-03-31
 */
#ifndef AMAX_MULTILINK_DEV_H_
#define AMAX_MULTILINK_DEV_H_

#include <linux/cdev.h>
#include <linux/device.h>

typedef struct {
	struct cdev cdev;
	struct class *class;
	struct device *dev;
	dev_t devid;
	int major;
} amax_device;

typedef int32_t (*amax_stat_ctl_hook_func)(uint8_t enable);
typedef int32_t (*amax_thold_set_hook_func)(uint8_t tp, uint8_t latency, uint8_t pkt_num);

typedef void (*amax_module_init_hook_func)(void);
typedef void (*amax_module_exit_hook_func)(void);

typedef void (*dps_msg_hook_func)(uint8_t *msg_info, unsigned int msg_len);
extern void dps_ioctl_reg(dps_msg_hook_func msg_func);
extern void dps_ioctl_unreg(void);

extern void amax_ioctl_reg(amax_stat_ctl_hook_func ctl_func, amax_thold_set_hook_func set_func);
extern void amax_ioctl_unreg(void);

extern void amax_mod_reg(amax_module_init_hook_func init_func, amax_module_exit_hook_func exit_func);
extern void amax_mod_unreg(void);
#endif