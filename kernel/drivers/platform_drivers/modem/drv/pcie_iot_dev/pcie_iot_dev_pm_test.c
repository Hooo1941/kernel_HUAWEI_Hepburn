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
 * Description: PCIE IOT Host Power Management Test Driver.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <bsp_pcie.h>
#include <linux/random.h>
#include <osl_types.h>

#define WAIT_MS_RANDOM_SHIFT 22
#define PM_TEST_WAIT_INTERVAL 2000
#define MSLEEP_MS 6000

enum pm_test_task_enum {
    PM_TEST_TASK_A,
    PM_TEST_TASK_B,
    PM_TEST_TASK_C,
    PM_TEST_TASK_D,
    PM_TEST_TASK_NUM
};

struct pcie_pm_test_info {
    struct task_struct *test_task[PM_TEST_TASK_NUM];
    volatile int pcie_random_test;
    volatile int stop_task_run_flag;
    u32 lock_interval_ms;
    struct semaphore rc_start_pm_test;
    struct task_struct *rc_pm_user_test_task;
    void *pm_test_virt_bar_addr;
};

struct pcie_pm_test_info g_pcie_pm_test_info;

void set_test_interval(u32 interval_ms)
{
    g_pcie_pm_test_info.lock_interval_ms = interval_ms;
}

void stop_task_run(void)
{
    g_pcie_pm_test_info.stop_task_run_flag = 1;
}

void continue_task_run(void)
{
    g_pcie_pm_test_info.stop_task_run_flag = 0;
}

void set_pcie_random_interval_test(void)
{
    g_pcie_pm_test_info.pcie_random_test = 1;
}

void set_pcie_same_interval_test(void)
{
    g_pcie_pm_test_info.pcie_random_test = 0;
}

int rc_pm_bar_test(u32 offset)
{
    int read_back;
    writel(0x1, g_pcie_pm_test_info.pm_test_virt_bar_addr + PCIE_RC_PM_BAR_TEST_OFFSET + offset);
    read_back = readl(g_pcie_pm_test_info.pm_test_virt_bar_addr + PCIE_RC_PM_BAR_TEST_OFFSET + offset);
    if (read_back != 0x1) {
        printk(KERN_ERR "bar test failed, offset %d write 0x1, but read back %d\n", offset, read_back);
        return -1;
    }
    writel(0x0, g_pcie_pm_test_info.pm_test_virt_bar_addr + PCIE_RC_PM_BAR_TEST_OFFSET + offset);
    return 0;
}

// 模拟4个用户并发连续执行，中间间隔固定的timeout时间
int multi_task_a(void *data)
{
    u32 wait_random = 0;
    u32 wait_ms;
    do {
        if (g_pcie_pm_test_info.pcie_random_test) {
            get_random_bytes(&wait_random, sizeof(u32));
            wait_ms = wait_random >> WAIT_MS_RANDOM_SHIFT;  // 0~1024
        } else {
            wait_ms = g_pcie_pm_test_info.lock_interval_ms;
        }
        bsp_pcie_rc_vote_lock(PCIE_USER_CHAR_DEV, 0x1);
        msleep(wait_ms);
        if (rc_pm_bar_test(0x10)) {
            stop_task_run();
            bsp_pcie_rc_vote_unlock(PCIE_USER_CHAR_DEV);
            return -1;
        }
        bsp_pcie_rc_vote_unlock(PCIE_USER_CHAR_DEV);
        msleep(wait_ms);
        while (g_pcie_pm_test_info.stop_task_run_flag) { }
    } while (1);

    return 0;
}

int multi_task_b(void *data)
{
    u32 wait_random = 0;
    u32 wait_ms;
    do {
        if (g_pcie_pm_test_info.pcie_random_test) {
            get_random_bytes(&wait_random, sizeof(u32));
            wait_ms = wait_random >> WAIT_MS_RANDOM_SHIFT;  // 0~1024
        } else {
            wait_ms = g_pcie_pm_test_info.lock_interval_ms;
        }
        bsp_pcie_rc_vote_lock(PCIE_USER_RFS, 0x1);
        msleep(wait_ms);
        if (rc_pm_bar_test(0x20)) {
            stop_task_run();
            bsp_pcie_rc_vote_unlock(PCIE_USER_RFS);
            return -1;
        }
        bsp_pcie_rc_vote_unlock(PCIE_USER_RFS);
        msleep(wait_ms);
        while (g_pcie_pm_test_info.stop_task_run_flag) { }
    } while (1);

    return 0;
}

