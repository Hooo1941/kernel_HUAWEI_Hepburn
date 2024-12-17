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
#include <linux/rtc.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <securec.h>
#include <product_config.h>
#include <mdrv_errno.h>
#include <osl_types.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <bsp_slice.h>
#include <bsp_reset.h>
#include <bsp_dump.h>
#include <bsp_wdt.h>
#include <dump_modid.h>
#include <dump_logs.h>
#include "dump_area.h"
#include "dump_frame.h"
#include "dump_errno.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

rdr_exc_info_s g_rdr_exc_info;
dump_exception_info_s g_curr_excption;
struct rdr_exception_info_s g_curr_exc_node;
dump_exc_reentry_info_s g_dump_reentry_excption;

dump_frame_ctrl_s g_dump_frame_ctrl;

void dump_fill_exc_info(u32 mod_id, u32 arg1, u32 arg2, u32 core, u32 reason, const char *desc)
{
    dump_exception_info_s *curr_exc = &g_curr_excption;
    curr_exc->core = core;
    curr_exc->mod_id = mod_id;
    curr_exc->arg1 = arg1;
    curr_exc->arg2 = arg2;
    curr_exc->reason = reason;
    curr_exc->voice = dump_get_mdm_voice_status();
    if (desc != NULL) {
        if (EOK != memcpy_s(curr_exc->exc_desc, sizeof(curr_exc->exc_desc), desc, strlen(desc))) {
            bsp_debug("err\n");
        }
    }
}

void dump_fill_exc_contex(dump_reboot_ctx_e contex, u32 task_id, u32 int_no, const char *task_name)
{
    dump_exception_info_s *curr_exc = &g_curr_excption;
    curr_exc->reboot_contex = contex;
    if (curr_exc->reboot_contex == DUMP_CTX_INT) {
        curr_exc->int_no = int_no;
    } else {
        curr_exc->task_id = task_id;
        if (task_name != NULL) {
            if (EOK != memcpy_s(curr_exc->task_name, sizeof(curr_exc->task_name), task_name, strlen(task_name))) {
                bsp_debug("err\n");
            }
        }
    }
    dump_print("fill excption info done\n");
}

struct rdr_exception_info_s *bsp_dump_get_exc_node_info(void)
{
    return &g_curr_exc_node;
}

dump_exception_info_s *dump_get_cur_exc_info(void)
{
    return &g_curr_excption;
}

dump_exc_reentry_info_s *dump_get_reentry_exc_info(void)
{
    return &g_dump_reentry_excption;
}

rdr_exc_info_s *dump_get_rdr_exc_info(void)
{
    return &g_rdr_exc_info;
}

void dump_show_exc_info(dump_exception_info_s *exc)
{
    dump_print("mod_id = 0x%x\n", exc->mod_id);
    dump_print("rdr_mod_id = 0x%x\n", exc->rdr_mod_id);
    dump_print("arg1 = 0x%x\n", exc->arg1);
    dump_print("arg2 = 0x%x\n", exc->arg2);
    dump_print("voice = 0x%x\n", exc->voice);
    dump_print("int_no = 0x%x\n", exc->int_no);
    dump_print("task_id = 0x%x\n", exc->task_id);
    dump_print("reboot_contex = 0x%x\n", exc->reboot_contex);
    dump_print("timestamp = 0x%x\n", exc->timestamp);
    dump_print("reason = 0x%x\n", exc->reason);
    if (strlen(exc->task_name) != 0) {
        dump_print("task_name = %s\n", exc->task_name);
    }
    if (strlen(exc->exc_desc) != 0) {
        dump_print("exc_desc = %s\n", exc->exc_desc);
    }
}

int dump_is_in_exception(void)
{
    return (g_curr_excption.status == DUMP_STATUS_REGISTER);
}

void dump_exc_handle_done(void)
{
    unsigned long flags;
    u32 size = (u32)((uintptr_t)(&g_curr_excption.lock) - (uintptr_t)(&g_curr_excption));

    spin_lock_irqsave(&g_curr_excption.lock, flags);
    (void)memset_s(&g_curr_excption, size, 0, size);
    (void)memset_s(&g_curr_exc_node, sizeof(struct rdr_exception_info_s), 0, sizeof(struct rdr_exception_info_s));
    g_curr_excption.status = DUMP_STATUS_NONE;
    g_curr_excption.exc_subsys = SUBSYS_MAX;
    spin_unlock_irqrestore(&g_curr_excption.lock, flags);
    dump_print("dump_exc handle_done\n");
}

