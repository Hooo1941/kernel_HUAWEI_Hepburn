/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: This module is used to start vcom between ril and kernel
 * Author: lizhenzhou1@huawei.com
 * Create: 2022-11-26
 */

#ifndef _VCOM_H_
#define _VCOM_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// BBIC firmware load ioctl cmd
#define BBIC_IOCTL_MAGIC 'U'
#define BBIC_START_CMD _IO(BBIC_IOCTL_MAGIC, 1)
#define BBIC_STOP_CMD _IO(BBIC_IOCTL_MAGIC, 0)
#define BBIC_SET_BOOT_MODE_CMD _IOW(BBIC_IOCTL_MAGIC, 2, int)

// BBIC factory version ioctl cmd
#define BBIC_IOCTL_MAGIC_FAC 'F'
#define BBIC_START_CMD_FAC _IO(BBIC_IOCTL_MAGIC_FAC, 1)
#define BBIC_STOP_CMD_FAC _IO(BBIC_IOCTL_MAGIC_FAC, 0)
#define BBIC_SET_BOOT_MODE_CMD_FAC _IOW(BBIC_IOCTL_MAGIC_FAC, 2, int)

// SIM switch,0->modem,1->BBIC
#define BBIC_SET_SIM0_SWITCH_CMD_FAC _IOW(BBIC_IOCTL_MAGIC_FAC, 3, int)
#define BBIC_SET_SIM1_SWITCH_CMD_FAC _IOW(BBIC_IOCTL_MAGIC_FAC, 4, int)

// BBIC sim ldo io,0->1v8,1->3v
#define BBIC_SET_SIM_VCC_CMD_FAC _IOW(BBIC_IOCTL_MAGIC_FAC, 5, int)

// sleep gpio direction,0->initial status,1->all input
#define BBIC_SET_SLEEP_GPIO_SWITCH_CMD_FAC _IOW(BBIC_IOCTL_MAGIC_FAC, 6, int)
// sleep gpio status query,0->all low,1->all high,2->other
#define BBIC_SET_SLEEP_GPIO_QUERY_CMD_FAC _IOW(BBIC_IOCTL_MAGIC_FAC, 7, int)

// BBIC release version ioctl cmd
#define BBIC_IOCTL_MAGIC_RELEASE 'R'
#define BBIC_START_CMD_RELEASE _IO(BBIC_IOCTL_MAGIC_RELEASE, 1)
#define BBIC_STOP_CMD_RELEASE _IO(BBIC_IOCTL_MAGIC_RELEASE, 0)
#define BBIC_SET_BOOT_MODE_CMD_RELEASE _IOW(BBIC_IOCTL_MAGIC_RELEASE, 2, int)

// SIM switch,0->modem,1->BBIC
#define BBIC_SET_SIM0_SWITCH_CMD_RELEASE _IOW(BBIC_IOCTL_MAGIC_RELEASE, 3, int)
#define BBIC_SET_SIM1_SWITCH_CMD_RELEASE _IOW(BBIC_IOCTL_MAGIC_RELEASE, 4, int)

// BBIC sim ldo io,0->1v8,1->3v
#define BBIC_SET_SIM_VCC_CMD_RELEASE _IOW(BBIC_IOCTL_MAGIC_RELEASE, 5, int)

int hw_stavcom_init(void);
void hw_stavcom_exit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

