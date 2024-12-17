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

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <bsp_mci.h>
#include <securec.h>
#include <osl_types.h>
#include <osl_module.h>
#include "mdmpm_debug.h"

#undef THIS_MODU
#define THIS_MODU mod_pm

static mdmpm_debug_ipc g_pm_wakeup_ipc[CCORE_MAX][PM_WAKEUP_TYPE_MAX] = {
    { { IPC_TSP_INT_SRC_AMCPU_DSS0_LOCKUP, NULL }, { IPC_TSP_INT_SRC_AMCPU_DSS0_WAKAEUP, NULL }, },
    { { IPC_TSP_INT_SRC_AMCPU_DSS1_LOCKUP, NULL }, { IPC_TSP_INT_SRC_AMCPU_DSS1_WAKAEUP, NULL }, },
    { { IPC_TSP_INT_SRC_AMCPU_DSS2_LOCKUP, NULL }, { IPC_TSP_INT_SRC_AMCPU_DSS2_WAKAEUP, NULL }, },
    { { IPC_TSP_INT_SRC_AMCPU_DSS3_LOCKUP, NULL }, { IPC_TSP_INT_SRC_AMCPU_DSS3_WAKAEUP, NULL }, },
};

static void pm_wakeup_ipc_send(ccore_coreid_e core_id, wakeup_type_e flag)
{
    s32 ret;
    ipc_handle_t phandle = g_pm_wakeup_ipc[core_id][flag].ipc_handle;
    if (phandle == NULL) {
        pm_err("ipc(0x%x) handle null!\n", g_pm_wakeup_ipc[core_id][flag].ipc_id);
        return;
    }
    ret = bsp_ipc_send(phandle);
    if (ret != BSP_OK) {
        pm_err("fail to send ipc(0x%x), ret %u!\n", g_pm_wakeup_ipc[core_id][flag].ipc_id, ret);
    }
}

/* wakeup for debug */
/*
 * flag -0 : send ipc to ccore, ccore lock
 * 1 : send ipc to ccore, ccore unlock
 */
void pm_wakeup_ccore(u32 dss_id, wakeup_type_e flag)
{
    if (dss_id >= PM_WU_MAX_DSS_NUM || flag >= PM_WAKEUP_TYPE_MAX) {
        pm_err("out of range, dss id %u !\n", dss_id);
        return;
    }
    pm_wakeup_ipc_send(DSS0 + dss_id, flag);
}

static void pm_wakeup_ipc_open(void)
{
    u32 core_id;
    wakeup_type_e flag;
    s32 ret;
    ipc_handle_t phandle = NULL;
    for (core_id = DSS0; core_id < CCORE_MAX; core_id++) {
        for (flag = PM_WAKEUP_THEN_LOCK; flag < PM_WAKEUP_TYPE_MAX; flag++) {
            ret = bsp_ipc_open(g_pm_wakeup_ipc[core_id][flag].ipc_id, &phandle);
            if (ret != BSP_OK) {
                pm_err("ipc(0x%x) ret(%d) failed!\n", g_pm_wakeup_ipc[core_id][flag].ipc_id, ret);
                continue;
            }
            g_pm_wakeup_ipc[core_id][flag].ipc_handle = phandle;
        }
    }
}

core_wakeup_ctrl g_ccore_wakeup_ctrl = { 0 };

static ssize_t wakeup_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t len;
    unsigned char kbuf[MAX_U64_STR_SIZE] = {0};
    core_wakeup_ctrl *wakeup_ctrl = &g_ccore_wakeup_ctrl;
    if (filp == NULL || buf == NULL) {
        return -EFAULT;
    }
    if (ppos == NULL) {
        return -EFAULT;
    }
    if (*ppos > 0) {
        return 0;
    }
    len = sprintf_s(kbuf, MAX_U64_STR_SIZE, "%llu", (unsigned long long)wakeup_ctrl->cmd);
    if (len == -1) {
        pm_err("sprintf_s failed\n");
        return 0;
    }
    if (count < len) {
        pr_err("buffer overflow: %lu > %lu  \n", count, len);
        return 0;
    }
    if (copy_to_user(buf, kbuf, len)) {
        pm_err("fail copy to user \n");
        return 0;
    }
    *ppos = len;
    return len;
}

static ssize_t wakeup_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned long val;
    unsigned int core_id, wakeup_type;
    unsigned char kbuf[MAX_U64_STR_SIZE] = {0};
    core_wakeup_ctrl *wakeup_ctrl = &g_ccore_wakeup_ctrl;
    if (filp == NULL || buf == NULL) {
        return -EFAULT;
    }
    if (ppos == NULL) {
        return -EFAULT;
    }
    if (*ppos) {
        return 0;
    }
    if (count >= MAX_U64_STR_SIZE) {
        pr_err("buffer overflow: %lu > %u\n", count, MAX_U64_STR_SIZE);
        return -EINVAL;
    }
    ret = copy_from_user(kbuf, buf, count);
    if (ret) {
        pr_err("fail copy from user\n");
        return -EIO;
    }
    if (kstrtoul(kbuf, 0, &val)) {
        return -EINVAL;
    }
    core_id = (unsigned int)(val & CORE_MASK) >> CORE_SHIFT;
    wakeup_type = (unsigned int)(val & WAKEUP_TYPE_MASK);
    pm_err("wakeup_ccore core_id %u, wakeup_type %u !\n", core_id, wakeup_type);
    if (core_id >= CCORE_MAX) {
        return -EINVAL;
    }
    pm_wakeup_ccore((u32)core_id, (wakeup_type_e)wakeup_type);
    wakeup_ctrl->wakeup[core_id].ipc_cnt++;
    wakeup_ctrl->wakeup[core_id].cur_wakeup_type = wakeup_type;
    wakeup_ctrl->cmd = val;
    *ppos += count;
    return count;
}

struct proc_ops g_pm_wakeup_ccore_ops = {
    .proc_read = wakeup_read,
    .proc_write = wakeup_write,
};

int mdmpm_debug_init(void)
{
    int ret;
    ret = bsp_mci_proc_create("pm_ccore_wakeup", PM_WAKEUP_MODE, &g_pm_wakeup_ccore_ops);
    if (ret) {
        pm_err("mci proc create err!\n");
    }
    pm_wakeup_ipc_open();
    pm_err("pm_ccore_agent init ok!\n");
    return BSP_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(mdmpm_debug_init);
#endif
