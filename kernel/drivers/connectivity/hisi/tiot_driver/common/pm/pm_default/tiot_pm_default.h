/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT pm default. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-06-09, Create file. \n
 */
#ifndef TIOT_PM_DEFAULT_H
#define TIOT_PM_DEFAULT_H

#include "tiot_pm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_pm_default PM Default Handlers
 * @ingroup  tiot_common_pm
 * @{
 */

/**
 * @brief  Default power off handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_power_off(tiot_pm *pm);

/**
 * @brief  Default power on handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_power_on(tiot_pm *pm);

#ifdef CONFIG_PM_WAKEUP_BY_UART
/**
 * @brief  Default wakeup by uart handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_wakeup_device_by_uart(tiot_pm *pm);
#endif

/**
 * @brief  Default wakeup by gpio handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_wakeup_device_by_gpio(tiot_pm *pm);

#ifdef CONFIG_PM_MANAGE_LOWPOWER
/**
 * @brief  Default request sleep handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_request_sleep_handle(tiot_pm *pm);

/**
 * @brief  Default wakeup ack handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_wakeup_ack_handle(tiot_pm *pm);

/**
 * @brief  Default work vote up handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_work_vote_up(tiot_pm *pm);

/**
 * @brief  Default work vote down handler.
 * @param  [in]  pm PM control block.
 * @return see@ref tiot_pm_retcode.
 */
tiot_pm_retcode tiot_pm_default_work_vote_down(tiot_pm *pm);
#endif

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif