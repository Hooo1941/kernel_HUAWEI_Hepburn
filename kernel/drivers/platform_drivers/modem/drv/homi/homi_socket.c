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
#include <linux/net.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <osl_list.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <mdrv_socket.h>
#include <bsp_homi.h>
#include <bsp_version.h>
#include <bsp_dfxcert.h>
#include "homi_core.h"
#include "homi_debug.h"
#include "homi_socket.h"

/* must same order to homi_chan_type of socket */
static int g_socketport[] = {3000, 20249, 20250, 20253};

socket_verify_info_s g_socket_verify_info;

static struct socket_chan_cfg g_tcp_socket_cfg = {
    .family = AF_INET,
    .type = SOCK_STREAM,
    .protocol = 0,
};

static struct socket_chan_cfg g_udp_socket_cfg = {
    .family = AF_INET,
    .type = SOCK_DGRAM,
    .protocol = 0,
};

static int socket_cfg_init(struct homi_chan *chan, struct socket_chan_cfg *skcfg)
{
    struct socket_chan_cfg *cfg = NULL;
    switch (chan->type) {
        case HOMI_CH_SK_3000:
#ifdef CONFIG_OM_SOCKET_AT_PORT
        case HOMI_CH_SK_20249:
#endif
        case HOMI_CH_SK_20250:
            cfg = &g_tcp_socket_cfg;
            break;
        case HOMI_CH_SK_20253:
            cfg = &g_udp_socket_cfg;
            break;
        default:
            homi_err("socket cfg invalid, chan type:%d\n", chan->type);
            return BSP_HOMI_SK_TYPE_ERR;
    }

    if (memcpy_s(skcfg, sizeof(struct socket_chan_cfg), cfg, sizeof(struct socket_chan_cfg))) {
        homi_err("socket cfg copy fail\n");
    }

    skcfg->port = g_socketport[chan->type - HOMI_CH_SK_BASE];

    skcfg->buf = osl_malloc(chan->blength);
    if (skcfg->buf == NULL) {
        homi_err("socket cfg buf malloc fail\n");
        return BSP_HOMI_SK_BUF_MALLOC_ERR;
    }

    return 0;
}

static void socket_cfg_release(struct socket_chan_cfg *skcfg)
{
    if (skcfg->buf != NULL) {
        osl_free(skcfg->buf);
    }

    osl_free(skcfg);
}

static int socket_bind_listen(struct homi_chan *chan, struct socket_chan_cfg *skcfg)
{
    int ret;
    struct sockaddr_in *skaddr = (struct sockaddr_in *)&skcfg->skaddr;

    skaddr->sin_family = skcfg->family;
    skaddr->sin_port = htons(skcfg->port);

    skaddr->sin_addr.s_addr = g_socket_verify_info.ip_addr;
    homi_printf("%d socket chan bind socket:%lld, addr:%u, port:%u\n",
        chan->type, chan->fd, skaddr->sin_addr.s_addr, skcfg->port);

    ret = mdrv_bind((int)chan->fd, (struct sockaddr *)skaddr, sizeof(struct sockaddr));
    if (ret) {
        homi_err("%d socket chan bind socket:%lld fail\n", chan->type, chan->fd);
        return ret;
    }

    ret = mdrv_listen((int)chan->fd, SOCKET_NUM_MAX);
    if (ret) {
        homi_err("%d socket chan listen socket:%lld fail\n", chan->type, chan->fd);
        return ret;
    }

    homi_printf("%d socket chan bind and listen socket:%lld success\n", chan->type, chan->fd);
    return 0;
}

char *socket_name(char name[], char *namefmt, ...)
{
    int ret;
    va_list args;

    va_start(args, namefmt);
    ret = vsnprintf_s(name, (size_t)SOCKSRVNAME_LEN, (size_t)SOCKSRVNAME_LEN - 1, namefmt, args);
    va_end(args);

    return (ret ? "sockserver" : name);
}

/* must put chan after used */
static struct homi_chan *socket_get_pair(struct homi_chan *tcpchan)
{
    struct homi_chan *pchan = NULL;
    enum homi_chan_type ctype;

    switch (tcpchan->type) {
        case HOMI_CH_SK_3000:
            ctype = HOMI_CH_SK_20253;
            break;
        default:
            homi_debug(HLEVDBG, "use invalid pair type:%d get pair socket\n", tcpchan->type);
            return NULL;
    }

    /* must put chan after used */
    pchan = homi_get_valid_chan_by_type(HOMI_PORT_SOCKET, ctype);
    if (pchan == NULL) {
        homi_err("get pair socket chan fail\n");
        return NULL;
    }

    return pchan;
}

static void socket_bind_udpchan(struct homi_chan *tcpchan, int skfd)
{
    int ret;
    struct homi_chan *pchan;
    struct socket_chan_cfg *skcfg = NULL;
    int len = (int)sizeof(struct sockaddr_in);
    struct sockaddr_in *skaddr = NULL;

    /* must put chan after used */
    pchan = socket_get_pair(tcpchan);
    if (pchan == NULL) {
        homi_debug(HLEVDBG, "bind %d socket chan get null pair chan\n", tcpchan->type);
        return;
    }

    skcfg = (struct socket_chan_cfg *)pchan->private;
    ret = mdrv_getpeername(skfd, (struct sockaddr *)&skcfg->skaddr, &len);
    if (ret != 0) {
        homi_err("bind pair socket get peer name fail, ret:%d\n", ret);
        goto put_chan;
    }

    skaddr = (struct sockaddr_in *)&skcfg->skaddr;
    homi_err("old udp port:%d, addr:0x%x\n", skaddr->sin_port, skaddr->sin_addr.s_addr);
    skaddr->sin_port = htons(skcfg->port);
    homi_err("modify udp port:%d, addr:0x%x\n", skaddr->sin_port, skaddr->sin_addr.s_addr);

    osl_sem_down(&pchan->chsem);
    pchan->state = HOMI_CHSTATE_CONNECTED;
    osl_sem_up(&pchan->chsem);

    if (pchan->cb.eventcb != NULL) {
        pchan->cb.eventcb(HOMI_PORT_SOCKET, HOMI_CHSTATE_CONNECTED);
    }

put_chan:
    homi_chan_put(pchan);
    return;
}

static int socket_tcp_connect(struct homi_chan *tcpchan, int *acceptfd)
{
    int tmpfd;
    struct sockaddr_in from = { 0 };
    int oldfd = *acceptfd;
    int len = (int)sizeof(struct sockaddr_in);

    tmpfd = mdrv_accept((int)tcpchan->fd, (struct sockaddr *)&from, &len);
    if (tmpfd < 0) {
        homi_err("chan type:%d listen fd:%lld accept fail, ret:%d\n", tcpchan->type, tcpchan->fd, tmpfd);
        return BSP_HOMI_SK_ACCEPT_ERR;
    }
    homi_err("accept from port:%d, addr:0x%x\n", from.sin_port, from.sin_addr.s_addr);

    if (oldfd != -1) {
        mdrv_shutdown(oldfd, SHUT_RDWR);
        mdrv_close(oldfd);
        homi_err("new accept fd:%d connected, closed old accept fd:%d\n", tmpfd, oldfd);
    }

    *acceptfd = tmpfd;
    tcpchan->acpfd = tmpfd;

    osl_sem_down(&tcpchan->chsem);
    tcpchan->state = HOMI_CHSTATE_CONNECTED;
    osl_sem_up(&tcpchan->chsem);

    if (tcpchan->cb.eventcb != NULL) {
        tcpchan->cb.eventcb(HOMI_PORT_SOCKET, HOMI_CHSTATE_CONNECTED);
    }

    /* bind acceptfd address to the udp protocol chan relating to this tcp chan.
     * set udp protocol socket in connected state.
     */
    socket_bind_udpchan(tcpchan, *acceptfd);
    homi_printf("chan type:%d listen fd:%lld recv accept fd:%d connected\n", tcpchan->type, tcpchan->fd, *acceptfd);

    return 0;
}

