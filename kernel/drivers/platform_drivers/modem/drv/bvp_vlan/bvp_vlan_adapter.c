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
#include "bvp_vlan_adapter.h"


#define UDP_MAX_PACKAGE 65507
#define SOCK_NAME_LEN 32
#define SOCK_NUM_MAX 1

static int g_usb_state = 0;
static struct chan_info g_channels[USB_MAX_NUM];
static usb_udi_enable_cb g_enable_cb[USB_MAX_NUM] = {0};
static usb_udi_disable_cb g_disable_cb[USB_MAX_NUM] = {0};
static int g_socket_port[] = {3000, 20253, 8888};

static char *vlan_socket_name(char name[], char *namefmt, ...)
{
    int ret;
    va_list args;

    va_start(args, namefmt);
    ret = vsnprintf_s(name, (size_t)SOCK_NAME_LEN, (size_t)SOCK_NAME_LEN - 1, namefmt, args);
    va_end(args);

    return (ret ? "sockserver" : name);
}

static int vlan_socket_bind_listen(struct chan_info *chan)
{
    int ret;
    struct sockaddr_in *skaddr = (struct sockaddr_in *)&chan->skcfg->skaddr;

    skaddr->sin_family = chan->skcfg->family;
    skaddr->sin_port = htons(chan->skcfg->port);

    skaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    vlan_err("%d socket chan bind socket:%lld, addr:%u, port:%u\n",
        chan->skcfg->type,
        chan->skcfg->fd,
        skaddr->sin_addr.s_addr,
        chan->skcfg->port);

    ret = mdrv_bind((int)chan->skcfg->fd, (struct sockaddr *)skaddr, sizeof(struct sockaddr));
    if (ret) {
        vlan_err("%d socket chan bind socket:%lld fail\n", chan->skcfg->type, chan->skcfg->fd);
        return ret;
    }

    ret = mdrv_listen((int)chan->skcfg->fd, SOCK_NUM_MAX);
    if (ret) {
        vlan_err("%d socket chan listen socket:%lld fail\n", chan->skcfg->type, chan->skcfg->fd);
        return ret;
    }

    vlan_err("%d socket chan bind and listen socket:%lld success\n", chan->skcfg->type, chan->skcfg->fd);
    return 0;
}

static void vlan_bind_udp_addr(struct chan_info *tcpchan, int skfd)
{
    int ret;
    struct chan_info *pchan;
    struct socketcfg *skcfg = NULL;
    int len = (int)sizeof(struct sockaddr_in);
    struct sockaddr_in *skaddr = NULL;

    if (tcpchan->chan_id != 0) {
        return;
    }

    pchan = &g_channels[1];
    if (pchan == NULL) {
        vlan_err("bind %d socket chan get null pair chan\n", tcpchan->skcfg->type);
        return;
    }

    skcfg = (struct socketcfg *)pchan->skcfg;
    if (skcfg == NULL) {
        vlan_err("udp channel uninit\n");
        return;
    }
    ret = mdrv_getpeername(skfd, (struct sockaddr *)&skcfg->skaddr, &len);
    if (ret != 0) {
        vlan_err("bind pair socket get peer name fail, ret:%d\n", ret);
        return;
    }

    skaddr = (struct sockaddr_in *)&skcfg->skaddr;
    vlan_err("old udp port:%d, addr:0x%x\n", skaddr->sin_port, skaddr->sin_addr.s_addr);
    skaddr->sin_port = htons(skcfg->port);
    vlan_err("modify udp port:%d, addr:0x%x\n", skaddr->sin_port, skaddr->sin_addr.s_addr);

    pchan->socket_state = 1;

    if (pchan->eventcb != NULL) {
        pchan->eventcb(ACM_EVT_DEV_READY);
    }
}

static void vlan_unbind_udp_addr(struct chan_info *tcpchan)
{
    struct chan_info *pchan;

    if (tcpchan->chan_id != 0) {
        return;
    }

    pchan = &g_channels[1];
    if (pchan == NULL) {
        vlan_err("bind %d socket chan get null pair chan\n", tcpchan->skcfg->type);
        return;
    }

    pchan->socket_state = 0;

    if (pchan->eventcb != NULL) {
        pchan->eventcb(ACM_EVT_DEV_SUSPEND);
    }
}

