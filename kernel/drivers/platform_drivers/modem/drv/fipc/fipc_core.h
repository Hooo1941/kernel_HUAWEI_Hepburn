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

#ifndef _FIPC_CORE_H
#define _FIPC_CORE_H

#include "fipc_platform.h"
#include "fipc_device.h"
#include "fipc_driver.h"
#define FIPC_GET_DEV_ID(drv_chn_id) ((drv_chn_id) >> 28)
#define FIPC_GET_PIPE_ID(drv_chn_id) (((drv_chn_id) >> 16) & 0xfff)
#define FIPC_GET_CORE_ID(drv_chn_id) (((drv_chn_id) >> 8) & 0xff)
#define FIPC_GET_RCORE_ID(drv_chn_id) ((drv_chn_id)&0xff)

#define FIPC_GET_PHYCHN_ID(x_chn_id) ((x_chn_id) >> 16)
#define GET_DEV_ID(phy_chn_id) ((phy_chn_id) >> 12)
#define GET_PIPE_ID(phy_chn_id) ((phy_chn_id)&0xFFF)

#define FIPC_PHYCHNID_MK(dev_id, pipe_id) (((dev_id) << 12) | (pipe_id))

#define FIPC_ERR_ESRCH (-3)             /* No such process */
#define FIPC_ERR_EIO (-5)               /* Input/output error */
#define FIPC_ERR_ENXIO (-6)             /* No such device or address */
#define FIPC_ERR_ENOMEM (-12)           /* Cannot allocate memory */
#define FIPC_ERR_EEXIST (-17)           /* File exists */
#define FIPC_ERR_EINVAL (-22)           /* Invalid argument */
#define FIPC_ERR_ECHRNG (-44)           /* Channel number out of range */
#define FIPC_ERR_ETIME (-62)            /* Timer expired */
#define FIPC_ERR_EPROTO (-71)           /* Protocol error */
#define FIPC_ERR_EBADMSG (-74)          /* Bad message */
#define FIPC_ERR_EBADFD (-77)           /* File descriptor in bad state */
#define FIPC_ERR_ENETUNREACH (-101)     /* Network is unreachable */
#define FIPC_ERR_EHOSTUNREACH (-113)    /* No route to host */
#define FIPC_ERR_ENOTRECOVERABLE (-131) /* State not recoverable */
#define FIPC_ERR_PM_OPIPE_NOT_IDLE (-512)
#define FIPC_ERR_PM_IPIPE_NOT_IDLE (-513)
#define FIPC_ERR_PM_IPIPE_NEG_REFUSE (-514)
#define FIPC_ERR_PM_IPIPE_NEG_TIMEOUT (-515)
#define FIPC_ERR_PM_IPIPE_NEG_FAIL (-516)
#define FIPC_ERR_PM_OPIPE_NOT_EMPTY (-517)
#define FIPC_ERR_PM_IPIPE_NOT_EMPTY (-518)
#define FIPC_ERR_HALT_QUERY_WAIT_TIMEOUT (-519)

struct fipc_send_helper {
    u32 channel_id;
    void *send_desc;
    u32 send_flags;
    u32 total_msglen;
    u32 head_len;
    u32 write_len;
    u32 space_left;
    u32 wptr;
};
struct fipc_recv_helper {
    u32 channel_id;
    void *buf;
    unsigned len;
    u32 recv_flags;
    u32 used_len;
    u32 rptr;
    u32 seek;
    u32 payloadskip_len;
};

int fipc_normal_event_notify(u32 inttype, struct fipc_device *pdev, u32 pipe_id);
int fipc_wakeup_event_notify(struct fipc_device *pdev, u32 pipe_id);
void fipc_opipe_enable(struct fipc_device *pdev, struct fipc_channel *pchannel);
void fipc_ipipe_enable(struct fipc_device *pdev, struct fipc_channel *pchannel);
int fipc_channel_open(struct fipc_device *pdev, struct fipc_channel *pchannel, struct compat_attr *pcompat_attr);


#endif /* FIPC_CORE_H */
