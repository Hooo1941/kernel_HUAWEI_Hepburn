/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_protocol_ufcs.h
 *
 * ufcs protocol driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _ADAPTER_PROTOCOL_UFCS_H_
#define _ADAPTER_PROTOCOL_UFCS_H_

#include <linux/device.h>
#include <linux/completion.h>
#include <linux/notifier.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>

#define HWUFCS_POWER_READY_RETRY              14
#define HWUFCS_SEND_MSG_MAX_RETRY             3
#define HWUFCS_POWER_CURVE_SIZE               32

enum hwufcs_wait_type {
	HWUFCS_WAIT_CRC_ERROR = 0x1,
	HWUFCS_WAIT_SEND_PACKET_COMPLETE = 0x2,
	HWUFCS_WAIT_DATA_READY = 0x4,
	HWUFCS_ACK_RECEIVE_TIMEOUT = 0x8,
};

enum hwufcs_detect_error_code {
	HWUFCS_DETECT_OTHER = -1,
	HWUFCS_DETECT_SUCC = 0,
	HWUFCS_DETECT_FAIL = 1,
};

/*
 * message header data as below:
 * bit0~2: message type
 * bit3~8: protocol version
 * bit9~12: message number
 * bit13~15: device address
 */
#define HWUFCS_MSG_PROT_VERSION               0x1
#define HWUFCS_MSG_MAX_COUNTER                16
#define HWUFCS_MSG_MAX_BUFFER_SIZE            256
#define HWUFCS_MSG_VERSION_ID                 0x0001
#define HWUFCS_ADP_TYPE_FCR_66W               0x1

#define HWUFCS_HDR_MASK_MSG_TYPE              0x7
#define HWUFCS_HDR_MASK_PROT_VERSION          0x3f
#define HWUFCS_HDR_MASK_MSG_NUMBER            0xf
#define HWUFCS_HDR_MASK_DEV_ADDRESS           0x7

#define HWUFCS_HDR_HEADER_LEN                 2
#define HWUFCS_HDR_HEADER_H_OFFSET            0
#define HWUFCS_HDR_HEADER_L_OFFSET            1

#define HWUFCS_MSG_CMD_LEN                    1
#define HWUFCS_MSG_LENGTH_LEN                 1
#define HWUFCS_MSG_VER_ID_LEN                 2

/* control/data msg body */
#define HWUFCS_CTRL_DATA_MSG_MAX_BUF_SIZE     254
#define HWUFCS_CTRL_DATA_MSG_CMD_OFFSET       2
#define HWUFCS_CTRL_DATA_MSG_LEN_OFFSET       3
#define HWUFCS_CTRL_DATA_MSG_DATA_OFFSET      4

/* vendor defined msg body */
#define HWUFCS_VD_MSG_MAX_BUF_SIZE            251
#define HWUFCS_VD_MSG_VER_ID_H_OFFSET         2
#define HWUFCS_VD_MSG_VER_ID_L_OFFSET         3
#define HWUFCS_VD_MSG_MSG_LEN_OFFSET          4
#define HWUFCS_VD_MSG_CMD_H_OFFSET            5
#define HWUFCS_VD_MSG_CMD_L_OFFSET            6
#define HWUFCS_VD_MSG_DATA_OFFSET             5

#define HWUFCS_VDR_MASK_MSG_CMD               0x1fff
#define HWUFCS_VDR_MASK_MSG_CMD_TYPE          0x7

#define HWUFCS_VDR_HEADER_LEN  2

enum hwufcs_header_data_shift {
	HWUFCS_HDR_SHIFT_MSG_TYPE = 0,
	HWUFCS_HDR_SHIFT_PROT_VERSION = 3,
	HWUFCS_HDR_SHIFT_MSG_NUMBER = 9,
	HWUFCS_HDR_SHIFT_DEV_ADDRESS = 13,
};

enum hwufcs_message_type {
	HWUFCS_MSG_TYPE_BEGIN,
	HWUFCS_MSG_TYPE_CONTROL = HWUFCS_MSG_TYPE_BEGIN,
	HWUFCS_MSG_TYPE_DATA,
	HWUFCS_MSG_TYPE_VENDOR_DEFINED,
	HWUFCS_MSG_TYPE_END,
};