int dump_sysreboot_exception_reentry_handler(u32 ori_modid, u32 arg1, u32 arg2, u32 core)
{
    u32 slice = bsp_get_slice_value();
    /* 若当前存在异常，且已经是整机复位，则不上报该并发异常
     * 可能存在并发异常时，status尚未置成REGISTER的情况，这种时候，reset_core_mask可能无效，为保证不丢失整机复位的事件，
     * 此时应视前一次异常为单独复位，此次异常重入继续上报；这样即使前一次异常是整机复位，我们只会多上报一次整机复位，不会丢失log，
     * (但此时，可能会多记录一次异常，甚至可能在启动后多存一次log，这个策略由rdr决定，这里无法保证)
     */
    if ((g_curr_excption.status == DUMP_STATUS_REGISTER) && (g_curr_exc_node.e_reset_core_mask & RDR_AP)) {
        return DUMP_ERRNO_EXISTED;
    }
    if (g_dump_reentry_excption.modid != 0) {
        return DUMP_ERRNO_EXISTED;
    }
    g_dump_reentry_excption.modid = ori_modid;
    g_dump_reentry_excption.arg1 = arg1;
    g_dump_reentry_excption.arg2 = arg2;
    g_dump_reentry_excption.core = core;
    g_dump_reentry_excption.slice = slice;
    dump_print("reentry exc req sysreboot, modid:0x%x arg1:0x%x arg2:0x%x core:0x%x\n", ori_modid, arg1, arg2, core);
    rdr_system_error(RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID, ori_modid, slice);
    return DUMP_OK;
}

s32 dump_exception_handler(void)
{
    dump_exception_info_s *cur_exc = &g_curr_excption;

    if (cur_exc->status == DUMP_STATUS_NONE) {
        cur_exc->status = DUMP_STATUS_REGISTER;
    } else {
        dump_error("now is handle exception, exit\n");
        return BSP_ERROR;
    }

    cur_exc->status = DUMP_STATUS_REGISTER;
    dump_print("enter exception handler\n");
    dump_show_exc_info(cur_exc);
    rdr_system_error(cur_exc->rdr_mod_id, cur_exc->arg1, cur_exc->arg2);
    dump_print("exit exception handler\n");

    return BSP_OK;
}

void dump_queue_work(struct work_struct *wq)
{
    if (wq == NULL) {
        return;
    }
    queue_work(g_dump_frame_ctrl.done_wq, wq);
}

void dump_done_cb(u32 modid, u32 sysid)
{
    unsigned long flags;
    u32 coreid = DUMP_CORE(sysid);
    dump_print("sys_id: %d(modid: 0x%x) dump done\n", sysid, modid);
    if (g_dump_frame_ctrl.cur_modid != modid) {
        dump_error("not this time modid,current is 0x%x,old is 0x%x\n", g_dump_frame_ctrl.cur_modid, modid);
        return;
    }
    spin_lock_irqsave(&g_dump_frame_ctrl.spinlock, flags);
    if (g_dump_frame_ctrl.notify_core & coreid) {
        g_dump_frame_ctrl.notify_core &= ~coreid;
        spin_unlock_irqrestore(&g_dump_frame_ctrl.spinlock, flags);
        return;
    }
    spin_unlock_irqrestore(&g_dump_frame_ctrl.spinlock, flags);
    dump_error("invalid sysid=0x%x dump done!\n", sysid);
    return;
}

s32 dump_wait_save_done(void)
{
    int time = g_dump_frame_ctrl.wait_max_time;
    dump_print("dump wait save done\n");
    dump_wait_done(&g_dump_frame_ctrl.notify_core, &time);
    if (time <= 0) {
        dump_error("wait for module dump done time out(%d s)\n", g_dump_frame_ctrl.wait_max_time);
        return BSP_ERROR;
    }
    dump_print("dump save done\n");
    return BSP_OK;
}

void dump_notify_module_dump_cb(u32 modid)
{
    u32 core_mask = DUMP_AP | DUMP_CP | DUMP_M3;
    unsigned long flags;
    u32 subsys;
    u32 coreid;
    u32 i;
    dump_exception_info_s *cur_exc = &g_curr_excption;

    subsys = cur_exc->exc_subsys;
    coreid = DUMP_CORE(subsys);

    spin_lock_irqsave(&g_dump_frame_ctrl.spinlock, flags);
    g_dump_frame_ctrl.notify_core |= core_mask;
    g_dump_frame_ctrl.cur_modid = modid;
    spin_unlock_irqrestore(&g_dump_frame_ctrl.spinlock, flags);

    if (core_mask & coreid) {
        if (g_dump_frame_ctrl.subsys_ops[subsys].notify_ops != NULL) {
            g_dump_frame_ctrl.subsys_ops[subsys].notify_ops(modid, dump_done_cb);
        } else {
            spin_lock_irqsave(&g_dump_frame_ctrl.spinlock, flags);
            g_dump_frame_ctrl.notify_core &= ~(coreid); /* 该子系统未注册dump回调,无需等待 */
            spin_unlock_irqrestore(&g_dump_frame_ctrl.spinlock, flags);
        }
        core_mask &= ~(coreid);
    }
    for (i = 0; i < SUBSYS_MAX; i++) {
        if (core_mask & DUMP_CORE(i)) {
            if (g_dump_frame_ctrl.subsys_ops[i].notify_ops != NULL) {
                g_dump_frame_ctrl.subsys_ops[i].notify_ops(modid, dump_done_cb);
            } else {
                spin_lock_irqsave(&g_dump_frame_ctrl.spinlock, flags);
                g_dump_frame_ctrl.notify_core &= ~(DUMP_CORE(i)); /* 该子系统未注册dump回调,无需等待 */
                spin_unlock_irqrestore(&g_dump_frame_ctrl.spinlock, flags);
            }
            core_mask &= ~(DUMP_CORE(i));
        }
    }
    if (core_mask) {
        dump_error("notify other core_mask= 0x%x is error!\n", core_mask);
    }
}

