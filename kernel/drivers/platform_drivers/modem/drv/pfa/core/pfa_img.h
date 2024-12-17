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

#ifndef __PFA_IMG_H__
#define __PFA_IMG_H__

#include "pfa.h"

// load_type定义格式：
// load_type bit0 上电加载场景：0-不做处理，1-加载镜像；
// load_type bit1 备份恢复场景 ：00-不做处理，01-不用备份，唤醒时重新加载镜像，10-备份恢复，11-RSV
// load_type bit2 异常dump场景：0-不做处理，1-异常情况下dump数据
#define LOAD_TYPE_POWERON_NEED_LOAD(load_type)  ((load_type) & 0x1) // bit 0 为1表示需要加载
#define LOAD_TYPE_IDRX(load_type)  (((load_type) >> 1) & 0x3)
#define LOAD_TYPE_EXC_NEED_DUMP(load_type)  (((load_type) >> 2) & 0x1)

struct pfa_img_header {
    char bin_name[4]; // name 4
    char time[20];  // time 20
    char bin_version;
    unsigned int ip_version;
    unsigned int img_bytes;
    unsigned int sect_num;
    unsigned int data_offset;
};

#define PFA_SEG_HDR_OFFSET 0x40 // seg相对整体bin的offset
struct pfa_img_header_section {
    u8 rsv[2]; // rsv 2
    u8 load_type; // 0：上电加载+备份恢复；1：备份恢复
    u8 mem_type; // 0：pm内存；1：dm内存
    unsigned int sect_data_offset; // 相对data首地址的offset
    unsigned int dest_addr; // dm/pm 地址
    unsigned int size; // 4字节对齐
};

int pfa_load_img(struct pfa *pfa, char *data, unsigned int size);
int pfa_start_core(char *data, unsigned int size);
#endif /* __PFA_IMG_H__ */