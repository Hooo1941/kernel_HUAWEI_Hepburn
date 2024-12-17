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
#include "odt_core.h"
#include "odt_debug.h"
#include "odt_hal.h"


#undef THIS_MODU
#define THIS_MODU mod_odt

odt_mode_swt_info_s g_odt_mode_swt_info;

void odt_mode_swt_init(void)
{
    spin_lock_init(&g_odt_mode_swt_info.lock);
}

s32 odt_set_enc_dst_mode(u32 dst_chan, u32 mode)
{
    u32 mod_state, i;
    unsigned long lock_flag;
    u32 real_chan_id = ODT_REAL_CHAN_ID(dst_chan);
    u32 waittime = 10000;

    /* 关闭自动时钟门控，否则上报模式配置不生效 */
    odt_hal_set_clk_ctrl(0);

    mod_state = odt_hal_get_dst_chan_mode(real_chan_id);
    if ((mode == ODT_IND_MODE_DIRECT || mode == ODT_IND_MODE_DELAY) && mod_state) {
        spin_lock_irqsave(&g_odt_mode_swt_info.lock, lock_flag);
        odt_hal_set_dst_chan_mode(real_chan_id, 0);
        spin_unlock_irqrestore(&g_odt_mode_swt_info.lock, lock_flag);

        for (i = 0; i < waittime; i++) {
            mod_state = odt_hal_get_dst_chan_mode(real_chan_id);
            if (mod_state == 0) {
                break;
            }
        }

        if (waittime == i) {
            odt_mntn_record_mode_swt_fail_cnt(real_chan_id);
            odt_error("set encdst cycle off timeout!\n");
            odt_hal_set_clk_ctrl(1);
            return BSP_ERROR;
        }

        bsp_odt_data_send_manager(dst_chan, ODT_DEST_DSM_ENABLE);
    }else if ((mode == ODT_IND_MODE_CYCLE) && (!mod_state)) {
        bsp_odt_data_send_manager(dst_chan, ODT_DEST_DSM_DISABLE);
        spin_lock_irqsave(&g_odt_mode_swt_info.lock, lock_flag);
        odt_hal_set_dst_chan_mode(real_chan_id, 1);
        spin_unlock_irqrestore(&g_odt_mode_swt_info.lock, lock_flag);

        for (i = 0; i < waittime; i++) {
            mod_state = odt_hal_get_dst_chan_mode(real_chan_id);
            if (mod_state == 1) {
                break;
            }
        }

        if (waittime == i) {
            odt_mntn_record_mode_swt_fail_cnt(real_chan_id);
            odt_error("set encdst cycle on timeout!\n");
            odt_hal_set_clk_ctrl(1);
            return BSP_ERROR;
        }
    }

    odt_hal_set_clk_ctrl(1);
    return BSP_OK;
}

void odt_set_mode_direct(void)
{
    u32 ret;

    bsp_odt_set_timeout(ODT_TIMEOUT_TFR_LONG, ODT_TIMEOUT_TFR_LONG_MIN);
    g_odt_mode_swt_info.cur_timeout = ODT_TIMEOUT_TFR_LONG_MIN;

    bsp_odt_set_enc_dst_threshold(ODT_IND_MODE_DIRECT, ODT_CODER_DST_OM_IND);
    ret = odt_set_enc_dst_mode(ODT_CODER_DST_OM_IND, ODT_IND_MODE_DIRECT);
    if (ret) {
        odt_error("direct mode config failed!\n");
    } else {
        g_odt_mode_swt_info.odt_cur_mode = ODT_IND_MODE_DIRECT;
        odt_crit("direct mode config sucess!\n");
    }
}

void odt_set_mode_delay(void)
{
    u32 ret;

    /* if logbuffer is not configed, can't enable delay mode */
    if (odt_get_mem_log_on_flag() == ODT_DST_CHAN_DELAY) {
        bsp_odt_set_timeout(ODT_TIMEOUT_TFR_LONG, ODT_TIMEOUT_TFR_LONG_MAX);
        g_odt_mode_swt_info.cur_timeout = ODT_TIMEOUT_TFR_LONG_MAX;

        bsp_odt_set_enc_dst_threshold(ODT_IND_MODE_DELAY, ODT_CODER_DST_OM_IND);
        ret = odt_set_enc_dst_mode(ODT_CODER_DST_OM_IND, ODT_IND_MODE_DELAY);
        if (ret) {
            odt_error("delay mode config failed!\n");
        } else {
            g_odt_mode_swt_info.odt_cur_mode = ODT_IND_MODE_DELAY;
            odt_crit("delay mode config sucess!\n");
            odt_crit("delay mode, read/write pointer:%x/%x!\n", odt_hal_get_dst_read_ptr(1),
                odt_hal_get_dst_write_ptr(1));
        }
    } else {
        odt_crit("delay mode can't config:mem can't be setted!\n");
    }
}

void odt_set_mode_cycle(void)
{
    u32 ret;
    u32 log_on_flag;

    log_on_flag = odt_get_mem_log_on_flag();
    /* 如果dst方式malloc 10M内存,也支持配置循环模式，支持商用10Mlog抓取 */
    if ((log_on_flag == ODT_DST_CHAN_DELAY) || (log_on_flag == ODT_DST_CHAN_DTS)) {
        ret = odt_set_enc_dst_mode(ODT_CODER_DST_OM_IND, ODT_IND_MODE_CYCLE);
        if (ret) {
            odt_error("cycle mode config failed, ret=0x%x\n", ret);
        } else {
            g_odt_mode_swt_info.odt_cur_mode = ODT_IND_MODE_CYCLE;
            odt_crit("the ind cycle mode config sucess!\n");
        }
    } else {
        odt_crit("ind delay can't config:mem can't be setted!\n");
    }
}

/*
 * 函 数 名: bsp_odt_get_ind_chan_mode
 * 功能描述: 上报模式接口
 * 输入参数: 模式参数
 * 输出参数: 无
 */
s32 bsp_odt_get_ind_chan_mode(void)
{
    return g_odt_mode_swt_info.odt_cur_mode;
}

/*
 * 函 数 名: bsp_odt_set_ind_mode
 * 功能描述: 上报模式接口
 * 输入参数: 模式参数
 * 输出参数: 无
 * 返 回 值: BSP_S32 BSP_OK:成功 BSP_ERROR:失败
 */
s32 bsp_odt_set_ind_mode(odt_ind_mode_e mode)
{
    switch (mode) {
        case ODT_IND_MODE_DIRECT: {
            odt_set_mode_direct();
            break;
        }
        case ODT_IND_MODE_DELAY: {
            odt_set_mode_delay();
            break;
        }
        case ODT_IND_MODE_CYCLE: {
            odt_set_mode_cycle();
            break;
        }
        default: {
            odt_error("set invalid mode=0x%x\n", mode);
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

u32 odt_get_ind_dst_cur_timouet(void)
{
    return g_odt_mode_swt_info.cur_timeout;
}

EXPORT_SYMBOL(bsp_odt_set_ind_mode);

