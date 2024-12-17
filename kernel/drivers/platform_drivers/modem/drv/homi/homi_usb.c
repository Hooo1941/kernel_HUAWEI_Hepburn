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
#include <osl_list.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <mdrv_usb.h>
#include <mdrv_udi.h>
#include <bsp_udi.h>
#include <bsp_homi.h>
#include "homi_core.h"
#include "homi_debug.h"
#include "homi_usb.h"

static int udi_chan_enable(enum homi_port_type ptype, enum homi_chan_type ctype, struct udi_chan_cfg *ucfg)
{
    int ret;
    struct homi_chan *chan;
    struct acm_read_buff_info acm_buf = { 0 };
    struct udi_open_param param;

    /* must put chan after used */
    chan = homi_get_valid_chan_by_type(ptype, ctype);

    if (chan == NULL) {
        return BSP_HOMI_GET_CHAN_ERR;
    }

    acm_buf.buff_size = chan->blength;
    acm_buf.buff_num = chan->bnum;

    param.devid = ucfg->devid;
    chan->fd = (u64)(uintptr_t)mdrv_udi_open(&param);
    if (chan->fd == 0) {
        ret = BSP_HOMI_USB_UDI_ENABLE_ERR;
        goto put_chan;
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_RELLOC_READ_BUFF, &acm_buf);

    if (ret) {
        goto put_chan;
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_SET_READ_CB, ucfg->readcb);
    if (ret) {
        goto put_chan;
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_SET_WRITE_CB, ucfg->writecb);
    if (ret) {
        goto put_chan;
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_SET_EVT_CB, ucfg->eventcb);
    if (ret) {
        goto put_chan;
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_WRITE_DO_COPY, NULL);
    if (ret) {
        goto put_chan;
    }

    osl_sem_down(&chan->chsem);
    chan->state = HOMI_CHSTATE_CONNECTED;
    osl_sem_up(&chan->chsem);

    if (chan->cb.eventcb != NULL) {
        chan->cb.eventcb(HOMI_PORT_USB, HOMI_CHSTATE_CONNECTED);
    }

put_chan:
    homi_err("homi udi chan enable ret=%d\n", ret);
    homi_chan_put(chan);
    return ret;
}

static int udi_chan_disable(enum homi_port_type ptype, enum homi_chan_type ctype)
{
    struct homi_chan *chan;

    /* must put chan after used */
    chan = homi_get_valid_chan_by_type(ptype, ctype);
    if (chan == NULL) {
        homi_err("homi disable %d chan get fail\n", ctype);
        return BSP_HOMI_GET_CHAN_ERR;
    }

    if (chan->fd != 0) {
        mdrv_udi_close(chan->fd);
        chan->fd = 0;
    }

    osl_sem_down(&chan->chsem);
    chan->state = HOMI_CHSTATE_DISCONNECTED;
    osl_sem_up(&chan->chsem);

    if (chan->cb.eventcb != NULL) {
        chan->cb.eventcb(HOMI_PORT_USB, HOMI_CHSTATE_DISCONNECTED);
    }

    homi_printf("homi %d disable handle done\n", ctype);

    homi_chan_put(chan);
    return 0;
}

static int udi_chan_readcb(enum homi_port_type ptype, enum homi_chan_type ctype)
{
    int ret;
    struct acm_wr_async_info wrinfo = { 0 };
    struct homi_chan *chan;

    /* must put chan after used */
    chan = homi_get_valid_chan_by_type(ptype, ctype);
    if (chan == NULL) {
        homi_err("homi readcb %d chan get fail\n", ctype);
        return BSP_HOMI_GET_CHAN_ERR;
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_GET_RD_BUFF, &wrinfo);
    if (ret) {
        homi_err("%d chan get rd buff fail, ret:%d\n", ctype, ret);
        goto put_chan;
    }

    if (chan->cb.recvcb != NULL) {
        chan->cb.recvcb(chan, wrinfo.virt_addr, wrinfo.size);
    }

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_RETURN_BUFF, &wrinfo);
    if (ret) {
        homi_err("%d chan return buff fail, ret:%d\n", ctype, ret);
        goto put_chan;
    }

    homi_debug(HLEVDBG, "%d chan readcb done\n", ctype);

put_chan:
    homi_chan_put(chan);
    return ret;
}

