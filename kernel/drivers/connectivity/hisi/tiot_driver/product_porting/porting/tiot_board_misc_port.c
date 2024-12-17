/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board misc port. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-07, Create file. \n
 */
#include "tiot_board_misc_port.h"
#include <linux/compiler.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
#include <linux/pm_wakeup.h>
#endif
#include "securec.h"

#define TIOT_WAKEUP_LOCK_NAME    "tiot_wk"

typedef struct {
    struct wakeup_source ws;
    spinlock_t ws_lock;
    unsigned long lock_count;
} tiot_kernel_wakelock;

int32_t tiot_board_wake_lock_init(tiot_board_wakelock *lock)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    tiot_kernel_wakelock *kwakelock = NULL;
    if (unlikely(lock == NULL)) {
        return -1;
    }
    kwakelock = (tiot_kernel_wakelock *)kzalloc(sizeof(tiot_kernel_wakelock), GFP_KERNEL);
    if (kwakelock == NULL) {
        return -1;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
    memset_s((void *)&kwakelock->ws, sizeof(struct wakeup_source), 0, sizeof(struct wakeup_source));
    kwakelock->ws.name = TIOT_WAKEUP_LOCK_NAME;
    wakeup_source_add(&kwakelock->ws);
#else
    wakeup_source_init(&kwakelock->ws, TIOT_WAKEUP_LOCK_NAME);
#endif
    spin_lock_init(&kwakelock->ws_lock);
    lock->lock = kwakelock;
#endif
    return 0;
}

void tiot_board_wake_lock(tiot_board_wakelock *lock)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    unsigned long flags;
    tiot_kernel_wakelock *kwakelock = NULL;
    if (unlikely((lock == NULL) || (lock->lock == NULL))) {
        return;
    }

    kwakelock = lock->lock;
    spin_lock_irqsave(&kwakelock->ws_lock, flags);
    if (!kwakelock->lock_count) {
        __pm_stay_awake(&kwakelock->ws);
    }
    kwakelock->lock_count++;
    spin_unlock_irqrestore(&kwakelock->ws_lock, flags);
#endif
}

void tiot_board_wake_unlock(tiot_board_wakelock *lock)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    unsigned long flags;
    tiot_kernel_wakelock *kwakelock = NULL;
    if (unlikely((lock == NULL) || (lock->lock == NULL))) {
        return;
    }

    kwakelock = lock->lock;
    spin_lock_irqsave(&kwakelock->ws_lock, flags);
    if (kwakelock->lock_count) {
        kwakelock->lock_count--;
        if (!kwakelock->lock_count) {
            __pm_relax(&kwakelock->ws);
        }
    }
    spin_unlock_irqrestore(&kwakelock->ws_lock, flags);
#endif
}

void tiot_board_wake_lock_destroy(tiot_board_wakelock *lock)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    tiot_kernel_wakelock *kwakelock = NULL;
    if (unlikely((lock == NULL) || (lock->lock == NULL))) {
        return;
    }

    kwakelock = lock->lock;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
    wakeup_source_remove(&kwakelock->ws);
    __pm_relax(&kwakelock->ws);
#else
    wakeup_source_trash(&kwakelock->ws);
#endif
    kfree(kwakelock);
#endif
}

void tiot_board_udelay(uint32_t us)
{
    udelay(us);
}