static int vlan_socket_connect(struct chan_info *chan, int *acceptfd)
{
    int tmpfd;
    struct sockaddr_in from = {0};
    int oldfd = *acceptfd;
    int len = (int)sizeof(struct sockaddr_in);

    tmpfd = mdrv_accept((int)chan->skcfg->fd, (struct sockaddr *)&from, &len);
    if (tmpfd < 0) {
        vlan_err("chan type:%d listen fd:%lld accept fail, ret:%d\n", chan->skcfg->type, chan->skcfg->fd, tmpfd);
        return 15;
    }
    vlan_err("accept from port:%d, addr:0x%x\n", from.sin_port, from.sin_addr.s_addr);

    if (oldfd != -1) {
        mdrv_shutdown(oldfd, SHUT_RDWR);
        mdrv_close(oldfd);
        vlan_err("new accept fd:%d connected, closed old accept fd:%d\n", tmpfd, oldfd);
    }

    *acceptfd = tmpfd;
    chan->skcfg->acpfd = tmpfd;

    chan->socket_state = 1;

    if (chan->eventcb != NULL) {
        chan->eventcb(ACM_EVT_DEV_READY);
    }

    vlan_bind_udp_addr(chan, *acceptfd);
    vlan_err("chan type:%d listen fd:%lld recv accept fd:%d connected\n", chan->skcfg->type, chan->skcfg->fd, *acceptfd);

    return 0;
}

static void vlan_socket_disconnect(struct chan_info *tcpchan, int acceptfd)
{
    mdrv_shutdown(acceptfd, SHUT_RDWR);
    mdrv_close(acceptfd);
    vlan_err("acceptfd:%d recv done, already closed\n", acceptfd);

    tcpchan->socket_state = 0;

    if (tcpchan->eventcb != NULL) {
        tcpchan->eventcb(ACM_EVT_DEV_SUSPEND);
    }

    vlan_unbind_udp_addr(tcpchan);
    vlan_err("chan type:%d listen fd:%lld recv accept fd:%d disconnected\n",
        tcpchan->skcfg->type,
        tcpchan->skcfg->fd,
        acceptfd);
}

static int vlan_socket_tcp_server(void *data)
{
    int ret, maxfd;
    sk_fd_set fdmap = {0};
    struct chan_info *chan = (struct chan_info *)data;
    struct socketcfg *skcfg = (struct socketcfg *)chan->skcfg;
    int listenfd = (int)skcfg->fd;
    int acceptfd = -1;

    vlan_err("vlan %d tcp server chan fd:%lld start\n", skcfg->type, skcfg->fd);
    for (;;) {
        fdmap.fds_bits = 0;
        SK_FD_SET(listenfd, &fdmap);
        if (acceptfd != -1) {
            SK_FD_SET(acceptfd, &fdmap);
        }

        maxfd = (acceptfd > listenfd) ? acceptfd : listenfd;

        ret = mdrv_select(maxfd + 1, &fdmap, NULL, NULL, NULL);
        if (ret < 0) {
            vlan_err("tcp socket select acceptfd:%d listenfd:%d err, ret:%d\n", acceptfd, listenfd, ret);
            continue;
        }

        if (SK_FD_ISSET(listenfd, &fdmap)) {
            ret = vlan_socket_connect(chan, &acceptfd);
            if (ret) {
                vlan_err("%d chan socket:%d connect fail, ret:%d\n", chan->skcfg->type, listenfd, ret);
                continue;
            }
        }

        if (acceptfd == -1) {
            continue;
        }

        ret = mdrv_recv(acceptfd, skcfg->recv_buf, (size_t)READ_BUFF_SIZE, 0);
        if (ret <= 0) {
            vlan_socket_disconnect(chan, acceptfd);
            skcfg->acpfd = -1;
            acceptfd = -1;
            continue;
        }
        skcfg->recv_len = ret;

        if (chan->readcb != NULL) {
            chan->readcb();
        }
    }

    return 0;
}