static int udi_chan_writecb(enum homi_port_type ptype, enum homi_chan_type ctype, char *vaddr, int len)
{
    struct homi_chan *chan;

    /* must put chan after used */
    chan = homi_get_valid_chan_by_type(ptype, ctype);
    if (chan == NULL) {
        homi_err("homi writecb %d chan get fail\n", ctype);
        return BSP_HOMI_GET_CHAN_ERR;
    }

    if (chan->cb.sendcb != NULL) {
        chan->cb.sendcb(vaddr, len);
    }

    homi_debug(HLEVDBG, "homi %d chan writecb done\n", ctype);

    homi_chan_put(chan);
    return 0;
}

static int udi_chan_eventcb(enum homi_port_type ptype, enum homi_chan_type ctype, ACM_EVT_E state)
{
    int change_flags = 0;
    enum homi_chan_state chstate;
    struct homi_chan *chan;

    /* must put chan after used */
    chan = homi_get_valid_chan_by_type(ptype, ctype);
    if (chan == NULL) {
        homi_err("get %d udi chan fail to eventcb\n", ctype);
        return BSP_HOMI_GET_CHAN_ERR;
    }

    switch (state) {
        case ACM_EVT_DEV_SUSPEND:
            osl_sem_down(&chan->chsem);
            chan->state = HOMI_CHSTATE_SUSPEND;
            osl_sem_up(&chan->chsem);

            chstate = HOMI_CHSTATE_SUSPEND;
            change_flags = 1;
            homi_printf("%d udi chan suspend eventcb\n", ctype);
            break;
        case ACM_EVT_DEV_READY:
            osl_sem_down(&chan->chsem);
            chan->state = HOMI_CHSTATE_CONNECTED;
            osl_sem_up(&chan->chsem);

            chstate = HOMI_CHSTATE_CONNECTED;
            change_flags = 1;
            homi_printf("%d udi chan ready eventcb\n", ctype);
            break;
        default:
            homi_printf("%d udi chan invalid state:%d event\n", ctype, state);
            break;
    }

    if ((chan->cb.eventcb != NULL) && (change_flags == 1)) {
        chan->cb.eventcb(HOMI_PORT_USB, chstate);
    }

    homi_debug(HLEVDBG, "%d udi chan eventcb done\n", ctype);

    homi_chan_put(chan);
    return 0;
}

