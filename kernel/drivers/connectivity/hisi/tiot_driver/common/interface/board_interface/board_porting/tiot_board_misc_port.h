/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board misc porting header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-24, Create file. \n
 */
#ifndef TIOT_BOARD_MISC_PORT_H
#define TIOT_BOARD_MISC_PORT_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_porting_misc Board Misc Port
 * @ingroup  tiot_common_interface_board_porting
 * @{
 */

/**
 * @if Eng
 * @brief Board wake lock struct.
 * @else
 * @brief 板级唤醒锁结构体。
 * @endif
 */
typedef struct {
    void *lock;
} tiot_board_wakelock;

/**
 * @if Eng
 * @brief Board wake lock initialization interface.
 * @param  [out] lock wake lock to be initialized.
 * @return if OK return 0, else return -1.
 * @note if board has no sleep or just enter wfi, implement as empty function.
 * @else
 * @brief 板级唤醒锁初始化接口。
 * @param  [out] lock 待初始化的唤醒锁。
 * @return 初始化成功返回0，否则返回-1。
 * @note 如果板级不睡眠或仅进入wfi，实现为空函数即可。
 * @endif
 */
int32_t tiot_board_wake_lock_init(tiot_board_wakelock *lock);

/**
 * @if Eng
 * @brief Board wake lock interface.
 * @param  [in] lock wake lock to be locked.
 * @note if board has no sleep or just enter wfi, implement as empty function.
 * @else
 * @brief 板级唤醒锁接口。
 * @param  [in] lock 待上锁的唤醒锁。
 * @note 如果板级不睡眠或仅进入wfi，实现为空函数即可。
 * @endif
 */
void tiot_board_wake_lock(tiot_board_wakelock *lock);

/**
 * @if Eng
 * @brief Board wake unlock interface.
 * @param  [in]  lock wake lock to be unlocked.
 * @note if board has no sleep or just enter wfi, implement as empty function.
 * @else
 * @brief 板级唤醒解锁接口。
 * @param  [in]  lock 待解锁的唤醒锁。
 * @note 如果板级不睡眠或仅进入wfi，实现为空函数即可。
 * @endif
 */
void tiot_board_wake_unlock(tiot_board_wakelock *lock);

/**
 * @if Eng
 * @brief Board wake lock destroy interface.
 * @param  [in]  lock wake lock to be destroyed.
 * @note if board has no sleep or just enter wfi, implement as empty function.
 * @else
 * @brief 板级唤醒锁销毁接口。
 * @param  [in]  lock 待销毁的唤醒锁.
 * @note 如果板级不睡眠或仅进入wfi，实现为空函数即可。
 * @endif
 */
void tiot_board_wake_lock_destroy(tiot_board_wakelock *lock);

/**
 * @if Eng
 * @brief  Board μs level delay.
 * @param  [in]  us μs count.
 * @else
 * @brief  板级微秒级delay接口。
 * @param  [in]  us 微秒。
 * @endif
 */
void tiot_board_udelay(uint32_t us);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif