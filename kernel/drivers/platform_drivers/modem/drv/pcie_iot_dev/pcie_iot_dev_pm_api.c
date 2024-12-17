/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * Description: PCIE IOT Power Management API Host Driver.
 * Note: This Driver is specially used on X+B PCIE Style.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/io.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>

#include "bsp_pcie.h"
#include "adrv_pcie.h"
#include "securec.h"
#include "pcie_iot_dev.h"
#include "pcie_iot_debug.h"
#include "bsp_print.h"

#undef THIS_MODU
#define THIS_MODU mod_pcie

#define PCIE_PM_API_TRACE(fmt, ...) do { \
    bsp_err("%s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
} while (0)

#define MAX_USER_NAME_LENGTH 50
#define INVALID_LOCK_VAL 0x10000000

#define PCIE_ALREADY_IN_WAKE 0x1
#define PCIE_IN_SLEEP_NO_FORCE_WAKE 0x2

struct user_pm_info {
    u32 lock_count;
    u32 unlock_count;
    struct timespec last_lock_time;
    struct timespec last_unlock_time;
};

enum pm_api_state {
    PM_API_INIT = 0,
    PM_API_RUNNING = 1,
    PM_API_RESET = 2,
    PM_API_SHUTDOWN = 3,
    PM_WORK_NUM
};

struct pcie_rc_pm_api_info {
    struct work_struct ep_vote_work;
    struct work_struct ep_unvote_work;
    struct work_struct ep_gpio_vote_work;

    void *rc_pm_bar_virt_addr;

    wait_queue_head_t pcie_vote_wait;
    wait_queue_head_t pcie_vote_wait_timeout;

    struct user_pm_info rc_pm_user_info[PCIE_USER_NUM];
    int rc_pm_vote_count[PCIE_USER_NUM];
    int rc_pm_vote_err_count[PCIE_USER_NUM];

    unsigned long receive_status_count[EP_PCIE_PM_STATUS_NUM];

    struct wakeup_source  ep_vote_wake_lock;
    struct wakeup_source  user_vote_wake_lock[PCIE_USER_NUM];

    spinlock_t spinlock; /* for controller */
    struct workqueue_struct *pm_wq;

    unsigned long ep_wake_rc_gpio_count;
    unsigned long rc_wake_ep_gpio_count[GPIO_STATUS_NUM];

    unsigned long rc_sleep_count;
    unsigned long rc_wake_count;
    unsigned long rc_pm_api_resume_count;
    unsigned long rc_pm_api_suspend_count;

    u32 rc_pcie_lock_val;
    u32 rc_lock_wake_from_user;

    u32 ep_wake_rc_gpio_disable;
    u32 ep_wake_rc_gpio_id;
    u32 rc_wake_ep_gpio_id;
    u32 rc_wake_ep_gpio_status;

    enum pm_api_state rc_pm_api_state;
    enum pcie_state rc_pm_pcie_state;
};

struct pcie_rc_pm_api_info *g_pcie_rc_pm_info = NULL;
u32 g_pcie_rc_pm_init_flag = 0;
volatile int g_pcie_enable_print = 0;
char g_user_wake_lock_name[PCIE_USER_NUM][MAX_USER_NAME_LENGTH];
volatile int g_pcie_ep_final_state = 0;

void enable_pcie_vote_print(void)
{
    g_pcie_enable_print = 1;
}

void pcie_vote_lock_print(u32 user_id)
{
    if (g_pcie_enable_print) {
        PCIE_PM_API_TRACE("user id %d, vote_lock", user_id);
    }
}

void pcie_vote_unlock_print(u32 user_id)
{
    if (g_pcie_enable_print) {
        PCIE_PM_API_TRACE("user id %d, vote_unlock", user_id);
    }
}

void pcie_call_suspend_print(u32 user_id)
{
    if (g_pcie_enable_print) {
        PCIE_PM_API_TRACE("user id %d, call suspend event", user_id);
    }
}

void pcie_call_resume_print(u32 user_id)
{
    if (g_pcie_enable_print) {
        PCIE_PM_API_TRACE("user id %d, call resume event", user_id);
    }
}

void record_user_valid_lock(enum pcie_user_id user_id)
{
    g_pcie_rc_pm_info->rc_pm_user_info[user_id].lock_count++;
    g_pcie_rc_pm_info->rc_pm_user_info[user_id].last_lock_time = current_kernel_time();
}

void record_user_valid_unlock(enum pcie_user_id user_id)
{
    g_pcie_rc_pm_info->rc_pm_user_info[user_id].unlock_count++;
    g_pcie_rc_pm_info->rc_pm_user_info[user_id].last_unlock_time = current_kernel_time();
}

int bsp_pcie_check_vote_clear(u32 wait_ms)
{
    int vote_val = g_pcie_rc_pm_info->rc_pcie_lock_val;
    int valid_val = 0x1 << PCIE_USER_EP_PM;
    int ap_val = vote_val | valid_val;
    int count_ms = 0;

    while (ap_val != valid_val) {
        if (count_ms == wait_ms) {
            break;
        }
        mdelay(1);
        vote_val = g_pcie_rc_pm_info->rc_pcie_lock_val;
        ap_val = vote_val | valid_val;
        count_ms++;
    }

    if (count_ms == wait_ms) {
        PCIE_PM_API_TRACE("check vote clear timeout, last pcie_vote_val 0x%x", vote_val);
        return -1;
    }

    PCIE_PM_API_TRACE("rc pcie user all votes clear");
    return 0;
}

