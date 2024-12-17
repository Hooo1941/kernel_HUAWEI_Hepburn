/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_bluez.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_BLUEZ_H
#define BFGX_BLUEZ_H

#include "hw_bfg_ps.h"

int32_t hw_bt_release(struct inode *inode, struct file *filp);
ssize_t hw_bt_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
uint32_t hw_bt_poll(struct file *filp, poll_table *wait);
long hw_bt_ioctl(struct file *file, uint32_t cmd, unsigned long arg);
ssize_t hw_bt_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
int32_t hw_bt_open(struct inode *inode, struct file *filp);

extern struct bt_data_combination g_bt_data_combination;

#endif
