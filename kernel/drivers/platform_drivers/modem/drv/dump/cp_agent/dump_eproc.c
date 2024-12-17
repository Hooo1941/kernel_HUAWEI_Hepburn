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
#include <securec.h>
#include <bsp_dump.h>
#include <bsp_dump_mem.h>
#include <bsp_ipc_fusion.h>
#include <bsp_fiq.h>
#include <bsp_reset.h>
#include <bsp_coresight.h>
#include <dump_area.h>
#include <dump_logs.h>
#include <dump_log_strategy.h>
#include <dump_teeos_service.h>
#include "dump_agent.h"
#include "dump_eproc.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

#define DUMP_TASKINFO_FIELD_STAGE_NONE 0x0
#define DUMP_TASKINFO_FIELD_STAGE_TASKNAME 0xAAAA0001
#define DUMP_TASKINFO_FIELD_STAGE_TASKTCB 0xAAAA0002
#define DUMP_CP_TASK_STACK_LAYERS 8
typedef struct {
    u32 pid;
    u32 core;
    u32 entry;
    u32 status;
    u32 priority;
    u32 stack_base;
    u32 stack_end;
    char name[DUMP_NAME_LEN];
    u32 dump_stack[DUMP_CP_TASK_STACK_LAYERS];
    u32 regs[0]; /* regs个数由dts中配置 */
} dump_cp_task_info_s;

typedef struct {
    u32 field_stage;
    u16 task_num;
    u16 regs_num;
    u32 regs_mask_l;
    u32 regs_mask_h;
    dump_cp_task_info_s task_tcbs[0];
} dump_cp_taskinfo_field_head_s;

#define DUMP_CP_TASK_NUM_MAX 256
#define DUMP_CP_TASK_REGS_MAX 37
#define DUMP_SINGLE_TCB_SIZE(reg_num) (sizeof(dump_cp_task_info_s) + sizeof(u32) * (reg_num))
#define DUMP_TCB_FIELD_SIZE(tcb_num, reg_num) (sizeof(dump_cp_taskinfo_field_head_s) + \
                                               (tcb_num) * DUMP_SINGLE_TCB_SIZE(reg_num))
#define DUMP_TCB_NEXT(tcb_addr, reg_num) ((dump_cp_task_info_s *)((uintptr_t)(tcb_addr) + DUMP_SINGLE_TCB_SIZE(reg_num)))

dump_cp_eproc_ctrl_s g_dump_cp_eproc;

void dump_get_cp_task_name_by_id(u32 task_id, char *task_name, u32 task_name_len)
{
    dump_cp_taskinfo_field_head_s *task_field = NULL;
    dump_cp_task_info_s *tcb = NULL;
    u32 task_cnt, reg_num, task_index;

    task_field = (dump_cp_taskinfo_field_head_s *)bsp_dump_get_field_addr(DUMP_LRCCPU_TASK_TCB_SMP_NEW);
    if (task_field == NULL) {
        dump_error("fail to get cp task tcb field\n");
        return;
    }

    task_cnt = (u32)task_field->task_num;
    reg_num = (u32)task_field->regs_num;
    if (((task_field->field_stage != DUMP_TASKINFO_FIELD_STAGE_TASKNAME) &&
        (task_field->field_stage != DUMP_TASKINFO_FIELD_STAGE_TASKTCB)) ||
        (task_cnt > DUMP_CP_TASK_NUM_MAX) || (reg_num > DUMP_CP_TASK_REGS_MAX)) {
        dump_error("cp task tcb err\n");
        return;
    }

    tcb = &task_field->task_tcbs[0];

    for (task_index = 0; task_index < task_cnt; task_index++) {
        if (tcb->pid == task_id) {
            if (memcpy_s(task_name, task_name_len, tcb->name, sizeof(tcb->name)) != EOK) {
                bsp_debug("err\n");
            }
            break;
        }
        tcb = DUMP_TCB_NEXT(tcb, reg_num);
    }
}

