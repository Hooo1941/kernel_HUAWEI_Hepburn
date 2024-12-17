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
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "osl_malloc.h"
#include "bsp_odt.h"
#include "mprobe_channel_manager.h"
#include "mprobe_port.h"
#include "mprobe_debug.h"

#undef THIS_MODU
#define THIS_MODU mod_mprobe

spinlock_t g_src_buf_spin_lock;

src_chan_cfg_s g_src_chan_cfg = { 
    .init_state = MPROBE_CHANNEL_UNINIT,
    .chan_id = ODT_CODER_SRC_AP_MPROBE,
    .dst_chan_id = ODT_CODER_DST_LOGSERVER,
    .data_type = ODT_DATA_TYPE_0,
    .chan_mode = ODT_ENCSRC_CHNMODE_CTSPACKET,
    .chan_prio = ODT_CHAN_PRIORITY_3,
    .trans_id_en = ODT_TRANS_ID_EN,
    .buff_len = MPROBE_SRC_BUFF_LEN,
    .src_virt_buff = NULL,
    .src_phy_buff = NULL,
};

dst_chan_cfg_s g_dst_chan_cfg = {
    .init_state = MPROBE_CHANNEL_UNINIT,
    .chan_id = ODT_CODER_DST_LOGSERVER,
    .buff_len = MPROBE_DST_BUFF_LEN,
    .threshold = MPROBE_DST_THRESHOLD,
    .timeout_mode = ODT_TIMEOUT_TFR_SHORT,
    .dst_virt_buff = NULL,
    .dst_phy_buff = NULL,
};


unsigned long *mprobe_src_chan_phy_to_virt(const u8 *phy_addr)
{
    if (unlikely((phy_addr < g_src_chan_cfg.src_phy_buff) || 
                 (phy_addr >= (g_src_chan_cfg.src_phy_buff + g_src_chan_cfg.buff_len)))) {
        return NULL;
    }

    return (unsigned long *)((phy_addr - g_src_chan_cfg.src_phy_buff) + g_src_chan_cfg.src_virt_buff);
}

unsigned long *mprobe_dst_chan_phy_to_virt(const u8 *phy_addr)
{
    if (unlikely((phy_addr < g_dst_chan_cfg.dst_phy_buff) || 
                 (phy_addr >= (g_dst_chan_cfg.dst_phy_buff + g_dst_chan_cfg.buff_len)))) {
        return NULL;
    }

    return (unsigned long *)((phy_addr - g_dst_chan_cfg.dst_phy_buff) + g_dst_chan_cfg.dst_virt_buff);
}

void mprobe_enable_dst_chan(void)
{
    bsp_odt_encdst_dsm_init(g_dst_chan_cfg.chan_id, ODT_DEST_DSM_ENABLE);
}

int mprobe_rls_dst_buff(u32 read_size)
{
    u32 len;
    odt_buffer_rw_s buffer;

    if (read_size == 0) { /* 释放通道所有数据 */
        if (bsp_odt_get_read_buff(g_dst_chan_cfg.chan_id, &buffer) != BSP_OK) {
            mprobe_error("Get Read Buffer is Error\n");
            return BSP_ERROR;
        }
        len = buffer.size + buffer.rb_size;
    } else {
        len = read_size;
    }

    if (bsp_odt_read_data_done(g_dst_chan_cfg.chan_id, len) != BSP_OK) {
        mprobe_error("Read Data Done is Error\n");
        return BSP_ERROR;
    }

    return BSP_OK;
}

static void mprobe_send_data_to_port(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    u32 send_len;
    int send_result;

    mprobe_get_data_len(len, &send_len);

    send_result = mprobe_data_send(virt_addr, phy_addr, send_len);
    if (send_result != MPROBE_SEND_AYNC) {
        mprobe_error("send to data error\n");
    }
}

