/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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
#include "relay_acm.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include <securec.h>
#include <mdrv_udi.h>
#include <mdrv_usb.h>
#include <bsp_usb.h>
#include <bsp_relay.h>
#include <bsp_ppp_ndis.h>
#include <product_config.h>

#define THIS_MODU mod_relay

void relay_acm_read_done_cb(unsigned int minor_type);
#define ACM_READ_DONE_CB(_name)                \
    static void acm_read_done_cb_##_name(void) \
    {                                          \
        relay_acm_read_done_cb(_name);         \
    }

ACM_READ_DONE_CB(RELAY_DEV_ACM_CTRL);
ACM_READ_DONE_CB(RELAY_DEV_AT);
ACM_READ_DONE_CB(RELAY_DEV_OM);
ACM_READ_DONE_CB(RELAY_DEV_MODEM);
ACM_READ_DONE_CB(RELAY_DEV_SKYTONE);
ACM_READ_DONE_CB(RELAY_DEV_PCIE_AT);
#define ACM_READ_DONE_CB_NAME(_name) (acm_read_done_cb_##_name)

void relay_acm_write_done_cb(unsigned int minor_type, char *vir_addr, char *phy_addr, int size);
#define ACM_WRITE_DONE_CB(_name)                                                    \
    static void acm_write_done_cb_##_name(char *vir_addr, char *phy_addr, int size) \
    {                                                                               \
        relay_acm_write_done_cb(_name, vir_addr, phy_addr, size);                   \
    }

ACM_WRITE_DONE_CB(RELAY_DEV_ACM_CTRL);
ACM_WRITE_DONE_CB(RELAY_DEV_AT);
ACM_WRITE_DONE_CB(RELAY_DEV_OM);
ACM_WRITE_DONE_CB(RELAY_DEV_MODEM);
ACM_WRITE_DONE_CB(RELAY_DEV_SKYTONE);
ACM_WRITE_DONE_CB(RELAY_DEV_PCIE_AT);
#define ACM_WRITE_DONE_CB_NAME(_name) (acm_write_done_cb_##_name)

void relay_acm_evt_cb(unsigned int minor_type, enum relay_evt port_status);
#define ACM_EVT_CB(_name)                                     \
    static void acm_evt_cb_##_name(ACM_EVT_E port_status)     \
    {                                                         \
        relay_acm_evt_cb(_name, (enum relay_evt)port_status); \
    }

ACM_EVT_CB(RELAY_DEV_ACM_CTRL);
ACM_EVT_CB(RELAY_DEV_AT);
ACM_EVT_CB(RELAY_DEV_OM);
ACM_EVT_CB(RELAY_DEV_MODEM);
ACM_EVT_CB(RELAY_DEV_SKYTONE);
ACM_EVT_CB(RELAY_DEV_PCIE_AT);
#define ACM_EVT_CB_NAME(_name) (acm_evt_cb_##_name)

void relay_acm_msc_read_cb(unsigned int minor_type, struct modem_msc_stru *msc);
#define ACM_MSC_READ_CB(_name) \
    static void acm_msc_read_cb_##_name(struct modem_msc_stru *msc)     \
    {                                                         \
        relay_acm_msc_read_cb(_name, msc); \
    }

ACM_MSC_READ_CB(RELAY_DEV_MODEM);
#define ACM_MSC_READ_CB_NAME(_name) (acm_msc_read_cb_##_name)

void relay_acm_switch_mode_cb(unsigned int minor_type);
#define ACM_SWITCH_MODE_CB(_name) \
    static void acm_switch_mode_cb_##_name(void)     \
    {                                                         \
        relay_acm_switch_mode_cb(_name); \
    }

ACM_SWITCH_MODE_CB(RELAY_DEV_MODEM);
#define ACM_SWITCH_MODE_CB_NAME(_name) (acm_switch_mode_cb_##_name)

#define ACM_MAX_FLOW_NUM 512