static void socket_unbind_udpchan(struct homi_chan *tcpchan)
{
    struct homi_chan *pchan;

    pchan = socket_get_pair(tcpchan);
    if (pchan == NULL) {
        homi_err("unbind udp protocol chan get pair chan fail\n");
        return;
    }

    osl_sem_down(&pchan->chsem);
    pchan->state = HOMI_CHSTATE_DISCONNECTED;
    osl_sem_up(&pchan->chsem);

    if (pchan->cb.eventcb != NULL) {
        pchan->cb.eventcb(HOMI_PORT_SOCKET, HOMI_CHSTATE_DISCONNECTED);
    }

    homi_chan_put(pchan);
    return;
}

static void socket_tcp_disconnect(struct homi_chan *tcpchan, int acceptfd)
{
    mdrv_shutdown(acceptfd, SHUT_RDWR);
    mdrv_close(acceptfd);
    homi_printf("acceptfd:%d recv done, already closed\n", acceptfd);

    osl_sem_down(&tcpchan->chsem);
    tcpchan->state = HOMI_CHSTATE_DISCONNECTED;
    osl_sem_up(&tcpchan->chsem);

    if (tcpchan->cb.eventcb != NULL) {
        tcpchan->cb.eventcb(HOMI_PORT_SOCKET, HOMI_CHSTATE_DISCONNECTED);
    }

    /* unbind acceptfd address to the udp protocol chan relating to this tcp chan
     * set udp protocol socket in disconnected state.
     */
    socket_unbind_udpchan(tcpchan);
    homi_printf("chan type:%d listen fd:%lld recv accept fd:%d disconnected\n", tcpchan->type, tcpchan->fd, acceptfd);
}

static int socket_tcp_server(void *data)
{
    int ret, maxfd, i;
    sk_fd_set fdmap = { 0 };
    struct homi_chan *chan = (struct homi_chan *)data;
    struct socket_chan_cfg *skcfg = (struct socket_chan_cfg *)chan->private;
    int listenfd = (int)chan->fd;
    int acceptfd = -1;

    if (bsp_get_version_info()->plat_type == PLAT_ASIC) {
        wait_event(g_socket_verify_info.wq, ((g_socket_verify_info.can_conn == true) && (g_socket_verify_info.ip_addr != 0)));
        ret = socket_bind_listen(chan, skcfg);
        if (ret) {
            homi_err("socket address bind and listen err:%d\n", ret);
            goto shutdown;
        }
    }   

    homi_debug(HLEVDBG, "homi %d tcp server chan fd:%lld start\n", chan->type, chan->fd);
    for (;;) {
        fdmap.fds_bits = 0;
        SK_FD_SET(listenfd, &fdmap);
        if (acceptfd != -1) {
            SK_FD_SET(acceptfd, &fdmap);
        }

        maxfd = (acceptfd > listenfd) ? acceptfd : listenfd;

        if ((bsp_get_version_info()->plat_type == PLAT_ASIC) && (g_socket_verify_info.can_conn == false)) {
            homi_err("socket disconn, chan type:%d, listenfd:%d, acceptfd:%d, port:%d, addr:%u\n",
                chan->type, listenfd, acceptfd, skcfg->port, ((struct sockaddr_in *)&skcfg->skaddr)->sin_addr.s_addr);
                goto socket_disconn;           
        }

        ret = mdrv_select(maxfd + 1, &fdmap, NULL, NULL, NULL);
        if (ret < 0) {
            homi_err("tcp socket select acceptfd:%d listenfd:%d err, ret:%d\n", acceptfd, listenfd, ret);
            continue;
        }

        if (SK_FD_ISSET(listenfd, &fdmap)) {
            ret = socket_tcp_connect(chan, &acceptfd);
            if (ret) {
                homi_err("%d chan socket:%d connect fail, ret:%d\n", chan->type, listenfd, ret);
                continue;
            }
        }

        if (acceptfd == -1) {
            continue;
        }

        ret = mdrv_recv(acceptfd, skcfg->buf, (size_t)chan->blength, 0);
        if (ret <= 0) {
            socket_tcp_disconnect(chan, acceptfd);
            chan->acpfd = -1;
            acceptfd = -1;
            continue;
        }

        if (chan->cb.recvcb != NULL) {
            chan->cb.recvcb(chan, skcfg->buf, ret);
        }
    }

    return 0;
socket_disconn:
    for (i = listenfd + 1; i <= maxfd; i++) {
        if (SK_FD_ISSET(i, &fdmap)) {
            socket_tcp_disconnect(chan, i);
            homi_printf("acceptfd:%d disconnect\n", i);
        }
    }

shutdown:
    (void)mdrv_shutdown(listenfd, SHUT_RDWR);
    (void)mdrv_close(listenfd);
    socket_cfg_release(skcfg);

    return -1;
}

#define IP_ADDR_127_0_0_1 16777343
static int socket_ipaddr_notifier_fn(struct notifier_block *nb, unsigned long action, void *data)
{
    struct in_ifaddr *ina = (struct in_ifaddr *)data;

	homi_err("action=%lu\n", action);
	if ((ina == NULL) || action != NETDEV_UP) {
        return 0;
    }
		
	homi_err("ifa_label=%s, ifa_address = %u\n", ina->ifa_label, ina->ifa_address);

    if (ina->ifa_address == IP_ADDR_127_0_0_1) {
       homi_err("ifa_address = 127.0.0.1\n");
       return 0; 
    }

    if (g_socket_verify_info.ip_addr == 0) {
        if (ina->ifa_address != 0) {
            g_socket_verify_info.ip_addr = ina->ifa_address;
            wake_up(&g_socket_verify_info.wq);
            homi_err("g_socket_verify_info.ip_addr=%u\n", g_socket_verify_info.ip_addr);  
        }
    } else {
        g_socket_verify_info.ip_addr = ina->ifa_address;
        if (ina->ifa_address != 0) {            
            wake_up(&g_socket_verify_info.wq);
            homi_err("ip_addr is changed:%u\n", g_socket_verify_info.ip_addr);
        } else {
            g_socket_verify_info.can_conn = false;
            homi_err("ifa_address=0,can_conn=%d\n", g_socket_verify_info.can_conn);
        }
    }

	return 0;
}

int socket_register_inetaddr_notifier(enum homi_chan_type type)
{
    int ret;

    g_socket_verify_info.can_conn = false;

    if (type == HOMI_CH_SK_3000) {
        init_waitqueue_head(&g_socket_verify_info.wq);
        g_socket_verify_info.ipaddr_notifier.notifier_call = socket_ipaddr_notifier_fn;
        ret = register_inetaddr_notifier(&g_socket_verify_info.ipaddr_notifier);
        if (ret) {
            homi_err("register_inetaddr_notifier fail, ret=0x%x\n", ret);
            return ret; 
        }
        homi_err("register_inetaddr_notifier success, type=0x%x\n", type);
    }
    
    return 0;    
}

static int socket_dfxcert_verify_fn(struct notifier_block *nb, unsigned long action, void *data)
{
    int verify = bsp_dfxcert_status_get(DIAG_SOCKET);

    if (verify == DFXCERT_PORT_SUPPORT) {
        g_socket_verify_info.can_conn = true;
        wake_up(&g_socket_verify_info.wq);           
    } else {
        g_socket_verify_info.can_conn = false;

    }
    
    homi_err("socket_dfxcert_verify_fn, can_conn=0x%x\n", g_socket_verify_info.can_conn);
    return 0;
}

static void socket_register_dfxcert_notifier(void)
{
    if (g_socket_verify_info.dfxcert_notifer.notifier_call == NULL) {
        g_socket_verify_info.dfxcert_notifer.notifier_call = socket_dfxcert_verify_fn; 
        bsp_dfxcert_verify_notifier_register(&g_socket_verify_info.dfxcert_notifer);
    }
}

static int socket_bind_listen_proc(struct homi_chan *chan, struct socket_chan_cfg *skcfg)
{
    int ret;

    if (bsp_get_version_info()->plat_type == PLAT_ASIC) {
        ret = socket_register_inetaddr_notifier(chan->type);
        if (ret) {
            return ret;        
        }
        socket_register_dfxcert_notifier();
    } else {
        ret = socket_bind_listen(chan, skcfg);
        if (ret) {
            homi_err("socket address bind and listen err:%d\n", ret);
        }  
    }

    return ret;
}

static int socket_chan_init(struct homi_chan *chan)
{
    int ret;
    struct socket_chan_cfg *skcfg;
    char name[SOCKSRVNAME_LEN] = { 0 };

    skcfg = (struct socket_chan_cfg *)osl_malloc(sizeof(struct socket_chan_cfg));
    if (skcfg == NULL) {
        homi_err("socket init malloc skcfg fail\n");
        return BSP_HOMI_SK_CFG_MALLOC_ERR;
    }
    (void)memset_s(skcfg, sizeof(struct socket_chan_cfg), 0, sizeof(struct socket_chan_cfg));

    ret = socket_cfg_init(chan, skcfg);
    if (ret) {
        homi_err("socket %d chan config get fail\n", chan->type);
        goto cfg_release;
    }

    ret = mdrv_socket(skcfg->family, skcfg->type, skcfg->protocol);
    if (ret < 0) {
        homi_err("socket %d chan create fail:%d\n", chan->type, ret);
        goto cfg_release;
    }

    chan->fd = (u64)ret;
    chan->acpfd = -1;
    if (skcfg->type == SOCK_DGRAM) {
        chan->private = skcfg;
        homi_printf("%d udp protocol socket fd:%lld init success\n", chan->type, chan->fd);
        return 0;
    }

    ret = socket_bind_listen_proc(chan, skcfg);
    if (ret) {
        goto shutdown;
    }   
    chan->private = (void *)skcfg; /* must set skcfg to private before task create */

    ret = osl_task_init(socket_name(name, "sockserver%d", chan->type), 0x4C, \
        0x2000, (OSL_TASK_FUNC)socket_tcp_server, (void *)chan, &skcfg->taskid);
    if (ret) {
        homi_err("%d socket server create err:%d\n", chan->type, ret);
        goto shutdown;
    }

    homi_printf("%d tcp protocol socket:%lld init success\n", chan->type, chan->fd);
    return 0;

shutdown:
    mdrv_shutdown(chan->fd, SHUT_RDWR);
    mdrv_close(chan->fd);

cfg_release:
    socket_cfg_release(skcfg);
    return ret;
}

static int socket_tcp_send(struct homi_chan *chan, void *data, unsigned int len)
{
    int ret;

    if (chan->acpfd == -1) {
        homi_err("%d type tcp chan have invalid acpfd\n", chan->type);
        return BSP_HOMI_SK_INVALID_ACPFD_ERR;
    }

    ret = mdrv_send((int)chan->acpfd, data, (size_t)len, 0);
    if (ret != (int)len) {
        homi_err("%d type tcp socket send fail:%d(%d)\n", chan->type, ret, len);
        return len;
    }

    homi_debug(HLEVDBG, "%d type tcp socket send data success\n", chan->type);
    return 0;
}

