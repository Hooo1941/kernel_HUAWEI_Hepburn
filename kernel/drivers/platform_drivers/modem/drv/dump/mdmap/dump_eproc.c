/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <securec.h>
#include <osl_types.h>
#include <osl_thread.h>
#include <osl_malloc.h>
#include <mdrv_memory_layout.h>
#include <bsp_dump.h>
#include <bsp_dump_mem.h>
#include <bsp_reset.h>
#include <bsp_adump.h>
#include <dump_logs.h>
#include <dump_log_strategy.h>
#include <dump_frame.h>
#include <dump_config.h>
#include <dump_modid.h>
#include "dump_baseinfo.h"
#include "dump_notifer.h"
#include "dump_area.h"
#include "dump_ko.h"
#include "dump_lastkmsg.h"
#include "dump_eproc.h"
#include "dump_exc_table.h"
#include "dump_teeos_service.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

dump_ap_eproc_ctrl_s g_dump_ap_eproc;

void dump_set_ap_exc(void)
{
    g_dump_ap_eproc.ap_exc_flag = true;
}

bool dump_is_ap_exc(void)
{
    return g_dump_ap_eproc.ap_exc_flag;
}

/*
 * 功能描述: 打印调用栈，用于非arm异常
 */
void dump_show_stack(u32 modid, u32 reason)
{
    if (!(reason == AP_DUMP_REASON_ARM)) {
        if (modid == 0x11000025 || modid == 0x1100002A) {
            dump_error("not current task exc\n");
            show_stack(find_task_by_vpid(reason), NULL, KERN_ERR);
        } else {
            show_stack(current, NULL, KERN_ERR);
        }
    }
}

/*
 * 功能描述: 保存用户数据区
 */
void dump_save_usr_data(const char *data, u32 length)
{
    u32 len = 0;
    void *addr = NULL;
    dump_field_map_s *pfield = NULL;
    u32 usr_data_size = dump_get_u32_cfg(DUMP_CFG_USERDATA_SIZE);

    if ((data != NULL) && (length)) {
        pfield = (dump_field_map_s *)bsp_dump_get_field_map(DUMP_MODEMAP_USER_DATA);
        addr = (void *)bsp_dump_get_field_addr(DUMP_MODEMAP_USER_DATA);
        len = (length > usr_data_size) ? usr_data_size : length;

        if (addr != NULL) {
            if (EOK !=
                memcpy_s((void *)addr, usr_data_size, (const void *)(uintptr_t)data, (size_t)len)) {
                bsp_debug("err\n");
            }
        }

        if (pfield != NULL) {
            pfield->length = len;
        }
    }
    dump_print("dump save usr data finish\n");
    return;
}

u32 dump_match_special_rdr_id(u32 modid)
{
    switch (modid) {
        case DRV_ERRNO_RESET_SIM_SWITCH:
            return RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID;
        case DRV_ERRNO_RESET_RILD:
            return RDR_MODEM_CP_RESET_RILD_MOD_ID;
        case DRV_ERRNO_RESET_3RD_MODEM:
            return RDR_MODEM_CP_RESET_3RD_MOD_ID;
        case DRV_ERRNO_RESET_REBOOT_REQ:
            return RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID;
        case DRV_ERROR_USER_RESET:
            return RDR_MODEM_CP_RESET_USER_RESET_MOD_ID;
        case DRV_ERRNO_RST_FAIL:
            return RDR_MODEM_CP_RESET_FAIL_MOD_ID;
        case DRV_ERRNO_NOC_PHONE:
            return RDR_MODEM_NOC_MOD_ID;
        case DRV_ERRNO_DMSS_PHONE:
            return RDR_MODEM_DMSS_MOD_ID;
        case DRV_ERRNO_DRA_ADDR_CHECK_FAIL:
            return RDR_MODEM_CP_RESET_DRA_MOD_ID;
        default:
            return RDR_MODEM_DRV_BUTT_MOD_ID;
    }
    return RDR_MODEM_DRV_BUTT_MOD_ID;
}

u32 dump_ap_match_nolog_rdr_id(u32 rdr_modid)
{
    switch (rdr_modid) {
        case RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID:
        case RDR_MODEM_CP_RESET_USER_RESET_MOD_ID:
        case RDR_MODEM_CODE_PATCH_REVERT_MOD_ID:
            return 1;
        default:
            return 0;
    }
}

