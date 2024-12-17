/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-26, Create file. \n
 */
#ifndef TIOT_XFER_UTILS_H
#define TIOT_XFER_UTILS_H

#include "tiot_xfer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_transfer_utils Transfer Utils
 * @ingroup  tiot_common_transfer
 * @{
 */

int32_t tiot_xfer_direct_send(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len);

int32_t tiot_xfer_read_store(tiot_circ_buf *cb, uint8_t *data, uint32_t len);

int32_t tiot_xfer_read_out(tiot_xfer_manager *xfer, uint8_t *buff, uint32_t len, uint32_t timeout_ms);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif