/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _HIUSB_COMMON_H_
#define _HIUSB_COMMON_H_

enum usb_state {
	USB_STATE_UNKNOWN = 0,
	USB_STATE_OFF,
	USB_STATE_DEVICE,
	USB_STATE_HOST,
	USB_STATE_HIFI_USB,
	USB_STATE_HIFI_USB_HIBERNATE,
	USB_STATE_ILLEGAL,
};

enum chip_charger_type {
	CHARGER_TYPE_SDP = 0, /* Standard Downstreame Port */
	CHARGER_TYPE_CDP, /* Charging Downstreame Port */
	CHARGER_TYPE_DCP, /* Dedicate Charging Port */
	CHARGER_TYPE_UNKNOWN, /* non-standard */
	CHARGER_TYPE_NONE, /* not connected */

	/* other messages */
	PLEASE_PROVIDE_POWER, /* host mode, provide power */
	CHARGER_TYPE_ILLEGAL, /* illegal type */
};

/*
 * chip usb bc
 */

#define BC_AGAIN_DELAY_TIME_1 200
#define BC_AGAIN_DELAY_TIME_2 8000
#define BC_AGAIN_ONCE	1
#define BC_AGAIN_TWICE	2

enum usb_connect_state {
	USB_CONNECT_DCP = 0xEE, /* connect a charger */
	USB_CONNECT_HOST = 0xFF, /* usb state change form hifi to host */
};

#endif /* _HIUSB_COMMON_H_ */