enum hwufcs_vdm_header_data_shift {
	HWUFCS_VDR_SHIFT_MSG_CMD = 0,
	HWUFCS_VDR_SHIFT_MSG_CMD_TYPE = 13,
};

enum hwufcs_vdm_message_type {
	HWUFCS_VDM_TYPE_BEGIN,
	HWUFCS_VDM_TYPE_CONTROL = HWUFCS_VDM_TYPE_BEGIN,
	HWUFCS_VDM_TYPE_DATA,
	HWUFCS_VDM_TYPE_END,
};

enum hwufcs_device_address {
	HWUFCS_DEV_ADDRESS_SOURCE = 0x01,
	HWUFCS_DEV_ADDRESS_SINK,
	HWUFCS_DEV_ADDRESS_CABLE_ELECTRONIC_LABEL,
};

enum hwufcs_control_message {
	HWUFCS_CTL_MSG_BEGIN = 0x0,
	HWUFCS_CTL_MSG_PING = HWUFCS_CTL_MSG_BEGIN,
	HWUFCS_CTL_MSG_ACK = 0x1,
	HWUFCS_CTL_MSG_NCK = 0x2,
	HWUFCS_CTL_MSG_ACCEPT = 0x3,
	HWUFCS_CTL_MSG_SOFT_RESET = 0x4,
	HWUFCS_CTL_MSG_POWER_READY = 0x5,
	HWUFCS_CTL_MSG_GET_OUTPUT_CAPABILITIES = 0x6,
	HWUFCS_CTL_MSG_GET_SOURCE_INFO = 0x7,
	HWUFCS_CTL_MSG_GET_SINK_INFO = 0x8,
	HWUFCS_CTL_MSG_GET_CABLE_INFO = 0x9,
	HWUFCS_CTL_MSG_GET_DEVICE_INFO = 0xa,
	HWUFCS_CTL_MSG_GET_ERROR_INFO = 0xb,
	HWUFCS_CTL_MSG_DETECT_CABLE_INFO = 0xc,
	HWUFCS_CTL_MSG_START_CABLE_DETECT = 0xd,
	HWUFCS_CTL_MSG_END_CABLE_DETECT = 0xe,
	HWUFCS_CTL_MSG_EXIT_HWUFCS_MODE = 0xf,
	HWUFCS_CTL_MSG_END,
};

enum hwufcs_data_message {
	HWUFCS_DATA_MSG_BEGIN = 0x1,
	HWUFCS_DATA_MSG_OUTPUT_CAPABILITIES = HWUFCS_DATA_MSG_BEGIN,
	HWUFCS_DATA_MSG_REQUEST = 0x2,
	HWUFCS_DATA_MSG_SOURCE_INFO = 0x3,
	HWUFCS_DATA_MSG_SINK_INFO = 0x4,
	HWUFCS_DATA_MSG_CABLE_INFO = 0x5,
	HWUFCS_DATA_MSG_DEVICE_INFO = 0x6,
	HWUFCS_DATA_MSG_ERROR_INFO = 0x7,
	HWUFCS_DATA_MSG_CONFIG_WATCHDOG = 0x8,
	HWUFCS_DATA_MSG_REFUSE = 0x9,
	HWUFCS_DATA_MSG_VERIFY_REQUEST = 0xa,
	HWUFCS_DATA_MSG_VERIFY_RESPONSE = 0xb,
	HWUFCS_DATA_MSG_POWER_CHANGE = 0xc,
	HWUFCS_DATA_MSG_TEST_REQUEST = 0xff,
	HWUFCS_DATA_MSG_END,
};

enum hwufcs_vdm_control_message {
	HWUFCS_VDM_CTL_MSG_BEGIN = 0x1,
	HWUFCS_VDM_CTL_MSG_GET_SOURCE_ID = HWUFCS_VDM_CTL_MSG_BEGIN,
	HWUFCS_VDM_CTL_MSG_GET_SCPB_POWER,
	HWUFCS_VDM_CTL_MSG_END,
};

