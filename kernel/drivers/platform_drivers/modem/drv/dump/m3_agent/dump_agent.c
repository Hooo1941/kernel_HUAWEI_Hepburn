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
#include <bsp_ipc_fusion.h>
#include <mdrv_memory_layout.h>
#include <drv_comm.h>
#include <bsp_slice.h>
#include <bsp_fiq.h>
#include <bsp_reset.h>
#include <bsp_dump.h>
#include <dump_area.h>
#include <dump_modid.h>
#include "dump_eproc.h"
#include "dump_agent.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

dump_m3_agent_ctrl_s g_dump_m3_agent;

bool dump_is_m3_exc(void)
{
    return g_dump_m3_agent.m3_exc_flag;
}

struct work_struct *dump_get_m3_wq(void)
{
    return &g_dump_m3_agent.m3_wq;
}

int dump_m3_modem_reset_cb(void)
{
    if (g_dump_m3_agent.m3_exc_flag == true) {
        g_dump_m3_agent.m3_exc_flag = false;
        dump_print("m3_exc_flag clear!\n");
    }
    /* 不论phone mcore是否异常，总是会存log的；因此每次存完log后，都要把save_done标记清除，
       否则下次异常时，lpmcu即使没存完log， 也会判断成保存完了， 可能导致log保存不完整；
       当然，由于lpmcu log保存的都是最快的，所以目前并不会出现这种情况  */
    dump_clear_m3_save_done();
    return BSP_OK;
}

void dump_m3_exc_reentry_handler(void)
{
    dump_sysreboot_exception_reentry_handler(RDR_MODEM_LPM3_MOD_ID, 0, 0, DUMP_CPU_MDMM3);
}

void dump_m3_ipc_handle(void *arg)
{
    dump_base_info_s *base_ctrl = NULL;
    unsigned long flags;
    struct rdr_exception_info_s *exc_node = bsp_dump_get_exc_node_info();
    u32 slice = bsp_get_slice_value();

    dump_exception_info_s *cur_exc = dump_get_cur_exc_info();
    spin_lock_irqsave(&cur_exc->lock, flags);
    if (cur_exc->exc_subsys < SUBSYS_MAX) {
        dump_error("subsys %d exc has exsited(timestamp:0x%x), try reentry handler!\n", cur_exc->exc_subsys, slice);
        spin_unlock_irqrestore(&cur_exc->lock, flags);
        dump_m3_exc_reentry_handler();
        return;
    }
    cur_exc->exc_subsys = SUBSYS_M3;
    spin_unlock_irqrestore(&cur_exc->lock, flags);

    g_dump_m3_agent.m3_exc_flag = true;
    dump_print("modem lpmcu exc int handler enter!slice=0x%x\n", slice);

    base_ctrl = (dump_base_info_s *)bsp_dump_get_field_addr(DUMP_MDM_M3_BASE_INFO);
    if (base_ctrl != NULL) {
        dump_get_m3_exc_node(exc_node, base_ctrl->mod_id);
        cur_exc->rdr_mod_id = RDR_MODEM_LPM3_MOD_ID;
        dump_fill_exc_info(base_ctrl->mod_id, base_ctrl->arg1, base_ctrl->arg2, DUMP_CPU_MDMM3, DUMP_REASON_NORMAL,
            NULL);
        dump_fill_exc_contex(base_ctrl->reboot_context, base_ctrl->reboot_task, base_ctrl->reboot_int, NULL);
    } else {
        dump_error("mdm_m3 base ctrl is null!\n");
        dump_get_m3_exc_node(exc_node, RDR_MODEM_LPM3_MOD_ID);
        dump_fill_exc_info(0, 0, 0, DUMP_CPU_MDMM3, DUMP_REASON_NORMAL, NULL);
    }
    dump_exception_handler();
}

