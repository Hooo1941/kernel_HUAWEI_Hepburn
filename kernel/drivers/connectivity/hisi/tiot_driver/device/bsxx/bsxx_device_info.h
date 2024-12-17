/**
 * Copyright (c) @CompanyNameMagicTag 2023-2024. All rights reserved. \n
 *
 * Description: BSXX device info header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-02, Create file. \n
 */
#ifndef BSXX_DEVICE_INFO_H
#define BSXX_DEVICE_INFO_H

#include "tiot_device_info.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_device_bsxx_device_info  BSXX Device Info
 * @ingroup  tiot_device_bsxx
 * @{
 */

/**
 * @brief  BSXX device get info, same for all BSXX devices.
 * @return BSXX device info.
 */
const tiot_device_info *bsxx_device_get_info(void);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* BSXX_DEVICE_INFO_H */