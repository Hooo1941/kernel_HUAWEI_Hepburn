// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * t91407_api.h
 *
 * t91407_api.c head file
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

#ifndef __T91407_API_H__
#define __T91407_API_H__

#include "t91407.h"

#define T91407_SUCCEED                              (0)
#define T91407_FAILED                               (-1)

/* ow time delay para */
#define T91407_DEFAULT_TAU_DELAY                    50

/* com para */
#define T91407_MAX_PACKAGE_RETRY_COUNT              5
#define T91407_MAX_COMMAND_RETRY_COUNT              3
#define T91407_MAX_AUTHENTICATION_RETRY_COUNT       3
#define T91407_MAX_ICCHECK_RETRY_COUNT              3

#define T91407_MODE_READ                            02
#define T91407_MODE_WRITE                           01
#define T91407_PAGE_NOT_LOCK                        00
#define T91407_PAGE_LOCKED                          01

#define T91407_SEND_DATA_LEN                        4
#define T91407_LEN_COS_VERSION                      3
#define T91407_BYTE_OF_ONE_PAGE                     8   /* Size of one page. */
#define T91407_PACKAGE_DEEP                         8
#define T91407_USER_SPACE                           128 /* User space. */
#define T91407_USER_PAGE_NUM                        21  /* User space. */
#define T91407_LEN_OF_UID                           8   /* Length of UID. */
#define T91407_LEN_OF_UID_CRC                       2   /* Length of UID CRC. */
#define T91407_LEN_OF_VENDERID                      1   /* Length of vender id. */
#define T91407_LEN_OF_HASH_32                       32  /* Length of HASH. */
#define T91407_LEN_OF_HASHUPDATE                    64
#define T91407_LEN_OF_LOCKSTATE                     4   /* Length of lock stete. */
#define T91407_LEN_OF_CMD_STATE                     1
#define T91407_LEN_OF_WRITEREADMODE                 1   /* Length of wirte/read mode. */
#define T91407_LEN_OF_BATTERY_STATE                 1
#define T91407_LEN_OF_RECEIVE_DATA                  255
#define T91407_LEN_OF_SEND_DATA                     264
#define T91407_LEN_OF_CMD_HEADER                    5
#define T91407_LEN_OF_PATCH_NUMBER                  2   /* Length of patch number. */
#define T91407_LEN_OF_SEND_CMD_HEADER               3
#define T91407_LEN_OF_SELECT_DEV_REC                1
#define T91407_MAX_INDEX_OF_BYTE                    7

#define T91407_LEN_OF_SENDING_FORMAT                9
#define T91407_LEN_OF_DATA                          8
#define T91407_LEN_OF_INVERT                        5
#define T91407_LEN_OF_REC_BUF                       3
#define T91407_REC_LEN_BYT                          2
#define T91407_LEN_OF_PACKAGE                       3
#define T91407_CMD_OFFSETS                          5
#define T91407_SEND_CODE_OFFSETS                    9
#define T91407_SEND_DATA_OFFSETS                    1
#define T91407_CRC_CALC_OFFSETS                     4
#define T91407_ONE_BYT_OFFSETS                      8

#define T91407_LEN_OF_CERTIFICATION                 136     /* Length of certification */
#define T91407_SIGN_LEN                             64
#define T91407_PUBKEY_LEN                           64
#define T91407_RANDOM_LEN                           32     /* Length of random */
#define T91407_DELAY_OF_M_SIGN                      220    /* Delay of ecc sign */

/* us */
#define T91407_DELAY_OF_WRITE_RAM                   250
#define T91407_DELAY_OF_WRITE_RANDOM                500
#define T91407_DELAY_OF_WRITE_FLASH                 5000
#define T91407_DELAY_OF_LOCK                        5000
#define T91407_DELAY_OF_EXECUTION                   1800
/* ms */
#define T91407_DELAY_OF_M_SOFTRESET                 10
#define T91407_DELAY_OF_M_WRITE_FLASHDATA           40
#define T91407_DELAY_OF_M_LONGCMD                   50

/*
 * Transaction Elements
 * BroadCast
 */
#define T91407_SWI_BC                               0x04u    /* Bus Command */

#define T91407_DATA_FRAME_TAG                       0xAAu    /* package header */
#define T91407_GET_RESPONSE_TAG                     0xC0u    /* get response header */
#define T91407_BUSCMD_FRAME_TAG                     0xE0u    /* get response header */

#define T91407_INS_READ                             0xB0u    /* read cmd INS */
#define T91407_INS_WRITE                            0xDCu    /* write cmd INS */

/* MultiCast */
#define T91407_SWI_WD                               0x08u    /* Write Data */
#define T91407_SWI_ERA                              0x09u    /* Extended Register Address */
#define T91407_SWI_WRA                              0x0Au    /* Write Register Address */
#define T91407_SWI_RRA                              0x0Bu    /* Read Register Address */

/* Bus Command */
#define T91407_SWI_SFCD                             0x00u    /* Search For Connect Device */
#define T91407_SWI_BRES                             0x01u    /* Bus Reset */
#define T91407_SWI_SPDN                             0x02u    /* Soft Power Down */
#define T91407_SWI_ECCE                             0x03u    /* ECCE Start Command */
#define T91407_SWI_WRBL0                            0x20u    /* RBLn Set Read Burst Length 1 */

