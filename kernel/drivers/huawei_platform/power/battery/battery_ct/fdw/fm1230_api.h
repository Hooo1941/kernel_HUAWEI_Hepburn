/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fm1230_api.h
 *
 * fm1230_api.c head file
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef __FM1230_API_H__
#define __FM1230_API_H__

#include "fm1230.h"
#include "../batt_aut_checker.h"

/* ow command time delay para */
#define OW_READ_ROMID_DELAY                          250
#define OW_SE_CALC_CRC_DELAY                         5
#define OW_SE_PROC_CMD_DELAY                         12
#define OW_DEV_RESET_DELAY                           500
#define OW_RESET_DELAY                               50
#define FM1230_POWER_ON                              5

/* ow commands */
#define CMD_READ_ROM                                 0x33
#define CMD_START                                    0x66
#define CMD_SEND_APDU                                0x99
#define CMD_SKIP_ROM                                 0xCC
#define CMD_RELEASE_BYTE                             0xAA

/* CRC16 result */
#define CRC16_RESULT                                 0xB001

/* result bytes */
#define RESULT_SUCCESS                               0x00
#define RESULT_RXUID_ERR                             0x11
#define RESULT_TXCMD_ERR                             0x22
#define RESULT_TXDAT_ERR                             0x33
#define RESULT_PARA_ERR                              0x44
#define RESULT_CMP1_ERR                              0x55
#define RESULT_CMP2_ERR                              0x66
#define RESULT_REST_ERR                              0x77
#define RESULT_SIGN_ERR                              0x88
#define RESULT_SESW_ERR                              0x99
#define RESULT_LOCK_ERR                              0xAA
#define RESULT_LKRET_ERR                             0xBB
#define RESULT_NONE_ERR                              0xFF
#define OWSE_XFER_ERR                                0xFFEE

/* USER API COMMAND TIME DELAY */
#define CMD_GET_RND_DELAY                            5
#define CMD_READ_BIN_DELAY                           10
#define CMD_ECC_EX_SIGN_DELAY                        50
#define CMD_ECC_SIGN_DELAY                           100
#define CMD_VERY_PIN_DELAY                           10
#define CMD_WRITE_BIN_DELAY                          100
#define CMD_LOCK_FILE_DELAY                          100

/* ow reset time delay para */
#define FM1230_DEFAULT_RESET_START_DELAY             300
#define FM1230_DEFAULT_RESET_SAMPLE_DELAY            10
#define FM1230_DEFAULT_RESET_END_DELAY               60
#define OW_RESET_READ_CNT                            40000

/* the following values of delay time are not precise and should be adjusted according to the waveform.  */
/* ow write time delay para */
#define FM1230_DEFAULT_WRITE_START_DELAY             2
#define FM1230_DEFAULT_WRITE_LOW_DELAY               53
#define FM1230_DEFAULT_WRITE_HIGH_DELAY              6
#define FM1230_DEFAULT_WRITE_END_DELAY               50
#define OW_WRITE_BYTE_INV                            20

/* ow read time delay para */
#define FM1230_DEFAULT_READ_START_DELAY              1
#define FM1230_DEFAULT_READ_SAMPLE_DELAY             7
#define FM1230_DEFAULT_READ_END_DELAY                100
#define OW_READ_BYTE_INV                             20

/* PAGE SIZE */
#define ECC_SIG_SIZE                                 64
#define ECC_SIG_OFS                                  (ECC_SIG_SIZE << 1)
#define ECC_SIG_RET                                  (ECC_SIG_OFS + 1)
#define DEV_CERT_SIZE                                144
#define DEV_CERT_OFS                                 (DEV_CERT_SIZE << 1)
#define READ_CERT_RET                                (DEV_CERT_OFS + 1)
#define READ_UID_SIZE                                7
#define READ_UID_OFS                                 (READ_UID_SIZE << 1)
#define READ_UID_RET                                 (READ_UID_OFS + 1)
#define UID_DATA_SIZE                                7
#define SE_UID_LENGTH                                8

