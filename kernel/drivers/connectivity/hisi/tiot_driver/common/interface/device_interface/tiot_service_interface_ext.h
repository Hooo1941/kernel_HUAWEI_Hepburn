/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-29, Create file. \n
 */
#ifndef TIOT_SERVICE_INTERFACE_EXT_H
#define TIOT_SERVICE_INTERFACE_EXT_H

#include "tiot_service_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_service_ext Service Interface Ext
 * @ingroup  tiot_common_interface
 * @{
 */

/**
 * @brief  TIoT Tx param, for tiot_service_write_ext.
 */
typedef struct {
    uint32_t subsys;     /*!< Write for which sub system, varies for different device. */
} tiot_service_tx_param;

/**
 * @brief  TIoT Rx param, for tiot_service_read_ext.
 */
typedef struct {
    uint32_t timeout_ms; /*!< Wait timeout ms. */
    uint32_t subsys;     /*!< Read for which sub system, varies for different device. */
#ifdef __KERNEL__
    struct file *filp;     /*!< for poll */
    poll_table *poll_wait; /*!< for poll */
#endif
} tiot_service_rx_param;

/**
 * @brief  TIoT Service Write Message extende interface.
 * @param  [in]  data Data need to send.
 * @param  [in]  len Data length.
 * @param  [in]  param Write Param, see @ref tiot_service_tx_param.
 * @return zero or postive for writen OK length;
 *         negtive if write FAIL.
 * @note   Will return after data is sent to board uart interface.
 */
int32_t tiot_service_write_ext(tiot_handle handle, const uint8_t *data,
                               uint32_t len, const tiot_service_tx_param *param);

/**
 * @brief  TIoT Service Read Message extended interface.
 * @param  [in/out] buff User read buff.
 * @param  [in]  buff_len User read buff size.
 * @param  [in]  param Read Param, see @ref tiot_service_rx_param.
 * @return if read wait timeout(nothing to read), return 0.
 *         if read fail, return -1.
 *         if support ASCII message, message type(2 bytes, ASCII or BINARY) and message will be copied to buff,
 *         else only message will be copied to buff,
 *         return read length.
 * @note   This interface will return only single complete message once,
 *         for continous message read, call this interface repeatly.
 */
int32_t tiot_service_read_ext(tiot_handle handle, uint8_t *buff, uint32_t buff_len, const tiot_service_rx_param *param);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif