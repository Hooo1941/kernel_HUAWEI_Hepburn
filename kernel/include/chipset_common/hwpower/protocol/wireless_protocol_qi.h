/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_protocol_qi.h
 *
 * qi protocol driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_PROTOCOL_QI_H_
#define _WIRELESS_PROTOCOL_QI_H_

#include <linux/bitops.h>

/* qi protocol command set */
#define HWQI_CMD_GET_SIGNAL_STRENGTH                0x01
#define HWQI_CMD_RX_EPT_TYPE                        0x02
#define HWQI_CMD_SET_TX_FOP                         0x03 /* set tx operating freq */
#define HWQI_CMD_GET_TX_FOP                         0x04 /* get tx operating freq */
#define HWQI_CMD_GET_TX_VERSION                     0x05 /* read tx fw version */
#define HWQI_CMD_GET_TX_IIN                         0x06 /* read tx vin */
#define HWQI_CMD_GET_TX_VIN                         0x07 /* read tx vin */
#define HWQI_CMD_SET_TX_VIN                         0x0a
#define HWQI_CMD_GET_TX_ADAPTER_TYPE                0x0b /* get adapter type */
#define HWQI_CMD_GET_RX_PRODUCT_INFO                0x0e /* get rx product info */
#define HWQI_CMD_SEND_RX_READY                      0x0f /* rx ready */

#define HWQI_CMD_CONFIRM_START_CE                   0x10
#define HWQI_CMD_CONFIRM_STOP_CE                    0x11
#define HWQI_CMD_SEND_SN                            0x12 /* sn3~sn0 */
#define HWQI_CMD_SN7_4                              0x13
#define HWQI_CMD_SN11_8                             0x14
#define HWQI_CMD_SN15_12                            0x15
#define HWQI_CMD_SEND_BATT_TEMP                     0x16 /* send temp */
#define HWQI_CMD_SEND_BATT_CAPACITY                 0x17 /* send battery voltage */
#define HWQI_CMD_SET_CURRENT_LIMIT                  0x18 /* confirm 300ma */
#define HWQI_CMD_LIGHTSTRAP_CTRL                    0x19
#define HWQI_CMD_SEND_RX_EVT                        0x1a
#define HWQI_CMD_OPEN_VBUS                          0x1b
#define HWQI_CMD_VBUS_TO_VBATT                      0x1c
#define HWQI_CMD_SEND_RX_VOUT                       0x1d
#define HWQI_CMD_SEND_RX_IOUT                       0x1e
#define HWQI_CMD_RX_BOOST_SUCC                      0x1f /* rx boost success */

#define HWQI_CMD_CERT_SUCC                          0x20 /* rx auth success */
#define HWQI_CMD_CERT_FAIL                          0x21 /* rx auth fail */
#define HWQI_CMD_CHARGER_IC_IIN                     0x22
#define HWQI_CMD_SEND_CHRG_MODE                     0x23

#define HWQI_CMD_TX_START_COUNT_CE                  0x32
#define HWQI_CMD_TX_STOP_COUNT_CE                   0x33
#define HWQI_CMD_GET_TX_TEMP                        0x34
#define HWQI_CMD_GET_TX_COIL_NO                     0x35
#define HWQI_CMD_START_CERT                         0x36 /* send radom3~0 */
#define HWQI_CMD_SEND_RAMDOM7_4                     0x37
#define HWQI_CMD_GET_HASH                           0x38 /* get hash3~0 */
#define HWQI_CMD_GET_HASH7_4                        0x39
#define HWQI_CMD_GET_GET_HANDSHAKE_TIME             0x3a
#define HWQI_CMD_GET_TX_ID                          0x3b /* handshake */
#define HWQI_CMD_GET_CABLE_TYPE                     0x3c
#define HWQI_CMD_SET_ATE_TEST_MODE                  0x3d
#define HWQI_CMD_USER_ID                            0x3f