void dump_get_ap_exc_node(struct rdr_exception_info_s *exc_node, u32 modid)
{
    dump_mdm_exc_table_info_s *exc_tab = NULL;
    dump_exception_info_s *cur_exc = NULL;
    u32 idx = DUMP_RET_INT_VAL;
    u32 rdr_modid;
    u32 temp_id;
    u32 size = 0;
    s32 ret;

    if (exc_node == NULL) {
        return;
    }
    exc_tab = dump_get_ap_exc_tab(&size);
    temp_id = modid;
    rdr_modid = dump_match_special_rdr_id(modid);
    if (rdr_modid != RDR_MODEM_DRV_BUTT_MOD_ID) {
        temp_id = rdr_modid;
        dump_print("mdmap rdr_modid=0x%x match(modid=0x%x)!\n", temp_id, modid);
    }

    dump_get_ap_exc_node_idx(&idx, temp_id);
    dump_fill_ap_exc_modid(exc_node, idx, &rdr_modid, temp_id);

    cur_exc = dump_get_cur_exc_info();
    cur_exc->rdr_mod_id = rdr_modid;

    exc_node->e_process_priority = RDR_ERR;
    exc_node->e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3;
    exc_node->e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    exc_node->e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    ret = memcpy_s(exc_node->e_desc, sizeof(exc_node->e_desc), exc_tab[idx].exc_info.exc_desc,
        strlen(exc_tab[idx].exc_info.exc_desc));
    if (ret) {
        dump_debug("memcpy_s exc_node->e_desc fail, ret=%d\n", ret);
    }
    exc_node->e_save_log_flags = RDR_SAVE_LOGBUF;
}

int dump_ap_reset_cb(void)
{
    if (g_dump_ap_eproc.ap_exc_flag == true) {
        g_dump_ap_eproc.ap_exc_flag = false;
        dump_print("ap_exc_flag clear!\n");
    }
    return BSP_OK;
}

void dump_ap_exc_reentry_handler(u32 modid, u32 arg1, u32 arg2)
{
    dump_mdm_exc_table_info_s *exc_tab = NULL;
    u32 idx = DUMP_RET_INT_VAL;
    u32 temp_id = modid;
    u32 size = 0;
    u32 rdr_modid = dump_match_special_rdr_id(modid);
    
    exc_tab = dump_get_ap_exc_tab(&size);
    if (rdr_modid != RDR_MODEM_DRV_BUTT_MOD_ID) {
        temp_id = rdr_modid;
    }
    dump_get_ap_exc_node_idx(&idx, temp_id);
    if ((exc_tab[idx].e_reset_core_mask & RDR_AP) == 0) {
        dump_error("modid[0x%x] is not sysreboot exc, ignore reentry\n", modid);
        return;
    }
    dump_sysreboot_exception_reentry_handler(modid, arg1, arg2, DUMP_CPU_APP);
}

void system_error(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    unsigned long flags;
    u32 reboot_contex, slice;
    struct rdr_exception_info_s *exc_node = bsp_dump_get_exc_node_info();
    dump_exception_info_s *cur_exc = dump_get_cur_exc_info();
    slice = bsp_get_slice_value();
    spin_lock_irqsave(&cur_exc->lock, flags);
    if (cur_exc->exc_subsys < SUBSYS_MAX) {
        dump_error("subsys %d exc has exsited(timestamp:0x%x), try reentry handler!\n", cur_exc->exc_subsys, slice);
        spin_unlock_irqrestore(&cur_exc->lock, flags);
        dump_ap_exc_reentry_handler(mod_id, arg1, arg2);
        return;
    }
    cur_exc->exc_subsys = SUBSYS_AP;
    spin_unlock_irqrestore(&cur_exc->lock, flags);

    g_dump_ap_eproc.ap_exc_flag = true;
    if (in_interrupt()) {
        reboot_contex = DUMP_CTX_INT;
    } else {
        reboot_contex = DUMP_CTX_TASK;
    }

    dump_print("modem acore enter system error! timestamp:0x%x\n", slice);
    dump_print("mod_id=0x%x arg1=0x%x arg2=0x%x  len=0x%x\n", mod_id, arg1, arg2, length);

    bsp_adump_stop_sche_record();

    dump_get_ap_exc_node(exc_node, mod_id);

    /* AT+TOMODE=3和切卡执行的Modem单独复位 */
    if (mod_id == DRV_ERROR_USER_RESET || mod_id == DRV_ERRNO_RESET_SIM_SWITCH) {
        dump_error("rdr mod id no need to save log,enter reboot modem directly\n");
        dump_fill_exc_info(mod_id, arg1, arg2, DUMP_CPU_APP, DUMP_REASON_NORMAL, "Modem NOR RESET");
        dump_fill_exc_contex(reboot_contex, current->pid, 0, current->comm);
        dump_exception_handler();
        return;
    }
    dump_fill_exc_info(mod_id, arg1, arg2, DUMP_CPU_APP, DUMP_REASON_NORMAL, "Modem AP EXC");
    dump_fill_exc_contex(reboot_contex, current->pid, 0, current->comm);
    dump_exception_handler();

    dump_save_usr_data(data, length);
    bsp_adump_save_exc_scene(mod_id, 0, 0);
    dump_save_base_info(mod_id, arg1, arg2, data, length, NULL);

    if (bsp_dump_get_product_type() == DUMP_PHONE) {
        dump_show_stack(mod_id, arg1);
    }
    return;
}

