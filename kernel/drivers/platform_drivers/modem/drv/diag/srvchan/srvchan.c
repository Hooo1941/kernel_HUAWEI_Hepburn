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
#include <osl_sem.h>
#include <mdrv_diag.h>
#include <bsp_version.h>
#include <bsp_homi.h>
#include <bsp_odt.h>
#include "diag_debug.h"
#include "encoder.h"
#include "srvchan.h"
#include "nlsrvport.h"


#define SRVPORT_USB_LOGBUFSIZE (2 * 1024)
#define SRVPORT_USB_LOGBUFCNT (2)
#define SRVPORT_USB_CNFBUFSIZE (8 * 1024)
#define SRVPORT_USB_CNFBUFCNT (8)

#define SRVPORT_SOCK_BUFSIZE (1000)

static struct srvchan g_srvchan;

void register_srvpoint_sendcb(srvpoint_sendcb sendcb)
{
    g_srvchan.sendcb = sendcb;
}

void register_srvpoint_recvcb(srvpoint_recvcb recvcb)
{
    g_srvchan.recvcb = recvcb;
}

void register_srvport_disconnectcb(srvport_disconnectcb disconnectcb)
{
    g_srvchan.disconnectcb = disconnectcb;
}

void srvport_disconnect(void)
{
    if (g_srvchan.disconnectcb != NULL) {
        g_srvchan.disconnectcb();
    }
}

static void cnf_srvpoint_sendcb(void *data, int len)
{
    if (g_srvchan.sendcb != NULL) {
        g_srvchan.sendcb(SRVPOINT_CNF, len);
    }
}

static void log_srvpoint_sendcb(void *data, int len)
{
    if (g_srvchan.sendcb != NULL) {
        g_srvchan.sendcb(SRVPOINT_LOG, len);
    }
}

void srvpoint_data_recv(struct homi_chan *chan, void *data, int len)
{
    struct srvchan *srvchan = &g_srvchan;

    osl_sem_down(&srvchan->tpsem);
    if (srvchan->curtype != chan->port->type) {
        diag_err("receive invalid %d port data, curtype:%d\n", chan->port->type, srvchan->curtype);
        osl_sem_up(&srvchan->tpsem);
        return;
    }
    osl_sem_up(&srvchan->tpsem);

    if (g_srvchan.recvcb != NULL) {
        g_srvchan.recvcb((u8 *)data, len);
    }
}

static u32 convert_srvport_type(u32 type)
{
    switch (type) {
        case HOMI_PORT_USB:
            return SRVPORT_USB;
        case HOMI_PORT_NETLINK:
            return SRVPORT_NETLINK;
        case HOMI_PORT_SOCKET:
            return SRVPORT_SOCK;
        default:
            return SRVPORT_BUTT;
    }
}

static u32 convert_srvport_state(u32 state)
{
    switch (state) {
        case HOMI_CHSTATE_DISCONNECTED:
        case HOMI_CHSTATE_SUSPEND:
        case HOMI_CHSTATE_REMOVED:
            return SRVPORT_STATE_DISCONNECT;
        case HOMI_CHSTATE_CONNECTED:
            return SRVPORT_STATE_CONNECT;
        default:
            return SRVPORT_STATE_INVALID;
    }
}

static void nortify_srvport_event(enum homi_port_type type, enum homi_chan_state state)
{
    struct srvchan *srvchan = &g_srvchan;
    u32 p_type = convert_srvport_type(type);
    u32 p_state = convert_srvport_state(state);
    struct srvport *srvport = NULL;

    if ((p_type >= SRVPORT_BUTT) || (p_state == SRVPORT_STATE_INVALID)) {
        diag_err("port type(%d) get invalid event or state\n", type);
        return;
    }

    srvport = &srvchan->port[p_type];

    osl_sem_down(&srvport->stsem);
    srvport->state = p_state;
    osl_sem_up(&srvport->stsem);

    osl_sem_down(&srvchan->tpsem);
    if (p_type != srvchan->curtype) {
        diag_err("srvchan port curtype is %d not %d\n", srvchan->curtype, p_type);
        goto uptpsem;
    }

    osl_sem_down(&srvchan->stsem);
    if (p_state == srvchan->curstate) {
        diag_err("port type:%d get same state:%d(0:disconnect,1:connect).\n", p_type, srvchan->curstate);
        goto upstsem;
    }

    srvchan->curstate = p_state;

    if (p_state == SRVPORT_STATE_DISCONNECT) {
        srvport_disconnect();
    }

    diag_err("port type:%d state change to %d(0:disconnect,1:connect)\n", p_type, p_state);

upstsem:
    osl_sem_up(&srvchan->stsem);
uptpsem:
    osl_sem_up(&srvchan->tpsem);
}

/* until now only usb use this notify port event to handle lost send callback */
void nortify_cnfsrvport_event(enum homi_port_type type, enum homi_chan_state state)
{
    struct srvchan *srvchan = &g_srvchan;
    u32 p_type = convert_srvport_type(type);
    u32 p_state = convert_srvport_state(state);

    nortify_srvport_event(type, state);

    osl_sem_down(&srvchan->tpsem);
    if ((p_type == SRVPORT_USB) && (p_type == srvchan->curtype) && (p_state == SRVPORT_STATE_CONNECT)) {
        encoder_reset_cnf_dstch();
        /* usb connect will use direct mode, no other mode */
        encoder_set_cntworkmode(ODT_IND_MODE_DIRECT);
    }
    osl_sem_up(&srvchan->tpsem);
}

u32 get_srvport_curtype(void)
{
    u32 curtype;
    struct srvchan *srvchan = &g_srvchan;

    osl_sem_down(&srvchan->tpsem);
    curtype = srvchan->curtype;
    osl_sem_up(&srvchan->tpsem);

    return curtype;
}

u32 srvpoint_data_send(u32 port, u32 point, u8 *data, u8 *phyaddr, int len)
{
    u32 curstate;
    struct srvchan *srvchan = &g_srvchan;
    struct srvport *srvport = &srvchan->port[port];
    struct srvpoint *srvpoint = NULL;

    osl_sem_down(&srvchan->stsem);
    curstate = srvchan->curstate;
    osl_sem_up(&srvchan->stsem);
    if (curstate == SRVPORT_STATE_DISCONNECT) {
        diag_err("srvchan %d not in connect, send data fail\n", port);
        return DIAG_SRVPORT_NOTCONNECT_ERR;
    }

    if ((point >= SRVPOINT_BUTT) || (srvport->point[point].epoint == NULL)) {
        diag_err("srvpoint send data get null port:%d, point:%d\n", port, point);
        return DIAG_SRVPORT_GET_POINT_ERR;
    }

    srvpoint = &srvport->point[point];

    if (port == SRVPORT_USB) {
        srvpoint->epoint->private = (void *)phyaddr;
    }

    return srvpoint->epoint->ops->send(srvpoint->epoint, (void *)data, len);
}

u32 srvport_data_send(u32 srvpoint, u8 *data, u8 *phyaddr, int len)
{
    u32 curtype;

    curtype = get_srvport_curtype();
    if (curtype == SRVPORT_NETLINK) {
        return nlsrvport_data_send(srvpoint, data, len);
    }

    return srvpoint_data_send(curtype, srvpoint, data, phyaddr, len);
}

static bool is_need_switch_srvport(struct srvchan *srvchan, u32 type)
{
    if (srvchan->curtype == type) {
        diag_err("switch same port type:%d(0:USB,1:NETLINK,2:SOCK)\n", type);
        return false;
    }

    if ((srvchan->curtype == SRVPORT_USB) && (mdrv_diag_get_conn_state() == 1)) {
        diag_err("srvport use usb port and in connect state, can't switch srvport\n");
        return false;
    }

    return true;
}

u32 switch_srvport_type(u32 type)
{
    struct srvchan *srvchan = &g_srvchan;
    struct srvport *srvport = NULL;

    if (type >= SRVPORT_BUTT) {
        diag_err("srvport switch invalid %d type\n", type);
        return DIAG_SRVPORT_PORT_TYPE_ERR;
    }

    srvport = &srvchan->port[type];

    if ((srvport->point[SRVPOINT_CNF].epoint == NULL) \
        || (srvport->point[SRVPOINT_LOG].epoint == NULL)) {
        diag_err("srvport switch %d port have invalid point\n", type);
        return DIAG_SRVPORT_GET_POINT_ERR;
    }

    osl_sem_down(&srvchan->tpsem);
    osl_sem_down(&srvchan->stsem);

    if (!is_need_switch_srvport(srvchan, type)) {
        goto upsem;
    }

    /* need trigger disconnect if old state is connect state */
    if (srvchan->curstate == SRVPORT_STATE_CONNECT) {
        srvport_disconnect();
    }

    srvchan->curtype = type;

    osl_sem_down(&srvport->stsem);
    srvchan->curstate = srvport->state;
    osl_sem_up(&srvport->stsem);

    diag_err("srvport switch to %d(0:USB,1:NETLINK,2:SOCK) port\n", type);

upsem:
    osl_sem_up(&srvchan->stsem);
    osl_sem_up(&srvchan->tpsem);
    return 0;
}

