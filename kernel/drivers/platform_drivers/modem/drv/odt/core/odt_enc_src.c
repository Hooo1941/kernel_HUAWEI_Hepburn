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

#include <securec.h>
#include <bsp_odt.h>
#include "odt_core.h"
#include "odt_debug.h"


/*lint --e{124}*/

odt_enc_src_info_s g_odt_enc_src_info;

#define ODT_MAX_WAIT_TIME 0x64
s32 odt_wait_single_chan_idle(u32 chan_id)
{
    int stat;
    int wait_time = 0;

    stat = odt_hal_get_single_src_state(chan_id);
    while (stat && (wait_time < ODT_MAX_WAIT_TIME)) {
        udelay(1);
        wait_time += 1;
        stat = odt_hal_get_single_src_state(chan_id);
    }
    if (stat) {
        return BSP_ERROR;
    }

    return BSP_OK;
}
EXPORT_SYMBOL(odt_wait_single_chan_idle);

static void odt_coder_set_src_info(u32 chan_id, odt_enc_src_chan_s *src_chan, odt_src_chan_cfg_s *src_attr)
{
    u32 buf_len =
        (uintptr_t)src_attr->coder_src_setbuf.input_end - (uintptr_t)src_attr->coder_src_setbuf.input_start + 1;
    u32 rd_buf_len =
        (uintptr_t)src_attr->coder_src_setbuf.rd_input_end - (uintptr_t)src_attr->coder_src_setbuf.rd_input_start + 1;

    src_chan->chan_mode = src_attr->mode;
    src_chan->priority = src_attr->priority;
    src_chan->data_type = src_attr->data_type;
    src_chan->data_type_en = src_attr->data_type_en;
    src_chan->debug_en = src_attr->debug_en;
    src_chan->dst_chan_id = src_attr->dest_chan_id;
    src_chan->bypass_en = src_attr->bypass_en;
    src_chan->trans_id_en = src_attr->trans_id_en;
    src_chan->ptr_img_en = src_attr->rptr_img_en;
    src_chan->read_ptr_img_phy_addr = src_attr->read_ptr_img_phy_addr;
    src_chan->read_ptr_img_vir_addr = src_attr->read_ptr_img_vir_addr;
    src_chan->enc_src_buff.start = (uintptr_t)src_attr->coder_src_setbuf.input_start;
    src_chan->enc_src_buff.end = (uintptr_t)src_attr->coder_src_setbuf.input_end;
    src_chan->enc_src_buff.write = 0;
    src_chan->enc_src_buff.read = 0;
    src_chan->enc_src_buff.length = buf_len;
    src_chan->enc_src_buff.idle_size = 0;

    if (src_attr->mode == ODT_ENCSRC_CHNMODE_LIST) {
        src_chan->rd_buff.start = (uintptr_t)src_attr->coder_src_setbuf.rd_input_start;
        src_chan->rd_buff.end = (uintptr_t)src_attr->coder_src_setbuf.rd_input_end;
        src_chan->rd_buff.write = 0;
        src_chan->rd_buff.read = 0;
        src_chan->rd_buff.length = rd_buf_len;
        src_chan->rd_threshold = src_attr->coder_src_setbuf.rd_threshold;
        src_chan->rd_irq_mask = (u32)src_attr->coder_src_setbuf.rd_irq_mask;
    }

    src_chan->alloc_state = ODT_CHN_ALLOCATED;
}


/*
 * 函 数 名: bsp_odt_coder_set_src_chan
 * 功能描述: 编码源通道配置函数
 * 输入参数: src_attr 编码源通道配置参数
 * src_chan_id  编码源通道ID
 * 输出参数: 无
 * 返 回 值: 申请及配置成功与否的标识码
 */