static struct relay_acm_device g_acm_devices[RELAY_DEV_ACM_MAX] = {
    [RELAY_DEV_ACM_CTRL] = {
        .adp_dev.name = "ACM_CTRL",
        .adp_dev.udi_id = UDI_ACM_CTRL_ID,
        .buff_num = CONFIG_RELAY_USB_BUFF_NUM,
        .buff_size = ACM_CTRL_BUFSIZE,
        .flow_ctrl = true,
        .funcs.read_done_cb = ACM_READ_DONE_CB_NAME(RELAY_DEV_ACM_CTRL),
        .funcs.write_done_cb = ACM_WRITE_DONE_CB_NAME(RELAY_DEV_ACM_CTRL),
        .funcs.evt_cb = ACM_EVT_CB_NAME(RELAY_DEV_ACM_CTRL),
    },
    [RELAY_DEV_AT] = {
        .adp_dev.name = "ACM_AT",
        .adp_dev.udi_id = UDI_ACM_AT_ID,
        .buff_num = CONFIG_RELAY_USB_BUFF_NUM,
        .buff_size = PCUI_BUFSIZE,
        .flow_ctrl = true,
        .funcs.read_done_cb = ACM_READ_DONE_CB_NAME(RELAY_DEV_AT),
        .funcs.write_done_cb = ACM_WRITE_DONE_CB_NAME(RELAY_DEV_AT),
        .funcs.evt_cb = ACM_EVT_CB_NAME(RELAY_DEV_AT),
    },
    [RELAY_DEV_OM] = {
        .adp_dev.name = "ACM_OM",
        .adp_dev.udi_id = UDI_ACM_OM_ID,
        .buff_num = CONFIG_RELAY_USB_BUFF_NUM,
        .buff_size = G3DIAG_BUFSIZE,
        .flow_ctrl = true,
        .funcs.read_done_cb = ACM_READ_DONE_CB_NAME(RELAY_DEV_OM),
        .funcs.write_done_cb = ACM_WRITE_DONE_CB_NAME(RELAY_DEV_OM),
        .funcs.evt_cb = ACM_EVT_CB_NAME(RELAY_DEV_OM),
    },
    [RELAY_DEV_MODEM] = {
        .adp_dev.name = "ACM_MODEM",
        .adp_dev.udi_id = UDI_ACM_MODEM_ID,
        .buff_num = USB_MODEM_UL_DATA_BUFF_NUM,
        .buff_size = MODEM_BUFSIZE,
        .flow_ctrl = false,
        .funcs.read_done_cb = ACM_READ_DONE_CB_NAME(RELAY_DEV_MODEM),
        .funcs.write_done_cb = ACM_WRITE_DONE_CB_NAME(RELAY_DEV_MODEM),
        .funcs.msc_read_cb = ACM_MSC_READ_CB_NAME(RELAY_DEV_MODEM),
        .funcs.switch_mode_cb = ACM_SWITCH_MODE_CB_NAME(RELAY_DEV_MODEM),
        .funcs.evt_cb = ACM_EVT_CB_NAME(RELAY_DEV_MODEM),
    },
    [RELAY_DEV_SKYTONE] = {
        .adp_dev.name = "ACM_SKYTONE",
        .adp_dev.udi_id = UDI_ACM_SKYTONE_ID,
        .buff_num = CONFIG_RELAY_USB_BUFF_NUM,
        .buff_size = SKYTONE_BUFSIZE,
        .flow_ctrl = true,
        .funcs.read_done_cb = ACM_READ_DONE_CB_NAME(RELAY_DEV_SKYTONE),
        .funcs.write_done_cb = ACM_WRITE_DONE_CB_NAME(RELAY_DEV_SKYTONE),
        .funcs.evt_cb = ACM_EVT_CB_NAME(RELAY_DEV_SKYTONE),
    },
    [RELAY_DEV_PCIE_AT] = {
        .adp_dev.name = "PCIE_AT",
        .adp_dev.udi_id = UDI_PCDEV_AT_ID,
        .buff_num = PCIE_RELAY_BUFF_NUM0,
        .buff_size = PCDEV_AT_BUFSIZE,
        .flow_ctrl = false,
        .funcs.read_done_cb = ACM_READ_DONE_CB_NAME(RELAY_DEV_PCIE_AT),
        .funcs.write_done_cb = ACM_WRITE_DONE_CB_NAME(RELAY_DEV_PCIE_AT),
        .funcs.evt_cb = ACM_EVT_CB_NAME(RELAY_DEV_PCIE_AT),
    },
};