#define HWQI_CMD_GET_ERROR_TEMP                     0x40
#define HWQI_CMD_GET_TX_CAP                         0x41 /* get tx capability */
#define HWQI_CMD_GET_TX_MAX_VIN_IIN                 0x42
#define HWQI_CMD_SEND_CHRG_STATE                    0x43 /* charger state */
#define HWQI_CMD_FIX_TX_FOP                         0x44 /* set tx fix freq */
#define HWQI_CMD_UNFIX_TX_FOP                       0x45 /* set tx not fix freq */
#define HWQI_CMD_GET_TX_COIL_COUNT                  0x46
#define HWQI_CMD_GET_TX_COIL_NUMBER                 0x47
#define HWQI_CMD_SEND_FOD_STATUS                    0x48 /* fod status packet */
#define HWQI_CMD_GET_GET_TX_EXT_CAP                 0x49 /* get tx extra capability */
#define HWQI_CMD_TX_ALARM                           0x4a
#define HWQI_CMD_PLOSS_ALARM_THRESHOLD              0x4b
#define HWQI_CMD_GET_EPT_TYPE                       0x4d
#define HWQI_CMD_GET_TX_FOP_RANGE                   0x4f
#define HWQI_CMD_SEND_FOD_KM                        0x88 /* fod km */

/* for wireless accessory */
#define HWQI_CMD_SEND_BT_MAC1                       0x52 /* rx to tx */
#define HWQI_CMD_SEND_BT_MAC2                       0x53 /* rx to tx */
#define HWQI_CMD_SEND_BT_MODEL_ID                   0x54 /* rx to tx */
#define HWQI_CMD_GET_RX_PMAX                        0x55 /* get rx max power */

#define HWQI_CMD_GET_TX_MAX_PWR                     0x61
#define HWQI_CMD_SET_FAN_SPEED_LIMIT                0x69
#define HWQI_CMD_GET_RPP_FORMAT                     0x6b
#define HWQI_CMD_SET_RX_MAX_POWER                   0x6c
#define HWQI_CMD_SEND_START_PREVFOD_CHK             0x6d
#define HWQI_CMD_RX_READY_MISSING                   0x6e
#define HWQI_CMD_SEND_RX_PRODUCT_ID                 0x6f
#define HWQI_CMD_GET_ID_PKT                         0x71
#define HWQI_CMD_GET_RX_EPT                         0xaa

#define HWQI_CMD_ACK_HEAD                           0x1e
#define HWQI_CMD_ACK_BST_ERR                        0xf0
#define HWQI_CMD_NACK                               0xf5
#define HWQI_CMD_ACK                                0xff

/* qi protocol handshake */
#define HWQI_HANDSHAKE_ID_HIGH                      0x88
#define HWQI_HANDSHAKE_ID_LOW                       0x66
#define HWQI_HANDSHAKE_ID_HW                        0x8866
#define HWQI_HANDSHAKE_ID_HW_SLE                    0x88

/* qi protocol ack */
#define HWQI_ACK_CMD_OFFSET                         0

/* cmd 0x02, 0xA0-0xFF is private */
#define HWQI_EPT_UNKNOWN                            0x00
#define HWQI_EPT_CHRG_COMPLETE                      0x01
#define HWQI_EPT_INTERNAL_FAULT                     0x02
#define HWQI_EPT_OTP                                0x03
#define HWQI_EPT_OVP                                0x04
#define HWQI_EPT_OCP                                0x05
#define HWQI_EPT_BATT_FAILURE                       0x06
#define HWQI_EPT_RESERVED                           0x07
#define HWQI_EPT_NO_RESPONSE                        0x08
#define HWQI_EPT_ERR_VRECT                          0xA0
#define HWQI_EPT_ERR_VOUT                           0xA1

/* cmd 0x05 */
#define HWQI_ACK_TX_FWVER_LEN                       5
#define HWQI_ACK_TX_FWVER_S                         1
#define HWQI_ACK_TX_FWVER_E                         4

/* cmd 0x0b */
#define HWQI_ACK_TX_ADP_TYPE_LEN                    2
#define HWQI_ACK_TX_ADP_TYPE_OFFSET                 1

/* cmd 0x0e */
enum hwqi_type {
	HWQI_UNKNOWN     = 0x00,
	HWQI_PHONE       = 0x01,
	HWQI_HEADSET     = 0x02,
	HWQI_PAD         = 0x03,
	HWQI_WATCH       = 0x04,
	HWQI_POWER_BANK  = 0x05,
	HWQI_WRISTBAND   = 0x06,
	HWQI_LIGHTSTRAP  = 0x07,
	HWQI_PEN         = 0x08,
	HWQI_KB          = 0x09,
	HWQI_COOLINGCASE = 0x0A,
};

enum hwqi_pen_id {
	HWQI_PEN_CD52ID = 0x01, /* for MRR, Wagner, Debussy */
	HWQI_PEN_CD54ID = 0x02, /* for Marx, Marx 5G, MarxR, Wagner, Debussy */
};

