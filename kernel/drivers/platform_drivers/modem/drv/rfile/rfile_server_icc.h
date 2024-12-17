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

#ifndef __RFILE_SERVER_ICC_H__
#define __RFILE_SERVER_ICC_H__

#define RFILE_MAX_SEND_TIMES 10 /* 发送失败时，最大尝试次数 */

#include "bsp_fipc.h"
#define RFILE_FIPC_PAYLOAD 0x80
#define RFILE_CHANNAL_FULL FIPC_ERR_EAGAIN
/* RFILE FIPC通道最大64K，寄存器最大只能表示0xffff，FIPC深度有128字节对齐要求，所以通道最大为0xff80 */
#define RFILE_FIPC_CHN_MAX (64 * 1024 - 128)

#define RFILE_USER_MAX_BUFSIZE (64 * 1024)

#define RFILE_LEN_MAX_CTOA_PHONE (RFILE_FIPC_CHN_MAX - RFILE_FIPC_PAYLOAD)
#define RFILE_LEN_MAX_ATOC_PHONE (RFILE_FIPC_CHN_MAX - RFILE_FIPC_PAYLOAD)

#define RFILE_LEN_MAX_CTOA_MBB (32 * 1024 - RFILE_FIPC_PAYLOAD)
#define RFILE_LEN_MAX_ATOC_MBB (32 * 1024 - RFILE_FIPC_PAYLOAD)

void rfile_icc_common_cb(void);
int rfile_icc_common_init(void);
void rfile_icc_common_exit(void);
int rfile_icc_common_recv(void *databuf, u32 datalen);
int rfile_icc_common_send(void *databuf, u32 datalen);
u32 rfile_icc_chn_len_max_atoc(void);
u32 rfile_icc_chn_len_max_ctoa(void);
#endif /* end of rfile_server_icc.h */
