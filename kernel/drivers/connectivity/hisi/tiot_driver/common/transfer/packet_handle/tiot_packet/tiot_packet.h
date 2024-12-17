/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-07-21, Create file. \n
 */
#ifndef TIOT_PACKET_H
#define TIOT_PACKET_H

#include "tiot_types.h"
#include "tiot_xfer.h"
#include "tiot_list.h"
#include "tiot_packet_manager.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_transfer_packet_handle_tiot_packet Packet Handle
 * @ingroup  tiot_common_transfer_packet_handle
 * @{
 */

#define TIOT_PKT_FLAGS_SUPPORT_ASCII       0x1U

#define TIOT_PKT_RX_QUEUE_SIZE    8     /* 暂时只缓存8包数据. */

#define TIOT_PKT_PAYLOAD_MAX_SIZE 4095U
#define TIOT_PKT_HEAD_SIZE 4
#define TIOT_PKT_TAIL_SIZE 1
#define TIOT_PKT_PKG_SIZE  (TIOT_PKT_HEAD_SIZE + TIOT_PKT_TAIL_SIZE)

#define TIOT_PKT_HEAD  0x7e
#define TIOT_PKT_TAIL  0x7e

/* 重新定义, 暂时只需要这些 */
typedef enum {
    SUBSYS_GNSS = 0x00,
    SUBSYS_OM  = 0x01,
    SUBSYS_SYS  = 0x02,
    SUBSYS_BUTT
} subsys_type_enum;

typedef enum {
    TIOT_PKT_TAG_BINARY,
    TIOT_PKT_TAG_ASCII
} tiot_pkt_rx_tag;

typedef enum {
    PKG_NOT_SEPRETED = 0,
    PKG_SEPRETED = 1,
} pkg_sepreted_state;

enum TIOT_PKT_DATA_MSG_TYPE_ENUM {
    SYS_MSG = 0x00,         /* 系统串口消息 */
    BT_MSG = 0x01,          /* BT串口消息 */
    GNSS_FIRST_MSG = 0x02,  /* GNSS串口消息，第一个分段消息 */
    GNSS_COMMON_MSG = 0x03, /* GNSS串口消息，中间分段消息 */
    GNSS_LAST_MSG = 0x04,   /* GNSS串口消息，最后一个分段消息 */
    FM_FIRST_MSG = 0x05,    /* FM串口消息，第一个分段消息 */
    FM_COMMON_MSG = 0x06,   /* FM串口消息，中间分段消息 */
    FM_LAST_MSG = 0x07,     /* FM串口消息，最后一个分段消息 */
    IR_FIRST_MSG = 0x08,    /* 红外串口消息，第一个分段消息 */
    IR_COMMON_MSG = 0x09,   /* 红外串口消息，中间分段消息 */
    IR_LAST_MSG = 0x0A,     /* 红外串口消息，最后一个分段消息 */
    NFC_FIRST_MSG = 0x0B,   /* NFC串口消息，第一个分段消息 */
    NFC_COMMON_MSG = 0x0C,  /* NFC串口消息，中间分段消息 */
    NFC_LAST_MSG = 0x0D,    /* NFC串口消息，最后一个分段消息 */
    OML_MSG = 0x0E,         /* 可维可测消息 */
    MEM_DUMP_SIZE = 0x0f,   /* bfgx异常时，要dump的mem长度消息 */
    MEM_DUMP = 0x10,        /* bfgx异常时，内存dump消息 */
    WIFI_MEM_DUMP = 0x11,   /* UART READ WIFI MEM，内存dump消息 */
    BFGX_CALI_MSG = 0x12,   /* BFGX 校准数据上报 */

    /* GF_CPU消息 */
    GF_SYS_MSG = 0x13,      /* GF系统串口消息 */
    GF_OML_MSG = 0x14,      /* GF可维可测消息 */

    SLE_MSG = 0x15,         /* SLE串口消息 */

    MSG_BUTT,
};

typedef struct {
    uint8_t seperated;
    uint8_t first;
    uint8_t common;
    uint8_t last;
} tiot_packet_subsys_info;

typedef struct {
    uint8_t head;
    uint8_t subsys_code;
    uint16_t len;
} tiot_packet_head;

typedef struct {
    uint8_t *start;
    uint32_t size;
    uint32_t recv_cnt;
} tiot_packet_buffer;

#ifndef CONFIG_BOARD_DYNAMIC_ALLOC
typedef struct {
    tiot_circ_buf buf;
    uint32_t buff_write;
    uint32_t buff_read;
} tiot_packet_circ_buffer;
#endif

typedef struct {
    uint32_t flags;              /*!< Control behave. */
    tiot_packet_manager rx_manager_info;
    tiot_packet_buffer *buff;
} tiot_packet_context_param;

/**
 * @brief  Packet parser context.
 */
typedef struct {
    tiot_packet_manager rx_manager;
    tiot_packet *packet;
    uint32_t flags;              /*!< Control behave. */
    osal_mutex tx_packet_mutex;
#ifndef CONFIG_BOARD_DYNAMIC_ALLOC
    tiot_packet_circ_buffer rx_buff;
#endif
    /* Rx State machine. */
    uint8_t *pos;
    tiot_packet_head head;
    uint8_t tail;
    uint8_t sys_msg;
    uint8_t frame_error;
    uint16_t frame_cnt;
    uint16_t current_frame;
    uint16_t frame_need_read;
    uint16_t rx_queue_id;

    tiot_xfer_manager *xfer;  /*!< Connect transfer manager. */
} tiot_packet_context;

typedef struct {
    tiot_xfer_tx_data data;
    uint16_t subsys;
} tiot_packet_tx_data;

/**
 * @brief  Packet context init.
 * @param  [in]  ctx   packet context.
 * @param  [in]  xfer  Transfer manager.
 * @param  [in]  flags control flags.
 * @return if OK, return 0, else return -1.
 */
int32_t tiot_packet_init(tiot_packet_context *ctx, tiot_xfer_manager *xfer, const tiot_packet_context_param *param);

/**
 * @brief  Packet context deinit.
 * @param  [in]  ctx packet context.
 */
void tiot_packet_deinit(tiot_packet_context *ctx);

/**
 * @brief  tx packet push.
 * @param  [in]  xfer Transfer manager.
 * @param  [out] data Caller send data.
 * @param  [in]  len  Caller data len.
 * @param  [in]  param Caller send param, must.
 * @return if OK, return sent data, else return -1.
 */
int32_t tiot_packet_tx_push(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len,
                            const tiot_xfer_tx_param *param);

/**
 * @brief  rx data store.
 * @param  [in]  xfer Transfer manager.
 * @param  [in]  data Received data.
 * @param  [in]  len  Received data length.
 */
void tiot_packet_rx_data_store(tiot_xfer_manager *xfer, uint8_t *data, uint32_t len);

/**
 * @brief  rx data read out, copy uint16_t data_tag and single complete packet into user buffer.
 * @param  [in]  xfer Transfer manager.
 * @param  [out] buff User buffer.
 * @param  [in]  len  Max read length.
 * @param  [in]  param Caller receive param.
 * @return int32_t if OK, return readout length;if wait timeout, return 0; if err, return -1.
 * @note   First two bytes copyed to buff indicate data type, ASCII or BINARY.
 */
int32_t tiot_packet_read_out(tiot_xfer_manager *xfer, uint8_t *buff, uint32_t len, const tiot_xfer_rx_param *param);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
