/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oal_usb_netbuf.c header file
 * Author: @CompanyNameTag
 */

#include "oal_usb_host_if.h"

#ifndef OAL_USB_NETBUF_H
#define OAL_USB_NETBUF_H

void usb_netbuf_dev_init(struct oal_usb_dev *usb_dev);
int32_t oal_usb_rx_netbuf(hcc_bus *bus, oal_netbuf_head_stru *head);
int32_t oal_usb_tx_netbuf(hcc_bus *bus, oal_netbuf_head_stru *head, hcc_netbuf_queue_type qtype);

#endif
