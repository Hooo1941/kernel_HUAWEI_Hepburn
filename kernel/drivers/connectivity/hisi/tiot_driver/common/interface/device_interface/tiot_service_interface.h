/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-10-24, Create file. \n
 */
#ifndef TIOT_SERVICE_INTERFACE_H
#define TIOT_SERVICE_INTERFACE_H

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/poll.h>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_service Service Interface
 * @ingroup  tiot_common_interface
 * @{
 */

#define TIOT_READ_TIMEOUT_FOREVER  0xFFFFFFFU  /*!< Read wait timeout forever. */

typedef uintptr_t tiot_handle;

typedef struct {
    uint8_t *buff; /* 用户需要创建静态buff */
    uint32_t buff_len;
    void (*rx_callback)(const uint8_t *buff, uint32_t len);
    uint32_t flags;
} tiot_service_open_param;

/**
 * @brief  TIoT Service Init interface.
 * @return zero if OK;
 *         -1 if FAIL.
 * @note Init for all driver and board.
 */
int32_t tiot_service_init(void);

/**
 * @brief  TIoT Service Deinit interface.
 * @note Deinit for all driver and board.
 */
void tiot_service_deinit(void);

/**
 * @brief  TIoT Service Open interface.
 * @param  [in]  device_name TIoT device compatible string.
 * @param  [in]  param User/Service defined params.
 * @return valid handle if OK;
 *         zero if FAIL.
 * @note   Must write/read after open.
 */
tiot_handle tiot_service_open(const char *device_name, void *param);

/**
 * @brief  TIoT Service Close interface.
 * @note   Meaningless to return error.
 */
void tiot_service_close(tiot_handle handle);

/**
 * @brief  TIoT Service Write Message interface.
 * @param  [in]  data Data need to send.
 * @param  [in]  len Data length.
 * @return zero or postive for writen OK length;
 *         negtive if write FAIL.
 * @note   Will return after data is sent to board uart interface.
 */
int32_t tiot_service_write(tiot_handle handle, const uint8_t *data, uint32_t len);

/**
 * @brief  TIoT Service Read Message interface.
 * @param  [in/out] buff User read buff.
 * @param  [in]  buff_len User read buff size.
 * @param  [in]  timeout_ms Wait timeout ms.
 * @return if read wait timeout(nothing to read), return 0.
 *         if read fail, return -1.
 *         if support ASCII message, message type(2 bytes, ASCII or BINARY) and message will be copied to buff,
 *         else only message will be copied to buff,
 *         return read length.
 * @note   This interface will return only single complete message once,
 *         for continous message read, call this interface repeatly.
 */
int32_t tiot_service_read(tiot_handle handle, uint8_t *buff, uint32_t buff_len, uint32_t timeout_ms);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif