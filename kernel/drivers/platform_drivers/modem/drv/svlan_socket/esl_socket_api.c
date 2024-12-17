/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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


#include "product_config.h"

#ifdef FEATURE_SVLSOCKET
#include <linux/of.h>
#include "esl_socket_channel_ctr.h"
#include "esl_socket_api.h"
#include "securec.h"
#include <bsp_print.h>
#include <osl_types.h>
#include <bsp_version.h>
#include <osl_malloc.h>

#define HIGH32(a) ((unsigned long)(((a) >> 32) & 0xffffffff))
#define LOW32(a) ((unsigned long)((a)&0xffffffff))

#define COMPLETED_EXEC(val) (((val) & (1 << 31)) == 0)
#define IRQ_CLEAR(ret) (!((ret) >> 31))

#define SIMSOCK_OP_START (1 << 31)
#define INT_TIMEOVER 1
#define THIS_MODU mod_socket
#define SOCKET_DBG(format, arg...)                       \
    do {                                                 \
        if (DISABLE_SOCKET_DEBUG == g_simsock_debug)     \
            bsp_info(format, ##arg);                     \
        else if (ENABLE_SOCKET_DEBUG == g_simsock_debug) \
            bsp_err(format, ##arg);                      \
    } while (0)

#define MAX_CMD 14
osl_sem_id g_sem_code[SIMSOCK_CHAN_MAX][MAX_CMD];

enum copy_mode { map_single_mode = 0, alloc_mode, dirc_trans_mode };

enum simsock_cmd {
    simsock_cmd_socket = 0,
    simsock_cmd_select,
    simsock_cmd_getpeername,
    simsock_cmd_setsockopt,
    simsock_cmd_bind,
    simsock_cmd_listen,
    simsock_cmd_accept,
    simsock_cmd_connect,
    simsock_cmd_send,
    simsock_cmd_recv,
    simsock_cmd_sendto,
    simsock_cmd_recvfrom,
    simsock_cmd_shutdown,
    simsock_cmd_close,
    simsock_cmd_max
};

int g_simsock_debug = DISABLE_SOCKET_DEBUG;
struct simsock_dbg_table {
    // interface func count
    atomic_t sock;
    atomic_t bind;
    atomic_t listen;
    atomic_t accept_in;
    atomic_t accept_out;
    atomic_t select_in;
    atomic_t select_out;
    atomic_t send;
    atomic_t recv_in;
    atomic_t recv_out;
    atomic_t sendto;
    atomic_t getpeername;
    atomic_t shutdown;
    atomic_t close;
    // core func count
    atomic_t sockregfd;
    atomic_t sockunregfd;
    atomic_t sockgetfd;
    atomic_t sockputfd;
};
struct simsock_dbg_table g_simsockdbg;

struct simsock_ctx g_simsock_ctx;

#define hi_get_sockfd(fd) g_simsock_ctx.socks[fd].sockfd

static unsigned long long g_simsock_dma_mask = 0xffffffffULL;

struct g_simsock_dbg_t {
    int svlan_module_init_begin;
    int platform_ret;
    int svlan_module_init_end;
    int svlan_probe_begin;
    int svlan_probe_end;
};
struct g_simsock_dbg_t g_simsock_dbg = {0};

struct simsock_ctx *get_simsock_config(void)
{
    return &g_simsock_ctx;
}

void find_complete_cmd(unsigned int value, int ch)
{
    int i;
    for (i = 0; i < MAX_CMD; i++) {
        if (((value >> i) & 0x1) == 0x1) {
            osl_sem_up(&g_sem_code[ch][i]);
            return;
        }
    }
}

irqreturn_t sock_interrupt(int irq, void *dev)
{
    unsigned int status;
    unsigned int value;
    int i;

    status = simsock_readl(g_simsock_ctx.simsock_iqrstatus);

    for (i = 0; i < SIMSOCK_CHAN_MAX; i++) {
        if ((status & (1 << i))) {
            value = simsock_readl((void *)(SIMSOCK_INTRAW(i) + g_simsock_ctx.simsock_regs));
            simsock_writel(value, (void *)(SIMSOCK_INTRAW(i) + g_simsock_ctx.simsock_regs));
            find_complete_cmd(value, i);
        }
    }

    return SIM_IRQ_HANDLED;
}

inline int simsock_chan_complete(unsigned int ch)
{
    unsigned int value;
    int timeout = 1000;

    do {
        value = simsock_readl(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs);
        if (COMPLETED_EXEC(value)) {
            break;
        }

        sim_sleep(1);
    } while (timeout--);

    if (timeout <= 0) {
        return SIMSOCK_FAIL;
    }

    return SIMSOCK_OK;
}

int simsock_create(int family, int type, int protocol)
{
    int ret;
    int ch;
    struct simsock_chan *chan = NULL;

    ret = simsock_chan_alloc(0, simsock_cmd_socket);
    if (ret < 0)
        return ret;

    ch = ret;
    chan = &g_simsock_ctx.chan[ch];

    simsock_writel(family, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(type, (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(protocol, (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_socket | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    ret = simsock_chan_complete(ch);
    if (ret) {
        return ret;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    chan->sockfd = ret;

    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_bind(int sockfd, struct sockaddr *addr, int addrlen)
{
    int ret;
    int ch;
    sim_dma_addr_t dma_phy;

    if (addr == NULL) {
        return SIMSOCK_FAIL;
    }

    ret = simsock_chan_alloc(sockfd, simsock_cmd_bind);
    if (ret < 0)
        return ret;

    ch = ret;

    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    dma_phy = sim_dma_map_single(g_simsock_ctx.dev, addr, sizeof(struct sockaddr), SIM_DMA_TO_DEVICE);
    simsock_writel(LOW32(dma_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_phy), (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(addrlen, (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));

    simsock_writel(simsock_cmd_bind | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    ret = simsock_chan_complete(ch);

    sim_dma_unmap_single(g_simsock_ctx.dev, dma_phy, sizeof(struct sockaddr), SIM_DMA_FROM_DEVICE);

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));

    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_listen(int sockfd, int backlog)
{
    int ret;
    int ch;

    ret = simsock_chan_alloc(sockfd, simsock_cmd_listen);
    if (ret < 0)
        return ret;
    ch = ret;
    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(backlog, (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_listen | SIMSOCK_OP_START, SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs);
    ret = simsock_chan_complete(ch);

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_accept(int sockfd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
    int ret;
    int ch;
    int sleep_stats;
    sim_dma_addr_t dma_upaddr_phy;
    sim_dma_addr_t dma_addrlen_phy;

    if ((upeer_sockaddr == NULL) || (upeer_addrlen == NULL)) {
        return SIMSOCK_FAIL;
    }

    ret = simsock_chan_alloc(sockfd, simsock_cmd_accept);
    if (ret < 0)
        return ret;
    ch = ret;

    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    dma_upaddr_phy = sim_dma_map_single(g_simsock_ctx.dev, upeer_sockaddr, sizeof(struct sockaddr), SIM_DMA_TO_DEVICE);
    simsock_writel(LOW32(dma_upaddr_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_upaddr_phy), (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));
    dma_addrlen_phy = sim_dma_map_single(g_simsock_ctx.dev, upeer_addrlen, sizeof(int), SIM_DMA_TO_DEVICE);
    simsock_writel(LOW32(dma_addrlen_phy), (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_addrlen_phy), (void *)(SIMSOCK_PARAM4(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_accept | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    sim_dma_unmap_single(g_simsock_ctx.dev, dma_upaddr_phy, sizeof(struct sockaddr), SIM_DMA_FROM_DEVICE);
    sim_dma_unmap_single(g_simsock_ctx.dev, dma_addrlen_phy, sizeof(struct sockaddr), SIM_DMA_FROM_DEVICE);

    if (down_interruptible(&g_sem_code[ch][simsock_cmd_accept])) {
        sleep_stats = simsock_readl((void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        simsock_writel(sleep_stats | (TOSLEEP << ch), (void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        bsp_err("accept was interrupted by intr\n");
        (void)simsock_chan_free(ch);
        return -EINTR;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_recv(int sockfd, void *ubuf, size_t size, int flags)
{
    int ret;
    int ch;
    int sleep_stats;
    sim_dma_addr_t dma_ubuf_phy;

    if (ubuf == NULL) {
        return SIMSOCK_FAIL;
    }

    ret = simsock_chan_alloc(sockfd, simsock_cmd_recv);
    if (ret < 0)
        return ret;
    ch = ret;

    dma_ubuf_phy = sim_dma_map_single(g_simsock_ctx.dev, ubuf, size, SIM_DMA_TO_DEVICE);
    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(LOW32(dma_ubuf_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_ubuf_phy), (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(size, (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(flags, (void *)(SIMSOCK_PARAM4(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_recv | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    sim_dma_unmap_single(g_simsock_ctx.dev, dma_ubuf_phy, size, SIM_DMA_FROM_DEVICE);

    if (down_interruptible(&g_sem_code[ch][simsock_cmd_recv])) {
        sleep_stats = simsock_readl((void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        simsock_writel(sleep_stats | (TOSLEEP << ch), (void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        bsp_err("recv was interrupted by intr\n");
        (void)simsock_chan_free(ch);
        return -EINTR;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

void get_dma_buf(sim_dma_addr_t *dma_buff_phy, int ch, void *buff, size_t size)
{
    int ret;

    switch (g_simsock_ctx.copy_mode) {
        case map_single_mode: {
            *dma_buff_phy = sim_dma_map_single(g_simsock_ctx.dev, buff, sizeof(struct sockaddr), SIM_DMA_TO_DEVICE);
            break;
        }
        case alloc_mode: {
            ret = memcpy_s(g_simsock_ctx.chan[ch].buf, g_simsock_ctx.chan[ch].size, buff, size);
            if (ret != 0) {
                bsp_err("in alloc mode, get_dma_buf failed!\n");
                return;
            }
            g_simsock_ctx.chan[ch].len = size;

            *dma_buff_phy = g_simsock_ctx.chan[ch].buf_phy;
            break;
        }
        case dirc_trans_mode: {
            *dma_buff_phy = (uintptr_t)buff;
            break;
        }
        default:
            break;
    }
}

int simsock_send(int sockfd, void *buff, size_t size, int flags)
{
    int ret;
    int ch;
    int sleep_stats;
    sim_dma_addr_t dma_buff_phy = {0};

    if (buff == NULL) {
        return SIMSOCK_FAIL;
    }

    ret = simsock_chan_alloc(sockfd, simsock_cmd_send);
    if (ret < 0) {
        return ret;
    }

    ch = ret;

    get_dma_buf(&dma_buff_phy, ch, buff, size);

    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(LOW32(dma_buff_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_buff_phy), (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(size, (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(flags, (void *)(SIMSOCK_PARAM4(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_send | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    if (!g_simsock_ctx.copy_mode) {
        sim_dma_unmap_single(g_simsock_ctx.dev, dma_buff_phy, size, SIM_DMA_FROM_DEVICE);
    }

    if (down_interruptible(&g_sem_code[ch][simsock_cmd_send])) {
        sleep_stats = simsock_readl((void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        simsock_writel(sleep_stats | (TOSLEEP << ch), (void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        bsp_err("send was interrupted by intr\n");
        (void)simsock_chan_free(ch);
        return -EINTR;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_sendto(int sockfd, void *buff, size_t size, int flags, struct sockaddr *addr, int addrlen)
{
    int ret;
    int ch;
    int sleep_stats;
    sim_dma_addr_t dma_buff_phy = {0};
    sim_dma_addr_t dma_addr_phy = {0};

    if ((buff == NULL) || (addr == NULL)) {
        return SIMSOCK_FAIL;
    }

    ret = simsock_chan_alloc(sockfd, simsock_cmd_sendto);
    if (ret < 0) {
        return ret;
    }

    ch = ret;

    get_dma_buf(&dma_buff_phy, ch, buff, size);

    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(LOW32(dma_buff_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));

    simsock_writel(size, (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(flags, (void *)(SIMSOCK_PARAM4(ch) + g_simsock_ctx.simsock_regs));

    dma_addr_phy = sim_dma_map_single(g_simsock_ctx.dev, addr, sizeof(struct sockaddr), SIM_DMA_TO_DEVICE);
    simsock_writel(LOW32(dma_addr_phy), (void *)(SIMSOCK_PARAM5(ch) + g_simsock_ctx.simsock_regs));

    simsock_writel(simsock_cmd_sendto | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    if (!g_simsock_ctx.copy_mode) {
        sim_dma_unmap_single(g_simsock_ctx.dev, dma_buff_phy, size, SIM_DMA_FROM_DEVICE);
    }

    sim_dma_unmap_single(g_simsock_ctx.dev, dma_addr_phy, sizeof(struct sockaddr), SIM_DMA_FROM_DEVICE);

    if (down_interruptible(&g_sem_code[ch][simsock_cmd_sendto])) {
        sleep_stats = simsock_readl((void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        simsock_writel(sleep_stats | (TOSLEEP << ch), (void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        bsp_err("sendto was interrupted by intr\n");
        (void)simsock_chan_free(ch);
        return -EINTR;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_getpeername(int sockfd, struct sockaddr *addr, int *addrlen)
{
    int ret;
    int ch;
    sim_dma_addr_t dma_addr_phy;
    sim_dma_addr_t dma_addrlen_phy;

    if ((addr == NULL) || (addrlen == NULL)) {
        return SIMSOCK_FAIL;
    }

    ret = simsock_chan_alloc(sockfd, simsock_cmd_getpeername);
    if (ret < 0)
        return ret;
    ch = ret;
    dma_addr_phy = sim_dma_map_single(g_simsock_ctx.dev, addr, sizeof(struct sockaddr), SIM_DMA_TO_DEVICE);
    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(LOW32(dma_addr_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_addr_phy), (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));

    dma_addrlen_phy = sim_dma_map_single(g_simsock_ctx.dev, addrlen, sizeof(int), SIM_DMA_TO_DEVICE);
    simsock_writel(LOW32(dma_addrlen_phy), (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_addrlen_phy), (void *)(SIMSOCK_PARAM4(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_getpeername | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    ret = simsock_chan_complete(ch);
    if (ret) {
        return ret;
    }

    sim_dma_unmap_single(g_simsock_ctx.dev, dma_addr_phy, sizeof(struct sockaddr), SIM_DMA_FROM_DEVICE);
    sim_dma_unmap_single(g_simsock_ctx.dev, dma_addrlen_phy, sizeof(int), SIM_DMA_FROM_DEVICE);

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_shutdown(int sockfd, enum sock_shutdown_cmd how)
{
    int ret;
    int ch;

    ret = simsock_chan_alloc(sockfd, simsock_cmd_shutdown);
    if (ret < 0) {
        bsp_err("channel %d alloc failed!\n", ret);
        return ret;
    }

    ch = ret;
    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(how, (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_shutdown | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));
    ret = simsock_chan_complete(ch);
    if (ret) {
        bsp_err("channel %d complete timeout!\n", ch);
        return ret;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_release(int sockfd)
{
    int ret;
    int ch;

    ret = simsock_chan_alloc(sockfd, simsock_cmd_close);
    if (ret < 0)
        return ret;
    ch = ret;
    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_close | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));
    ret = simsock_chan_complete(ch);
    if (ret) {
        return ret;
    }
    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_select(int nfds, int *readfds, int *writefds, int *exceptfds)
{
    int ret;
    int ch;
    int sleep_stats;
    sim_dma_addr_t dma_rd_phy;

    ret = simsock_chan_alloc(0, simsock_cmd_sendto);
    if (ret < 0)
        return ret;
    ch = ret;

    simsock_writel(nfds, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    dma_rd_phy = sim_dma_map_single(g_simsock_ctx.dev, readfds, SIMSOCK_FDS_SIZE, SIM_DMA_TO_DEVICE);
    simsock_writel(LOW32(dma_rd_phy), (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(HIGH32(dma_rd_phy), (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));

    simsock_writel(simsock_cmd_select | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    sim_dma_unmap_single(g_simsock_ctx.dev, dma_rd_phy, SIMSOCK_FDS_SIZE, SIM_DMA_FROM_DEVICE);

    if (down_interruptible(&g_sem_code[ch][simsock_cmd_select])) {
        sleep_stats = simsock_readl((void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        simsock_writel(sleep_stats | (TOSLEEP << ch), (void *)(SIMSOCK_SLPSTAT + g_simsock_ctx.simsock_regs));
        bsp_err("select was interrupted by intr\n");
        (void)simsock_chan_free(ch);
        return -EINTR;
    }

    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);

    return ret;
}

int simsock_setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen)
{
    int ret;
    int ch;
    sim_dma_addr_t addr_phy;

    ret = simsock_chan_alloc(sockfd, simsock_cmd_setsockopt);
    if (ret < 0)
        return ret;
    ch = ret;

    simsock_writel(sockfd, (void *)(SIMSOCK_PARAM0(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(level, (void *)(SIMSOCK_PARAM1(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(optname, (void *)(SIMSOCK_PARAM2(ch) + g_simsock_ctx.simsock_regs));
    addr_phy = sim_dma_map_single(g_simsock_ctx.dev, optval, sizeof(uint32_t), SIM_DMA_TO_DEVICE);
    simsock_writel(addr_phy, (void *)(SIMSOCK_PARAM3(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(optlen, (void *)(SIMSOCK_PARAM4(ch) + g_simsock_ctx.simsock_regs));
    simsock_writel(simsock_cmd_setsockopt | SIMSOCK_OP_START, (void *)(SIMSOCK_OP(ch) + g_simsock_ctx.simsock_regs));

    ret = simsock_chan_complete(ch);
    if (ret) {
        return ret;
    }
    ret = simsock_readl((void *)(SIMSOCK_RESULT(ch) + g_simsock_ctx.simsock_regs));
    (void)simsock_chan_free(ch);
    sim_dma_unmap_single(g_simsock_ctx.dev, addr_phy, sizeof(uint32_t), SIM_DMA_FROM_DEVICE);

    return ret;
}

/* register a socket to table,set reference count to 1 */
static int hi_sock_reg_fd(int sockfd)
{
    int i = -1;

    sim_mutex_lock(&g_simsock_ctx.simfd_mutex);

    if (SIMSOCK_MAX <= g_simsock_ctx.used) {
        bsp_err("sock_reg fail, sock used full\n");
        goto fail;
    }
    for (i = 0; i < SIMSOCK_MAX; i++)
        if (!sim_atomic_read(&g_simsock_ctx.socks[i].cnt))
            break;
    if (SIMSOCK_MAX <= i) {
        i = -1;
        goto fail;
    }

    g_simsock_ctx.socks[i].sockfd = sockfd;
    sim_atomic_set(&g_simsock_ctx.socks[i].cnt, 1);

    g_simsock_ctx.used++;

    atomic_inc(&g_simsockdbg.sockregfd);

fail:
    sim_mutex_unlock(&g_simsock_ctx.simfd_mutex);

    return i;
}

static int hi_sock_unreg_fd(int fd)
{
    if (fd > SIMSOCK_MAX - 1 || fd < 0) {
        bsp_err("sock_unreg fail, sock used full\n");
        return -1;
    }

    sim_mutex_lock(&g_simsock_ctx.simfd_mutex);

    g_simsock_ctx.used--;

    sim_atomic_set(&g_simsock_ctx.socks[fd].cnt, 0);

    g_simsock_ctx.socks[fd].sockfd = 0;

    atomic_inc(&g_simsockdbg.sockunregfd);

    sim_mutex_unlock(&g_simsock_ctx.simfd_mutex);

    return 1;
}

static int hi_sock_get_fd(int fd)
{
    int err = -1;

    if (fd > SIMSOCK_MAX - 1 || fd < 0) {
        bsp_err("sock_getfd fail, fd = %d\n", fd);
        return err;
    }

    if (likely(sim_atomic_inc_not_zero(&g_simsock_ctx.socks[fd].cnt)))
        err = 0;
    atomic_inc(&g_simsockdbg.sockgetfd);

    return err;
}

static int hi_sock_put_fd(int fd)
{
    int sockfd;

    int err = -1;

    if (fd > SIMSOCK_MAX - 1 || fd < 0) {
        bsp_err("sock_putfd fail, fd = %d\n", fd);
        return err;
    }

    /* no use */
    if (unlikely(sim_atomic_dec_and_test(&g_simsock_ctx.socks[fd].cnt))) {
        sockfd = g_simsock_ctx.socks[fd].sockfd;

        err = hi_sock_unreg_fd(fd);
        bsp_err("close fd = %d\n", sockfd);
        simsock_release(sockfd);
    }
    atomic_inc(&g_simsockdbg.sockputfd);

    return 1;
}

int bsp_esl_socket(int family, int type, int protocol)
{
    int sockfd = 0;
    int retval;

    atomic_inc(&g_simsockdbg.sock);

    retval = simsock_create(family, type, protocol);
    if (retval < 0) {
        bsp_err("create sock fail!\n");
        goto out;
    }
    sockfd = retval;
    retval = hi_sock_reg_fd(sockfd);
    if (retval < 0)
        goto out_release;

out:
    return retval;

out_release:
    simsock_release(sockfd);
    return retval;
}

int bsp_esl_bind(int fd, struct sockaddr *addr, int addrlen)
{
    int sockfd;
    int retval = -1;

    atomic_inc(&g_simsockdbg.bind);

    if (hi_sock_get_fd(fd) < 0) {
        return retval;
    }

    sockfd = hi_get_sockfd(fd);

    retval = simsock_bind(sockfd, addr, addrlen);
    if (retval < 0) {
        bsp_err("bind fail!\n");
    }

    hi_sock_put_fd(fd);

    return retval;
}

int bsp_esl_listen(int fd, int backlog)
{
    int sockfd;
    int retval = -1;

    atomic_inc(&g_simsockdbg.listen);

    if (hi_sock_get_fd(fd) < 0) {
        return retval;
    }

    sockfd = hi_get_sockfd(fd);

    retval = simsock_listen(sockfd, backlog);
    if (retval < 0) {
        bsp_err("listen fail!\n");
    }

    hi_sock_put_fd(fd);

    return retval;
}

int bsp_esl_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
    int sockfd;
    int newsockfd;
    int err;

    atomic_inc(&g_simsockdbg.accept_in);

    if (hi_sock_get_fd(fd) < 0) {
        atomic_inc(&g_simsockdbg.accept_out);
        return -1;
    }

    sockfd = hi_get_sockfd(fd);

    err = simsock_accept(sockfd, upeer_sockaddr, upeer_addrlen);
    if (err < 0)
        goto done;

    newsockfd = err;
    err = hi_sock_reg_fd(newsockfd);
    if (err < 0)
        simsock_release(newsockfd);

done:
    hi_sock_put_fd(fd);

    atomic_inc(&g_simsockdbg.accept_out);

    return err;
}

int bsp_esl_recv(int fd, void *ubuf, size_t size, int flags)
{
    int sockfd;
    int err = -1;

    atomic_inc(&g_simsockdbg.recv_in);

    if (hi_sock_get_fd(fd) < 0)
        goto out;

    sockfd = hi_get_sockfd(fd);

    if (ubuf == NULL) {
        bsp_err("recv buff is NULL \n");
        atomic_inc(&g_simsockdbg.recv_out);
        return -1;
    }

    err = simsock_recv(sockfd, ubuf, size, flags);

    hi_sock_put_fd(fd);

out:
    atomic_inc(&g_simsockdbg.recv_out);
    return err;
}

int bsp_esl_send(int fd, void *buff, size_t len, int flags)
{
    int sockfd;
    int err = -1;

    atomic_inc(&g_simsockdbg.send);

    if (hi_sock_get_fd(fd) < 0)
        goto out;

    sockfd = hi_get_sockfd(fd);

    if (buff == NULL) {
        bsp_err("send buff is NULL \n");
        return -1;
    }

    err = simsock_send(sockfd, buff, len, flags);

    hi_sock_put_fd(fd);

out:
    return err;
}

int bsp_esl_sendto(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int addr_len)
{
    int sockfd;
    int err = -1;

    atomic_inc(&g_simsockdbg.sendto);

    if (hi_sock_get_fd(hiskinfo->fd) < 0)
        goto out;

    sockfd = hi_get_sockfd(hiskinfo->fd);

    err = simsock_sendto(sockfd, hiskinfo->buff, hiskinfo->len, flags, addr, addr_len);

    hi_sock_put_fd(hiskinfo->fd);
out:
    return err;
}

int bsp_esl_getpeername(int fd, struct sockaddr *addr, int *addrlen)
{
    int sockfd;
    int err = -1;

    atomic_inc(&g_simsockdbg.getpeername);

    if (hi_sock_get_fd(fd) < 0)
        goto out;

    sockfd = hi_get_sockfd(fd);

    err = simsock_getpeername(sockfd, addr, addrlen);

    hi_sock_put_fd(fd);
out:
    return err;
}

int bsp_esl_shutdown(int fd, enum sock_shutdown_cmd how)
{
    int sockfd;
    int err = -1;

    atomic_inc(&g_simsockdbg.shutdown);

    if (hi_sock_get_fd(fd) < 0)
        goto out;

    sockfd = hi_get_sockfd(fd);
    err = simsock_shutdown(sockfd, how);
    if (err < 0) {
        bsp_err("simsock shutdown fail\n");
    }

    hi_sock_put_fd(fd);
out:
    return err;
}

int bsp_esl_close(int fd)
{
    int ret;

    atomic_inc(&g_simsockdbg.close);

    ret = hi_sock_put_fd(fd);

    return ret;
}

int bsp_esl_select(int n, sk_fd_set *inp, sk_fd_set *outp, sk_fd_set *exp_fdset, struct timespec64 *tvp)
{
    int i;
    int ret;
    int *readfds = NULL;
    int nfds = 0;
    int err;

    atomic_inc(&g_simsockdbg.select_in);
    if ((SIMSOCK_MAX_READFDS < n) || (inp == NULL)) {
        bsp_err("select err \n");
        atomic_inc(&g_simsockdbg.select_out);
        return SIMSOCK_FAIL;
    }

    readfds = (int *)osl_malloc(sizeof(int) * SIMSOCK_MAX_READFDS);
    if (readfds == NULL) {
        return -1;
    }

    err = memset_s((char *)readfds, (sizeof(int) * SIMSOCK_MAX_READFDS), 0, (sizeof(int) * SIMSOCK_MAX_READFDS));
    if (err) {
        bsp_err("memset failed\n");
    }

    for (i = 0; i < SIMSOCK_MAX_READFDS; i++) {
        if (inp->fds_bits & (1 << i)) {
            readfds[i] = hi_get_sockfd(i);
            if (nfds < readfds[i])
                nfds = readfds[i];
        }
    }

    ret = simsock_select((nfds + 1), readfds, NULL, NULL);
    if (ret < 0) {
        bsp_err("simsock select err \n");
        atomic_inc(&g_simsockdbg.select_out);
        osl_free(readfds);
        return ret;
    }

    inp->fds_bits = 0;
    for (i = 0; i < SIMSOCK_MAX_READFDS; i++) {
        if (readfds[i] != 0) {
            inp->fds_bits |= (1 << i);
        }
    }

    atomic_inc(&g_simsockdbg.select_out);
    osl_free(readfds);
    return ret;
}

int bsp_esl_setsockopt(int fd, int level, int optname, char *optval, unsigned int optlen)
{
    int sockfd;
    int err = -1;

    if (hi_sock_get_fd(fd) < 0)
        return err;

    sockfd = hi_get_sockfd(fd);
    err = simsock_setsockopt(sockfd, level, optname, optval, optlen);

    hi_sock_put_fd(fd);
    return err;
}

void bsp_esl_sock_fd_zero(sk_fd_set *set)
{
    int ret;

    ret = memset_s((void *)set, sizeof(sk_fd_set), 0, sizeof(*set));
    if (ret) {
        bsp_err("memset_s failed\n");
    }
}

int bsp_esl_connect(int fd, struct sockaddr *addr, int addrlen)
{
    bsp_err("bsp_esl_connect is stub!\n");
    return 0;
}

int bsp_esl_recvfrom(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int *addr_len)
{
    bsp_err("bsp_esl_recvfrom is stub!\n");
    return 0;
}

int bsp_esl_getsockname(int fd, struct sockaddr *addr, int *addrlen)
{
    bsp_err("bsp_esl_getsockname is stub!\n");
    return 0;
}

int bsp_esl_getsockopt(int fd, int level, int optname, char *optval, int *optlen)
{
    bsp_err("bsp_esl_getsockopt is stub!\n");
    return 0;
}

int bsp_esl_ioctl(int fd, int cmd, unsigned int arg)
{
    bsp_err("bsp_esl_ioctl is stub!\n");
    return 0;
}

unsigned int bsp_esl_inet_addr(const char *str)
{
    bsp_err("bsp_esl_inet_addr is stub!\n");
    return 0;
}

void svlan_show(void)
{
    bsp_err("sock:  %u", atomic_read(&g_simsockdbg.sock));
    bsp_err("bind:  %u", atomic_read(&g_simsockdbg.bind));
    bsp_err("listen:  %u", atomic_read(&g_simsockdbg.listen));
    bsp_err("select_in:  %u", atomic_read(&g_simsockdbg.select_in));
    bsp_err("select_out:  %u", atomic_read(&g_simsockdbg.select_out));
    bsp_err("accept_in:  %u", atomic_read(&g_simsockdbg.accept_in));
    bsp_err("accept_out:  %u", atomic_read(&g_simsockdbg.accept_out));
    bsp_err("send:  %u", atomic_read(&g_simsockdbg.send));
    bsp_err("recv_in:  %u", atomic_read(&g_simsockdbg.recv_in));
    bsp_err("recv_out:  %u", atomic_read(&g_simsockdbg.recv_out));
    bsp_err("sendto:  %u", atomic_read(&g_simsockdbg.sendto));
    bsp_err("getpeername:  %u", atomic_read(&g_simsockdbg.getpeername));
    bsp_err("shutdown:  %u", atomic_read(&g_simsockdbg.shutdown));
    bsp_err("close:  %u", atomic_read(&g_simsockdbg.close));
    bsp_err("sockregfd:  %u", atomic_read(&g_simsockdbg.sockregfd));
    bsp_err("sockunregfd:  %u", atomic_read(&g_simsockdbg.sockunregfd));
    bsp_err("sockgetfd:  %u", atomic_read(&g_simsockdbg.sockgetfd));
    bsp_err("sockputfd:  %u", atomic_read(&g_simsockdbg.sockputfd));
}

int simsock_set_copymode(int copy_mode)
{
    g_simsock_ctx.copy_mode = copy_mode;
    return g_simsock_ctx.copy_mode;
}
void set_simsock_debug(int level)
{
    g_simsock_debug = level;
}

static int sock_probe(sim_platform_device *pdev)
{
    int i;
    int ret;
    struct resource *regs = NULL;
    unsigned int irq;
    g_simsock_dbg.svlan_probe_begin = 1;
    bsp_err("[init]start \n");

    g_simsock_ctx.dev = &pdev->dev;

    g_simsock_ctx.copy_mode = alloc_mode;

    regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (regs == NULL) {
        return -ENXIO;
    }

    g_simsock_ctx.simsock_regs = devm_ioremap_resource(&pdev->dev, regs);
    if (IS_ERR(g_simsock_ctx.simsock_regs)) {
        return PTR_ERR(g_simsock_ctx.simsock_regs);
    }

    irq = platform_get_irq(pdev, 0);
    if (unlikely(irq == 0)) {
        return -ENXIO;
    }

    g_simsock_ctx.irq = irq;

    g_simsock_ctx.dev->dma_mask = &g_simsock_dma_mask;

    g_simsock_ctx.used = 0;
    g_simsock_ctx.chan_used = 0;
    g_simsock_ctx.chan_num = SIMSOCK_CHAN_MAX;
    g_simsock_ctx.chan_map = 0xffffffff;

    sim_mutex_init(&g_simsock_ctx.simfd_mutex);
    sim_mutex_init(&g_simsock_ctx.simch_mutex);

    for (i = 0; i < SIMSOCK_MAX; i++) {
        sim_atomic_set(&g_simsock_ctx.socks[i].cnt, 0);
        g_simsock_ctx.socks[i].sockfd = 0;
    }

    ret = simsock_chan_ctrl_init();
    if (unlikely(ret)) {
        bsp_err("[init]chan_init fail \n");
    }

    ret = sim_request_irq(irq, sock_interrupt, SIM_IRQF_SHARED, "sim_sock_driver", g_simsock_ctx.dev);
    if (ret < 0) {
        bsp_err("[init]request irq fail \n");
        return -ENXIO;
    }

    bsp_err("[init] ok\n");
    g_simsock_dbg.svlan_probe_end = 1;
    return 0;
}

int sock_remove(sim_platform_device *pdev)
{
    return 0;
}

static const struct of_device_id g_sock_match[] = {
    {.compatible = "sim_sock_drv"},
    {},
};
MODULE_DEVICE_TABLE(of, g_sock_match);

static struct platform_driver g_sim_sock_driver = {
    .probe = sock_probe,
    .remove = sock_remove,
    .driver =
        {
            .name = "sim_sock_driver",
            .of_match_table = of_match_ptr(g_sock_match),
        },
};

static int svlan_socket_init(void)
{
    int ret;
    bsp_version_info_s *version_info = NULL;
    int i, j;

    g_simsock_dbg.svlan_module_init_begin = 1;
    pr_err("[pr_err]svlan_socket init start!\n");
    version_info = (bsp_version_info_s *)bsp_get_version_info();
    if (version_info->plat_type == PLAT_ESL || version_info->plat_type == PLAT_HYBRID) {
        ret = platform_driver_register(&g_sim_sock_driver);
    } else {
        ret = 0;
    }
    g_simsock_dbg.platform_ret = ret;
    g_simsock_dbg.svlan_module_init_end = 1;

    for (i = 0; i < SIMSOCK_CHAN_MAX; i++) {
        for (j = 0; j < MAX_CMD; j++) {
            osl_sem_init(0, &g_sem_code[i][j]);
        }
    }
    return ret;
}
module_init(svlan_socket_init);
#endif