void relay_acm_up_deliver(struct relay_acm_device *acm_dev, unsigned int cmd, void *buf, unsigned int len)
{
    int ret;

    if (acm_dev->adp_dev.opend == RELAY_OPEND_MAGIC) {
        acm_dev->stat.read_cb_sched++;
        if (acm_dev->cbs.read_cb != NULL) {
            ret = acm_dev->cbs.read_cb(acm_dev->adp_dev.private, cmd, buf, len);
            if (ret) {
                acm_dev->stat.read_done_fail++;
            }
        } else {
            acm_dev->stat.no_read_cb++;
        }
    } else {
        acm_dev->adp_dev.base_stat.port_not_open++;
    }
}

void relay_acm_read_done_cb(unsigned int minor_type)
{
    struct relay_acm_device *acm_dev = NULL;
    struct acm_wr_async_info buf_info = { 0 };
    unsigned int udi_handler;
    int ret;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.read_done++;
    udi_handler = acm_dev->adp_dev.udi_handler;
    ret = mdrv_udi_ioctl(udi_handler, ACM_IOCTL_GET_RD_BUFF, &buf_info);
    if (ret) {
        acm_dev->stat.read_buff_get_fail++;
        relay_err("minor_type:%d get read buff fail ret:%d\n", minor_type, ret);
        return;
    }

    acm_dev->stat.read_done_len += buf_info.size;
    relay_acm_up_deliver(acm_dev, RELAY_ACM_ACK_ASYNC, buf_info.virt_addr, buf_info.size);

    ret = mdrv_udi_ioctl(udi_handler, ACM_IOCTL_RETURN_BUFF, &buf_info);
    if (ret) {
        acm_dev->stat.ret_buf_fail++;
        relay_err("minor_type:%d return fail ret:%d\n", minor_type, ret);
    } else {
        acm_dev->stat.ret_buf++;
    }

    return;
}

void relay_acm_msc_read_cb(unsigned int minor_type, struct modem_msc_stru *msc)
{
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type:%d out of range\n", minor_type);
        return;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.ack_msc++;
    relay_acm_up_deliver(acm_dev, RELAY_ACM_ACK_MSC, (void*)msc, sizeof(struct modem_msc_stru));
}

void relay_acm_switch_mode_cb(unsigned int minor_type)
{
    unsigned int mode = 0;
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type:%d out of range\n", minor_type);
        return;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.switch_mode++;
    relay_acm_up_deliver(acm_dev, RELAY_ACM_NOTIFY_SWITCH_MODE, (void*)&mode, sizeof(unsigned int));
}

void relay_acm_evt_cb(unsigned int minor_type, enum relay_evt port_status)
{
    struct relay_acm_device *acm_dev = &g_acm_devices[minor_type];

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type:%d, is invalid\n", minor_type);
        return;
    }
    relay_info("minor_type:%d, port_status:%d\n", minor_type, port_status);

    acm_dev->port_status = port_status;
    if (acm_dev->cbs.evt_cb != NULL) {
        acm_dev->cbs.evt_cb(port_status);
    } else {
        relay_info("minor_type:%d, evt_cb is null\n", minor_type);
    }

    if (port_status == RELAY_EVT_DEV_SUSPEND) {
        relay_info("Receive USB disconnect\n");
    } else if (port_status == RELAY_EVT_DEV_READY) {
        relay_info("Receive USB connect\n");
    } else {
        relay_err("Receive USB status is unknow\n");
    }

    return;
}

void relay_acm_tx_callback(struct relay_acm_device *acm_dev)
{
    unsigned long flags;

    spin_lock_irqsave(&(acm_dev->tx_lock), flags);
    if (acm_dev->send_num == 0) {
        relay_err("dev %s callback error\n", acm_dev->adp_dev.name);
        spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);
        return;
    }
    acm_dev->send_num--;

    if (list_empty(&acm_dev->tx_queue)) {
        spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);
        return;
    }
    spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);

    queue_delayed_work(acm_dev->work_queue, &acm_dev->write_work, 0);
}

