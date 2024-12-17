/*
 * chip_usb_misc_ctrl.h
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _USB_MISC_CTRL_H_
#define _USB_MISC_CTRL_H_

#include <linux/types.h>
#include <linux/fwnode.h>

#ifdef CONFIG_CHIP_USB_MISC_CTRL
extern int misc_ctrl_init(void);
extern void misc_ctrl_exit(void);
extern bool misc_ctrl_is_ready(void);
extern bool multi_usb_misc_ctrl_is_ready(struct fwnode_handle *fwnode);
extern int multi_usb_misc_ctrl_init(struct fwnode_handle *fwnode);
extern void multi_usb_misc_ctrl_exit(struct fwnode_handle *fwnode);
#else
static inline int misc_ctrl_init(void){
	return 0;
}
static inline void misc_ctrl_exit(void){};
static inline bool misc_ctrl_is_ready(void)
{
	return false;
}
static inline bool multi_usb_misc_ctrl_is_ready(struct fwnode_handle *fwnode)
{
	return true;
};
static inline int multi_usb_misc_ctrl_init(struct fwnode_handle *fwnode)
{
	return 0;
}
static inline void multi_usb_misc_ctrl_exit(struct fwnode_handle *fwnode){};
#endif /* CONFIG_CHIP_USB_MISC_CTRL */

#endif /* _USB_MISC_CTRL_H_ */
