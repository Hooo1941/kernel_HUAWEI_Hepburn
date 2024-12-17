/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <product_config.h>

#include <mdrv_module_init.h>
#include <bsp_module_init.h>

#ifdef CONFIG_MODEM_DRIVER_MODULE
DECLARE_COMPLETION(g_modem_module_init_done_complete);
#endif
DECLARE_COMPLETION(g_modem_module_init_start_complete);

int modem_module_init_start(void *data)
{
    u32 i;
    int ret;

    wait_for_completion(&g_modem_module_init_start_complete);

    printk(KERN_ERR "drv module init start\n");
    for (i = 0; i < sizeof(g_modem_drv_module_init_func) / sizeof(g_modem_drv_module_init_func[0]); i++) {
        ret = g_modem_drv_module_init_func[i]();
        printk(KERN_ERR "module id %u return , ret = %d\n", i, ret);
    }

    printk(KERN_ERR "ps module init start\n");
    for (i = 0; i < sizeof(g_modem_ps_module_init_func) / sizeof(g_modem_ps_module_init_func[0]); i++) {
        ret = g_modem_ps_module_init_func[i]();
        printk(KERN_ERR "module id %u return , ret = %d\n", i, ret);
    }

    printk(KERN_ERR "drv late init start\n");
    for (i = 0; i < sizeof(g_modem_drv_late_init_func) / sizeof(g_modem_drv_late_init_func[0]); i++) {
        ret = g_modem_drv_late_init_func[i]();
        printk(KERN_ERR "module id %u return , ret = %d\n", i, ret);
    }

    printk(KERN_ERR "modem driver init done\n");
#ifdef CONFIG_MODEM_DRIVER_MODULE
    complete(&g_modem_module_init_done_complete);
#endif
    return ret;
}

static ssize_t modem_module_start_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int status = count;
    long modem_module_init = 0;

    if ((kstrtol(buf, 0, &modem_module_init) < 0)) {
        return -EINVAL;
    }

    complete(&g_modem_module_init_start_complete);

    printk(KERN_ERR "modem_module_start write ok\n");

    return status;
}

static DEVICE_ATTR(modem_module_start, S_IWUSR, NULL, modem_module_start_store);

int modem_module_probe(struct platform_device *pdev)
{
    int ret;

    ret = device_create_file(&pdev->dev, &dev_attr_modem_module_start);
    if (ret) {
        printk(KERN_ERR "fail to create modem_module_start node, ret = %d\n", ret);
        return ret;
    }
    printk(KERN_ERR "modem module probe out\n");

    return ret;
}

static struct platform_device g_modem_module_device = {
    .name = "sysboot",
    .id = -1,
};

struct platform_driver g_modem_module_driver = {
    .probe      = modem_module_probe,
    .driver     = {
        .name     = "sysboot",
        .owner    = THIS_MODULE,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int bsp_modem_module_init(void)
{
    int ret;
    struct task_struct *modem_init_thread = NULL;

    modem_init_thread =  kthread_run(modem_module_init_start, NULL, "modem_init");
    if (!IS_ERR(modem_init_thread)) {
        printk(KERN_ERR "succeed to start modem init thread\n");
    } else {
        ret = (int)PTR_ERR(modem_init_thread);
        printk(KERN_ERR "fail to start modem init thread, ret = %d\n", ret);
        return ret;
    }

    ret = platform_device_register(&g_modem_module_device);
    if (ret) {
        printk(KERN_ERR "fail to register modem_module device\n");
        return ret;
    }

    ret = platform_driver_register(&g_modem_module_driver);
    if (ret) {
        platform_device_unregister(&g_modem_module_device);
        printk(KERN_ERR "fail to register modem_module driver\n");
        return ret;
    }
#ifdef CONFIG_MODEM_DRIVER_MODULE
    wait_for_completion(&g_modem_module_init_done_complete);
#endif
    return ret;
}

module_init(bsp_modem_module_init);