void dump_notify_cp(u32 mod_id)
{
    s32 ret;
    ipc_handle_t phandle = NULL;

    ret = bsp_ipc_open(IPC_TSP_INT_SRC_ACPU_DUMP, &phandle);
    if (ret == BSP_OK) {
        if (phandle != NULL) {
            bsp_ipc_send(phandle);
        }
    }
    if (ret == BSP_OK) {
        dump_print("notify mdm ccore success\n");
    } else {
        dump_error("notify modem ccore fail,please let ipc check\n");
    }
}

s32 dump_get_cp_save_done(void)
{
    s32 flag;
    struct dump_area_mntn_addr_info_s area_info = {
        NULL,
    };
    struct dump_area_s *dump_area_info = NULL;

    flag = dump_get_area_info(DUMP_AREA_LR, &area_info);
    if (flag == BSP_ERROR || area_info.vaddr == NULL) {
        dump_error("fail to find cp area head\n");
        return BSP_ERROR;
    }
    dump_area_info = ((struct dump_area_s *)(area_info.vaddr));
    flag = dump_area_info->area_head.share_info.done_flag;
    if (flag == DUMP_SAVE_SUCCESS) {
        return BSP_OK;
    }
    return BSP_ERROR;
}

s32 dump_wait_cp_save_done(u32 ms)
{
    u32 time_start;

    time_start = bsp_get_elapse_ms();

    do {
        if (BSP_OK == dump_get_cp_save_done()) {
            dump_print("mdmcp save done\n");
            return BSP_OK;
        }

        if (ms <= (bsp_get_elapse_ms() - time_start)) {
            dump_error("wait mdmcp done timeout!\n");
            return BSP_ERROR;
        }
        msleep(DUMP_SLEEP_5MS_NUM);
    } while (1);
    return BSP_ERROR;
}

void dump_cp_timeout_proc(void)
{
    s32 ret;
    ret = bsp_send_cp_fiq(FIQ_DUMP);
    if (ret == BSP_ERROR) {
        dump_error("fail to send cp fiq!\n");
        return;
    } else {
        dump_print("trig cp fiq process success\n");
    }
    ret = dump_wait_cp_save_done(DUMP_WAIT_15S_NUM);
    if (ret == BSP_ERROR) {
        dump_error("ipc fiq save cp log both fail!\n");
    } else {
        dump_print("fiq save cp log success\n");
    }
}

void dump_wait_cp_done_wq(struct work_struct *work)
{
    s32 ret;
    rdr_exc_info_s *rdr_exc_info = dump_get_rdr_exc_info();

    dump_print("begin to wait cp save done\n");
    if (dump_is_minidump()) {
        ret = dump_wait_cp_save_done(DUMP_WAIT_15S_NUM);
        if (ret == BSP_ERROR) {
            dump_cp_timeout_proc();
        }
        /* 保存modem trace&寄存器，该接口中会判断之前是否保存过，不会重复保存，这里不用再判断了 */
        (void)bsp_coresight_stop_cp();
        (void)dump_call_tee_service(DUMP_TEE_CMD_CP_DUMP);
        dump_save_log_callback(rdr_exc_info->log_path, SUBSYS_CP);
    }
    dump_print("cp_callback finish\n");
    /* 通知frame, cp已存完log */
    if (g_dump_cp_eproc.cp_done) {
        g_dump_cp_eproc.cp_done(g_dump_cp_eproc.cp_modid, SUBSYS_CP);
    }
    return;
}

s32 dump_cp_notify_callback(u32 modid, dump_done done_cb)
{
    if (dump_is_cp_exc() == true) {
        dump_print("CP exception,no need to notify C core,modid=0x%x\n", modid);
        g_dump_cp_eproc.cp_done = done_cb;
        g_dump_cp_eproc.cp_modid = modid;
        dump_queue_work(dump_get_cp_wq());
        return BSP_OK;
    }
    dump_print("enter cp callback, mod id:0x%x\n", modid);
    if (bsp_reset_ccore_is_reboot() == 0) {
        dump_notify_cp(modid);
        g_dump_cp_eproc.cp_done = done_cb;
        g_dump_cp_eproc.cp_modid = modid;
        dump_queue_work(dump_get_cp_wq());
    } else {
        dump_print("modem is reseting now,do not notify\n");
        done_cb(modid, SUBSYS_CP);
    }
    return BSP_OK;
}
