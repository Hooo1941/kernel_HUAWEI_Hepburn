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
 */
#include "bsp_print.h"
#include <mdrv_diag.h>
#include "sysview_comm.h"
#include "sysview_hids.h"
#undef THIS_MODU
#define THIS_MODU mod_coresight
struct sysview_hids_ctrl g_syshids;
static u32 sysview_hids_send(u32 ulMsgId, void *usage, u32 len)
{
    mdrv_diag_trans_ind_s trans_msg;
    trans_msg.ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_COMM, DIAG_MSG_TYPE_BSP);
    trans_msg.ulPid = 0x8003;
    trans_msg.ulReserve = 0;
    trans_msg.ulMsgId = ulMsgId;
    trans_msg.ulLength = len;
    trans_msg.pData = usage;
    bsp_debug("trans_msg.ulMsgId 0x%x\n", trans_msg.ulMsgId);
    return mdrv_diag_trans_report(&trans_msg);
}
int sysview_hids_deal_tsp(void *buf, u32 len)
{
    diag_drv_cpuinfo_req_s *cpuinfo = (diag_drv_cpuinfo_req_s *)(uintptr_t)buf;
    if (buf == NULL || len != sizeof(diag_drv_cpuinfo_req_s) || g_syshids.init_flag != SYSVIEW_INIT_OK) {
        bsp_err("recv unexpected msg %d\n", len);
        return BSP_ERROR;
    }
    sysview_start(&cpuinfo->ap_cpu_usg, &g_syshids.ctrl[SYSVIEW_USAGE]);
    sysview_start(&cpuinfo->ap_memory, &g_syshids.ctrl[SYSVIEW_MEMORY]);
    return BSP_OK;
}
static int sysview_hids_send_task(void *data)
{
    u32 i;
    drv_cpuinfo_data_s *usage = NULL;
    usage = (drv_cpuinfo_data_s *)osl_malloc(sizeof(drv_cpuinfo_data_s));
    if (usage == NULL) {
        bsp_err("usage malloc err\n");
        return BSP_ERROR;
    }
    for_each_present_cpu(i)
    {
        sysview_cpu_usage_get(i);
    }
    while (1) {
        osl_sem_downtimeout(&g_syshids.ctrl[SYSVIEW_USAGE].load_task_sem, g_syshids.ctrl[SYSVIEW_USAGE].jiffies);
        if (g_syshids.ctrl[SYSVIEW_USAGE].mode == 0) {
            continue;
        }
        (void)memset_s(usage, sizeof(drv_cpuinfo_data_s), 0, sizeof(drv_cpuinfo_data_s));
        for_each_present_cpu(i)
        {
            if (i >= SYSVIEW_MAX_CORE) {
                break;
            }
            if ((1U << i) & g_syshids.ctrl[SYSVIEW_USAGE].core_mask) {
                usage->usage[i] = sysview_cpu_usage_get(i);
            }
            usage->core_num++;
        }
        sysview_hids_send(SYSVIEW_MSGID_CPU, (void *)(uintptr_t)usage, sizeof(drv_cpuinfo_data_s));
    }
    osl_free(usage);
    return BSP_OK;
}
static int memload_hids_send_task(void *data)
{
    sysview_meminfo_s mem_info;
    while (1) {
        osl_sem_downtimeout(&g_syshids.ctrl[SYSVIEW_MEMORY].load_task_sem, g_syshids.ctrl[SYSVIEW_MEMORY].jiffies);
        if (g_syshids.ctrl[SYSVIEW_MEMORY].mode == 0) {
            continue;
        }
        (void)memset_s(&mem_info, sizeof(sysview_meminfo_s), 0, sizeof(sysview_meminfo_s));
        sysview_memory_get(&mem_info);
        sysview_hids_send(SYSVIEW_MSGID_MEM, (void *)(uintptr_t)&mem_info, sizeof(sysview_meminfo_s));
    }
    return BSP_OK;
}
static void sysview_diag_notify_func(mdrv_diag_state_e state)
{
    if (state == DIAG_STATE_DISCONN) {
        sysview_stop(&g_syshids.ctrl[SYSVIEW_USAGE]);
        sysview_stop(&g_syshids.ctrl[SYSVIEW_MEMORY]);
    }
}
void sysview_hids_init(void)
{
    if (sysview_task_init(&g_syshids.ctrl[SYSVIEW_USAGE], "sysview_hids", sysview_hids_send_task)) {
        return;
    }
    if (sysview_task_init(&g_syshids.ctrl[SYSVIEW_MEMORY], "memload_hids", memload_hids_send_task)) {
        return;
    }
    mdrv_diag_conn_state_notify_fun_reg(sysview_diag_notify_func);
    g_syshids.init_flag = SYSVIEW_INIT_OK;
    return;
}