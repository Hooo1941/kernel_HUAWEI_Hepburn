/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oal_usb_msg.c header file
 * Author: @CompanyNameTag
 */

#include "oal_usb_host_if.h"

#ifndef OAL_USB_MSG_H
#define OAL_USB_MSG_H

void usb_msg_dev_init(struct oal_usb_dev *usb_dev);
int32_t oal_usb_send_msg(hcc_bus *bus, uint32_t val);
int32_t oal_usb_gpio_rx_data(hcc_bus *bus);

#endif
