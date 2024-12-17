/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/gpio-fan.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <securec.h>
#include <linux/reboot.h>
#include "adrv_load.h"

#include <linux/time.h>
#include <linux/timer.h>
#include "product_config.h"
#include "bsp_pcie.h"
#include "bsp_dump.h"
#include "bsp_slice.h"
#include "bsp_pmu_gpio.h"
#include "bsp_print.h"
#include <bsp_dt.h>
#include <mdrv_errno.h>
#include "bsp_dump.h"
#include "modem_load.h"
#include "modem_boot.h"
#include <linux/dma-mapping.h>
#include <bsp_modem_patch.h>

#define THIS_MODU mod_modem_boot
#define MODEM_BOOT_START 1

DECLARE_COMPLETION(g_modem_boot_start_complete);

struct modem_boot_state g_modem_state_info;
int g_modemready_state = 0;
int g_modem_poweroff_state = 0;
struct delayed_work g_wait_modemready_work;
struct platform_device *g_modem_boot_platform_pdev;

int modem_boot_get_gpio(void)
{
    return gpio_get_value(g_modem_state_info.gpio_rescan);
}

void modem_boot_gpio_handle_func(struct work_struct *work)
{
    int ret = modem_boot_get_gpio(); // change to falling detect
    if (ret == 0x0) {
        bsp_pcie_boot_dbg(PCIE_LOAD_IMAGE_DONE);
        bsp_modem_boot_trigger_event(MODEM_BOOT_STATE_EP_PCIE_DONE);
    }
    bsp_err("gpio interrupt handler, status = %d\n", ret);
}

struct delayed_work g_modem_boot_gpio_handle_work;
irqreturn_t modem_boot_gpio_handler(int irq, void *data)
{
    (void)schedule_delayed_work(&g_modem_boot_gpio_handle_work, 1);

    return IRQ_HANDLED;
}

int modem_boot_gpio_init(void)
{
    int ret;

    INIT_DELAYED_WORK(&g_modem_boot_gpio_handle_work, modem_boot_gpio_handle_func);

    ret = gpio_request(g_modem_state_info.gpio_rescan, "modem_boot_gpio");
    if (ret) {
        bsp_err("gpio request failed, ret = %d\n", ret);
        return ret;
    }

    ret = gpio_direction_input(g_modem_state_info.gpio_rescan);
    if (ret) {
        bsp_err("failed to set gpio_rescan to input, ret = %d\n", ret);
        return ret;
    }

    ret = request_threaded_irq(gpio_to_irq(g_modem_state_info.gpio_rescan), modem_boot_gpio_handler, NULL,
        IRQF_TRIGGER_FALLING, "modem_boot_gpio", NULL);
    if (ret) {
        bsp_err("request interrupt failed, ret = %d\n", ret);
        gpio_free(g_modem_state_info.gpio_rescan);
        return ret;
    }

    return ret;
}

void modem_boot_gpio_exit(void)
{
    free_irq(gpio_to_irq(g_modem_state_info.gpio_rescan), NULL);
    gpio_free(g_modem_state_info.gpio_rescan);
}

int bsp_modem_boot_get_status(void)
{
    return g_modem_state_info.current_state;
}

void bsp_modem_boot_set_status(int state)
{
    g_modem_state_info.current_state = state;

    bsp_err("change to new state, state = %d\n", g_modem_state_info.current_state);
}

void bsp_modem_boot_trigger_event(int next_state)
{
    g_modem_state_info.next_state = next_state;

    __pm_stay_awake(&g_modem_state_info.boot_wakelock);

    bsp_err("trigger new boot event, event = %d\n", g_modem_state_info.next_state);

    up(&g_modem_state_info.boot_evnet_sem);
}

void bsp_modem_boot_clear_outer_event(void)
{
    g_modem_state_info.outer_event = MODEM_BOOT_STATE_UNINITED;
    bsp_err("clear modem boot outer event\n");
}

void bsp_modem_boot_trigger_outer_event(int outer_event)
{
    g_modem_state_info.outer_event = outer_event;

    __pm_stay_awake(&g_modem_state_info.boot_wakelock);

    bsp_err("trigger new boot event, event = %d\n", g_modem_state_info.outer_event);

    up(&g_modem_state_info.boot_evnet_sem);
}

void bsp_modem_boot_set_modemready_state(void)
{
    g_modemready_state = 1;
#ifdef CONFIG_MLOADER_HOT_PATCH
    bsp_modem_update_hot_patch_nve();
#endif
    bsp_err("modem boot receive modemready msg\n");
}

void modem_boot_wait_modemready_timeout_handle(struct work_struct *work)
{
    if (g_modemready_state == 0) {
        bsp_err("modem boot wait modemready msg timeout \n");
        system_error(DRV_ERRNO_WAIT_MODEMREADY_TIMEOUT_ERR, 0, 0, NULL, 0);
    }
    return;
}

void modem_boot_cancel_modemready_work(void)
{
    (void)cancel_delayed_work_sync(&g_wait_modemready_work);
}

static int modem_boot_get_property(void)
{
    device_node_s *dev_node;

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "modem_loader");
    if (dev_node == NULL) {
        bsp_err("fail to find modem loader dts\n");
        return -1;
    }

    if (bsp_dt_property_read_u32(dev_node, "gpio_rescan", &g_modem_state_info.gpio_rescan)) {
        bsp_err("read gpio_rescan failed.\n");
        return -1;
    }

    return 0;
}

int modem_boot_power_on_modem(void)
{
    int ret;

    ret = bsp_modem_boot_poweron();
    if (ret) {
        bsp_err("fail to power on modem, ret = %d\n", ret);
        return ret;
    }

    ret = modem_boot_get_property();
    if (ret) {
        return ret;
    }

    ret = bsp_modem_load_init();
    if (ret) {
        return ret;
    }

    bsp_modem_boot_set_status(MODEM_BOOT_STATE_POWER_ON);

    bsp_modem_boot_trigger_event(MODEM_BOOT_STATE_LOADING);

    return ret;
}

#define MODEM_BOOT_WAIT_PMU_READY_MAX_TIME 380
#define MODEM_BOOT_WAIT_MODEM_READY_MAX_TIME 620

int modem_boot_do_load_image(void)
{
    int ret;
    u32 start, end;

    start = bsp_get_slice_value();
    while (1) {
        ret = bsp_get_pmu_status();
        if (!ret) {
            bsp_err("modem pmu is ready\n");
            break;
        }

        if (unlikely(get_timer_slice_delta(start, bsp_get_slice_value()) >
            (MODEM_BOOT_WAIT_PMU_READY_MAX_TIME * bsp_get_slice_freq() / 1000U))) {
            bsp_err("get pmu state time out\n");
            break;
        }
    }
    end = bsp_get_slice_value();
    bsp_err("pmu ready cost 0x%x slice\n", get_timer_slice_delta(start, end));
    msleep(MODEM_BOOT_WAIT_MODEM_READY_MAX_TIME); /* wait until modem ready */

    ret = modem_boot_gpio_init();
    if (ret) {
        bsp_err("fail to register gpio interrupt\n");
        goto gpio_init_error;
    }

    ret = bsp_pcie_dump_init();
    if (ret) {
        bsp_err("pcie rc pm dump init fail, ret %d\n", ret);
    }

    ret = bsp_modem_load_start();
    if (ret) {
        bsp_err("fail to start load\n");
        goto load_image_error;
    }

    bsp_modem_boot_set_status(MODEM_BOOT_STATE_LOADING);

    bsp_pcie_boot_dbg(PCIE_BOOT_LINK);

    return ret;

load_image_error:
    modem_boot_gpio_exit();

gpio_init_error:
    return ret;
}

int modem_boot_do_load_done(void)
{
    bsp_modem_load_stop();
    bsp_modem_boot_set_status(MODEM_BOOT_STATE_LOAD_DONE);
    return 0;
}

int modem_boot_do_ep_pcie_done(void)
{
    modem_boot_gpio_exit();

    bsp_modem_boot_set_status(MODEM_BOOT_STATE_EP_PCIE_DONE);
    bsp_modem_boot_trigger_event(MODEM_BOOT_STATE_BOOT_DONE);
    return 0;
}

