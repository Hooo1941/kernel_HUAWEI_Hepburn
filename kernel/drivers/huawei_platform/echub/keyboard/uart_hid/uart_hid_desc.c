/*
 * Huawei uart hid Driver for uart keyboard hid report
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/types.h>

#include "uart_hid_desc.h"

#define REPORT_ID_KEYBOARD 0x01
#define REPORT_ID_POWER_KEY 0x02
#define REPORT_ID_ENAHNCE_KEY 0x03
#define REPORT_ID_CIR 0x04

struct uart_hid_desc_override {
	__u8 *uart_hid_desc_buffer;
	__u8 *hid_report_desc;
	unsigned int hid_report_desc_size;
	__u8 *uart_name;
};

static __u8 uart_hid_desc[] = {
	0x1e, 0x00, /* Length of descriptor                 */
	0x00, 0x01, /* Version of descriptor                */
	0x8A, 0x00, /* Length of report descriptor          */
	0x02, 0x00, /* Location of report descriptor        */
	0x03, 0x00, /* Location of input report             */
	0x0b, 0x00, /* Max input report length              */
	0x04, 0x00, /* Location of output report            */
	0x04, 0x00, /* Max output report length             */
	0x05, 0x00, /* Location of command register         */
	0x06, 0x00, /* Location of data register            */
	0xF3, 0x14, /* Vendor ID                            */
	0x00, 0x14, /* Product ID                           */
	0x00, 0x00, /* Version ID                           */
	0x00, 0x00,
	0x00, 0x00 /* Reserved                             */
};

static __u8 uart_hid_report[] = {
	// Keyboard         // 65 Bytes
	// Input report: Length LSB(0x0B), Length MSB(0x00), ID(0x01),
	// -------------------------------------------------------------------------
	//                 Button, Reserve, Key0, Key1, Key2, Key3, Key4, Key5
	// Output report: Length LSB(0x04), Length MSB(0x00), ID(0x01), LED states
	// -------------------------------------------------------------------------
	0x05, 0x01, // Usage Page (Generic Desktop),
	0x09, 0x06, // Usage (Keyboard),
	0xA1, 0x01, // Collection (Application),
	0x85, REPORT_ID_KEYBOARD, 0x05,
	0x07, // Usage Page (Key Codes);
	0x19, 0xE0, // Usage Minimum (224),
	0x29, 0xE7, // Usage Maximum (231),
	0x15, 0x00, // Logical Minimum (0),
	0x25, 0x01, // Logical Maximum (1),
	0x75, 0x01, // Report Size (1),
	0x95, 0x08, // Report Count (8),
	0x81,
	0x02, // Input (Data, Variable, Absolute), Modifier byte
	0x95, 0x01, // Report Count (1),
	0x75, 0x08, // Report Size (8),
	0x81, 0x03, // Input (Constant, Var, Abs), Reserved byte
	0x95, 0x05, // Report Count (5),
	0x75, 0x01, // Report Size (1),
	0x05, 0x08, // Usage Page (Page# for LEDs),
	0x19, 0x01, // Usage Minimum (1),
	0x29, 0x05, // Usage Maximum (5),
	0x91,
	0x02, // Output (Data, Variable, Absolute), LED report
	0x95, 0x01, // Report Count (1),
	0x75, 0x03, // Report Size (3),
	0x91,
	0x03, // Output (Constant, Var, Abs), LED report padding
	0x95, 0x06, // Report Count (6),
	0x75, 0x08, // Report Size (8),
	0x15, 0x00, // Logical Minimum (0),
	0x25, 0xAF, // Logical Maximum(175),
	0x05, 0x07, // Usage Page (Key Codes),
	0x19, 0x00, // Usage Minimum (0),
	0x29, 0xDD, // Usage Maximum (221),
	0x81, 0x00, // Input (Data, Array), Key arrays (6 bytes)
	0xC0, // End Collection
	/* multimedia  keyboard */
	0x05, 0x0C, // Usage Page (Consumer)
	0x09, 0x01, // Usage (Consumer Control)
	0xA1, 0x01, // Collection (Application)
	0x85, REPORT_ID_POWER_KEY, // Report ID (2)
	0x19, 0x00, // Usage Minimum (0)
	0x2A, 0x3C, 0x02, // Usage Maximum (572)
	0x15, 0x00, // Logical Minimum (0)
	0x26, 0x3C, 0x02, // Logical Maximum (572)
	0x75, 0x10, // Report Size (16)
	0x95, 0x01, // Report Count (1)
	0x81, 0x00, // Input  Data, Array, Absolute
	0xC0, // End Collection

	0x05, 0x01, 0x09,
	0x0C, // Usage (Wireless Radio Controls)
	0xA1, 0x01, // COLLECTION (Application)
	0x85, REPORT_ID_ENAHNCE_KEY, // Report ID (3)
	0x15, 0x00, // LOGICAL_MINIMUM (0)
	0x25, 0x01, // LOGICAL_MAXIMUM (1)
	0x09, 0xC6, // USAGE (Wireless Radio Button)
	0x95, 0x01, // REPORT_COUNT (1)
	0x75, 0x01, // REPORT_SIZE (1)
	0x81, 0x06, // INPUT (Data,Var,Rel)
	0x75, 0x07, // REPORT_SIZE (7)
	0x81, 0x03, // INPUT (Cnst,Var,Abs)
	0xC0, // End Collection
	0x05, 0x01, // Usage Page (Generic Desktop Control)
	0x09, 0x80, // Usage System)
	0xA1, 0x01, // Collection (Application)
	0x85, REPORT_ID_CIR, // Report ID (4 + OFSET)
	0x19, 0x81, // Usage Minimum (129)
	0x29, 0x83, // Usage Maximum (131)
	0x15, 0x00, // Logical Minimum (0)
	0x25, 0x01, // Logical Maximum (1)
	0x95, 0x08, // Report Count (8)
	0x75, 0x01, // Report Size (1)
	0x81, 0x02, // Input(Data, Variable, Absolute)
	0xC0, // End Collection
};
/* descriptors for the uart keyboard */
static const struct uart_hid_desc_override uart_hid_keyboard_desc = {
	.uart_hid_desc_buffer = uart_hid_desc,
	.hid_report_desc = uart_hid_report,
	.hid_report_desc_size = 138,
	.uart_name = "uart_keyboard"
};

__u8 *uart_hid_get_keyboard_hid_desc(void)
{
	return uart_hid_keyboard_desc.uart_hid_desc_buffer;
}
__u8 *uart_hid_get_keyboard_report_desc(unsigned int *size)
{
	*size = uart_hid_keyboard_desc.hid_report_desc_size;
	return uart_hid_keyboard_desc.hid_report_desc;
}
