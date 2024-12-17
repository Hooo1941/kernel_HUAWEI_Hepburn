/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT transfer module. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-18, Create file. \n
 */
#ifndef TIOT_XFER_H
#define TIOT_XFER_H

#include "tiot_board_xmit.h"
#include "tiot_board_xmit_ops.h"
#include "tiot_circ_queue.h"
#include "tiot_circ_buf.h"
#include "tiot_osal.h"

#ifdef __KERNEL__
#include <linux/poll.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_transfer Transfer
 * @ingroup  tiot_common
 * @{
 */

/* 临时flags, 用于拆分低功耗持锁 & disallow消息. */
#define TIOT_XFER_RESUME                0U
#define TIOT_XFER_SUSPEND               ((uint16_t)1 << 1)
#define TIOT_XFER_DELAY_MSG             ((uint16_t)1 << 2)

#define TIOT_XFER_RECV_NO_WAIT       ((uint32_t)0)
#define TIOT_XFER_RECV_WAIT_FOREVER  ((uint32_t)0xFFFFFFF)

/**
 * @brief  TIoT Transmit port state.
 */
typedef enum {
    TIOT_XMIT_STATE_CLOSE,  /* TIoT Transmit state close. */
    TIOT_XMIT_STATE_OPEN,   /* TIoT Transmit state open. */
    TIOT_XMIT_STATE_BUSY    /* TIoT Transmit state busy for reading or writing. */
} tiot_xmit_state;

/**
 * @brief  TIoT Transfer send node, cound be exetend by packet ops with other attributes.
 */
typedef struct {
    const uint8_t *data;    /* Pointer to send buff. */
    uint32_t len;     /* Send len. */
} tiot_xfer_tx_data;

/**
 * @brief  TIoT Transfer send param, cound be exetend by packet ops with other attributes.
 */
typedef struct {
    uint32_t subsys;     /* subsys info. */
} tiot_xfer_tx_param;

/**
 * @brief  TIoT Transfer receive param, cound be exetend by packet ops with other attributes.
 */
typedef struct {
    uint32_t timeout;    /* Read timeout by ms, 0 timeout means no wait. */
    uint32_t subsys;     /* subsys info. */
#ifdef __KERNEL__
    struct file *filp;
    poll_table *poll_wait;
#endif
} tiot_xfer_rx_param;

struct _tiot_xfer_manager;
struct _tiot_xfer_packet;

/**
 * @brief  TIoT Transfer packet operations.
 */
typedef struct {
    /*!< Packet parse tx node, and push to transmit or tx queue. */
    int32_t (* tx_push)(struct _tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len,
                        const tiot_xfer_tx_param *param);
    /*!< Read data out to caller buffer, same as tiot_xfer_recv. */
    int32_t (* read_out)(struct _tiot_xfer_manager *xfer, uint8_t *buff, uint32_t len, const tiot_xfer_rx_param *param);
    /*!< For async read, store data from board transmit isr. */
    void (* rx_data_store)(struct _tiot_xfer_manager *xfer, uint8_t *data, uint32_t len);
} tiot_xfer_packet_ops;

/**
 * @brief  TIoT Transfer device info.
 */
typedef struct {
    tiot_xmit_type xmit_type;
    osal_wait_condition_func rx_wait_func;  /*!< Device related read wait func, could be gpio level. */
} tiot_xfer_device_info;

typedef struct {
    uint8_t *buff;
    uint32_t buff_len;
    void (*rx_callback)(const uint8_t *buff, uint32_t len);
} tiot_xfer_cbk_param;

typedef struct {
    tiot_xfer_cbk_param rx_cbk_param;
} tiot_xfer_open_param;

/**
 * @brief  TIoT Transfer manager.
 */
typedef struct _tiot_xfer_manager {
    /* Board transmit infos. */
    tiot_xmit xmit;                   /*!< Board transmit instance. */
    const tiot_xmit_ops *xmit_ops;    /*!< Board transmit operations. */
    struct tiot_xmit_config config;   /*!< Board transmit config saved. */
    uint16_t xmit_state;              /*!< Board transmit state, close or open. */
    uint16_t xmit_suspend;            /*!< Tmp flag for suspend, will merge to state. */

    /* Mutexes */
    osal_mutex dev_mutex;             /*!< Transmit device mutex. */
    osal_mutex read_mutex;            /*!< Transmit read mutex. */

    /* Send */
    tiot_circ_queue_t tx_queue;       /*!< Tx circular queue, with tiot_xfer_tx_node as node. */
    uintptr_t tx_queue_mem;           /*!< Tx circular queue allocated memory. */

    /* Recieve */
    osal_wait rx_wait;
    osal_wait_condition_func rx_wait_func;  /*!< Registed read wait func, could be gpio level. */

    tiot_circ_buf rx_buff;            /*!< Rx circular buff. */
    uint32_t rx_buff_write;           /*!< Rx circular buff write index. */
    uint32_t rx_buff_read;            /*!< Rx circular buff read index. */
#ifdef CONFIG_XFER_DEFAULT_RX_BUFF
    /*!< Rx circular buff memory. */
    uint8_t rx_buff_mem[CONFIG_XFER_RX_BUFF_SIZE];
#endif

    /* Packet */
    tiot_xfer_packet_ops pkt_ops;    /*!< Packet operation, tx & rx data packet parse. */
    uintptr_t pkt_context;           /*!< Packet operation used context. */

    tiot_xfer_cbk_param rx_cbk_param;
} tiot_xfer_manager;

/**
 * @brief  TIoT Transfer manager init.
 * @param  [in]  xfer Transfer manager.
 * @param  [in]  xmit_id Board Transmit id.
 * @param  [in]  dev_info Device Transfer related info.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_xfer_init(tiot_xfer_manager *xfer, uintptr_t xmit_id, const tiot_xfer_device_info *dev_info);

/**
 * @brief  TIoT Transfer manager deinit.
 * @param  [in]  xfer Transfer manager.
 */
void tiot_xfer_deinit(tiot_xfer_manager *xfer);

/**
 * @brief  TIoT Transfer manager install packet.
 * @param  [in]  xfer Transfer manager.
 * @param  [in]  pkt_ops Transfer packet ops.
 * @param  [in]  pkt_context Transfer packet context.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_xfer_install_packet(tiot_xfer_manager *xfer, tiot_xfer_packet_ops *pkt_ops, uintptr_t pkt_context);

/**
 * @brief  TIoT Transfer manager uninstall packet.
 * @param  [in]  xfer Transfer manager.
 */
void tiot_xfer_uninstall_packet(tiot_xfer_manager *xfer);

/**
 * @brief  TIoT Transfer manager open, with packet installed.
 * @param  [in]  xfer Transfer manager.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_xfer_open(tiot_xfer_manager *xfer, tiot_xfer_open_param *param);

/**
 * @brief  TIoT Transfer manager close.
 * @param  [in]  xfer Transfer manager.
 */
void tiot_xfer_close(tiot_xfer_manager *xfer);

/**
 * @brief  TIoT Transfer manager set config.
 * @param  [in]  xfer Transfer manager.
 * @param  [in]  config New config to be set.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_xfer_set_config(tiot_xfer_manager *xfer, struct tiot_xmit_config *config);

/**
 * @brief  TIoT Transfer manager get config.
 * @param  [in]  xfer Transfer manager.
 * @param  [out] config copy old transmit config to caller.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_xfer_get_config(tiot_xfer_manager *xfer, struct tiot_xmit_config *config);

/**
 * @brief  TIoT Transfer manager send.
 * @param  [in]  xfer Transfer manager.
 * @param  [out] data Caller send data.
 * @param  [in]  len  Caller data len.
 * @param  [in]  param Caller send param, optional.
 * @return if Send OK return actual send len, else return -1.
 */
int32_t tiot_xfer_send(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len, const tiot_xfer_tx_param *param);

/**
 * @brief  TIoT Transfer RX read notify.
 * @param  [in]  xmit Transmit instance, will be extend to Transfer manager instance.
 * @param  [in]  data RX data need to read, could be NULL for sync read.
 * @param  [in]  len  Rx data len need to read, 0 indicates sync read.
 */
void tiot_xfer_rx_notify(tiot_xmit *xmit, uint8_t *data, uint32_t len);

/**
 * @brief  TIoT Transfer receive.
 * @param  [in]  xfer Transfer manager.
 * @param  [out] data Caller read buff.
 * @param  [in]  len  Caller expected read len.
 * @param  [in]  param Caller receive param.
 * @return if Read OK return actual read len, if timeout, return 0, else return -1.
 */
int32_t tiot_xfer_recv(tiot_xfer_manager *xfer, uint8_t *data, uint32_t len, const tiot_xfer_rx_param *param);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
