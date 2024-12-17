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
#include <securec.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/timer.h>
#include <bsp_odt.h>
#include <bsp_homi.h>
#include "diag_debug.h"
#include "encoder.h"
#include "srvchan.h"
#include "nlsrvport.h"


#define NLSRVPORT_PM_TIMEOUT 120

struct nlsrvport_pm {
    struct wakeup_source *wakelock;
    struct timer_list timer;
};

struct nlsrvport_pm g_nlsrvport_pm = {0};

void nlsrvport_pm_awake(void)
{
    struct nlsrvport_pm *nlportpm = &g_nlsrvport_pm;

    __pm_stay_awake(nlportpm->wakelock);
    mod_timer(&nlportpm->timer, jiffies + NLSRVPORT_PM_TIMEOUT * HZ);
}

void nlsrvport_pm_relax(void)
{
    struct nlsrvport_pm *nlportpm = &g_nlsrvport_pm;

    del_timer(&nlportpm->timer);
    __pm_relax(nlportpm->wakelock);
}

static void nlsrvpoint_cmd_response(u8 *data, u32 len)
{
    int ret;
    int curtype = get_srvport_curtype();

    if (curtype != SRVPORT_NETLINK) {
        diag_err("srvchan not use netlink port, curtype:%d\n", curtype);
        return;
    }

    ret = srvpoint_data_send(SRVPORT_NETLINK, SRVPOINT_CNF, data, NULL, len);
    if (ret) {
        diag_err("nlsrvpoint cmd response fail:%d\n", ret);
    }
}

