/*
 * Copyright (C) 2014 Google, Inc.
 * Author: Badhri Jagan Sridharan <badhri@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __USB_GADGET_F_MTP_H
#define __USB_GADGET_F_MTP_H

#include <linux/types.h>
#include <linux/usb/composite.h>

extern struct usb_function_instance *alloc_inst_mtp_ptp(bool mtp_config);
extern struct usb_function *function_alloc_mtp_ptp(
			struct usb_function_instance *fi, bool mtp_config);

#endif
