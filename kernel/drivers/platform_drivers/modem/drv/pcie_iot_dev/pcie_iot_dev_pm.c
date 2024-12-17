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
 * Description: PCIE IOT Host Power Management Strategy Driver
 * Note: This Driver is specially used for X+B Host PCIE PM.
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
#include "bsp_slice.h"
#include "adrv_pcie.h"
#include "pcie_iot_dev.h"
#include "pcie_iot_debug.h"
#include <mdrv_errno.h>
#include "bsp_dump.h"
#include "bsp_print.h"

#define EP_INIT_MAX_WAIT_MS 20000
#define EP_INIT_WAIT_DIV         2
#define PCIE_NO_VOTE_TIMEOUT_MS 200

#undef THIS_MODU
#define THIS_MODU mod_pcie

#define PCIE_PM_TRACE(fmt, ...) do { \
    bsp_err("%s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
} while (0)


struct pcie_pm_args_info {
    enum pcie_pm_mode pm_mode;
    u32 timeout_ms;
};

struct pcie_pm_info {
    spinlock_t spinlock;
    struct task_struct *pm_task;
    struct semaphore pm_task_sem;

    struct wakeup_source  pm_wake_lock;
    struct delayed_work idle_timeout_work;
    struct delayed_work ep_init_timeout_work;
    struct pcie_pm_args_info pm_args;

    unsigned long pm_bar_phys_addr;
    void *pm_bar_virt_addr;

    unsigned long send_msg_count[RC_PCIE_PM_MSG_NUM];       // for debug
    unsigned long receive_msg_count[EP_PCIE_PM_MSG_NUM];    // for debug
    unsigned long send_status_gpio_count[GPIO_STATUS_NUM];  // for debug
    unsigned long receive_status_gpio_count;                // for debug
    unsigned long powerup_jiffies;                 // for debug
    unsigned long powerup_timeout_jiffies;  // for debug
    unsigned long powerup_slice;                   // for debug
    unsigned long powerup_timeout_slice;    // for debug

    u32 ep_status_gpio_id;
    u32 rc_status_gpio_id;

    u32 current_state;
    u32 inner_event;
    u32 outer_event;
    u32 debug_print_enable;
    u32 pcie_port_id;
    volatile u32 pcie_thread_done;
};

struct pcie_pm_info g_pcie_pm_info;
u32 g_pcie_pm_init_flag = 0;

void pcie_pm_debug_print_enable(void)
{
    g_pcie_pm_info.debug_print_enable = 0x1;
}

void pcie_pm_debug_print_disable(void)
{
    g_pcie_pm_info.debug_print_enable = 0x0;
}

void pcie_pm_wakeup_task(struct pcie_pm_info *pm_info)
{
    up(&pm_info->pm_task_sem);
}

int pcie_pm_send_msg(struct pcie_pm_info *pm_info, enum rc_pcie_pm_msg msg_value)
{
    int ret;
    int read_back;

    if (msg_value >= RC_PCIE_PM_MSG_NUM) {
        PCIE_PM_TRACE("Invalid send msg value");
        return -1;
    }

    writel(msg_value, pm_info->pm_bar_virt_addr + PCIE_RC_PM_MSG_OFFSET);
    read_back = readl(pm_info->pm_bar_virt_addr + PCIE_RC_PM_MSG_OFFSET);

    ret = bsp_pcie_rc_send_msi(PCIE_RC_MSI_PM);
    if (ret) {
        PCIE_PM_TRACE("fail to send msg to ep");
        return -1;
    }

    pm_info->send_msg_count[msg_value]++;
    pcie_pm_msg_dbg(msg_value, MSG_SEND);

    return 0;
}

int pcie_pm_read_msg(struct pcie_pm_info *pm_info, u32 msg_offset)
{
    int msg_value = 0;

    if (pm_info->pm_bar_virt_addr != NULL) {
        msg_value = readl(pm_info->pm_bar_virt_addr + msg_offset);
    } else {
        PCIE_PM_TRACE("msg bar space is NULL, ret -1");
        return -1;
    }

    return msg_value;
}

irqreturn_t pcie_pm_msi_handler(int irq_num, void *data)
{
    int ep_msg_value;
    struct pcie_pm_info *pm_info = &g_pcie_pm_info;

    ep_msg_value = pcie_pm_read_msg(pm_info, PCIE_EP_PM_MSG_OFFSET);
    if (ep_msg_value < EP_PCIE_PM_MSG_NUM) {
        pcie_pm_msg_dbg(ep_msg_value, MSG_READ);
    } else {
        PCIE_PM_TRACE("invalid msg: %d", ep_msg_value);
        return IRQ_NONE;
    }

    switch (ep_msg_value) {
        case EP_PCIE_PM_LOCK_FREE:
            pm_info->inner_event = PCIE_PM_STATE_POWER_DOWN;
            pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_POWER_DOWN);
            pm_info->receive_msg_count[EP_PCIE_PM_LOCK_FREE]++;
            break;
        case EP_PCIE_PM_LOCK_BUSY:
            pm_info->inner_event = PCIE_PM_STATE_LINK_IDLE;
            pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_LINK_IDLE);
            pm_info->receive_msg_count[EP_PCIE_PM_LOCK_BUSY]++;
            break;
        default:
            pm_info->inner_event = PCIE_PM_STATE_LINK_IDLE;
            pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_LINK_IDLE);
            PCIE_PM_TRACE("receive unknown ep pcie pm msi msg: 0x%x\n", ep_msg_value);
            break;
    }

    pcie_pm_event_dbg(GET_PCIE_MSI);
    pcie_pm_wakeup_task(pm_info);
    return IRQ_HANDLED;
}

irqreturn_t pcie_pm_ep_status_gpio_handler(int irq, void *data)
{
    struct pcie_pm_info *pm_info = &g_pcie_pm_info;

    pm_info->inner_event = PCIE_PM_STATE_LINK_UP;

    disable_irq_nosync(gpio_to_irq(pm_info->ep_status_gpio_id));

    pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_LINK_UP);
    pcie_pm_event_dbg(GET_GPIO);
    pcie_pm_gpio_dbg(MODEM_STATUS_GPIO, GPIO_RECEIVE);
    pcie_pm_wakeup_task(pm_info);

    pm_info->receive_status_gpio_count++;

    return IRQ_HANDLED;
}

void pcie_pm_ep_irq_enable(struct pcie_pm_info *pm_info)
{
    enable_irq(gpio_to_irq(pm_info->ep_status_gpio_id));
}

int pcie_pm_set_rc_ready_gpio(struct pcie_pm_info *pm_info, int value)
{
    int ret = gpio_direction_output(pm_info->rc_status_gpio_id, value);
    if (ret) {
        PCIE_PM_TRACE("fail to set GPIO<%u> to %d, ret = %d", pm_info->rc_status_gpio_id, value, ret);
        return -1;
    }

    if (value) {
        pcie_pm_gpio_dbg(AP_STATUS_GPIO, GPIO_UP);
    } else {
        pcie_pm_gpio_dbg(AP_STATUS_GPIO, GPIO_DOWN);
    }

    if (value) {
        pm_info->send_status_gpio_count[GPIO_SET]++;
    } else {
        pm_info->send_status_gpio_count[GPIO_CLEAR]++;
    }

    return ret;
}

void pcie_pm_do_idle(struct pcie_pm_info *pm_info)
{
    u32 timeout_jiffies = msecs_to_jiffies(pm_info->pm_args.timeout_ms);

    (void)schedule_delayed_work(&pm_info->idle_timeout_work, timeout_jiffies);

    pm_info->current_state = PCIE_PM_STATE_LINK_IDLE;

    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_LINK_IDLE);
}

void pcie_pm_do_idle_to_link_up(struct pcie_pm_info *pm_info)
{
    (void)cancel_delayed_work_sync(&pm_info->idle_timeout_work);
    pm_info->inner_event = PCIE_PM_STATE_UNINITED;
    pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_UNINITED);
    pm_info->current_state = PCIE_PM_STATE_LINK_UP;
    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_LINK_UP);
}

int pcie_pm_do_suspend_callback(struct pcie_pm_info *pm_info)
{
    int ret = bsp_pcie_rc_cb_run(PCIE_RC_CB_SUSPEND);
    if (ret) {
        PCIE_PM_TRACE("fail to call suspend callback, callback return %d", ret);
    }

    if (pm_info->outer_event == PCIE_PM_STATE_POWER_UP) {
        (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_RESUME);
        return PCIE_PM_STATE_POWER_UP;
    }

    return 0;
}

void pcie_pm_do_presuspend(struct pcie_pm_info *pm_info)
{
    int ret = pcie_pm_do_suspend_callback(pm_info);
    if (ret == PCIE_PM_STATE_POWER_UP) {
        pm_info->inner_event = PCIE_PM_STATE_UNINITED;
        pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_UNINITED);
        return;
    }

    pcie_pm_send_msg(pm_info, RC_PCIE_VOTE_TO_SLEEP);

    pm_info->current_state = PCIE_PM_STATE_PRESUSPEND;
    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_PRESUSPEND);
}

void pcie_pm_do_presuspend_to_idle(struct pcie_pm_info *pm_info)
{
    (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_RESUME);

    pcie_pm_do_idle(pm_info);
}

void pcie_pm_do_power_down(struct pcie_pm_info *pm_info)
{
    int ret;

    bsp_pcie_rc_linkdown_event_disable();

    (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_POWER_DOWN);

    bsp_pcie_rc_irq_disable();  // disable ep to rc intx irq, wait for onging irq done

    bsp_pcie_save_all_ep_cfg();

    // start to switch PCIE Link from L0->L3
    ret = pcie_kport_pm_control(0x0, pm_info->pcie_port_id);
    if (ret) {
        PCIE_PM_TRACE("fail to power off AP pcie, ret %d", ret);
        goto err_ret;
    }

    pcie_pm_ep_irq_enable(pm_info);

    pm_info->current_state = PCIE_PM_STATE_POWER_DOWN;
    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_POWER_DOWN);

    __pm_relax(&pm_info->pm_wake_lock);
    pcie_wakelock_dbg(PCIE_PM_DRV, WAKE_UNLOCK);

    return;

err_ret:
    system_error(DRV_ERRNO_PCIE_ABNORMAL, RC_POWER_DOWN_FAIL, 0, NULL, 0);
    return;
}

void pcie_pm_do_power_up(struct pcie_pm_info *pm_info)
{
    int ret;

    __pm_stay_awake(&pm_info->pm_wake_lock);
    pcie_wakelock_dbg(PCIE_PM_DRV, WAKE_LOCK);

    // step1: provide pcie clock
    ret = pcie_kport_pm_control(0x3, pm_info->pcie_port_id);
    if (ret) {
        PCIE_PM_TRACE("fail to provide pcie clock to ep, ret = %d", ret);
        goto err_ret;
    }

    ret = pcie_pm_set_rc_ready_gpio(pm_info, 0x1);
    if (ret) {
        goto err_ret;
    }

    g_pcie_pm_info.powerup_slice = bsp_get_slice_value();
    g_pcie_pm_info.powerup_jiffies = jiffies;

    (void)schedule_delayed_work(&pm_info->ep_init_timeout_work, msecs_to_jiffies(EP_INIT_MAX_WAIT_MS));

    pm_info->current_state = PCIE_PM_STATE_POWER_UP;
    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_POWER_UP);

    return;

err_ret:
    system_error(DRV_ERRNO_PCIE_ABNORMAL, RC_POWER_UP_FAIL, 0, NULL, 0);
    return;
}

void pcie_pm_do_link_up(struct pcie_pm_info *pm_info)
{
    int ret;

    (void)cancel_delayed_work_sync(&pm_info->ep_init_timeout_work);

    ret = pcie_pm_set_rc_ready_gpio(pm_info, 0);
    if (ret) {
        goto err_ret;
    }

    ret = pcie_kport_pm_control(0x1, pm_info->pcie_port_id);
    if (ret) {
        PCIE_PM_TRACE("fail to establish link with ep device, ret %d", ret);
        goto err_ret;
    }

    // step4: restore ep config
    ret = bsp_pcie_restore_all_ep_cfg();
    if (ret) {
        PCIE_PM_TRACE("fail to recover ep device cfg");
        goto err_ret;
    }

    bsp_pcie_rc_irq_enable();

    (void)pcie_pm_send_msg(pm_info, RC_PCIE_SLOW_AWAKE_DONE);

    bsp_pcie_rc_linkdown_event_enable();

    (void)bsp_pcie_rc_cb_run(PCIE_RC_CB_RESUME);

    pm_info->current_state = PCIE_PM_STATE_LINK_UP;
    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_LINK_UP);

    return;

err_ret:
    system_error(DRV_ERRNO_PCIE_ABNORMAL, RC_LINK_UP_FAIL, 0, NULL, 0);
    return;
}

static pcie_pm_state_linkup_process(struct pcie_pm_info *pm_info)
{
    if (pm_info->outer_event == PCIE_PM_STATE_LINK_IDLE) {
        // receive link idle event, pcie pcie do idle
        pcie_pm_do_idle(pm_info);
    }
}

static pcie_pm_state_idle_process(struct pcie_pm_info *pm_info)
{
    if (pm_info->outer_event == PCIE_PM_STATE_POWER_UP) {
        // receive power up event, cancel pcie idle
        pcie_pm_do_idle_to_link_up(pm_info);
    }
    if (pm_info->inner_event == PCIE_PM_STATE_PRESUSPEND) {
        // vote clear timeout, start check ep vote
        pcie_pm_do_presuspend(pm_info);
    }
}

static pcie_pm_state_presd_process(struct pcie_pm_info *pm_info)
{
    if (pm_info->inner_event == PCIE_PM_STATE_POWER_DOWN) {
        // ep vote clear, start pcie sleep
        pcie_pm_do_power_down(pm_info);
    } else if (pm_info->inner_event == PCIE_PM_STATE_LINK_IDLE) {
        // ep has vote, back to idle
        pcie_pm_do_presuspend_to_idle(pm_info);
    } else if (pm_info->outer_event == PCIE_PM_STATE_POWER_UP) {
        msleep(1);
        pcie_pm_wakeup_task(pm_info);  // record power up event
    }
}

static pcie_pm_state_pd_process(struct pcie_pm_info *pm_info)
{
    if (pm_info->outer_event == PCIE_PM_STATE_POWER_UP) {
        // pcie sleep done, receive power up event, provide rc clk
        pcie_pm_do_power_up(pm_info);
    }
}

static pcie_pm_state_pu_process(struct pcie_pm_info *pm_info)
{
    if (pm_info->inner_event == PCIE_PM_STATE_LINK_UP) {
        // receive ep pcie ready , establish link and notify
        pcie_pm_do_link_up(pm_info);
    }
}

int pcie_pm_thread(void *data)
{
    struct pcie_pm_info *pm_info = (struct pcie_pm_info *)data;

    while (1) {
        pm_info->pcie_thread_done = 1;
        while (down_interruptible(&pm_info->pm_task_sem)) { }
        pm_info->pcie_thread_done = 0;

        switch (pm_info->current_state) {
            case PCIE_PM_STATE_UNINITED:
            case PCIE_PM_STATE_RESETTING:
                break;
            case PCIE_PM_STATE_LINK_UP:
                pcie_pm_state_linkup_process(pm_info);
                break;
            case PCIE_PM_STATE_LINK_IDLE:
                pcie_pm_state_idle_process(pm_info);
                break;
            case PCIE_PM_STATE_PRESUSPEND:
                pcie_pm_state_presd_process(pm_info);
                break;
            case PCIE_PM_STATE_POWER_DOWN:
                pcie_pm_state_pd_process(pm_info);
                break;
            case PCIE_PM_STATE_POWER_UP:
                pcie_pm_state_pu_process(pm_info);
                break;
            default:
                PCIE_PM_TRACE("error pcie inner pm state, %d", pm_info->current_state);
        }
    }
    return 0;
}

void pcie_pm_idle_timeout_handler(struct work_struct *work)
{
    g_pcie_pm_info.inner_event = PCIE_PM_STATE_PRESUSPEND;

    pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_PRESUSPEND);

    pcie_pm_event_dbg(SUSPEND_TIMEOUT);

    pcie_pm_wakeup_task(&g_pcie_pm_info);
}