enum hwufcs_vdm_data_message {
	HWUFCS_VDM_DATA_MSG_BEGIN = 0x1,
	HWUFCS_VDM_DATA_MSG_SOURCE_ID = HWUFCS_VDM_DATA_MSG_BEGIN,
	HWUFCS_VDM_DATA_MSG_SCPB_POWER,
	HWUFCS_VDM_DATA_MSG_END,
};

/*
 * capabilities data message as below:
 * bit0~7: min output current
 * bit8~23: max output current
 * bit24~39: min output voltage
 * bit40~55: max output voltage
 * bit56: voltage adjust step
 * bit57~59: current adjust step
 * bit60~63: output mode
 */
#define HWUFCS_CAP_MASK_MIN_CURR              0xff
#define HWUFCS_CAP_MASK_MAX_CURR              0xffff
#define HWUFCS_CAP_MASK_MIN_VOLT              0xffff
#define HWUFCS_CAP_MASK_MAX_VOLT              0xffff
#define HWUFCS_CAP_MASK_VOLT_STEP             0x1
#define HWUFCS_CAP_MASK_CURR_STEP             0x7
#define HWUFCS_CAP_MASK_OUTPUT_MODE           0xf

#define HWUFCS_CAP_UNIT_CURR                  10 /* ma */
#define HWUFCS_CAP_UNIT_VOLT                  10 /* mv */

#define HWUFCS_CAP_MAX_OUTPUT_MODE            15

enum hwufcs_capabilities_data_shift {
	HWUFCS_CAP_SHIFT_MIN_CURR = 0,
	HWUFCS_CAP_SHIFT_MAX_CURR = 8,
	HWUFCS_CAP_SHIFT_MIN_VOLT = 24,
	HWUFCS_CAP_SHIFT_MAX_VOLT = 40,
	HWUFCS_CAP_SHIFT_VOLT_STEP = 56,
	HWUFCS_CAP_SHIFT_CURR_STEP = 57,
	HWUFCS_CAP_SHIFT_OUTPUT_MODE = 60,
};

struct hwufcs_capabilities_data {
	u16 min_curr;
	u16 max_curr;
	u16 min_volt;
	u16 max_volt;
	u16 volt_step;
	u16 curr_step;
	u16 output_mode;
};

/*
 * output power range rules as follows:
 * 5V: 3.4V~5.5V
 * 10V: 5.5V~11V
 * 20V: 11V~21V
 * 30V: 21V~36V
 */
#define HWUFCS_5V_VOLT_MIN                    3400 /* mv */
#define HWUFCS_5V_VOLT_MAX                    5500 /* mv */
#define HWUFCS_10V_VOLT_MIN                   5500 /* mv */
#define HWUFCS_10V_VOLT_MAX                   11000 /* mv */
#define HWUFCS_20V_VOLT_MIN                   11000 /* mv */
#define HWUFCS_20V_VOLT_MAX                   21000 /* mv */
#define HWUFCS_30V_VOLT_MIN                   21000 /* mv */
#define HWUFCS_30V_VOLT_MAX                   36000 /* mv */

/*
 * request data message as below:
 * bit0~15: request output current
 * bit16~31: request output voltage
 * bit32~59: reserved
 * bit60~63: output mode
 */
#define HWUFCS_REQ_MASK_OUTPUT_CURR           0xffff
#define HWUFCS_REQ_MASK_OUTPUT_VOLT           0xffff
#define HWUFCS_REQ_MASK_OUTPUT_MODE           0xf

#define HWUFCS_REQ_BASE_OUTPUT_MODE           1
#define HWUFCS_REQ_MIN_OUTPUT_MODE            1
#define HWUFCS_REQ_MAX_OUTPUT_MODE            15

#define HWUFCS_REQ_UNIT_OUTPUT_CURR           10 /* ma */
#define HWUFCS_REQ_UNIT_OUTPUT_VOLT           10 /* mv */
#define HWUFCS_REQ_LVC_OUTPUT_VOLT            5000 /* mv */
#define HWUFCS_REQ_SC_OUTPUT_VOLT             10000 /* mv */
#define HWUFCS_REQ_SC4_OUTPUT_VOLT            20000 /* mv */