int rc_wake_ep_gpio_set(void)
{
    int ret = gpio_direction_output(g_pcie_rc_pm_info->rc_wake_ep_gpio_id, 1);
    if (ret) {
        PCIE_PM_API_TRACE("fail to set GPIO<%u> to 1. \n", g_pcie_rc_pm_info->rc_wake_ep_gpio_id);
        return -1;
    }
    g_pcie_rc_pm_info->rc_wake_ep_gpio_count[GPIO_SET]++;
    pcie_pm_gpio_dbg(AP_WAKE_MODEM_GPIO, GPIO_UP);
    return ret;
}

int rc_wake_ep_gpio_clear(void)
{
    int ret = gpio_direction_output(g_pcie_rc_pm_info->rc_wake_ep_gpio_id, 0);
    if (ret) {
        PCIE_PM_API_TRACE("fail to set GPIO<%u> to 0. \n", g_pcie_rc_pm_info->rc_wake_ep_gpio_id);
        return -1;
    }
    g_pcie_rc_pm_info->rc_wake_ep_gpio_count[GPIO_CLEAR]++;
    pcie_pm_gpio_dbg(AP_WAKE_MODEM_GPIO, GPIO_DOWN);
    return ret;
}

int pcie_rc_wake_ep(void)
{
    int ret = rc_wake_ep_gpio_set();  // if slow wake comes from rc, push ep into wake task
    if (ret) {
        return -1;
    }
    g_pcie_rc_pm_info->rc_wake_ep_gpio_status = 0x1;
    return 0;
}

void pcie_rc_wake_lock_init(void)
{
    u32 i;
    int ret;
    wakeup_source_init(&g_pcie_rc_pm_info->ep_vote_wake_lock, "pcie_ep_vote_wakelock");
    for (i = 0; i < PCIE_USER_NUM; i++) {
        ret = snprintf_s((char*)g_user_wake_lock_name[i], MAX_USER_NAME_LENGTH,
            MAX_USER_NAME_LENGTH - 1, "pcie_wake_lock_user(%d)", i);
        if (ret >= 0) {
            wakeup_source_init(&g_pcie_rc_pm_info->user_vote_wake_lock[i], (char*)g_user_wake_lock_name[i]);
        }
    }
    return;
}

void pcie_rc_wake_lock_destroy(void)
{
    u32 i;
    for (i = 0; i < PCIE_USER_NUM; i++) {
        wakeup_source_trash(&g_pcie_rc_pm_info->user_vote_wake_lock[i]);
    }
    wakeup_source_trash(&g_pcie_rc_pm_info->ep_vote_wake_lock);
    return;
}

void pcie_pm_vote_record(enum pcie_user_id user_id, int force_wake, long jiffies)
{
    record_user_valid_lock(user_id);

    if (jiffies > 0) {
        pcie_pm_vote_dbg(user_id, TIMEOUT_LOCK);
    } else if (force_wake) {
        pcie_pm_vote_dbg(user_id, FORCE_LOCK);
    } else if (!force_wake) {
        pcie_pm_vote_dbg(user_id, TRY_LOCK);
    }
}

void pcie_pm_unvote_record(enum pcie_user_id user_id)
{
    record_user_valid_unlock(user_id);
    pcie_pm_vote_dbg(user_id, UNLOCK);
}

void pcie_pm_user_wake_lock(enum pcie_user_id user_id)
{
    if (user_id == PCIE_USER_CHAR_DEV || user_id == PCIE_USER_RFS ||
        user_id == PCIE_USER_ETH_DEV || user_id == PCIE_USER_ICC) {
        __pm_stay_awake(&g_pcie_rc_pm_info->user_vote_wake_lock[user_id]);
    }
}

void pcie_pm_user_wake_unlock(enum pcie_user_id user_id)
{
    if (user_id == PCIE_USER_CHAR_DEV || user_id == PCIE_USER_RFS ||
        user_id == PCIE_USER_ETH_DEV || user_id == PCIE_USER_ICC) {
        __pm_relax(&g_pcie_rc_pm_info->user_vote_wake_lock[user_id]);
    }
}

int pcie_rc_vote_wait_event(long jiffies)
{
    int ret;
    if (jiffies <= 0) {
        while (wait_event_interruptible(g_pcie_rc_pm_info->pcie_vote_wait,
            g_pcie_rc_pm_info->rc_pm_pcie_state == PCIE_WAKE)) { } // wait until condition met
    } else {
        do {
            ret = wait_event_interruptible_timeout(g_pcie_rc_pm_info->pcie_vote_wait_timeout,
                g_pcie_rc_pm_info->rc_pm_pcie_state == PCIE_WAKE, jiffies);
        } while (ret < 0);  // keep wait if interrupted by signal

        if (ret == 0) {
            return -ETIME;  // if ret = 0, means condition not meet and timeout
        } else {
            return 0; // condition met and return remaining jiffies before timeout
        }
    }
    return 0;
}

