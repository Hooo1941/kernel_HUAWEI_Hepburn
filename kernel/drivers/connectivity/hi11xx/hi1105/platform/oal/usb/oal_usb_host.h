/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oal_usb_host.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAL_USB_HOST_H
#define OAL_USB_HOST_H

#include "oal_usb_host_if.h"

void *oal_usb_device_get(enum OAL_USB_INTERFACE type, enum OAL_USB_CHAN_INDEX chan);

#endif