s32 bsp_odt_coder_set_src_chan(unsigned int src_chan_id, odt_src_chan_cfg_s *src_attr)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[src_chan_id];

    odt_hal_src_chan_init(src_chan_id, src_attr);
    if (src_attr->mode == ODT_ENCSRC_CHNMODE_LIST && src_attr->coder_src_setbuf.rd_irq_mask == 0) {
        odt_rd_int_mask_proc(src_chan_id, ODT_RD_MASK_CLEAR);
    }

    odt_hal_set_rate_ctrl_group(src_chan_id, src_attr->chan_group);

    odt_coder_set_src_info(src_chan_id, src_chan, src_attr);
    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_coder_set_src_chan);

/*
 * 函 数 名: bsp_odt_free_channel
 * 功能描述: 通道释放函数
 * 输入参数: chan_id 编解码通道指针
 * 输出参数: 无
 * 返 回 值: 释放成功与否的标识码
 */
s32 bsp_odt_free_channel(u32 chan_id)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[chan_id];

    odt_hal_set_src_chan_start_stop(chan_id, 0); /* 关闭当前通道 */

    odt_hal_set_src_buffer_addr(chan_id, 0, 0);
    odt_hal_set_src_write_ptr(chan_id, 0);
    odt_hal_set_src_read_ptr(chan_id, 0);
    odt_hal_set_src_buffer_length(chan_id, 0);

    if (src_chan->chan_mode == ODT_ENCSRC_CHNMODE_LIST) {
        odt_hal_set_src_rd_buffer_addr(chan_id, 0, 0);
        odt_hal_set_src_rd_write_ptr(chan_id, 0);
        odt_hal_set_src_rd_read_ptr(chan_id, 0);
        odt_hal_set_src_rd_buffer_length(chan_id, 0);
    }

    if (src_chan->ptr_img_en) { /* 读指针镜像使能，需要将读指针镜像寄存器清0 */
        odt_hal_set_rptr_img_addr(chan_id, 0, 0);
    }

    odt_hal_clear_src_chan_cfg(chan_id);

    (void)memset_s(src_chan, sizeof(odt_enc_src_chan_s), 0, sizeof(odt_enc_src_chan_s));

    return BSP_OK;
}

/*
 * 函 数 名: bsp_odt_start
 * 功能描述: 编码或者解码启动函数
 * 输入参数: src_chan_id 通道ID
 * 返 回 值: 启动成功与否的标识码
 */
s32 bsp_odt_start(u32 src_chan_id)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[src_chan_id];

    if (src_chan->chan_en == ODT_CHN_ENABLE) {
        return BSP_OK;
    }

    odt_clear_enc_src_int_state(src_chan_id, src_chan->chan_mode, src_chan->rd_irq_mask);

    odt_hal_clear_rate_ctrl_cnt(src_chan_id);

    odt_hal_set_src_chan_start_stop(src_chan_id, 1); /* 源端通道使能 */

    src_chan->chan_en = ODT_CHN_ENABLE;
    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_start);

/*
 * 函 数 名: bsp_odt_stop
 * 功能描述: 编码或者解码停止函数
 * 输入参数: src_chan_id 通道ID
 * 返 回 值: 停止成功与否的标识码
 */
s32 bsp_odt_stop(u32 chan_id)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[chan_id];

    /* 清除原始rd中断 */
    if (src_chan->chan_mode == ODT_ENCSRC_CHNMODE_LIST) {
        odt_rd_int_mask_proc(chan_id, ODT_RD_MASK_SET);
    }

    /* 去使能源通道 */
    odt_hal_set_src_chan_start_stop(chan_id, 0); /* 源端通道使能 */

    src_chan->chan_en = ODT_CHN_DISABLE;
    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_stop);

/*
 * 函 数 名: bsp_odt_get_write_buff
 * 功能描述: 上层获取写数据buffer函数
 * 输入参数: src_chan_id 源通道ID
 * 输出参数: p_buff 获取的buffer
 * 返 回 值: 获取成功与否的标识码
 */
