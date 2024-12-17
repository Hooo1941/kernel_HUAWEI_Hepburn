/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <osl_types.h>
#include <bsp_reset.h>
#include <bsp_diag_cmdid.h>
#include "connectsrv.h"
#include "diagmsg.h"
#include "diag_debug.h"
#include "msgsrv.h"


static u32 g_srvmsg_sn = 0;

static u32 pack_srvmsg(struct srvmsg *srvmsg)
{
    u32 ret;
    struct packmsg packmsg = {0};

    packmsg.mdmid = 0;
    packmsg.srcsn = g_srvmsg_sn++;
    packmsg.msgtype = srvmsg->type;
    packmsg.encsrch = 0xFF;
    packmsg.payloadid = srvmsg->msgid;
    packmsg.datahd = NULL;
    packmsg.datahdsz = 0;
    packmsg.data = srvmsg->data;
    packmsg.datasz = srvmsg->datasz;
    ret = pack_diagmsg(&packmsg);
    if (ret) {
        diag_err("pack srv msg fail:%d\n", ret);
    }

    return ret;
}

u32 srvmsg_response(u32 msgid, struct srvhead *srvhd, u32 result)
{
    u32 ret;
    struct srvresponse srvrsp = {0};
    struct srvmsg srvmsg = {0};

    srvrsp.auid = srvhd->auid;
    srvrsp.sn = srvhd->sn;
    srvrsp.result = result;

    srvmsg.msgid = msgid;
    srvmsg.type = DIAG_MSG_TYPE_CNF;
    srvmsg.datasz = sizeof(struct srvresponse);
    srvmsg.data = (u8 *)&srvrsp;
    ret = pack_srvmsg(&srvmsg);
    if (ret) {
        diag_err("srvmsg response pack srv msg fail:%d\n", ret);
    }

    return ret;
}

int diag_modem_reset(drv_reset_cb_moment_e event, int userdata)
{
    struct srvhead srvhd = {0};

    (void)srvmsg_response(DIAG_CMD_MODEM_WILL_RESET, &srvhd, 0);

    if (event == MDRV_RESET_CB_BEFORE) {
        handle_disconnect_cmd();
    }

    return 0;
}

u32 msgsrv_init(void)
{
    u32 ret;

    ret = connectsrv_init();
    if (ret) {
        diag_err("connect srv init fail:%d\n", ret);
        return ret;
    }

    ret = mdrv_sysboot_register_reset_notify("DIAG", diag_modem_reset, 0, DRV_RESET_CB_PIOR_DIAG);
    if (ret) {
        diag_err("diag reset handle register fail:%d\n", ret);
    }

    return ret;
}