int multi_task_c(void *data)
{
    u32 wait_random = 0;
    u32 wait_ms;
    do {
        if (g_pcie_pm_test_info.pcie_random_test) {
            get_random_bytes(&wait_random, sizeof(u32));
            wait_ms = wait_random >> WAIT_MS_RANDOM_SHIFT;  // 0~1024
        } else {
            wait_ms = g_pcie_pm_test_info.lock_interval_ms;
        }
        bsp_pcie_rc_vote_lock(PCIE_USER_ETH_DEV, 0x1);
        msleep(wait_ms);
        if (rc_pm_bar_test(0x30)) {
            stop_task_run();
            bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV);
            return -1;
        }
        bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV);
        msleep(wait_ms);
        while (g_pcie_pm_test_info.stop_task_run_flag) { }
    } while (1);

    return 0;
}

int multi_task_d(void *data)
{
    u32 wait_random = 0;
    u32 wait_ms;
    do {
        if (g_pcie_pm_test_info.pcie_random_test) {
            get_random_bytes(&wait_random, sizeof(u32));
            wait_ms = wait_random >> WAIT_MS_RANDOM_SHIFT;  // 0~1024
        } else {
            wait_ms = g_pcie_pm_test_info.lock_interval_ms;
        }
        bsp_pcie_rc_vote_lock(PCIE_USER_ICC, 0x1);
        msleep(wait_ms);
        if (rc_pm_bar_test(0x40)) {
            stop_task_run();
            bsp_pcie_rc_vote_unlock(PCIE_USER_ICC);
            return -1;
        }
        bsp_pcie_rc_vote_unlock(PCIE_USER_ICC);
        msleep(wait_ms);
        while (g_pcie_pm_test_info.stop_task_run_flag) { }
    } while (1);

    return 0;
}

void start_rc_pm_simulate_test(void)
{
    set_pcie_same_interval_test();
    set_test_interval(PM_TEST_WAIT_INTERVAL);

    g_pcie_pm_test_info.test_task[PM_TEST_TASK_A] = kthread_run(multi_task_a, NULL, "multi_task_a");
    g_pcie_pm_test_info.test_task[PM_TEST_TASK_B] = kthread_run(multi_task_b, NULL, "multi_task_b");
    g_pcie_pm_test_info.test_task[PM_TEST_TASK_C] = kthread_run(multi_task_c, NULL, "multi_task_c");
    g_pcie_pm_test_info.test_task[PM_TEST_TASK_D] = kthread_run(multi_task_d, NULL, "multi_task_d");
}

volatile u32 g_icc_test_enable_flag = 1;
volatile u32 g_pcdev_test_enable_flag = 1;
volatile u32 g_weth_test_enable_flag = 1;

void disable_icc_test(void)
{
    g_icc_test_enable_flag = 0;
}

void disable_pcdev_test(void)
{
    g_pcdev_test_enable_flag = 0;
}

void disable_weth_test(void)
{
    g_weth_test_enable_flag = 0;
}

void enable_icc_test(void)
{
    g_icc_test_enable_flag = 1;
}

void enable_pcdev_test(void)
{
    g_pcdev_test_enable_flag = 1;
}

void enable_weth_test(void)
{
    g_weth_test_enable_flag = 1;
}

irqreturn_t pcie_user_pm_msi_handler(int irq_num, void *data)
{
    u32 read_value;
    read_value = readl(g_pcie_pm_test_info.pm_test_virt_bar_addr + PCIE_EP_PM_TEST_MSG_OFFSET);
    if (read_value == START_RC_PM_TEST) {
        up(&g_pcie_pm_test_info.rc_start_pm_test);
    }
    return IRQ_HANDLED;
}