enum hwufcs_request_data_shift {
	HWUFCS_REQ_SHIFT_OUTPUT_CURR = 0,
	HWUFCS_REQ_SHIFT_OUTPUT_VOLT = 16,
	HWUFCS_REQ_SHIFT_OUTPUT_MODE = 60,
};

struct hwufcs_request_data {
	u16 output_curr;
	u16 output_volt;
	u16 output_mode;
};

/*
 * source information data message as below:
 * bit0~15: current output current
 * bit16~31: current output voltage
 * bit32~39: current usb port temp
 * bit40~47: current device temp
 */
#define HWUFCS_SOURCE_INFO_MASK_OUTPUT_CURR   0xffff
#define HWUFCS_SOURCE_INFO_MASK_OUTPUT_VOLT   0xffff
#define HWUFCS_SOURCE_INFO_MASK_PORT_TEMP     0xff
#define HWUFCS_SOURCE_INFO_MASK_DEV_TEMP      0xff

#define HWUFCS_SOURCE_INFO_UNIT_CURR          10 /* ma */
#define HWUFCS_SOURCE_INFO_UNIT_VOLT          10 /* mv */

#define HWUFCS_SOURCE_INFO_TEMP_BASE          50 /* centigrade */

enum hwufcs_source_info_data_shift {
	HWUFCS_SOURCE_INFO_SHIFT_OUTPUT_CURR = 0,
	HWUFCS_SOURCE_INFO_SHIFT_OUTPUT_VOLT = 16,
	HWUFCS_SOURCE_INFO_SHIFT_PORT_TEMP = 32,
	HWUFCS_SOURCE_INFO_SHIFT_DEV_TEMP = 40,
};

struct hwufcs_source_info_data {
	u16 output_curr;
	u16 output_volt;
	int port_temp;
	int dev_temp;
};

/*
 * sink information data message as below:
 * bit0~15: current charging current
 * bit16~31: current battery voltage
 * bit32~39: current usb temp
 * bit40~47: current battery temp
 */
#define HWUFCS_SINK_INFO_MASK_BAT_CURR        0xffff
#define HWUFCS_SINK_INFO_MASK_BAT_VOLT        0xffff
#define HWUFCS_SINK_INFO_MASK_USB_TEMP        0xff
#define HWUFCS_SINK_INFO_MASK_BAT_TEMP        0xff

#define HWUFCS_SINK_INFO_UNIT_CURR            10 /* ma */
#define HWUFCS_SINK_INFO_UNIT_VOLT            10 /* mv */

#define HWUFCS_SINK_INFO_TEMP_BASE            50 /* centigrade */

enum hwufcs_sink_info_data_shift {
	HWUFCS_SINK_INFO_SHIFT_BAT_CURR = 0,
	HWUFCS_SINK_INFO_SHIFT_BAT_VOLT = 16,
	HWUFCS_SINK_INFO_SHIFT_USB_TEMP = 32,
	HWUFCS_SINK_INFO_SHIFT_BAT_TEMP = 40,
};

struct hwufcs_sink_info_data {
	u16 bat_curr;
	u16 bat_volt;
	int usb_temp;
	int bat_temp;
};

/*
 * cable information data message as below:
 * bit0~7: max loading current
 * bit8~15: max loading voltage
 * bit16~31: cable resistance
 * bit32~47: cable electronic lable vendor id
 * bit48~63: cable vendor id
 */
#define HWUFCS_CABLE_INFO_MASK_MAX_CURR       0xff
#define HWUFCS_CABLE_INFO_MASK_MAX_VOLT       0xff
#define HWUFCS_CABLE_INFO_MASK_RESISTANCE     0xffff
#define HWUFCS_CABLE_INFO_MASK_RESERVED_1     0xffff
#define HWUFCS_CABLE_INFO_MASK_RESERVED_2     0xffff

#define HWUFCS_CABLE_INFO_UNIT_CURR           1000 /* ma */
#define HWUFCS_CABLE_INFO_UNIT_VOLT           1000 /* mv */
#define HWUFCS_CABLE_INFO_UNIT_RESISTANCE     1000 /* mo */