enum hwqi_kb_id {
	HWQI_KB_MATEPADID = 1,
};

/* cmd 0x19 */
#define HWQI_PARA_LIGHTSTRAP_CTRL_MSG_LEN           1
#define HWQI_PARA_LIGHTSTRAP_LIGHT_UP_MSG           0x01

/* cmd 0x1a */
#define HWQI_PARA_RX_EVT_LEN                        1

/* cmd 0x1d */
#define HWQI_PARA_RX_VOUT_LEN                       2

/* cmd 0x1e */
#define HWQI_PARA_RX_IOUT_LEN                       2

/* cmd 0x23 */
#define HWQI_PARA_CHARGE_MODE_LEN                   1

/* cmd 0x36 & 0x37 */
#define HWQI_PARA_RANDOM_LEN                        8
#define HWQI_PARA_KEY_INDEX_OFFSET                  0
#define HWQI_PARA_RANDOM_S_OFFSET                   1
#define HWQI_PARA_RANDOM_E_OFFSET                   7
#define HWQI_PARA_RANDOM_GROUP_LEN                  4

/* cmd 0x38 & 0x39 */
#define HWQI_ACK_HASH_LEN                           10
#define HWQI_ACK_HASH_S_OFFSET                      1
#define HWQI_ACK_HASH_E_OFFSET                      4
#define HWQI_ACK_HASH_GROUP_LEN                     5

/* cmd 0x3b */
#define HWQI_PARA_TX_ID_LEN                         2
#define HWQI_ACK_TX_ID_LEN                          3
#define HWQI_ACK_TX_ID_S_OFFSET                     1
#define HWQI_ACK_TX_ID_E_OFFSET                     2

/* cmd 0x3c */
#define HWQI_PARA_TX_CABLE_TYPE_LEN                 3
#define HWQI_PARA_TX_CABLE_TYPE_OFFSET              1
#define HWQI_PARA_TX_CABLE_IOUT_OFFSET              2
#define HWQI_PARA_TX_CABLE_TYPE_BASE                100 /* base 100mA */

/* cmd 0x41 */
#define HWQI_PARA_TX_CAP_LEN                        5
#define HWQI_PARA_TX_MAIN_CAP_S                     1
#define HWQI_PARA_TX_MAIN_CAP_E                     4
#define HWQI_PARA_TX_CAP_VOUT_STEP                  100
#define HWQI_PARA_TX_CAP_IOUT_STEP                  100
#define HWQI_PARA_TX_CAP_CABLE_OK_MASK              BIT(0)
#define HWQI_PARA_TX_CAP_CAN_BOOST_MASK             BIT(1)
#define HWQI_PARA_TX_CAP_EXT_TYPE_MASK              (BIT(2) | BIT(3))
#define HWQI_PARA_TX_EXT_TYPE_CAR                   0x4 /* bit[3:2]=01b */
#define HWQI_PARA_TX_EXT_TYPE_PWR_BANK              0x8 /* bit[3:2]=10b */
#define HWQI_PARA_TX_CAP_CERT_MASK                  BIT(4)
#define HWQI_PARA_TX_CAP_SUPPORT_SCP_MASK           BIT(5)
#define HWQI_PARA_TX_CAP_SUPPORT_12V_MASK           BIT(6)
#define HWQI_PARA_TX_CAP_SUPPORT_EXTRA_MASK         BIT(7)

/* cmd 0x42 */
#define HWQI_PARA_TX_MAX_ADAPTER_LEN                5
#define HWQI_PARA_TX_MAX_ADAPTER_VOUT_S             1
#define HWQI_PARA_TX_MAX_ADAPTER_VOUT_E             2
#define HWQI_PARA_TX_MAX_ADAPTER_IOUT_S             3
#define HWQI_PARA_TX_MAX_ADAPTER_IOUT_E             4

/* cmd 0x43 */
#define HWQI_PARA_CHARGE_STATE_LEN                  1
#define HWQI_CHRG_STATE_FULL                        BIT(0)
#define HWQI_CHRG_STATE_DONE                        BIT(1)

/* cmd 0x44 */
#define HWQI_PARA_TX_FOP_LEN                        2
#define HWQI_FIXED_FOP_MAX                          148
#define HWQI_FIXED_FOP_MIN                          120
#define HWQI_FIXED_HIGH_FOP_MAX                     217
#define HWQI_FIXED_HIGH_FOP_MIN                     211