static void gps_udi_readcb(void)
{
    int ret;

    homi_debug(HLEVDBG, "gps udi chan readcb start\n");
    ret = udi_chan_readcb(HOMI_PORT_USB, HOMI_CH_USB_GPS);
    if (ret) {
        homi_err("homi gps udi chan readcb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "gps udi chan readcb done\n");
    return;
}

static void gps_udi_writecb(char *vaddr, char *paddr, int len)
{
    int ret;

    homi_debug(HLEVDBG, "gps udi chan writecb start\n");
    ret = udi_chan_writecb(HOMI_PORT_USB, HOMI_CH_USB_GPS, vaddr, len);
    if (ret) {
        homi_err("homi gps udi chan writecb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "gps udi chan writecb done\n");
    return;
}

static void gps_udi_eventcb(ACM_EVT_E state)
{
    int ret;

    homi_debug(HLEVDBG, "gps udi chan eventcb start\n");
    ret = udi_chan_eventcb(HOMI_PORT_USB, HOMI_CH_USB_GPS, state);
    if (ret) {
        homi_err("homi gps udi chan eventcb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "gps udi chan eventcb done\n");
    return;
}

static struct udi_chan_cfg g_gps_udi_cfg = {
    .devid = UDI_ACM_GPS_ID,
    .readcb = gps_udi_readcb,
    .writecb = gps_udi_writecb,
    .eventcb = gps_udi_eventcb,
};

static void gps_udi_enable(void)
{
    int ret;

    ret = udi_chan_enable(HOMI_PORT_USB, HOMI_CH_USB_GPS, &g_gps_udi_cfg);
    if (ret) {
        homi_err("homi gps udi enable fail, ret:%d\n", ret);
        return;
    }

    homi_printf("gps udi chan enable success\n");
    return;
}

static void gps_udi_disable(void)
{
    int ret;

    ret = udi_chan_disable(HOMI_PORT_USB, HOMI_CH_USB_GPS);
    if (ret) {
        homi_err("homi disable gps udi chan fail\n");
        return;
    }

    homi_printf("gps udi disable success\n");
    return;
}

static void lte_udi_readcb(void)
{
    int ret;

    homi_debug(HLEVDBG, "lte udi chan readcb start\n");
    ret = udi_chan_readcb(HOMI_PORT_USB, HOMI_CH_USB_4G);
    if (ret) {
        homi_err("homi lte udi chan readcb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "lte udi chan readcb done\n");
    return;
}

static void lte_udi_writecb(char *vaddr, char *paddr, int len)
{
    int ret;

    homi_debug(HLEVDBG, "lte udi chan writecb start\n");
    ret = udi_chan_writecb(HOMI_PORT_USB, HOMI_CH_USB_4G, vaddr, len);
    if (ret) {
        homi_err("homi lte udi chan writecb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "lte udi chan writecb done\n");
    return;
}

static void lte_udi_eventcb(ACM_EVT_E state)
{
    int ret;

    homi_debug(HLEVDBG, "lte udi chan eventcb start\n");
    ret = udi_chan_eventcb(HOMI_PORT_USB, HOMI_CH_USB_4G, state);
    if (ret) {
        homi_err("homi lte udi chan eventcb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "lte udi chan eventcb done\n");
    return;
}

static struct udi_chan_cfg g_lte_udi_cfg = {
    .devid = UDI_ACM_LTE_DIAG_ID,
    .readcb = lte_udi_readcb,
    .writecb = lte_udi_writecb,
    .eventcb = lte_udi_eventcb,
};

static void lte_udi_enable(void)
{
    int ret;

    ret = udi_chan_enable(HOMI_PORT_USB, HOMI_CH_USB_4G, &g_lte_udi_cfg);
    if (ret) {
        homi_err("homi lte udi enable fail, ret:%d\n", ret);
        return;
    }

    homi_printf("lte udi chan enable success\n");
    return;
}

static void lte_udi_disable(void)
{
    int ret;

    ret = udi_chan_disable(HOMI_PORT_USB, HOMI_CH_USB_4G);
    if (ret) {
        homi_err("homi disable lte udi chan fail\n");
        return;
    }

    homi_printf("lte udi disable handle success\n");
    return;
}


static void mprobe_udi_readcb(void)
{
    int ret;

    homi_debug(HLEVDBG, "mprobe udi chan readcb start\n");
    ret = udi_chan_readcb(HOMI_PORT_USB, HOMI_CH_USB_MPROBE);
    if (ret) {
        homi_err("homi mprobe udi chan readcb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "mprobe udi chan readcb done\n");
    return;
}

static void mprobe_udi_writecb(char *vaddr, char *paddr, int len)
{
    int ret;

    homi_debug(HLEVDBG, "mprobe udi chan writecb start\n");
    ret = udi_chan_writecb(HOMI_PORT_USB, HOMI_CH_USB_MPROBE, vaddr, len);
    if (ret) {
        homi_err("homi mprobe udi chan writecb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "mprobe udi chan writecb done\n");
    return;
}

static void mprobe_udi_eventcb(ACM_EVT_E state)
{
    int ret;

    homi_debug(HLEVDBG, "mprobe udi chan eventcb start\n");
    ret = udi_chan_eventcb(HOMI_PORT_USB, HOMI_CH_USB_MPROBE, state);
    if (ret) {
        homi_err("homi mprobe udi chan eventcb handle fail\n");
        return;
    }

    homi_debug(HLEVDBG, "mprobe udi chan eventcb done\n");
    return;
}

static struct udi_chan_cfg g_mprobe_udi_cfg = {
#ifdef BSP_CONFIG_PHONE_TYPE
    .devid = UDI_ACM_CDMA_LOG_ID,
#else
    .devid = UDI_ACM_MPROBE_ID,
#endif
    .readcb = mprobe_udi_readcb,
    .writecb = mprobe_udi_writecb,
    .eventcb = mprobe_udi_eventcb,
};

static void mprobe_udi_enable(void)
{
    int ret;

    ret = udi_chan_enable(HOMI_PORT_USB, HOMI_CH_USB_MPROBE, &g_mprobe_udi_cfg);
    if (ret) {
        homi_err("homi mprobe udi enable fail, ret:%d\n", ret);
        return;
    }

    homi_printf("mprobe udi chan enable success\n");
    return;
}

static void mprobe_udi_disable(void)
{
    int ret;

    ret = udi_chan_disable(HOMI_PORT_USB, HOMI_CH_USB_MPROBE);
    if (ret) {
        homi_err("homi disable mprobe udi chan fail\n");
        return;
    }

    homi_printf("mprobe udi disable handle success\n");
    return;
}

static int udi_chan_init(struct homi_chan *chan)
{
    usb_udi_enable_cb enable = NULL;
    usb_udi_disable_cb disable = NULL;

    switch (chan->type) {
        case HOMI_CH_USB_GPS:
            enable = gps_udi_enable;
            disable = gps_udi_disable;
            break;
        case HOMI_CH_USB_4G:
            enable = lte_udi_enable;
            disable = lte_udi_disable;
            break;
        case HOMI_CH_USB_MPROBE:
            enable = mprobe_udi_enable;
            disable = mprobe_udi_disable;
            break;            
        default:
            homi_err("homi usb udi init chan type fail\n");
            return BSP_HOMI_USB_UDI_INIT_ERR;
    }

    mdrv_usb_reg_enablecb(enable);

    mdrv_usb_reg_disablecb(disable);

    return 0;
}

static int udi_chan_send(struct homi_chan *chan, void *data, unsigned int len)
{
    int ret;
    struct acm_wr_async_info wrinfo = { 0 };

    homi_debug(HLEVDBG, "udi %d chan send data start\n", chan->type);
    if (!homi_chan_test_and_get(chan, HOMI_CHSTATE_CONNECTED)) {
        homi_err("%d usb chan not connected, can't send data\n", chan->type);
        return BSP_HOMI_CHAN_NOT_CONNECT_ERR;
    }

    wrinfo.virt_addr = (char *)data;
    wrinfo.phy_addr = (char *)chan->private;
    wrinfo.size = len;

    ret = mdrv_udi_ioctl(chan->fd, ACM_IOCTL_WRITE_ASYNC, &wrinfo);
    if (ret) {
        homi_err("homi %d chan send data err, ret:%d\n", chan->type, ret);
        goto put_chan;
    }

    homi_debug(HLEVDBG, "udi %d chan send data success\n", chan->type);
put_chan:
    homi_chan_put(chan);
    return ret;
}

static int udi_chan_release(struct homi_chan *chan)
{
    return 0;
}

static struct homi_chan_ops g_udi_chan_ops = {
    .init = udi_chan_init,
    .send = udi_chan_send,
    .release = udi_chan_release,
};

static struct homi_chan *homi_usb_chan_alloc(struct homi_port *port, struct homi_cfg *cfg)
{
    struct homi_chan *chan;

    chan = homi_chan_init(port, &g_udi_chan_ops, cfg);
    if (chan == NULL) {
        homi_err("homi usb chan alloc init fail\n");
        return NULL;
    }

    osl_sem_down(&chan->chsem);
    if (chan->state == HOMI_CHSTATE_INITING) {
        chan->state = HOMI_CHSTATE_DISCONNECTED;
    }
    osl_sem_up(&chan->chsem);

    return chan;
}

static int homi_usb_chan_release(struct homi_port *port, struct homi_chan *chan)
{
    int refcnt;
    unsigned long flags;
    struct homi_ctrl *hctrl = port->hctrl;

    spin_lock_irqsave(&chan->ref_lock, flags);
    refcnt = chan->refcnt;
    spin_unlock_irqrestore(&chan->ref_lock, flags);

    if (refcnt != 0) {
        /* chan must have removed from port */
        spin_lock_irqsave(&hctrl->gc_lock, flags);
        list_add_tail(&chan->list, &hctrl->gc_list);
        spin_unlock_irqrestore(&hctrl->gc_lock, flags);
        queue_delayed_work(system_wq, &hctrl->gc_work, msecs_to_jiffies(HOMI_CH_GC_TIME));
        homi_err("homi chan refcount is not 0, add to gc list\n");
        return 0;
    }

    if (chan->ops->release != NULL) {
        chan->ops->release(chan);
    }

    kfree(chan);

    return 0;
}

int homi_usb_init(void)
{
    int ret;
    struct homi_port *port;

    port = homi_port_alloc();
    if (port == NULL) {
        homi_err("homi usb port alloc fail\n");
        return BSP_HOMI_PORT_MALLOC_ERR;
    }

    port->type = HOMI_PORT_USB;

    port->alloc = homi_usb_chan_alloc;
    port->release = homi_usb_chan_release;

    /* register port at last */
    ret = homi_port_register(port);
    if (ret) {
        osl_free(port);
        homi_err("homi usb port register fail, ret:%d\n", ret);
        return ret;
    }

    homi_printf("homi usb port init success\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(homi_usb_init);
#endif