int pcie_rc_vote_lock_in_wake(enum pcie_user_id user_id, int force_wake, long jiffies)
{
    // in wake, user_id from ep, allow to vote
    // in wake, user id from native, force_wake > 0, allow to vote
    // in wake, user id from native, force_wake = 0, wake not complete, return PCIE_IN_SLEEP_NO_FORCE, no vote
    // in wake, user id from native, force_wake = 0, wake complete, count vote and return
    if (user_id == PCIE_USER_EP_PM) {
        g_pcie_rc_pm_info->rc_pcie_lock_val |= 0x1 << user_id;
        pcie_pm_vote_record(user_id, force_wake, jiffies);
    } else if (force_wake) {
        g_pcie_rc_pm_info->rc_pm_vote_count[user_id]++;
        if (g_pcie_rc_pm_info->rc_pm_vote_count[user_id] == 0x1) {
            g_pcie_rc_pm_info->rc_pcie_lock_val |= 0x1 << user_id;
            pcie_pm_vote_record(user_id, force_wake, jiffies);
            pcie_pm_user_wake_lock(user_id);
        }
    } else if (g_pcie_rc_pm_info->rc_pm_pcie_state == PCIE_SLEEP) {
        return PCIE_IN_SLEEP_NO_FORCE_WAKE;
    } else {
        g_pcie_rc_pm_info->rc_pm_vote_count[user_id]++;
        if (g_pcie_rc_pm_info->rc_pm_vote_count[user_id] == 0x1) {
            g_pcie_rc_pm_info->rc_pcie_lock_val |= 0x1 << user_id;
            pcie_pm_vote_record(user_id, force_wake, jiffies);
            pcie_pm_user_wake_lock(user_id);
        }
        // if pcie has been wake up, direct return
        return PCIE_ALREADY_IN_WAKE;
    }
    return 0;
}

int pcie_rc_vote_lock_in_sleep(enum pcie_user_id user_id, int force_wake, long jiffies, u32 *lock_wake_enable)
{
    // in sleep, force_wake = 0, return PCIE_IN_SLEEP_NO_FORCE, no vote
    // in sleep, force_wake > 0, user id from ep, allow to vote and wake
    // in sleep, force_wake > 0, user id from native, count vote and allow wake
    if (!force_wake) {
        return PCIE_IN_SLEEP_NO_FORCE_WAKE;
    }
    if (user_id == PCIE_USER_EP_PM) {
        lock_wake_enable = 1;
        g_pcie_rc_pm_info->rc_lock_wake_from_user = user_id;
        g_pcie_rc_pm_info->rc_pcie_lock_val |= 0x1 << user_id;
        pcie_pm_vote_record(user_id, force_wake, jiffies);
    } else {
        g_pcie_rc_pm_info->rc_pm_vote_count[user_id]++;
        if (g_pcie_rc_pm_info->rc_pm_vote_count[user_id] == 0x1) {
            lock_wake_enable = 1;
            g_pcie_rc_pm_info->rc_lock_wake_from_user = user_id;
            g_pcie_rc_pm_info->rc_pcie_lock_val |= 0x1 << user_id;
            pcie_pm_vote_record(user_id, force_wake, jiffies);
            pcie_pm_user_wake_lock(user_id);
        }
    }
    return 0;
}

int pcie_rc_vote_lock(enum pcie_user_id user_id, int force_wake, long jiffies)
{
    int ret = 0;
    unsigned long irq_flags = 0;
    u32 lock_wake_enable = 0;

    pcie_vote_lock_print(user_id);
    spin_lock_irqsave(&g_pcie_rc_pm_info->spinlock, irq_flags);

    if (g_pcie_rc_pm_info->rc_pcie_lock_val > 0 && g_pcie_rc_pm_info->rc_pcie_lock_val < INVALID_LOCK_VAL) {
        ret = pcie_rc_vote_lock_in_wake(user_id, force_wark, jiffies);
    } else if (g_pcie_rc_pm_info->rc_pcie_lock_val == 0) {
        ret = pcie_rc_vote_lock_in_sleep(user_id, force_wake, jiffies, &lock_wake_enable);
    }

    if (ret == PCIE_ALREADY_IN_WAKE) {
        spin_unlock_irqrestore(&g_pcie_rc_pm_info->spinlock, irq_flags);
        return 0;
    } else if (ret == PCIE_IN_SLEEP_NO_FORCE_WAKE) {
        spin_unlock_irqrestore(&g_pcie_rc_pm_info->spinlock, irq_flags);
        return -1;
    }

    if (lock_wake_enable) {
        pcie_pm_vote_edge_dbg(user_id, FIRST_LOCK);
        pcie_call_resume_print(user_id);
        ret |= pcie_rc_wake_ep();
        ret |= bsp_pcie_pm_resume();  // 先调用resume，再判断rc_pm_pcie_state的状态。
        g_pcie_rc_pm_info->rc_pm_api_resume_count++;
    }

    spin_unlock_irqrestore(&g_pcie_rc_pm_info->spinlock, irq_flags);
    if (g_pcie_rc_pm_info->rc_pm_pcie_state == PCIE_SLEEP) {
        ret = pcie_rc_vote_wait_event(jiffies);
    } else {
        return 0;
    }
    if (g_pcie_rc_pm_info->rc_pm_api_state == PM_API_RESET || g_pcie_rc_pm_info->rc_pm_api_state == PM_API_SHUTDOWN) {
        return -1;
    }
    return ret;
}

int bsp_pcie_rc_vote_lock(enum pcie_user_id user_id, int force_wake)
{
    int ret;
    if (!g_pcie_rc_pm_init_flag) {
        PCIE_PM_API_TRACE("rc pm user: %d  lock failed, lock before pcie_pm_init \n", user_id);
        return -1;
    }
    ret = pcie_rc_vote_lock(user_id, force_wake, -1);
    return ret;
}

