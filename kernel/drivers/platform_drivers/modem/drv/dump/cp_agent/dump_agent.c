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
 *
 */
#include <product_config.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <securec.h>
#include <osl_types.h>
#include <bsp_sysctrl.h>
#include <bsp_slice.h>
#include <bsp_wdt.h>
#include <bsp_reset.h>
#include <bsp_ipc_fusion.h>
#include <bsp_dump.h>
#include <bsp_adump.h>
#include <bsp_slice.h>
#include <dump_area.h>
#include <dump_baseinfo.h>
#include <dump_modid.h>
#include "dump_log.h"
#include "dump_wdt.h"
#include "dump_eproc.h"
#include "dump_agent.h"
#include "dump_exc_table.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

dump_cp_agent_ctrl_s g_dump_cp_agent;

bool dump_is_cp_exc(void)
{
    return g_dump_cp_agent.cp_exc_flag;
}

struct work_struct *dump_get_cp_wq(void)
{
    return &g_dump_cp_agent.cp_wq;
}

void dump_get_cp_exc_node(struct rdr_exception_info_s *exc_node, u32 modid)
{
    dump_mdm_exc_table_info_s *exc_tab = NULL;
    dump_exception_info_s *cur_exc = NULL;
    u32 rdr_modid = DUMP_INVALID_MODID;
    u32 idx = DUMP_RET_INT_VAL;
    u32 size = 0;
    s32 ret;

    if (exc_node == NULL) {
        return;
    }
    exc_tab = dump_get_cp_exc_tab(&size);

    dump_get_cp_exc_node_idx(&idx, modid);
    dump_fill_cp_exc_modid(exc_node, idx, &rdr_modid, modid);
    cur_exc = dump_get_cur_exc_info();
    cur_exc->rdr_mod_id = rdr_modid;
    exc_node->e_process_priority = RDR_ERR;
    exc_node->e_reboot_priority = RDR_REBOOT_WAIT;
    exc_node->e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3;
    exc_node->e_reset_core_mask = exc_tab[idx].e_reset_core_mask;
    exc_node->e_from_core = RDR_CP;
    exc_node->e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    exc_node->e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    ret = memcpy_s(exc_node->e_from_module, sizeof(exc_node->e_from_module), "MDMCP", strlen("MDMCP"));
    ret |= memcpy_s(exc_node->e_desc, sizeof(exc_node->e_desc), exc_tab[idx].exc_info.exc_desc,
        strlen(exc_tab[idx].exc_info.exc_desc));
    if (ret != EOK) {
        dump_debug("err\n");
    }
    exc_node->e_save_log_flags = RDR_SAVE_LOGBUF;
}

void dump_cp_area_reset(dump_area_id_e areaid)
{
    int ret;
    struct dump_area_mntn_addr_info_s area_info = { 0 };
    struct dump_area_s *area;
    ret = bsp_dump_get_area_info(areaid, &area_info);
    if (ret == BSP_OK && area_info.vaddr != NULL && area_info.paddr != NULL && area_info.len != 0) {
         area = (struct dump_area_s *)area_info.vaddr;
         area->area_head.magic_num = DUMP_LEVEL1_AREA_EXPIRED_MAGICNUM;
         area->area_head.share_info.done_flag = 0;
         dump_print("area[%d] reset done\n", areaid);
    }
}

int dump_cp_reset_cb(void)
{
    if (g_dump_cp_agent.cp_exc_flag == true) {
        g_dump_cp_agent.cp_exc_flag = false;
        dump_print("cp_exc_flag clear!\n");
    }
    /* 单独复位前重置CP area，防止后续单独复位失败且CP在启动早期异常，dump未初始化时，AP获取到残留的CP信息，
     * 例如CPlog是否保存完成，一些CP_FIELD里用作交互的信息 */
    dump_cp_area_reset(DUMP_AREA_LR);
    dump_cp_area_reset(DUMP_AREA_MTEE);
    return BSP_OK;
}

void dump_cp_async_exc_proc(u32 mod_id)
{
    /* mark cp_exc_flag, so will not notify cp(proc in func dump_cp_notify_callback) later in rdr callback */
    g_dump_cp_agent.cp_exc_flag = true;
    dump_notify_cp(mod_id);
    dump_error("modem exc catch by ap!\n");
}