static int vlan_socket_chan_init(int chan_id, struct chan_info *chan)
{
    int ret;
    char sock_name[32] = {};

    chan->chan_id = chan_id;
    chan->skcfg = (struct socketcfg *)kmalloc(sizeof(struct socketcfg), GFP_KERNEL);
    chan->skcfg->port = g_socket_port[chan_id];
    chan->skcfg->family = AF_INET;
    chan->skcfg->type = chan_id == 1 ? SOCK_DGRAM : SOCK_STREAM;
    chan->skcfg->protocol = 0;
    ret = mdrv_socket(chan->skcfg->family, chan->skcfg->type, chan->skcfg->protocol);
    if (ret < 0) {
        vlan_err("socket %d chan create fail:%d\n", chan_id, ret);
    }
    chan->skcfg->fd = (u64)ret;
    chan->skcfg->acpfd = -1;

    if (chan->skcfg->type == SOCK_DGRAM) {
        return 0;
    }

    ret = vlan_socket_bind_listen(chan);
    if (ret) {
        mdrv_shutdown(chan->skcfg->fd, SHUT_RDWR);
        mdrv_close(chan->skcfg->fd);
    }
    ret = osl_task_init(vlan_socket_name(sock_name, "sockserver%d", chan->skcfg->type),
        0x4C,
        0x2000,
        (OSL_TASK_FUNC)vlan_socket_tcp_server,
        (void *)chan,
        &chan->skcfg->taskid);
    if (ret) {
        vlan_err("%d socket server create err:%d\n", chan->skcfg->type, ret);
        mdrv_shutdown(chan->skcfg->fd, SHUT_RDWR);
        mdrv_close(chan->skcfg->fd);
    }

    vlan_err("%d tcp protocol socket:%lld init success\n", chan->skcfg->type, chan->skcfg->fd);
    return 0;
}

static int vlan_socket_send(struct chan_info *chan, void *data, unsigned int len)
{
    int ret;
    struct socketcfg *skcfg = chan->skcfg;
    unsigned int sendlen;
    u8 *tmpdata = (u8 *)data;
    struct hisk_info hiskinfo = {0};

    if (skcfg->type == SOCK_DGRAM) {
        hiskinfo.fd = (int)skcfg->fd;
        while (len) {
            if (len > UDP_MAX_PACKAGE) {
                sendlen = UDP_MAX_PACKAGE;
            } else {
                sendlen = len;
            }
            hiskinfo.buff = tmpdata;
            hiskinfo.len = (size_t)sendlen;
            ret = mdrv_sendto(&hiskinfo, 0, &skcfg->skaddr, sizeof(struct sockaddr));
            if (ret != sendlen) {
                vlan_err("%d udp socket send fail:%d(%d)\n", skcfg->type, ret, sendlen);
                return ret;
            }

            tmpdata += sendlen;
            len -= sendlen;
        }
    } else {
        if (skcfg->acpfd == -1) {
            vlan_err("%d type tcp chan have invalid acpfd\n", skcfg->type);
        }

        ret = mdrv_send((int)skcfg->acpfd, data, (size_t)len, 0);
        if (ret != (int)len) {
            vlan_err("%d type tcp socket send fail:%d(%d)\n", skcfg->type, ret, len);
            return len;
        }
    }
    return 0;
}

static int vlan_socket_chan_release(struct chan_info *chan)
{
    if (chan->skcfg->taskid != NULL) {
        kthread_stop(chan->skcfg->taskid);
    }
    if (chan->skcfg->recv_buf != NULL) {
        kfree(chan->skcfg->recv_buf);
    }
    kfree(chan->skcfg);

    return 0;
}

int esl_version_check_for_vlan(void)
{
    const bsp_version_info_s *verinfo = bsp_get_version_info();
    if ((verinfo != NULL) && ((verinfo->plat_type == PLAT_ESL))) {
        return ESL_VERSION_SUPPORT_VLAN;
    }
    return 0;
}

int bvp_vlan_reg_enablecb(usb_udi_enable_cb func)
{
    int i;
    for (i = 0; i < USB_MAX_NUM; i++) {
        if (g_enable_cb[i] == NULL) {
            g_enable_cb[i] = func;
            if (g_usb_state == 1) {
                func();
            }
            return 0;
        }
    }
    vlan_err("mdrv_usb_reg_enablecb error\n");
    return -1;
}