int bsp_pcie_rc_vote_lock_timeout(enum pcie_user_id user_id, long jiffies)
{
    int ret;
    if (!g_pcie_rc_pm_init_flag) {
        PCIE_PM_API_TRACE("rc pm user: %d  lock_timeout failed, lock before pcie_pm_init", user_id);
        return -1;
    }
    ret = pcie_rc_vote_lock(user_id, 1, jiffies);
    if (ret == -ETIME) {
        bsp_pcie_rc_vote_unlock(user_id);
    }
    return ret;
}

int bsp_pcie_rc_vote_unlock(enum pcie_user_id user_id)
{
    unsigned long irq_flags = 0;
    u32 sleep_enable_flag = 0;

    if (!g_pcie_rc_pm_init_flag) {
        PCIE_PM_API_TRACE("rc pm user: %d  unlock failed, unlock before pcie_pm_init", user_id);
        return -1;
    }

    pcie_vote_unlock_print(user_id);

    spin_lock_irqsave(&g_pcie_rc_pm_info->spinlock, irq_flags);
    // if vote comes from rc side, then ignore count process
    if (user_id == PCIE_USER_EP_PM) {
        // gpio can not be fully trusted, if lock and unlock not pair, just unlock one time
        if (g_pcie_rc_pm_info->rc_pcie_lock_val == (0x1 << user_id)) {
            sleep_enable_flag = 1;
        }
        g_pcie_rc_pm_info->rc_pcie_lock_val &= ~(0x1 << user_id);
        pcie_pm_unvote_record(user_id);
    } else {
        g_pcie_rc_pm_info->rc_pm_vote_count[user_id]--;
        if (g_pcie_rc_pm_info->rc_pm_vote_count[user_id] < 0) {
            g_pcie_rc_pm_info->rc_pm_vote_count[user_id]++;
            g_pcie_rc_pm_info->rc_pm_vote_err_count[user_id]++;
            pcie_pm_vote_dbg(user_id, ERR_LOCK);
            goto err_ret;
        } else if (g_pcie_rc_pm_info->rc_pm_vote_count[user_id] == 0x0) {
            g_pcie_rc_pm_info->rc_pcie_lock_val &= ~(0x1 << user_id);
            pcie_pm_unvote_record(user_id);
            pcie_pm_user_wake_unlock(user_id);
            if (g_pcie_rc_pm_info->rc_pcie_lock_val == 0x0) {
                sleep_enable_flag = 1;
            }
        }
    }

    if (sleep_enable_flag) {
        pcie_pm_vote_edge_dbg(user_id, LAST_UNLOCK);
        pcie_call_suspend_print(user_id);
        bsp_pcie_pm_suspend();
        g_pcie_rc_pm_info->rc_pm_api_suspend_count++;
    }

    spin_unlock_irqrestore(&g_pcie_rc_pm_info->spinlock, irq_flags);

    return 0;

err_ret:
    PCIE_PM_API_TRACE("rc pm user: %d vote redundant unlock detected", user_id);
    spin_unlock_irqrestore(&g_pcie_rc_pm_info->spinlock, irq_flags);
    return -1;
}

void pcie_ep_vote_task(struct work_struct *work)
{
    int ret;
    if (g_pcie_ep_final_state == EP_PCIE_PM_START_LOCK) {
        ret = bsp_pcie_rc_vote_lock(PCIE_USER_EP_PM, 0x1);  // receive 0->1 gpio event
        if (ret) {
            PCIE_PM_API_TRACE("ep vote in rc failed, ret %d", ret);
        }
    }
    __pm_relax(&g_pcie_rc_pm_info->ep_vote_wake_lock);
    pcie_wakelock_dbg(PCIE_PM_API, WAKE_UNLOCK);
    return;
}

void pcie_ep_gpio_vote_task(struct work_struct *work)
{
    int ret = bsp_pcie_rc_vote_lock(PCIE_USER_EP_PM, 0x1);  // receive 0->1 gpio event
    if (ret) {
        PCIE_PM_API_TRACE("ep vote in rc failed, ret %d", ret);
    }
    __pm_relax(&g_pcie_rc_pm_info->ep_vote_wake_lock);
    pcie_wakelock_dbg(PCIE_PM_API, WAKE_UNLOCK);
    return;
}

void pcie_ep_unvote_task(struct work_struct *work)
{
    int ret = bsp_pcie_rc_vote_unlock(PCIE_USER_EP_PM);  // receive 1->0 gpio event
    if (ret) {
        PCIE_PM_API_TRACE("ep unvote in rc failed, ret %d", ret);
        return;
    }
    return;
}

irqreturn_t pcie_ep_wake_rc_handler(int irq, void *data)
{
    // start modem wake/sleep ap work
    __pm_stay_awake(&g_pcie_rc_pm_info->ep_vote_wake_lock); // if receive awake from ep, prevent rc sleep
    pcie_wakelock_dbg(PCIE_PM_API, WAKE_LOCK);
    pcie_pm_gpio_dbg(MODEM_WAKE_AP_GPIO, GPIO_RECEIVE);
    queue_work(g_pcie_rc_pm_info->pm_wq, &(g_pcie_rc_pm_info->ep_gpio_vote_work));
    disable_irq_nosync(gpio_to_irq(g_pcie_rc_pm_info->ep_wake_rc_gpio_id));
    g_pcie_rc_pm_info->ep_wake_rc_gpio_disable = 0x1;
    g_pcie_rc_pm_info->ep_wake_rc_gpio_count++;
    return IRQ_HANDLED;
}

