/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#include "pcie_cdev.h"
#include "pcie_cdev_desc.h"
#include "pcie_cdev_dbg.h"
#include <linux/timer.h>
#include <linux/jiffies.h>

#define PCDEV_MAX_INTR_COUNT 32
#define PCDEV_DATA_PRT_MAX_LEN_LINE 4
#define PCDEV_DATA_PRT_WORD_DATA_NUM 4

const unsigned char *g_pcdev_work_mode[] = {
    "EP", 0, 0, 0, "RC",
};

void set_pcdev_msg(unsigned int level)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    pcdev_ctx->msg_level = level;
}

void set_pcdev_print_pkt(unsigned int port_num)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    if (port_num == 0xFFFF) {
        pcdev_ctx->print_port = 0;
    }
    pcdev_ctx->print_port |= (1 << port_num);
}

unsigned int get_pcdev_msg(void)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_ERR("level : 0x%x\n", pcdev_ctx->msg_level);
    return pcdev_ctx->msg_level;
}

static void port_info_dump(struct pcie_cdev_port *port)
{
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_ERR("port name:            %s\n", ((port->name == NULL) ? ("unknown") : (port->name)));
    if (pcdev_ctx->pcdev_vote_lock(PRINT_MODE)) {
        PCDEV_ERR("vote err\n");
        return;
    }
    PCDEV_ERR("port num:             %d\n", port->port_num);
    PCDEV_ERR("port id:              %d\n", *port->port_id);
    PCDEV_ERR("local stat:           %d\n", port->local_stat->u32);
    PCDEV_ERR("remote stat:          %d\n", port->remote_stat->u32);
    PCDEV_ERR("dmamap skip:          %d\n", port->dmamap_skip);

    PCDEV_ERR("event cb:            %s\n", ((port->event_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("event_type:           %d\n", *port->event_type);
    PCDEV_ERR("read_sig_cb           %s\n", ((port->read_sig_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("read_sig:             %lx\n", (uintptr_t)port->read_sig);
    PCDEV_ERR("rel_ind_cb            %s\n", ((port->rel_ind_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("rel_ind:              %d\n", *port->rel_ind);
    PCDEV_ERR("msc_stru_cb           %s\n", ((port->msc_stru_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("msc_stru:             %lx\n", (uintptr_t)port->msc_stru);

    PCDEV_ERR("rx_wp:                %d\n", *port->rx_wp);
    PCDEV_ERR("rx_rp:                %d\n", *port->rx_rp);
    PCDEV_ERR("rx_rp_toget:          %d\n", port->rx_rp_toget);
    PCDEV_ERR("rx_rp_process:        %d\n", port->rx_rp_process);
    PCDEV_ERR("rx_buf_max_sz:        %d\n", *port->rx_buf_max_sz);
    PCDEV_ERR("rx_buf_max_sz_port:   %d\n", port->rx_buf_max_size);
    PCDEV_ERR("rx_num:               %d\n", port->rx_num);
    PCDEV_ERR("read_done_cb          %s\n", ((port->read_done_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("rx_desc:              %lx\n", (uintptr_t)port->rx_desc);

    PCDEV_ERR("tx_wp:                %d\n", *port->tx_wp);
    PCDEV_ERR("tx_rp:                %d\n", *port->tx_rp);
    PCDEV_ERR("tx_rp_todo:           %d\n", *port->tx_rp_todo);
    PCDEV_ERR("tx_rp_complete:       %d\n", port->tx_rp_complete);
    PCDEV_ERR("tx_buf_max_sz:        %d\n", *port->tx_buf_max_sz);
    PCDEV_ERR("tx_num:               %d\n", port->tx_num);
    PCDEV_ERR("write_done_cb         %s\n", ((port->write_done_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("write_info_done_cb    %s\n", ((port->write_info_done_cb == NULL) ? ("NULL") : ("SET")));
    PCDEV_ERR("tx_desc:              %lx\n", (uintptr_t)port->tx_desc);

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    if (pcdev_ctx->work_mode == PCIE_EP) {
        PCDEV_ERR("tx_rp_todo_rc:        %d\n", *port->tx_rp_todo_rc);
        PCDEV_ERR("tx_rp_dma_cmp_rc:     %d\n", *port->tx_rp_dma_cmp_rc);
        PCDEV_ERR("tx_rp_dma_cfg_rc:     %d\n", port->tx_rp_dma_cfg_rc);
        PCDEV_ERR("tx_rp_cmp:            %d\n", port->tx_rp_cmp);
    }
#endif
    pcdev_ctx->pcdev_vote_unlock(PRINT_MODE);
}

static void tx_rx_state_dump(struct pcie_cdev_port *port)
{
    PCDEV_ERR("tx_empty :            %ld\n", port->pstats.tx_empty);
    PCDEV_ERR("tx_full :             %ld\n", port->pstats.tx_full);
    PCDEV_ERR("tx_todo_full :        %ld\n", port->pstats.tx_todo_full);
    PCDEV_ERR("tx_desc_err :         %ld\n", port->pstats.tx_desc_err);
    PCDEV_ERR("tx_buf_size_err :     %ld\n", port->pstats.tx_buf_size_err);
    PCDEV_ERR("kick_dma 1:           %ld\n", port->pstats.kick_dma_1);
    PCDEV_ERR("kick_dma 2:           %ld\n", port->pstats.kick_dma_2);
    PCDEV_ERR("kick_dma :            %ld\n", port->pstats.kick_dma_transfer);
    PCDEV_ERR("kick_dma_fail :       %ld\n", port->pstats.kick_dma_transfer_fail);
    PCDEV_ERR("tx_dma_send_comp :    %ld\n", port->pstats.tx_dma_send_complete);
    PCDEV_ERR("write_base :          %ld\n", port->pstats.write_base);
    PCDEV_ERR("tx_packets :          %ld\n", port->pstats.tx_packets);
    PCDEV_ERR("tx_packets_finish :   %ld\n", port->pstats.tx_packets_finish);
    PCDEV_ERR("tx_packets_cb :       %ld\n", port->pstats.tx_packets_cb);
    PCDEV_ERR("tx_packets_fail :     %ld\n", port->pstats.tx_packets_fail);
    PCDEV_ERR("tx_bytes :            %ld\n", port->pstats.tx_bytes);
    PCDEV_ERR("write_cb_call :       %ld\n", port->pstats.write_cb_call);
    PCDEV_ERR("write_info_cb_call :  %ld\n", port->pstats.write_info_cb_call);
    PCDEV_ERR("write_cb_null :       %ld\n", port->pstats.write_cb_null);
    PCDEV_ERR("write_async_call :    %ld\n", port->pstats.write_async_call);
    PCDEV_ERR("sync_tx_submit :      %ld\n", port->pstats.sync_tx_submit);
    PCDEV_ERR("sync_tx_wait_fail :   %ld\n", port->pstats.sync_tx_wait_fail);
    PCDEV_ERR("sync_tx_done :        %ld\n", port->pstats.sync_tx_done);
    PCDEV_ERR("sync_tx_fail :        %ld\n", port->pstats.sync_tx_fail);
    PCDEV_ERR("rx_close :            %ld\n", port->pstats.rx_close);
    PCDEV_ERR("rx_empty :            %ld\n", port->pstats.rx_empty);
    PCDEV_ERR("rx_todo_empty :       %ld\n", port->pstats.rx_todo_empty);
    PCDEV_ERR("rx_process_empty :    %ld\n", port->pstats.rx_process_empty);
    PCDEV_ERR("rx_full :             %ld\n", port->pstats.rx_full);
    PCDEV_ERR("rx_desc_err :         %ld\n", port->pstats.rx_desc_err);
    PCDEV_ERR("rx_get_bur_err :      %ld\n", port->pstats.rx_get_bur_err);
    PCDEV_ERR("rx_packets :          %ld\n", port->pstats.rx_packets);
    PCDEV_ERR("rx_packets_finish :   %ld\n", port->pstats.rx_packets_finish);
    PCDEV_ERR("rx_packets_fail :     %ld\n", port->pstats.rx_packets_fail);
    PCDEV_ERR("rx_bytes :            %ld\n", port->pstats.rx_bytes);
    PCDEV_ERR("rx_get_buf_pram_err : %ld\n", port->pstats.rx_get_buf_pram_err);
    PCDEV_ERR("get_buf_call :        %ld\n", port->pstats.get_buf_call);
    PCDEV_ERR("ret_buf_call :        %ld\n", port->pstats.ret_buf_call);
    PCDEV_ERR("read_cb_call :        %ld\n", port->pstats.read_cb_call);
    PCDEV_ERR("read_cb_null :        %ld\n", port->pstats.read_cb_null);
    PCDEV_ERR("callback_null :       %ld\n", port->pstats.callback_null);
}

static void state_dump(struct pcie_cdev_port *port)
{
    PCDEV_ERR("open_count :          %ld\n", port->pstats.open_count);
    PCDEV_ERR("close_count :         %ld\n", port->pstats.close_count);
    PCDEV_ERR("close_count_repeat :  %ld\n", port->pstats.close_count_repeat);
    PCDEV_ERR("openclose :           %ld\n", port->pstats.openclose);
    PCDEV_ERR("remote_close :        %ld\n", port->pstats.remote_close);
    PCDEV_ERR("local_close :         %ld\n", port->pstats.local_close);
    PCDEV_ERR("irq_send :            %ld\n", port->pstats.irq_send);
    PCDEV_ERR("event_send :          %ld\n", port->pstats.event_send);
    PCDEV_ERR("event_cb_call :       %ld\n", port->pstats.event_cb_call);
    PCDEV_ERR("event_cb_null :       %ld\n", port->pstats.event_cb_null);
    PCDEV_ERR("read_sig_send :          %ld\n", port->pstats.read_sig_send);
    PCDEV_ERR("read_sig_cb_call :       %ld\n", port->pstats.read_sig_cb_call);
    PCDEV_ERR("read_sig_cb_null :       %ld\n", port->pstats.read_sig_cb_null);
    PCDEV_ERR("rel_ind_send :          %ld\n", port->pstats.rel_ind_send);
    PCDEV_ERR("rel_ind_cb_call :       %ld\n", port->pstats.rel_ind_cb_call);
    PCDEV_ERR("rel_ind_cb_null :       %ld\n", port->pstats.event_cb_null);
    PCDEV_ERR("msc_stru_send :          %ld\n", port->pstats.msc_stru_send);
    PCDEV_ERR("msc_stru_cb_call :       %ld\n", port->pstats.msc_stru_cb_call);
    PCDEV_ERR("msc_stru_cb_null :       %ld\n", port->pstats.msc_stru_cb_null);

    tx_rx_state_dump(port);
    return;
}

void pcie_cdev_port_info_dump(unsigned int port_num)
{
    struct pcie_cdev_port *port = NULL;
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();

    if (port_num >= PCIE_CDEV_COUNT) {
        PCDEV_ERR("port_num is %d \n", PCIE_CDEV_COUNT);
        return;
    }

    port = pcdev_ports[port_num].port;
    if (port == NULL) {
        PCDEV_ERR("port %d is NULL \n", port_num);
        return;
    }

    port_info_dump(port);
    state_dump(port);
}

static void pcdev_ctx_dmainfo_dump(void)
{
    struct pcdev_ctx *ctx = get_pcdev_ctx();
    PCDEV_ERR("ep:\n");
    PCDEV_ERR("kick_dma_trans_succ :   %ld\n", ctx->dma_ctx.kick_dma_trans_succ);
    PCDEV_ERR("kick_dma_trans_fail :   %ld\n", ctx->dma_ctx.kick_dma_trans_fail);
    PCDEV_ERR("dma_trans_cmp :         %ld\n", ctx->dma_ctx.dma_trans_cmp);

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    PCDEV_ERR("rc:\n");
    PCDEV_ERR("kick_dma_trans_succ :   %ld\n", ctx->dma_ctx.kick_dma_trans_succ_rc);
    PCDEV_ERR("kick_dma_trans_fail :   %ld\n", ctx->dma_ctx.kick_dma_trans_fail_rc);
    PCDEV_ERR("dma_trans_cmp :         %ld\n", ctx->dma_ctx.dma_trans_cmp_rc);
#endif
}

void pcie_info_dump(void)
{
    unsigned int i = 0;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();
    PCDEV_ERR("work mode :              %s\n", g_pcdev_work_mode[pcdev_ctx->work_mode]);
    PCDEV_ERR("pcie id :                %d\n", pcdev_ctx->pcie_id);
    PCDEV_ERR("init count :             %d\n", pcdev_ctx->init_count);
    PCDEV_ERR("exit count :             %d\n", pcdev_ctx->exit_count);
    PCDEV_ERR("diag mode :              %d\n", pcdev_ctx->diag_mode);
    PCDEV_ERR("physaddr :               %lx\n", (uintptr_t)pcdev_ctx->phys_addr);
    PCDEV_ERR("virtaddr :               %lx\n", (uintptr_t)pcdev_ctx->virt_addr);
    for (i = 0; i <= PCIE_CDEV_COUNT; i++) {
        PCDEV_ERR("vote info port(%d):\n", i);
        PCDEV_ERR("vote:                    %lld\n", pcdev_ctx->vote_dbg[i].vote_port);
        PCDEV_ERR("vote fail:               %lld\n", pcdev_ctx->vote_dbg[i].vote_fail_port);
        PCDEV_ERR("vote lasttime:           %lld\n", pcdev_ctx->vote_dbg[i].vote_lasttime);
        PCDEV_ERR("vote lastmode:           %d\n", pcdev_ctx->vote_dbg[i].vote_lastmode);
        PCDEV_ERR("unvote:                  %lld\n", pcdev_ctx->vote_dbg[i].unvote_port);
        PCDEV_ERR("unvote lasttime:         %lld\n", pcdev_ctx->vote_dbg[i].unvote_lasttime);
        PCDEV_ERR("unvote lastmode:         %d\n", pcdev_ctx->vote_dbg[i].unvote_lastmode);
    }

    PCDEV_ERR("buffer size :            %ld\n", pcdev_ctx->buffer_size);
    PCDEV_ERR("vote in interrupt :      %ld\n", pcdev_ctx->vote_in_interrupt);

    pcdev_ctx_dmainfo_dump();
}

void pcie_cdev_info_dump(void)
{
    int i;
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();
    pcie_info_dump();

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        PCDEV_ERR("desc addr :             %lx\n", (uintptr_t)pcdev_ports[i].desc);
        pcie_cdev_port_info_dump(i);
    }

    return;
}

static void pcie_cdev_print_data(unsigned int *data, unsigned int size)
{
    unsigned int i = 0;

    if ((size % PCDEV_DATA_PRT_WORD_DATA_NUM) != 0) {
        size = size / PCDEV_DATA_PRT_WORD_DATA_NUM + 1;
    } else {
        size /= PCDEV_DATA_PRT_WORD_DATA_NUM;
    }

    printk("data: ");
    for (i = 0; i < size; i++) {
        printk(KERN_CONT "0x%08X ", data[i]);
        if ((i > 0) && ((i % PCDEV_DATA_PRT_MAX_LEN_LINE) == 0)) {
            printk(KERN_CONT "\n");
        }
    }
    printk("\n");
}

void pcie_cdev_data_dump(unsigned int port_index, unsigned int dir, unsigned int desc_index)
{
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();
    struct pcie_cdev_port *port = NULL;
    struct pcdev_desc *desc = NULL;
    unsigned int phy_addr = 0;
    unsigned int *data = NULL;

    /* get port */
    if (port_index >= PCIE_CDEV_COUNT) {
        printk("pcie cdev max port count is %d \n", PCIE_CDEV_COUNT);
        return;
    }
    port = pcdev_ports[port_index].port;
    if (port == NULL) {
        printk("port is NULL\n");
        return;
    }

    /* get desc and addr */
    if (dir == 0) {
        /* rx desc */
        if (desc_index >= port->rx_num) {
            printk("pcie cdev max rx desc count is %d\n", port->rx_num);
            return;
        }
        desc = &port->rx_desc[desc_index];
        phy_addr = desc->d_dma_low;
    } else {
        /* tx desc */
        if (desc_index >= port->tx_num) {
            printk("pcie cdev max tx desc count is %d\n", port->tx_num);
            return;
        }
        desc = &port->tx_desc[desc_index];
        phy_addr = desc->s_dma_low;
    }

    printk("data size %d addr 0x%08X\n", desc->data_size, phy_addr);
    /* check data_size and dma_addr */
    if (desc->data_size == 0 || phy_addr == 0) {
        printk("data size or addr invalid\n");
        return;
    }

    data = (unsigned int *)phys_to_virt(phy_addr);
    pcie_cdev_print_data(data, desc->data_size);
}

void pcie_cdev_desc_dump(unsigned int n)
{
    unsigned int i;
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();
    struct pcie_cdev_port *port = NULL;
    struct pcdev_desc *desc = NULL;

    if (n >= PCIE_CDEV_COUNT) {
        printk("pcie cdev count is %d \n", PCIE_CDEV_COUNT);
        return;
    }
    port = pcdev_ports[n].port;
    if (port == NULL) {
        printk("port is NULL\n");
        return;
    }

    printk("\n[%s]rx desc\n", __func__);
    for (i = 0; i < port->rx_num; i++) {
        desc = &port->rx_desc[i];
        printk("[%d] own:0x%x  s_dma_low:0x%x  s_dma_high:0x%x  d_dma_low:0x%x  d_dma_high:0x%x\n", i, desc->own,
               desc->s_dma_low, desc->s_dma_high, desc->d_dma_low, desc->d_dma_high);
        printk("data_size:%d  userfield0:0x%llx  userfield1:0x%llx\n", desc->data_size, desc->userfield0,
               desc->userfield1);
    }

    printk("\n[%s]tx desc\n", __func__);
    for (i = 0; i < port->tx_num; i++) {
        desc = &port->tx_desc[i];
        printk("[%d] own:0x%x  s_dma_low:0x%x  s_dma_high:0x%x  d_dma_low:0x%x  d_dma_high:0x%x\n", i, desc->own,
               desc->s_dma_low, desc->s_dma_high, desc->d_dma_low, desc->d_dma_high);
        printk("data_size:%d  userfield0:0x%llx  userfield1:0x%llx\n", desc->data_size, desc->userfield0,
               desc->userfield1);
    }
}

void pcie_cdev_irq_table_show(void)
{
    unsigned int i;
    struct pcdev_ctx *pcdev = get_pcdev_ctx();

    PCDEV_ERR("\n");
    for (i = 0; i < PCDEV_MAX_INTR_COUNT; i++) {
        PCDEV_ERR("%s\tcnt(%d)\n", pcdev->irq_table[i].name, pcdev->irq_table[i].cnt);
    }
}

#define PCDEV_DBG_TIME 3000 /* ms unit */
#define PCDEV_DBG_TIMEOUT() (jiffies + msecs_to_jiffies(PCDEV_DBG_TIME))
int g_pcdev_dbg_timer_enable = 0x1;
unsigned int g_pcdev_dbg_timer_count = 0;
unsigned int g_pcdev_dbg_port = 13;
struct timer_list g_pcdev_dbg_timer;

void pcdev_dbg_enable_timer(void)
{
    g_pcdev_dbg_timer_enable = 0x1;
}

void pcdev_dbg_disable_timer(void)
{
    g_pcdev_dbg_timer_enable = 0x0;
}

void pcdev_dbg_port_set(unsigned int port_num)
{
    g_pcdev_dbg_port = port_num;
}

static void pcdev_dbg_timeout_isr(struct timer_list *t)
{
    g_pcdev_dbg_timer_count++;
    pcie_cdev_port_info_dump(g_pcdev_dbg_port);
    if (g_pcdev_dbg_timer_enable) {
        PCDEV_ERR("vcom wake timer count: %d", g_pcdev_dbg_timer_count);
        mod_timer(&g_pcdev_dbg_timer, PCDEV_DBG_TIMEOUT());
    } else {
        (void)del_timer(&g_pcdev_dbg_timer);
    }
    return;
}

void pcdev_dbg_timer_start(void)
{
    timer_setup(&g_pcdev_dbg_timer, pcdev_dbg_timeout_isr, 0);
    g_pcdev_dbg_timer.expires = PCDEV_DBG_TIMEOUT();
    add_timer(&g_pcdev_dbg_timer);
}

#ifdef CONFIG_PCDEV_DEBUG
void print_pkt(unsigned int port_num, unsigned char *buf, int len)
{
    int i, j, count;
    struct pcdev_ctx *pcdev_ctx = get_pcdev_ctx();

    if (!(pcdev_ctx->msg_level & PCDEV_LEVEL_PKT_DBG)) {
        return;
    }
    if (!(pcdev_ctx->print_port & (1 << port_num))) {
        return;
    }

    count = len > PCDEV_MAX_PRINT_COUNT ? PCDEV_MAX_PRINT_COUNT : len;
    pr_err("[pcdev]buf addr: 0x%llx  len: %d\n", (unsigned long long)(uintptr_t)buf, len);
    for (j = 0; j < count; j += PCDEV_PRINT_LEN) {
        pr_err("%03x: ", j);
        for (i = 0; i < PCDEV_PRINT_LEN; i++) {
            pr_err("%02x ", buf[j + i]);
        }
        pr_err("\n");
    }
    pr_err("\n");
}
#endif