int modem_boot_do_pcie_rescan(void)
{
    int ret;

    bsp_pcie_boot_dbg(PCIE_RECEIVE_GPIO);

    bsp_err("receive modem gpio success\n");

    ret = pcie_kport_pm_control(1, 0);
    if (ret) {
        bsp_err("rc fail to link up ep device, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_pcie_boot_rescan();
    if (ret) {
        bsp_err("fail to reenum ep pcie device, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_pcie_modem_kernel_init();
    if (ret) {
        bsp_err("pcie kernel init fail, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_pcie_rc_cb_run(PCIE_RC_CB_ENUM_DONE);
    if (ret) {
        bsp_err("pcie rc fail to callback, ret = %d\n", ret);
        return ret;
    }

    bsp_pcie_boot_dbg(PCIE_USER_INIT_DONE);

    ret = bsp_pcie_rc_notify_ep_user_init();
    if (ret) {
        bsp_err("pcie rc notify rescan fail, ret = %d\n", ret);
        return ret;
    }

    bsp_modem_boot_set_status(MODEM_BOOT_STATE_BOOT_DONE);
    __pm_relax(&g_modem_state_info.work_wakelock);

    bsp_err("modem boot start init end\n");
    return 0;
}

int modem_boot_do_ep_pcie_done_load_image(void)
{
    bsp_modem_boot_clear_outer_event();
    return modem_boot_do_load_image();
}

int modem_boot_do_poweron_reload_image(void)
{
    bsp_modem_boot_clear_outer_event();
    bsp_modem_load_stop();
    return modem_boot_do_load_image();
}

int modem_boot_do_loading_reload_image(void)
{
    bsp_modem_boot_clear_outer_event();
    bsp_modem_load_stop();
    modem_boot_gpio_exit();
    return modem_boot_do_load_image();
}

int modem_boot_do_load_done_reload_image(void)
{
    bsp_modem_boot_clear_outer_event();
    modem_boot_gpio_exit();
    return modem_boot_do_load_image();
}

static int modem_boot_get_thread_flag(unsigned int nr)
{
    return g_modem_state_info.thread_flags & BIT(nr);
}

static int modem_boot_thread_handle(struct modem_boot_state *state_info)
{
    int ret = 0;

    switch (state_info->current_state) {
        case MODEM_BOOT_STATE_POWER_ON:
            if (state_info->outer_event == MODEM_BOOT_STATE_RESETTING) {
                ret = modem_boot_do_poweron_reload_image();
            }
            break;
        case MODEM_BOOT_STATE_LOADING:
            if (state_info->outer_event == MODEM_BOOT_STATE_RESETTING) {
                ret = modem_boot_do_loading_reload_image();
            } else if (state_info->next_state == MODEM_BOOT_STATE_LOAD_DONE) {
                ret = modem_boot_do_load_done();
            }
            break;
        case MODEM_BOOT_STATE_LOAD_DONE:
            if (state_info->outer_event == MODEM_BOOT_STATE_RESETTING) {
                ret = modem_boot_do_load_done_reload_image();
            } else if (state_info->next_state == MODEM_BOOT_STATE_EP_PCIE_DONE) {
                ret = modem_boot_do_ep_pcie_done();
            }
            break;
        case MODEM_BOOT_STATE_EP_PCIE_DONE:
            if (state_info->outer_event == MODEM_BOOT_STATE_RESETTING) {
                ret = modem_boot_do_ep_pcie_done_load_image();
            } else if (state_info->next_state == MODEM_BOOT_STATE_BOOT_DONE) {
                ret = modem_boot_do_pcie_rescan();
            }
            break;
        case MODEM_BOOT_STATE_BOOT_DONE:
            if (state_info->outer_event == MODEM_BOOT_STATE_RESETTING) {
                ret = modem_boot_do_ep_pcie_done_load_image();
            }
            break;
        default:
            bsp_err("error boot state, state = %d\n", state_info->current_state);
    }

    return ret;
}

static int modem_boot_thread(void *data)
{
    int ret;
    struct modem_boot_state *state_info = NULL;

    state_info = (struct modem_boot_state *)data;

    bsp_err("wait for start modem\n");

    bsp_modem_boot_set_status(MODEM_BOOT_STATE_INITED);

    wait_for_completion(&g_modem_boot_start_complete);

    if (!bsp_need_loadmodem()) {
        bsp_err("update or charge mode, will not start modem\n");
        __pm_relax(&g_modem_state_info.work_wakelock);
        return 0;
    }

    ret = modem_boot_power_on_modem();
    if (ret) {
        bsp_err("fail to power on modem, ret = %d\n", ret);
        return ret;
    }

    // after max wait, call system error
    (void)schedule_delayed_work(&g_wait_modemready_work, msecs_to_jiffies(150000U));

    ret = modem_boot_do_load_image();
    if (ret) {
        bsp_err("fail to load modem, ret = %d\n", ret);
    }

    while (!kthread_should_stop()) {
        while (down_interruptible(&g_modem_state_info.boot_evnet_sem)) {
            ;
        }

        if (modem_boot_get_thread_flag(MODEM_BOOT_THREAD_FLAG_BIT_SIGNAL_HALT)) {
            set_bit(MODEM_BOOT_THREAD_FLAG_BIT_STATE_HALT, &g_modem_state_info.thread_flags);
            bsp_err("modem boot thread receive halt signal\n");
            goto done;
        }

        ret = modem_boot_thread_handle(state_info);

    done:
        __pm_relax(&g_modem_state_info.boot_wakelock);
    }

    return ret;
}

#define MODEM_BOOT_GET_THREAD_FLAG_TIMEOUT 300

void modem_boot_hold_task_enable(int outer_event)
{
    int timeout;

    /* hold modem boot thread */
    clear_bit(MODEM_BOOT_THREAD_FLAG_BIT_STATE_HALT, &g_modem_state_info.thread_flags);
    set_bit(MODEM_BOOT_THREAD_FLAG_BIT_SIGNAL_HALT, &g_modem_state_info.thread_flags);
    bsp_err("send modem boot thread halt signal\n");
    bsp_modem_boot_trigger_outer_event(outer_event);

    timeout = MODEM_BOOT_GET_THREAD_FLAG_TIMEOUT;
    while (!modem_boot_get_thread_flag(MODEM_BOOT_THREAD_FLAG_BIT_STATE_HALT)) {
        msleep(10U);
        timeout--;
        if (timeout < 0) {
            bsp_err("wait for boot thread halt timeout\n");
            return;
        }
    }
    bsp_err("modem boot thread has halt\n");

    /* hold modem load thread */
    bsp_modem_load_thread_hold_enable();
}

void modem_boot_hold_task_disable(void)
{
    bsp_modem_load_thread_hold_disable();

    clear_bit(MODEM_BOOT_THREAD_FLAG_BIT_SIGNAL_HALT, &g_modem_state_info.thread_flags);
    bsp_err("clear modem boot thread halt signal\n");
}

ssize_t modem_sysboot_start_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int status = count;
    long start_flag = 0;

    if (kstrtol(buf, 10U, &start_flag) < 0) {
        return -EINVAL;
    }

    if (start_flag == MODEM_BOOT_START) {
        __pm_stay_awake(&g_modem_state_info.work_wakelock);
        complete(&g_modem_boot_start_complete);
        bsp_err("modem_boot_start write ok \n");
    } else {
        bsp_err("modem_boot_start write is %x\n", (unsigned int)start_flag);
    }

    return status;
}

struct platform_device *modem_boot_get_device(void)
{
    return g_modem_boot_platform_pdev;
}

static DEVICE_ATTR(modem_sysboot_start, S_IWUSR, NULL, modem_sysboot_start_store);

#ifdef CONFIG_MLOADER_HOT_PATCH
static DEVICE_ATTR(modem_image_patch_status, 0660, modem_imag_patch_status_show, modem_imag_patch_status_store);
#endif

static struct attribute *g_modem_boot_attributes[] = { &dev_attr_modem_sysboot_start.attr,
#ifdef CONFIG_MLOADER_HOT_PATCH
                                                       &dev_attr_modem_image_patch_status.attr,
#endif
                                                       NULL };

static const struct attribute_group g_modem_boot_group = {
    .attrs = g_modem_boot_attributes,
};

int modem_boot_probe(struct platform_device *pdev)
{
    int ret;
    struct task_struct *modem_boot_task = NULL;

    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64U));
    g_modem_boot_platform_pdev = pdev;

    sema_init(&g_modem_state_info.boot_evnet_sem, 0);
    wakeup_source_init(&g_modem_state_info.boot_wakelock, "modem_boot_wake_lock");
    wakeup_source_init(&g_modem_state_info.work_wakelock, "modem_boot_work_wake_lock");

    INIT_DELAYED_WORK(&g_wait_modemready_work, modem_boot_wait_modemready_timeout_handle);

    g_modem_state_info.current_state = MODEM_BOOT_STATE_UNINITED;
    g_modem_state_info.next_state = MODEM_BOOT_STATE_UNINITED;
    g_modem_state_info.outer_event = MODEM_BOOT_STATE_UNINITED;

    modem_boot_task = kthread_run(modem_boot_thread, (void *)&g_modem_state_info, "modem_boot_thread");
    if (IS_ERR(modem_boot_task)) {
        bsp_err("fail to create sysfs file, ret = %ld\n", PTR_ERR(modem_boot_task));
        return -1;
    }

    ret = sysfs_create_group(&pdev->dev.kobj, &g_modem_boot_group);
    if (ret) {
        bsp_err("fail to create sysfs file, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

void modem_boot_poweroff(void)
{
    if (g_modem_poweroff_state) {
        bsp_err("modem has been powered off, ignore repeat request\n");
        return;
    }
    modem_boot_hold_task_enable(MODEM_BOOT_STATE_SHUTDOWN); // stop modem boot thread
    bsp_modem_boot_set_status(MODEM_BOOT_STATE_SHUTDOWN);   // set current state to shutdown
    g_modem_poweroff_state = 0x1;
}

void modem_boot_platform_poweroff(struct platform_device *pdev)
{
    int ret;

    ret = bsp_modem_boot_poweroff();
    if (ret) {
        bsp_err("fail to power off modem, ret = %d\n", ret);
    }
    return;
}

static struct platform_device g_modem_boot_device = {
    .name = "modem_boot",
    .id = 0,
    .dev = {
        .init_name = "modem_boot",
    }
};

static struct platform_driver g_modem_boot_driver = {
    .probe      = modem_boot_probe,
    .shutdown   = modem_boot_platform_poweroff,
    .driver     = {
        .name     = "modem_boot",
        .owner    = THIS_MODULE,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int modem_boot_halt(struct notifier_block *nb, unsigned long event, void *buf)
{
    bsp_err("notifier shutdown in, event = 0x%lx\n", event);
    switch (event) {
        case SYS_RESTART:
        case SYS_POWER_OFF:
        case SYS_HALT:
            modem_boot_poweroff();
            break;
        default:
            return 0;
    }
    bsp_err("notifier shutdown out\n");
    return 0;
}

static struct notifier_block g_modem_boot_notifier = {
    .notifier_call = modem_boot_halt,
};

int modem_boot_init(void)
{
    int ret;

    ret = platform_device_register(&g_modem_boot_device);
    if (ret) {
        bsp_err("register modem_boot_dev failed\n");
        return ret;
    }

    ret = platform_driver_register(&g_modem_boot_driver);
    if (ret) {
        bsp_err("register modem_boot_drv failed\n");
        platform_device_unregister(&g_modem_boot_device);
        return ret;
    }

    register_reboot_notifier(&g_modem_boot_notifier);
    return ret;
}

int bsp_modem_boot_reset(void)
{
    (void)cancel_delayed_work_sync(&g_wait_modemready_work);

    modem_load_debug_print();
#ifdef CONFIG_MLOADER_HOT_PATCH
    modem_hot_patch_debug_print();
#endif
    modem_load_debug_clear();

    modem_boot_hold_task_enable(MODEM_BOOT_STATE_RESETTING);

    if (bsp_modem_boot_get_status() < MODEM_BOOT_STATE_LOAD_DONE) {
        bsp_show_mdmext_pstore();
    }

    if (bsp_modem_boot_get_status() < MODEM_BOOT_STATE_BOOT_DONE) {
        __pm_relax(&g_modem_state_info.work_wakelock);
    }
    return 0;
}

int bsp_modem_boot_start(void)
{
    __pm_stay_awake(&g_modem_state_info.work_wakelock);
    modem_boot_hold_task_disable();

    bsp_modem_boot_trigger_outer_event(MODEM_BOOT_STATE_RESETTING);

    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(modem_boot_init);
#endif