int pcie_rc_pm_read_msg(u32 msg_offset)
{
    int ep_msg_value = 0;

    if (g_pcie_rc_pm_info->rc_pm_bar_virt_addr != NULL) {
        ep_msg_value = readl(g_pcie_rc_pm_info->rc_pm_bar_virt_addr + msg_offset);
    } else {
        PCIE_PM_API_TRACE("msg bar space is NULL, ret -1");
        return -1;
    }
    return ep_msg_value;
}

irqreturn_t pcie_ep_status_msi_handler(int irq_num, void *data)
{
    int ep_status_value = pcie_rc_pm_read_msg(PCIE_EP_PM_STATUS_OFFSET);
    g_pcie_ep_final_state = ep_status_value;

    switch (ep_status_value) {
        case EP_PCIE_PM_START_LOCK:
            __pm_stay_awake(&g_pcie_rc_pm_info->ep_vote_wake_lock); // if receive awake from ep, prevent rc sleep
            pcie_wakelock_dbg(PCIE_PM_API, WAKE_LOCK);
            queue_work(g_pcie_rc_pm_info->pm_wq, &(g_pcie_rc_pm_info->ep_vote_work));
            g_pcie_rc_pm_info->receive_status_count[EP_PCIE_PM_START_LOCK]++;
            break;

        case EP_PCIE_PM_LOCK_RELEASE:
            queue_work(g_pcie_rc_pm_info->pm_wq, &(g_pcie_rc_pm_info->ep_unvote_work));
            g_pcie_rc_pm_info->receive_status_count[EP_PCIE_PM_LOCK_RELEASE]++;
            break;

        default:
            PCIE_PM_API_TRACE("receive unknown ep pcie pm status msg : 0x%x \n", ep_status_value);
            break;
    }
    return IRQ_HANDLED;
}

irqreturn_t pcie_ep_wake_rc_user_handler(int irq_num, void *data)
{
    u32 ep_wake_user = pcie_rc_pm_read_msg(PCIE_EP_WAKE_RC_USER_OFFSET);
    if (ep_wake_user < PCIE_EP_MSI_NUM) {
        if (ep_wake_user != PCIE_USER_RC_PM) {
            printk(KERN_INFO "[PCIE_PM]iot pcie power up, ep wake user id: %d\n", ep_wake_user);
        }
    } else {
        printk(KERN_INFO "[PCIE_PM]invalid ep wake rc user id: %d\n", ep_wake_user);
    }
    return IRQ_HANDLED;
}

void pcie_rc_pm_gpio_release(void)
{
    disable_irq(gpio_to_irq(g_pcie_rc_pm_info->ep_wake_rc_gpio_id));
    free_irq(gpio_to_irq(g_pcie_rc_pm_info->ep_wake_rc_gpio_id), NULL);
    gpio_free(g_pcie_rc_pm_info->rc_wake_ep_gpio_id);
    gpio_free(g_pcie_rc_pm_info->ep_wake_rc_gpio_id);
}

void pcie_rc_pm_info_clear(void)
{
    int i;
    u32 user_id;

    /* func flag clear */
    g_pcie_rc_pm_info->rc_pm_bar_virt_addr = NULL;

    for (user_id = 0; user_id < PCIE_USER_NUM; user_id++) {
        memset_s(&g_pcie_rc_pm_info->rc_pm_user_info[user_id], sizeof(struct user_pm_info), 0,
                 sizeof(struct user_pm_info));
        g_pcie_rc_pm_info->rc_pm_vote_count[user_id] = 0;
        g_pcie_rc_pm_info->rc_pm_vote_err_count[user_id] = 0;
    }

    g_pcie_rc_pm_info->rc_lock_wake_from_user = 0;
    g_pcie_rc_pm_info->ep_wake_rc_gpio_disable = 0;
    g_pcie_rc_pm_info->rc_wake_ep_gpio_status = 0;

    /* debug info clear */
    for (i = 0; i < EP_PCIE_PM_STATUS_NUM; i++) {
        g_pcie_rc_pm_info->receive_status_count[i] = 0;
    }

    for (i = 0; i < GPIO_STATUS_NUM; i++) {
        g_pcie_rc_pm_info->rc_wake_ep_gpio_count[i] = 0;
    }

    g_pcie_rc_pm_info->rc_wake_count = 0;
    g_pcie_rc_pm_info->rc_sleep_count = 0;
    g_pcie_rc_pm_info->ep_wake_rc_gpio_count = 0;
    g_pcie_rc_pm_info->rc_pm_api_suspend_count = 0;
    g_pcie_rc_pm_info->rc_pm_api_resume_count = 0;

    return;
}

void pcie_rc_pm_cancel_work(void)
{
    cancel_work_sync(&g_pcie_rc_pm_info->ep_vote_work);
    cancel_work_sync(&g_pcie_rc_pm_info->ep_unvote_work);
    cancel_work_sync(&g_pcie_rc_pm_info->ep_gpio_vote_work);
}

