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

#include <bsp_dt.h>
#include <bsp_print.h>
#include "hal_common.h"


odt_hal_info_s g_odt_hal_info;

s32 odt_hal_init(void)
{
    s32 ret;

    g_odt_hal_info.dev = bsp_dt_find_compatible_node(NULL, NULL, "oam,odt_app");
    if (g_odt_hal_info.dev == NULL) {
        odt_error("odt dev find failed\n");
        return BSP_ERROR;
    }

    g_odt_hal_info.base_addr = bsp_dt_iomap(g_odt_hal_info.dev, 0);
    if (g_odt_hal_info.base_addr == NULL) {
        odt_error("odt base addr init failed\n");
        return BSP_ERROR;
    }

    ret = bsp_dt_property_read_u32_array(g_odt_hal_info.dev, "chan_group", g_odt_hal_info.rate_info.chan_group,
        (size_t)ODT_MAX_ENCSRC_CHN);
    if (ret) {
        odt_crit("no rate_ctrl group, ret=0x%x\n", ret);
    }
    ret = bsp_dt_property_read_u32_array(g_odt_hal_info.dev, "group_limits",
        g_odt_hal_info.rate_info.group_limit, (size_t)ODT_MAX_ENCSRC_CHN);
    if (ret) {
        odt_crit("no rate_ctrl group limits, ret=0x%x\n", ret);
    }

    g_odt_hal_info.odt_version = ODT_REG_READ(ODT_REG_VERSION);

    ODT_REG_WRITE(ODT_REG_CLKCTRL, 0x1f);
    return BSP_OK;
}

void odt_hal_set_rate_ctrl_threshold(u32 rate_limits)
{
    unsigned int temp;
    u32 index;
    u32 *chan_group = (u32 *)&g_odt_hal_info.rate_info.chan_group;
    u32 *group_limit = (u32 *)&g_odt_hal_info.rate_info.group_limit;

    odt_crit("set rate limits:%d\n", rate_limits);

    /* 流控阈值配置 */
    for (index = 0; index < ODT_MAX_ENCSRC_CHN; index++) {
        temp = (u64)rate_limits * (u64)0x400 * (u64)0x400 * (u64)group_limit[chan_group[index]] / (u64)0x64;
        temp &= 0xfffffff8;
        ODT_REG_WRITE(ODT_REG_ENCSRC_RATE_THR(index), temp);

        odt_crit("set odt chan rate, chan:%d group:%d, group limit:%x\n", index, chan_group[index],
            group_limit[chan_group[index]]);
    }

    return;
}

/* lint -save -e679 -e647 */
void odt_hal_unmask_header_error_int(void)
{
    ODT_REG_WRITE(ODT_REG_APP_MASK1_L, 0);
    ODT_REG_WRITE(ODT_REG_APP_MASK1_H, 0);
}

void odt_hal_get_header_error_int_state(u32 *low_chan, u32 *high_chan)
{
    *low_chan = ODT_REG_READ(ODT_REG_APP_INTSTAT1_L);
    *high_chan = ODT_REG_READ(ODT_REG_APP_INTSTAT1_H);
}

void odt_hal_clear_header_error_raw_int(u32 low_chan, u32 high_chan)
{
    ODT_REG_WRITE(ODT_REG_ENC_RAWINT1_L, low_chan);
    ODT_REG_WRITE(ODT_REG_ENC_RAWINT1_H, high_chan);
}

void odt_hal_set_header_error_int_mask(u32 chan_id, u32 value)
{
    u32 reg = chan_id < 0x20 ? ODT_REG_APP_MASK1_L : ODT_REG_APP_MASK1_H;
    ODT_REG_SETBITS(reg, (chan_id % 0x20), 1, value);
}

void odt_hal_set_rate_ctrl_cfg(u32 value1, u32 value2)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_RATE_EN_L, value1);
    ODT_REG_WRITE(ODT_REG_ENCSRC_RATE_EN_H, value2);
}

