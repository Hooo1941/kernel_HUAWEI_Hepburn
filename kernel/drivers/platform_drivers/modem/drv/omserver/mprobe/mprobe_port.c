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

#include <securec.h>
#include <osl_malloc.h>
#include <osl_sem.h>
#include <bsp_version.h>
#include <bsp_homi.h>
#include <bsp_odt.h>
#include "mprobe_port.h"
#include "mprobe_channel_manager.h"
#include "mprobe_debug.h"
#include "mprobe_service.h"

#undef THIS_MODU
#define THIS_MODU mod_mprobe

struct mprobe_port_ctrl *g_mpctrl = NULL;

struct mprobe_port_ctrl *mprobe_get_dpctrl(void)
{
    return g_mpctrl;
}

struct mprobe_port *mprobe_port_get(struct mprobe_port_ctrl *dpctrl, enum homi_port_type ptype)
{
    struct mprobe_port *dport = NULL;

    switch (ptype) {
        case HOMI_PORT_USB:
            dport = &dpctrl->usbport;
            break;
        default:
            mprobe_error("use invalid %d type get mprobe port\n", ptype);
            break;
    }

    if (dport->chanel == NULL) {
        return NULL;
    }

    return dport;
}

void mprobe_get_data_len(u32 data_len, u32 *send_len)
{
    u32 curtype;
    u32 length = data_len;
    struct mprobe_port_ctrl *dpctrl = mprobe_get_dpctrl();

    if (dpctrl == NULL) {
        mprobe_error("mprobe port init fail\n");
        *send_len = 0;
        return;
    }

    osl_sem_down(&dpctrl->tpsem);
    curtype = dpctrl->curtype;
    osl_sem_up(&dpctrl->tpsem);

    if ((curtype == HOMI_PORT_USB) && (data_len > USB_MAX_DATA_LEN)) {
        length = USB_MAX_DATA_LEN;
    }

    *send_len = length;
    return;
}

static int mprobe_port_state(enum homi_chan_state state)
{
    int dpstate;

    switch (state) {
        case HOMI_CHSTATE_DISCONNECTED:
        case HOMI_CHSTATE_SUSPEND:
        case HOMI_CHSTATE_REMOVED:
            dpstate = MPROBE_DISCONNECTED_STATE;
            break;
        case HOMI_CHSTATE_CONNECTED:
            dpstate = MPROBE_CONNECTED_STATE;
            break;
        default:
            dpstate = MPROBE_INVALID_STATE;
            break;
    }

    return dpstate;
}


int mprobe_port_send(struct mprobe_port *dport, u8 *data, u8 *paddr, u32 len)
{
    int ret;
    struct homi_chan *chan;

    chan = dport->chanel;
    if (chan == NULL) {
        mprobe_error("mprobe send chan get chan null\n");
        return BSP_ERROR;
    }

    if (dport->p_type == HOMI_PORT_USB) {
        chan->private = (void *)paddr;
    }

    ret = chan->ops->send(chan, (void *)data, len);
    if (ret) {
        mprobe_error("mprobe data send fail:%d\n", ret);
    }

    return ret;
}

int mprobe_data_send(u8 *data, u8 *paddr, int len)
{
    int ret;
    int curtype;
    struct mprobe_port *dport = NULL;
    struct mprobe_port_ctrl *dpctrl = mprobe_get_dpctrl();

    if (dpctrl == NULL) {
        mprobe_error("mprobe port init fail\n");
        return BSP_ERROR;
    }

    osl_sem_down(&dpctrl->dp_sem);
    if (dpctrl->dp_state == MPROBE_DISCONNECTED_STATE) {
        osl_sem_up(&dpctrl->dp_sem);
        mprobe_printf("mprobe send data, port not connect\n");
        return BSP_ERROR;
    }
    osl_sem_up(&dpctrl->dp_sem);

    osl_sem_down(&dpctrl->tpsem);
    curtype = dpctrl->curtype;
    osl_sem_up(&dpctrl->tpsem);

    dport = mprobe_port_get(dpctrl, curtype);
    if (dport == NULL) {
        mprobe_error("mprobe have no %d type port\n", curtype);
        return BSP_ERROR;
    }

    ret = mprobe_port_send(dport, data, paddr, len);
    if (ret) {
        mprobe_error("mprobe data send fail:%d\n", ret);
        return ret;
    }

    ret = MPROBE_SEND_AYNC;

    return ret;
}


static void mprobe_data_sendcb(void *data, int len)
{
    if (data == NULL || len < 0) {
        len = 0;
    }

    (void)mprobe_rls_dst_buff(len);
}

