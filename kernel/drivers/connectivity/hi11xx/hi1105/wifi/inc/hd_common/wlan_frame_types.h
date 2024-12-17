/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应所有公共的信息(HAL同时可以使用的)放到该文件中
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef WLAN_FRAME_TYPES_H
#define WLAN_FRAME_TYPES_H
#include "oal_ext_if.h"

#define MAC_BYTE_ALIGN_VALUE 4 /* 4字节对齐 */

#define WLAN_MAC_ADDR_LEN          6    /* 长度宏 */
#define WLAN_MAX_FRAME_HEADER_LEN  36   /* 最大的MAC帧头长度，数据帧36，管理帧为28 */
#define WLAN_MIN_FRAME_HEADER_LEN  10   /* ack与cts的帧头长度为10 */
#define WLAN_MAX_FRAME_LEN         1600 /* 维测用，防止越界 */
#define WLAN_MGMT_FRAME_HEADER_LEN 24   /* 管理帧的MAC帧头长度，数据帧36，管理帧为28 */
#define WLAN_IWPRIV_MAX_BUFF_LEN   100  /* iwpriv上传的字符串最大长度 */

/* MAC ADDR 移位定义 */
#define MAC_ADDR_0  0
#define MAC_ADDR_1  1
#define MAC_ADDR_2  2
#define MAC_ADDR_3  3
#define MAC_ADDR_4  4
#define MAC_ADDR_5  5

/* SSID最大长度, +1为\0预留空间 */
#define WLAN_SSID_MAX_LEN (32 + 1)

/* 80211MAC帧头FC字段宏定义 */
#define WLAN_PROTOCOL_VERSION 0x00 /* 协议版本 */
#define WLAN_FC0_TYPE_MGT     0x00 /* 管理帧 */
#define WLAN_FC0_TYPE_CTL     0x04 /* 控制帧 */
#define WLAN_FC0_TYPE_DATA    0x08 /* 数据帧 */

/* 管理帧subtype */
#define WLAN_FC0_SUBTYPE_ASSOC_REQ     0x00
#define WLAN_FC0_SUBTYPE_ASSOC_RSP     0x10
#define WLAN_FC0_SUBTYPE_REASSOC_REQ   0x20
#define WLAN_FC0_SUBTYPE_REASSOC_RSP   0x30
#define WLAN_FC0_SUBTYPE_PROBE_REQ     0x40
#define WLAN_FC0_SUBTYPE_PROBE_RSP     0x50
#define WLAN_FC0_SUBTYPE_BEACON        0x80
#define WLAN_FC0_SUBTYPE_ATIM          0x90
#define WLAN_FC0_SUBTYPE_DISASSOC      0xa0
#define WLAN_FC0_SUBTYPE_AUTH          0xb0
#define WLAN_FC0_SUBTYPE_DEAUTH        0xc0
#define WLAN_FC0_SUBTYPE_ACTION        0xd0
#define WLAN_FC0_SUBTYPE_ACTION_NO_ACK 0xe0

/* 控制帧subtype */
#define WLAN_FC0_SUBTYPE_TRIG            0x20
#define WLAN_FC0_SUBTYPE_NDPA            0x50
#define WLAN_FC0_SUBTYPE_CONTROL_WRAPPER 0x70 /* For TxBF RC */
#define WLAN_FC0_SUBTYPE_BAR             0x80
#define WLAN_FC0_SUBTYPE_BA              0x90
#define WLAN_FC0_SUBTYPE_BFRP            0x40

#define WLAN_FC0_SUBTYPE_PS_POLL    0xa0
#define WLAN_FC0_SUBTYPE_RTS        0xb0
#define WLAN_FC0_SUBTYPE_CTS        0xc0
#define WLAN_FC0_SUBTYPE_ACK        0xd0
#define WLAN_FC0_SUBTYPE_CF_END     0xe0
#define WLAN_FC0_SUBTYPE_CF_END_ACK 0xf0

/* 数据帧subtype */
#define WLAN_FC0_SUBTYPE_DATA          0x00
#define WLAN_FC0_SUBTYPE_CF_ACK        0x10
#define WLAN_FC0_SUBTYPE_CF_POLL       0x20
#define WLAN_FC0_SUBTYPE_CF_ACPL       0x30
#define WLAN_FC0_SUBTYPE_NODATA        0x40
#define WLAN_FC0_SUBTYPE_CFACK         0x50
#define WLAN_FC0_SUBTYPE_CFPOLL        0x60
#define WLAN_FC0_SUBTYPE_CF_ACK_CF_ACK 0x70
#define WLAN_FC0_SUBTYPE_QOS           0x80
#define WLAN_FC0_SUBTYPE_QOS_NULL      0xc0