void pcie_ep_init_timeout_handler(struct work_struct *work)
{
    u32 passed_slice;
    u64 passed_ms;
    u32 slice_freq = bsp_get_slice_freq();

    g_pcie_pm_info.powerup_timeout_slice = bsp_get_slice_value();
    g_pcie_pm_info.powerup_timeout_jiffies = jiffies;

    passed_slice = get_timer_slice_delta(g_pcie_pm_info.powerup_slice, g_pcie_pm_info.powerup_timeout_slice);
    passed_ms = (u64)(passed_slice * MSEC_PER_SEC) / (u64)slice_freq;
    if (passed_ms < (EP_INIT_MAX_WAIT_MS / EP_INIT_WAIT_DIV)) {
        PCIE_PM_TRACE("ep init timeout invalid, only passed %d ms", passed_ms);
        PCIE_PM_TRACE("before schedule jiffies : 0x%lx, ep init work scheduled, jiffies : 0x%lx",
                      g_pcie_pm_info.powerup_jiffies, g_pcie_pm_info.powerup_timeout_jiffies);
        return;
    } else {
        PCIE_PM_TRACE("after %d ms, wait ep init done timeout, call system_error", passed_ms);
        system_error(DRV_ERRNO_PCIE_ABNORMAL, RC_POWERUP_WAIT_TIMEOUT, 0, NULL, 0);
        return;
    }
}

int bsp_pcie_pm_suspend(void)
{
    int ret = 0;

    g_pcie_pm_info.outer_event = PCIE_PM_STATE_LINK_IDLE;

    pcie_pm_state_dbg(OUTER_EVENT, PCIE_PM_STATE_LINK_IDLE);

    pcie_pm_event_dbg(GET_API_SUSPEND);

    pcie_pm_wakeup_task(&g_pcie_pm_info);

    return ret;
}

int bsp_pcie_pm_resume(void)
{
    int ret = 0;

    g_pcie_pm_info.outer_event = PCIE_PM_STATE_POWER_UP;

    pcie_pm_state_dbg(OUTER_EVENT, PCIE_PM_STATE_POWER_UP);

    pcie_pm_event_dbg(GET_API_RESUME);

    pcie_pm_wakeup_task(&g_pcie_pm_info);

    return ret;
}

void pcie_pm_gpio_release(void)
{
    struct pcie_pm_info *pm_info = &g_pcie_pm_info;
    disable_irq(gpio_to_irq(pm_info->ep_status_gpio_id));
    free_irq(gpio_to_irq(pm_info->ep_status_gpio_id), NULL);
    pcie_pm_set_rc_ready_gpio(pm_info, 0);
    gpio_free(pm_info->rc_status_gpio_id);
    gpio_free(pm_info->ep_status_gpio_id);
}

void pcie_pm_debug_info_clear(void)
{
    int i;

    for (i = 0; i < RC_PCIE_PM_MSG_NUM; i++) {
        g_pcie_pm_info.send_msg_count[i] = 0;
    }

    for (i = 0; i < EP_PCIE_PM_MSG_NUM; i++) {
        g_pcie_pm_info.receive_msg_count[i] = 0;
    }

    g_pcie_pm_info.receive_status_gpio_count = 0;
    g_pcie_pm_info.send_status_gpio_count[GPIO_SET] = 0;
    g_pcie_pm_info.send_status_gpio_count[GPIO_CLEAR] = 0;
}

int bsp_pcie_pm_modem_reset_cb(void)
{
    struct pcie_pm_info *pm_info = &g_pcie_pm_info;

    (void)cancel_delayed_work_sync(&pm_info->idle_timeout_work);
    (void)cancel_delayed_work_sync(&pm_info->ep_init_timeout_work);

    pm_info->current_state = PCIE_PM_STATE_LINK_UP;
    pm_info->inner_event = PCIE_PM_STATE_UNINITED;
    pm_info->outer_event = PCIE_PM_STATE_UNINITED;

    while (!pm_info->pcie_thread_done) { } // wait for unfinished thread handle done

    pcie_pm_state_dbg(CURRENT_STATE, PCIE_PM_STATE_LINK_UP);
    pcie_pm_state_dbg(INNER_EVENT, PCIE_PM_STATE_UNINITED);
    pcie_pm_state_dbg(OUTER_EVENT, PCIE_PM_STATE_UNINITED);

    wakeup_source_trash(&g_pcie_pm_info.pm_wake_lock);

    if (pm_info->pm_bar_virt_addr != NULL) {
        iounmap(pm_info->pm_bar_virt_addr);
        pm_info->pm_bar_phys_addr = 0;
    }

    pcie_pm_gpio_release();
    pcie_pm_debug_info_clear();
    bsp_pcie_rc_msi_free(PCIE_EP_MSI_PM);

    return 0;
}

