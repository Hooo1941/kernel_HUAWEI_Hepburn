/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT firmware load module utils. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-05, Create file. \n
 */
#ifndef TIOT_FIRMWARE_UTILS_H
#define TIOT_FIRMWARE_UTILS_H

#include "tiot_firmware.h"
#include "tiot_controller.h"
#include "tiot_xfer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_firmware_utils Firmware Utils
 * @ingroup  tiot_common_firmware
 * @{
 */

static inline int32_t tiot_firmware_write(tiot_fw *fw, const uint8_t *data, uint32_t len)
{
    tiot_controller *controller = tiot_container_of(fw, tiot_controller, firmware);
    return tiot_xfer_send(&controller->transfer, data, len, NULL);
}

static inline int32_t tiot_firmware_read(tiot_fw *fw, uint8_t *data, uint32_t len)
{
    tiot_xfer_rx_param rx_param = { TIOT_XFER_RECV_WAIT_FOREVER, 0 };
    tiot_controller *controller = tiot_container_of(fw, tiot_controller, firmware);
    return tiot_xfer_recv(&controller->transfer, data, len, &rx_param);
}

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif