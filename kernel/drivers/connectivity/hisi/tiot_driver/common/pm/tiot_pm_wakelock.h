/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-03-01, Create file. \n
 */
#ifndef TIOT_PM_WAKELOCK_H
#define TIOT_PM_WAKELOCK_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_pm_wakelock PM Wake Lock
 * @ingroup  tiot_common_pm
 * @{
 */

/**
 * @if Eng
 * @brief PM wake lock initialization interface.
 * @return if OK return 0, else return -1.
 * @else
 * @brief PM唤醒锁初始化接口。
 * @return 初始化成功返回0，否则返回-1。
 * @endif
 */
int32_t tiot_pm_wake_lock_init(void);

/**
 * @if Eng
 * @brief PM wake lock interface, support nested, pair with unlock.
 * @else
 * @brief PM唤醒锁接口，支持嵌套使用，需要和unlock配对。
 * @endif
 */
void tiot_pm_wake_lock(void);

/**
 * @if Eng
 * @brief PM wake unlock interface, support nested, pair with unlock.
 * @else
 * @brief PM唤醒解锁接口，支持嵌套使用，需要和lock配对。
 * @endif
 */
void tiot_pm_wake_unlock(void);

/**
 * @if Eng
 * @brief PM wake lock destroy interface.
 * @else
 * @brief PM唤醒锁销毁接口。
 * @endif
 */
void tiot_pm_wake_lock_destroy(void);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif