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


#include <linux/kernel.h>
#include <bsp_dt.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include <bsp_adump.h>
#include <bsp_wdt.h>
#include <bsp_nvim.h>
#include <bsp_slice.h>
#include <bsp_coresight.h>
#include "dump_agent.h"
#include "dump_eproc.h"
#include "dump_wdt.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

void dump_save_cp_baseinfo_single_core(dump_base_info_s *base_info)
{
    dump_cpu_info_s *cpuinfo = NULL;

    dump_print("modem cp is single core\n");
    base_info->reboot_cpu = 0;

    cpuinfo = (dump_cpu_info_s *)bsp_dump_get_field_addr(DUMP_CP_CPUINFO_START);
    if (cpuinfo == NULL) {
        dump_error("fail to get modem_cp_cpuinfo\n");
        return;
    }

    if ((cpuinfo->current_int & 0xFFFF0000) == DUMP_FLAG_INT_ENTER) {
        cpuinfo->current_ctx = DUMP_CTX_INT;
    } else if ((cpuinfo->current_int & 0xFFFF0000) == DUMP_FLAG_INT_EXIT) {
        cpuinfo->current_ctx = DUMP_CTX_TASK;
    }
    cpuinfo->current_int &= 0xFFFF;

    if (cpuinfo->current_ctx == DUMP_CTX_TASK) {
        base_info->reboot_task = cpuinfo->current_task;
        dump_get_cp_task_name_by_id(base_info->reboot_task, (char *)(base_info->task_name),
            sizeof(base_info->task_name));
        base_info->reboot_int = (u32)(-1);
        base_info->reboot_context = DUMP_CTX_TASK;
    } else {
        base_info->reboot_task = (u32)(-1);
        base_info->reboot_int = cpuinfo->current_int;
        base_info->reboot_context = DUMP_CTX_INT;
    }
}

void dump_save_cp_baseinfo_multi_core(dump_base_info_s *base_info)
{
    dump_cp_reboot_contex_s *reboot_contex = NULL;
    dump_cpu_info_s *cpuinfo = NULL;
    u8 *addr = NULL;
    u32 i;

    base_info->reboot_cpu = 0xff;
    addr = (u8 *)bsp_dump_get_field_addr(DUMP_LRCCPU_REBOOTCONTEX);
    if (addr == NULL) {
        dump_error("fail to get cp reboot field\n");
        return;
    }

    for (i = 0; i < base_info->cpu_max_num; i++) {
        cpuinfo = (dump_cpu_info_s *)bsp_dump_get_field_addr(DUMP_CP_CPUINFO_START + i);
        if (cpuinfo == NULL) {
            dump_error("fail to get modem_cp_cpuinfo field\n");
            return;
        }
        if ((cpuinfo->current_int & 0xFFFF0000) == DUMP_FLAG_INT_ENTER) {
            cpuinfo->current_ctx = DUMP_CTX_INT;
        } else if ((cpuinfo->current_int & 0xFFFF0000) == DUMP_FLAG_INT_EXIT) {
            cpuinfo->current_ctx = DUMP_CTX_TASK;
        }
        cpuinfo->current_int &= 0xFFFF;
        reboot_contex = (dump_cp_reboot_contex_s *)((uintptr_t)(addr) + i * sizeof(dump_cp_reboot_contex_s));
        if (cpuinfo->current_ctx == DUMP_CTX_TASK) {
            reboot_contex->reboot_context = DUMP_CTX_TASK;
            reboot_contex->reboot_task = cpuinfo->current_task;
            dump_get_cp_task_name_by_id(reboot_contex->reboot_task, (char *)(reboot_contex->task_name),
                sizeof(base_info->task_name));
            reboot_contex->reboot_int = (u32)(-1);
        } else {
            reboot_contex->reboot_task = (u32)(-1);
            reboot_contex->reboot_int = cpuinfo->current_int;
            reboot_contex->reboot_context = DUMP_CTX_INT;
        }
        reboot_contex->task_name[DUMP_NAME_LEN - 1] = '\0';
    }
}