void odt_hal_set_rate_ctrl_peroid(u32 value)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_RATE_PERIOD, value);
}

void odt_hal_set_dst_chan_trans_id_en(u32 chan_id, u32 value)
{
    ODT_REG_SETBITS(ODT_REG_ENCDEST_BUFMODE_CFG(chan_id), 0x5, 1, value);
}

void odt_hal_set_rptr_img_addr(u32 chan_id, u32 addr_low, u32 addr_high)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFM_RPTRIMG_L(chan_id), addr_low);
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFM_RPTRIMG_H(chan_id), addr_high);
}

void odt_hal_set_src_rd_timeout(u32 chan_id, u32 value)
{
    if (g_odt_hal_info.odt_version >= ODT_VERSION_V303) {
        ODT_REG_SETBITS(ODT_REG_ENCSRC_RDQCFG(chan_id), 0x10, 0x10, value); /*lint !e647*/
    }
}

void odt_hal_set_rate_ctrl_group(u32 chan_id, u32 value)
{
    if (g_odt_hal_info.odt_version >= ODT_VERSION_V303) {
        ODT_REG_SETBITS(ODT_REG_ENCSRC_RATE_CTRL(chan_id), 0x18, 0x6, value);
    }
}

void odt_hal_clear_src_chan_cfg(u32 chan_id)
{
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x108); /* 该寄存器默认值为0x108 */
}

void odt_hal_clear_rate_ctrl_cnt(u32 chan_id)
{
    ODT_REG_SETBITS(ODT_REG_ENCSRC_RATE_CTRL(chan_id), 0x1F, 1, 1); /* 清除限流计数 */
    ODT_REG_SETBITS(ODT_REG_ENCSRC_RATE_CTRL(chan_id), 0x1F, 1, 0); /* 重启限流计数 */
}

void odt_hal_clear_src_rd_raw_int(u32 chan_id)
{
    u32 reg = chan_id < 0x20 ? ODT_REG_ENC_RAWINT3_L : ODT_REG_ENC_RAWINT3_H;
    ODT_REG_SETBITS(reg, chan_id % 0x20, 1, 1);
}

u32 odt_hal_get_src_state(void)
{
    return (ODT_REG_READ(ODT_REG_ENCSTAT_L) || ODT_REG_READ(ODT_REG_ENCSTAT_H));
}

u32 odt_hal_get_single_src_state(u32 chan_id)
{
    u32 reg = chan_id < 0x20 ? ODT_REG_ENCSTAT_L : ODT_REG_ENCSTAT_H;
    return ODT_REG_GETBITS(reg, chan_id % 0x20, 1);
}

s32 odt_hal_get_rate_ctrl_cnt(u32 chan_id)
{
    if (g_odt_hal_info.odt_version >= ODT_VERSION_V303) {
        return ODT_REG_GETBITS(ODT_REG_ENCSRC_RATE_CTRL(chan_id), 0, 0x18);
    } else {
        return 0;
    }
}

void odt_hal_set_single_src_rd_timeout_int_mask(u32 chan_id, u32 value)
{
    u32 reg = chan_id < 0x20 ? ODT_REG_RD_TIMEOUT_MODEM_MASK_L : ODT_REG_RD_TIMEOUT_MODEM_MASK_H;
    if (g_odt_hal_info.odt_version >= ODT_VERSION_V303) {
        ODT_REG_SETBITS(reg, chan_id % 0x20, 1, value);
    }
}

void odt_hal_clear_src_rd_timout_raw_int(u32 chan_id)
{
    u32 reg = chan_id < 0x20 ? ODT_REG_RD_TIMEOUT_RAWINT_L : ODT_REG_RD_TIMEOUT_RAWINT_H;
    ODT_REG_SETBITS(reg, chan_id % 0x20, 1, 1);
}

