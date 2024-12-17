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
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <mdrv_errno.h>
#include <osl_types.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <bsp_slice.h>
#include <bsp_dump.h>
#include <bsp_dump_mem.h>
#include <bsp_reset.h>
#include <bsp_modem_patch.h>
#include <bsp_coresight.h>
#include <dump_modid.h>
#include "dump_baseinfo.h"
#include "dump_log_strategy.h"
#include "dump_log_parser.h"
#include "dump_log_uploader.h"
#include "dump_extra_logs.h"
#include "dump_area.h"
#include "dump_eproc.h"
#include "dump_logs.h"
#include "dump_sharemem.h"
#include "dump_core.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

dump_core_ctrl_s g_dump_core_ctrl;

dump_reset_result_e dump_mdm_reset_proc(u32 modid, u32 etype, u64 coreid)
{
    s32 ret;
    dump_print("enter dump reset, mod id:0x%x\n", modid);
    ret = bsp_cp_reset();
    if (ret != 0) {
        dump_error("mdm reset not support!\n");
        return DUMP_RESET_NOT_SUPPORT;
    }

    if (!bsp_reset_is_successful(RDR_MODEM_CP_RESET_TIMEOUT)) {
        /* 类似看门狗超时, 首先保存下C核当前trace和debug寄存器信息 */
        bsp_coresight_stop_cp();
        dump_error("mdm reset fail!\n");
        return DUMP_RESET_NOT_SUCCES;
    }
    return DUMP_RESET_SUCCES;
}

s32 dump_check_reset_freq(u32 rdr_id)
{
    u64 diff = 0;
    dump_cp_reset_s *reset_info = &g_dump_core_ctrl.reset_info;

    if (reset_info->count % DUMP_CP_REST_TIME_COUNT == 0 && reset_info->count != 0) {
        diff = (reset_info->reset_time[DUMP_CP_REST_TIME_COUNT - 1] - reset_info->reset_time[0]);
        if (diff < DUMP_CP_REST_TIME_COUNT * DUMP_CP_REST_TIME_SLICE) {
            dump_error("stop modem single reset\n ");
            return BSP_ERROR;
        }

        if (EOK != memset_s(reset_info, sizeof(g_dump_core_ctrl.reset_info), 0, sizeof(g_dump_core_ctrl.reset_info))) {
            bsp_debug("err\n");
        }
    }
    if (rdr_id != RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID && rdr_id != RDR_MODEM_CP_RESET_USER_RESET_MOD_ID) {
        /* 该接口不会失败，不需要判断返回值 */
        (void)bsp_slice_getcurtime(&reset_info->reset_time[reset_info->count % DUMP_CP_REST_TIME_COUNT]);
        reset_info->count++;
    }
    return BSP_OK;
}

u32 dump_fill_rdr_exc_core(u32 core)
{
    u32 rdr_core;

    switch (core) {
        case DUMP_CPU_LRCCPU:
            rdr_core = RDR_CP;
            break;
        case DUMP_CPU_APP:
            rdr_core = RDR_MODEMAP;
            break;
        case DUMP_CPU_MDMM3:
            rdr_core = RDR_LPM3;
            break;
        default:
            rdr_core = RDR_CP;
    }
    return rdr_core;
}