int mprobe_dst_chan_read_cb(u32 chan_id)
{
    odt_buffer_rw_s buffer;
    unsigned long *virt_addr = NULL;

    if (bsp_odt_get_read_buff(g_dst_chan_cfg.chan_id, &buffer) != BSP_OK) {
        mprobe_error("Get Read Buffer is Error\n");
        return BSP_ERROR;
    }

    if (((buffer.size + buffer.rb_size) == 0) || (buffer.buffer == NULL)) {
        bsp_odt_read_data_done(g_dst_chan_cfg.chan_id, buffer.size + buffer.rb_size); /* 清空数据 */
        mprobe_error("Get RB error\n");
        return BSP_ERROR;
    }

    /* 发送数据 */
    virt_addr = mprobe_dst_chan_phy_to_virt((u8 *)buffer.buffer);
    if (virt_addr == NULL) {
        bsp_odt_read_data_done(g_dst_chan_cfg.chan_id, buffer.size + buffer.rb_size); /* 清空数据 */
        mprobe_error("buffer convert to virt fail\n");
        return BSP_ERROR;
    }

    mprobe_send_data_to_port((u8 *)virt_addr, (u8 *)buffer.buffer, (u32)buffer.size);

    return BSP_OK;
}


int mprobe_dst_chan_cfg(void)
{
    odt_dst_chan_cfg_s chan_info = {0};

    chan_info.encdst_thrh = MPROBE_CODER_DST_GTHRESHOLD;               /*left space */
    chan_info.encdst_timeout_mode = g_dst_chan_cfg.timeout_mode;
    chan_info.coder_dst_setbuf.threshold = g_dst_chan_cfg.threshold;   /* waterline */

    g_dst_chan_cfg.dst_virt_buff = (u8 *)dma_alloc_coherent(&g_odt_pdev->dev, g_dst_chan_cfg.buff_len,
        (dma_addr_t *)&g_dst_chan_cfg.dst_phy_buff, GFP_KERNEL);
    if (g_dst_chan_cfg.dst_virt_buff == NULL) {
        g_dst_chan_cfg.init_state = MPROBE_CHANNEL_MEM_FAIL;
        mprobe_error("dst chan buffer alloc fail\n");
        return BSP_ERROR;
    }

    chan_info.coder_dst_setbuf.output_start = g_dst_chan_cfg.dst_phy_buff;
    chan_info.coder_dst_setbuf.output_end = (g_dst_chan_cfg.dst_phy_buff + g_dst_chan_cfg.buff_len) - 1;

    /* 调用ODT接口进行编码目的通道配置 */
    if (bsp_odt_coder_set_dst_chan(g_dst_chan_cfg.chan_id, &chan_info)) {
        mprobe_error("odt dst chan cfg fail\n");
        return BSP_ERROR;
    }

    mprobe_crit("dst chan %d init successfull\n", g_dst_chan_cfg.chan_id);
    return BSP_OK;
}

int mprobe_src_chan_cfg(void)
{
    odt_src_chan_cfg_s chan_info = {0};

    chan_info.dest_chan_id = g_src_chan_cfg.dst_chan_id;
    chan_info.data_type = g_src_chan_cfg.data_type;
    chan_info.mode = g_src_chan_cfg.chan_mode;
    chan_info.priority = g_src_chan_cfg.chan_prio;
    chan_info.trans_id_en = g_src_chan_cfg.trans_id_en;
    chan_info.data_type_en = ODT_DATA_TYPE_EN;             /* disable actually */
    chan_info.debug_en = ODT_ENC_DEBUG_DIS;
    chan_info.bypass_en = ODT_HDLC_ENABLE;                 /* disable actually */

    g_src_chan_cfg.src_virt_buff = (u8 *)dma_alloc_coherent(&g_odt_pdev->dev, g_src_chan_cfg.buff_len,
        (dma_addr_t *)&g_src_chan_cfg.src_phy_buff, GFP_KERNEL);
    if (g_src_chan_cfg.src_virt_buff == NULL) {
        g_src_chan_cfg.init_state = MPROBE_CHANNEL_MEM_FAIL;
        mprobe_error("src chan buffer alloc fail\n");
        return BSP_ERROR;
    }

    chan_info.coder_src_setbuf.input_start = g_src_chan_cfg.src_phy_buff;
    chan_info.coder_src_setbuf.input_end = (g_src_chan_cfg.src_phy_buff + g_src_chan_cfg.buff_len) - 1;

    /* 调用ODT接口进行编码源通道配置 */
    if (bsp_odt_coder_set_src_chan(g_src_chan_cfg.chan_id, &chan_info)) {
        mprobe_error("odt src chan cfg fail\n");
        return BSP_ERROR;
    }

    mprobe_crit("src chan %d init successfull\n", g_src_chan_cfg.chan_id);
    return BSP_OK;
}