void dump_get_m3_exc_node(struct rdr_exception_info_s *exc_node, u32 modid)
{
    s32 ret;
    exc_node->e_modid = (unsigned int)RDR_MODEM_LPM3_MOD_ID;
    exc_node->e_modid_end = (unsigned int)RDR_MODEM_LPM3_MOD_ID;
    exc_node->e_process_priority = RDR_WARN;
    exc_node->e_reboot_priority = RDR_REBOOT_WAIT;
    exc_node->e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3;
    exc_node->e_reset_core_mask = RDR_AP;
    exc_node->e_from_core = RDR_CP;
    exc_node->e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    exc_node->e_exce_type = CP_S_EXCEPTION;
    exc_node->e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    ret = memcpy_s(exc_node->e_from_module, sizeof(exc_node->e_from_module), "MDMLPM3", strlen("MDMLPM3"));
    ret |= memcpy_s(exc_node->e_desc, sizeof(exc_node->e_desc), "modem lpm3 exc", strlen("modem lpm3 exc"));
    if (ret != EOK) {
        dump_debug("err\n");
    }
    exc_node->e_save_log_flags = RDR_SAVE_LOGBUF;
}

void dump_m3_exc_register(void)
{
    struct rdr_exception_info_s rdr_exc = { 0 };
    rdr_exc.e_modid = (unsigned int)RDR_MODEM_LPM3_MOD_ID;
    rdr_exc.e_modid_end = (unsigned int)RDR_MODEM_LPM3_MOD_ID;
    rdr_exc.e_process_priority = RDR_WARN;
    rdr_exc.e_reboot_priority = RDR_REBOOT_WAIT;
    rdr_exc.e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3;
    rdr_exc.e_reset_core_mask = RDR_AP;
    rdr_exc.e_from_core = RDR_CP;
    rdr_exc.e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    rdr_exc.e_exce_type = CP_S_EXCEPTION;
    rdr_exc.e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    rdr_exc.e_save_log_flags = RDR_SAVE_LOGBUF;
    if (strcpy_s((char *)rdr_exc.e_from_module, sizeof(rdr_exc.e_from_module), "MDMLPM3") != EOK) {
        dump_error("cp err\n");
    }
    if (strcpy_s((char *)rdr_exc.e_desc, sizeof(rdr_exc.e_desc), "modem lpm3 exc") != EOK) {
        dump_error("cp err\n");
    }

    if (!rdr_register_exception(&rdr_exc)) {
        dump_error("cp exc register failed!modid=0x%x\n", rdr_exc.e_modid);
    }
}

s32 dump_m3_ipc_init(void)
{
    s32 ret;

    ipc_handle_t phandle = NULL;

    ret = bsp_ipc_open(IPC_ACPU_INT_SRC_MCU_DUMP, &phandle);
    if (ret != BSP_OK) {
        dump_error("ipc_int_open fail!lpm3_agent fail!\n");
        return BSP_OK;
    }

    ret = bsp_ipc_connect(phandle, (ipc_cbk)dump_m3_ipc_handle, NULL);
    if (ret != BSP_OK) {
        dump_error("ipc_int_connect fail!lpm3_agent fail!\n");
        return BSP_OK;
    }

    return BSP_OK;
}

void dump_m3_agent_init(void)
{
    s32 ret;

    INIT_WORK(&(g_dump_m3_agent.m3_wq), dump_wait_m3_done_wq);

    dump_m3_exc_register();
    dump_register_module_ops(SUBSYS_M3, dump_m3_notify_callback, dump_m3_modem_reset_cb);
    bsp_dump_register_log_notifier(DUMP_SAVE_MCU_DDR, dump_save_mcu_ddr_log);

    g_dump_m3_agent.m3_exc_flag = false;
    g_dump_m3_agent.init_state = true;
    ret = dump_m3_ipc_init();
    if (ret != BSP_OK) {
        dump_error("register m3 ipc fail\n");
    }

    dump_print("dump mdm m3 init ok\n");
}