static void nlsrvpoint_bind_port(struct srvchan_nlmsg *nlmsg)
{
    /* response modemlogcat netlink message */
    struct srvchan_nlmsg respnlmsg = {0};

    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_CMD;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

static void nlsrvpoint_unbind_port(struct srvchan_nlmsg *nlmsg)
{
    struct srvchan_nlmsg respnlmsg = {0};

    srvport_disconnect();

    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_CMD;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

static void nlsrvpoint_switch_port(struct srvchan_nlmsg *nlmsg)
{
    int ret;
    int ptype;
    struct srvchan_nlmsg respnlmsg = {0};

    ptype = (nlmsg->log_port_config.port == 0) ? SRVPORT_USB : SRVPORT_NETLINK;

    ret = switch_srvport_type(ptype);
    if (ret) {
        diag_err("nlsrvpoint switch port fail:%d\n", ret);
    }

    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_CMD;
    respnlmsg.result = ret;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

static void nlsrvpoint_get_curporttype(struct srvchan_nlmsg *nlmsg)
{
    struct srvchan_nlmsg respnlmsg = {0};
    int curtype = get_srvport_curtype();

    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_DATA;
    respnlmsg.args_length = sizeof(respnlmsg.log_port_config);
    respnlmsg.log_port_config.port = (curtype == SRVPORT_USB) ? 0 : 1;
    respnlmsg.result = 0;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

static void nlsrvpoint_set_logmode(struct srvchan_nlmsg *nlmsg)
{
    int ret = 0;
    int curtype;
    struct srvchan_nlmsg respnlmsg = {0};

    curtype = get_srvport_curtype();
    if (curtype != SRVPORT_NETLINK) {
        diag_err("srvport not netlink, can't set log mode\n");
        ret = DIAG_SRVPORT_SETLOGMODE_ERR;
        goto response;
    }

    encoder_set_cntworkmode(nlmsg->log_config.mode);

    encoder_switch_cntworkmode();

response:
    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_CMD;
    respnlmsg.result = ret;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

static void nlsrvpoint_get_logmode(struct srvchan_nlmsg *nlmsg)
{
    u32 mode;
    struct srvchan_nlmsg respnlmsg = {0};

    mode = bsp_odt_get_ind_chan_mode();

    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_DATA;
    respnlmsg.args_length = sizeof(respnlmsg.log_config);
    respnlmsg.log_config.mode = mode;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

static void nlsrvpoint_handle_cmd(struct homi_chan *chan, struct srvchan_nlmsg *nlmsg, u32 msglen)
{
    int ret = 0;
    u32 length;
    struct srvchan_nlmsg respnlmsg = {0};

    length = nlmsg->args_length - sizeof(nlmsg->om_command);
    if ((length < nlmsg->om_command.length) || (nlmsg->om_command.length > \
        (msglen - sizeof(struct srvchan_nlmsg)))) {
        ret = DIAG_SRVPORT_NETLINK_DATA_ERR;
        goto response;
    }

    /* need send netlink recv data to decode */
    srvpoint_data_recv(chan, (void *)nlmsg->om_command.data, nlmsg->om_command.length);

response:
    if (ret) {
        respnlmsg.msg_type = nlmsg->msg_type;
        respnlmsg.op = SRVCHAN_NLMSG_CMD;
        respnlmsg.result = ret;
        nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
    }
}

static void nlsrvpoint_handle_invalidcmd(struct srvchan_nlmsg *nlmsg)
{
    struct srvchan_nlmsg respnlmsg = {0};

    respnlmsg.msg_type = nlmsg->msg_type;
    respnlmsg.op = SRVCHAN_NLMSG_CMD;
    respnlmsg.result = DIAG_SRVPORT_NETLINK_CMD_ERR;

    nlsrvpoint_cmd_response((u8 *)&respnlmsg, (u32)sizeof(struct srvchan_nlmsg));
}

void nlsrvpoint_data_recv(struct homi_chan *chan, void *data, int len)
{
    struct srvchan_nlmsg *nlmsg = (struct srvchan_nlmsg *)data;

    if (len < (int)sizeof(struct srvchan_nlmsg)) {
        diag_err("nlsrvpoint get invalid nlmsg len:0x%x\n", len);
        return;
    }

    switch (nlmsg->msg_type) {
        case SRVCHAN_NLBIND_PORT:
            nlsrvpoint_bind_port(nlmsg);
            break;

        case SRVCHAN_NLUNBIND_PORT:
            nlsrvpoint_unbind_port(nlmsg);
            break;

        case SRVCHAN_NLSWITCH_PORT:
            nlsrvpoint_switch_port(nlmsg);
            break;

        case SRVCHAN_NLGET_PORT_TYPE:
            nlsrvpoint_get_curporttype(nlmsg);
            break;

        case SRVCHAN_NLSET_LOG_MODE:
            nlsrvpoint_set_logmode(nlmsg);
            break;

        case SRVCHAN_NLGET_LOG_MODE:
            nlsrvpoint_get_logmode(nlmsg);
            break;

        case SRVCHAN_NLHANDLE_CMD:
            nlsrvpoint_handle_cmd(chan, nlmsg, len);
            break;

        default:
            diag_err("nlsrvpoint get invalid msg type:%d\n", nlmsg->msg_type);
            nlsrvpoint_handle_invalidcmd(nlmsg);
            break;
    }

    diag_err("nlsrvpoint %d msg handle end\n", nlmsg->msg_type);
}

static int log_nlsrvpoint_send(u32 point, u8 *data, u32 len)
{
    int ret = 0;
    u32 curblk = 0;
    u8 *newdata = NULL;
    u8 *tmpdata = data;
    u32 blknum;
    u32 sendlen;
    struct srvchan_nllog *nllog = NULL;

    blknum = len / SRVCHAN_NLLOG_SIZE;
    blknum = (len % SRVCHAN_NLLOG_SIZE) ? (blknum + 1) : blknum;

    newdata = (u8 *)osl_malloc(sizeof(struct srvchan_nllog) + SRVCHAN_NLLOG_SIZE);
    if (newdata == NULL) {
        diag_err("log nlsrvpoint data send malloc data fail\n");
        return DIAG_SRVPORT_NLDATA_MALLOC_ERR;
    }

    nlsrvport_pm_awake();

    while (len) {
        (void)memset_s(newdata, (sizeof(struct srvchan_nllog) + SRVCHAN_NLLOG_SIZE), \
            0, (sizeof(struct srvchan_nllog) + SRVCHAN_NLLOG_SIZE));

        if (len > SRVCHAN_NLLOG_SIZE) {
            sendlen = SRVCHAN_NLLOG_SIZE;
        } else {
            sendlen = len;
        }

        nllog = (struct srvchan_nllog *)newdata;
        nllog->msg_type = SRVCHAN_NLHANDLE_DATA;
        nllog->op = SRVCHAN_NLMSG_DATA;
        nllog->args_length = sizeof(nllog->om_data) + sendlen;
        nllog->om_data.blk_num = blknum;
        nllog->om_data.blk_seq = curblk;
        nllog->om_data.length = sendlen;

        if (memcpy_s(nllog->om_data.data, nllog->om_data.length, tmpdata, sendlen)) {
            diag_err("log nlsrvpoint data send memcpy fail\nl");
            ret = DIAG_SRVPORT_MEMCPY_ERR;
            break;
        }

        ret = srvpoint_data_send(SRVPORT_NETLINK, point, newdata, NULL, (sizeof(struct srvchan_nllog) + sendlen));
        if (ret) {
            break;
        }

        tmpdata += sendlen;
        len -= sendlen;
        curblk++;
    }

    nlsrvport_pm_relax();

    osl_free(newdata);

    return ret;
}

static int cnf_nlsrvpoint_send(u32 point, u8 *data, u32 len)
{
    int ret;
    u8 *newdata;
    struct srvchan_nlmsg *nlmsg = NULL;

    newdata = (u8 *)osl_malloc(sizeof(struct srvchan_nlmsg) + len);
    if (newdata == NULL) {
        diag_err("cnf nlsrvpoint data send malloc data fail\n");
        return DIAG_SRVPORT_NLDATA_MALLOC_ERR;
    }

    nlsrvport_pm_awake();

    (void)memset_s(newdata, (sizeof(struct srvchan_nlmsg) + len), 0, (sizeof(struct srvchan_nlmsg) + len));

    nlmsg = (struct srvchan_nlmsg *)newdata;
    nlmsg->msg_type = SRVCHAN_NLHANDLE_CMD;
    nlmsg->op = SRVCHAN_NLMSG_DATA;
    nlmsg->args_length = sizeof(nlmsg->om_command) + len;
    nlmsg->om_command.length = len;

    if (memcpy_s(nlmsg->om_command.data, nlmsg->om_command.length, data, len)) {
        osl_free(newdata);
        diag_err("cnf nlsrvpoint data send memcpy fail\n");
        nlsrvport_pm_relax();
        return DIAG_SRVPORT_MEMCPY_ERR;
    }

    ret = srvpoint_data_send(SRVPORT_NETLINK, point, newdata, NULL, (sizeof(struct srvchan_nlmsg) + len));
    if (ret) {
        diag_err("cnf nlsrvpoint data send fail:%d\n", ret);
    }

    nlsrvport_pm_relax();

    osl_free(newdata);

    return ret;
}

u32 nlsrvport_data_send(u32 srvpoint, u8 *data, int len)
{
    int ret;

    if (srvpoint == SRVPOINT_CNF) {
        ret = cnf_nlsrvpoint_send(srvpoint, data, len);
    } else {
        ret = log_nlsrvpoint_send(srvpoint, data, len);
    }

    if (ret == -ECONNREFUSED) {
        diag_err("user apk exit, disconnect\n");
        srvport_disconnect();
    } else if (ret < 0) {
        diag_err("nlsrvpoint data send fail:%d\n", ret);
        return (u32)ret;        
    }

    return 0;
}

void nlsrvport_pm_timer(struct timer_list *timer)
{
    struct nlsrvport_pm *nlportpm = &g_nlsrvport_pm;

    __pm_relax(nlportpm->wakelock);

    diag_err("nlsrvport wakelock timer expire, have relax netlink wakelock\n");
}

u32 nlsrvport_init(void)
{
    struct nlsrvport_pm *nlportpm = &g_nlsrvport_pm;

    nlportpm->wakelock = wakeup_source_register(NULL, "nlsrvport");
    if (nlportpm->wakelock == NULL) {
        diag_err("diag nlsrvport wakelock register fail\n");
        return DIAG_SRVPORT_NLWALELOCK_INIT_ERR;
    }

    timer_setup(&nlportpm->timer, nlsrvport_pm_timer, 0);

    return 0;
}

