/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: hiusb debug common
 * Create: 2022-08-13
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _USB_DEBUG_COMMON_H_
#define _USB_DEBUG_COMMON_H_
#include <linux/types.h>
#include <linux/debugfs.h>

typedef ssize_t (*hiusb_debug_show_ops)(void *, char *, size_t);
typedef ssize_t (*hiusb_debug_store_ops)(void *, const char *, size_t);

/*
 * debugfs interface call back data
 */
#define DBG_NODE_NAME_LENG 16
struct hiusb_debug_attr {
	char name[DBG_NODE_NAME_LENG];
	void *dev_data;
	hiusb_debug_show_ops show;
	hiusb_debug_store_ops write;
	struct list_head list;
};

int hiusb_debug_template_open(struct inode *inode, struct file *file);
ssize_t hiusb_debug_template_write(struct file *file,
			const char __user *buf, size_t size, loff_t *ppos);
void hiusb_debug_register_addlist(const char *name, void *dev_data,
		hiusb_debug_show_ops show, hiusb_debug_store_ops store, struct list_head *head);

#endif /* _USB_DEBUG_COMMON_H_ */
