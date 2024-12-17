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
#ifndef __BSP_MPROBE_H__
#define __BSP_MPROBE_H__

#include <product_config.h>
#include <stdarg.h>
#include <osl_types.h>
#include <bsp_omp.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPROBE_BASE_MAGIC (0x26765210)
#define MPROBE_ODT_HEAD_MAGIC (MPROBE_BASE_MAGIC + 0x21D30139)
/* 单帧最大长度 */
#define MPROBE_FRAME_MAX_LEN (4 * 1024)
/* 4字节对齐 */
#define ALIGN_DDR_WITH_4BYTE(len) (((len) + 3) & (~3))
/* 8字节对齐 */
#define ALIGN_DDR_WITH_8BYTE(len) (((len) + 7) & (~7))


typedef enum {
    MPROBE_SSID_DEFAULT = 0x0,      /* 0x00：默认值 */
    MPROBE_SSID_ACPU = 0x1,         /* 0x01：A-CPU */
    MPROBE_SSID_CCPU = 0x2,         /* 0x02：C-CPU */
    MPROBE_FRAME_SSID_BUTT,
} mprobe_frame_ssid_type_e;

typedef enum {
    MPROBE_COMPID_DEFAULT = 0,
    MPROBE_COMPID_TRACE = 4,
    MPROBE_COMPID_SYSVIEW = 5,
    MPROBE_COMPID_MEMORY = 6,
    MPROBE_COMPID_BUTT
} mprobe_compid_e;

/* 描述 :二级头: mprobe消息头 */
typedef struct {
    u32 msgid_16b : 16;
    u32 comp_8b : 8;
    u32 rsv_8b : 8;
} mprobe_msg_id_s;

typedef struct {
    u32 msg_len : 16;
    u32 resv : 16;
} mprobe_msg_len_s;

/* 描述 :整体帧结构 */
typedef struct {
    omp_service_head_s srv_head;
    mprobe_msg_id_s msg_id;
    mprobe_msg_len_s msg_len;
    u8 data[0]; /*lint !e43 */
} mprobe_frame_head_s;

typedef struct {
    u32 magic;
    u32 data_len;   /* 数据长度 */
} mprobe_odt_head_s;

typedef struct {
    mprobe_odt_head_s header;
    mprobe_frame_head_s frame_header;
} mprobe_srv_head_s;


typedef struct {
    u32 header_size; /* 数据头的长度 */
    void *header;    /* 数据头 */
    u32 data_size;   /* ucData的长度 */
    void *data;      /* 数据 */
} mprobe_msg_report_head_s;

typedef enum {
    MPROBE_STATE_DISCONNECT = 0,
    MPROBE_STATE_CONNECT = 1,
} mprobe_conn_state_e;

#define MPROBE_SERVICE_HEAD_LEN sizeof(omp_service_head_s)
#define MPROBE_ODT_HEAD_SIZE (sizeof(mprobe_odt_head_s))
#define MPROBE_FRAME_HEAD_SIZE (sizeof(mprobe_frame_head_s))

#define MPROBE_SUB_SYS_ID(data) ((u32)(((struct omp_service_msg *)(data))->ssid))
#define MPROBE_PAYLOAD_COMP_ID(data) ((u32)(((mprobe_frame_head_s *)(data))->msg_id.comp_8b))
#define MPROBE_SERVCIE_GET_REAL_LEN(len) ((len) & 0xffff)


typedef int (*mprobe_service_func)(void *data, u32 len);

typedef int (*mprobe_notify_func)(mprobe_conn_state_e state);

#ifdef CONFIG_MPROBE

void bsp_mprobe_func_reg(mprobe_compid_e compid, mprobe_service_func srv_fn);

void bsp_mprobe_notify_reg(mprobe_compid_e compid, mprobe_notify_func notify_fn);
void bsp_mprobe_fill_srv_head(mprobe_srv_head_s *mprobe_head);

int bsp_mprobe_data_report(mprobe_msg_report_head_s *data);
#else

static inline void bsp_mprobe_func_reg(mprobe_compid_e compid, mprobe_service_func srv_fn)
{
    return;
}

static inline void bsp_mprobe_notify_reg(mprobe_compid_e compid, mprobe_notify_func notify_fn)
{
    return;
}

static inline void bsp_mprobe_fill_srv_head(mprobe_srv_head_s *mprobe_head)
{
    return;
}

static inline int bsp_mprobe_data_report(mprobe_msg_report_head_s *data)
{
    return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#endif
