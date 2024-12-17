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
#include "odt_debug.h"
#include <securec.h>
#include "osl_sem.h"
#include "osl_thread.h"
#include "osl_types.h"
#include "bsp_slice.h"

odt_mntn_info_s g_odt_mntn_info;

void bsp_odt_mntn_enc_dst_int_info(u32 *trf_info, u32 *thrh_ovf_info)
{
    u32 chan_id = ODT_REAL_CHAN_ID(ODT_CODER_DST_OM_IND);

    *trf_info = g_odt_mntn_info.dst_tfr_int_cnt[chan_id];
    *thrh_ovf_info = g_odt_mntn_info.dst_overflow_int_cnt[chan_id];
}

/*
 * 函 数 名: bsp_get_odt_ind_dst_int_slice
 * 功能描述: 上报本次read cb调用时间戳
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: read_cb调用时间戳
 */
u32 bsp_get_odt_ind_dst_int_slice(void)
{
    u32 chan_id = ODT_REAL_CHAN_ID(ODT_CODER_DST_OM_IND);
    u32 i = g_odt_mntn_info.int_time[chan_id].ptr;
    return g_odt_mntn_info.int_time[chan_id].read_cb_start_slice[i];
}


void odt_mntn_record_read_done_cnt(u32 chan_id)
{
    g_odt_mntn_info.dst_read_done_cnt[chan_id]++;
}

void odt_mntn_record_mode_swt_fail_cnt(u32 chan_id)
{
    g_odt_mntn_info.mode_swt_fail_cnt[chan_id]++;
}

void odt_mntn_record_read_done_start_slice(u32 chan_id)
{
    u32 i = g_odt_mntn_info.read_done_time[chan_id].ptr;
    g_odt_mntn_info.read_done_time[chan_id].read_done_start[i] = bsp_get_slice_value();
}

void odt_mntn_record_read_done_end_slice(u32 chan_id)
{
    u32 i = g_odt_mntn_info.read_done_time[chan_id].ptr;
    g_odt_mntn_info.read_done_time[chan_id].read_done_end[i] = bsp_get_slice_value();
    g_odt_mntn_info.read_done_time[chan_id].ptr = (g_odt_mntn_info.read_done_time[chan_id].ptr + 1) % ODT_ENC_DST_RECORD_MAX;
}

void odt_mntn_record_read_done_fail_cnt(u32 chan_id)
{
    g_odt_mntn_info.dst_read_done_fail_cnt[chan_id]++;
}

void odt_mntn_record_read_done_succ_cnt(u32 chan_id)
{
    g_odt_mntn_info.dst_read_done_succ_cnt[chan_id]++;
}

void odt_mntn_record_header_error(u32 chan_id)
{
    g_odt_mntn_info.src_header_error_cnt[chan_id]++;
}

void odt_mntn_record_int_start_slice(u32 chan_id)
{
    u32 i = g_odt_mntn_info.int_time[chan_id].ptr;

    g_odt_mntn_info.int_time[chan_id].int_start_slice[i] = bsp_get_slice_value();
}

void odt_mntn_record_dst_tfr_int_cnt(u32 chan_id)
{
    g_odt_mntn_info.dst_tfr_int_cnt[chan_id]++;
}

void odt_mntn_record_dst_overflow_int(u32 chan_id)
{
    g_odt_mntn_info.dst_overflow_int_cnt[chan_id]++;
}

void odt_mntn_record_tfr_task_cb_cnt(u32 chan_id)
{
    g_odt_mntn_info.tfr_task_read_cb_cnt[chan_id]++;
}

void odt_mntn_record_throvf_task_cb_cnt(u32 chan_id)
{
    g_odt_mntn_info.overflow_task_read_cb_cnt[chan_id]++;
}

void odt_mntn_record_read_cb_start_slice(u32 chan_id)
{
    u32 i = g_odt_mntn_info.int_time[chan_id].ptr;
    g_odt_mntn_info.int_time[chan_id].read_cb_start_slice[i] = bsp_get_slice_value();
}

void odt_mntn_record_read_cb_end_slice(u32 chan_id)
{
    u32 i = g_odt_mntn_info.int_time[chan_id].ptr;
    g_odt_mntn_info.int_time[chan_id].read_cb_end_slice[i] = bsp_get_slice_value();
    g_odt_mntn_info.int_time[chan_id].ptr = (g_odt_mntn_info.int_time[chan_id].ptr + 1) % ODT_ENC_DST_RECORD_MAX;
}

void odt_show_mntn_info(void)
{
    u32 i;

    for (i = 0; i < ODT_MAX_ENCSRC_CHN; i++) {
        odt_crit("src_header_error_cnt[%d] = %d\n", i, g_odt_mntn_info.src_header_error_cnt[i]);
        odt_crit("***********************************\n");
        odt_crit("\n");
    }

    for (i = 0; i < ODT_MAX_ENCDST_CHN; i++) {
        odt_crit("dst_tfr_int_cnt[%d] = %d\n", i, g_odt_mntn_info.dst_tfr_int_cnt[i]);
        odt_crit("dst_overflow_int_cnt[%d] = %d\n", i, g_odt_mntn_info.dst_overflow_int_cnt[i]);
        odt_crit("tfr_task_read_cb_cnt[%d] = %d\n", i, g_odt_mntn_info.tfr_task_read_cb_cnt[i]);
        odt_crit("overflow_task_read_cb_cnt[%d] = %d\n", i, g_odt_mntn_info.overflow_task_read_cb_cnt[i]);
        odt_crit("dst_read_done_cnt[%d] = %d\n", i, g_odt_mntn_info.dst_read_done_cnt[i]);
        odt_crit("dst_read_done_fail_cnt[%d] = %d\n", i, g_odt_mntn_info.dst_read_done_fail_cnt[i]);
        odt_crit("dst_read_done_succ_cnt[%d] = %d\n", i, g_odt_mntn_info.dst_read_done_succ_cnt[i]);
        odt_crit("mode_swt_fail_cnt[%d] = %d\n", i, g_odt_mntn_info.mode_swt_fail_cnt[i]);
        odt_crit("***********************************\n");
        odt_crit("\n");
    }
}

void odt_show_mntn_time_info(void)
{
    u32 i, j;

    for (i = 0; i < ODT_MAX_ENCDST_CHN; i++) {
        for (j = 0; j < g_odt_mntn_info.int_time[i].ptr; j++) {
            odt_crit("ptr[%d] = %d\n", i, j);
            odt_crit("int_start_slice[%d] = 0x%x\n", i, g_odt_mntn_info.int_time[i].int_start_slice[j]);
            odt_crit("read_cb_start_slice[%d] = 0x%x\n", i, g_odt_mntn_info.int_time[i].read_cb_start_slice[j]);
            odt_crit("read_cb_end_slice[%d] = 0x%x\n", i, g_odt_mntn_info.int_time[i].read_cb_end_slice[j]);
            odt_crit("***********************************\n");
            odt_crit("\n");
        }
    }

    for (i = 0; i < ODT_MAX_ENCDST_CHN; i++) {
        for (j = 0; j < g_odt_mntn_info.read_done_time[i].ptr; j++) {
            odt_crit("ptr[%d] = %d\n", i, j);
            odt_crit("read_done_start[%d] = 0x%x\n", i, g_odt_mntn_info.read_done_time[i].read_done_start[j]);
            odt_crit("read_done_end[%d] = 0x%x\n", i,  g_odt_mntn_info.read_done_time[i].read_done_end[j]);
            odt_crit("***********************************\n");
            odt_crit("\n");
        }
    }

    return;
}

EXPORT_SYMBOL(g_odt_mntn_info);