void odt_hal_set_single_src_rd_int_mask(u32 chan_id, u32 value)
{
    u32 reg = chan_id < 0x20 ? ODT_REG_APP_MASK3_L : ODT_REG_APP_MASK3_H;
    ODT_REG_SETBITS(reg, chan_id % 0x20, 1, value);
}


void odt_hal_src_chan_init(u32 chan_id, odt_src_chan_cfg_s *src_attr)
{
    u32 buf_len, rd_buf_len;
    u32 start_low, start_high;

    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 1, 0x2, (u32)src_attr->mode);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x3, 1, (u32)src_attr->trans_id_en);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x4, 0x2, ODT_REAL_CHAN_ID(src_attr->dest_chan_id));
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x8, 0x2, (u32)src_attr->priority);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0xA, 1, (u32)src_attr->bypass_en);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0xB, 1, (u32)src_attr->data_type_en);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0xC, 1, (u32)src_attr->rptr_img_en);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x10, 0x8, (u32)src_attr->data_type);
    ODT_REG_SETBITS(ODT_REG_ENCSRC_BUFCFG(chan_id), 0x1F, 1, (u32)src_attr->debug_en);
    buf_len = (uintptr_t)src_attr->coder_src_setbuf.input_end - (uintptr_t)src_attr->coder_src_setbuf.input_start + 1;
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFDEPTH(chan_id), buf_len);

    start_low = (u32)(uintptr_t)src_attr->coder_src_setbuf.input_start;
    start_high = (u32)(((u64)(uintptr_t)src_attr->coder_src_setbuf.input_start) >> 0x20);
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFADDR_L(chan_id), start_low);
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFADDR_H(chan_id), start_high);

    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFWPTR(chan_id), 0);
    ODT_REG_WRITE(ODT_REG_ENCSRC_BUFRPTR(chan_id), 0);

    if (src_attr->rptr_img_en) { /* 配置读指针镜像 */
        ODT_REG_WRITE(ODT_REG_ENCSRC_BUFM_RPTRIMG_L(chan_id), src_attr->read_ptr_img_phy_addr);
        ODT_REG_WRITE(ODT_REG_ENCSRC_BUFM_RPTRIMG_H(chan_id), 0);
    }

    if (src_attr->mode == ODT_ENCSRC_CHNMODE_LIST) {
        rd_buf_len = (uintptr_t)src_attr->coder_src_setbuf.rd_input_end -
            (uintptr_t)src_attr->coder_src_setbuf.rd_input_start + 1;
        ODT_REG_SETBITS(ODT_REG_ENCSRC_RDQCFG(chan_id), 0, 0x10, rd_buf_len);
        ODT_REG_SETBITS(ODT_REG_ENCSRC_RDQCFG(chan_id), 0x10, 0x10, 0);

        ODT_REG_WRITE(ODT_REG_ENCSRC_RDQADDR_L(chan_id), (uintptr_t)src_attr->coder_src_setbuf.rd_input_start);
        ODT_REG_WRITE(ODT_REG_ENCSRC_RDQADDR_H(chan_id), 0);

        ODT_REG_WRITE(ODT_REG_ENCSRC_RDQWPTR(chan_id), 0);
        ODT_REG_WRITE(ODT_REG_ENCSRC_RDQRPTR(chan_id), 0);

        if (g_odt_hal_info.odt_version >= ODT_VERSION_V303) {
            ODT_REG_SETBITS(ODT_REG_ENCSRC_RDQCFG(chan_id), 0x10, 0x10,
                src_attr->coder_src_setbuf.rd_timeout); /*lint !e647 */
        }

        ODT_REG_WRITE(ODT_REG_ENCSRC_RDQ_WPTRIMG_L(chan_id), src_attr->rd_write_ptr_img_addr_low);
        ODT_REG_WRITE(ODT_REG_ENCSRC_RDQ_WPTRIMG_H(chan_id), src_attr->rd_write_ptr_img_addr_high);
    }
}

EXPORT_SYMBOL(g_odt_hal_info);

/* lint -restore +e679 +e647 */