#define WLAN_FC1_DIR_MASK   0x03
#define WLAN_FC1_DIR_NODS   0x00 /* STA->STA */
#define WLAN_FC1_DIR_TODS   0x01 /* STA->AP  */
#define WLAN_FC1_DIR_FROMDS 0x02 /* AP ->STA */
#define WLAN_FC1_DIR_DSTODS 0x03 /* AP ->AP  */

#define WLAN_FC1_MORE_FRAG 0x04
#define WLAN_FC1_RETRY     0x08
#define WLAN_FC1_PWR_MGT   0x10
#define WLAN_FC1_MORE_DATA 0x20
#define WLAN_FC1_WEP       0x40
#define WLAN_FC1_ORDER     0x80

#define WLAN_HDR_DUR_OFFSET   2  /* duartion相对于mac头的字节偏移 */
#define WLAN_HDR_ADDR1_OFFSET 4  /* addr1相对于mac头的字节偏移 */
#define WLAN_HDR_ADDR2_OFFSET 10 /* addr1相对于mac头的字节偏移 */
#define WLAN_HDR_ADDR3_OFFSET 16 /* addr1相对于mac头的字节偏移 */
#define WLAN_HDR_FRAG_OFFSET  22 /* 分片序号相对于mac的字节偏移 */

#define WLAN_REASON_CODE_LEN 2

/* 帧头DS位 */
#define WLAN_FRAME_TO_AP   0x0100
#define WLAN_FRAME_FROM_AP 0x0200
/* FCS长度(4字节) */
#define WLAN_HDR_FCS_LENGTH 4

#define WLAN_RANDOM_MAC_OUI_LEN 3 /* 随机mac地址OUI长度 */
#define WLAN_MAX_BAR_DATA_LEN 20  /* BAR帧的最大长度 */
#define WLAN_CHTXT_SIZE       128 /* challenge text的长度 */
#define WLAN_SEQ_SHIFT 4

#define WLAN_TB_PPDU_SCHEDULE_TIMEOUT 15
#define WLAN_MAC_FRAME_LEN  1560

/* 控制帧、管理帧特殊帧长 */
#define WLAN_NDPA_HDR_LEN 16  /* NDPA mac hdr len */
#define WLAN_MAX_BAR_HDR_LEN  16   /* BAR帧HDR的长度 */

/* 管理帧子类型 */
typedef enum {
    WLAN_ASSOC_REQ = 0,           /* 0000 */
    WLAN_ASSOC_RSP = 1,           /* 0001 */
    WLAN_REASSOC_REQ = 2,         /* 0010 */
    WLAN_REASSOC_RSP = 3,         /* 0011 */
    WLAN_PROBE_REQ = 4,           /* 0100 */
    WLAN_PROBE_RSP = 5,           /* 0101 */
    WLAN_TIMING_AD = 6,           /* 0110 */
    WLAN_MGMT_SUBTYPE_RESV1 = 7,  /* 0111 */
    WLAN_BEACON = 8,              /* 1000 */
    WLAN_ATIM = 9,                /* 1001 */
    WLAN_DISASOC = 10,            /* 1010 */
    WLAN_AUTH = 11,               /* 1011 */
    WLAN_DEAUTH = 12,             /* 1100 */
    WLAN_ACTION = 13,             /* 1101 */
    WLAN_ACTION_NO_ACK = 14,      /* 1110 */
    WLAN_MGMT_SUBTYPE_RESV2 = 15, /* 1111 */

    WLAN_MGMT_SUBTYPE_BUTT = 16, /* 一共16种管理帧子类型 */
} wlan_frame_mgmt_subtype_enum;

/* 认证的transaction number */
typedef enum {
    WLAN_AUTH_TRASACTION_NUM_ONE = 0x0001,
    WLAN_AUTH_SEQ_SAE_COMMIT = WLAN_AUTH_TRASACTION_NUM_ONE,
    WLAN_AUTH_TRASACTION_NUM_TWO = 0x0002,
    WLAN_AUTH_SEQ_SAE_CONFIRM = WLAN_AUTH_TRASACTION_NUM_TWO,
    WLAN_AUTH_TRASACTION_NUM_THREE = 0x0003,
    WLAN_AUTH_TRASACTION_NUM_FOUR = 0x0004,

    WLAN_AUTH_TRASACTION_NUM_BUTT
} wlan_auth_transaction_number_enum;
typedef uint16_t wlan_auth_transaction_number_enum_uint16;