int pcie_pm_get_dts(void)
{
    int ret = 0;
    struct device_node *dev_node = NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "iot,pcie_iot");
    if (dev_node == NULL) {
        PCIE_PM_TRACE("pcie device not found!");
        return -1;
    }

    ret |= of_property_read_u32(dev_node, "ep_status_gpio_id", &g_pcie_pm_info.ep_status_gpio_id);
    ret |= of_property_read_u32(dev_node, "rc_status_gpio_id", &g_pcie_pm_info.rc_status_gpio_id);
    if (ret) {
        PCIE_PM_TRACE("fail to get iot pcie pm dts info, ret = %d", ret);
        return ret;
    }
    return 0;
}

int pcie_pm_gpio_init(void)
{
    int ret;

    ret = gpio_request(g_pcie_pm_info.rc_status_gpio_id, "rc_status_gpio");
    if (ret) {
        PCIE_PM_TRACE("rc_status_gpio request failed, ret %d", ret);
        goto err_free0;
    }
    ret = gpio_request(g_pcie_pm_info.ep_status_gpio_id, "ep_status_gpio");
    if (ret) {
        PCIE_PM_TRACE("ep_status_gpio request failed, ret %d", ret);
        goto err_free1;
    }

    ret = gpio_direction_input(g_pcie_pm_info.ep_status_gpio_id);
    if (ret) {
        PCIE_PM_TRACE("failed to set ep status gpio to input, ret = %d\n", ret);
        return ret;
    }

    ret = request_threaded_irq(gpio_to_irq(g_pcie_pm_info.ep_status_gpio_id), pcie_pm_ep_status_gpio_handler, NULL,
                               IRQF_TRIGGER_HIGH, "pcie_ep_status_gpio_notify", NULL);
    if (ret) {
        PCIE_PM_TRACE("request ep pcie status gpio interrupt failed, ret %d.", ret);
        goto err_free2;
    }

    disable_irq_nosync(gpio_to_irq(g_pcie_pm_info.ep_status_gpio_id));

    return ret;

err_free2:
    gpio_free(g_pcie_pm_info.ep_status_gpio_id);
err_free1:
    gpio_free(g_pcie_pm_info.rc_status_gpio_id);

err_free0:
    return ret;
}

int pcie_pm_msi_init(void)
{
    int ret;

    ret = bsp_pcie_rc_msi_request(PCIE_EP_MSI_PM, pcie_pm_msi_handler, "pcie_pm_msg_msi", NULL);
    if (ret) {
        PCIE_PM_TRACE("fail to request rc pm receive msi handler, ret %d", ret);
        return ret;
    }

    return ret;
}

void *bsp_pcie_pm_get_bar_virt_addr(void)
{
    if (g_pcie_pm_info.pm_bar_virt_addr != NULL) {
        return g_pcie_pm_info.pm_bar_virt_addr;
    } else {
        return NULL;
    }
}

int pcie_pm_bar_init(void)
{
    unsigned long pcie_pm_bar_phys_addr;
    unsigned long pcie_pm_bar_size;
    void *pcie_pm_bar_virt_addr = NULL;

    pcie_pm_bar_phys_addr = bsp_pcie_rc_get_bar_addr(PCIE_BAR_PM_CTL);
    pcie_pm_bar_size = bsp_pcie_rc_get_bar_size(PCIE_BAR_PM_CTL);
    if (!pcie_pm_bar_phys_addr || !pcie_pm_bar_size) {
        PCIE_PM_TRACE("pcie pm get bar resource fail");
        return -1;
    }

    pcie_pm_bar_virt_addr = ioremap_wc(pcie_pm_bar_phys_addr, pcie_pm_bar_size);
    if (pcie_pm_bar_virt_addr == NULL) {
        PCIE_PM_TRACE("pcie pm bar addr remap fail");
        return -1;
    }

    g_pcie_pm_info.pm_bar_phys_addr = pcie_pm_bar_phys_addr;
    g_pcie_pm_info.pm_bar_virt_addr = pcie_pm_bar_virt_addr;

    return 0;
}