static int socket_udp_send(struct homi_chan *chan, void *data, unsigned int len)
{
    int ret;
    unsigned int sendlen;
    u8 *tmpdata = (u8 *)data;
    struct socket_chan_cfg *skcfg = (struct socket_chan_cfg *)chan->private;
    struct hisk_info hiskinfo = { 0 };

    hiskinfo.fd = (int)chan->fd;

    while (len) {
        if (len > SOCKET_UDP_PACKAGE) {
            sendlen = SOCKET_UDP_PACKAGE;
        } else {
            sendlen = len;
        }

        hiskinfo.buff = tmpdata;
        hiskinfo.len = (size_t)sendlen;
        ret = mdrv_sendto(&hiskinfo, 0, &skcfg->skaddr, sizeof(struct sockaddr));
        if (ret != sendlen) {
            homi_err("%d udp socket send fail:%d(%d)\n", chan->type, ret, sendlen);
            return ret;
        }

        tmpdata += sendlen;
        len -= sendlen;
        homi_debug(HLEVDBG, "%d udp socket send %d data success, %d data left\n", chan->type, sendlen, len);
    }

    homi_debug(HLEVDBG, "%d udp socket send all data success\n", chan->type);
    return 0;
}

static int socket_chan_send(struct homi_chan *chan, void *data, unsigned int len)
{
    int ret;
    struct socket_chan_cfg *skcfg = NULL;

    /* must put chan after used */
    if (!homi_chan_test_and_get(chan, HOMI_CHSTATE_CONNECTED)) {
        homi_err("%d socket chan not connected, can't send data\n", chan->type);
        return BSP_HOMI_CHAN_NOT_CONNECT_ERR;
    }

    skcfg = (struct socket_chan_cfg *)chan->private;
    switch (skcfg->type) {
        case SOCK_STREAM:
            ret = socket_tcp_send(chan, data, len);
            break;
        case SOCK_DGRAM:
            ret = socket_udp_send(chan, data, len);
            break;
        default:
            homi_err("%d socket data send use invalid protocol type:%d\n", chan->type, skcfg->type);
            homi_chan_put(chan);
            return BSP_HOMI_SK_SEND_TYPE_ERR;
    }

    if (ret) {
        homi_err("%d socket data send fail:%d\n", chan->type, ret);
    }

    homi_chan_put(chan);
    return ret;
}

static int socket_chan_release(struct homi_chan *chan)
{
    struct socket_chan_cfg *skcfg = (struct socket_chan_cfg *)chan->private;

    if (skcfg->taskid != NULL) {
        kthread_stop(skcfg->taskid);
    }

    socket_cfg_release(skcfg);

    return 0;
}

static struct homi_chan_ops g_socket_chan_ops = {
    .init = socket_chan_init,
    .send = socket_chan_send,
    .release = socket_chan_release,
};

static struct homi_chan *homi_socket_chan_alloc(struct homi_port *port, struct homi_cfg *cfg)
{
    struct homi_chan *chan;

    chan = homi_chan_init(port, &g_socket_chan_ops, cfg);
    if (chan == NULL) {
        homi_err("homi socket chan alloc init fail\n");
        return NULL;
    }

    osl_sem_down(&chan->chsem);
    if (chan->state == HOMI_CHSTATE_INITING) {
        chan->state = HOMI_CHSTATE_DISCONNECTED;
    }
    osl_sem_up(&chan->chsem);

    return chan;
}

static int homi_socket_chan_release(struct homi_port *port, struct homi_chan *chan)
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

int homi_socket_init(void)
{
    int ret;
    struct homi_port *port;

    port = homi_port_alloc();
    if (port == NULL) {
        homi_err("homi socket port alloc fail\n");
        return BSP_HOMI_PORT_MALLOC_ERR;
    }

    port->type = HOMI_PORT_SOCKET;

    port->alloc = homi_socket_chan_alloc;
    port->release = homi_socket_chan_release;

    /* register port at last */
    ret = homi_port_register(port);
    if (ret) {
        osl_free(port);
        homi_err("homi socket port register fail, ret:%d\n", ret);
        return ret;
    }

    homi_printf("homi socket port init success\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(homi_socket_init);
#endif
