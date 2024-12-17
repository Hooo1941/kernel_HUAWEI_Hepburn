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

#include <linux/types.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <bsp_pcie.h>
#include <bsp_dt.h>
#include <product_config.h>
#include <osl_types.h>

u32 g_modem_boot_gpio = 0;
u32 g_modem_boot_pcie_dbg_en;
struct semaphore g_modem_boot_sem;
struct task_struct *g_modem_boot_wait_task;

int modem_boot_callback(u32 usr_id, u32 cb_id, void *callback_args)
{
    if (cb_id == PCIE_EP_CB_PCIE_INIT_DONE) {
        up(&g_modem_boot_sem);
    }
    return 0;
}

int modem_boot_register_ep_pcie(void)
{
    int ret;
    struct pcie_callback_info pcie_mboot_callback_info = { 0 };
    pcie_mboot_callback_info.callback = modem_boot_callback;
    ret = bsp_pcie_ep_cb_register(PCIE_USER_MBOOT, &pcie_mboot_callback_info);
    return ret;
}

#ifndef CONFIG_PCIE_NORMALIZE
static int modem_boot_get_property(void)
{
    device_node_s *dev_node;

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "modem_loader");
    if (dev_node == NULL) {
        printk(KERN_ERR "[MODEM_BOOT]find modem_loader dts failed!\n");
        return -1;
    }

    if (bsp_dt_property_read_u32(dev_node, "gpio_boot", &g_modem_boot_gpio)) {
        printk(KERN_ERR "[MODEM_BOOT]read gpio_boot failed.\n");
        return -1;
    }

    return 0;
}
#endif

int modem_boot_gpio_init(void)
{
#ifndef CONFIG_PCIE_NORMALIZE
    int ret;

    ret = modem_boot_get_property();
    if (ret) {
        return ret;
    }
    ret = gpio_request(g_modem_boot_gpio, "modem_boot_gpio");
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]gpio request failed, ret %d. \n", ret);
        return ret;
    }
#endif
    return 0;
}

int modem_boot_gpio_rescan_notify(void)
{
#ifndef CONFIG_PCIE_NORMALIZE
    int ret;

    ret = gpio_direction_output(g_modem_boot_gpio, 1);
    if (ret) {
        printk("[MODEM_BOOT]fail to set GPIO<%u> to 1, ret = %d\n", g_modem_boot_gpio, ret);
        return -1;
    }

    ret = gpio_direction_output(g_modem_boot_gpio, 0);
    if (ret) {
        printk("[MODEM_BOOT]fail to set GPIO<%u> to 0, ret = %d\n", g_modem_boot_gpio, ret);
        return -1;
    }
#endif
    return 0;
}

void modem_boot_pcie_dbg_init(void)
{
#ifdef CONFIG_IOT_PCIE_DEBUG
    int ret = bsp_pcie_dump_init();
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie ep pm dump init fail, ret %d\n", ret);
        g_modem_boot_pcie_dbg_en = 0;
    } else {
        g_modem_boot_pcie_dbg_en = 1;
    }
#endif
}

void modem_boot_pcie_dbg(int stage)
{
#ifdef CONFIG_IOT_PCIE_DEBUG
    if (g_modem_boot_pcie_dbg_en) {
        bsp_pcie_boot_dbg(stage);
    }
#else
    UNUSED(stage);
#endif
}

int modem_boot_pcie_init(void)
{
    int ret;

    ret = bsp_iot_pcie_init();
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie ep pcie init fail, ret %d\n", ret);
        return ret;
    }

    if (!bsp_pcie_ep_port_available()) {
        printk(KERN_ERR "[MODEM_BOOT]pcie ep pcie port not enabled\n");
        return -1;
    }

    modem_boot_pcie_dbg(PCIE_KERNEL_INIT_DONE);

#ifdef CONFIG_IOT_PCIE_EP_PM
    ret = bsp_pcie_ep_pm_init();
    ret |= bsp_pcie_ep_pm_api_init();
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie ep pm init fail\n");
        return -1;
    }
    modem_boot_pcie_dbg(PCIE_PM_INIT_DONE);
#endif

    ret = bsp_pcie_ep_cb_run(PCIE_EP_CB_BAR_CONFIG);
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie ep callback bar fail, ret %d\n", ret);
        return ret;
    }

    ret = modem_boot_register_ep_pcie();
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie mboot pcie callback fail, ret %d\n", ret);
        return ret;
    }

    ret = bsp_pcie_ep_callback_msi_register();
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie ep callback msi fail, ret %d\n", ret);
        return ret;
    }

    return 0;
}

int modem_boot_wait_thread(void *data)
{
    down(&g_modem_boot_sem);
    modem_boot_pcie_dbg(PCIE_USER_INIT_DONE);
    printk(KERN_ERR "[MODEM_BOOT]modem boot init success, all user callback run over\n");
    return 0;
}

int modem_boot_init(void)
{
    int ret;

    sema_init(&g_modem_boot_sem, 0);

    modem_boot_pcie_dbg_init();

    ret = modem_boot_gpio_init();
    if (ret) {
        return ret;
    }

    ret = modem_boot_pcie_init();
    if (ret) {
        return ret;
    }

    ret = modem_boot_gpio_rescan_notify();
    if (ret) {
        printk(KERN_ERR "[MODEM_BOOT]pcie rescan gpio notify failed.\n");
        return ret;
    }

    gpio_free(g_modem_boot_gpio);

    modem_boot_pcie_dbg(PCIE_SEND_GPIO);

    printk(KERN_ERR "[MODEM_BOOT]pcie init done, wait for rc to establish link\n");

#ifdef CONFIG_PCIE_NORMALIZE
    g_modem_boot_wait_task = kthread_run(modem_boot_wait_thread, NULL, "modem_boot_wait_thread");
    if (IS_ERR(g_modem_boot_wait_task)) {
        ret = PTR_ERR(g_modem_boot_wait_task);
        printk(KERN_ERR "fail to start modem boot wait thread, err_code: %d\n", ret);
        g_modem_boot_wait_task = NULL;
        return ret;
    }
#else
    modem_boot_wait_thread(NULL);
#endif
    return 0;
}

EXPORT_SYMBOL(modem_boot_init);

late_initcall(modem_boot_init);