/* cmd 0x48 */
#define HWQI_PARA_FOD_STATUS_LEN                    2

/* cmd 0x49 */
#define HWQI_PARA_TX_EXT_CAP_LEN                    5
#define HWQI_PARA_TX_EXT_CAP_S                      1
#define HWQI_PARA_TX_EXT_CAP_E                      4
#define HWQI_PARA_TX_EXT_CAP_SUPPORT_FAN_MASK       BIT(2)
#define HWQI_PARA_TX_EXT_CAP_SUPPORT_TEC_MASK       BIT(3)
#define HWQI_PARA_TX_EXT_CAP_SUPPORT_QVAL_MASK      BIT(4)
#define HWQI_PARA_TX_EXT_CAP_SUPPORT_GET_EPT_MASK   BIT(5)
#define HWQI_PARA_TX_EXT_CAP_SUPPORT_FOP_RANGE_MASK BIT(7)
#define HWQI_PARA_TX_EXT_CAP_VPA_MAX_MASK           (BIT(5) | BIT(6) | BIT(7))
#define HWQI_PARA_TX_EXT_CAP_VPA_MAX_SHIFT          5
#define HWQI_PARA_TX_EXT_CAP_SUPPORT_FOD_KM_MASK    BIT(4)
#define HWQI_PARA_TX_EXT_CAP_DC_PWR_MASK            BIT(0)

/* cmd 0x4a */
#define HWQI_TX_ALARM_LEN                           4
#define HWQI_TX_ALARM_SRC                           0
#define HWQI_TX_ALARM_PLIM                          1
#define HWQI_TX_ALARM_VLIM                          2
#define HWQI_TX_ALARM_RSVD                          3
#define HWQI_TX_ALARM_PLIM_STEP                     1000 /* mW */
#define HWQI_TX_ALARM_VLIM_STEP                     300 /* mV */

/* cmd 0x4d */
#define HWQI_ACK_EPT_TYPE_LEN                       3
#define HWQI_ACK_EPT_TYPE_S_OFFSET                  1
#define HWQI_ACK_EPT_TYPE_E_OFFSET                  2

/* cmd 0x4f */
#define HWQI_PARA_TX_FOP_RANGE_LEN                  5
#define HWQI_PARA_TX_FOP_RANGE_BASE                 100
#define HWQI_PARA_TX_FOP_RANGE_EXT_LIMIT            3

/* cmd 0x55 */
#define HWQI_SEND_RX_PMAX_UNIT                      5 /* 0.5w per bit */
#define HWQI_EARPHONE_BOX_MIN_PWR                   25 /* 2.5w */

/* cmd 0x61 */
#define HWQI_PARA_TX_MAX_PWR_LEN                    3
#define HWQI_PARA_TX_MAX_PWR_S                      1
#define HWQI_PARA_TX_MAX_PWR_E                      2
#define HWQI_PARA_TX_MAX_PWR_BASE                   100 /* base 100mW */

/* cmd 0x69 */
#define HWQI_PARA_FAN_SPEED_LIMIT_LEN               1

/* cmd 0x6b */
#define HWQI_ACK_RPP_FORMAT_LEN                     2
#define HWQI_ACK_RPP_FORMAT_OFFSET                  1
#define HWQI_ACK_RPP_FORMAT_8BIT                    0
#define HWQI_ACK_RPP_FORMAT_24BIT                   1
#define HWQI_FSK_RPP_FORMAT_LEN                     1

/* cmd 0x6c */
#define HWQI_PARA_RX_PMAX_LEN                       1
#define HWQI_PARA_RX_PMAX_OFFSET                    0
#define HWQI_PARA_RX_PMAX_UNIT                      5 /* 5w per bit, max 75w */
#define HWQI_PARA_RX_PMAX_SHIFT                     4 /* high 4 bits, 0b1111 xxxx */

/* cmd 0x6f */
#define HWQI_PARA_RX_PRODUCT_ID_LEN                 1
#define HWQI_WITHOUT_PREVFOD_RXID_BEGIN             0XE0

/* cmd 0x71 */
#define HWQI_PARA_MANUFATURER_ID_HW                 0x0060
#define HWQI_PARA_ACC_SUPPORT_SLE                   0x7F00

/* cmd 0x88 */
#define HWQI_PARA_FOD_KM_LEN                        2