/* ECC Algorithm Data Address */
#define T91407_SWI_CHALLENGE                        0x00u    /* ECCE Challenge */
#define T91407_SWI_SIGNATURE                        0x20u    /* ECCE Signture */
#define T91407_SWI_LIFESPANCNT_ENABLE               0x82u    /* Enable life span counter */
#define T91407_SWI_PERSON_CMD                       0x87u
#define T91407_SWI_SELFDEFINEDCOMMAND               0xF0u    /* Self Defined command */
#define T91407_SWI_ECCFINISHED                      0xF1u    /* ECC Finish */
#define T91407_SWI_CMDSW                            0xF2u    /* Command State Word */
#define T91407_SWI_PROCESSSW                        0xF3u    /* Procedure State Word */
#define T91407_SWI_READLEN                          0xF4u    /* Read Length */
#define T91407_SWI_CERT                             0x101u   /* ODC Signature */

#define T91407_SEND_CODE                            0x04u
#define T91407_SEND_INVERT_CODE                     0x03u
#define T91407_RES_STATE_SUCC                       0x01u
#define T91407_RES_STATE_SUCC1                      0x90u

#define T91407_SWI_UID                              0x1A3u   /* UID */
#define T91407_SWI_VENDERID                         0x1A3u   /* Vendor ID */

#define T91407_SWI_PAGELOCK0                        0x01ADu  /* LOCK Flag For Page 0~7 */
#define T91407_SWI_PAGELOCK1                        0x01AEu  /* LOCK Flag For Page 8~15 */
#define T91407_SWI_READMODE                         0x0199u  /* Read Mode */
#define T91407_SWI_WRITEMODE                        0x01A4u  /* Write Mode */
#define T91407_SWI_BATTERY_STATE                    0x019Bu  /* Battery state */
#define T91407_SWI_COS_VERSION                      0x019Fu  /* COS Version */
#define T91407_SWI_USERPAGE_START                   0x01F0u  /* Userpage Start */
#define T91407_SWI_USERPAGE_END                     0x026Fu  /* Userpage End */
#define T91407_SWI_LONGCMD_START                    0x0270u  /* Long cmd Start */

#define T91407_CALC_HASH_SUCC                       0xAAAA
#define T91407_MAX_OF_BYT1                          0x0f
#define T91407_MAX_OF_BYT2                          0xff
#define T91407_MAX_OF_ADDR                          0x00FF

/* Error code */
#define T91407_SUCCESS                              0x00000000
#define T91407_ERR_CMD_NOT_DONE                     0x00000001
#define T91407_ERR_NOT_SUPPORT                      0x00000003
#define T91407_ERR_WRONG_PARAMETER                  0x0000004E
#define T91407_ERR_OPEN_FAILED                      0x0000004F
#define T91407_ERR_DEVICE_NOT_EXIT                  0x00000050
#define T91407_ERR_CLOSE_FAILED                     0x00000051
#define T91407_ERR_NOT_INIT                         0x00000052
#define T91407_ERR_NOT_SELECT_DEVICE                0x00000053
#define T91407_ERR_VERIFY_FAILED                    0x00000054
#define T91407_ERR_PAGE_LOCKED                      0x00000055
#define T91407_ERR_CALC_HASH_FAILED                 0x00000056
#define T91407_ERR_PAGE_LOCK_FAILED                 0x00000057
#define T91407_ERR_OPERATION_NOT_SUPPORT            0x00000058

/* file page start no. */
#define T91407_FILE_ACT                             0
#define T91407_FILE_ACT_PAGE                        0
#define T91407_FILE_ACT_NUM                         10
#define T91407_FILE_BATTTYPE                        1
#define T91407_FILE_BATTTYPE_PAGE                   10
#define T91407_FILE_BATTTYPE_NUM                    1
#define T91407_FILE_LOCK                            2
#define T91407_FILE_LOCK_LEN                        8
#define T91407_FILE_LOCK_PAGE                       11
#define T91407_FILE_LOCK_NUM                        1
#define T91407_FILE_SN                              3
#define T91407_FILE_SN_PAGE                         12
#define T91407_FILE_SN_NUM                          3
#define T91407_FILE_GSN                             4
#define T91407_FILE_GSN_LEN                         40
#define T91407_FILE_GSN_PAGE                        15
#define T91407_FILE_GSN_NUM                         5

#define T91407_READ                                 0
#define T91407_WRITE                                1
#define T91407_LOCK                                 2
#define T91407_LOCK_STATUS                          3

#define T91407_READ_RECV_MAX_LEN                    150

int t91407_ic_ck(struct t91407_dev *di);
int t91407_read_romid(struct t91407_dev *di, char *buf);
int t91407_operate_file(struct t91407_dev *di, uint8_t file, int operation, unsigned char *buf, int buf_len);
int t91407_do_authentication(struct t91407_dev *di, struct power_genl_attr *key_res);
#endif /* __T91407_API_H__ */