void dump_save_glb_baseinfo(struct dump_global_base_info_s *global_base_info, dump_exception_info_s *excption_node,
    struct rdr_exception_info_s *rdr_exc_info)
{
    time64_t txc = 0;
    struct rtc_time tm = {
        0,
    };
    char temp[DUMP_DATETIME_LEN] = {0};
    struct dump_global_top_head_s *top_head = (struct dump_global_top_head_s *)dump_get_rdr_top_head();
    txc = ktime_get_real_seconds();
    rtc_time64_to_tm(txc, &tm);
    if (snprintf_s(temp, sizeof(temp), (sizeof(temp) - 1), "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + DUMP_LOG_YEAR_START, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) < 0) {
        bsp_debug("er\n");
    }

    (void)memset_s(global_base_info, sizeof(*global_base_info), 0, sizeof(*global_base_info));
    global_base_info->modid = rdr_exc_info->e_modid;
    global_base_info->arg1 = 0;
    global_base_info->arg2 = 0;
    if (EOK != memcpy_s(global_base_info->e_module, sizeof(global_base_info->e_module), rdr_exc_info->e_from_module,
        (unsigned long)DUMP_MAX_MODULE_LEN)) {
        bsp_debug("err\n");
    }
    global_base_info->e_type = rdr_exc_info->e_exce_type;

    global_base_info->e_core = dump_fill_rdr_exc_core(excption_node->core);
    global_base_info->start_flag = DUMP_START_EXCH;
    global_base_info->savefile_flag = DUMP_SAVE_FILE_END;
    (void)memset_s(global_base_info->e_desc, sizeof(global_base_info->e_desc), 0, sizeof(global_base_info->e_desc));
    if (memcpy_s((void *)(global_base_info->e_desc), sizeof(global_base_info->e_desc), rdr_exc_info->e_desc,
        (u32)(strlen((const char *)(rdr_exc_info->e_desc)) < DUMP_MAX_DESC_LEN ?
        strlen((const char *)(rdr_exc_info->e_desc)) : DUMP_MAX_DESC_LEN)) != EOK) {
        bsp_debug("err\n");
    }
    if (memcpy_s(global_base_info->datetime, sizeof(global_base_info->datetime), temp,
        (unsigned long)DUMP_DATETIME_LEN) != EOK) {
        bsp_debug("err\n");
    }
    global_base_info->timestamp = bsp_get_slice_value();
    if (top_head != NULL) {
        top_head->codepatch = bsp_modem_cold_patch_is_exist();
    }
}

void dump_save_rdr_baseinfo(u32 mod_id)
{
    struct dump_global_base_info_s *global_base_info = NULL;
    struct rdr_exception_info_s *rdr_exc_info = NULL;
    dump_exception_info_s *cur_exc = NULL;

    global_base_info = (struct dump_global_base_info_s *)dump_get_global_baseinfo();
    if (global_base_info == NULL) {
        dump_error("fail to get global_base_info\n");
        return;
    }

    rdr_exc_info = bsp_dump_get_exc_node_info();
    if (rdr_exc_info == NULL) {
        dump_error("fail to get exception node\n");
        return;
    }
    cur_exc = dump_get_cur_exc_info();
    if (cur_exc == NULL) {
        dump_error("fail to get current exception node\n");
        return;
    }

    dump_save_glb_baseinfo(global_base_info, cur_exc, rdr_exc_info);
    dump_print("save rdr global info ok\n");
}

const struct dump_global_base_info_s *bsp_dump_get_cur_exc_baseinfo(void)
{
    return (const struct dump_global_base_info_s *)dump_get_global_baseinfo();
}

void dump_save_rdr_cb_info(u32 modid, u32 etype, u64 coreid, char *logpath, pfn_cb_dump_done fndone)
{
    rdr_exc_info_s *rdr_info = NULL;

    if (logpath == NULL) {
        dump_error("logpath is null\n");
        return;
    }

    rdr_info = dump_get_rdr_exc_info();
    if (rdr_info == NULL) {
        dump_error("rdr_info is null\n");
        return;
    }

    rdr_info->modid = modid;
    rdr_info->coreid = coreid;
    rdr_info->dump_done = fndone;

    if ((strlen(logpath) + strlen(RDR_DUMP_FILE_CP_PATH)) >= (RDR_DUMP_FILE_PATH_LEN - 1)) {
        dump_error("log path is too long %s\n", logpath);
        return;
    }

    (void)memset_s(rdr_info->log_path, sizeof(rdr_info->log_path), '\0', sizeof(rdr_info->log_path));

    if (EOK != memcpy_s(rdr_info->log_path, sizeof(rdr_info->log_path), logpath, strlen(logpath))) {
        bsp_debug("err\n");
    }
    if (EOK != memcpy_s(rdr_info->log_path + strlen(logpath), (sizeof(rdr_info->log_path) - strlen(logpath)),
        RDR_DUMP_FILE_CP_PATH, strlen(RDR_DUMP_FILE_CP_PATH))) {
        bsp_debug("err\n");
    }

    dump_print("this exception logpath is %s\n", rdr_info->log_path);

    if (DUMP_PHONE == bsp_dump_get_product_type()) {
        dump_save_rdr_baseinfo(rdr_info->modid);
    }
}

void dump_create_save_done_file(const char *dir)
{
    char filename[MODEM_DUMP_FILE_NAME_LENGTH] = {
        0,
    };
    if (0 > snprintf_s(filename, sizeof(filename), sizeof(filename) - 1, "%s%s", dir, DUMP_SAVE_FLAG)) {
        bsp_info("snp err\n");
    }
    (void)dump_create_file(filename);
}

void dump_compress_all_logs(rdr_exc_info_s *rdr_exc_info)
{
    struct dump_file_save_info_s data_info;
    const char *dir_name = (const char *)rdr_exc_info->log_path;

    (void)memset_s(&data_info, sizeof(struct dump_file_save_info_s), 0, sizeof(struct dump_file_save_info_s));
    if (0 > strncpy_s(data_info.dstfilename, sizeof(data_info.dstfilename), "modem_dump.bin",
        strlen("modem_dump.bin"))) {
        dump_error("cpy err\n");
        return;
    }
    data_info.magic = DUMP_FILE_TRANS_MAGIC;
    data_info.links = DUMP_FILE_LINKS_MAGIC;
    if (BSP_OK == dump_trigger_compress(dir_name, sizeof(rdr_exc_info->log_path), &(data_info))) {
        dump_wait_compress_done(rdr_exc_info->log_path);
    }
}

int dump_wait_done_task(void *data)
{
    s32 ret;
    dump_core_ctrl_s *dump_ctrl = (dump_core_ctrl_s *)(uintptr_t)data;
    rdr_exc_info_s *rdr_exc_info = dump_get_rdr_exc_info();

    while (1) {
        osl_sem_down(&dump_ctrl->sem_dump_task);

        dump_print("enter save log task\n");

        dump_uploader_prepare();
        ret = dump_wait_save_done();
        if (ret == BSP_OK) {
            if (BSP_OK != dump_create_dir(rdr_exc_info->log_path)) {
                dump_error("creat dir error\n");
            } else {
                /* IOT产品可定制外部log路径保存额外日志，存log前先准备好 */
                dump_prepare_extra_logs_log_path(rdr_exc_info->log_path);
                dump_save_log_callback(rdr_exc_info->log_path, SUBSYS_COMM);
                dump_parse_all_logs(rdr_exc_info->log_path);
                dump_compress_all_logs(rdr_exc_info);
                dump_create_save_done_file(rdr_exc_info->log_path);
            }
        }
        dump_uploader_wait_done();
        if (rdr_exc_info->dump_done != BSP_NULL) {
            rdr_exc_info->dump_done(rdr_exc_info->modid, rdr_exc_info->coreid);
            dump_print("notify rdr dump handle finished\n");
            if (rdr_exc_info->modid == RDR_MODEM_NOC_MOD_ID || rdr_exc_info->modid == RDR_MODEM_DMSS_MOD_ID) {
                dump_exc_handle_done();
            }
        }
    }
    return BSP_OK;
}

void dump_mdm_callback(unsigned int modid, unsigned int etype, unsigned long long coreid, char *logpath,
    pfn_cb_dump_done fndone)
{
    dump_print("enter modem dump callback\n");
    dump_save_rdr_cb_info(modid, etype, coreid, logpath, fndone);
    dump_frame_proc_reentry_exc_info(modid);
    dump_notify_module_dump_cb(modid);
    /* 如果是panic,直接通知rdr保存完成 */
    if (modid == RDR_PHONE_MDMAP_PANIC_MOD_ID) {
        if (fndone != BSP_NULL) {
            fndone(modid, coreid);
            dump_print("mamap panic, notify rdr dump handle finished!\n");
        }
    }
    osl_sem_up(&g_dump_core_ctrl.sem_dump_task);
}

void dump_reset_fail_handle(void)
{
    struct rdr_exception_info_s *exc_node = NULL;
    dump_exception_info_s *cur_exc = NULL;
    unsigned long flags;

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
    dump_set_ap_exc();
    dump_get_ap_exc_node(exc_node, DRV_ERRNO_RST_FAIL);
    dump_fill_exc_info(DRV_ERRNO_RST_FAIL, 0, 0, DUMP_CPU_APP, DUMP_REASON_RST_FAIL, "reset fail");
    dump_fill_exc_contex(DUMP_CTX_TASK, 0, 0, "modem_reset");
    dump_save_base_info(DRV_ERRNO_RST_FAIL, 0, 0, NULL, 0, "MDM_RST_FAIL");
    (void)dump_exception_handler();
}

void dump_mdm_reset(unsigned int modid, unsigned int etype, unsigned long long coreid)
{
    s32 ret;
    dump_notify_module_reset_start();
    if (g_dump_core_ctrl.modem_off == DUMP_MODEM_OFF) {
        dump_error("modem already power off, should not do single reset.\n");
        goto done_success;
    }

    ret = dump_check_reset_freq(modid);
    if (ret == BSP_ERROR) {
        /* 当前modem单独复位过多，直接关闭modem */
        if (g_dump_core_ctrl.modem_off != DUMP_MODEM_OFF) {
            bsp_modem_power_off();
            g_dump_core_ctrl.modem_off = DUMP_MODEM_OFF;
            dump_error("modem reset too many times,shut down\n");
        }
        /* 关modem之后，也需要标记当前异常处理完毕，这样之后调用system_error触发的整机复位，才能真正走下去；
         * 至于之后调用的单独复位，会因为g_dump_core_ctrl.modem_off=DUMP_MODEM_OFF，而直接退出，不会重新打开modem
         */
        goto done_success;
    }

    ret = dump_mdm_reset_proc(modid, etype, coreid);
    if (ret != DUMP_RESET_SUCCES) {
        dump_exc_handle_done();
        dump_reset_fail_handle();
        return;
    }
done_success:
    dump_base_info_init();
    dump_exc_handle_done();
    return;
}

s32 dump_register_mdm_eproc(void)
{
    struct rdr_module_ops_pub soc_ops = {
        .ops_dump = NULL,
        .ops_reset = NULL
    };
    struct rdr_register_module_result soc_rst = { 0, 0, 0 };

    soc_ops.ops_dump = (pfn_dump)dump_mdm_callback;
    soc_ops.ops_reset = (pfn_reset)dump_mdm_reset;

    if (rdr_register_module_ops(RDR_CP, &soc_ops, &soc_rst) != BSP_OK) {
        dump_error("fail to register rdr ops\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

s32 bsp_dump_init(void)
{
    struct task_struct *pid = NULL;
    s32 ret;

    dump_sharemem_init();
    dump_file_cfg_init();
    dump_logs_list_init();

    osl_sem_init(0, &g_dump_core_ctrl.sem_dump_task);
    if (osl_task_init("dump_wait_done", DUMP_WAIT_TASK_PRI, 0x2000, (OSL_TASK_FUNC)dump_wait_done_task,
        &g_dump_core_ctrl, &pid)) {
        dump_error("creat save_modem_bootlog task error!\n");
        return BSP_ERROR;
    }
    ret = dump_register_mdm_eproc();
    if (ret == BSP_ERROR) {
        return BSP_ERROR;
    }
    ret = dump_frame_init();
    if (ret == BSP_ERROR) {
        return BSP_ERROR;
    }

    dump_ap_init();
    dump_cp_agent_init();
    dump_m3_agent_init();
    dump_logs_init();
    g_dump_core_ctrl.init_done = true;
    dump_print("bsp_dump_init ok");
    return BSP_OK;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_dump_init);
#endif