/*
 * ask: rx to tx
 * byte[0]: header, byte[1]: command, byte[2~5]: data
 * byte[0]: cmd, byte[1~5]: data
 */
#define HWQI_ASK_PACKET_LEN                         6
#define HWQI_ASK_PACKET_HDR                         0
#define HWQI_ASK_PACKET_CMD                         1
#define HWQI_ASK_PACKET_DAT0                        1
#define HWQI_ASK_PACKET_DAT1                        2
#define HWQI_ASK_PACKET_DAT2                        3
#define HWQI_ASK_PACKET_DAT3                        4
#define HWQI_ASK_PACKET_DAT4                        5
#define HWQI_ASK_PACKET_DAT_LEN                     4

#define HWQI_ASK_PACKET_HDR_MSG_SIZE_1_BYTE         0x18
#define HWQI_ASK_PACKET_HDR_MSG_SIZE_2_BYTE         0x28
#define HWQI_ASK_PACKET_HDR_MSG_SIZE_3_BYTE         0x38
#define HWQI_ASK_PACKET_HDR_MSG_SIZE_4_BYTE         0x48
#define HWQI_ASK_PACKET_HDR_MSG_SIZE_5_BYTE         0x58

#define HWQI_FSK_PKT_CMD_MSG_SIZE_1_BYTE            0x1f
#define HWQI_FSK_PKT_CMD_MSG_SIZE_2_BYTE            0x2f
#define HWQI_FSK_PKT_CMD_MSG_SIZE_3_BYTE            0x3f
#define HWQI_FSK_PKT_CMD_MSG_SIZE_4_BYTE            0x4f
#define HWQI_FSK_PKT_CMD_MSG_SIZE_5_BYTE            0x5f

#define HWQI_PKT_LEN                                6
#define HWQI_PKT_HDR                                0
#define HWQI_PKT_CMD                                1
#define HWQI_PKT_DATA                               2 /* data: B2-B5 */

/* for wireless accessory */
#define HWQI_ACC_TX_DEV_MAC_LEN                     6
#define HWQI_ACC_TX_DEV_MODELID_LEN                 3
#define HWQI_ACC_OFFSET0                            0
#define HWQI_ACC_OFFSET1                            1
#define HWQI_ACC_OFFSET2                            2
#define HWQI_ACC_OFFSET3                            3
#define HWQI_ACC_OFFSET4                            4
#define HWQI_ACC_OFFSET5                            5

/* for wireless tx attr */
#define HWQI_TX_SUPPORT_SLE                         BIT(0)
#define HWQI_TX_SUPPORT_PREVFOD_CHK                 BIT(1)

/* for wireless tx_type */
#define HWQI_TX_TYPE_CP85                           0x8866
#define HWQI_TX_TYPE_CP60                           0x8867
#define HWQI_TX_TYPE_CP61                           0x8868
#define HWQI_TX_TYPE_CP62_LX                        0x8e69
#define HWQI_TX_TYPE_CP62_XW                        0x8f69
#define HWQI_TX_TYPE_CP62R                          0x886a
#define HWQI_TX_TYPE_CP39S                          0x8c68
#define HWQI_TX_TYPE_CP39S_HK                       0x8d68
#define HWQI_TX_FW_CP39S_HK                         0x0303688d
#define HWQI_TX_TYPE_CK30_LX                        0x906a
#define HWQI_TX_TYPE_CK30_LP                        0x916a
#define HWQI_TX_TYPE_W081                           0x8874
#define HWQI_TX_TYPE_K080                           0x8875

/* for wireless tx certification */
#define HWQI_RX_RANDOM_LEN                          8
#define HWQI_TX_CIPHERKEY_LEN                       8
#define HWQI_TX_CIPHERKEY_FSK_LEN                   4

enum hwqi_tx_cap_info {
	HWQI_TX_CAP_TYPE = 1,
	HWQI_TX_CAP_VOUT_MAX,
	HWQI_TX_CAP_IOUT_MAX,
	HWQI_TX_CAP_ATTR,
};

enum hwqi_tx_extra_cap_info {
	HWQI_TX_EXTRA_CAP_ATTR1 = 1,
	HWQI_TX_EXTRA_CAP_ATTR2,
	HWQI_TX_EXTRA_CAP_ATTR3,
	HWQI_TX_EXTRA_CAP_ATTR4,
};