int bvp_vlan_reg_disablecb(usb_udi_disable_cb func)
{
    int i;
    for (i = 0; i < USB_MAX_NUM; i++) {
        if (g_disable_cb[i] == NULL) {
            g_disable_cb[i] = func;
            return 0;
        }
    }
    vlan_err("mdrv_usb_reg_disablecb error\n");
    return -1;
}

void *bvp_vlan_open(u32 dev_id)
{
    if (dev_id >= USB_MAX_NUM) {
        vlan_err("dev_id error, channel open failed\n");
        return NULL;
    }
    vlan_err("bsp vlan open dev_id = %d\n", dev_id);
    if (dev_id == UDI_USB_ACM_GPS) {
        vlan_socket_chan_init(HIDS_CMD, &g_channels[HIDS_CMD]);
        return (void *)&g_channels[HIDS_CMD];
    } else if (dev_id == UDI_USB_ACM_LTE_DIAG) {
        vlan_socket_chan_init(HIDS_DATA, &g_channels[HIDS_DATA]);
        return (void *)&g_channels[HIDS_DATA];
    } else if (dev_id == VLAN_TEST) {
        vlan_socket_chan_init(VLAN_TEST, &g_channels[VLAN_TEST]);
        return (void *)&g_channels[VLAN_TEST];
    } else {
        vlan_err("vlan : undefined channel id\n");
        return NULL;
    }
    return (void *)&g_channels[dev_id];
}

s32 bvp_vlan_close(void *handle)
{
    if (handle == NULL) {
        vlan_err("vlan close error, handle is null\n");
        return -1;
    }
    vlan_err("bsp vlan close\n");
    vlan_socket_chan_release((struct chan_info *)handle);
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

static int vlan_ioctl_send(struct chan_info *channel, struct acm_wr_async_info *wr_info)
{
    int ret;
    int act_send_len;

    ret = vlan_socket_send(channel, wr_info->virt_addr, wr_info->size);
    if (ret != 0) {
        if (ret < 0) {
            vlan_err("socket send failed, act send len = 0\n");
            act_send_len = 0;
        }
        else {
            vlan_err("socket send failed, act send len = %d\n", ret);
            act_send_len = ret;
        }
        if (channel->writecb != NULL) {
            channel->writecb(wr_info->virt_addr, NULL, act_send_len);
        }
        return ret;
    }
    if (channel->writecb != NULL) {
        channel->writecb(wr_info->virt_addr, NULL, wr_info->size);
    }
    return 0;
}

s32 bvp_vlan_ioctl(void *handle, u32 cmd, void *para)
{
    struct chan_info *channel = (struct chan_info *)handle;
    struct acm_wr_async_info *read_info;
    struct acm_wr_async_info *wr_info;
    struct acm_read_buff_info *recv_buff;
    int *buff_addr;
    int ret;

    if (handle == NULL) {
        vlan_err("vlan ioctl error, handle is null\n");
        return -1;
    }

    switch (cmd) {
        case ACM_IOCTL_RELLOC_READ_BUFF:
            recv_buff = (struct acm_read_buff_info *)para;
            buff_addr = kmalloc(READ_BUFF_SIZE, GFP_KERNEL | GFP_DMA);
            channel->skcfg->recv_buf = buff_addr;
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
            read_info = (struct acm_wr_async_info *)para;
            read_info->virt_addr = channel->skcfg->recv_buf;
            read_info->size = channel->skcfg->recv_len;
            break;
        case ACM_IOCTL_RETURN_BUFF:
            channel->skcfg->recv_len = 0;
            break;
        case ACM_IOCTL_WRITE_ASYNC:
            wr_info = (struct acm_wr_async_info *)para;
            ret = vlan_ioctl_send(channel, wr_info);
            if (ret != 0) {
                return ret;
            }
            break;
        default:
            vlan_err("vlan : ioctl cmd = %d, undefined\n", cmd);
            break;
    }
    return 0;
}

static int __init vlan_adapter_init(void)
{
    vlan_err("vlan adapter init\n");
    g_usb_state = 1;
    return 0;
}

module_init(vlan_adapter_init);
MODULE_LICENSE("GPL");
