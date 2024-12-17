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
#include <osl_types.h>
#include <bsp_dump.h>
#include <bsp_slice.h>
#include <bsp_dump_mem.h>
#include <bsp_ipc_fusion.h>
#include <dump_log_strategy.h>
#include <dump_logs.h>
#include "dump_agent.h"
#include "dump_eproc.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

dump_m3_eproc_ctrl_s g_dump_m3_eproc;
void dump_notify_m3(u32 mod_id)
{
    s32 ret;
    ipc_handle_t phandle = NULL;

    ret = bsp_ipc_open(IPC_MCU_INT_SRC_ACPU_DUMP, &phandle);
    if ((ret != BSP_OK) || (phandle == NULL)) {
        dump_error("notify lpm3 fail,please let ipc check!\n");
        return;
    }

    ret = bsp_ipc_send(phandle);
    if (ret != BSP_OK) {
        dump_error("notify lpm3 fail,please let ipc check!\n");
        return;
    }
    dump_print("notify lpm3 success\n");
}

struct dump_area_s *dump_get_lpmcu_area(void)
{
    int ret;
    struct dump_area_mntn_addr_info_s area_info = {
        NULL,
    };

    ret = bsp_dump_get_area_info(DUMP_AREA_LPM3, &area_info);
    if (ret != BSP_OK) {
        dump_error("fail to find lpm3 area head\n");
        return NULL;
    }
    return (struct dump_area_s *)area_info.vaddr;
}

s32 dump_get_m3_save_done(void)
{
    int flag = 0;
    struct dump_area_s *lpmcu_area = dump_get_lpmcu_area();

    if (lpmcu_area == NULL) {
        return BSP_ERROR;
    }
    flag = lpmcu_area->area_head.share_info.done_flag;
    if (flag == DUMP_SAVE_SUCCESS) {
        return BSP_OK;
    }
    return BSP_ERROR;
}

void dump_clear_m3_save_done(void)
{
    struct dump_area_s *lpmcu_area = dump_get_lpmcu_area();

    if (lpmcu_area == NULL) {
        return;
    }
    lpmcu_area->area_head.share_info.done_flag = 0;
    return;
}

s32 dump_wait_m3_save_done(u32 ms)
{
    u32 time_start = bsp_get_elapse_ms();

    do {
        if (BSP_OK == dump_get_m3_save_done()) {
            dump_print("m3 save done\n");
            return BSP_OK;
        }

        if (ms <= (bsp_get_elapse_ms() - time_start)) {
            dump_error("wait m3 done timeout!\n");
            return BSP_ERROR;
        }
        msleep(DUMP_SLEEP_5MS_NUM);
    } while (1);
    return BSP_ERROR;
}

void dump_wait_m3_done_wq(struct work_struct *work)
{
    rdr_exc_info_s *rdr_exc_info = dump_get_rdr_exc_info();
    dump_print("begin to wait m3 save done\n");
    if (dump_is_minidump()) {
        (void)dump_wait_m3_save_done(DUMP_WAIT_3S_NUM);
    }
    dump_save_log_callback(rdr_exc_info->log_path, SUBSYS_M3);
    dump_print("m3_callback finish\n");
    /* 通知frame, cp已存完log */
    if (g_dump_m3_eproc.m3_done) {
        g_dump_m3_eproc.m3_done(g_dump_m3_eproc.m3_modid, SUBSYS_M3);
    }
    return;
}

void dump_save_mcu_ddr_log(const char *dir_name)
{
    s32 ret;
    ret = dump_save_ddr_file(dir_name, "mcu_ddr", "mcu_ddr.bin", 0);
    if (ret == BSP_OK) {
        dump_print("save mcu_ddr.bin finished\n");
    }
}

s32 dump_m3_notify_callback(u32 modid, dump_done done_cb)
{
    if (dump_is_m3_exc() == true) {
        dump_print("mdm m3 exception,no need to notify M core,modid=0x%x\n", modid);
        g_dump_m3_eproc.m3_done = done_cb;
        g_dump_m3_eproc.m3_modid = modid;
        dump_queue_work(dump_get_m3_wq());
        return BSP_OK;
    }
    dump_print("enter m3 callback, mod id:0x%x\n", modid);
    dump_notify_m3(modid);
    g_dump_m3_eproc.m3_done = done_cb;
    g_dump_m3_eproc.m3_modid = modid;
    dump_queue_work(dump_get_m3_wq());

    return BSP_OK;
}