int mprobe_dst_chan_start(void)
{
    int ret;

    ret = mprobe_dst_chan_cfg();
    if (ret != BSP_OK) {
        g_dst_chan_cfg.init_state = MPROBE_CHANNEL_CFG_FAIL;
        mprobe_error("dst chan cfg fail\n");
        return ret;
    }

    (void)bsp_odt_register_read_cb(g_dst_chan_cfg.chan_id, (odt_read_cb)mprobe_dst_chan_read_cb);

    (void)bsp_odt_dst_channel_enable(g_dst_chan_cfg.chan_id);

    bsp_odt_encdst_dsm_init(g_dst_chan_cfg.chan_id, ODT_DEST_DSM_ENABLE);

    return ret;
}

int mprobe_src_chan_start(void)
{
    int ret;

    ret = mprobe_src_chan_cfg();
    if (ret != BSP_OK) {
        g_src_chan_cfg.init_state = MPROBE_CHANNEL_CFG_FAIL;
        mprobe_error("src chan cfg fail\n");
        return ret;
    }

    ret = bsp_odt_start(g_src_chan_cfg.chan_id);
    if (ret != BSP_OK) {
        g_src_chan_cfg.init_state = MPROBE_CHANNEL_START_FAIL;
        mprobe_error("src chan start fail\n");
        return ret;
    }

    return ret;
}

void mprobe_dst_chan_stop(void)
{
    bsp_odt_free_enc_dst_chan(g_src_chan_cfg.dst_chan_id);

    dma_free_coherent(&g_odt_pdev->dev, g_dst_chan_cfg.buff_len,
        g_dst_chan_cfg.dst_virt_buff, (dma_addr_t)(uintptr_t)g_dst_chan_cfg.dst_phy_buff);

    g_dst_chan_cfg.dst_virt_buff = NULL;
    g_dst_chan_cfg.dst_phy_buff = NULL;
}

void mprobe_src_chan_stop(void)
{
    u32 stat;
    u32 max_delay = MPROBE_WAIT_ODT_IDLE_MAX_DELAY;

    bsp_odt_stop(g_src_chan_cfg.chan_id);

    stat = bsp_odt_get_single_src_state(g_src_chan_cfg.chan_id);
    while (stat && max_delay){
        msleep(MPROBE_WAIT_ODT_IDLE_SINGLE_TIMEMS);
        stat = bsp_odt_get_single_src_state(g_src_chan_cfg.chan_id);
        max_delay--;
    }
    if (stat) {
        mprobe_error("wait odt src chan idle timeout\n");
    }

    dma_free_coherent(&g_odt_pdev->dev, g_src_chan_cfg.buff_len,
        g_src_chan_cfg.src_virt_buff, (dma_addr_t)(uintptr_t)g_src_chan_cfg.src_phy_buff);

    g_src_chan_cfg.src_virt_buff = NULL;
    g_src_chan_cfg.src_phy_buff = NULL;
}

void mprobe_channel_stop(void)
{
    mprobe_src_chan_stop();

    mprobe_dst_chan_stop();

    g_src_chan_cfg.init_state = MPROBE_CHANNEL_UNINIT;
    g_dst_chan_cfg.init_state = MPROBE_CHANNEL_UNINIT;

    mprobe_crit("mprobe_channel_stop ok\n");
}

int mprobe_channel_start(void)
{
    int ret;

    spin_lock_init(&g_src_buf_spin_lock);

    ret = mprobe_src_chan_start();
    if (ret != BSP_OK) {
        mprobe_error("src chan start fail\n");
        return ret;
    }

    ret = mprobe_dst_chan_start();
    if (ret != BSP_OK) {
        mprobe_error("dst chan start fail\n");
        return ret;
    }

    g_src_chan_cfg.init_state = MPROBE_CHANNEL_INIT_SUCC;
    g_dst_chan_cfg.init_state = MPROBE_CHANNEL_INIT_SUCC;

    mprobe_crit("mprobe_channel_start ok\n");

    return ret;
}