#if defined(IS_HOST) && IS_HOST
typedef enum {
    WLAN_WITP_AUTH_OPEN_SYSTEM = 0,
    WLAN_WITP_AUTH_SHARED_KEY,
    WLAN_WITP_AUTH_FT,
    WLAN_WITP_AUTH_NETWORK_EAP,
    WLAN_WITP_AUTH_SAE,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
    /* 4.10以上版本新增3个FILS相关认证类型 */
    WLAN_WITP_AUTH_FILS_SK,
    WLAN_WITP_AUTH_FILS_SK_PFS,
    WLAN_WITP_AUTH_FILS_PK,
#endif
    WLAN_WITP_AUTH_TBPEKE,
    WLAN_WITP_AUTH_NUM,
    WLAN_WITP_AUTH_MAX = WLAN_WITP_AUTH_NUM - 1,
    WLAN_WITP_AUTH_AUTOMATIC,
    WLAN_WITP_AUTH_BUTT
} wlan_auth_alg_mode_enum;
#else
typedef enum {
    WLAN_WITP_AUTH_OPEN_SYSTEM = 0,
    WLAN_WITP_AUTH_SHARED_KEY,
    WLAN_WITP_AUTH_FT,
    WLAN_WITP_AUTH_NETWORK_EAP,
    WLAN_WITP_AUTH_SAE,
} wlan_auth_alg_mode_enum;
#endif

typedef uint8_t wlan_auth_alg_mode_enum_uint8;

/* 帧类型 (2-bit) */
typedef enum {
    WLAN_MANAGEMENT = 0,
    WLAN_CONTROL = 1,
    WLAN_DATA_BASICTYPE = 2,
    WLAN_RESERVED = 3,

    WLAN_FRAME_TYPE_BUTT
} wlan_frame_type_enum;
typedef uint8_t wlan_frame_type_enum_uint8;

/* 帧子类型 (4-bit) */
/* 管理帧子类型放入平台SPEC */
/* 控制帧帧子类型 */
typedef enum {
    /* 0~6 reserved */
    WLAN_HE_TRIG_FRAME = 2, /* 0010 */
    WLAN_VHT_NDPA = 5,      /* 0101 */
    WLAN_CONTROL_WRAPPER = 7,
    WLAN_BLOCKACK_REQ = 8,
    WLAN_BLOCKACK = 9,
    WLAN_PS_POLL = 10,
    WLAN_RTS = 11,
    WLAN_CTS = 12,
    WLAN_ACK = 13,
    WLAN_CF_END = 14,
    WLAN_CF_END_CF_ACK = 15,

    WLAN_CONTROL_SUBTYPE_BUTT = 16,
} wlan_frame_control_subtype_enum;

/* 数据帧子类型 */
typedef enum {
    WLAN_DATA = 0,
    WLAN_DATA_CF_ACK = 1,
    WLAN_DATA_CF_POLL = 2,
    WLAN_DATA_CF_ACK_POLL = 3,
    WLAN_NULL_FRAME = 4,
    WLAN_CF_ACK = 5,
    WLAN_CF_POLL = 6,
    WLAN_CF_ACK_POLL = 7,
    WLAN_QOS_DATA = 8,
    WLAN_QOS_DATA_CF_ACK = 9,
    WLAN_QOS_DATA_CF_POLL = 10,
    WLAN_QOS_DATA_CF_ACK_POLL = 11,
    WLAN_QOS_NULL_FRAME = 12,
    WLAN_DATA_SUBTYPE_RESV1 = 13,
    WLAN_QOS_CF_POLL = 14,
    WLAN_QOS_CF_ACK_POLL = 15,

    WLAN_DATA_SUBTYPE_MGMT = 16,
} wlan_frame_data_subtype_enum;

/* ACK类型定义 */
typedef enum {
    WLAN_TX_NORMAL_ACK = 0,
    WLAN_TX_NO_ACK,
    WLAN_TX_NO_EXPLICIT_ACK,
    WLAN_TX_BLOCK_ACK,

    WLAN_TX_NUM_ACK_BUTT
} wlan_tx_ack_policy_enum;
typedef uint8_t wlan_tx_ack_policy_enum_uint8;
#endif /* end of wlan_protocol_types.h */