void bsp_pcie_rc_pm_remove(enum rc_pcie_pm_rm_event rm_event)
{
    if (g_pcie_rc_pm_info == NULL) {
        PCIE_PM_API_TRACE("g_pcie_rc_pm_info not init,direct return");
        return;
    }

    PCIE_PM_API_TRACE("rc pm remove start");

    if (rm_event == RC_PCIE_SHUTDOWN) {
        PCIE_PM_API_TRACE("rc pm remove start from shutdown event");
        g_pcie_rc_pm_info->rc_pm_api_state = PM_API_SHUTDOWN;
    } else if (rm_event == RC_PCIE_RESET) {
        PCIE_PM_API_TRACE("rc pm remove start from reset event");
        g_pcie_rc_pm_info->rc_pm_api_state = PM_API_RESET;
    }

    g_pcie_rc_pm_info->rc_pcie_lock_val = INVALID_LOCK_VAL;                 // avoid further wake and sleep
    g_pcie_rc_pm_info->rc_pm_pcie_state = PCIE_WAKE;                        // avoid further wake and sleep
    wake_up_interruptible_all(&g_pcie_rc_pm_info->pcie_vote_wait);          // stop holding pcie user
    wake_up_interruptible_all(&g_pcie_rc_pm_info->pcie_vote_wait_timeout);  // stop holding pcie user

    bsp_pcie_rc_msi_free(PCIE_EP_MSI_PM_STATUS);     // stop handle ep to rc status msi
    bsp_pcie_rc_msi_free(PCIE_EP_WAKE_RC_USER_MSI);  // stop handle ep to rc wake user msi
    pcie_rc_pm_gpio_release();                       // disable all gpio
    pcie_rc_pm_cancel_work();                        // cancle all pending work
    pcie_rc_pm_info_clear();                         // clear pm info
    pcie_rc_wake_lock_destroy();                     // trash wakelock
    bsp_pcie_pm_modem_reset_cb();                    // reset pcie dev pm

    PCIE_PM_API_TRACE("rc pm remove done");
}

int get_rc_pcie_pm_dts_cfg(void)
{
    u32 ret;
    struct device_node *dev_node = NULL;
    dev_node = of_find_compatible_node(NULL, NULL, "iot,pcie_iot");
    if (dev_node == NULL) {
        PCIE_PM_API_TRACE("pcie device not found!");
        return -1;
    }
    ret = of_property_read_u32(dev_node, "ep_wake_rc_gpio_id", &g_pcie_rc_pm_info->ep_wake_rc_gpio_id);
    ret |= of_property_read_u32(dev_node, "rc_wake_ep_gpio_id", &g_pcie_rc_pm_info->rc_wake_ep_gpio_id);
    if (ret) {
        PCIE_PM_API_TRACE("fail to get iot pcie dts info, ret = %u", ret);
        return (int)ret;
    }
    return 0;
}

int pcie_rc_pm_gpio_init(void)
{
    int ret = gpio_request(g_pcie_rc_pm_info->ep_wake_rc_gpio_id, "ep_wake_rc_gpio");
    if (ret) {
        PCIE_PM_API_TRACE("gpio request failed, ret %d. ", ret);
        return -1;
    }

    ret = gpio_direction_input(g_pcie_rc_pm_info->ep_wake_rc_gpio_id);
    if (ret) {
        PCIE_PM_API_TRACE("failed to set ep wake rc gpio to input, ret = %d\n", ret);
        return ret;
    }

    ret = gpio_request(g_pcie_rc_pm_info->rc_wake_ep_gpio_id, "rc_wake_ep_gpio");
    if (ret) {
        PCIE_PM_API_TRACE("rc_wake_ep_gpio request failed, ret %d. \n", ret);
        goto err_free;
    }

    ret |= request_threaded_irq(gpio_to_irq(g_pcie_rc_pm_info->ep_wake_rc_gpio_id), pcie_ep_wake_rc_handler, NULL,
                                IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND, "pcie_ep_wake_rc_handler", NULL);
    if (ret) {
        PCIE_PM_API_TRACE("request ep wake rc interrupt failed, ret %d.", ret);
        goto err_free1;
    }

    return 0;

err_free1:
    gpio_free(g_pcie_rc_pm_info->rc_wake_ep_gpio_id);
err_free:
    gpio_free(g_pcie_rc_pm_info->ep_wake_rc_gpio_id);
    return -1;
}

int pcie_rc_pm_bar_init(void)
{
    g_pcie_rc_pm_info->rc_pm_bar_virt_addr = bsp_pcie_pm_get_bar_virt_addr();
    if (g_pcie_rc_pm_info->rc_pm_bar_virt_addr == NULL) {
        PCIE_PM_API_TRACE("fail to get pcie pm bar addr");
        return -1;
    }
    return 0;
}

void pcie_rc_pm_common_init(void)
{
    pcie_rc_pm_info_clear();
    g_pcie_rc_pm_info->rc_pm_api_state = PM_API_RUNNING;
    g_pcie_rc_pm_info->rc_pm_pcie_state = PCIE_WAKE;
    g_pcie_rc_pm_info->rc_pcie_lock_val = 0x1 << PCIE_USER_EP_PM;  // defaultly, modem is alive and has vote
    g_pcie_rc_pm_info->rc_pm_user_info[PCIE_USER_EP_PM].lock_count++;
    pcie_pm_vote_dbg(PCIE_USER_EP_PM, FORCE_LOCK);
    g_pcie_rc_pm_info->rc_pm_user_info[PCIE_USER_EP_PM].last_lock_time = current_kernel_time();
    pcie_rc_wake_lock_init();
    return;
}

void pcie_ep_pm_irq_enable(void)
{
    if (g_pcie_rc_pm_info->ep_wake_rc_gpio_disable) {
        g_pcie_rc_pm_info->ep_wake_rc_gpio_disable = 0x0;
        enable_irq(gpio_to_irq(g_pcie_rc_pm_info->ep_wake_rc_gpio_id));
    }
    return;
}