enum hwufcs_cable_info_data_shift {
	HWUFCS_CABLE_INFO_SHIFT_MAX_CURR = 0,
	HWUFCS_CABLE_INFO_SHIFT_MAX_VOLT = 8,
	HWUFCS_CABLE_INFO_SHIFT_RESISTANCE = 16,
	HWUFCS_CABLE_INFO_SHIFT_RESERVED_1 = 32,
	HWUFCS_CABLE_INFO_SHIFT_RESERVED_2 = 48,
};

struct hwufcs_cable_info_data {
	u16 max_curr;
	u16 max_volt;
	u16 resistance;
	u16 reserved_1;
	u16 reserved_2;
};

/*
 * device information data message as below:
 * bit0~15: software version
 * bit16~31: hardware version
 * bit32~47: protocol chip vendor id
 * bit48~63: manufacture vendor id
 */
#define HWUFCS_DEV_INFO_MASK_SW_VER           0xffff
#define HWUFCS_DEV_INFO_MASK_HW_VER           0xffff
#define HWUFCS_DEV_INFO_MASK_RESERVED_1       0xffff
#define HWUFCS_DEV_INFO_MASK_RESERVED_2       0xffff

enum hwufcs_dev_info_data_shift {
	HWUFCS_DEV_INFO_SHIFT_SW_VER = 0,
	HWUFCS_DEV_INFO_SHIFT_HW_VER = 16,
	HWUFCS_DEV_INFO_SHIFT_RESERVED_1 = 32,
	HWUFCS_DEV_INFO_SHIFT_RESERVED_2 = 48,
};

struct hwufcs_dev_info_data {
	u16 sw_ver;
	u16 hw_ver;
	u16 reserved_1;
	u16 reserved_2;
};

/*
 * error information data message as below:
 * bit0: output ovp
 * bit1: output uvp
 * bit2: output ocp
 * bit3: output scp
 * bit4: usb otp
 * bit5: device otp
 * bit6: cc ovp
 * bit7: d- ovp
 * bit8: d+ ovp
 * bit9: input ovp
 * bit10: input uvp
 * bit11: over leakage current
 * bit12: input drop
 * bit13: crc error
 * bit14: watchdog overflow
 */
#define HWUFCS_ERROR_INFO_MASK_OUTPUT_OVP     0x1
#define HWUFCS_ERROR_INFO_MASK_OUTPUT_UVP     0x1
#define HWUFCS_ERROR_INFO_MASK_OUTPUT_OCP     0x1
#define HWUFCS_ERROR_INFO_MASK_OUTPUT_SCP     0x1
#define HWUFCS_ERROR_INFO_MASK_USB_OTP        0x1
#define HWUFCS_ERROR_INFO_MASK_DEVICE_OTP     0x1
#define HWUFCS_ERROR_INFO_MASK_CC_OVP         0x1
#define HWUFCS_ERROR_INFO_MASK_DMINUS_OVP     0x1
#define HWUFCS_ERROR_INFO_MASK_DPLUS_OVP      0x1
#define HWUFCS_ERROR_INFO_MASK_INPUT_OVP      0x1
#define HWUFCS_ERROR_INFO_MASK_INPUT_UVP      0x1
#define HWUFCS_ERROR_INFO_MASK_OVER_LEAKAGE   0x1
#define HWUFCS_ERROR_INFO_MASK_INPUT_DROP     0x1
#define HWUFCS_ERROR_INFO_MASK_CRC_ERROR      0x1
#define HWUFCS_ERROR_INFO_MASK_WTG_OVERFLOW   0x1

#define HWUFCS_ERROR_INFO_NORMAL              0
#define HWUFCS_ERROR_INFO_ABNORMAL            1

enum hwufcs_error_info_data_shift {
	HWUFCS_ERROR_INFO_SHIFT_OUTPUT_OVP = 0,
	HWUFCS_ERROR_INFO_SHIFT_OUTPUT_UVP = 1,
	HWUFCS_ERROR_INFO_SHIFT_OUTPUT_OCP = 2,
	HWUFCS_ERROR_INFO_SHIFT_OUTPUT_SCP = 3,
	HWUFCS_ERROR_INFO_SHIFT_USB_OTP = 4,
	HWUFCS_ERROR_INFO_SHIFT_DEVICE_OTP = 5,
	HWUFCS_ERROR_INFO_SHIFT_CC_OVP = 6,
	HWUFCS_ERROR_INFO_SHIFT_DMINUS_OVP = 7,
	HWUFCS_ERROR_INFO_SHIFT_DPLUS_OVP = 8,
	HWUFCS_ERROR_INFO_SHIFT_INPUT_OVP = 9,
	HWUFCS_ERROR_INFO_SHIFT_INPUT_UVP = 10,
	HWUFCS_ERROR_INFO_SHIFT_OVER_LEAKAGE = 11,
	HWUFCS_ERROR_INFO_SHIFT_INPUT_DROP = 12,
	HWUFCS_ERROR_INFO_SHIFT_CRC_ERROR = 13,
	HWUFCS_ERROR_INFO_SHIFT_WTG_OVERFLOW = 14,
};

struct hwufcs_error_info_data {
	u8 output_ovp;
	u8 output_uvp;
	u8 output_ocp;
	u8 output_scp;
	u8 usb_otp;
	u8 device_otp;
	u8 cc_ovp;
	u8 dminus_ovp;
	u8 dplus_ovp;
	u8 input_ovp;
	u8 input_uvp;
	u8 over_leakage;
	u8 input_drop;
	u8 crc_error;
	u8 wtg_overflow;
};

/*
 * config watchdog data message as below:
 * bit0~15: watchdog overflow time
 */
#define HWUFCS_WTG_MASK_TIME                  0xffff

#define HWUFCS_WTG_UNIT_TIME                  1000 /* ms */

enum hwufcs_wtg_data_shift {
	HWUFCS_WTG_SHIFT_TIME = 0,
};

struct hwufcs_wtg_data {
	u16 time;
};

/*
 * refuse data message as below:
 * bit0~7: reason
 * bit8~15: command number
 * bit16~18: message type
 * bit24~27: message number
 */
#define HWUFCS_REFUSE_MASK_REASON             0xff
#define HWUFCS_REFUSE_MASK_CMD_NUMBER         0xff
#define HWUFCS_REFUSE_MASK_MSG_TYPE           0x7
#define HWUFCS_REFUSE_MASK_MSG_NUMBER         0xf

#define HWUFCS_REFUSE_REASON_NOT_IDENTIFY     0x1
#define HWUFCS_REFUSE_REASON_NOT_SUPPORT      0x2
#define HWUFCS_REFUSE_REASON_DEVICE_BUSY      0x3
#define HWUFCS_REFUSE_REASON_OVER_RANGE       0x4
#define HWUFCS_REFUSE_REASON_OTHER            0x5

enum hwufcs_refuse_data_shift {
	HWUFCS_REFUSE_SHIFT_REASON = 0,
	HWUFCS_REFUSE_SHIFT_CMD_NUMBER = 8,
	HWUFCS_REFUSE_SHIFT_MSG_TYPE = 16,
	HWUFCS_REFUSE_SHIFT_MSG_NUMBER = 24,
};

struct hwufcs_refuse_data {
	u8 reason;
	u8 cmd_number;
	u8 msg_type;
	u8 msg_number;
};

/*
 * Baud rate base gear
 * default : 115200bps
 */
enum hwufcs_baud_rate_gear {
	HWUFCS_BAUD_RATE_115200_GEAR,
	HWUFCS_BAUD_RATE_57600_GEAR,
	HWUFCS_BAUD_RATE_38400_GEAR,
	HWUFCS_BAUD_RATE_END,
};

/*
 * power change data message as below:
 * bit0~15: max output current
 * bit16~19: reserve byte
 * bit20~23: output mode
 */
#define HWUFCS_POWER_CHANGE_MASK_MAX_CURR     0xffff
#define HWUFCS_POWER_CHANGE_MASK_OUTPUT_MODE  0xf
#define HWUFCS_POWER_CHANGE_MAX_OUTPUT_MODE   7
#define HWUFCS_POWER_CHANGE_DATA_SIZE         3
#define HWUFCS_POWER_CHANGE_UNIT_CURR         10 /* ma */
enum hwufcs_power_change_data_shift {
	HWUFCS_POWER_CHANGE_SHIFT_MAX_CURR = 0,
	HWUFCS_POWER_CHANGE_SHIFT_RESERVE = 16,
	HWUFCS_POWER_CHANGE_SHIFT_OUTPUT_MODE = 20,
};

struct hwufcs_power_change_data {
	u16 max_curr;
	u8 output_mode;
};

/*
 * test request data message as below:
 * bit0~7: message cmd
 * bit8~10: message type
 * bit11~13: device address
 * bit14: voltage accuracy test mode
 */
#define HWUFCS_TEST_REQ_MASK_MSG_CMD          0xff
#define HWUFCS_TEST_REQ_MASK_MSG_TYPE         0x7
#define HWUFCS_TEST_REQ_MASK_DEV_ADDRESS      0x7
#define HWUFCS_TEST_REQ_MASK_VOLT_TEST_MODE   0x1
#define HWUFCS_TEST_REQ_MASK_EN_TEST_MODE     0x1

enum hwufcs_test_request_data_shift {
	HWUFCS_TEST_REQ_SHIFT_MSG_CMD = 0,
	HWUFCS_TEST_REQ_SHIFT_MSG_TYPE = 8,
	HWUFCS_TEST_REQ_SHIFT_DEV_ADDRESS = 11,
	HWUFCS_TEST_REQ_SHIFT_VOLT_TEST_MODE = 14,
	HWUFCS_TEST_REQ_SHIFT_EN_TEST_MODE = 15,
};

struct hwufcs_test_request_data {
	u8 msg_cmd;
	u8 msg_type;
	u8 dev_address;
	u8 volt_test_mode;
	u8 en_test_mode;
};

/*
 * vdm source id data message as below:
 * bit0~15: source id
 * bit16~20: vdm msg number
 * bit20~31: vdm msg type
 */
enum hwufcs_vdm_source_id_data_shift {
	HWUFCS_VDM_SOURCE_ID_SHIFT_ID = 0,
	HWUFCS_VDM_SOURCE_ID_SHIFT_MSG_NUMBER = 16,
	HWUFCS_VDM_SOURCE_ID_SHIFT_MSG_TYPE = 20,
};

struct hwufcs_vdm_source_id_data {
	u16 source_id;
	u16 msg_number;
	u8 msg_type;
};

/*
 * vdm scpb power data message as below:
 * bit0~7: 5V power or min curr
 * bit8~23: 10V power or max curr
 * bit24~39: 15V power or min vol
 * bit40~55: 20V power or max vol
 * bit56~59: output mode number
 * bit60~63: power type:0:power gear,1:power curve
 */

#define HWUFCS_SCPB_MAX_PWR_MODE              15

#define HWUFCS_VDM_SCPB_PWR_MASK_PWR_5V       0xff
#define HWUFCS_VDM_SCPB_PWR_MASK_PWR_10V      0xffff
#define HWUFCS_VDM_SCPB_PWR_MASK_PWR_15V      0xffff
#define HWUFCS_VDM_SCPB_PWR_MASK_PWR_20V      0xffff

#define HWUFCS_VDM_SCPB_PWR_STEP              2 /* 0.5mw */

#define HWUFCS_VDM_SCPB_PWR_MASK_MIN_CURR     0xff
#define HWUFCS_VDM_SCPB_PWR_MASK_MAX_CURR     0xffff
#define HWUFCS_VDM_SCPB_PWR_MASK_MIN_VOLT     0xffff
#define HWUFCS_VDM_SCPB_PWR_MASK_MAX_VOLT     0xffff

#define HWUFCS_VDM_SCPB_PWR_UNIT_CURR         10 /* ma */
#define HWUFCS_VDM_SCPB_PWR_UNIT_VOLT         10 /* mv */

#define HWUFCS_VDM_SCPB_PWR_MASK_OUTPUT_MODE  0xf
#define HWUFCS_VDM_SCPB_PWR_MASK_PWR_TYPE     0xf

#define HWUFCS_VDM_SCPB_PWR_GEAR_TYPE         0
#define HWUFCS_VDM_SCPB_PWR_CURVE_TYPE        1

enum hwufcs_vdm_scpb_pwr_gear_data_shift {
	HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_5V = 0,
	HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_10V = 8,
	HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_15V = 24,
	HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_20V = 40,
};

enum hwufcs_vdm_scpb_pwr_curve_data_shift {
	HWUFCS_VDM_SCPB_PWR_SHIFT_MIN_CURR = 0,
	HWUFCS_VDM_SCPB_PWR_SHIFT_MAX_CURR = 8,
	HWUFCS_VDM_SCPB_PWR_SHIFT_MIN_VOLT = 24,
	HWUFCS_VDM_SCPB_PWR_SHIFT_MAX_VOLT = 40,
};

enum hwufcs_vdm_scpb_pwr_data_shift {
	HWUFCS_VDM_SCPB_PWR_SHIFT_OUTPUT_MODE = 56,
	HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_TYPE = 60,
};

enum hwufcs_error_code {
	HWUFCS_ERR_UNKNOWN = -99,
	HWUFCS_ERR_TIMEOUT,
	HWUFCS_ERR_ILLEGAL_DATA,
	HWUFCS_ERR_UNSUPPORT_DATA,
	HWUFCS_ERR_UNEXPECT_DATA,
	HWUFCS_ERR_REFUSED_DATA,
	HWUFCS_ERR_REFUSED_SUPPORT_DATA,
};

enum hwufcs_communication_state {
	HWUFCS_FAIL = -1,
	HWUFCS_OK = 0,
	HWUFCS_NEED_RETRY,
};

struct hwufcs_package_data {
	u8 msg_type;
	u8 prot_version;
	u8 msg_number;
	u8 dev_address;
	u8 cmd;
	u8 vdm_type;
	u16 vender_id;
	u8 len; /* data length */
	u8 data[HWUFCS_MSG_MAX_BUFFER_SIZE];
};

struct hwufcs_device_info {
	int support_mode; /* adapter support mode */
	u8 detect_finish_flag;
	u8 outout_capabilities_rd_flag;
	u8 dev_info_rd_flag;
	u8 cable_info_rd_flag;
	u16 output_volt;
	u16 output_curr;
	u8 output_mode;
	u8 mode_quantity;
	u8 scpb_pwr_quantity;
	struct hwufcs_capabilities_data cap[HWUFCS_CAP_MAX_OUTPUT_MODE];
	struct adp_scpb_pwr_data scpb_pwr_data[HWUFCS_SCPB_MAX_PWR_MODE];
	struct hwufcs_power_change_data change_data[HWUFCS_POWER_CHANGE_MAX_OUTPUT_MODE];
	struct hwufcs_dev_info_data dev_info;
	struct hwufcs_cable_info_data cable_info;
};

struct hwufcs_ops {
	const char *chip_name;
	void *dev_data;
	int (*detect_adapter)(void *dev_data);
	int (*write_msg)(void *dev_data, u8 *data, u8 len, u8 flag);
	int (*wait_msg_ready)(void *dev_data, u8 flag);
	int (*read_msg)(void *dev_data, u8 *data, u8 len);
	int (*end_read_msg)(void *dev_data);
	int (*soft_reset_master)(void *dev_data);
	int (*clear_rx_buff)(void *dev_data);
	int (*get_rx_len)(void *dev_data, u8 *len);
	int (*set_communicating_flag)(void *dev_data, bool flag);
	int (*config_baud_rate)(void *dev_data, int baud_rate);
	int (*hard_reset_cable)(void *dev_data);
	bool (*need_check_ack)(void *dev_data);
	bool (*ignore_detect_cable_info)(void *dev_data);
	int (*hard_reset_mask)(void *dev_data, u8 mask);
};

struct hwufcs_dev {
	struct device *dev;
	struct notifier_block event_nb;
	struct notifier_block event_ufcs_nb;
	struct hwufcs_device_info info;
	struct work_struct handle_msg_work;
	bool plugged_state;
};

#ifdef CONFIG_ADAPTER_PROTOCOL_UFCS
int hwufcs_ops_register(struct hwufcs_ops *ops);
#else
static inline int hwufcs_ops_register(struct hwufcs_ops *ops)
{
	return 0;
}
#endif /* CONFIG_ADAPTER_PROTOCOL_UFCS */

#endif /* _ADAPTER_PROTOCOL_UFCS_H_ */
