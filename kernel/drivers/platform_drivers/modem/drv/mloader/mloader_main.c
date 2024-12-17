/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <product_config.h>
#include <securec.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>

#include <bsp_nvim.h>
#include <bsp_rfile.h>
#include <bsp_version.h>
#include <bsp_sec_call.h>
#include <mdrv_sysboot.h>

#include "mloader_err.h"
#include "mloader_msg.h"
#include "mloader_main.h"
#include "mloader_debug.h"
#include "mloader_load_image.h"
#include "mloader_load_patch.h"
#include "mloader_load_modem.h"

#define MLOADER_SCHED_PRIORITY (14)

typedef struct {
    unsigned int load_type;
    struct completion start_completion;
    struct device *dev;
    struct task_struct *taskid;
    struct semaphore task_trigger_sem;
    volatile unsigned long task_flag;
} mloader_info_s;

mloader_info_s g_mloader_info;

static int mloader_task_get_flag(u32 flag)
{
    return g_mloader_info.task_flag & BIT(flag);
}

static void mloader_task_set_flag(u32 flag)
{
    set_bit(flag, &g_mloader_info.task_flag);
}

static void mloader_task_clear_flag(u32 flag)
{
    clear_bit(flag, &g_mloader_info.task_flag);
}

void mloader_trigger_load_task(u32 event)
{
    mloader_task_clear_flag(event + 1);
    mloader_task_set_flag(event);
    mloader_print_err("mloader trigger event %d\n", event);
    pm_stay_awake(g_mloader_info.dev);
    up(&g_mloader_info.task_trigger_sem);
}

static void mloader_modules_init(void)
{
    int ret;
    const bsp_version_info_s *version_info = bsp_get_version_info();

    bsp_fs_ok_notify();

    if (version_info != NULL && version_info->product_type == PRODUCT_PHONE && version_info->plat_type != PLAT_ASIC) {
        wait_for_completion(&g_mloader_info.start_completion);
        bsp_rfile_get_load_mode(g_mloader_info.load_type);
        ret = bsp_sec_call(FUNC_MDRV_LOAD_MODE_UPDATE, g_mloader_info.load_type);
        if (ret) {
            mloader_print_err("set load mode err 0x%x\n", ret);
        }
    }

    mloader_print_err("modem nv init start\n");
    ret = modem_nv_init();
    if (ret) {
        mloader_print_err("modem nv init fail, ret = %d\n", ret);
    }
}

int bsp_mloader_load_reset(void)
{
    int timeout;

    /* halt running task */
    mloader_trigger_load_task(MLOADER_TASK_HALT_REQ);

    timeout = 300U;
    while (!mloader_task_get_flag(MLOADER_TASK_HALT_DONE)) {
        msleep(10U);
        timeout--;
        if (timeout < 0) {
            mloader_print_err("wait for mloader halt timeout\n");
            return -1;
        }
    }
    mloader_print_err("mloader is in halt\n");
    return 0;
}

int bsp_load_modem_images(void)
{
    mloader_trigger_load_task(MLOADER_TASK_LOAD_MODEM_REQ);

    return 0;
}

void mloader_image_task_loop(void)
{
    while (!kthread_should_stop()) {
        while (down_interruptible(&g_mloader_info.task_trigger_sem)) { }
        if (mloader_task_get_flag(MLOADER_TASK_HALT_REQ)) {
            mloader_task_set_flag(MLOADER_TASK_HALT_DONE);
            mloader_task_clear_flag(MLOADER_TASK_HALT_REQ);
            mloader_print_err("mloader change to halt state\n");
        }

        if (mloader_task_get_flag(MLOADER_TASK_LOAD_MODEM_REQ)) {
            mloader_start_modem();
            mloader_task_clear_flag(MLOADER_TASK_LOAD_MODEM_REQ);
        }

        mloader_print_info("mloader task done\n");
        pm_relax(g_mloader_info.dev);
    }
}

int mloader_image_task(void *data)
{
    int ret;

    wait_for_completion(&g_mloader_info.start_completion);

    if (!bsp_need_loadmodem()) {
        mloader_print_err("update or charge mode, will not start modem\n");
        return 0;
    }

    mloader_modules_init();

    ret = bsp_load_modem_images();

    mloader_image_task_loop();

    return ret;
}

static ssize_t modem_start_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    ssize_t status = count;
    unsigned int load_type = 0;

    if ((kstrtouint(buf, 0, &load_type) < 0)) {
        return -EINVAL;
    }

    if (load_type >= MLOADER_BUTT) {
        mloader_print_info("fail to start modem, cmd = %d\n", load_type);
        return -EINVAL;
    }

    g_mloader_info.load_type = load_type;

    complete(&g_mloader_info.start_completion);

    mloader_print_info("modem_start write ok\n");

    return status;
}
static DEVICE_ATTR(modem_start, S_IWUSR, NULL, modem_start_store);

int mloader_tsk_init(void)
{
    int ret;
    struct sched_param sch_para = {0};
    sema_init(&g_mloader_info.task_trigger_sem, 0);

    sch_para.sched_priority = MLOADER_SCHED_PRIORITY;

    g_mloader_info.taskid = kthread_run(mloader_image_task, NULL, "mloader_task");
    if (IS_ERR_OR_NULL(g_mloader_info.taskid)) {
        mloader_print_err("fail to create kthread, ret = %ld\n", PTR_ERR(g_mloader_info.taskid));
        return -ECHILD;
    }
    ret = sched_setscheduler(g_mloader_info.taskid, SCHED_FIFO, &sch_para);
    if (ret) {
        mloader_print_err("sched_setscheduler fail\n");
    }

    return ret;
}

int mloader_probe(struct platform_device *pdev)
{
    int ret;

    g_mloader_info.dev = &pdev->dev;
    device_init_wakeup(&pdev->dev, 1);
    init_completion(&g_mloader_info.start_completion);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(BITS_PER_LONG));

    ret = mloader_tsk_init();
    if (ret) {
        return ret;
    }

    ret = mloader_msg_init();
    if (ret) {
        return ret;
    }

    ret =  mloader_modem_patch_init(pdev);
    if (ret) {
        return ret;
    }

    ret = device_create_file(&pdev->dev, &dev_attr_modem_start);
    if (ret) {
        mloader_print_err("fail to create modem start node, ret = %d\n", ret);
        return ret;
    }

    mloader_print_err("mloader init success\n");

    return ret;
}

static struct platform_device g_mloader_device = {
    .name = "modem_loader",
    .id = -1,
};

struct platform_driver g_mloader_driver = {
    .probe      = mloader_probe,
    .driver     = {
        .name     = "modem_loader",
        .owner    = THIS_MODULE,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int mloader_driver_init(void)
{
    int ret;

    ret = platform_device_register(&g_mloader_device);
    if (ret) {
        mloader_print_err("fail to register mloader device, ret = %d\n", ret);
        return ret;
    }

    ret = platform_driver_register(&g_mloader_driver);
    if (ret) {
        platform_device_unregister(&g_mloader_device);
        mloader_print_err("fail to register mloader driver, ret = %d\n", ret);
    }
    return ret;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(mloader_driver_init);
#endif
