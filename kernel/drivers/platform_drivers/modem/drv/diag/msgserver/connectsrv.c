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
#include <mdrv_diag.h>
#include <bsp_version.h>
#include <bsp_odt.h>
#include <bsp_diag_cmdid.h>
#include "srvchan.h"
#include "diagmsg.h"
#include "logreport.h"
#include "msgsrv.h"
#include "transfertask.h"
#include "diag_debug.h"
#include "encoder.h"
#include "connectsrv.h"


u32 g_toolcnt_state = TOOLCNT_STATE_DISCONNET;

static u32 g_cntauthstate = CNTSRV_AUTH_FAIL;

#define CONNSTATE_NOTIFIER_MAX 32
static mdrv_diag_conn_state_notify_fun g_cntstate_notifier[CONNSTATE_NOTIFIER_MAX];

bool is_tool_connected(void)
{
    return (g_toolcnt_state == TOOLCNT_STATE_CONNECT) ? \
        true : false;
}

u32 get_toolcnt_state(void)
{
    return g_toolcnt_state;
}

void set_toolcnt_state(u32 state)
{
    g_toolcnt_state = state;
}

u32 connectstate_notifer_register(mdrv_diag_conn_state_notify_fun func)
{
    u32 i;

    for (i = 0; i < CONNSTATE_NOTIFIER_MAX; i++) {
        if (g_cntstate_notifier[i] == NULL) {
            g_cntstate_notifier[i] = func;
            return 0;
        }
    }

    diag_err("connectstate notifier have no space\n");
    return ERR_MSP_FAILURE;
}

static void connectstate_changed(u32 state)
{
    u32 i;

    for (i = 0; i < CONNSTATE_NOTIFIER_MAX; i++) {
        if (g_cntstate_notifier[i] != NULL) {
            g_cntstate_notifier[i](state);
        }
    }
}

static void handle_connect_cmd(void)
{
    const bsp_version_info_s *verinfo = NULL;

    diag_err("receive tool connect cmd\n");

    diag_reportptr_clear();

    diag_dstmntn_clear();

    logreport_onoff_reset();

    set_toolcnt_state(TOOLCNT_STATE_CONNECT);

    connectstate_changed(DIAG_STATE_CONN);

    verinfo = bsp_get_version_info();
    if ((verinfo != NULL) && (verinfo->plat_type == PLAT_EMU)) {
        encoder_set_cntworkmode(ODT_IND_MODE_CYCLE);
        logreport_onoff_allopen();
    }

    bsp_diag_dpa_set_onoff(1);

    diag_srvptr_record(DIAG_SRVPTR_CONNECT_END, 0, 0);
}

u32 get_cnt_authstate(void)
{
    return g_cntauthstate;
}

static void clear_cnt_authstate(void)
{
    g_cntauthstate = CNTSRV_AUTH_FAIL;
}

static void handle_cntauth_cmd(u8 *data, u32 datasz)
{
    diag_err("receive tool auth cmd\n");

    if (datasz != sizeof(u32)) {
        diag_err("connect auth get invalid data\n");
        return;
    }

    g_cntauthstate = *data;

    if (g_cntauthstate == CNTSRV_AUTH_SUCCESS) {
        diag_err("connect auth success\n");
    }

    diag_srvptr_record(DIAG_SRVPTR_AUTH_END, 0, g_cntauthstate);
}

void handle_rxready_cmd(void)
{
    /* must ensure open log dst tranfer&overflow irq in rxready cmd for tool
     * log time sequence, otherwise log's time will be out of order in tool.
     */
    encoder_switch_cntworkmode();

    bsp_odt_data_send_manager(ODT_CODER_DST_OM_IND, ODT_DEST_DSM_ENABLE);
    diag_err("get rxready cmd and have open odt log dst chan\n");
}

void handle_disconnect_cmd(void)
{
    diag_err("receive tool disconnect cmd\n");

    set_toolcnt_state(TOOLCNT_STATE_DISCONNET);

    clear_cnt_authstate();

    logreport_onoff_reset();

    logreport_clear_sn();

    connectstate_changed(DIAG_STATE_DISCONN);

    bsp_odt_data_send_manager(ODT_CODER_DST_OM_IND, ODT_DEST_DSM_DISABLE);

    encoder_switch_cyclemode();

    bsp_diag_dpa_set_onoff(0);

    diag_srvptr_record(DIAG_SRVPTR_DISCONNECT_END, 0, 0);
}

void connectsrv_msg_handle(u32 msgid, u8 *data, u32 datasz)
{
    switch (msgid) {
        case TTMSG_CNTSRV_CONNECT:
            handle_connect_cmd();
            break;

        case TTMSG_CNTSRV_AUTHSTAT:
            handle_cntauth_cmd(data, datasz);
            break;

        case TTMSG_CNTSRV_RXREADY:
            handle_rxready_cmd();
            break;

        case TTMSG_CNTSRV_DISCONNECT:
            handle_disconnect_cmd();
            break;

        default:
            diag_err("recv invalid connect msgid:%d\n", msgid);
            break;
    }
}

struct disconnectmsg {
    struct oldmsghead msghd;
    struct srvhead srvhd;
};

static void trigger_disconnect_msg(void)
{
    u32 ret;
    struct ipc_ttmsg tspttmsg = {0};
    struct disconnectmsg dcmsg = {0};

    dcmsg.msghd.pyld.p_id = DIAG_CMD_HOST_DISCONNECT;
    dcmsg.msghd.pyld.p_len = sizeof(struct srvhead);

    tspttmsg.msgid = TTMSG_SERVER_REQUEST;
    tspttmsg.datasz = sizeof(struct oldmsghead);
    tspttmsg.datasz += dcmsg.msghd.pyld.p_len;

    diag_err("receive port disconnect, send disconnect msg to tsp\n");
    ret = send_ttmsg_to_tsp(&tspttmsg, (u8 *)&dcmsg, tspttmsg.datasz);
    if (ret) {
        diag_err("transfer disconnect msg to tsp fail:%d\n", ret);
    }
}

u32 connectsrv_init(void)
{
    register_srvport_disconnectcb(trigger_disconnect_msg);

    return 0;
}