void mprobe_write_data(odt_buffer_rw_s *rw_buffer, const void *payload, u32 len)
{
    u32 size;
    u32 rb_size;
    errno_t ret;

    if (len == 0) {
        return;
    }

    if (rw_buffer->size > len) {
        ret = memcpy_s(((u8 *)rw_buffer->buffer), rw_buffer->size, payload, len);
        if (ret != EOK) {
            mprobe_error("memory copy fail %x\n", ret);
        }
        rw_buffer->buffer = rw_buffer->buffer + len;
        rw_buffer->size = rw_buffer->size - len;
    } else {
        size = rw_buffer->size;
        ret = memcpy_s(((u8 *)rw_buffer->buffer), rw_buffer->size, payload, size);
        if (ret != EOK) {
            mprobe_error("memory copy fail %x\n", ret);
        }

        rb_size = len - rw_buffer->size;
        if (rb_size && rw_buffer->rb_buffer != NULL) {
            ret = memcpy_s(rw_buffer->rb_buffer, rw_buffer->rb_size, ((u8 *)payload + size), rb_size);
            if (ret != EOK) {
                mprobe_error("memory copy fail %x\n", ret);
            }
        }

        rw_buffer->buffer = rw_buffer->rb_buffer + rb_size;
        rw_buffer->size = rw_buffer->rb_size - rb_size;
        rw_buffer->rb_buffer = NULL;
        rw_buffer->rb_size = 0;
    }

    return;
}

int mprobe_src_chan_send(mprobe_msg_report_head_s *msg)
{
    u32 ret;
    u32 total_len;
    unsigned long lock;
    mprobe_odt_head_s *header;
    odt_buffer_rw_s buf = { NULL, NULL, 0, 0 };

    total_len = ALIGN_DDR_WITH_8BYTE(msg->header_size + msg->data_size);

    header = &(((mprobe_srv_head_s *)(msg->header))->header);
    header->data_len = msg->header_size + msg->data_size - sizeof(mprobe_odt_head_s);

    mprobe_debug("msg_head_size:0x%x", msg->header_size);
    mprobe_debug("msg_data_size:0x%x", msg->data_size);

    spin_lock_irqsave(&g_src_buf_spin_lock, lock);

    ret = bsp_odt_get_write_buff(g_src_chan_cfg.chan_id, &buf);
    if (ret) {
        spin_unlock_irqrestore(&g_src_buf_spin_lock, lock);
        return BSP_ERROR;
    }

    /* 虚拟地址转换 */
    if ((buf.size + buf.rb_size) >= (total_len)) {
        buf.buffer = (char *)mprobe_src_chan_phy_to_virt((u8 *)buf.buffer);
        buf.rb_buffer = (char *)mprobe_src_chan_phy_to_virt((u8 *)buf.rb_buffer);
    } else {
        spin_unlock_irqrestore(&g_src_buf_spin_lock, lock);
        return BSP_ERROR;
    }

    if (buf.buffer == NULL) {
        spin_unlock_irqrestore(&g_src_buf_spin_lock, lock);
        mprobe_error("get odt buffer is null\n");
        return BSP_ERROR;
    }

    mprobe_write_data(&buf, msg->header, msg->header_size);
    mprobe_write_data(&buf, msg->data, msg->data_size);

    ret = (u32)bsp_odt_write_done(g_src_chan_cfg.chan_id, total_len);
    spin_unlock_irqrestore(&g_src_buf_spin_lock, lock);
    if (ret) {
        mprobe_error("write done fail,ret:0x%x\n", ret);
        return BSP_ERROR;
    }

    return ret;
}

int mprobe_srv_pack_data(mprobe_msg_report_head_s *msg)
{
    int ret;
    int data_len;
    u32 frame_header_size;
    mprobe_msg_report_head_s report_info;
    mprobe_frame_head_s *frame_header = NULL;

    frame_header_size = msg->header_size - sizeof(mprobe_odt_head_s);
    data_len = (s32)(frame_header_size + MPROBE_SERVCIE_GET_REAL_LEN(msg->data_size));
    /* 所要发送数据的总长度 */
    if (data_len > MPROBE_FRAME_MAX_LEN) {
        data_len = MPROBE_FRAME_MAX_LEN;
        mprobe_error("frame more than max len, data will be truncated!\n");
    }

    frame_header = &(((mprobe_srv_head_s *)(msg->header))->frame_header);
    frame_header->srv_head.sflag = 0;
    frame_header->srv_head.esflag = 1;

    report_info.header = msg->header;
    report_info.header_size = msg->header_size;
    report_info.data = msg->data;
    report_info.data_size = data_len - frame_header_size;

    ret = mprobe_src_chan_send(&report_info);
    if (ret != BSP_OK) {
        mprobe_error("send to odt fail\n");
        return BSP_ERROR;
    }

    return ret;
}