s32 bsp_odt_get_write_buff(u32 chan_id, odt_buffer_rw_s *rw_buff)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[chan_id];

    if (src_chan->ptr_img_en) { /* 读指针镜像使能，需要读取读指针镜像地址中的读指针 */
        src_chan->enc_src_buff.read = *((u32 *)(uintptr_t)src_chan->read_ptr_img_vir_addr);
    } else { /* 读指针镜像不使能，直接从寄存器中读取读指针 */
        src_chan->enc_src_buff.read = odt_hal_get_src_read_ptr(chan_id);
    }

    if (src_chan->enc_src_buff.write < src_chan->enc_src_buff.read) {
        /* 读指针大于写指针，直接计算 */
        rw_buff->buffer = (char *)(uintptr_t)(src_chan->enc_src_buff.start + src_chan->enc_src_buff.write);
        rw_buff->size = src_chan->enc_src_buff.read - src_chan->enc_src_buff.write - 1;
        rw_buff->rb_buffer = NULL;
        rw_buff->rb_size = 0;
    } else {
        /* 写指针大于读指针，需要考虑回卷 */
        rw_buff->buffer = (char *)(uintptr_t)(src_chan->enc_src_buff.start + src_chan->enc_src_buff.write);
        rw_buff->size =
            (u32)(src_chan->enc_src_buff.end - (src_chan->enc_src_buff.start + src_chan->enc_src_buff.write) + 1);
        rw_buff->rb_buffer = (char *)(uintptr_t)(src_chan->enc_src_buff.start);
        rw_buff->rb_size = src_chan->enc_src_buff.read;
    }

    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_get_write_buff);

/*
 * 函 数 名  : bsp_odt_write_done
 * 功能描述  : 写数据完成函数
 * 输入参数  : src_chan_id    源通道ID
 * write_size 写入数据的长度
 * 返 回 值  : 操作完成与否的标识码
 */
s32 bsp_odt_write_done(u32 src_chan_id, u32 write_size)
{
    odt_ring_buff_s *src_buffer = &g_odt_enc_src_info.enc_src_chan[src_chan_id].enc_src_buff;
    u32 cur_size;

    cur_size = src_buffer->end - (src_buffer->start + src_buffer->write) + 1;
    if (cur_size > write_size) {
        src_buffer->write += write_size;
    } else {
        src_buffer->write = write_size - cur_size;
    }

    /* 设置写指针 */
    odt_hal_set_src_write_ptr(src_chan_id, src_buffer->write);

    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_write_done);

/*
 * 函 数 名  : bsp_odt_register_rd_cb
 * 功能描述  : RDbuffer回调函数注册函数
 * 输入参数  : src_chan_id    ：源通道ID
 * rd_cb：RDbuffer完成回调函数
 * 输出参数  : 无
 * 返 回 值  : 注册成功与否的标识码
 */
s32 bsp_odt_register_rd_cb(u32 src_chan_id, odt_rd_cb rd_cb)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[src_chan_id];

    if (src_chan->chan_mode == ODT_ENCSRC_CHNMODE_LIST) {
        src_chan->rd_cb = rd_cb;
    }

    return BSP_OK;
}

/*
 * 函 数 名: bsp_odt_get_rd_buffer
 * 功能描述: 获取RDbuffer函数
 * 输入参数: src_chan_id    源通道ID
 * 输出参数: p_buff           获取的RDbuffer
 * 返 回 值: 获取成功与否的标识码
 */
