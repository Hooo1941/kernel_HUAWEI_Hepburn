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


#include "esl_socket_api.h"
#include "esl_socket_os_interface.h"
#include "esl_socket_channel_ctr.h"

#include <bsp_print.h>
#define THIS_MODU mod_socket

#define BUFF_SIZE (4 * 1024 * 1024)

int simsock_chan_alloc(int sockfd, int opt)
{
    int ret;
    int ch;
    struct simsock_chan *chan = NULL;
    struct simsock_ctx* simsock_ctx_e = get_simsock_config();

    sim_mutex_lock(&simsock_ctx_e->simch_mutex);
    if (SIMSOCK_CHAN_MAX <= simsock_ctx_e->chan_used) {
        ret = SIMSOCK_FAIL;
        goto exit0;
    }

    ch = find_first_bit(&simsock_ctx_e->chan_map, simsock_ctx_e->chan_num);
    if (SIMSOCK_CHAN_MAX <= ch) {
        ret = SIMSOCK_FAIL;
        goto exit0;
    }

    simsock_ctx_e->chan_used++;
    simsock_ctx_e->chan_map &= ~(1 << ch);

    chan = &simsock_ctx_e->chan[ch];
    chan->opt = opt;
    chan->sockfd = sockfd;

    ret = ch;

exit0:
    sim_mutex_unlock(&simsock_ctx_e->simch_mutex);

    return ret;
}

int simsock_chan_free(int ch)
{
    struct simsock_chan *chan = NULL;
    struct simsock_ctx* simsock_ctx_e = get_simsock_config();

    if (ch > SIMSOCK_CHAN_MAX || ch < 0) {
        return SIMSOCK_FAIL;
    }

    chan = &simsock_ctx_e->chan[ch];

    sim_mutex_lock(&simsock_ctx_e->simch_mutex);
    simsock_ctx_e->chan_map |= 1 << ch;
    simsock_ctx_e->chan_used--;
    chan->opt = 0;
    chan->sockfd = 0;
    sim_mutex_unlock(&simsock_ctx_e->simch_mutex);

    return 0;
}

int simsock_chan_ctrl_init(void)
{
    unsigned int i;
    unsigned int value;
    struct simsock_ctx *simsock_ctx_e = get_simsock_config();
    unsigned int size;
    void *virtual_addr = NULL;

    unsigned long phy_addr = mdrv_mem_region_get("simsock_ddr", &size);
    if(size == 0){
        return -1;
    }
    bsp_err("get size is %x\n", size);
    virtual_addr = ioremap(phy_addr, size);
    for (i = 0; i < SIMSOCK_CHAN_MAX; i++) {
        sim_atomic_set(&simsock_ctx_e->chan[i].cnt, 0);
        osl_sem_init(0, &simsock_ctx_e->chan[i].sock_wait);
        simsock_ctx_e->chan[i].opt = 0;
        simsock_ctx_e->chan[i].sockfd = 0;

        value = simsock_readl((void *)(SIMSOCK_INTRAW(i) + simsock_ctx_e->simsock_regs));
        simsock_writel(value, (void *)(SIMSOCK_INTRAW(i) + simsock_ctx_e->simsock_regs));

        if (simsock_ctx_e->copy_mode) {
            simsock_ctx_e->chan[i].size = BUFF_SIZE;
            simsock_ctx_e->chan[i].buf = (void*)(virtual_addr + i * BUFF_SIZE);
            simsock_ctx_e->chan[i].buf_phy = (sim_dma_addr_t)(phy_addr + i * BUFF_SIZE);

            if (!simsock_ctx_e->chan[i].buf) {
                return -ENOMEM;
            }
        }

        simsock_writel(INTMASKENABLE, (void *)(SIMSOCK_INTMASK(i) + simsock_ctx_e->simsock_regs));
    }

    simsock_ctx_e->simsock_iqrstatus = SIMSOCK_CHINTSTATS + simsock_ctx_e->simsock_regs;

    return 0;
}

void simsock_chan_ctrl_exit(void)
{
    return;
}