#define BASE_CMD_LENGTH                              5
#define CMD_DATA_LENGTH                              8
#define PAGE_CMD_LENGTH                              7
#define PAGE_STATE_LENGTH                            4

/* INPUT DATA MAX SIZE */
#define ECC_HASH_MAX_SIZE                            65

/* CRC ALG */
#define CRC_XOR_VAL                                  0x8C
#define CRC16_XOR_VAL                                0xC001

/* COMM PROTOCOL */
#define SW_SUCCESS                                   0x9000
#define SW_BYTE_NUM                                  2
#define CMD_BYTE_LENGTH                              1
#define CMD_SEND_LENGTH                              3
#define RECV_PACK_LENGTH                             3
#define RECV_DATA_LENGTH                             2
#define RECV_DATA_OFFSET                             1
#define MAX_RETRY_COUNT                              6
#define READ                                         0
#define WRITE                                        1
#define LOCK                                         2
#define LOCK_STATUS                                  3
#define FILE11                                       0x11
#define FILE12                                       0x12
#define FILE13                                       0x13
#define FILE14                                       0x14
#define FILE15                                       0x15
#define FILE16                                       0x16
#define FILE17                                       0x17

#define BYTE_SIZE                                    8
#define PARTIY_LEN                                   16
#define CERT_HEAD                                    16
#define CMD_FID                                      2
#define CMD_SIZE                                     4
#define CMD_SIZE_WRITE                               128
#define RECV_LEN                                     8
#define LOCK_RES_LEN                                 4
#define READ_RECV_LEN                                128
#define TRND_LEN                                     8
#define COMB_TABLE_QBUF_LEN                          1920
#define TRANSCEIVE_LEN                               258
#define RECV_ROMID_LEN                               10
#define CRC_INDEX                                    8
#define CERT_RECV_LEN                                146
#define ECC_RECV_LEN                                 66
#define ECC_CMD_LEN                                  45
#define MSG_LEN                                      32
#define DEV_INFO_LEN                                 300
#define CERT_LEN                                     80
#define APDU_XFER_LEN                                300
#define HALF_SECOND                                  500
#define CRC_BYTE1                                    7
#define CRC_BYTE2                                    8

enum transceive_cmd_index {
	CMD_START_INDEX = 0,
	CMD_WRITE_LEN_INDEX,
	CMD_SEND_APDU_INDEX,
};

enum ecc_sign_mode {
	ECC_NORMAL = 0,
	ECC_EX,
	ECC_INVALID
};

struct fm1230_swi_rdwr_ops {
	void (*wbyte)(struct fm1230_dev *di, uint8_t val);
	uint8_t (*rbyte)(struct fm1230_dev *di);
	int (*reset)(struct fm1230_dev *di);
	uint8_t (*dev_reset)(struct fm1230_dev *di);
};

int fm1230_read_romid(struct fm1230_dev *di, char *buf, int times,
	int delay_ms);
int fm1230_ic_ck(struct fm1230_dev *di);
int fm1230_operate_file(struct fm1230_dev *di, int file, int operation,
	unsigned char *buf, int buf_len);
int fm1230_do_authentication(struct fm1230_dev *di, struct power_genl_attr *res,
	enum key_cr *result, enum ecc_sign_mode mode);
void fm_hex_to_str(char *pdest, const char *psrc, int ilen);
void fm_str_to_hex(char *pdest, const char *psrc, int ilen);
void fm1230_ops_register(struct fm1230_swi_rdwr_ops *se_ops_swi);
void fm1230_gpio_dir_input(int onewire_gpio);
void fm1230_gpio_dir_output(int onewire_gpio, uint8_t value);
void fm1230_gpio_set_value(int onewire_gpio, uint8_t value);
int fm1230_gpio_get_value(int onewire_gpio);
#endif /* __FM1230_API_H__ */
