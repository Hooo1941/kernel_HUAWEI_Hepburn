/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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
#include "bvp_vlan.h"
static int g_vlan_dev_connect_state = 0;
static struct chan_info g_channels[CHAN_MAX_NUM];
static usb_udi_enable_cb g_enable_cb[CHAN_MAX_NUM] = {0};
static usb_udi_disable_cb g_disable_cb[CHAN_MAX_NUM] = {0};

int esl_version_check_for_vlan(void)
{
#ifdef FEATURE_VLAN
    const bsp_version_info_s *verinfo = bsp_get_version_info();
    if ((verinfo != NULL) && ((verinfo->plat_type == PLAT_ESL))) {
        return ESL_VERSION_SUPPORT_VLAN;
    }
#endif

    return 0;
}

int bvp_vlan_reg_enablecb(usb_udi_enable_cb func)
{
    int i;

    if (func == NULL) {
        vlan_err("vlan reg enablecb error, cbfunc is null\n");
        return -1;
    }

    for (i = 0; i < CHAN_MAX_NUM; i++) {
        if (g_enable_cb[i] == func) {
            vlan_err("vlan reg enablecb error, is registered\n");
            return 0;
        }
        if (g_enable_cb[i] == NULL) {
            g_enable_cb[i] = func;
            if (g_vlan_dev_connect_state == 1) {
                func();
            }
            return 0;
        }
    }
    vlan_err("bvp_vlan_reg_enablecb error, too much chan num\n");
    return -1;
}

int bvp_vlan_reg_disablecb(usb_udi_disable_cb func)
{
    int i;

    if (func == NULL) {
        vlan_err("vlan reg disable error, cbfunc is null\n");
        return -1;
    }

    for (i = 0; i < CHAN_MAX_NUM; i++) {
        if (g_disable_cb[i] == func) {
            vlan_err("vlan reg disablecb error, is registered\n");
            return 0;
        }
        if (g_disable_cb[i] == NULL) {
            g_disable_cb[i] = func;
            return 0;
        }
    }
    vlan_err("bvp_vlan_reg_disablecb error, too much chan num\n");
    return -1;
}

void *bvp_vlan_open(unsigned int dev_id)
{
    if (dev_id >= CHAN_MAX_NUM) {
        vlan_err("dev_id error, channel open failed\n");
        return NULL;
    }

    vlan_err("bsp vlan open dev_id = %d\n", dev_id);
    if (dev_id == UDI_USB_ACM_GPS) {
        return (void *)&g_channels[VLAN_GPS];
    }
    else if (dev_id == UDI_USB_ACM_LTE_DIAG) {
        return (void *)&g_channels[VLAN_LTE];
    }
    else if (dev_id == UDI_USB_ACM_AT) {
        return (void *)&g_channels[VLAN_AT];
    }
    else if (dev_id == UDI_USB_ACM_OM) {
        return (void *)&g_channels[VLAN_OM];
    }
    else if (dev_id == VLAN_TEST) {
        return (void *)&g_channels[VLAN_TEST];
    }
    else {
        vlan_err("undefined channel id\n");
        return NULL;
    }
    return NULL;
}

signed int bvp_vlan_close(void *handle)
{
    vlan_err("bsp vlan close\n");
    if (handle == NULL) {
        vlan_err("vlan channel close handle error\n");
        return -1;
    }
    if (g_vlan_dev_connect_state == 1) {
        vlan_err("vlan dev still connect, should not close channel\n");
        return -1;
    }
    return 0;
}

signed int bvp_vlan_write(void *handle, void *buf, u32 size)
{
    vlan_err("bvp_vlan_write is not implemented\n");
    return 0;
}

signed int bvp_vlan_read(void *handle, void *buf, u32 size)
{
    vlan_err("bvp_vlan_read is not implemented\n");
    return 0;
}