enum hwqi_tx_fop_range_info {
	HWQI_TX_FOP_RANGE_BASE_MIN = 1,
	HWQI_TX_FOP_RANGE_BASE_MAX,
	HWQI_TX_FOP_RANGE_EXT1,
	HWQI_TX_FOP_RANGE_EXT2,
};

enum hwqi_tx_max_power_info {
	HWQI_TX_MAX_POWER_CP85 = 13333, /* 13333 * 0.75 = 10W */
	HWQI_TX_MAX_POWER_CP60 = 20000, /* 20000 * 0.75 = 15W */
	HWQI_TX_MAX_POWER_CP61 = 36000, /* 36000 * 0.75 = 27W */
	HWQI_TX_MAX_POWER_CP62 = 53333, /* 53333 * 0.75 = 40W */
};

enum hwqi_rx_product_type {
	HWQI_RX_TYPE_UNKNOWN = 0,
	HWQI_RX_TYPE_CELLPHONE,
	HWQI_RX_TYPE_EARPHONE_BOX,
	HWQI_RX_TYPE_TABLET_PC,
	HWQI_RX_TYPE_WATCH,
	HWQI_RX_TYPE_POWER_BANK,
	HWQI_RX_TYPE_BRACELET,
	HWQI_RX_TYPE_LIGHT_STRAP,
};

struct hwqi_device_info {
	u8 tx_fwver[HWQI_ACK_TX_FWVER_LEN - 1];
	u8 tx_main_cap[HWQI_PARA_TX_CAP_LEN - 1];
	u8 tx_ext_cap[HWQI_PARA_TX_EXT_CAP_LEN - 1];
	int tx_type;
};

struct hwqi_acc_device_info {
	u8 dev_state;
	u8 dev_no;
	u8 dev_mac[HWQI_ACC_TX_DEV_MAC_LEN];
	u8 dev_model_id[HWQI_ACC_TX_DEV_MODELID_LEN];
	u8 dev_submodel_id;
	u8 dev_version;
	u8 dev_business;
	u8 dev_info_cnt;
	u8 support_sle;
};

struct hwqi_cipherkey_info {
	u8 rx_random[HWQI_RX_RANDOM_LEN];
	u8 tx_cipherkey[HWQI_TX_CIPHERKEY_LEN];
	u8 rx_random_len;
};

struct hwqi_handle {
	u8 (*get_ask_hdr)(int data_len);
	u8 (*get_fsk_hdr)(int data_len);
	int (*hdl_qi_ask_pkt)(void *dev_data);
	int (*hdl_non_qi_ask_pkt)(void *dev_data);
	void (*hdl_non_qi_fsk_pkt)(u8 *pkt, int len, void *dev_data);
};

struct hwqi_ops {
	const char *chip_name;
	void *dev_data;
	int (*send_msg)(u8 cmd, u8 *data, int data_len, void *dev_data);
	int (*send_msg_with_ack)(u8 cmd, u8 *data, int data_len, void *dev_data);
	int (*receive_msg)(u8 *data, int data_len, void *dev_data);
	int (*send_fsk_msg)(u8 cmd, u8 *data, int data_len, void *dev_data);
	int (*auto_send_fsk_with_ack)(u8 cmd, u8 *data, int data_len, void *dev_data);
	int (*get_ask_packet)(u8 *data, int data_len, void *dev_data);
	int (*get_chip_fw_version)(u8 *data, int data_len, void *dev_data);
	int (*get_tx_id_pre)(void *dev_data); /* process non protocol flow */
	int (*set_rpp_format_post)(u8 pmax, int mode, void *dev_data);
};

struct hwqi_dev {
	struct device *dev;
	struct hwqi_device_info info;
	struct hwqi_acc_device_info acc_info;
	struct hwqi_cipherkey_info cipherkey_info;
	int dev_id;
	u16 dev_attr;
	struct hwqi_ops *p_ops;
};

#ifdef CONFIG_WIRELESS_PROTOCOL_QI
int hwqi_ops_register(struct hwqi_ops *ops, unsigned int ic_type);
struct hwqi_handle *hwqi_get_handle(void);
#else
static inline int hwqi_ops_register(struct hwqi_ops *ops, unsigned int ic_type)
{
	return -ENOTSUPP;
}

static inline struct hwqi_handle *hwqi_get_handle(void)
{
	return NULL;
}
#endif /* CONFIG_WIRELESS_PROTOCOL_QI */

#endif /* _WIRELESS_PROTOCOL_QI_H_ */