void dump_register_module_ops(dump_subsys_e subsys, notify_hook ops, dump_reset_hook dump_reset_cb)
{
    if ((ops == NULL) || (subsys >= SUBSYS_MAX)) {
        dump_error("invalid parameter,subsys=%u!\n", subsys);
    }
    g_dump_frame_ctrl.subsys_ops[subsys].subsys = subsys;
    g_dump_frame_ctrl.subsys_ops[subsys].notify_ops = ops;
    g_dump_frame_ctrl.subsys_ops[subsys].reset_cb = dump_reset_cb;
}

void dump_notify_module_reset_start(void)
{
    u32 i;
    for (i = 0; i < SUBSYS_MAX; i++) {
        if (g_dump_frame_ctrl.subsys_ops[i].reset_cb != NULL) {
            g_dump_frame_ctrl.subsys_ops[i].reset_cb();
        }
    }
    /* 当前没有COMM子系统注册进来，后续挪到comm子系统回调里 */
    dump_logs_reset_proc();
}

void dump_frame_fill_reentry_exc_info(struct rdr_exception_info_s *rdr_exc)
{
    rdr_exc->e_modid = (unsigned int)RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID;
    rdr_exc->e_modid_end = (unsigned int)RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID;
    rdr_exc->e_process_priority = RDR_ERR;
    rdr_exc->e_reboot_priority = RDR_REBOOT_NOW;
    rdr_exc->e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3;
    rdr_exc->e_reset_core_mask = RDR_AP;
    rdr_exc->e_from_core = RDR_CP;
    rdr_exc->e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW; /* 整机复位只允许上报一次，不支持异常重入 */
    rdr_exc->e_exce_type = CP_S_MODEMAP;
    rdr_exc->e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    rdr_exc->e_save_log_flags = RDR_SAVE_LOGBUF;
    if (strcpy_s((char *)rdr_exc->e_from_module, sizeof(rdr_exc->e_from_module), "MDMAP") != EOK) {
        dump_error("cp err\n");
    }
    if (strcpy_s((char *)rdr_exc->e_desc, sizeof(rdr_exc->e_from_module), "mdm req sysrst in mdmsinglerst") != EOK) {
        dump_error("cp err\n");
    }
}

void dump_frame_proc_reentry_exc_info(u32 modid)
{
    struct rdr_exception_info_s *exc_node = NULL;
    dump_exception_info_s *cur_exc = NULL;
    unsigned long flags;

    if (modid != RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID) {
        return;
    }
    dump_print("reentry exc callback, ori_modid:0x%x arg1:0x%x arg2:0x%x slice:0x%x\n", g_dump_reentry_excption.modid,
               g_dump_reentry_excption.arg1, g_dump_reentry_excption.arg2, g_dump_reentry_excption.slice);
    exc_node = bsp_dump_get_exc_node_info();
    cur_exc = dump_get_cur_exc_info();
    spin_lock_irqsave(&cur_exc->lock, flags);
    if (cur_exc->exc_subsys < SUBSYS_MAX) {
        dump_error("subsys %d exc has exsited(timestamp:0x%x), exit!\n", cur_exc->exc_subsys, bsp_get_slice_value());
        spin_unlock_irqrestore(&cur_exc->lock, flags);
        return;
    }
    cur_exc->exc_subsys = SUBSYS_AP;
    spin_unlock_irqrestore(&cur_exc->lock, flags);
    
    cur_exc->rdr_mod_id = RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID;
    dump_fill_exc_info(g_dump_reentry_excption.modid, g_dump_reentry_excption.arg1, g_dump_reentry_excption.arg2,
                       g_dump_reentry_excption.core, DUMP_REASON_NORMAL, "mdm req sysrst in mdmsinglerst");
    dump_fill_exc_contex(DUMP_CTX_TASK, 0, 0, NULL);

    dump_frame_fill_reentry_exc_info(exc_node);
}

void dump_frame_exc_register(void)
{
    struct rdr_exception_info_s rdr_exc = { 0 };
    dump_frame_fill_reentry_exc_info(&rdr_exc);
    if (!rdr_register_exception(&rdr_exc)) {
        dump_error("frame exc register failed!modid=0x%x\n", rdr_exc.e_modid);
    }
}

s32 dump_frame_init(void)
{
    spin_lock_init(&g_dump_frame_ctrl.spinlock);
    spin_lock_init(&g_curr_excption.lock);
    g_curr_excption.status = DUMP_STATUS_NONE;
    g_curr_excption.exc_subsys = SUBSYS_MAX;
    dump_frame_exc_register();
    g_dump_frame_ctrl.wait_max_time = DUMP_MSLEEP_60;
    g_dump_frame_ctrl.done_wq = create_singlethread_workqueue("dump_wq");
    if (g_dump_frame_ctrl.done_wq == NULL) {
        dump_error("create dump wq fail!\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}