void pcie_rc_pm_set_to_sleep(void)
{
    g_pcie_rc_pm_info->rc_pm_pcie_state = PCIE_SLEEP;
    g_pcie_rc_pm_info->rc_sleep_count++;
    pcie_ep_pm_irq_enable();
}

void pcie_rc_pm_set_to_wake(void)
{
    unsigned long irq_flags = 0;

    g_pcie_rc_pm_info->rc_pm_pcie_state = PCIE_WAKE;
    g_pcie_rc_pm_info->rc_wake_count++;

    spin_lock_irqsave(&g_pcie_rc_pm_info->spinlock, irq_flags);

    if (g_pcie_rc_pm_info->rc_wake_ep_gpio_status) {
        (void)rc_wake_ep_gpio_clear();
        g_pcie_rc_pm_info->rc_wake_ep_gpio_status = 0;
    }

    spin_unlock_irqrestore(&g_pcie_rc_pm_info->spinlock, irq_flags);

    if (g_pcie_rc_pm_info->rc_lock_wake_from_user != PCIE_USER_EP_PM) {
        printk(KERN_INFO "[PCIE_PM]iot pcie power up, rc wake user id: %d\n",
               g_pcie_rc_pm_info->rc_lock_wake_from_user);
    }

    wake_up_interruptible_all(&g_pcie_rc_pm_info->pcie_vote_wait);
    wake_up_interruptible_all(&g_pcie_rc_pm_info->pcie_vote_wait_timeout);
}

int pcie_rc_pm_cb(u32 user_id, u32 user_action_id, void *callback_args)
{
    int ret = 0;

    if (user_action_id == PCIE_RC_CB_SUSPEND) {
        pcie_rc_pm_set_to_sleep();
    } else if (user_action_id == PCIE_RC_CB_RESUME) {
        pcie_rc_pm_set_to_wake();
    } else if (user_action_id == PCIE_RC_CB_POWER_DOWN) {
        printk(KERN_INFO "[PCIE_PM]iot pcie power down\n");
    }
    return ret;
}

int pcie_rc_pm_cb_register(void)
{
    u32 ret;
    struct pcie_callback_info user_callback_info = {
        .callback = pcie_rc_pm_cb,
        .callback_args = NULL,
    };

    ret = bsp_pcie_rc_cb_register(PCIE_USER_RC_PM, &user_callback_info);
    return ret;
}

int pcie_rc_pm_single_init(void)
{
    int ret;

    g_pcie_rc_pm_info = (struct pcie_rc_pm_api_info *)kzalloc(sizeof(struct pcie_rc_pm_api_info), GFP_KERNEL);
    if (g_pcie_rc_pm_info == NULL) {
        PCIE_PM_API_TRACE("fail to kzalloc, size: 0x%lX", (unsigned long)sizeof(struct pcie_rc_pm_api_info));
        return -1;
    }

    pcie_rc_pm_common_init();

    spin_lock_init(&g_pcie_rc_pm_info->spinlock);

    init_waitqueue_head(&g_pcie_rc_pm_info->pcie_vote_wait);
    init_waitqueue_head(&g_pcie_rc_pm_info->pcie_vote_wait_timeout);

    ret = get_rc_pcie_pm_dts_cfg();
    if (ret) {
        PCIE_PM_API_TRACE("fail to get pcie pm dts info, ret %d", ret);
        goto err_free;
    }

    ret = pcie_rc_pm_cb_register();
    if (ret) {
        PCIE_PM_API_TRACE("fail to register pm callback, ret %d", ret);
        goto err_free;
    }

    g_pcie_rc_pm_info->pm_wq = create_singlethread_workqueue("rc_pm_wq");

    if (g_pcie_rc_pm_info->pm_wq == NULL) {
        PCIE_PM_API_TRACE("create rc pm work queue fail!");
        goto err_free;
    }

    INIT_WORK(&g_pcie_rc_pm_info->ep_vote_work, pcie_ep_vote_task);
    INIT_WORK(&g_pcie_rc_pm_info->ep_unvote_work, pcie_ep_unvote_task);
    INIT_WORK(&g_pcie_rc_pm_info->ep_gpio_vote_work, pcie_ep_gpio_vote_task);

    return 0;

err_free:
    pcie_rc_wake_lock_destroy();
    kfree(g_pcie_rc_pm_info);
    g_pcie_rc_pm_info = NULL;
    return -1;
}

int bsp_pcie_rc_pm_api_init(void)
{
    int ret;

    PCIE_PM_API_TRACE("probe +");
    if (!g_pcie_rc_pm_init_flag) {
        ret = pcie_rc_pm_single_init();
        if (ret) {
            PCIE_PM_API_TRACE("rc pcie pm single init failed");
            return -1;
        }
    } else {
        pcie_rc_pm_common_init();
    }

    ret = pcie_rc_pm_bar_init();
    if (ret) {
        PCIE_PM_API_TRACE("fail to init rc pm bar, ret %d", ret);
        goto err_free;
    }

    ret = pcie_rc_pm_gpio_init();
    if (ret) {
        PCIE_PM_API_TRACE("fail to init rc pm gpio, ret %d", ret);
        goto err_free;
    }

    ret = bsp_pcie_rc_msi_request(PCIE_EP_MSI_PM_STATUS, pcie_ep_status_msi_handler, "pcie_ep_pm_status_msi", NULL);
    if (ret) {
        PCIE_PM_API_TRACE("fail to request ep status msi handler, ret %d", ret);
        goto err_free;
    }

    ret = bsp_pcie_rc_msi_request(PCIE_EP_WAKE_RC_USER_MSI, pcie_ep_wake_rc_user_handler,
                                  "pcie_ep_wake_rc_user_msi", NULL);
    if (ret) {
        PCIE_PM_API_TRACE("fail to request ep_wake_rc_user_msi handler, ret %d", ret);
        goto err_free;
    }

#ifdef CONFIG_IOT_PCIE_DEV_PM_TEST
    bsp_pcie_rc_pm_test_init_register();
#endif

    g_pcie_rc_pm_init_flag = 0x1;
    PCIE_PM_API_TRACE("probe -");
    return 0;

err_free:
    pcie_rc_wake_lock_destroy();
    kfree(g_pcie_rc_pm_info);
    g_pcie_rc_pm_info = NULL;
    return -1;
}