void rc_pm_test_send_msg(enum pm_test_msg msg_value)
{
    int read_back;
    writel(msg_value, g_pcie_pm_test_info.pm_test_virt_bar_addr + PCIE_RC_PM_TEST_MSG_OFFSET);
    read_back = readl(g_pcie_pm_test_info.pm_test_virt_bar_addr + PCIE_RC_PM_TEST_MSG_OFFSET);
    (void)bsp_pcie_rc_send_msi(PCIE_RC_MSI_PM_TEST);
    return;
}

void stop_pm_user_test(void)
{
    int ret;
    g_pcie_pm_test_info.stop_task_run_flag = 1;
    ret = bsp_pcie_rc_vote_lock(PCIE_USER_PM_TEST, 0x1);
    if (ret) {
        printk(KERN_ERR "rc vote lock test failed \n");
        return;
    }
    rc_pm_test_send_msg(STOP_PM_TEST);
    bsp_pcie_rc_vote_unlock(PCIE_USER_PM_TEST);
}

void continue_pm_user_test(void)
{
    int ret;
    g_pcie_pm_test_info.stop_task_run_flag = 0;
    ret = bsp_pcie_rc_vote_lock(PCIE_USER_PM_TEST, 0x1);
    if (ret) {
        printk(KERN_ERR "rc vote lock test failed \n");
        return;
    }
    rc_pm_test_send_msg(CONTINUE_PM_TEST);
    bsp_pcie_rc_vote_unlock(PCIE_USER_PM_TEST);
}

int rc_pm_user_test(void)
{
    int ret;
    ret = bsp_pcie_rc_vote_lock(PCIE_USER_PM_TEST, 0x1);
    if (ret) {
        printk(KERN_ERR "rc vote lock test failed \n");
        return;
    }
    rc_pm_test_send_msg(START_EP_PM_TEST);
    // user test init, for exam: st_pcdev_open_test;
    while (down_interruptible(&g_pcie_pm_test_info.rc_start_pm_test)) { }
    printk(KERN_ERR "rc pm user test start\n");
    bsp_pcie_rc_vote_unlock(PCIE_USER_PM_TEST);

    do {
        if (g_icc_test_enable_flag) {
            // require icc test instance, for exam: vcom_icc_test;
            msleep(PM_TEST_MSLEEP_MS);
        }

        if (g_weth_test_enable_flag) {
            // require weth rc test function
            msleep(PM_TEST_MSLEEP_MS);
        }

        if (g_pcdev_test_enable_flag) {
            // require pcdev test instance, for exam: st_pcdev_open_send_test;
            msleep(PM_TEST_MSLEEP_MS);
        }

        // require adc voltage instance, for exam: bsp_adc_get_voltage_debug;
        msleep(PM_TEST_MSLEEP_MS);

        while (g_pcie_pm_test_info.stop_task_run_flag) { }
    } while (1);

    return 0;
}

void start_rc_pm_user_test(void)
{
    g_pcie_pm_test_info.rc_pm_user_test_task = kthread_run(rc_pm_user_test, NULL, "start_rc_pm_user_test");
}

int rc_pm_user_test_init(u32 user_id, u32 user_action_id, void *callback_args)
{
    int ret;

    g_pcie_pm_test_info.stop_task_run_flag = 0;

    if (user_action_id == PCIE_RC_CB_ENUM_DONE) {
        sema_init(&g_pcie_pm_test_info.rc_start_pm_test, 0);
        g_pcie_pm_test_info.pm_test_virt_bar_addr = bsp_pcie_pm_get_bar_virt_addr();
        (void)bsp_pcie_rc_msi_request(PCIE_EP_MSI_PM_TEST, pcie_user_pm_msi_handler, "pcie_user_pm_msi_handler", NULL);
    }
    return 0;
}

void bsp_pcie_rc_pm_test_init_register(void)
{
    struct pcie_callback_info user_callback_info = {
        .callback = rc_pm_user_test_init,
        .callback_args = NULL,
    };

    (void)bsp_pcie_rc_cb_register(PCIE_USER_PM_TEST, &user_callback_info);
}