void relay_acm_write_done_cb(unsigned int minor_type, char *vir_addr, char *phy_addr, int size)
{
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX || vir_addr == NULL) {
        relay_err("minor_type:%d para err\n", minor_type);
        return;
    }

    acm_dev = &g_acm_devices[minor_type];

    if (acm_dev->flow_ctrl == true) {
        relay_acm_tx_callback(acm_dev);
    }

    if (acm_dev->cbs.write_cb) {
        acm_dev->cbs.write_cb(vir_addr, phy_addr, size); /* diag port reture buff */
    } else {
        kfree(vir_addr); /* vcom port free direct */
    }

    acm_dev->stat.write_done++;
    acm_dev->stat.write_done_len += size;
    return;
}

static int relay_acm_set_udi_ioctl(struct relay_acm_device *acm_dev)
{
    int ret = 0;
    struct relay_read_buff_info buf_info = { 0 };

    /* set usb read buff */
    if (acm_dev->buff_num && acm_dev->buff_size) {
        buf_info.buff_num = acm_dev->buff_num;
        buf_info.buff_size = acm_dev->buff_size;

        ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, ACM_IOCTL_RELLOC_READ_BUFF, (void *)&buf_info);
        if (ret) {
            relay_info("[%s] realloc buf failed\n", acm_dev->adp_dev.name);
            return ret;
        }
    }

    /* set usb read cb */
    ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, ACM_IOCTL_SET_READ_CB, acm_dev->funcs.read_done_cb);
    if (ret) {
        relay_info("[%s] set read cb failed\n", acm_dev->adp_dev.name);
        return ret;
    }

    /* set usb wrtie do not copy */
    ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, ACM_IOCTL_WRITE_DO_COPY, 0);
    if (ret) {
        relay_info("[%s] set wrtie do copy failed\n", acm_dev->adp_dev.name);
        return ret;
    }

    /* set write cb */
    ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, ACM_IOCTL_SET_WRITE_CB, acm_dev->funcs.write_done_cb);
    if (ret) {
        relay_info("[%s] set write cb failed\n", acm_dev->adp_dev.name);
        return ret;
    }

    /* set event cb */
    ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, ACM_IOCTL_SET_EVT_CB, acm_dev->funcs.evt_cb);
    if (ret) {
        relay_info("[%s] set event cb failed\n", acm_dev->adp_dev.name);
        return ret;
    }

    /* set msc read cb */
    if (acm_dev->funcs.msc_read_cb != NULL) {
        ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, ACM_MODEM_IOCTL_SET_MSC_READ_CB, acm_dev->funcs.msc_read_cb);
        if (ret) {
            relay_info("[%s] set msc read cb failed\n", acm_dev->adp_dev.name);
            return ret;
        }
    }

    /* set switch mode cb */
    if (acm_dev->funcs.switch_mode_cb != NULL) {
        ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, RELAY_ACM_MODEM_IOCTL_SET_SWITCH_MODE_CB, acm_dev->funcs.switch_mode_cb);
        if (ret) {
            relay_info("[%s] set switch mode cb failed\n", acm_dev->adp_dev.name);
            return ret;
        }
    }

    /* get write num */
    if (acm_dev->flow_ctrl) {
        ret = mdrv_udi_ioctl(acm_dev->adp_dev.udi_handler, RELAY_ACM_IOCTL_GET_WRITE_NUM, &acm_dev->capacity_num);
        if (ret) {
            relay_info("[%s] get write num failed\n", acm_dev->adp_dev.name);
            return ret;
        }
    }

    return ret;
}

static void relay_acm_tx_work(struct work_struct *work)
{
    int ret;
    unsigned long flags;
    struct relay_acm_packet *packet = NULL;
    struct relay_acm_device *acm_dev = container_of(work, struct relay_acm_device, write_work.work);
    int handler = acm_dev->adp_dev.udi_handler;

    while(1) {
        spin_lock_irqsave(&(acm_dev->tx_lock), flags);
        if (acm_dev->send_num >= acm_dev->capacity_num) {
            acm_dev->stat.write_full++;
            spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);
            return;
        }

        packet = list_first_entry_or_null(&acm_dev->tx_queue, struct relay_acm_packet, list);
        if (packet == NULL) {
            spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);
            return;
        }

        list_del(&packet->list);
        acm_dev->send_num++;
        acm_dev->pending_num--;
        spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);

        ret = mdrv_udi_ioctl(handler, ACM_IOCTL_WRITE_ASYNC, &packet->info);
        if (ret) {
            acm_dev->send_num--;
            acm_dev->stat.write_fail++;
            relay_err("data send fail:%d\n", ret);
        } else {
            acm_dev->stat.write_succ++;
        }
        kfree(packet);
    }
}