int rc_pcie_vote_lock_debug(void)
{
    int ret = bsp_pcie_rc_vote_lock(PCIE_USER_PM_TEST, 0x1);
    if (ret) {
        PCIE_PM_API_TRACE("rc debug vote lock failed, ret: %d", ret);
    }
    return ret;
}

int rc_pcie_vote_unlock_debug(void)
{
    int ret = bsp_pcie_rc_vote_unlock(PCIE_USER_PM_TEST);
    if (ret) {
        PCIE_PM_API_TRACE("rc debug vote unlock failed, ret: %d", ret);
    }
    return ret;
}

void pcie_pm_api_debug(void)
{
    u32 user_id;

    printk(KERN_ERR "*************************** PCIE RC PM VOTE INFO ************************** \n");

    printk(KERN_ERR "rc_pcie_lock_val: 0x%lx\n", (unsigned long)g_pcie_rc_pm_info->rc_pcie_lock_val);
    printk(KERN_ERR "rc_pm_pcie_state: %d. (0x0: WAKE,  0x1: SLEEP)\n", g_pcie_rc_pm_info->rc_pm_pcie_state);
    printk(KERN_ERR "rc pm last wake process comes from user_id : %d\n", g_pcie_rc_pm_info->rc_lock_wake_from_user);
    printk(KERN_ERR "rc pm sleep count: %ld,  wake count: %ld\n", g_pcie_rc_pm_info->rc_sleep_count,
           g_pcie_rc_pm_info->rc_wake_count);

    printk(KERN_ERR "rc pm api call suspend count : %ld, resume count: %ld\n",
           g_pcie_rc_pm_info->rc_pm_api_suspend_count, g_pcie_rc_pm_info->rc_pm_api_resume_count);

    for (user_id = 0; user_id < PCIE_USER_NUM; user_id++) {
        if (g_pcie_rc_pm_info->rc_pm_user_info[user_id].lock_count) {
            printk(KERN_ERR "rc pm user %d, lock_count:%d, unlock_count:%d, cur_vote_count:%d, vote_err_count:%d\n",
                   user_id, g_pcie_rc_pm_info->rc_pm_user_info[user_id].lock_count,
                   g_pcie_rc_pm_info->rc_pm_user_info[user_id].unlock_count,
                   g_pcie_rc_pm_info->rc_pm_vote_count[user_id], g_pcie_rc_pm_info->rc_pm_vote_err_count[user_id]);
            printk(KERN_ERR "last lock time: %lds+%-6ldus, last unlock time: %lds+%-6ldus\n",
                   g_pcie_rc_pm_info->rc_pm_user_info[user_id].last_lock_time.tv_sec,
                   g_pcie_rc_pm_info->rc_pm_user_info[user_id].last_lock_time.tv_nsec / NSEC_PER_USEC,
                   g_pcie_rc_pm_info->rc_pm_user_info[user_id].last_unlock_time.tv_sec,
                   g_pcie_rc_pm_info->rc_pm_user_info[user_id].last_unlock_time.tv_nsec / NSEC_PER_USEC);
        }
    }

    printk(KERN_ERR "*************************** PCIE RC PM IRQ INFO ************************** \n");

    printk(KERN_ERR "receive ep wake rc gpio, total count: %ld\n", g_pcie_rc_pm_info->ep_wake_rc_gpio_count);
    printk(KERN_ERR "receive ep status msi, vote(0~1) count: %ld, vote(1~0) count: %ld\n",
           g_pcie_rc_pm_info->receive_status_count[EP_PCIE_PM_START_LOCK],
           g_pcie_rc_pm_info->receive_status_count[EP_PCIE_PM_LOCK_RELEASE]);

    printk(KERN_ERR "send rc wake ep gpio, set_count : %ld, clear_count: %ld\n",
           g_pcie_rc_pm_info->rc_wake_ep_gpio_count[GPIO_SET], g_pcie_rc_pm_info->rc_wake_ep_gpio_count[GPIO_CLEAR]);

    printk(KERN_ERR "*************************** PCIE RC PM OTHER INFO ************************** \n");

    printk(KERN_ERR "ep wake rc gpio disable: %d.(0x0: enable, 0x1: disable)\n",
           g_pcie_rc_pm_info->ep_wake_rc_gpio_disable);
    printk(KERN_ERR "rc wake ep gpio status: %d.(0x0: gpio clear,  0x1: gpio set)\n",
           g_pcie_rc_pm_info->rc_wake_ep_gpio_status);

    printk(KERN_ERR "rc_pm_state %d. (0x1: RUNNING, 0x2: RESET, 0x3: SHUTDOWN)\n", g_pcie_rc_pm_info->rc_pm_api_state);

    printk(KERN_ERR "*************************** PCIE RC PM INFO OVER ************************** \n");
}