int bsp_pcie_rc_pm_init(void)
{
    PCIE_PM_TRACE("pcie pm probe +");

    if (!g_pcie_pm_init_flag) {
        spin_lock_init(&g_pcie_pm_info.spinlock);
        sema_init(&g_pcie_pm_info.pm_task_sem, 0);
        INIT_DELAYED_WORK(&g_pcie_pm_info.idle_timeout_work, pcie_pm_idle_timeout_handler);
        INIT_DELAYED_WORK(&g_pcie_pm_info.ep_init_timeout_work, pcie_ep_init_timeout_handler);

        g_pcie_pm_info.pm_task = kthread_run(pcie_pm_thread, &g_pcie_pm_info, "pcie_rc_pm");
        if (IS_ERR(g_pcie_pm_info.pm_task)) {
            g_pcie_pm_info.pm_task = NULL;
            PCIE_PM_TRACE("fail to start rc pm thread");
            goto err_ret;
        }
    }

    wakeup_source_init(&g_pcie_pm_info.pm_wake_lock, "pcie_rc_pm_wakelock");
    __pm_stay_awake(&g_pcie_pm_info.pm_wake_lock);

    g_pcie_pm_info.current_state = PCIE_PM_STATE_LINK_UP;
    g_pcie_pm_info.inner_event = PCIE_PM_STATE_UNINITED;
    g_pcie_pm_info.outer_event = PCIE_PM_STATE_UNINITED;
    g_pcie_pm_info.pm_args.timeout_ms = PCIE_NO_VOTE_TIMEOUT_MS;
    g_pcie_pm_info.pm_args.pm_mode = PCIE_LOW_POWER_MODE;
    g_pcie_pm_info.pcie_port_id = bsp_pcie_get_port_id();

    if (pcie_pm_get_dts()) {
        PCIE_PM_TRACE("fail to get dts");
        goto err_ret;
    }

    if (pcie_pm_gpio_init()) {
        PCIE_PM_TRACE("fail to init gpio");
        goto err_ret;
    }

    if (pcie_pm_msi_init()) {
        PCIE_PM_TRACE("fail to init msi");
        goto err_ret;
    }

    if (pcie_pm_bar_init()) {
        PCIE_PM_TRACE("fail to init bar");
        goto err_ret;
    }

    PCIE_PM_TRACE("pcie pm probe -");
    g_pcie_pm_init_flag = 1;

    return 0;

err_ret:
    wakeup_source_trash(&g_pcie_pm_info.pm_wake_lock);
    return -1;
}

void set_pcie_current_state_debug(u32 state)
{
    g_pcie_pm_info.current_state = state;
}

void set_pcie_inner_event_debug(u32 event)
{
    g_pcie_pm_info.inner_event = event;
}

void set_pcie_outer_event_debug(u32 event)
{
    g_pcie_pm_info.outer_event = event;
}

void set_pcie_timeout_debug(u32 timeout_ms)
{
    g_pcie_pm_info.pm_args.timeout_ms = timeout_ms;
}

void pcie_pm_debug(void)
{
    int i;

    printk(KERN_ERR "*************************** PCIE RC PM STATE INFO ************************** \n");

    printk(KERN_ERR "pcie pm current_state: %d\n", g_pcie_pm_info.current_state);
    printk(KERN_ERR "pcie pm inner_event: %d\n", g_pcie_pm_info.inner_event);
    printk(KERN_ERR "pcie pm outer_event: %d\n", g_pcie_pm_info.outer_event);

    for (i = 0; i < RC_PCIE_PM_MSG_NUM; i++) {
        if (g_pcie_pm_info.send_msg_count[i]) {
            printk(KERN_ERR "pcie pm send msg num %d, count: %ld\n", i, g_pcie_pm_info.send_msg_count[i]);
        }
    }

    for (i = 0; i < EP_PCIE_PM_MSG_NUM; i++) {
        if (g_pcie_pm_info.receive_msg_count[i]) {
            printk(KERN_ERR "pcie pm receive msg num %d, count: %ld\n", i, g_pcie_pm_info.receive_msg_count[i]);
        }
    }

    printk(KERN_ERR "receive status gpio, total count: %ld\n", g_pcie_pm_info.receive_status_gpio_count);

    printk(KERN_ERR "send status gpio, set_count : %ld, clear_count: %ld\n",
           g_pcie_pm_info.send_status_gpio_count[GPIO_SET], g_pcie_pm_info.send_status_gpio_count[GPIO_CLEAR]);

    printk(KERN_ERR "*************************** PCIE RC PM STATE INFO ************************** \n");
}
