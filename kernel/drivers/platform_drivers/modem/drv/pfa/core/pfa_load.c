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

#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <product_config.h>
#include <bsp_version.h>
#include <securec.h>
#include "pfa_img.h"
#include "pfa_core.h"
#include "pfa_dbg.h"
#include "pfa.h"

#undef THIS_MODU
#define THIS_MODU mod_pfa

#define PFA_FIRMWARE_NAME "pfa.bin"

#define PFA_CMD_INIT_NAME "pfa"

unsigned int g_pfa_load_flag = 0;

static DEFINE_MUTEX(pfa_fw_mutex);

int pfa_load_fw(char *data, unsigned int size)
{
    int ret;

    ret = pfa_start_core(data, size);
    if (ret) {
        PFA_ERR("core start err, ret:%d\n", ret);
        return ret;
    }

    return 0;
}

static void pfa_trigger_async_request_cb(const struct firmware *fw, void *context)
{
    if (fw == NULL) {
        PFA_ERR("pfa failed to async load firmware: '%s' \n", (char *)context);
        mutex_unlock(&pfa_fw_mutex);
        return;
    }

    if (pfa_load_fw((char *)fw->data, (unsigned int)fw->size)) {
        mutex_unlock(&pfa_fw_mutex);
        return;
    }
    pfa_hardware_init(&g_pfa);

    PFA_ERR("'%s' loaded size: %u\n", (char *)context, (unsigned int)fw->size);

    // 释放固件资源
    release_firmware(fw);
    mutex_unlock(&pfa_fw_mutex);

    return;
}

int pfa_load_firmware(void)
{
    int ret;
    char *name = "pfa.bin";

    mutex_lock(&pfa_fw_mutex);

    // 加载固件
    ret = request_firmware_nowait(THIS_MODULE, 1, PFA_FIRMWARE_NAME, g_pfa.dev, GFP_KERNEL, name, pfa_trigger_async_request_cb);
    if (ret) {
        PFA_ERR("async load of '%s' failed: %d\n", name, ret);
        return ret;
    }

    return 0;
}


static ssize_t pfa_load_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    return g_pfa_load_flag;
}

static ssize_t pfa_load_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;

    if ((kstrtouint(buf, 0, &g_pfa_load_flag) < 0)) {
        PFA_ERR("pfa_load_store read load flag err. \n");
        return count;
    }

    if (g_pfa_load_flag <= 0) {
        PFA_ERR("pfa_load_store load flag = %u. \n", g_pfa_load_flag);
        return count;
    } else {
        ret = pfa_load_firmware();
        if (ret) {
            PFA_ERR("pfa_load_firmware fail.\n");
            return count;
        }
    }

    return count;
}

static DEVICE_ATTR(pfa_load, S_IWUSR, pfa_load_show, pfa_load_store);

int pfa_load_init(struct pfa *pfa)
{
    int ret;

    ret = device_create_file(pfa->dev, &dev_attr_pfa_load);
    if (ret) {
        PFA_ERR("pfa device_create_file\n");
        return -1;
    }

    return 0;
}

void pfa_load_exit(struct pfa *pfa)
{
    device_remove_file(pfa->dev, &dev_attr_pfa_load);

    return;
}