static u32 srvport_usb_init(struct srvport *srvport)
{
    struct srvpoint *srvpoint = NULL;

    srvport->type = SRVPORT_USB;
    srvport->state = SRVPORT_STATE_DISCONNECT;
    osl_sem_init(1, &srvport->stsem);

    srvpoint = &srvport->point[SRVPOINT_CNF];
    srvpoint->cfg.ptype = HOMI_PORT_USB;
    srvpoint->cfg.ctype = HOMI_CH_USB_GPS;
    srvpoint->cfg.blength = SRVPORT_USB_CNFBUFSIZE;
    srvpoint->cfg.bnum = SRVPORT_USB_CNFBUFCNT;
    srvpoint->cfg.cb.sendcb = cnf_srvpoint_sendcb;
    srvpoint->cfg.cb.recvcb = srvpoint_data_recv;
    srvpoint->cfg.cb.eventcb = nortify_cnfsrvport_event;
    srvpoint->epoint = bsp_homi_chan_alloc(&srvpoint->cfg);
    if (srvpoint->epoint == NULL) {
        diag_err("alloc usb port cnf epoint fail\n");
        return DIAG_SRVPORT_ALLOC_CHAN_ERR;
    }

    srvpoint = &srvport->point[SRVPOINT_LOG];
    srvpoint->cfg.ptype = HOMI_PORT_USB;
    srvpoint->cfg.ctype = HOMI_CH_USB_4G;
    srvpoint->cfg.blength = SRVPORT_USB_LOGBUFSIZE;
    srvpoint->cfg.bnum = SRVPORT_USB_LOGBUFCNT;
    srvpoint->cfg.cb.sendcb = log_srvpoint_sendcb;
    srvpoint->cfg.cb.eventcb = nortify_srvport_event;
    srvpoint->epoint = bsp_homi_chan_alloc(&srvpoint->cfg);
    if (srvpoint->epoint == NULL) {
        diag_err("alloc usb port log epoint fail\n");
        return DIAG_SRVPORT_ALLOC_CHAN_ERR;
    }

    return 0;
}

#ifdef CONFIG_DIAG_NETLINK
u32 srvport_netlink_init(struct srvport *srvport)
{
    struct srvpoint *srvpoint = NULL;

    srvport->type = SRVPORT_NETLINK;
    srvport->state = SRVPORT_STATE_DISCONNECT;
    osl_sem_init(1, &srvport->stsem);

    srvpoint = &srvport->point[SRVPOINT_CNF];
    srvpoint->cfg.ptype = HOMI_PORT_NETLINK;
    srvpoint->cfg.ctype = HOMI_CH_NL_31;
    srvpoint->cfg.cb.recvcb = nlsrvpoint_data_recv;
    srvpoint->cfg.cb.eventcb = nortify_srvport_event;
    srvpoint->epoint = bsp_homi_chan_alloc(&srvpoint->cfg);
    if (srvpoint->epoint == NULL) {
        diag_err("alloc netlink port cnf epoint fail\n");
        return DIAG_SRVPORT_ALLOC_CHAN_ERR;
    }

    srvpoint = &srvport->point[SRVPOINT_LOG];
    srvpoint->cfg.ptype = HOMI_PORT_NETLINK;
    srvpoint->cfg.ctype = HOMI_CH_NL_29;
    srvpoint->cfg.cb.eventcb = nortify_srvport_event;
    srvpoint->epoint = bsp_homi_chan_alloc(&srvpoint->cfg);
    if (srvpoint->epoint == NULL) {
        diag_err("alloc netlink port log epoint fail\n");
        return DIAG_SRVPORT_ALLOC_CHAN_ERR;
    }

    return 0;
}
#endif


u32 srvchan_init(void)
{
    u32 ret;
    struct srvchan *srvchan = &g_srvchan;
    const bsp_version_info_s *ver = bsp_get_version_info();

    srvchan->curtype = SRVPORT_USB;
    srvchan->curstate = SRVPORT_STATE_DISCONNECT;
    osl_sem_init(1, &srvchan->tpsem);
    osl_sem_init(1, &srvchan->stsem);

    if ((ver != NULL) && (ver->plat_type == PLAT_HYBRID)) {
        srvchan->curtype = SRVPORT_SOCK;
    }

    ret = srvport_usb_init(&srvchan->port[SRVPORT_USB]);
    if (ret) {
        diag_err("srvchan usb port init fail:0x%x\n", ret);
        return ret;
    }

#ifdef CONFIG_DIAG_NETLINK
    ret = nlsrvport_init();
    if (ret) {
        diag_err("nlsrvport init fail:0x%x\n", ret);
        return ret;
    }

    ret = srvport_netlink_init(&srvchan->port[SRVPORT_NETLINK]);
    if (ret) {
        diag_err("srvchan netlink port init fail:0x%x\n", ret);
        return ret;
    }
#endif


    diag_err("srvport init success\n");
    return 0;
}