void dump_cp_ipc_handler(void *arg)
{
    dump_base_info_s *base_info = NULL;
    dump_reboot_reason_e reboot_reason;
    unsigned long flags;
    char *desc = NULL;
    struct rdr_exception_info_s *exc_node = bsp_dump_get_exc_node_info();
    dump_exception_info_s *cur_exc = dump_get_cur_exc_info();

    bsp_wdt_irq_disable(WDT_CCORE_ID);
    bsp_adump_stop_sche_record();
    bsp_adump_save_exc_scene(BSP_MODU_OTHER_CORE, 0, 0);

    spin_lock_irqsave(&cur_exc->lock, flags);
    if (cur_exc->exc_subsys < SUBSYS_MAX) {
        spin_unlock_irqrestore(&cur_exc->lock, flags);

        dump_error("subsys %d exc has exsited(timestamp:0x%x), exit!\n", cur_exc->exc_subsys, bsp_get_slice_value());
        return;
    }
    cur_exc->exc_subsys = SUBSYS_CP;
    spin_unlock_irqrestore(&cur_exc->lock, flags);

    g_dump_cp_agent.cp_exc_flag = true;
    dump_print("modem ccore enter system error! timestamp:0x%x\n", bsp_get_slice_value());

    base_info = (dump_base_info_s *)bsp_dump_get_field_addr(DUMP_LRCCPU_BASE_INFO_SMP);
    if (base_info != NULL) {
        reboot_reason = base_info->mod_id != DRV_ERRNO_DUMP_ARM_EXC ? DUMP_REASON_NORMAL : DUMP_REASON_ARM;
        if (reboot_reason == DUMP_REASON_NORMAL) {
            desc = "Modem CP SYSERROR";
        } else if (reboot_reason == DUMP_REASON_ARM) {
            desc = "Modem CP ABORT";
        }
        dump_get_cp_exc_node(exc_node, base_info->mod_id);
        dump_fill_exc_info(base_info->mod_id, base_info->arg1, base_info->arg2, DUMP_CPU_LRCCPU, reboot_reason, desc);
        dump_fill_exc_contex(base_info->reboot_context, base_info->reboot_task, base_info->reboot_int,
            base_info->task_name);
    } else {
        dump_error("modem_cp_base_info is NULL\n");
        dump_get_cp_exc_node(exc_node, RDR_MODEM_CP_MOD_ID);
        dump_fill_exc_info(0, 0, 0, DUMP_CPU_LRCCPU, DUMP_REASON_NORMAL, NULL);
    }
    dump_exception_handler();
    return;
}

void dump_cp_exc_register(void)
{
    struct rdr_exception_info_s rdr_exc = { 0 };
    dump_mdm_exc_table_info_s *exc_tab = NULL;
    u32 rdr_modid = DUMP_INVALID_MODID;
    bool reset_state = true;
    u32 size = 0;
    u32 i;

    rdr_exc.e_process_priority = RDR_ERR;
    rdr_exc.e_reboot_priority = RDR_REBOOT_WAIT;
    rdr_exc.e_notify_core_mask = RDR_AP | RDR_CP | RDR_HIFI | RDR_LPM3;
    rdr_exc.e_reset_core_mask = RDR_CP;
    rdr_exc.e_from_core = RDR_CP;
    rdr_exc.e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    rdr_exc.e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    rdr_exc.e_save_log_flags = RDR_SAVE_LOGBUF;
    if (strcpy_s((char *)rdr_exc.e_from_module, sizeof(rdr_exc.e_from_module), "MDMCP") != EOK) {
        dump_error("cp err\n");
    }

    exc_tab = dump_get_cp_exc_tab(&size);
    if (bsp_reset_is_feature_on() != MDM_RESET_SUPPORT) {
        reset_state = false;
    }
    for (i = 0; i < size; i++) {
        dump_fill_cp_exc_modid(&rdr_exc, i, &rdr_modid, DUMP_INVALID_MODID);
        if (reset_state == false) {
            exc_tab[i].e_reset_core_mask = RDR_AP;
            rdr_exc.e_reset_core_mask = RDR_AP;
        }
        if (EOK != memcpy_s(rdr_exc.e_desc, sizeof(rdr_exc.e_desc), exc_tab[i].exc_info.exc_desc,
            strlen(exc_tab[i].exc_info.exc_desc))) {
            dump_debug("er\n");
        }
        if (!rdr_register_exception(&rdr_exc)) {
            dump_error("cp exc register failed!modid=0x%x\n", rdr_exc.e_modid);
        }
        (void)memset_s(rdr_exc.e_desc, sizeof(rdr_exc.e_desc), 0, sizeof(rdr_exc.e_desc));
    }
}

s32 dump_cp_ipc_init(void)
{
    int ret;
    ipc_handle_t phandle = NULL;

    ret = bsp_ipc_open(IPC_ACPU_INT_SRC_TSP_DUMP, &phandle);
    if (ret != BSP_OK) {
        dump_error("fail to open ipc\n");
        return BSP_ERROR;
    }
    ret = bsp_ipc_connect(phandle, (ipc_cbk)dump_cp_ipc_handler, NULL);
    if (ret != BSP_OK) {
        dump_error("bsp_ipc_connect\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

void dump_cp_agent_init(void)
{
    s32 ret;

    INIT_WORK(&(g_dump_cp_agent.cp_wq), dump_wait_cp_done_wq);

    dump_cp_exc_register();
    dump_register_module_ops(SUBSYS_CP, dump_cp_notify_callback, dump_cp_reset_cb);

    ret = dump_cp_ipc_init();
    if (ret != BSP_OK) {
        dump_error("register cp ipc fail\n");
    }

    dump_cp_wdt_init();
    dump_cp_log_init();

    dump_print("dump cp init ok\n");
}
