/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef __CON_VIRTSHELL_H_
#define __CON_VIRTSHELL_H_

#include <linux/kfifo.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <osl_spinlock.h>
#include <osl_list.h>
#include <bsp_print.h>

#define VIRTSHL_MAX_PACKAGE_LEN 256
#define LOG_BUF_SHIFT 14
#define CMD_BUF_SHIFT 8
#define cshell_port_info(fmt, ...) (bsp_info("%s " fmt, __FUNCTION__, ##__VA_ARGS__))
#define cshell_port_err(fmt, ...) (bsp_err("%s " fmt, __FUNCTION__, ##__VA_ARGS__))


struct cshell_info {
    struct kfifo *cmdbuf;
    struct kfifo *logbuf;
    struct semaphore send_tsk_sem;
    OSL_TASK_ID send_tsk_id;
    void (*start_tx)(struct cshell_info *con);
    int (*start_shell)(struct cshell_info *con);
    int (*logbuff_out)(struct cshell_info *con, unsigned char *outbuf, unsigned int size);
    int (*cmdbuff_in)(struct cshell_info *con, unsigned char *inbuf, unsigned int size);
    void (*logbuff_in)(struct cshell_info *con, unsigned char *inbuf, unsigned int size);
    int (*cmdbuff_out)(struct cshell_info *con, unsigned char *outbuf, unsigned int size);
};

extern struct cshell_info g_cshell_ap;
void cshell_starttx(struct cshell_info *cshell);
int virtshl_init(void);
int chsell_send_to_tsp(void *data, u32 len);
int virtshl_start_shell(struct cshell_info *cshell);

#endif
