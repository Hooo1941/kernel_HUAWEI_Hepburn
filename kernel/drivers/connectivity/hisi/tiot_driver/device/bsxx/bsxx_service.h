/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description: BSXX service header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-02, Create file. \n
 */
#ifndef BSXX_SERVICE_H
#define BSXX_SERVICE_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_device_bsxx_service_if  BSXX Service Interface
 * @ingroup  tiot_device_bsxx
 * @{
 */

/**
 * @brief  BSXX service init.
 * @return 0 if ok, -1 if failed.
 */
int32_t bsxx_service_init(void);

/**
 * @brief  BSXX service deinit.
 */
void bsxx_service_deinit(void);

/**
 * @brief  BSXX service get ctrl.
 * @param  dev_id Device id.
 * @return Tiot controller instace.
 */
uintptr_t bsxx_service_get_ctrl(uint8_t dev_id);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* BSXX_SERVICE_H */