static int vlan_relloc_read_buff(struct chan_info *channel, void *para)
{
    struct acm_read_buff_info *recv_buff;
    int i = 0;
    char *virt_addr;
    char *phy_addr;
    struct read_buf *req = NULL;
    unsigned long flag;

    recv_buff = (struct acm_read_buff_info *)para;
    if (recv_buff == NULL) {
        vlan_err("ACM_IOCTL_RELLOC_READ_BUFF failed\n");
        return -1;
    }

    channel->recv_buff_info.buff_num = recv_buff->buff_num;
    channel->recv_buff_info.buff_size = READ_BUFF_SIZE;

    for (i = 0; i < recv_buff->buff_num; i++) {
        virt_addr = kmalloc(READ_BUFF_SIZE, GFP_KERNEL | GFP_DMA);
        phy_addr = (char *)dma_map_single(channel->dev->dev, virt_addr, READ_BUFF_SIZE, DMA_TO_DEVICE);

        req = kmalloc(sizeof(*req), GFP_KERNEL);
        req->read_info.virt_addr = virt_addr;
        req->read_info.phy_addr = phy_addr;
        req->buff_state = STORE_IN_MODEL;

        spin_lock_irqsave(&channel->recv_buff_list_lock, flag);
        list_add_tail(&req->list, &channel->recv_buff_list);
        writel(READ_BUFF_SIZE, MSG_RECV_BUF_SIZE_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
        writel(GET_ADDR_HIGH(phy_addr), MSG_RECV_BUF_ADDR_HIGH_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
        writel(GET_ADDR_LOW(phy_addr), MSG_RECV_BUF_ADDR_LOW_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
        spin_unlock_irqrestore(&channel->recv_buff_list_lock, flag);
    }

    return 0;
}

static int vlan_get_rd_buff(struct chan_info *channel, void *para)
{
    struct acm_wr_async_info *read_info;
    struct list_head *req_que;
    struct read_buf *req = NULL;
    unsigned long flag;

    read_info = (struct acm_wr_async_info *)para;
    if (read_info == NULL) {
        vlan_err("ACM_IOCTL_GET_RD_BUFF failed\n");
        return -1;
    }

    req_que = &channel->waiting_read_que;

    spin_lock_irqsave(&channel->waiting_read_que_lock, flag);
    if (!list_empty(req_que)) {
        req = list_first_entry(req_que, struct read_buf, list);
        list_del(&req->list);
    }
    spin_unlock_irqrestore(&channel->waiting_read_que_lock, flag);

    read_info->virt_addr = req->read_info.virt_addr;
    read_info->size = req->read_info.size;

    req->buff_state = READING;

    spin_lock_irqsave(&channel->recv_buff_list_lock, flag);
    list_add_tail(&req->list, &channel->recv_buff_list);
    spin_unlock_irqrestore(&channel->recv_buff_list_lock, flag);

    return 0;
}

static int vlan_return_buff(struct chan_info *channel, void *para)
{
    struct acm_wr_async_info *read_info;
    struct read_buf *return_buf = NULL;
    struct read_buf *req = NULL;
    struct read_buf *tmp;
    char *phy_addr;
    unsigned long flag;

    read_info = (struct acm_wr_async_info *)para;
    if (read_info == NULL) {
        vlan_err("ACM_IOCTL_RETURN_BUFF failed\n");
        return -1;
    }

    spin_lock_irqsave(&channel->recv_buff_list_lock, flag);
    list_for_each_entry_safe(req, tmp, &channel->recv_buff_list, list)
    {
        if (read_info->virt_addr == req->read_info.virt_addr) {
            if (req->buff_state == WAITING_FOR_DEL) {
                list_del(&req->list);
                kfree(req->read_info.virt_addr);
                kfree(req);
            }
            else if (req->buff_state == READING) {
                return_buf = req;
            }
            else {
                vlan_err("return read buff state error\n");
            }
            break;
        }
    }
    spin_unlock_irqrestore(&channel->recv_buff_list_lock, flag);

    if (return_buf == NULL) {
        vlan_err("find return buff faild\n");
        spin_unlock_irqrestore(&channel->recv_buff_list_lock, flag);
        return -1;
    }

    phy_addr = (char *)dma_map_single(channel->dev->dev, return_buf->read_info.virt_addr, READ_BUFF_SIZE, DMA_TO_DEVICE);
    spin_lock_irqsave(&channel->recv_buff_list_lock, flag);
    return_buf->buff_state = STORE_IN_MODEL;
    return_buf->read_info.size = channel->recv_buff_info.buff_size;
    return_buf->read_info.phy_addr = phy_addr;

    writel(return_buf->read_info.size, MSG_RECV_BUF_SIZE_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
    writel(GET_ADDR_HIGH(phy_addr), MSG_RECV_BUF_ADDR_HIGH_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
    writel(GET_ADDR_LOW(phy_addr), MSG_RECV_BUF_ADDR_LOW_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
    spin_unlock_irqrestore(&channel->recv_buff_list_lock, flag);

    return 0;
}

static int vlan_write_async(struct chan_info *channel, void *para)
{
    struct acm_wr_async_info *wr_info;
    struct read_buf *req = NULL;
    unsigned long flag;
    int ret;

    wr_info = (struct acm_wr_async_info *)para;
    if (wr_info == NULL) {
        vlan_err("ACM_IOCTL_WRITE_ASYNC failed\n");
        return -1;
    }

    req = kmalloc(sizeof(*req), GFP_KERNEL);
    req->read_info.virt_addr = wr_info->virt_addr;
    req->read_info.phy_addr = wr_info->phy_addr;
    req->read_info.size = wr_info->size;
    req->private = NULL;

    if (req->read_info.phy_addr == NULL) {
        req->read_info.virt_addr = (char *)dma_alloc_coherent(channel->dev->dev, wr_info->size, (dma_addr_t *)req->read_info.phy_addr, GFP_KERNEL);
        ret = memcpy_s(req->read_info.virt_addr, wr_info->size, wr_info->virt_addr, wr_info->size);
        if (ret != 0) {
            vlan_err("write async memcpy_s failed!\n");
            return -1;
        }
        req->private = wr_info->virt_addr;
    }

    spin_lock_irqsave(&channel->writing_que_lock, flag);
    list_add_tail(&req->list, &channel->writing_que);
    writel(wr_info->size, MSG_SEND_REQ_SIZE_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
    writel(GET_ADDR_HIGH(wr_info->phy_addr), MSG_SEND_REQ_ADDR_HIGH_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
    writel(GET_ADDR_LOW(wr_info->phy_addr), MSG_SEND_REQ_ADDR_LOW_REG(channel->chan_id) + channel->dev->reg_vaddr_base);
    spin_unlock_irqrestore(&channel->writing_que_lock, flag);

    return 0;
}

signed int bvp_vlan_ioctl(void *handle, unsigned int cmd, void *para)
{
    struct chan_info *channel = (struct chan_info *)handle;
    int res = 0;

    if (handle == NULL) {
        vlan_err("vlan ioctl handle is null\n");
        return -1;
    }

    switch (cmd) {
    case ACM_IOCTL_RELLOC_READ_BUFF:
        res = vlan_relloc_read_buff(channel, para);
        break;
    case ACM_IOCTL_SET_READ_CB:
        channel->readcb = para;
        break;
    case ACM_IOCTL_SET_WRITE_CB:
        channel->writecb = para;
        break;
    case ACM_IOCTL_SET_EVT_CB:
        channel->eventcb = para;
        break;
    case ACM_IOCTL_WRITE_DO_COPY:
        break;
    case ACM_IOCTL_GET_RD_BUFF:
        res = vlan_get_rd_buff(channel, para);
        break;
    case ACM_IOCTL_RETURN_BUFF:
        res = vlan_return_buff(channel, para);
        break;
    case ACM_IOCTL_WRITE_ASYNC:
        res = vlan_write_async(channel, para);
        break;
    default:
        vlan_err("vlan : ioctl cmd = %d, undefined\n", cmd);
        break;
    }
    return res;
}

static void vlan_channel_disable(int chan_id)
{
    struct chan_info *channel;
    struct read_buf *req = NULL;
    struct read_buf *tmp;
    unsigned long flag;

    channel = &g_channels[chan_id];

    vlan_err("clear recv buff\n");

    spin_lock_irqsave(&channel->recv_buff_list_lock, flag);
    list_for_each_entry_safe(req, tmp, &channel->recv_buff_list, list)
    {
        if (req->buff_state != READING) {
            req->buff_state = WAITING_FOR_DEL;
        }
        else {
            list_del(&req->list);
            kfree(req->read_info.virt_addr);
            kfree(req);
        }
    }
    spin_unlock_irqrestore(&channel->recv_buff_list_lock, flag);

    vlan_err("clear recv que\n");

    spin_lock_irqsave(&channel->waiting_read_que_lock, flag);
    list_for_each_entry_safe(req, tmp, &channel->waiting_read_que, list)
    {
        list_del(&req->list);
        kfree(req->read_info.virt_addr);
        kfree(req);
    }
    spin_unlock_irqrestore(&channel->waiting_read_que_lock, flag);

    vlan_err("clear write que\n");

    spin_lock_irqsave(&channel->writing_que_lock, flag);
    list_for_each_entry_safe(req, tmp, &channel->writing_que, list)
    {
        list_del(&req->list);
        if (channel->writecb != NULL) {
            channel->writecb(req->read_info.virt_addr, req->read_info.phy_addr, 0);
        }
        kfree(req);
    }
    spin_unlock_irqrestore(&channel->writing_que_lock, flag);
}

static void vlan_irq_task_dev_connect(struct vlan_dev *dev)
{
    int i;
    int chan_state = 0;
    int wait_time;

    if (g_vlan_dev_connect_state == 1) {
        vlan_err("repeat insert event, please check\n");
    }
    else {
        vlan_err("vlan dev connect\n");
        g_vlan_dev_connect_state = 1;

        __pm_stay_awake(dev->wakelock);

        for (i = 0; i < CHAN_USED_NUM; i++) {
            writel(0x1, dev->reg_vaddr_base + CHAN_CTRL_REG(i));
            wait_time = 0;
            chan_state = readl(dev->reg_vaddr_base + CHAN_STATE_REG(i));
            while (!(chan_state & 0x1) && wait_time < MAX_WAIT_TIME) {
                chan_state = readl(dev->reg_vaddr_base + CHAN_STATE_REG(i));
                wait_time++;
            }

            if (wait_time == MAX_WAIT_TIME) {
                vlan_err("model does not respond for a long time, exit waiting\n");
                break;
            }

            vlan_err("channel %d enable\n", i);
        }

        for (i = 0; i < CHAN_MAX_NUM; i++) {
            if (g_enable_cb[i] != NULL) {
                g_enable_cb[i]();
            }
        }
    }
}

static void vlan_irq_task_client_connect(int chan_id)
{
    if (g_channels[chan_id].client_state == 1) {
        vlan_err("repeat connnect event, please check\n");
    }
    else {
        vlan_err("client connect\n");
        g_channels[chan_id].client_state = 1;

        if (g_channels[chan_id].eventcb == NULL) {
            vlan_err("chan eventcb error\n");
        }
        else {
            g_channels[chan_id].eventcb(ACM_EVT_DEV_READY);
        }
    }
}

static struct acm_wr_async_info vlan_get_recv_info(struct vlan_dev *dev, int chan_id)
{
    unsigned int addr_low;
    unsigned int addr_high;
    unsigned long addr;
    unsigned int len;
    struct acm_wr_async_info recv_info;

    addr_low = readl(dev->reg_vaddr_base + MSG_RECV_DONE_ADDR_LOW_REG(chan_id));
    addr_high = readl(dev->reg_vaddr_base + MSG_RECV_DONE_ADDR_HIGH_REG(chan_id));
    len = readl(dev->reg_vaddr_base + MSG_RECV_DONE_SIZE_REG(chan_id));

    addr = GET_ADDR_FULL(addr_high, addr_low);
    recv_info.phy_addr = (char *)addr;
    recv_info.size = len;
    return recv_info;
}

static struct read_buf *vlan_get_read_buf(struct vlan_dev *dev, int chan_id, char* addr)
{
    unsigned long flag;
    struct read_buf *req;
    struct read_buf *tmp;
    struct read_buf *recv_buf = NULL;

    spin_lock_irqsave(&g_channels[chan_id].recv_buff_list_lock, flag);
    list_for_each_entry_safe(req, tmp, &g_channels[chan_id].recv_buff_list, list)
    {
        if (addr == req->read_info.phy_addr) {
            if (req->buff_state == STORE_IN_MODEL) {
                recv_buf = req;
                list_del(&req->list);
            }
            else {
                vlan_err("get recv buff state error\n");
            }
            break;
        }
    }
    spin_unlock_irqrestore(&g_channels[chan_id].recv_buff_list_lock, flag);

    return recv_buf;
}

static void vlan_irq_task_recv(struct vlan_dev *dev, int chan_id, int recv_num)
{
    struct acm_wr_async_info recv_info;
    struct read_buf *recv_buf = NULL;
    unsigned long flag;
    int recv_count = 0;

    while (1) {
        if (recv_count >= recv_num) {
            break;
        }
        recv_count++;

        recv_info = vlan_get_recv_info(dev, chan_id);
        if (recv_info.phy_addr == 0 || recv_info.size == 0 || recv_info.size > READ_BUFF_SIZE) {
            vlan_err("read recv msg error, addr=%d, len=%d\n", recv_info.phy_addr, recv_info.size);
            break;
        }

        recv_buf = vlan_get_read_buf(dev, chan_id, recv_info.phy_addr);
        if (recv_buf == NULL) {
            vlan_err("get recv buff faild\n");
            break;
        }
        dma_unmap_single(g_channels[chan_id].dev->dev, (dma_addr_t)recv_buf->read_info.phy_addr, recv_info.size, DMA_FROM_DEVICE);

        spin_lock_irqsave(&g_channels[chan_id].waiting_read_que_lock, flag);
        recv_buf->read_info.size = recv_info.size;
        recv_buf->buff_state = WAITING_TO_READ;
        list_add_tail(&recv_buf->list, &g_channels[chan_id].waiting_read_que);
        spin_unlock_irqrestore(&g_channels[chan_id].waiting_read_que_lock, flag);

        if (g_channels[chan_id].readcb == NULL) {
            vlan_err("chan readcb null\n");
        }
        else {
            g_channels[chan_id].readcb();
        }
    }
}

static struct acm_wr_async_info vlan_get_sendcb_info(struct vlan_dev *dev, int chan_id)
{
    unsigned int addr_low;
    unsigned int addr_high;
    unsigned long addr;
    unsigned int len;
    struct acm_wr_async_info sendcb_info;

    addr_low = readl(dev->reg_vaddr_base + MSG_SEND_DONE_ADDR_LOW_REG(chan_id));
    addr_high = readl(dev->reg_vaddr_base + MSG_SEND_DONE_ADDR_HIGH_REG(chan_id));
    len = readl(dev->reg_vaddr_base + MSG_SEND_DONE_SIZE_REG(chan_id));

    addr = GET_ADDR_FULL(addr_high, addr_low);
    sendcb_info.phy_addr = (char *)addr;
    sendcb_info.size = len;
    return sendcb_info;
}

static struct read_buf *vlan_get_sendcb_buf(struct vlan_dev *dev, int chan_id, char* addr)
{
    unsigned long flag;
    struct read_buf *req;
    struct read_buf *tmp;
    struct read_buf *sendcb_buf = NULL;

    spin_lock_irqsave(&g_channels[chan_id].writing_que_lock, flag);
    list_for_each_entry_safe(req, tmp, &g_channels[chan_id].writing_que, list)
    {
        if (addr == req->read_info.phy_addr) {
            sendcb_buf = req;
            list_del(&req->list);
            break;
        }
    }
    spin_unlock_irqrestore(&g_channels[chan_id].writing_que_lock, flag);

    return sendcb_buf;
}

static void vlan_irq_task_send_completed(struct vlan_dev *dev, int chan_id, int sendcb_num)
{
    struct acm_wr_async_info sendcb_info;
    struct read_buf *sendcb_buf = NULL;
    int sendcb_count = 0;

    while (1) {
        if (sendcb_count >= sendcb_num) {
            break;
        }
        sendcb_count++;

        sendcb_info = vlan_get_sendcb_info(dev, chan_id);
        if (sendcb_info.phy_addr == 0) {
            vlan_err("sendcb num error\n");
            break;
        }

        sendcb_buf = vlan_get_sendcb_buf(dev, chan_id, sendcb_info.phy_addr);
        if (sendcb_buf == NULL) {
            vlan_err("vlan find send_buf faild\n");
            break;
        }

        if (sendcb_info.size < sendcb_buf->read_info.size) {
            vlan_err("send failed, actually send size = %d\n", sendcb_info.size);
        }

        if (sendcb_buf->private != NULL) {
            dma_free_coherent(g_channels[chan_id].dev->dev, sendcb_buf->read_info.size, sendcb_buf->read_info.virt_addr, (dma_addr_t)sendcb_buf->read_info.phy_addr);
            sendcb_buf->read_info.virt_addr = sendcb_buf->private;
            sendcb_buf->read_info.phy_addr = NULL;
        }

        if (g_channels[chan_id].writecb == NULL) {
            vlan_err("chan writecb null\n");
        }
        else {
            g_channels[chan_id].writecb(sendcb_buf->read_info.virt_addr, sendcb_buf->read_info.phy_addr, sendcb_info.size);
        }

        kfree(sendcb_buf);
    }
}

static void vlan_irq_task_client_disconnet(int chan_id)
{
    if (g_channels[chan_id].client_state == 0) {
        vlan_err("repeat suspend event, please check\n");
    }
    else {
        vlan_err("client close\n");
        g_channels[chan_id].client_state = 0;

        if (g_channels[chan_id].eventcb == NULL) {
            vlan_err("chan eventcb error\n");
        }
        else {
            g_channels[chan_id].eventcb(ACM_EVT_DEV_SUSPEND);
        }
    }
}

static void vlan_irq_task_dev_disconnect(struct vlan_dev *dev)
{
    int i;
    int chan_state = 0;
    int wait_time;

    if (g_vlan_dev_connect_state == 0) {
        vlan_err("repeat remove event, please check\n");
    }
    else {
        vlan_err("vlan dev disconnect\n");
        g_vlan_dev_connect_state = 0;
        for (i = 0; i < CHAN_USED_NUM; i++) {
            chan_state = readl(dev->reg_vaddr_base + CHAN_STATE_REG(i));
            if (chan_state & 0x2) {
                g_channels[i].client_state = 0;

                if (g_channels[i].eventcb == NULL) {
                    vlan_err("chan eventcb error\n");
                }
                else {
                    g_channels[i].eventcb(ACM_EVT_DEV_SUSPEND);
                }
            }

            writel(0x0, dev->reg_vaddr_base + CHAN_CTRL_REG(i));
            chan_state = readl(dev->reg_vaddr_base + CHAN_STATE_REG(i));
            wait_time = 0;
            while (chan_state & 0x1 && wait_time < MAX_WAIT_TIME) {
                chan_state = readl(dev->reg_vaddr_base + CHAN_STATE_REG(i));
                wait_time++;
            }

            if (wait_time == MAX_WAIT_TIME) {
                vlan_err("model does not respond for a long time, exit waiting\n");
                break;
            }

            vlan_channel_disable(i);

            vlan_err("channel %d disable\n", i);
        }

        for (i = 0; i < CHAN_MAX_NUM; i++) {
            if (g_disable_cb[i] != NULL) {
                g_disable_cb[i]();
            }
        }

        __pm_relax(dev->wakelock);
    }
}

void task_handler(struct intr_task *task, struct vlan_dev *dev)
{
    int cmd;
    int chan_id;
    int chan_state = 0;
    int i;

    if (task->vlan_intr_status & VLAN_DEV_CONNECT_EVENT && task->vlan_glb_status & 0x1) {
        vlan_irq_task_dev_connect(dev);
    }

    for (i = 0; i < CHAN_USED_NUM; i++) {
        if (!(task->vlan_intr_status & (1 << i))) {
            continue;
        }
        chan_id = i;
        cmd = task->chan_intr[i].chan_intr_status;
        chan_state = task->chan_intr[i].chan_state;

        if (cmd & VLAN_CLIENT_CONNECT_EVENT && chan_state & 0x2) {
            vlan_irq_task_client_connect(chan_id);
        }

        if (cmd & VLAN_RECV) {
            vlan_irq_task_recv(dev, chan_id, task->chan_intr[i].recv_num);
        }

        if (cmd & VLAN_SEND_COMPLETED) {
            vlan_irq_task_send_completed(dev, chan_id, task->chan_intr[i].sendcb_num);
        }

        if (cmd & VLAN_CLIENT_CONNECT_EVENT && !(chan_state & 0x2)) {
            vlan_irq_task_client_disconnet(chan_id);
        }
    }

    if (task->vlan_intr_status & VLAN_DEV_CONNECT_EVENT && !(task->vlan_glb_status & 0x1)) {
        vlan_irq_task_dev_disconnect(dev);
    }
}

void work_handler(struct kthread_work *work)
{
    unsigned long flag;
    struct intr_task *task = NULL;
    vlan_dev *v_dev = container_of(work, struct vlan_dev, irq_work.work);

    while (1) {
        spin_lock_irqsave(&v_dev->intr_task_list_lock, flag);
        if (list_empty(&v_dev->intr_task_list)) {
            spin_unlock_irqrestore(&v_dev->intr_task_list_lock, flag);
            return;
        }
        task = list_first_entry(&v_dev->intr_task_list, struct intr_task, list);
        list_del(&task->list);
        spin_unlock_irqrestore(&v_dev->intr_task_list_lock, flag);
        task_handler(task, v_dev);
        kfree(task);
    }
}

irqreturn_t irq_handler(int irq, void *dev)
{
    vlan_dev *v_dev = (vlan_dev *)dev;
    unsigned long flag;
    struct intr_task *task;
    int i;

    task = kmalloc(sizeof(*task), GFP_ATOMIC);

    task->vlan_intr_status = readl(v_dev->reg_vaddr_base + VLAN_INTR_STATUS_REG);
    task->vlan_glb_status = readl(v_dev->reg_vaddr_base + VLAN_GLB_STATUS_REG);
    for (i = 0; i < CHAN_USED_NUM; i++) {
        if (!(task->vlan_intr_status & (1 << i))) {
            continue;
        }
        task->chan_intr[i].chan_intr_status = readl(v_dev->reg_vaddr_base + CHAN_INTR_STATUS_REG(i));
        task->chan_intr[i].chan_state = readl(v_dev->reg_vaddr_base + CHAN_STATE_REG(i));
        if (task->chan_intr[i].chan_intr_status & VLAN_RECV) {
            task->chan_intr[i].recv_num = readl(v_dev->reg_vaddr_base + MSG_RECV_DONE_CNT_REG(i));
        }
        if (task->chan_intr[i].chan_intr_status & VLAN_SEND_COMPLETED) {
            task->chan_intr[i].sendcb_num = readl(v_dev->reg_vaddr_base + MSG_SEND_DONE_CNT_REG(i));
        }
        writel(task->chan_intr[i].chan_intr_status, v_dev->reg_vaddr_base + CHAN_INTR_RAW_REG(i));
    }
    writel(task->vlan_intr_status, v_dev->reg_vaddr_base + VLAN_INTR_RAW_REG);

    spin_lock_irqsave(&v_dev->intr_task_list_lock, flag);
    list_add_tail(&task->list, &v_dev->intr_task_list);
    spin_unlock_irqrestore(&v_dev->intr_task_list_lock, flag);

    kthread_queue_delayed_work(&v_dev->irq_worker, &v_dev->irq_work, 0);
    return 0;
}

static void vlan_channel_init(int chan_id)
{
    if (chan_id > CHAN_USED_NUM) {
        vlan_err("vlan channel init failed\n");
        return;
    }
    g_channels[chan_id].chan_id = chan_id;
    INIT_LIST_HEAD(&g_channels[chan_id].recv_buff_list);
    spin_lock_init(&g_channels[chan_id].recv_buff_list_lock);
    INIT_LIST_HEAD(&g_channels[chan_id].waiting_read_que);
    spin_lock_init(&g_channels[chan_id].waiting_read_que_lock);
    INIT_LIST_HEAD(&g_channels[chan_id].writing_que);
    spin_lock_init(&g_channels[chan_id].writing_que_lock);
}

#ifdef CONFIG_PM_SLEEP
static int runtime_pm_suspend(struct device *dev)
{
    int i;

    struct platform_device *pdev = to_platform_device(dev);
    struct vlan_dev *v_dev = platform_get_drvdata(pdev);
    vlan_err("vlan suspend\n");
    writel(0x0, v_dev->reg_vaddr_base + VLAN_GLB_CFG_REG);

    writel(0x0, v_dev->reg_vaddr_base + VLAN_INTR_MASK_REG);
    for (i = 0; i < CHAN_USED_NUM; i++) {
        writel(0x0, v_dev->reg_vaddr_base + CHAN_INTR_MASK_REG(i));
    }
    return 0;
}

static int runtime_pm_resume(struct device *dev)
{
    int i;

    struct platform_device *pdev = to_platform_device(dev);
    struct vlan_dev *v_dev = platform_get_drvdata(pdev);
    vlan_err("vlan resume\n");
    writel(0x1, v_dev->reg_vaddr_base + VLAN_GLB_CFG_REG);

    writel(~0x0, v_dev->reg_vaddr_base + VLAN_INTR_MASK_REG);
    for (i = 0; i < CHAN_USED_NUM; i++) {
        writel(~0x0, v_dev->reg_vaddr_base + CHAN_INTR_MASK_REG(i));
    }
    return 0;
}

static int runtime_pm_idle(struct device *dev)
{
    vlan_err("vlan idle\n");
    return 0;
}

static const struct dev_pm_ops runtime_pm_ops = {SET_RUNTIME_PM_OPS(runtime_pm_suspend, runtime_pm_resume,
                                                                    runtime_pm_idle)};
#endif

static int vlan_driver_probe(struct platform_device *pdev)
{
    vlan_dev *v_dev;
    int ret;
    int i;
    struct resource *regs;
    struct sched_param param = {
        .sched_priority = MAX_RT_PRIO - 1};

    vlan_err("vlan probe\n");

    v_dev = devm_kzalloc(&pdev->dev, sizeof(struct vlan_dev), GFP_KERNEL);

    regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (regs == NULL) {
        vlan_err("vlan platform_get_resource error\n");
    }

    v_dev->reg_vaddr_base = devm_ioremap_resource(&pdev->dev, regs);
    if (v_dev->reg_vaddr_base == NULL) {
        vlan_err("vlan devm_ioremap_resource error\n");
    }

    v_dev->irq = platform_get_irq(pdev, 0);
    if (v_dev->irq == 0) {
        vlan_err("vlan platform_get_irq error\n");
    }

    INIT_LIST_HEAD(&v_dev->intr_task_list);
    spin_lock_init(&v_dev->intr_task_list_lock);

    kthread_init_worker(&v_dev->irq_worker);
    kthread_init_delayed_work(&v_dev->irq_work, work_handler);
    v_dev->irq_worker_task = kthread_run(kthread_worker_fn, &v_dev->irq_worker, "vlan_irq_kthread");
    sched_setscheduler(v_dev->irq_worker_task, SCHED_FIFO, &param);

    v_dev->wakelock = wakeup_source_register(NULL, "vlan_wakelock");
    if (v_dev->wakelock == NULL) {
        vlan_err("vlan wakeup_source_register error\n");
    }

    ret = request_irq(v_dev->irq, irq_handler, IRQF_TRIGGER_HIGH, "vlan_irq_kthread", v_dev);
    if (ret != 0) {
        vlan_err("request_irq res = %d\n", ret);
    }

    v_dev->dev = &pdev->dev;
    platform_set_drvdata(pdev, v_dev);

    for (i = 0; i < CHAN_MAX_NUM; i++) {
        g_channels[i].dev = v_dev;
    }

    writel(~0x0, v_dev->reg_vaddr_base + VLAN_INTR_MASK_REG);

    for (i = 0; i < CHAN_USED_NUM; i++) {
        vlan_channel_init(i);
        writel(~0x0, v_dev->reg_vaddr_base + CHAN_INTR_MASK_REG(i));
    }

    writel(0x1, v_dev->reg_vaddr_base + VLAN_GLB_CFG_REG);

    vlan_err("vlan probe finished\n");
    return 0;
}

int vlan_driver_remove(struct platform_device *dev)
{
    return 0;
}

static const struct of_device_id vlan_match[] = {
    {.compatible = "bvp_vlan_drv"},
    {},
};
MODULE_DEVICE_TABLE(of, vlan_match);

static struct platform_driver vlan_driver = {
    .probe = vlan_driver_probe,
    .remove = vlan_driver_remove,
    .driver =
        {
            .name = "vlan_driver",
            .of_match_table = of_match_ptr(vlan_match),
#ifdef CONFIG_PM_SLEEP
            .pm = &runtime_pm_ops,
#endif
        },
};

static int __init vlan_init(void)
{
    if (!esl_version_check_for_vlan()) {
        return 0;
    }
    vlan_err("vlan init\n");

    platform_driver_register(&vlan_driver);
    return 0;
}

module_init(vlan_init);
MODULE_LICENSE("GPL");
