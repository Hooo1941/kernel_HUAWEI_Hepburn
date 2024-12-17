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
#include <linux/module.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <osl_malloc.h>
#include <securec.h>
#include <bsp_rfile.h>
#include <bsp_dump.h>
#include <bsp_slice.h>
#include <bsp_omp.h>
#include "omp_service.h"
#include "algorithm.h"
#include "decode.h"


#define THIS_MODU mod_soft_dec

struct soft_decode_info g_soft_decode_info[MAX_SOFT_DECODE_NUM];

void soft_decode_info_show(void)
{
    int num;
    for (num = 0; num < MAX_SOFT_DECODE_NUM; num++) {
        soft_decode_crit("scm_soft_decode_info_show for num%d: \n", num);

        soft_decode_crit("\r\nSem Creat Error %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.sem_create_err);
        soft_decode_crit("\r\nSem Give Error %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.sem_give_err);
        soft_decode_crit("\r\nRing Buffer Creat Error %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.ringbuff_create_err);
        soft_decode_crit("\r\nTask Id Error %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.task_id_err);
        soft_decode_crit("\r\nRing Buffer not Enough %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.buff_no_enough);
        soft_decode_crit("\r\nRing Buffer Flush %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.rbuff_flush);
        soft_decode_crit("\r\nRing Buffer Put Error %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.rb_info.rbuff_put_err);

        soft_decode_crit("\r\nRing Buffer Put Success Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.put_info.num);
        soft_decode_crit("\r\nRing Buffer Put Success Bytes %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.put_info.data_len);

        soft_decode_crit("\r\nRing Buffer Get Success Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.get_info.num);
        soft_decode_crit("\r\nRing Buffer Get Success Bytes %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.get_info.data_len);

        soft_decode_crit("\r\nHDLC Decode Success Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.hdlc_decap_data.num);
        soft_decode_crit("\r\nHDLC Decode Success Bytes %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.hdlc_decap_data.data_len);

        soft_decode_crit("\r\nHDLC Decode Error Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.frame_decap_data);

        soft_decode_crit("\r\nHDLC Init Error Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.hdlc_init_err);

        soft_decode_crit("\r\nHDLC Decode Data Type Error Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.data_type_err);

        soft_decode_crit("\r\nCPM Reg Logic Rcv Func Success Times %d:\r\n", g_soft_decode_info[num].soft_decode_mntn_info.cpm_reg_logic_rcv_suc);
    }
}

static struct soft_decode_info *hdlc_get_decode_info(unsigned int ring_id)
{
    if (ring_id < MAX_SOFT_DECODE_NUM) {
        return &g_soft_decode_info[ring_id];
    }

    return NULL;
}

static void soft_decode_data_dispatch(om_hdlc_s *hdlc_ctrl)
{
    omp_service_process(hdlc_ctrl->decap_buff + sizeof(u8), hdlc_ctrl->info_len - sizeof(u8), 0, 0);
}

static u32 soft_decode_data_recv(enum soft_decode_type ring_id, const u8 *buffer, u32 len)
{
    rw_buffer_s rw_buffer;
    u32 ret;
    struct soft_decode_info *sd_info = hdlc_get_decode_info(ring_id);
    if (sd_info == NULL) {
        soft_decode_error("soft decode recv err!\n");
        return BSP_ERROR;
    }

    om_get_idle_buffer(sd_info->soft_decode_rcv_ctrl.ring_buffer, &rw_buffer);
    if ((rw_buffer.size + rw_buffer.rb_size) < len) {
        sd_info->soft_decode_mntn_info.rb_info.buff_no_enough++;
        return BSP_ERROR;
    }

    ret = om_ring_buffer_put(sd_info->soft_decode_rcv_ctrl.ring_buffer, rw_buffer, buffer, (s32)len);
    if (ret) {
        sd_info->soft_decode_mntn_info.rb_info.rbuff_put_err++;
        return ret;
    }

    osl_sem_up(&(sd_info->soft_decode_rcv_ctrl.sem_id));

    return BSP_OK;
}

u32 bsp_hdlc_decode_data(enum soft_decode_type ring_id, u8 *buffer, u32 len)
{
    u32 ret;
    unsigned long lock_level;
    struct soft_decode_info *sd_info = hdlc_get_decode_info(ring_id);
    if (sd_info == NULL) {
        soft_decode_error("soft decode cfg recv err!\n");
        return BSP_ERROR;
    }
    spin_lock_irqsave(&sd_info->soft_decode_data_rcv_spinlock, lock_level);
    ret = soft_decode_data_recv(ring_id, buffer, len);
    spin_unlock_irqrestore(&sd_info->soft_decode_data_rcv_spinlock, lock_level);
    return ret;
}

static u32 soft_decode_send_to_hdlc_dec(om_hdlc_s *hdlc_ctrl, const u8 *data, u32 len)
{
    u32 i;
    u32 result;
    u8 tmp_char;
    struct soft_decode_info *sd_info = container_of(hdlc_ctrl, struct soft_decode_info, soft_decode_hdlc_entity);
    result = BSP_ERROR;

    for (i = 0; i < len; i++) {
        tmp_char = (u8)data[i];

        result = om_hdlc_decap(hdlc_ctrl, tmp_char);
        if (result == HDLC_SUCC) {
            sd_info->soft_decode_mntn_info.hdlc_decap_data.data_len += hdlc_ctrl->info_len;
            sd_info->soft_decode_mntn_info.hdlc_decap_data.num++;

            soft_decode_data_dispatch(hdlc_ctrl);
        } else if (result == HDLC_NOT_HDLC_FRAME) {
            /* 不是完整分帧,继续HDLC解封装 */
        } else {
            sd_info->soft_decode_mntn_info.frame_decap_data++;
        }
    }

    return BSP_OK;
}

static u32 soft_decode_hdlc_init(om_hdlc_s *hdlc_ctrl)
{
    /* 申请用于HDLC解封装的缓存 */
    hdlc_ctrl->decap_buff = (u8 *)osl_malloc(SEND_BUFFER_SIZE);

    if (hdlc_ctrl->decap_buff == NULL) {
        soft_decode_error("decap_buff malloc fail!\n");
        return BSP_ERROR;
    }

    hdlc_ctrl->decap_buff_size = SEND_BUFFER_SIZE;

    /* 初始化HDLC解封装控制上下文 */
    om_hdlc_init(hdlc_ctrl);

    return BSP_OK;
}

static s32 soft_decode_recv_task(void *para)
{
    s32 ret;
    s32 len;
    unsigned long lock_level;
    rw_buffer_s rw_buffer;
    struct soft_decode_info *sd_info = NULL;
    if (para == NULL) {
        soft_decode_error("soft decode task got null para!\n");
        return BSP_ERROR;
    }
    sd_info = (struct soft_decode_info *)para;

    for (;;) {
        osl_sem_down(&(sd_info->soft_decode_rcv_ctrl.sem_id));

        om_get_data_buffer(sd_info->soft_decode_rcv_ctrl.ring_buffer, &rw_buffer);
        if ((rw_buffer.size + rw_buffer.rb_size) == 0) {
            continue;
        }
        len = rw_buffer.size + rw_buffer.rb_size;

        ret = om_ring_buffer_get(sd_info->soft_decode_rcv_ctrl.ring_buffer, rw_buffer,
            sd_info->soft_decode_rcv_ctrl.buffer, len);
        if (ret) {
            spin_lock_irqsave(&sd_info->soft_decode_data_rcv_spinlock, lock_level);
            om_ring_buffer_flush(sd_info->soft_decode_rcv_ctrl.ring_buffer);
            spin_unlock_irqrestore(&sd_info->soft_decode_data_rcv_spinlock, lock_level);
            sd_info->soft_decode_mntn_info.rb_info.rbuff_flush++;

            continue;
        }
        sd_info->soft_decode_mntn_info.get_info.data_len += len;

        /* 调用HDLC解封装函数 */
        if (BSP_OK != soft_decode_send_to_hdlc_dec(&sd_info->soft_decode_hdlc_entity, (u8 *)sd_info->soft_decode_rcv_ctrl.buffer,
            (u32)len)) {
            soft_decode_error("soft_decode_send_to_hdlc_dec Fail\n");
        }
    }

    return 0;
}

u32 bsp_hdlc_decode_register(enum soft_decode_type ring_id, unsigned int ringbuf_size)
{
    u32 result;
    OSL_TASK_ID task_id;
    struct soft_decode_info *sd_info = NULL;
    sd_info = hdlc_get_decode_info(ring_id);
    if (sd_info == NULL || ringbuf_size > RING_BUFFER_SIZE) {
        soft_decode_error("sd_info init para err\n");
        return BSP_ERROR;
    }
    sd_info->soft_decode_rcv_ctrl.ring_buffer = (om_ring_s *)osl_malloc(sizeof(om_ring_s));
    if (sd_info->soft_decode_rcv_ctrl.ring_buffer == NULL) {
        soft_decode_error("ring_buffer malloc fail\n");
        return BSP_ERROR;
    }

    om_ring_buffer_flush(sd_info->soft_decode_rcv_ctrl.ring_buffer);

    sd_info->soft_decode_rcv_ctrl.ring_buffer->buf = (char *)osl_malloc(ringbuf_size);
    if (sd_info->soft_decode_rcv_ctrl.ring_buffer->buf == NULL) {
        soft_decode_error("ring_buffer->buf malloc fail\n");
        return BSP_ERROR;
    }
    sd_info->soft_decode_rcv_ctrl.ring_buffer->buf_size = ringbuf_size;

    osl_sem_init(0, &(sd_info->soft_decode_rcv_ctrl.sem_id));

    (void)memset_s(&(sd_info->soft_decode_mntn_info), sizeof(sd_info->soft_decode_mntn_info), 0, sizeof(sd_info->soft_decode_mntn_info));

    if (BSP_OK != soft_decode_hdlc_init(&sd_info->soft_decode_hdlc_entity)) {
        soft_decode_error("HDLC Init Fail.\n");
        sd_info->soft_decode_mntn_info.hdlc_init_err++;
        goto SOFT_DECODE_ERROR;
    }

    sd_info->soft_decode_rcv_ctrl.buffer = (char *)osl_malloc(SEND_BUFFER_SIZE);
    if (sd_info->soft_decode_rcv_ctrl.buffer == NULL) {
        soft_decode_error("ctrl_info.buffer malloc fail\n");
        goto SOFT_DECODE_ERROR;
    }

    spin_lock_init(&sd_info->soft_decode_data_rcv_spinlock);

    /* 注册OM配置数据接收自处理任务 */
    result = (u32)osl_task_init("soft_dec", SOFT_DECODE_TASK_PRIORITY, SOFT_DECODE_TASK_STACK_SIZE,
        (OSL_TASK_FUNC)soft_decode_recv_task, (void *)sd_info, &task_id);
    if (result) {
        soft_decode_error("soft_decode_recv_task init Fail.\n");
        goto SOFT_DECODE_ERROR;
    }

    sd_info->soft_decode_mntn_info.cpm_reg_logic_rcv_suc++;

    soft_decode_crit("[init]soft decode init ok\n");

    return BSP_OK;

SOFT_DECODE_ERROR:
    osl_free(sd_info->soft_decode_rcv_ctrl.ring_buffer->buf);
    osl_free(sd_info->soft_decode_rcv_ctrl.ring_buffer);
    return BSP_ERROR;
}

EXPORT_SYMBOL(bsp_hdlc_decode_data);