void dump_reentry_exc_callback_proc(u32 modid)
{
    dump_exc_reentry_info_s *reentry_exc = NULL;
    if (modid != RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID) {
        return;
    }
    reentry_exc = dump_get_reentry_exc_info();
    if (reentry_exc->core != DUMP_CPU_APP) {
        return;
    }
    g_dump_ap_eproc.ap_exc_flag = true;
    dump_save_base_info(reentry_exc->modid, reentry_exc->arg1, reentry_exc->arg2, 0, 0, NULL);
}

void dump_dmss_noc_proc(u32 modid)
{
    struct rdr_exception_info_s *exc_node = NULL;
    dump_exception_info_s *cur_exc = NULL;
    dump_reboot_reason_e reason;
    unsigned long flags;
    char *desc = NULL;
    u32 drv_id = DRV_ERRNO_BUTT;

    if (modid != RDR_MODEM_NOC_MOD_ID && modid != RDR_MODEM_DMSS_MOD_ID) {
        return;
    }
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
    g_dump_ap_eproc.ap_exc_flag = true;
    if (modid == RDR_MODEM_NOC_MOD_ID) {
        drv_id = DRV_ERRNO_NOC_PHONE;
        desc = "MODEM NOC";
        dump_print("[0x%x] modem NOC process\n", bsp_get_slice_value());
    } else if (modid == RDR_MODEM_DMSS_MOD_ID) {
        drv_id = DRV_ERRNO_DMSS_PHONE;
        desc = "MODEM DMSS";
        dump_print("[0x%x] modem DMSS process\n", bsp_get_slice_value());
    }
    dump_save_base_info(drv_id, 0, 0, 0, 0, "noc or dmss");
    reason = ((modid == RDR_MODEM_NOC_MOD_ID) ? DUMP_REASON_NOC : DUMP_REASON_DMSS);
    dump_get_ap_exc_node(exc_node, modid);
    dump_fill_exc_info(drv_id, 0, 0, DUMP_CPU_APP, reason, desc);
    dump_fill_exc_contex(DUMP_CTX_TASK, 0, 0, NULL);
}

void dump_wait_ap_done_wq(struct work_struct *work)
{
    rdr_exc_info_s *rdr_exc_info = dump_get_rdr_exc_info();
    if (dump_is_minidump()) {
        /* teeos dump在workqueue中处理，而不是直接在notify_callback中调用
         * 注意，这可能会导致app anic时，安全os的log无法保存；这么做是基于以下原因考虑：
         * 1. ap panic主要看kernel的log，安全os的log优先级低一些；缺失该log看起来影响不大
         * 2. dump回调是串行执行的，notify_callback不应该做太多的事情，存log的工作应该放到workqueue中，而且在dump子系统中，
         *    ap的notify_callback是先执行的，在notify_callback里耗时多，会导致后面cp/lpmcu执行的晚
         * 3. 如果在安全os dump回调中二次异常，问题不易定位；而且会阻塞后面cp/lpmcu存log
         */
        dump_call_tee_service(DUMP_TEE_CMD_TEE_DUMP_CB);
        dump_save_log_callback((const char *)rdr_exc_info->log_path, SUBSYS_AP);
    }
    /* 通知frame, ap已存完log */
    if (g_dump_ap_eproc.ap_done) {
        g_dump_ap_eproc.ap_done(g_dump_ap_eproc.ap_modid, SUBSYS_AP);
    }
    return;
}