int relay_acm_dev_open(unsigned int minor_type)
{
    struct relay_acm_device *acm_dev = NULL;
    struct udi_open_param param;
    unsigned int udi_id;
    int ret = -EINVAL;
    int handler;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return ret;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.open++;
    if (acm_dev->adp_dev.opend == RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d has opend\n", minor_type);
        acm_dev->stat.open_again++;
        return ret;
    }

    udi_id = acm_dev->adp_dev.udi_id;
    param.devid = udi_id;
    param.private = 0;
    handler = mdrv_udi_open(&param);
    if (handler == UDI_INVALID_HANDLE) {
        relay_info("minor_type:%d open fail\n", minor_type);
        acm_dev->stat.open_fail++;
        return ret;
    }

    acm_dev->adp_dev.udi_handler = handler;
    ret = relay_acm_set_udi_ioctl(acm_dev);
    if (ret) {
        relay_info("minor_type:%d set ioctl fail:%d\n", minor_type, ret);
        acm_dev->stat.ioctl_fail++;
        return ret;
    }

    acm_dev->adp_dev.opend = RELAY_OPEND_MAGIC;
    acm_dev->stat.open_succ++;
    relay_info("minor_type:%d open succ\n", minor_type);
    return ret;
}

int relay_acm_dev_close(unsigned int minor_type)
{
    struct relay_acm_device *acm_dev = NULL;
    int ret;
    int handler;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_info("minor_type:%d overflow\n", minor_type);
        return -EINVAL;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.close++;
    if (acm_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d not open\n", minor_type);
        acm_dev->stat.close_again++;
        return -EINVAL;
    }

    handler = acm_dev->adp_dev.udi_handler;
    ret = mdrv_udi_close(handler);
    if (ret) {
        relay_info("minor_type:%d handler:%d close fail:%d\n", minor_type, handler, ret);
        acm_dev->stat.close_fail++;
        return ret;
    }

    acm_dev->adp_dev.udi_handler = UDI_INVALID_HANDLE;
    acm_dev->adp_dev.opend = 0;
    acm_dev->stat.close_succ++;
    return ret;
}

int relay_acm_dev_write(unsigned int minor_type, char *addr, unsigned int size)
{
    struct relay_acm_device *acm_dev = NULL;
    int handler;

    if (minor_type >= RELAY_DEV_ACM_MAX || addr == NULL || size == 0) {
        relay_err("minor_type:%d para err\n", minor_type);
        return -EINVAL;
    }

    acm_dev = &g_acm_devices[minor_type];
    if (acm_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d not open\n", minor_type);
        acm_dev->stat.droped++;
        return -EINVAL;
    }

    handler = acm_dev->adp_dev.udi_handler;
    return mdrv_udi_write(handler, addr, size);
}

int relay_acm_query_ready_state(struct relay_acm_device *acm_dev, unsigned int minor_type)
{
    int ret = 0;
    unsigned int ready = acm_dev->adp_dev.b_enable == RELAY_ENABLE_MAGIC ? 1 : 0;

    if (acm_dev->cbs.read_cb != NULL) {
        ret = acm_dev->cbs.read_cb(acm_dev->adp_dev.private, RELAY_ACM_NOTIFY_READY,
                                   (void*)&ready, sizeof(unsigned int));
        if (ret < 0) {
            relay_err("minor_type:%d notify ready fail\n", minor_type);
        }
    }
    return ret;
}

static int relay_acm_dev_write_flowctrl(struct relay_acm_device *acm_dev, struct acm_wr_async_info *wr_info)
{
    unsigned long flags;
    struct relay_acm_packet *packet = NULL;

    spin_lock_irqsave(&(acm_dev->tx_lock), flags);
    if (acm_dev->pending_num >= ACM_MAX_FLOW_NUM) {
        spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);
        acm_dev->stat.droped++;
        return -ENOMEM;
    }
    spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);

    packet = (struct relay_acm_packet *)kzalloc(sizeof(struct relay_acm_packet), GFP_ATOMIC);
    if (packet == NULL) {
        return -ENOMEM;
    }
    packet->info = *wr_info;
    spin_lock_irqsave(&(acm_dev->tx_lock), flags);
    list_add_tail(&(packet->list), &(acm_dev->tx_queue));
    acm_dev->pending_num++;
    spin_unlock_irqrestore(&(acm_dev->tx_lock), flags);
    queue_delayed_work(acm_dev->work_queue, &acm_dev->write_work, 0);
    return 0;
}