dump_base_info_s *dump_save_cp_base_info_wdt(u32 mod_id)
{
    dump_base_info_s *base_info = NULL;
    u8 *addr = NULL;

    addr = bsp_dump_get_field_addr(DUMP_LRCCPU_BASE_INFO_SMP);
    if (addr == NULL) {
        dump_error("fail to get cp base info\n");
        return NULL;
    }
    base_info = (dump_base_info_s *)addr;
    if (base_info->mod_id == DRV_ERRNO_EARLYPM_EXC) {
        dump_error("earlypm exc, baseinfo is ok\n");
        return NULL;
    }
    base_info->mod_id = mod_id;
    base_info->arg1 = DUMP_REASON_WDT;
    base_info->arg2 = 0;
    base_info->arg3 = 0;
    base_info->arg3_length = 0;
    base_info->reboot_reason = DUMP_REASON_WDT;
    base_info->reboot_time = bsp_get_slice_value();

    if (base_info->cpu_max_num == 1) {
        dump_save_cp_baseinfo_single_core(base_info);
    } else {
        dump_save_cp_baseinfo_multi_core(base_info);
    }
    dump_print("save cp base info ok\n");
    return base_info;
}

void dump_cp_wdt_hook(unsigned int mod_id)
{
    char *desc = "Modem CP WDT";
    unsigned long flags;
    dump_base_info_s *base_info = NULL;
    struct rdr_exception_info_s *exc_node = bsp_dump_get_exc_node_info();
    dump_exception_info_s *cur_exc = dump_get_cur_exc_info();

    bsp_wdt_irq_disable(WDT_CCORE_ID);
    spin_lock_irqsave(&cur_exc->lock, flags);
    if (cur_exc->exc_subsys < SUBSYS_MAX) {
        dump_error("subsys %d exc has exsited(timestamp:0x%x), exit!\n", cur_exc->exc_subsys, bsp_get_slice_value());
        spin_unlock_irqrestore(&cur_exc->lock, flags);
        return;
    }
    cur_exc->exc_subsys = SUBSYS_CP;
    spin_unlock_irqrestore(&cur_exc->lock, flags);

    base_info = dump_save_cp_base_info_wdt(mod_id);
    /* 应该在baseinfo准备好之后，立刻通知CP(因为baseinfo中保存了一些核间交互的信息，所以一定要等他完成) */
    dump_cp_async_exc_proc(mod_id);

    dump_print("[0x%x]modem cp wdt enter system error! \n", bsp_get_slice_value());
    dump_print("mod_id=0x%x arg1=0x%x arg2=0x%x len=0x%x\n", mod_id, DUMP_REASON_WDT, 0, 0);

    bsp_coresight_stop_cp();
    /* bsp_adump_save_exc_scene应该在确定cp_wdt是第一次异常之后再调用，否则容易覆盖第一次异常的信息 */

    dump_get_cp_exc_node(exc_node, mod_id);
    spin_lock_irqsave(&cur_exc->lock, flags);
    dump_fill_exc_info(mod_id, DUMP_REASON_WDT, 0, DUMP_CPU_LRCCPU, DUMP_REASON_WDT, desc);
    if (base_info != NULL) {
        dump_fill_exc_contex(base_info->reboot_context, base_info->reboot_task, base_info->reboot_int,
            base_info->task_name);
    } else {
        dump_error("modem_cp_base_info is NULL\n");
    }
    spin_unlock_irqrestore(&cur_exc->lock, flags);
    dump_exception_handler();
}

void dump_cp_wdt_init(void)
{
    int ret;
    ret = bsp_wdt_register_hook(WDT_CCORE_ID, dump_cp_wdt_hook);
    if (ret != BSP_OK) {
        dump_error("fail to register wdt hook\n");
    }
}