static void mprobe_data_recvcb(struct homi_chan *chan, void *data, int len)
{
    int ret;

    ret = bsp_hdlc_decode_data(MPROBE_SOFT_DECODE, data, len);
    if (ret) {
        mprobe_error("mprobe receve data soft decode fail\n");
    }
}

static void mprobe_data_eventcb(enum homi_port_type ptype, enum homi_chan_state state)
{
    int dpstate;
    struct mprobe_port *dport = NULL;
    struct mprobe_port_ctrl *dpctrl = mprobe_get_dpctrl();

    if (dpctrl == NULL) {
        mprobe_error("mprobe port init fail\n");
        return;
    }

    dport = mprobe_port_get(dpctrl, ptype);
    if (dport == NULL) {
        mprobe_error("mprobe event cb get port fail\n");
        return;
    }

    dpstate = mprobe_port_state(state);
    if (dpstate == MPROBE_INVALID_STATE) {
        mprobe_error("enent cb get invalid event state:%d\n", state);
        return;
    }

    /* must change port state before mprobe port state. Because of
     * switching mprobe port, need get port state first.
     */
    osl_sem_down(&dport->p_sem);
    dport->p_state = dpstate;
    osl_sem_up(&dport->p_sem);

    osl_sem_down(&dpctrl->tpsem);
    if (dpctrl->curtype != ptype) {
        mprobe_printf("mprobe not use %d port\n", ptype);
        goto up_tpsem;
    }

    osl_sem_down(&dpctrl->dp_sem);
    if (dpctrl->dp_state == dpstate) {
        mprobe_printf("mprobe port state not change\n");
        goto up_dpsem;
    }

    dpctrl->dp_state = dpstate;
    if (dpstate == MPROBE_DISCONNECTED_STATE) {
        (void)mprobe_notify_port_disconn();
    }
    if (dpstate == MPROBE_CONNECTED_STATE) {
        mprobe_enable_dst_chan();
    }

    mprobe_debug("mprobe port curtype:%d, curstate:%d\n", ptype, dpstate);

up_dpsem:
    osl_sem_up(&dpctrl->dp_sem);
up_tpsem:
    osl_sem_up(&dpctrl->tpsem);
    return;
}

static u32 mprobe_usb_chan_init(struct mprobe_port_ctrl *dpctrl)
{
    struct homi_cfg hcfg = {0};
    struct mprobe_port *usbport = &dpctrl->usbport;

    usbport->p_state = MPROBE_DISCONNECTED_STATE;
    osl_sem_init(1, &usbport->p_sem);
    usbport->p_type = HOMI_PORT_USB;

    hcfg.ptype = HOMI_PORT_USB;
    hcfg.ctype = HOMI_CH_USB_MPROBE;
    hcfg.blength = MPROBE_USB_BUFSIZE;
    hcfg.bnum = MPROBE_USB_BUFNUM;
    hcfg.cb.sendcb = mprobe_data_sendcb;
    hcfg.cb.recvcb = mprobe_data_recvcb;
    hcfg.cb.eventcb = mprobe_data_eventcb;

    usbport->chanel = bsp_homi_chan_alloc(&hcfg);
    if (usbport->chanel == NULL) {
        mprobe_error("mprobe usb chan alloc fail\n");
        return BSP_ERROR;
    }

    mprobe_printf("mprobe usb chan init success\n");
    return BSP_OK;
}

int mprobe_port_init(void)
{
    int ret;
    struct mprobe_port_ctrl *dpctrl;

    dpctrl = osl_malloc(sizeof(struct mprobe_port_ctrl));
    if (dpctrl == NULL) {
        mprobe_error("mprobe port ctrl malloc fail\n");
        return BSP_ERROR;
    }
    (void)memset_s(dpctrl, sizeof(struct mprobe_port_ctrl), 0, sizeof(struct mprobe_port_ctrl));

    dpctrl->dp_state = MPROBE_DISCONNECTED_STATE;
    osl_sem_init(1, &dpctrl->dp_sem);
    dpctrl->curtype = HOMI_PORT_USB;
    osl_sem_init(1, &dpctrl->tpsem);

    ret = mprobe_usb_chan_init(dpctrl);
    if (ret) {
        mprobe_error("mprobe usb chan init fail, ret:%d\n", ret);
        osl_free(dpctrl);
        return ret;
    }

    g_mpctrl = dpctrl;
    mprobe_crit("mprobe port init success\n");

    return BSP_OK;
}