static int relay_acm_dev_write_only(struct relay_acm_device *acm_dev, int handler, void *para)
{
    int ret;

    ret = mdrv_udi_ioctl(handler, ACM_IOCTL_WRITE_ASYNC, para);
    if (ret) {
        acm_dev->stat.write_fail++;
    } else {
        acm_dev->stat.write_succ++;
    }
    return ret;
}

int relay_acm_dev_write_async(struct relay_acm_device *acm_dev, int handler, void *para, unsigned int para_size)
{
    struct acm_wr_async_info info = {0};

    info.virt_addr = para;
    info.size = para_size;
    info.phy_addr = 0;

    if (acm_dev->flow_ctrl == true) {
        return relay_acm_dev_write_flowctrl(acm_dev, &info);
    } else {
        return relay_acm_dev_write_only(acm_dev, handler, (void*)&info);
    }
}

int relay_acm_dev_write_msc(unsigned int minor_type, int handler, void *para, unsigned int para_size)
{
    if (para_size != sizeof(struct modem_msc_stru)) {
        relay_err("minor_type:%d buffer size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handler, ACM_MODEM_IOCTL_MSC_WRITE_CMD, para);
}

static int relay_acm_set_iface_info(unsigned int minor_type, int handler, void *para, unsigned int para_size)
{
    struct ppp_ndis_field_info field_info;
    struct ppp_iface_info *iface_info = NULL;

    if (para_size != sizeof(struct ppp_iface_info)) {
        relay_err("minor_type:%d, buffer size error\n", minor_type);
        return -EINVAL;
    }
    field_info.chan_type = PPP_NDIS_DEV_MODEM;
    field_info.chan_id = 0;
    field_info.data_len = sizeof(struct ppp_iface_info);
    iface_info = (struct ppp_iface_info *)para;
    bsp_set_ppp_iface_info(&field_info, iface_info);
    return mdrv_udi_ioctl(handler, RELAY_ACM_MODEM_IOCTL_SET_IFACE_INFO, para);
}

int relay_acm_enable_escape_detect(unsigned int minor_type, int handle, void *para, unsigned int para_size)
{
    if (para_size != sizeof(unsigned int)) {
        relay_err("minor_type:%d, para size error\n", minor_type);
        return -EINVAL;
    }
    return mdrv_udi_ioctl(handle, RELAY_ACM_MODEM_IOCTL_ENABLE_ESCAPE_DETECT, para);
}

static int relay_acm_set_trans_mode(unsigned int minor_type, int handler, void *para, unsigned int para_size)
{
    if (para_size != sizeof(enum ppp_trans_mode)) {
        relay_err("minor_type:%d, buffer size error\n", minor_type);
        return -EINVAL;
    }

    return mdrv_udi_ioctl(handler, RELAY_ACM_MODEM_IOCTL_SET_TRANS_MODE, para);
}

int relay_acm_dev_ioctl(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size)
{
    int ret = 0;
    int handler;
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX || para == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }
    acm_dev = &g_acm_devices[minor_type];
    if (acm_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d not open\n", minor_type);
        acm_dev->stat.droped++;
        return -EINVAL;
    }
    handler = acm_dev->adp_dev.udi_handler;
    switch (cmd) {
        case RELAY_ACM_REG_READ_CB:
            acm_dev->cbs.read_cb = (relay_acm_read_cb)para;
            break;

        case RELAY_ACM_REG_WRITE_CB:
            acm_dev->cbs.write_cb = (relay_acm_write_donecb)para;
            break;

        case RELAY_ACM_REG_EVT_CB:
            acm_dev->cbs.evt_cb = (relay_acm_event_cb)para;
            break;

        case RELAY_ACM_WRITE_ASYNC:
            if (acm_dev->port_status == RELAY_EVT_DEV_SUSPEND) {
                acm_dev->stat.suspend++;
                return -EINVAL;
            }
            ret = relay_acm_dev_write_async(acm_dev, handler, para, para_size);
            break;
        case RELAY_ACM_QUERY_READY:
            ret = relay_acm_query_ready_state(acm_dev, minor_type);
            break;
        case RELAY_ACM_WRITE_MSC:
            return relay_acm_dev_write_msc(minor_type, handler, para, para_size);
        case RELAY_ACM_SET_TRANS_MODE:
            return relay_acm_set_trans_mode(minor_type, handler, para, para_size);
        case RELAY_ACM_SET_IFACE_INFO:
            return relay_acm_set_iface_info(minor_type, handler, para, para_size);
        case RELAY_ACM_ENABLE_ESCAPE_DETECT:
            return relay_acm_enable_escape_detect(minor_type, handler, para, para_size);
        default:
            relay_err("minor_type:%d cmd error\n", minor_type);
            return -EINVAL;
    }
    return ret;
}

void relay_acm_enable(unsigned int minor_type)
{
    struct relay_acm_device *acm_dev = NULL;

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.enable++;
    if (acm_dev->cbs.enable_cb != NULL) {
        acm_dev->cbs.enable_cb(acm_dev->adp_dev.private);
    } else {
        acm_dev->stat.enable_no_cb++;
        relay_info("minor_type:%d, enable_cb NULL\n", minor_type);
    }
    acm_dev->adp_dev.b_enable = RELAY_ENABLE_MAGIC;
    return;
}

void relay_acm_disable(unsigned int minor_type)
{
    struct relay_acm_device *acm_dev = NULL;

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->stat.disable++;
    if (acm_dev->cbs.disable_cb != NULL) {
        acm_dev->cbs.disable_cb(acm_dev->adp_dev.private);
    } else {
        acm_dev->stat.disable_no_cb++;
        relay_info("minor_type:%d, enable_cb NULL\n", minor_type);
    }
    acm_dev->adp_dev.b_enable = 0;
    return;
}

int relay_acm_reg_enablecb(unsigned int minor_type, relay_enable_cb func, void *private)
{
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX || func == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->cbs.enable_cb = func;
    acm_dev->adp_dev.private = private;

    if (acm_dev->adp_dev.b_enable == RELAY_ENABLE_MAGIC) {
        acm_dev->cbs.enable_cb(private);
    }
    return 0;
}

int relay_acm_reg_disablecb(unsigned int minor_type, relay_disable_cb func)
{
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX || func == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    acm_dev = &g_acm_devices[minor_type];
    acm_dev->cbs.disable_cb = func;
    return 0;
}

int relay_acm_dev_stat_init(struct relay_acm_device *acm_dev, unsigned int minor_type)
{
    int ret;

    ret = memset_s(&acm_dev->stat, sizeof(acm_dev->stat), 0, sizeof(struct relay_acm_stat));
    if (ret) {
        relay_err("minor_type:%d memset_s fail:%d\n", minor_type, ret);
        return ret;
    }

    ret = memset_s(&acm_dev->adp_dev.base_stat, sizeof(acm_dev->adp_dev.base_stat), 0, sizeof(struct relay_base_stat));
    if (ret) {
        relay_err("minor_type:%d memset_s fail:%d\n", minor_type, ret);
        return ret;
    }
    return 0;
}

int relay_acm_init_tx_ctx(struct relay_acm_device *acm_dev)
{
    spin_lock_init(&(acm_dev->tx_lock));
    INIT_LIST_HEAD(&(acm_dev->tx_queue));
    INIT_DELAYED_WORK(&acm_dev->write_work, relay_acm_tx_work);
    acm_dev->work_queue = create_singlethread_workqueue("relay acm");
    if (acm_dev->work_queue == NULL) {
        relay_err("dev:%s creat workqueue failed\n", acm_dev->adp_dev.name);
        return -EINVAL;
    }
    return 0;
}

int relay_acm_init(void)
{
    int ret;
    unsigned int type;
    struct relay_acm_device *acm_dev = NULL;

    for (type = 0; type < RELAY_DEV_ACM_MAX; type++) {
        acm_dev = &g_acm_devices[type];
        ret = relay_acm_dev_stat_init(acm_dev, type);
        if (ret < 0) {
            return ret;
        }

        if (acm_dev->flow_ctrl == true) {
            ret = relay_acm_init_tx_ctx(acm_dev);
            if (ret < 0) {
                relay_err("init tx ctx fail\n");
                return ret;
            }
        }
    }
    return 0;
}

void relay_acm_info_show(unsigned int minor_type)
{
    struct relay_acm_device *acm_dev = NULL;

    if (minor_type >= RELAY_DEV_ACM_MAX) {
        relay_err("minor_type: %d overflow\n", minor_type);
        return;
    }
    acm_dev = &g_acm_devices[minor_type];

    relay_err("minor_type          : %u\n", minor_type);
    relay_err("dev name            : %s\n", acm_dev->adp_dev.name);

    relay_err("read_cb             : %s\n", acm_dev->cbs.read_cb ? "good" : "null");
    relay_err("write_cb            : %s\n", acm_dev->cbs.write_cb ? "good" : "null");
    relay_err("evt_cb              : %s\n", acm_dev->cbs.evt_cb ? "good" : "null");
    relay_err("enable_cb           : %s\n", acm_dev->cbs.enable_cb ? "good" : "null");
    relay_err("disable_cb          : %s\n", acm_dev->cbs.disable_cb ? "good" : "null");

    relay_err("open                : %u\n", acm_dev->stat.open);
    relay_err("open_again          : %u\n", acm_dev->stat.open_again);
    relay_err("open_succ           : %u\n", acm_dev->stat.open_succ);
    relay_err("open_fail           : %u\n", acm_dev->stat.open_fail);
    relay_err("ioctl_fail          : %u\n", acm_dev->stat.ioctl_fail);
    relay_err("close               : %u\n", acm_dev->stat.close);
    relay_err("close_again         : %u\n", acm_dev->stat.close_again);
    relay_err("close_succ          : %u\n", acm_dev->stat.close_succ);
    relay_err("close_fail          : %u\n", acm_dev->stat.close_fail);
    relay_err("enable              : %u\n", acm_dev->stat.enable);
    relay_err("enable_no_cb        : %u\n", acm_dev->stat.enable_no_cb);
    relay_err("disable             : %u\n", acm_dev->stat.disable);
    relay_err("disable_no_cb       : %u\n", acm_dev->stat.disable_no_cb);

    relay_err("port_not_open       : %u\n", acm_dev->adp_dev.base_stat.port_not_open);
    relay_err("capacity number     : %u\n", acm_dev->capacity_num);

    relay_err("droped num          : %u\n", acm_dev->stat.droped);
    relay_err("suspend num         : %u\n", acm_dev->stat.suspend);
    relay_err("ret_buf             : %u\n", acm_dev->stat.ret_buf);
    relay_err("ret_buf_fail        : %u\n", acm_dev->stat.ret_buf_fail);

    relay_err("read_done           : %u\n", acm_dev->stat.read_done);
    relay_err("read_done_len       : %u\n", acm_dev->stat.read_done_len);
    relay_err("read_done_fail      : %u\n", acm_dev->stat.read_done_fail);
    relay_err("read_buff_get_fail  : %u\n", acm_dev->stat.read_buff_get_fail);
    relay_err("read_cb_sched       : %u\n", acm_dev->stat.read_cb_sched);
    relay_err("ack_msc             : %u\n", acm_dev->stat.ack_msc);
    relay_err("switch_mode         : %u\n", acm_dev->stat.switch_mode);
    relay_err("write queue         : %u\n", acm_dev->stat.write_queue);
    relay_err("write succ          : %u\n", acm_dev->stat.write_succ);
    relay_err("write_suspend       : %u\n", acm_dev->stat.write_suspend);
    relay_err("write_len           : %u\n", acm_dev->stat.write_len);
    relay_err("write_fail          : %u\n", acm_dev->stat.write_fail);
    relay_err("write_full          : %u\n", acm_dev->stat.write_full);
    relay_err("write_done          : %u\n", acm_dev->stat.write_done);
    relay_err("write_done_len      : %u\n", acm_dev->stat.write_done_len);
}