s32 dump_ap_notify_callback(u32 modid, dump_done done_cb)
{
    dump_dmss_noc_proc(modid);
    dump_reentry_exc_callback_proc(modid);

    if (dump_is_ap_exc() == false) {
        dump_save_base_info(BSP_MODU_OTHER_CORE, 0, 0, NULL, 0, NULL);
    }
    dump_print("enter ap callback, mod id:0x%x\n", modid);
    /* 在调用callback之前保存kmsg，增加kmsg中的有效信息 */
    dump_save_lastkmsg();
    bsp_dump_hook_callback();
    g_dump_ap_eproc.ap_done = done_cb;
    g_dump_ap_eproc.ap_modid = modid;
    dump_queue_work(&g_dump_ap_eproc.ap_wq);
    dump_print("ap_callback finish\n");
    return BSP_OK;
}

void dump_save_ap_dts_file(const char *dir_name)
{
    unsigned long paddr = 0;
    unsigned int size = 0;
    void *vaddr = NULL;
    paddr = mdrv_mem_region_get("acore_dts_ddr", &size);
    if (paddr == 0 || size == 0) {
        dump_error("acore_dts_ddr no exsit!\n");
        return;
    }
    vaddr = __va(paddr);
    (void)bsp_dump_log_save(dir_name, "ap_dts.bin", vaddr, (const void *)(uintptr_t)paddr, size);
}

void dump_ap_exc_register(void)
{
    struct rdr_exception_info_s rdr_exc = { 0 };
    dump_mdm_exc_table_info_s *exc_tab = NULL;
    u32 rdr_modid = DUMP_INVALID_MODID;
    bool reset_state = true;
    u32 size = 0;
    u32 i;

    rdr_exc.e_process_priority = RDR_ERR;
    rdr_exc.e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3;
    rdr_exc.e_from_core = RDR_CP;
    rdr_exc.e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    rdr_exc.e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    rdr_exc.e_save_log_flags = RDR_SAVE_LOGBUF;
    if (strcpy_s((char *)rdr_exc.e_from_module, sizeof(rdr_exc.e_from_module), "MDMAP") != EOK) {
        dump_error("cp err\n");
    }
    exc_tab = dump_get_ap_exc_tab(&size);
    if (bsp_reset_is_feature_on() != MDM_RESET_SUPPORT) {
        reset_state = false;
    }
    for (i = 0; i < size; i++) {
        dump_fill_ap_exc_modid(&rdr_exc, i, &rdr_modid, DUMP_INVALID_MODID);
        if (reset_state == false) {
            rdr_exc.e_reset_core_mask = RDR_AP;
            exc_tab[i].e_reset_core_mask = RDR_AP;
        }
        if (dump_ap_match_nolog_rdr_id(rdr_modid)) {
            rdr_exc.e_notify_core_mask = 0;
        } else {
            rdr_exc.e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3;
        }
        if (EOK != memcpy_s(rdr_exc.e_desc, sizeof(rdr_exc.e_desc), exc_tab[i].exc_info.exc_desc,
            strlen(exc_tab[i].exc_info.exc_desc))) {
            dump_debug("er\n");
        }
        if (!rdr_register_exception(&rdr_exc)) {
            dump_error("ap exc register failed!modid=0x%x\n", rdr_exc.e_modid);
        }
        (void)memset_s(rdr_exc.e_desc, sizeof(rdr_exc.e_desc), 0, sizeof(rdr_exc.e_desc));
    }
}


void dump_ap_init(void)
{
    void *addr = NULL;
    s32 ret;
    u32 usr_data_size = dump_get_u32_cfg(DUMP_CFG_USERDATA_SIZE);

    INIT_WORK(&(g_dump_ap_eproc.ap_wq), dump_wait_ap_done_wq);

    dump_ap_exc_register();
    dump_register_module_ops(SUBSYS_AP, dump_ap_notify_callback, dump_ap_reset_cb);

    (void)bsp_dump_register_log_notifier(DUMP_SAVE_AP_DTS_DDR, dump_save_ap_dts_file);

    addr = bsp_dump_register_field(DUMP_MODEMAP_USER_DATA, "USER_DATA", usr_data_size, 0);
    if (addr == NULL) {
        dump_error("fail to get mdmap user data field addr\n");
    }

    ret = dump_base_info_init();
    if (ret) {
        dump_error("mdmap base info is null\n");
    }
    dump_ko_load_info_init();
    dump_lastkmsg_init();
    dump_print("dump mdmap init ok\n");
}
EXPORT_SYMBOL(system_error);
