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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

#include <securec.h>
#include "pfa_sysfs.h"
#include "pfa_dbg.h"

struct device g_pfa_cmd_dev;
struct device_attribute g_pfa_cmd_dev_attr;

#define CMD_INIT_NAME "pfa_cmd"

struct device_attribute g_dev_attr;

static ssize_t pfa_cmd_dev_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    struct pfa *pfa = &g_pfa;

    return sprintf_s(buf, PAGE_SIZE, "PFA status=%u\n", pfa->mask_flags);
}

static ssize_t pfa_cmd_dev_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
    // need to copy buf because we need to modify it
    int ret;
    char cmd_line[512] = {0}; /* 512, cmd line len */
    struct pfa *pfa = &g_pfa;

    if (count >= 512) { /* 512, cmd line len */
        PFA_TRACE("count is %d, too long\n", count);
        return count;
    }

    ret = memcpy_s(cmd_line, sizeof(cmd_line), buf, count);
    if (ret) {
        PFA_TRACE("memcpy_s fail\n");
        return count;
    }

    ret = filter_cmd_parser(cmd_line, sizeof(cmd_line));
    if (ret != PFA_OK) {
        return count;
    }

    clear_bit(PFA_EVENT_SYSFS_STOPED_BIT, (void *)&pfa->event);
    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)) {
        set_bit(PFA_EVENT_SYSFS_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return count;
    }
    ret = exe_pfa_cmd();
    if (ret) {
        PFA_TRACE("exe_pfa_cmd fail\n");
    }
    set_bit(PFA_EVENT_SYSFS_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);

    return count;
}

int pfa_filter_init(void)
{
    int ret;

    PFA_TRACE("enter\n");

    g_pfa_cmd_dev.init_name = CMD_INIT_NAME;

    device_initialize(&g_pfa_cmd_dev);

    ret = device_add(&g_pfa_cmd_dev);
    if (ret) {
        PFA_TRACE("200:device_add failed.   %d\n", ret);
        return -EIO;
    }

    g_pfa_cmd_dev_attr.show = pfa_cmd_dev_show;
    g_pfa_cmd_dev_attr.store = pfa_cmd_dev_store;
    g_pfa_cmd_dev_attr.attr.name = "pfa_cmd";
    g_pfa_cmd_dev_attr.attr.mode = S_IRUGO | S_IWUSR;

    ret = device_create_file(&g_pfa_cmd_dev, &g_pfa_cmd_dev_attr);
    if (ret) {
        PFA_TRACE("pfa device_create_file \n");
        return -EIO;
    }
    pfa_sys_cmd_handler_init();
    PFA_TRACE(" leave\n");

    return 0;
}

void pfa_filter_exit(void)
{
    device_remove_file(&g_pfa_cmd_dev, &g_pfa_cmd_dev_attr);
    device_del(&g_pfa_cmd_dev);

    return;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(pfa_filter_init);
module_exit(pfa_filter_exit);
#endif

MODULE_LICENSE("GPL");