s32 bsp_odt_get_rd_buffer(u32 src_chan_id, odt_buffer_rw_s *p_buff)
{
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[src_chan_id];

    /* 判断参数有效性 */
    if (p_buff == NULL) {
        odt_error("the parameter is NULL\n");
        return BSP_ERROR;
    }

    src_chan->rd_buff.write = odt_hal_get_src_rd_write_ptr(src_chan_id);

    if (src_chan->rd_buff.read <= src_chan->rd_buff.write) {
        p_buff->buffer = (char *)(uintptr_t)(src_chan->rd_buff.start + src_chan->rd_buff.read);
        p_buff->size = (u32)(src_chan->rd_buff.write - src_chan->rd_buff.read);
        p_buff->rb_buffer = NULL;
        p_buff->rb_size = 0;
    } else {
        p_buff->buffer = (char *)(uintptr_t)(src_chan->rd_buff.start + src_chan->rd_buff.read);
        p_buff->size = (uintptr_t)(src_chan->rd_buff.end - (src_chan->rd_buff.start + src_chan->rd_buff.read) + 1);
        p_buff->rb_buffer = (char *)(uintptr_t)(src_chan->rd_buff.start);
        p_buff->rb_size = src_chan->rd_buff.write;
    }

    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_get_rd_buffer);

/*
 * 函 数 名: bsp_odt_read_rd_done
 * 功能描述: 读取RDbuffer数据完成函数
 * 输入参数: src_chan_id   源通道ID
 * rd_size      读取的RDbuffer数据长度
 * 输出参数:
 * 返 回 值: 读取成功与否的标识码
 */
s32 bsp_odt_read_rd_done(u32 src_chan_id, u32 rd_size)
{
    odt_ring_buff_s *rd_buffer = &g_odt_enc_src_info.enc_src_chan[src_chan_id].rd_buff;
    odt_buffer_rw_s rw_buff;
    s32 ret;

    ret = bsp_odt_get_rd_buffer(src_chan_id, &rw_buff);
    if (ret) {
        return ret;
    }

    if (rw_buff.size + rw_buff.rb_size < rd_size) {
        odt_error("rw_buffer is not enough, rd_size=0x%x\n", rd_size);
        odt_rd_int_mask_proc(src_chan_id, ODT_RD_MASK_CLEAR);
        return BSP_ERROR;
    }

    rd_buffer->read += rd_size;
    if (rd_buffer->read > rd_buffer->end - rd_buffer->start) {
        rd_buffer->read -= rd_buffer->length;
    }

    odt_hal_set_src_rd_read_ptr(src_chan_id, rd_buffer->read);
    return BSP_OK;
}
EXPORT_SYMBOL(bsp_odt_read_rd_done);

s32 bsp_clear_odt_src_buffer(u32 src_chan_id)
{
    u32 value;
    odt_enc_src_chan_s *src_chan = &g_odt_enc_src_info.enc_src_chan[src_chan_id];

    if(odt_wait_single_chan_idle(src_chan_id)) {
        return BSP_ERROR;
    }

    value = odt_hal_get_src_read_ptr(src_chan_id);
    odt_hal_set_src_write_ptr(src_chan_id, value);
    src_chan->enc_src_buff.write = value;

    return BSP_OK;
}

u32 bsp_odt_get_single_src_state(u32 src_chan_id)
{
    return odt_hal_get_single_src_state(src_chan_id);
}
EXPORT_SYMBOL(bsp_odt_get_single_src_state);

void bsp_odt_set_rate_threshold(short rate_limits)
{
    odt_hal_set_rate_ctrl_threshold(rate_limits);
}

s32 bsp_odt_set_rate_ctrl(DRV_DIAG_RATE_STRU *rate_ctrl)
{
    if (rate_ctrl->chan_enable == 0) {
        odt_crit("rate ctrl closed\n");
        odt_hal_set_rate_ctrl_cfg(0, 0);
        return BSP_OK;
    }
    /* 流控周期控制 */
    odt_hal_set_rate_ctrl_peroid(ODT_RATE_PERIOD);

    /* 流控阈值配置 */
    bsp_odt_set_rate_threshold(rate_ctrl->rate_limits);

    /* 流控使能 */
    odt_hal_set_rate_ctrl_cfg(rate_ctrl->chan_rate_bits_l, rate_ctrl->chan_rate_bits_h);

    return BSP_OK;
}

