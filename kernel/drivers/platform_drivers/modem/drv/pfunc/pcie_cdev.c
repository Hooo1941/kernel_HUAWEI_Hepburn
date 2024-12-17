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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/tty.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <securec.h>
#include <bsp_pcdev.h>
#include <mdrv_om.h>

#include "pcie_cdev.h"
#include "pcie_cdev_ep.h"
#include "pcie_cdev_rc.h"
#include "pcie_cdev_desc.h"
#include "pcie_cdev_dbg.h"
#include "pcie_cdev_portinfo.h"
#include <bsp_net_om.h>
#include <bsp_file_ops.h>

#define PCIE_CDEV_PREFIX "pcdev_"
#define PCIE_CDEV_DRV_NAME "pcie_cdev"
#define PCIE_DUMP_PORT_MARK 0xFAFAFAFA
#define PCIE_CDEV_MAX_INTR 32
#define PCIE_REALLOC_WAIT_COUNT 300
#define PCIE_CDEV_MAX_IOCTL_CB 19

struct pcdev_ioctl_handler {
    unsigned int cmd;
    int (*pcdev_ioctl_cb)(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg);
};

static u64 g_pcdev_dma_mask = (u64)(-1);
struct pcdev_ctx g_pcdev_ctx;
struct pcie_cdev_port_manager g_pcie_cdev_ports[PCIE_CDEV_COUNT];
struct pcie_cdev_dump_s *g_pcdev_dump;

static struct pcie_cdev_driver *g_cdev_driver;
static unsigned int g_stat_drv_invalid = 0;
static unsigned int g_stat_port_num_err = 0;
static unsigned int g_stat_port_err = 0;
static struct pcie_cdev_evt_manage g_pcdev_write_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_read_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_sig_stat_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_read_sig_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_rel_ind_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_msc_stru_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_evt_send_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_read_sig_send_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_rel_ind_send_evt_manage;
static struct pcie_cdev_evt_manage g_pcdev_msc_stru_send_evt_manage;

static struct delayed_work g_pcdev_access_work;
static struct delayed_work g_pcdev_init_work;
static struct task_struct *g_pcdev_enumdone_check = NULL;
struct class *g_pcdev_class;

static void pcie_cdev_notify_cb(struct pcie_cdev_port *port);
static void pcie_cdev_read_sig_cb(struct pcie_cdev_port *port);
static void pcie_cdev_rel_ind_cb(struct pcie_cdev_port *port);
static void pcie_cdev_msc_stru_cb(struct pcie_cdev_port *port);
static void pcie_cdev_read_cb(struct pcie_cdev_port *port);
static void pcie_cdev_write_cb(struct pcie_cdev_port *port);
static void pcdev_send_evt_cb(struct pcie_cdev_port *port);
static void pcdev_modem_msc_write_cb(struct pcie_cdev_port *port);
static void pcdev_send_read_sig_cb(struct pcie_cdev_port *port);
static void pcdev_send_rel_ind_cb(struct pcie_cdev_port *port);
static int pcie_cdev_alloc_read_buf(struct pcie_cdev_port *port);
static int pcie_cdev_free_read_buf(struct pcie_cdev_port *port);
static void pcdev_tx_process(struct pcie_cdev_port *port);
static void pcdev_rx_process(struct pcie_cdev_port *port);
static void pcdev_event_send_irq(struct pcie_cdev_port *port);
static void pcdev_read_sig_send_irq(struct pcie_cdev_port *port);
static void pcdev_rel_ind_send_irq(struct pcie_cdev_port *port);
static void pcdev_msc_stru_send_irq(struct pcie_cdev_port *port);
static struct pcie_cdev_port *pcdev_get_port(struct inode *inode);

int kick_dma_transfer(struct pcie_cdev_port *port);
int pcie_cdev_get_read_buf(struct pcie_cdev_port *port, pcdev_wr_async_info_s *wr_info);
int pcie_cdev_ret_read_buf(struct pcie_cdev_port *port, pcdev_wr_async_info_s *wr_info);

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
int kick_dma_transfer_read(void);
static void pcdev_dma_send_irq(struct pcie_cdev_port *port);
static void pcdev_dma_send_cmp_irq(struct pcie_cdev_port *port);

void pcie_cdev_dma_read_process(struct pcie_cdev_port *port);
#endif

#ifdef CONFIG_PCIE_CDEV_ENG
struct pcie_cdev_name_type_tbl g_pcie_cdev_type_table[PCIE_CDEV_COUNT] = {
    /* name             type(prot id)   user_space_check */
    { "pcdev_ttyGS0",    IF_PROTOCOL_PCUI,       PCUI_RC_BUFSIZE, PCUI_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_gps",       IF_PROTOCOL_GPS,        GPS_RC_BUFSIZE, GPS_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_4g_diag",   IF_PROTOCOL_DIAG,       G4DIAG_RC_BUFSIZE, G4DIAG_EP_BUFSIZE, MAX_DESC_NUM, 1, 1 },
    { "pcdev_3g_diag",   IF_PROTOCOL_3G_DIAG,    G3DIAG_RC_BUFSIZE, G3DIAG_EP_BUFSIZE, 256, 1, 1 },
    { "pcdev_c_shell",   IF_PROTOCOL_BLUETOOTH,  RESERVE_BUFSIZE, RESERVE_BUFSIZE, 256, 1, 1 },
    { "pcdev_ctrl",      IF_PROTOCOL_CTRL,       CTRL_RC_BUFSIZE, CTRL_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_skytone",   IF_PROTOCOL_3G_PCUI,    SKYTONE_RC_BUFSIZE, SKYTONE_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_cdma_log",  IF_PROTOCOL_CDMA_LOG,   RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 1},
    { "pcdev_voice",     IF_PROTOCOL_PCVOICE,    RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 1 },
    { "pcdev_agent_nv",  IF_PROTOCOL_AGENT_NV,   AGENTNV_RC_BUFSIZE, AGENTNV_EP_BUFSIZE, 4, 1, 1 },
    { "pcdev_agent_om",  IF_PROTOCOL_AGENT_OM,   AGENTOM_RC_BUFSIZE, AGENTOM_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_agent_msg", IF_PROTOCOL_AGENT_MSG,  AGENTMSG_RC_BUFSIZE, AGENTMSG_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_a_shell",   IF_PROTOCOL_3G_GPS,     ASHELL_RC_BUFSIZE, ASHELL_EP_BUFSIZE, 256, 1, 1 },
    { "pcdev_appvcom",   IF_PROTOCOL_APPVCOM,    RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 1 },
    { "pcdev_modem",     IF_PROTOCOL_MODEM,      MODEM_RC_BUFSIZE, MODEM_EP_BUFSIZE, 256, 1, 1 },
    { "pcdev_pmom",      IF_PROTOCOL_PMOM,       PMOM_RC_BUFSIZE, PMOM_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_reserve",   IF_PROTOCOL_RESERVE,    RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 1 },
};
#else
struct pcie_cdev_name_type_tbl g_pcie_cdev_type_table[PCIE_CDEV_COUNT] = {
    /* name             type(prot id)   user_space_check */
    { "pcdev_ttyGS0",    IF_PROTOCOL_PCUI,       PCUI_RC_BUFSIZE, PCUI_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_gps",       IF_PROTOCOL_GPS,        GPS_RC_BUFSIZE, GPS_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_4g_diag",   IF_PROTOCOL_DIAG,       G4DIAG_RC_BUFSIZE, G4DIAG_EP_BUFSIZE, MAX_DESC_NUM, 1, 1 },
    { "pcdev_3g_diag",   IF_PROTOCOL_3G_DIAG,    G3DIAG_RC_BUFSIZE, G3DIAG_EP_BUFSIZE, 256, 1, 0 },
    { "pcdev_c_shell",   IF_PROTOCOL_BLUETOOTH,  RESERVE_BUFSIZE, RESERVE_BUFSIZE, 256, 1, 0 },
    { "pcdev_ctrl",      IF_PROTOCOL_CTRL,       CTRL_RC_BUFSIZE, CTRL_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_skytone",   IF_PROTOCOL_3G_PCUI,    SKYTONE_RC_BUFSIZE, SKYTONE_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_cdma_log",  IF_PROTOCOL_CDMA_LOG,   RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 0},
    { "pcdev_voice",     IF_PROTOCOL_PCVOICE,    RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 0 },
    { "pcdev_agent_nv",  IF_PROTOCOL_AGENT_NV,   AGENTNV_RC_BUFSIZE, AGENTNV_EP_BUFSIZE, 4, 1, 1 },
    { "pcdev_agent_om",  IF_PROTOCOL_AGENT_OM,   AGENTOM_RC_BUFSIZE, AGENTOM_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_agent_msg", IF_PROTOCOL_AGENT_MSG,  AGENTMSG_RC_BUFSIZE, AGENTMSG_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_a_shell",   IF_PROTOCOL_3G_GPS,     ASHELL_RC_BUFSIZE, ASHELL_EP_BUFSIZE, 256, 1, 0 },
    { "pcdev_appvcom",   IF_PROTOCOL_APPVCOM,    RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 0 },
    { "pcdev_modem",     IF_PROTOCOL_MODEM,      MODEM_RC_BUFSIZE, MODEM_EP_BUFSIZE, 256, 1, 0 },
    { "pcdev_pmom",      IF_PROTOCOL_PMOM,       PMOM_RC_BUFSIZE, PMOM_EP_BUFSIZE, 32, 1, 1 },
    { "pcdev_reserve",   IF_PROTOCOL_RESERVE,    RESERVE_BUFSIZE, RESERVE_BUFSIZE, 32, 1, 0 },
};
#endif

struct intr_handler g_pcdev_intr_table[PCIE_CDEV_MAX_INTR] = {
    { "RX_CMP", 0, pcdev_tx_process },
    { "TX_CMP", 0, pcdev_rx_process },
    { "EVENT_SEND", 0, pcdev_event_send_irq },
    { "READ_SIG_SEND", 0, pcdev_read_sig_send_irq },
    { "REL_IND_SEND", 0, pcdev_rel_ind_send_irq },
    { "MSC_STRU_SEND", 0, pcdev_msc_stru_send_irq },
#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    { "DMA_SEND", 0, pcdev_dma_send_irq },
    { "DMA_SEND_CMP", 0, pcdev_dma_send_cmp_irq },
#endif
};

#define PCIE_CDEV_GET_NAME(index) \
    ((g_pcie_cdev_type_table[index].name == NULL) ? ("unknown") : (g_pcie_cdev_type_table[index].name))
#define pcie_cdev_evt_push(port, evt) __pcie_cdev_rw_push((void *)(port), (evt))
static void __pcie_cdev_rw_push(void *port, struct pcie_cdev_evt_manage *evt)
{
    unsigned long flags;
    int add_new = 1;
    int i;

    spin_lock_irqsave(&evt->evt_lock, flags);
    for (i = 0; i <= evt->port_evt_pos; i++) {
        if (evt->port_evt_array[i] == port) {
            add_new = 0;
            break;
        }
    }
    if (add_new) {
        evt->port_evt_array[evt->port_evt_pos] = port;
        evt->port_evt_pos++;
    }
    spin_unlock_irqrestore(&evt->evt_lock, flags);
}

static int pcie_cdev_vote(int mode, unsigned int port_num)
{
    u64 curtime;
    struct pcdev_vote_dbg_s *vote_dbg = &g_pcdev_ctx.vote_dbg[port_num];
    vote_dbg->vote_port++;
    g_pcdev_ctx.get_curtime(&curtime);
    vote_dbg->vote_lasttime = curtime;
    vote_dbg->vote_lastmode = mode;
    if (g_pcdev_ctx.pcdev_vote_lock(mode)) {
        PCDEV_ERR("vote fail\n");
        vote_dbg->vote_fail_port++;
        return -ENOTBLK;
    }

    return 0;
}

static void pcie_cdev_unvote(int mode, unsigned int port_num)
{
    u64 curtime;
    struct pcdev_vote_dbg_s *vote_dbg = &g_pcdev_ctx.vote_dbg[port_num];
    vote_dbg->unvote_port++;
    g_pcdev_ctx.get_curtime(&curtime);
    vote_dbg->unvote_lasttime = curtime;
    vote_dbg->unvote_lastmode = mode;
    (void)g_pcdev_ctx.pcdev_vote_unlock(mode);
}

static inline int pcie_cdev_linkdown(void)
{
    unsigned int allocked;

    allocked = readl(&g_pcdev_ctx.ports_desc->allocked);
    return allocked != PCIE_CDEV_ALLOCED;
}

static inline void pcie_cdev_evt_init(struct pcie_cdev_evt_manage *evt, char *name)
{
    int ret;
    spin_lock_init(&evt->evt_lock);
    evt->port_evt_pos = 0;
    evt->name = name;
    ret = memset_s(evt->port_evt_array, sizeof(evt->port_evt_array), 0, PCIE_CDEV_COUNT * sizeof(void *));
    if (ret) {
        PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
    }
}

#if defined(CONFIG_PCIE_DEV_NORMALIZE) && defined(CONFIG_PCIE_CDEV_RC)
static int fs_user_space_check(int port_num)
{
    return 0;
}
#else
static int fs_user_space_check(int port_num)
{
    if (g_pcie_cdev_type_table[port_num].user_space_check) {
        if (get_fs() != KERNEL_DS) {
            PCDEV_ERR("can't support in usr space\n");
            return -ENOTSUPP;
        }
    }

    return 0;
}
#endif

void pcdev_ports_ctx_clear(void)
{
    int ret;
    ret = memset_s(g_pcie_cdev_ports, sizeof(g_pcie_cdev_ports), 0,
                   sizeof(struct pcie_cdev_port_manager) * PCIE_CDEV_COUNT);
    if (ret) {
        PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
    }
}

void pcdev_ctx_clear(void)
{
    int ret;
    ret = memset_s(&g_pcdev_ctx, sizeof(g_pcdev_ctx), 0, sizeof(struct pcdev_ctx));
    if (ret) {
        PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
    }
}

struct pcdev_ctx* get_pcdev_ctx(void)
{
    return &g_pcdev_ctx;
}

struct pcie_cdev_port_manager* get_pcdev_ports(void)
{
    return g_pcie_cdev_ports;
}

struct pcie_cdev_name_type_tbl* get_pcdev_type_table(void)
{
    return g_pcie_cdev_type_table;
}

static void pcdev_tx_process(struct pcie_cdev_port *port)
{
    PCDEV_TRACE("in\n");

    pcie_cdev_evt_push(port, &g_pcdev_write_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return;
}

static void pcdev_rx_process(struct pcie_cdev_port *port)
{
    PCDEV_TRACE("in\n");
    PCDEV_INFO(port->port_num, "in\n");

    pcie_cdev_evt_push(port, &g_pcdev_read_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return;
}

static void pcdev_event_send_irq(struct pcie_cdev_port *port)
{
    pcie_cdev_evt_push(port, &g_pcdev_sig_stat_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return;
}

static void pcdev_read_sig_send_irq(struct pcie_cdev_port *port)
{
    pcie_cdev_evt_push(port, &g_pcdev_read_sig_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return;
}

static void pcdev_rel_ind_send_irq(struct pcie_cdev_port *port)
{
    pcie_cdev_evt_push(port, &g_pcdev_rel_ind_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return;
}

static void pcdev_msc_stru_send_irq(struct pcie_cdev_port *port)
{
    pcie_cdev_evt_push(port, &g_pcdev_msc_stru_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return;
}

static void *pcie_cdev_evt_get(struct pcie_cdev_evt_manage *evt)
{
    unsigned long flags;
    void *ret_port = NULL;

    spin_lock_irqsave(&evt->evt_lock, flags);
    if (evt->port_evt_pos > 0) {
        ret_port = evt->port_evt_array[evt->port_evt_pos - 1];
        evt->port_evt_array[evt->port_evt_pos - 1] = NULL;
        evt->port_evt_pos--;
    }
    spin_unlock_irqrestore(&evt->evt_lock, flags);

    return ret_port;
}

/*
 * rw workqueue takes data out of the RX queue and hands it up to the TTY
 * layer until it refuses to take any more data (or is throttled back).
 * Then it issues reads for any further data.
 */
static void pcie_cdev_rw_push(struct work_struct *work)
{
    struct pcie_cdev_port *port = NULL;

    /* notify callback */
    while ((port = pcie_cdev_evt_get(&g_pcdev_sig_stat_evt_manage)) != NULL) {
        pcie_cdev_notify_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_read_sig_evt_manage)) != NULL) {
        pcie_cdev_read_sig_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_rel_ind_evt_manage)) != NULL) {
        pcie_cdev_rel_ind_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_msc_stru_evt_manage)) != NULL) {
        pcie_cdev_msc_stru_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_evt_send_evt_manage)) != NULL) {
        pcdev_send_evt_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_read_sig_send_evt_manage)) != NULL) {
        pcdev_send_read_sig_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_rel_ind_send_evt_manage)) != NULL) {
        pcdev_send_rel_ind_cb(port);
    }

    while ((port = pcie_cdev_evt_get(&g_pcdev_msc_stru_send_evt_manage)) != NULL) {
        pcdev_modem_msc_write_cb(port);
    }

    /* read callback */
    while ((port = pcie_cdev_evt_get(&g_pcdev_read_evt_manage)) != NULL) {
        pcie_cdev_read_cb(port);
    }
    g_pcdev_ctx.vote_flag = 0xffff;
    /* write callback */
    while ((port = pcie_cdev_evt_get(&g_pcdev_write_evt_manage)) != NULL) {
        pcie_cdev_write_cb(port);
    }

    return;
}

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
static int pcie_cdev_dma_read_complete(u32 direction, u32 status, void *dev_info)
{
    unsigned long flags;
    unsigned long dma_flags;
    struct pcie_cdev_dma_list *list = NULL;
    struct pcie_cdev_port *port = (struct pcie_cdev_port *)dev_info;
    unsigned int s_tx_rp_dma_cmp_rc;
    unsigned int need_kick = 0;

    PCDEV_TRACE("in\n");
    g_pcdev_ctx.dma_ctx.dma_trans_cmp_rc++;

    if (g_pcdev_ctx.work_mode != PCIE_EP) {
        PCDEV_ERR("dma send cmp mode err! \n");
        return -1;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (status) {
        PCDEV_ERR("dma read fail\n");
        port->pstats.rx_packets_fail++;
        if (kick_dma_transfer_read()) {
            PCDEV_ERR("kick_dma_transfer_read fail\n");
        }
        goto out;
    }

    if (!port->pstats.openclose) {
        port->pstats.local_close++;
        goto out;
    }

    s_tx_rp_dma_cmp_rc = readl(port->tx_rp_dma_cmp_rc);
    PCDEV_INFO(port->port_num, "buf(%d)\n", s_tx_rp_dma_cmp_rc);
    s_tx_rp_dma_cmp_rc++;
    writel(s_tx_rp_dma_cmp_rc, port->tx_rp_dma_cmp_rc);
    s_tx_rp_dma_cmp_rc = readl(port->tx_rp_dma_cmp_rc);

    spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);
    list = list_last_entry(&g_pcdev_ctx.dma_ctx.head_rc, struct pcie_cdev_dma_list, list);
    list_del(&list->list);

    if (!list_empty(&g_pcdev_ctx.dma_ctx.head_rc)) {
        need_kick = 1;
    }
    spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);

    if (need_kick) {
        kick_dma_transfer_read();
    }

    writel(1, port->pcie_cdev_send_irq[DMA_SEND_CMP]);
    (void)readl(port->pcie_cdev_send_irq[DMA_SEND_CMP]);

    g_pcdev_ctx.send_irq();
out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    return 0;
}

static void pcdev_dma_send_irq(struct pcie_cdev_port *port)
{
    if (g_pcdev_ctx.work_mode != PCIE_EP) {
        PCDEV_ERR("dma send mode err! \n");
        return;
    }

    pcie_cdev_dma_read_process(port);

    return;
}

void pcdev_dma_send_cmp_irq(struct pcie_cdev_port *port)
{
    unsigned long flags;
    unsigned int tx_rp_dma_cmp_rc;

    PCDEV_TRACE("in\n");

    if (g_pcdev_ctx.work_mode != PCIE_RC) {
        PCDEV_ERR("dma send cmp mode err! \n");
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        spin_unlock_irqrestore(&port->port_lock, flags);
        return;
    }

    tx_rp_dma_cmp_rc = readl(port->tx_rp_dma_cmp_rc);
    while (port->tx_rp_cmp != tx_rp_dma_cmp_rc) {
        if (!port->dma_callback) {
            port->pstats.callback_null++;
        } else {
            spin_unlock_irqrestore(&port->port_lock, flags);
            port->dma_callback(PCIE_DMA_DIRECTION_WRITE, 0, (void *)port);
            spin_lock_irqsave(&port->port_lock, flags);
        }
        port->tx_rp_cmp++;
    }

    spin_unlock_irqrestore(&port->port_lock, flags);
    return;
}

#endif

static void pcie_cdev_irq_process(struct pcie_cdev_port *port)
{
    int j;
    for (j = 0; j < IRQ_NUM; j++) {
        if (*(port->pcie_cdev_rcv_irq[j])) {
            *(port->pcie_cdev_rcv_irq[j]) = 0;
            g_pcdev_intr_table[j].cnt++;
            if (g_pcdev_intr_table[j].callback) {
                g_pcdev_intr_table[j].callback(port);
            }
        }
    }
}

static irqreturn_t pcie_cdev_irq_handler(void)
{
    int i;
    struct pcie_cdev_port *port = NULL;

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        port = g_pcie_cdev_ports[i].port;
        if (port == NULL) {
            g_stat_port_err++;
            break;
        }

        if (port->pstats.openclose == false) {
            continue;
        }

        pcie_cdev_irq_process(port);
    }

    return IRQ_HANDLED;
}

static int pcie_cdev_get_port_num(struct inode *inode)
{
    int port_num;

    if (!g_cdev_driver || !inode) {
        g_stat_drv_invalid++;
        return -ENXIO;
    }

    port_num = inode->i_rdev - g_cdev_driver->dev_no;

    if (port_num >= PCIE_CDEV_COUNT) {
        g_stat_port_num_err++;
        return -ENXIO;
    }

    return port_num;
}

void pcie_cdev_notify_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    pcdev_evt_e event_type;
    PCDEV_TRACE("in\n");

    if (!port->event_cb) {
        port->pstats.event_cb_null++;
        return;
    }

    port->pstats.event_cb_call++;
    if (pcie_cdev_vote(NOTIFY_CB, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (!port->pstats.openclose) {
        port->pstats.local_close++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        pcie_cdev_unvote(NOTIFY_CB, port->port_num);
        return;
    }

    event_type = (pcdev_evt_e)*port->event_type;
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(NOTIFY_CB, port->port_num);
    port->event_cb(event_type);
    return;
}

void pcie_cdev_read_sig_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    pcdev_mdm_msc_stru *read_sig = NULL;
    PCDEV_TRACE("in\n");

    if (!port->read_sig_cb) {
        port->pstats.read_sig_cb_null++;
        return;
    }

    port->pstats.read_sig_cb_call++;

    if (pcie_cdev_vote(READ_SIG_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (!port->pstats.openclose) {
        port->pstats.local_close++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        pcie_cdev_unvote(READ_SIG_MODE, port->port_num);
        return;
    }

    read_sig = port->read_sig;
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(READ_SIG_MODE, port->port_num);
    port->read_sig_cb(read_sig);

    return;
}

void pcie_cdev_rel_ind_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    unsigned int rel_ind;
    PCDEV_TRACE("in\n");

    if (!port->rel_ind_cb) {
        port->pstats.rel_ind_cb_null++;
        return;
    }

    port->pstats.rel_ind_cb_call++;
    if (pcie_cdev_vote(REL_IND_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (!port->pstats.openclose) {
        port->pstats.local_close++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        pcie_cdev_unvote(REL_IND_MODE, port->port_num);
        return;
    }

    rel_ind = *port->rel_ind;
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(REL_IND_MODE, port->port_num);
    port->rel_ind_cb(rel_ind);

    return;
}

void pcie_cdev_msc_stru_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    PCDEV_TRACE("in\n");

    if (!port->msc_stru_cb) {
        port->pstats.rel_ind_cb_null++;
        return;
    }

    port->pstats.msc_stru_cb_call++;
    if (pcie_cdev_vote(MSC_WRITE_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (!port->pstats.openclose) {
        port->pstats.local_close++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        pcie_cdev_unvote(MSC_WRITE_MODE, port->port_num);
        return;
    }

    memcpy_fromio(&port->msc_stru_arg, port->msc_stru, sizeof(pcdev_mdm_msc_stru));

    *port->msc_busy = 0;
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(MSC_WRITE_MODE, port->port_num);

    port->msc_stru_cb(&port->msc_stru_arg);

    return;
}

static void pcie_cdev_ret_buf(struct pcie_cdev_port *port, pcdev_wr_async_info_s *wr_info)
{
    unsigned int entry;
    struct pcdev_desc *desc = NULL;
    dma_addr_t d_pbuf = 0;
    int ret;

    PCDEV_TRACE("in\n");

    entry = *port->rx_wp % port->rx_num;
    desc = port->rx_desc + entry;
    if (desc == NULL) {
        port->pstats.rx_desc_null++;
        return;
    }

    if (desc->own) {
        port->pstats.rx_desc_err++;
        return;
    }

    desc->data_size = wr_info->size;
    PCDEV_INFO(port->port_num, "buf(%d)\n", *port->rx_wp);
    if (port->set_readbuf) {
        d_pbuf = (dma_addr_t)(uintptr_t)wr_info->p_paddr;
        desc->d_vaddr = (unsigned long long)(uintptr_t)wr_info->p_vaddr;
    } else {
        if (wr_info->p_vaddr) {
            desc->d_vaddr = (unsigned long long)(uintptr_t)wr_info->p_vaddr;
            port->d_vaddr[entry] = (unsigned long long)(uintptr_t)wr_info->p_vaddr;
            d_pbuf = dma_map_single(port->dev, (void *)wr_info->p_vaddr, port->rx_buf_max_size, DMA_FROM_DEVICE);
        } else {
            d_pbuf = (dma_addr_t)(uintptr_t)wr_info->p_paddr;
        }
    }

    desc->d_dma_low = (unsigned int)(d_pbuf & 0xFFFFFFFFU);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
    desc->d_dma_high = (unsigned int)((d_pbuf >> 32U) & 0xFFFFFFFFU);
#else
    desc->d_dma_high = 0;
#endif

    *port->rx_wp = *port->rx_wp + 1;
    port->pstats.rx_packets++;

    writel(1, port->pcie_cdev_send_irq[RX_CMP]);
    (void)readl(port->pcie_cdev_send_irq[RX_CMP]);
    ret = g_pcdev_ctx.send_irq();
    if (!ret) {
        port->pstats.irq_send++;
    }

    return;
}

static int read_cb_check(struct pcie_cdev_port *port, unsigned int *s_rx_rp)
{
    struct pcdev_buf_list *pos = NULL;
    struct pcdev_buf_list *n = NULL;

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        return -1;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return -1;
    }

    if (!port->local_stat->bits.read_start) {
        port->pstats.rx_close++;
        return -1;
    }

    list_for_each_entry_safe(pos, n, &port->ret_buf_head, list)
    {
        pcie_cdev_ret_buf(port, &pos->buf_info);
        list_move(&pos->list, &port->read_buf_head);
    }

    *s_rx_rp = *port->rx_rp;
    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return -1;
    }

    if (port->rx_rp_process == *s_rx_rp) {
        port->pstats.rx_process_empty++;
        return -1;
    }

    return 0;
}

static void pcie_pm_vote_check(struct pcie_cdev_port *port)
{
    int ret;
    if (g_pcdev_ctx.vote_flag == VOTE_FLAG && g_pcdev_ctx.pcie_first_user) {
        ret = g_pcdev_ctx.pcie_first_user(PCIE_EP_MSI_CHAR_DEV);
        if (ret) {
            printk(KERN_ERR "[C SR][pcdev]:port_name:%s\n", g_pcie_cdev_type_table[port->port_num].name);
        } else {
            g_pcdev_ctx.vote_flag = 0xffff;
        }
    }
}

void read_dump_save(unsigned int entry, struct pcie_cdev_port *port, struct pcdev_desc *desc)
{
    if (g_pcdev_dump) {
        entry = *port->rx_rp % port->tx_num;
        desc = port->tx_desc + entry;
        g_pcdev_dump->rx_desc_dump[port->port_num].vaddr = desc->d_vaddr;
        g_pcdev_dump->rx_desc_dump[port->port_num].dma_low = desc->d_dma_low;
        g_pcdev_dump->rx_desc_dump[port->port_num].dma_high = desc->d_dma_high;
    }
}

void pcdev_node_read_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;

    spin_lock_irqsave(&port->port_lock, flags);
    port->read_blocked++;
    spin_unlock_irqrestore(&port->port_lock, flags);
    wake_up_interruptible(&port->read_wait);
}

void pcie_cdev_read_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    unsigned int s_rx_rp;
    int ret;

    PCDEV_TRACE("in, port num:%d\n", port->port_num);

    if (pcie_cdev_vote(READ_CB_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    ret = read_cb_check(port, &s_rx_rp);
    if (ret) {
        goto out;
    }

    pcie_pm_vote_check(port);

    do {
        PCDEV_INFO(port->port_num, "read_done_cb(%d)\n", port->rx_rp_process);
        port->rx_rp_process++;
        port->pstats.read_cb_call++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        pcie_cdev_unvote(READ_CB_MODE, port->port_num);
        if (port->read_done_cb != NULL) {
            port->read_done_cb();
        } else {
            pcdev_node_read_cb(port);
        }

        if (pcie_cdev_vote(READ_CB_MODE, port->port_num)) {
            return;
        }
        spin_lock_irqsave(&port->port_lock, flags);
        if (port->pstats.openclose == false) {
            port->pstats.local_close++;
            goto out;
        }
    } while (port->rx_rp_process != s_rx_rp);

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(READ_CB_MODE, port->port_num);
    return;
}

static int write_cb_port_check(struct pcie_cdev_port *port)
{
    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        return -1;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return -1;
    }
    return 0;
}

static struct pcdev_desc *write_cb_status_check(struct pcie_cdev_port *port, int *entry)
{
    struct pcdev_desc *desc = NULL;
    if ((*port->tx_wp - port->tx_num) == port->tx_rp_complete) {
        return NULL;
    }

    *entry = port->tx_rp_complete % port->tx_num;
    desc = port->tx_desc + *entry;
    if (desc == NULL) {
        port->pstats.tx_desc_null++;
        return NULL;
    }

    if (desc->own) {
        port->pstats.tx_desc_err++;
        return NULL;
    }
    return desc;
}

static dma_addr_t write_cb_desc_set(unsigned long long *virt_addr, unsigned int *data_size,
                                    struct pcdev_desc *desc)
{
    dma_addr_t s_pbuf;
    s_pbuf = desc->s_dma_low;
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
    s_pbuf = ((unsigned long long)desc->s_dma_high << 32) + s_pbuf; /* get higher 32 bits of d_dma_high */
#endif

    *virt_addr = desc->s_vaddr;
    *data_size = desc->data_size;
    desc->s_vaddr = 0;
    desc->s_dma_low = 0;
    desc->s_dma_high = 0;
    desc->own = 1;
    return s_pbuf;
}

static void write_cb_count(struct pcie_cdev_port *port, dma_addr_t s_pbuf, unsigned int u32Size)
{
    if (!port->dmamap_skip) {
        dma_unmap_single(port->dev, s_pbuf, u32Size, DMA_TO_DEVICE);
    }

    g_pcdev_ctx.get_curtime((u64 *)&port->timestamp[port->tx_rp_complete % PCDEV_TIMESTAMP_COUNT].tx_packets_cb);
    port->tx_rp_complete++;
    port->pstats.tx_packets_cb++;
}

static void write_sync_process(struct pcie_cdev_port *port, dma_addr_t s_pbuf, unsigned int u32Size)
{
    port->write_blocked = 0;
    wake_up_interruptible(&port->write_wait);
    write_cb_count(port, s_pbuf, u32Size);
}

static void write_cb_list_del(struct pcie_cdev_port *port)
{
    struct pcdev_buf_list *list = NULL;

    list = list_last_entry(&port->write_buf_head, struct pcdev_buf_list, list);
    list_del(&list->list);
}

void pcie_cdev_done_cb(struct pcie_cdev_port *port, unsigned long long virt_addr,
    dma_addr_t s_pbuf, unsigned int data_size)
{
    pcdev_write_info_s rw_info;

    if (port->write_done_cb) {
        port->write_done_cb((char *)(uintptr_t)virt_addr, (char *)s_pbuf, data_size);
    } else if (port->write_info_done_cb) {
        rw_info.p_vaddr = (char *)(uintptr_t)virt_addr;
        rw_info.p_paddr = (char *)(uintptr_t)s_pbuf;
        rw_info.size = data_size;
        port->write_info_done_cb(&rw_info);
    }
}

void pcie_cdev_write_cb_stat(struct pcie_cdev_port *port)
{
    if (port->write_done_cb) {
        port->pstats.write_cb_call++;
    } else if (port->write_info_done_cb) {
        port->pstats.write_info_cb_call++;
    } else {
        port->pstats.write_cb_null++;
    }
}

void pcie_cdev_write_cb(struct pcie_cdev_port *port)
{
    struct pcdev_desc *desc = NULL;
    unsigned int entry, data_size;
    unsigned long flags;
    dma_addr_t s_pbuf = 0;
    unsigned long long virt_addr;

    PCDEV_TRACE("in\n");
    if (pcie_cdev_vote(WRITE_CB_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    if (write_cb_port_check(port)) {
        goto out;
    }

    for (;;) {
        desc = write_cb_status_check(port, &entry);
        if (IS_ERR_OR_NULL(desc)) {
            break;
        }

        s_pbuf = write_cb_desc_set(&virt_addr, &data_size, desc);

        if (port->write_sync) {
            write_sync_process(port, s_pbuf, data_size);
            continue;
        }

        write_cb_list_del(port);
        pcie_cdev_unvote(WRITE_CB_MODE, port->port_num);
        spin_unlock_irqrestore(&port->port_lock, flags);

        pcie_cdev_done_cb(port, virt_addr, s_pbuf, data_size);

        if (pcie_cdev_vote(WRITE_CB_MODE, port->port_num)) {
            return;
        }
        spin_lock_irqsave(&port->port_lock, flags);

        pcie_cdev_write_cb_stat(port);

        if (port->pstats.openclose == false) {
            port->pstats.local_close++;
            goto out;
        }
        write_cb_count(port, s_pbuf, data_size);
    }

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(WRITE_CB_MODE, port->port_num);
    return;
}

void dump_transfer_info(struct pcie_dma_transfer_info dma_info, struct pcie_cdev_port *port)
{
    bsp_err("\n");
    bsp_err("port num (%d) : \n", port->port_num);
    bsp_err("id: %d \n", dma_info.id);
    bsp_err("channel: %d \n", dma_info.channel);
    bsp_err("direction: %d \n", dma_info.direction);
    bsp_err("element_cnt: %d \n", dma_info.element_cnt);
    bsp_err("transfer_type: %d \n", dma_info.transfer_type);
    bsp_err("remote_int_enable: %d \n", dma_info.remote_int_enable);
}

int kick_dma_transfer(struct pcie_cdev_port *port)
{
    struct pcie_cdev_dma_list *list = NULL;
    unsigned long dma_flags;
    int ret;

    spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
    list = list_last_entry(&g_pcdev_ctx.dma_ctx.head, struct pcie_cdev_dma_list, list);
    spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
    port->pstats.kick_dma_transfer++;

    ret = g_pcdev_ctx.dma_transfer(0, &list->dma_info);
    if (ret) {
        PCDEV_ERR("dma send fail\n");
        port->pstats.kick_dma_transfer_fail++;
        g_pcdev_ctx.dma_ctx.kick_dma_trans_fail++;
        dump_transfer_info(list->dma_info, port);
    } else {
        g_pcdev_ctx.dma_ctx.kick_dma_trans_succ++;
    }

    return ret;
}

int kick_dma_transfer_read(void)
{
    int ret = 0;
#ifdef CONFIG_PCIE_CDEV
    struct pcie_cdev_dma_list *list = NULL;
    unsigned long dma_flags;

    PCDEV_TRACE("in\n");
    spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);
    list = list_last_entry(&g_pcdev_ctx.dma_ctx.head_rc, struct pcie_cdev_dma_list, list);
    spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);

    ret = bsp_pcie_ep_dma_transfer(0, &list->dma_info);
    if (ret) {
        PCDEV_ERR("dma get fail\n");
        g_pcdev_ctx.dma_ctx.kick_dma_trans_fail_rc++;
    } else {
        g_pcdev_ctx.dma_ctx.kick_dma_trans_succ_rc++;
    }
#endif
    return ret;
}

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
void pcie_cdev_dma_read_process(struct pcie_cdev_port *port)
{
    unsigned long flags;
    unsigned long dma_flags;
    struct pcdev_desc *desc = NULL;
    unsigned int entry;
    unsigned int is_empty = 0;

    PCDEV_TRACE("in\n");

    spin_lock_irqsave(&port->port_lock, flags);
    while (port->tx_rp_dma_cfg_rc != *port->tx_rp_todo_rc) {
        PCDEV_INFO(port->port_num, "buf(%d)\n", port->tx_rp_dma_cfg_rc);
        entry = port->tx_rp_dma_cfg_rc % port->tx_num;
        desc = port->rx_desc + entry;

        port->transfer_info_rc[entry].dma_info.element.sar_low = desc->s_dma_low;
        port->transfer_info_rc[entry].dma_info.element.sar_high = desc->s_dma_high;
        port->transfer_info_rc[entry].dma_info.element.dar_low = desc->d_dma_low;
        port->transfer_info_rc[entry].dma_info.element.dar_high = desc->d_dma_high;
        port->transfer_info_rc[entry].dma_info.element.transfer_size = desc->data_size;

        port->tx_rp_dma_cfg_rc++;

        spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);
        if (list_empty(&g_pcdev_ctx.dma_ctx.head_rc)) {
            is_empty = 1;
        } else {
            is_empty = 0;
        }
        list_add(&port->transfer_info_rc[entry].list, &g_pcdev_ctx.dma_ctx.head_rc);
        spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);

        if (is_empty) {
            kick_dma_transfer_read();
        }
    }
    spin_unlock_irqrestore(&port->port_lock, flags);
    return;
}
#endif

static void dma_write_complete_err_process(struct pcie_cdev_port *port)
{
    int ret;
    PCDEV_ERR("dma write fail\n");
    port->pstats.tx_packets_fail++;
    ret = kick_dma_transfer(port);
    if (ret) {
        PCDEV_ERR("kick_dma_transfer fail\n");
    }
}

static int dma_write_complete_desc_check(struct pcie_cdev_port *port, struct pcdev_desc *desc)
{
    if (desc == NULL) {
        port->pstats.tx_desc_null++;
        return -1;
    }

    if (!desc->own) {
        port->pstats.tx_desc_err++;
        return -1;
    }
    return 0;
}

static void dma_write_complete_process(struct pcie_cdev_port *port, struct pcdev_desc *desc,
                                       unsigned int s_tx_rp)
{
    int ret;
    desc->own = 0;
    g_pcdev_ctx.get_curtime((u64 *)&port->timestamp[port->tx_rp_complete % PCDEV_TIMESTAMP_COUNT].tx_packets_cb);
    s_tx_rp++;
    writel(s_tx_rp, port->tx_rp);
    s_tx_rp = readl(port->tx_rp);

    port->pstats.tx_packets_finish++;

    writel(1, port->pcie_cdev_send_irq[TX_CMP]);
    (void)readl(port->pcie_cdev_send_irq[TX_CMP]);

    ret = g_pcdev_ctx.send_irq();
    if (!ret) {
        port->pstats.irq_send++;
    }
}

static int pcie_cdev_dma_write_complete(u32 direction, u32 status, void *dev_info)
{
    struct pcie_cdev_port *port = (struct pcie_cdev_port *)dev_info;
    struct pcie_cdev_dma_list *list = NULL;
    struct pcdev_desc *desc = NULL;
    unsigned int entry;
    unsigned long flags;
    unsigned long dma_flags;
    unsigned int s_tx_rp;
    int ret;
    unsigned int need_kick = 0;

    PCDEV_INFO(port->port_num, "in\n");
    g_pcdev_ctx.dma_ctx.dma_trans_cmp++;
    spin_lock_irqsave(&port->port_lock, flags);

    if (status) {
        dma_write_complete_err_process(port);
        spin_unlock_irqrestore(&port->port_lock, flags);
        return status;
    }

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        goto out;
    }

    port->pstats.tx_dma_send_complete++;
    if (g_pcdev_ctx.work_mode == PCIE_EP) {
        spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
        list = list_last_entry(&g_pcdev_ctx.dma_ctx.head, struct pcie_cdev_dma_list, list);
        list_del(&list->list);

        if (!list_empty(&g_pcdev_ctx.dma_ctx.head)) {
            need_kick = 1;
        }
        spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);

        if (need_kick) {
            port->pstats.kick_dma_2++;
            kick_dma_transfer(port);
        }
    }

    s_tx_rp = readl(port->tx_rp);

    entry = s_tx_rp % port->tx_num;
    desc = port->tx_desc + entry;
    ret = dma_write_complete_desc_check(port, desc);
    if (ret) {
        goto out;
    }

    dma_write_complete_process(port, desc, s_tx_rp);

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(WRITE_BASE_MODE, port->port_num);
    return status;
}

ssize_t pcie_cdev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct inode *inode = file->f_path.dentry->d_inode;
    struct pcie_cdev_port *port = NULL;
    pcdev_wr_async_info_s rw_info;
    unsigned long flags;
    unsigned int size;
    int status;

    if (unlikely(buf == NULL || count == 0)) {
        PCDEV_ERR("invalid param\n");
        return 0;
    }

    port = pcdev_get_port(inode);
    if (IS_ERR_OR_NULL(port)) {
        PCDEV_ERR("get port fail\n");
        return 0;
    }

    status = wait_event_interruptible(port->read_wait, port->read_blocked != 0);
    if (status) {
        port->pstats.sync_read_wait_fail++;
        return 0;
    }

    spin_lock_irqsave(&port->port_lock, flags);
    port->read_blocked--;
    spin_unlock_irqrestore(&port->port_lock, flags);

    status = pcie_cdev_get_read_buf(port, &rw_info);
    if (status) {
        PCDEV_ERR("pcie_cdev_get_read_buf err!\n");
        return 0;
    }

    size = rw_info.size;
    if (count < size) {
        PCDEV_ERR("buff is not enough!\n");
        size = 0;
        goto OUT;
    }

    status = copy_to_user(buf, rw_info.p_vaddr, size);
    if (status != 0) {
        PCDEV_ERR("copy to user fail:%d!\n", status);
        size = 0;
        goto OUT;
    }

OUT:
    status = pcie_cdev_ret_read_buf(port, &rw_info);
    if (status) {
        PCDEV_ERR("pcie_cdev_ret_read_buf err!\n");
    }

    return size;
}

#ifdef CONFIG_PCIE_CDEV_DMA_DOUBLE
static int pcie_send_data_dma(struct pcie_cdev_port *port, struct pcdev_desc *desc)
{
    unsigned int entry;
    unsigned long dma_flags;
    unsigned int is_empty = 0;
    int ret = 0;

    entry = *port->tx_rp_todo % port->tx_num;
    port->transfer_info[entry].dma_info.element.sar_low = desc->s_dma_low;
    port->transfer_info[entry].dma_info.element.sar_high = desc->s_dma_high;
    port->transfer_info[entry].dma_info.element.dar_low = desc->d_dma_low;
    port->transfer_info[entry].dma_info.element.dar_high = desc->d_dma_high;
    port->transfer_info[entry].dma_info.element.transfer_size = desc->data_size;

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return -EIO;
    }

    spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
    if (list_empty(&g_pcdev_ctx.dma_ctx.head)) {
        is_empty = 1;
    }
    list_add(&port->transfer_info[entry].list, &g_pcdev_ctx.dma_ctx.head);
    spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);

    if (is_empty) {
        port->pstats.kick_dma_1++;
        ret = kick_dma_transfer(port);
    }

    return ret;
}
#endif

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
static int pcie_send_data_dma(struct pcie_cdev_port *port, struct pcdev_desc *desc)
{
    unsigned int entry;
    unsigned long dma_flags;
    unsigned int is_empty = 0;
    int ret = 0;

    if (g_pcdev_ctx.work_mode == PCIE_EP) {
        entry = *port->tx_rp_todo % port->tx_num;
        port->transfer_info[entry].dma_info.element.sar_low = desc->s_dma_low;
        port->transfer_info[entry].dma_info.element.sar_high = desc->s_dma_high;
        port->transfer_info[entry].dma_info.element.dar_low = desc->d_dma_low;
        port->transfer_info[entry].dma_info.element.dar_high = desc->d_dma_high;
        port->transfer_info[entry].dma_info.element.transfer_size = desc->data_size;

        if (pcie_cdev_linkdown()) {
            PCDEV_ERR("ep ports not allocked \n");
            return -EIO;
        }

        spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
        if (list_empty(&g_pcdev_ctx.dma_ctx.head)) {
            is_empty = 1;
        }
        list_add(&port->transfer_info[entry].list, &g_pcdev_ctx.dma_ctx.head);
        spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);

        if (is_empty) {
            port->pstats.kick_dma_1++;
            ret = kick_dma_transfer(port);
        }
    } else if (g_pcdev_ctx.work_mode == PCIE_RC) {
        writel(1, port->pcie_cdev_send_irq[DMA_SEND]);
        (void)readl(port->pcie_cdev_send_irq[DMA_SEND]);

        if (pcie_cdev_linkdown()) {
            PCDEV_ERR("ep ports not allocked \n");
            return -EIO;
        }
        return g_pcdev_ctx.send_irq();
    }
    return ret;
}
#endif

static int start_tx_check(struct pcie_cdev_port *port, struct pcdev_desc *desc,
                          pcdev_wr_async_info_s *rw_info)
{
    if (desc == NULL) {
        port->pstats.tx_desc_null++;
        return -ENOMEM;
    }

    if (!desc->own) {
        port->pstats.tx_desc_err++;
        return -EBUSY;
    }

    if (rw_info->size > *port->tx_buf_max_sz) {
        port->pstats.tx_buf_size_err++;
        return -EINVAL;
    }

    if (!port->remote_stat->bits.read_start) {
        PCDEV_ERR("remote realloc\n");
        return -EACCES;
    }
    return 0;
}

static void start_tx_desc_set(struct pcdev_desc *desc, pcdev_wr_async_info_s *rw_info)
{
    desc->s_vaddr = (unsigned long long)(unsigned long)rw_info->p_vaddr;
    desc->data_size = rw_info->size;
    desc->userfield0 = (unsigned long long)(unsigned long)rw_info->p_priv;
}

static void start_tx_dump_save(struct pcie_cdev_port *port, struct pcdev_desc *desc)
{
    if (g_pcdev_dump) {
        g_pcdev_dump->tx_desc_dump[port->port_num].vaddr = desc->s_vaddr;
        g_pcdev_dump->tx_desc_dump[port->port_num].dma_low = desc->s_dma_low;
        g_pcdev_dump->tx_desc_dump[port->port_num].dma_high = desc->s_dma_high;
    }
}

static void start_tx_desc_get(struct pcdev_desc *desc, dma_addr_t s_pbuf)
{
    desc->s_dma_low = (unsigned int)(s_pbuf & 0xFFFFFFFFU);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
    desc->s_dma_high = (unsigned int)((s_pbuf >> 32U) & 0xFFFFFFFFU);
#else
    desc->s_dma_high = 0;
#endif
}

int pcie_cdev_start_tx(struct pcie_cdev_port *port, pcdev_wr_async_info_s *rw_info)
{
    int status;
    unsigned int entry;
    struct pcdev_desc *desc = NULL;
    dma_addr_t s_pbuf;
    unsigned int s_tx_rp_todo;

    PCDEV_INFO(port->port_num, "in port:%d\n", port->port_num);

    if (*port->tx_rp_todo == *port->tx_wp) {
        port->pstats.tx_todo_full++;
        return -ENOMEM;
    }

    entry = *port->tx_rp_todo % port->tx_num;
    desc = port->tx_desc + entry;

    status = start_tx_check(port, desc, rw_info);
    if (status) {
        return status;
    }

    start_tx_desc_set(desc, rw_info);

    if (rw_info->p_paddr) {
        s_pbuf = (dma_addr_t)(uintptr_t)rw_info->p_paddr;
        port->dmamap_skip = 1;
    } else {
        PCDEV_INFO(port->port_num, "dma_map_single(%d) in\n", *port->tx_rp_todo);
        s_pbuf = dma_map_single(port->dev, (void *)rw_info->p_vaddr, rw_info->size, DMA_TO_DEVICE);
    }

    start_tx_desc_get(desc, s_pbuf);
    start_tx_dump_save(port, desc);

    s_tx_rp_todo = readl(port->tx_rp_todo);
    s_tx_rp_todo++;
    writel(s_tx_rp_todo, port->tx_rp_todo);
    s_tx_rp_todo = readl(port->tx_rp_todo);
    g_pcdev_ctx.get_curtime((u64 *)&port->timestamp[s_tx_rp_todo % PCDEV_TIMESTAMP_COUNT].tx_packets);
    port->pstats.tx_packets++;
    port->write_blocked = 1;
    status = pcie_send_data_dma(port, desc);
    if (!status) {
        if (!port->write_sync) {
            port->write_buf_list[entry].buf_info.p_vaddr = rw_info->p_vaddr;
            list_add(&port->write_buf_list[entry].list, &port->write_buf_head);
        }
    }
    port->pstats.tx_bytes += rw_info->size;
    return status;
}

static void modem_msc_write_cb_process(struct pcie_cdev_port *port)
{
    struct pcdev_msc_list *list = NULL;
    int ret;

    list = list_last_entry(&port->msc_stru_head, struct pcdev_msc_list, list);
    list_del(&list->list);

    memcpy_toio((void *)port->msc_stru, &list->msc_stru, sizeof(pcdev_mdm_msc_stru));
    *port->msc_busy = 1;
    kfree(list);

    writel(1, port->pcie_cdev_send_irq[MSC_STRU_SEND]);
    (void)readl(port->pcie_cdev_send_irq[MSC_STRU_SEND]);

    ret = g_pcdev_ctx.send_irq();
    if (!ret) {
        port->pstats.irq_send++;
    }
    port->pstats.msc_stru_send++;
}

void pcdev_modem_msc_write_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;

    PCDEV_ERR("in port(%d)\n", port->port_num);

    if (pcie_cdev_vote(MSC_WRITE_MODE, port->port_num)) {
        return;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        goto out;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        goto out1;
    }

    if (!port->remote_stat->bits.open) {
        port->pstats.remote_close++;
        goto out1;
    }

    if (*port->msc_busy == 1) {
        spin_unlock_irqrestore(&port->port_lock, flags);
        pcie_cdev_evt_push(port, &g_pcdev_msc_stru_send_evt_manage);
        queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
        goto out;
    }

    if (list_empty(&port->msc_stru_head)) {
        goto out1;
    }

    modem_msc_write_cb_process(port);

    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_evt_push(port, &g_pcdev_msc_stru_send_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    goto out;

out1:
    spin_unlock_irqrestore(&port->port_lock, flags);

out:
    pcie_cdev_unvote(MSC_WRITE_MODE, port->port_num);
    return;
}

static int pcdev_modem_msc_write(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    struct pcdev_msc_list *msc_list = NULL;
    int ret;
    unsigned long flags;
    PCDEV_ERR("in port(%d)\n", port->port_num);

    msc_list = kzalloc(sizeof(struct pcdev_msc_list), GFP_KERNEL);
    if (!msc_list) {
        PCDEV_ERR("kmalloc failed\n");
        return -1;
    }

    ret = memcpy_s(&msc_list->msc_stru, sizeof(msc_list->msc_stru), (void *)(uintptr_t)arg, sizeof(pcdev_mdm_msc_stru));
    if (ret) {
        PCDEV_ERR("memcpy_s failed, line: %d \n", __LINE__);
        return -1;
    }
    spin_lock_irqsave(&port->port_lock, flags);
    list_add(&msc_list->list, &port->msc_stru_head);
    spin_unlock_irqrestore(&port->port_lock, flags);

    pcie_cdev_evt_push(port, &g_pcdev_msc_stru_send_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    return 0;
}

static int write_base_check(struct pcie_cdev_port *port)
{
    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        PCDEV_ERR("write_base_check openclose %ld\n", port->pstats.openclose);
        return -EACCES;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return -EIO;
    }

    if (!port->remote_stat->bits.open) {
        PCDEV_ERR("write_base_check bits.open %d\n", port->remote_stat->bits.open);
        port->pstats.remote_close++;
        return -EACCES;
    }
    return 0;
}

int pcie_cdev_write_base(struct pcie_cdev_port *port, pcdev_wr_async_info_s *rw_info, unsigned int is_sync)
{
    int status = 0;
    unsigned long flags;

    PCDEV_TRACE("in\n");
    PCDEV_INFO(port->port_num, "port_name:%s, port_num:%d\n", port->name, port->port_num);

    if (rw_info->size == 0) {
        PCDEV_ERR("zero length packet to send\n");
        return status;
    }

    if ((rw_info->p_paddr == NULL) && !virt_addr_valid(rw_info->p_vaddr)) {
        PCDEV_ERR("buf is NULL\n");
        return -EINVAL;
    }

    if (pcie_cdev_vote(WRITE_BASE_MODE, port->port_num)) {
        return -ENOTBLK;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    status = write_base_check(port);
    if (status) {
        goto unlock;
    }

    port->write_sync = is_sync;
    port->pstats.write_base++;

    status = pcie_cdev_start_tx(port, rw_info);
    spin_unlock_irqrestore(&port->port_lock, flags);
    if (status) {
        pcie_cdev_unvote(WRITE_BASE_MODE, port->port_num);
        goto out;
    }

    if (!is_sync) {
        goto out;
    }

    status = wait_event_interruptible(port->write_wait, port->write_blocked == 0);
    if (status) {
        port->pstats.sync_tx_wait_fail++;
        goto out;
    }

    status = (int)rw_info->size;
    return status;

unlock:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(WRITE_BASE_MODE, port->port_num);
out:
    return status;
}

ssize_t pcie_cdev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct inode *inode = file->f_path.dentry->d_inode;
    int port_num;
    struct pcie_cdev_port *port = NULL;
    int status;
    pcdev_wr_async_info_s rw_info;
    char *write_buf = NULL;

    if (unlikely(NULL == buf || 0 == count)) {
        PCDEV_ERR("invalid param\n");
        return -EFAULT;
    }

    port_num = pcie_cdev_get_port_num(inode);
    if (port_num < 0) {
        return port_num;
    }

    status = fs_user_space_check(port_num);
    if (status) {
        return status;
    }

    port = g_pcie_cdev_ports[port_num].port;
    if (port == NULL) {
        return -ENODEV;
    }

    write_buf = kzalloc(count, GFP_KERNEL);
    if (write_buf == NULL) {
        PCDEV_ERR("no mem\n");
        return -ENODEV;
    }

    status = copy_from_user(write_buf, buf, count);
    if (status) {
        PCDEV_ERR("copy from user fail:%d\n", status);
        kfree(write_buf);
        return -EIO;
    }

    rw_info.p_vaddr = write_buf;
    rw_info.p_paddr = NULL;
    rw_info.size = (unsigned int)count;

    port->pstats.sync_tx_submit++;
    status = pcie_cdev_write_base(port, &rw_info, 1);
    if (status > 0) {
        port->pstats.sync_tx_done++;
    } else {
        port->pstats.sync_tx_fail++;
        PCDEV_ERR("write base fail sync_tx_fail status %d\n", status);
    }

    kfree(write_buf);
    return (ssize_t)status;
}

static void pcie_cdev_event_send(struct pcie_cdev_port *port)
{
    int ret;
    if (!port->remote_stat->bits.open) {
        port->pstats.remote_close++;
        PCDEV_ERR("remote close\n");
    }

    writel(1, port->pcie_cdev_send_irq[EVENT_SEND]);
    (void)readl(port->pcie_cdev_send_irq[EVENT_SEND]);

    ret = g_pcdev_ctx.send_irq();
    if (!ret) {
        port->pstats.irq_send++;
    }

    return;
}

static struct pcdev_desc *get_read_buf_status_check(struct pcie_cdev_port *port, unsigned int *entry)
{
    struct pcdev_desc *desc = NULL;
    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        return NULL;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return NULL;
    }

    if (!port->local_stat->bits.read_start) {
        port->pstats.rx_close++;
        return NULL;
    }

    if (port->rx_rp_toget == port->rx_rp_process) {
        port->pstats.rx_todo_empty++;
        return NULL;
    }

    *entry = port->rx_rp_toget % port->rx_num;
    desc = port->rx_desc + *entry;
    if (desc == NULL) {
        port->pstats.rx_desc_null++;
        return NULL;
    }

    if (desc->own) {
        port->pstats.rx_desc_err++;
        return NULL;
    }

    if (IS_ERR_OR_NULL((void *)(unsigned long)desc->d_vaddr)) {
        PCDEV_ERR("rx get bur err\n");
        port->pstats.rx_get_bur_err++;
        return NULL;
    }
    return desc;
}

int pcie_cdev_get_read_buf(struct pcie_cdev_port *port, pcdev_wr_async_info_s *wr_info)
{
    int status = 0;
    unsigned long flags;
    unsigned int entry;
    struct pcdev_desc *desc = NULL;
    dma_addr_t d_pbuf = 0;

    PCDEV_TRACE("in\n");

    if (pcie_cdev_vote(GET_READBUF_MODE, port->port_num)) {
        return -ENOTBLK;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    desc = get_read_buf_status_check(port, &entry);
    if (IS_ERR_OR_NULL(desc)) {
        status = -EACCES;
        goto out;
    }

    d_pbuf = desc->d_dma_low;
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
    d_pbuf = ((unsigned long)desc->d_dma_high << 32) + d_pbuf; /* get higher 32 bits of d_dma_high */
#endif

    wr_info->p_vaddr = (char *)(unsigned long)desc->d_vaddr;
    wr_info->p_paddr = (char *)d_pbuf;
    wr_info->size = desc->data_size;
    wr_info->p_priv = (void *)(uintptr_t)desc->userfield0;
    PCDEV_INFO(port->port_num, "buf(%d)\n", port->rx_rp_toget);

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        status = -EIO;
        goto out;
    }

    if (!port->set_readbuf) {
        dma_unmap_single(port->dev, d_pbuf, wr_info->size, DMA_FROM_DEVICE);
    }
    desc->d_vaddr = 0;
    desc->d_dma_low = 0;
    desc->d_dma_high = 0;
    port->d_vaddr[entry] = 0;

    port->pstats.rx_bytes += desc->data_size;
    port->pstats.rx_packets_finish++;
    port->rx_rp_toget++;

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(GET_READBUF_MODE, port->port_num);
    return status;
}

int pcie_cdev_ret_read_buf(struct pcie_cdev_port *port, pcdev_wr_async_info_s *wr_info)
{
    struct pcdev_buf_list *list = NULL;
    unsigned long flags;

    PCDEV_TRACE("in port(%d)\n", port->port_num);

    if ((wr_info->p_paddr == NULL) && !virt_addr_valid(wr_info->p_vaddr)) {
        PCDEV_ERR("buf err\n");
        return -EINVAL;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -EBUSY;
    }

    if (list_empty(&port->read_buf_head)) {
        PCDEV_ERR("ret buf not mach with get buf\n");
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -EPERM;
    }

    list = list_last_entry(&port->read_buf_head, struct pcdev_buf_list, list);
    list->buf_info.p_vaddr = wr_info->p_vaddr;
    list->buf_info.size = wr_info->size;
    list->buf_info.p_paddr = wr_info->p_paddr;
    list_move(&list->list, &port->ret_buf_head);

    spin_unlock_irqrestore(&port->port_lock, flags);

    pcie_cdev_evt_push(port, &g_pcdev_read_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);

    return 0;
}

#ifdef CONFIG_PCIE_CDEV
static int pcie_cdev_realloc_read_buf(struct pcie_cdev_port *port, pcdev_read_buf_info_s *buf_info)
{
    int status = 0;
    unsigned long flags;

    if (pcie_cdev_vote(REALLOC_MODE, port->port_num)) {
        return -ENOTBLK;
    }

    PCDEV_ERR("realloc buf: port(%d):size 0x%x\n", port->port_num, buf_info->buf_size);
    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        status = -EBUSY;
        goto out;
    }

    if (buf_info->buf_size == port->rx_buf_max_size) {
        goto out;
    }

    port->local_stat->bits.read_start = 0;
    *port->rx_buf_max_sz = buf_info->buf_size;
    port->rx_buf_max_size = buf_info->buf_size;

    spin_unlock_irqrestore(&port->port_lock, flags);

    (void)wait_event_timeout(port->buf_free_wait,
                             (*port->rx_wp - port->rx_rp_toget == port->rx_num),
                             PCIE_REALLOC_WAIT_COUNT);

    spin_lock_irqsave(&port->port_lock, flags);
    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        status = -EBUSY;
        goto out;
    }
    pcie_cdev_free_read_buf(port);
    status = pcie_cdev_alloc_read_buf(port);
    port->local_stat->bits.read_start = 1;

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(REALLOC_MODE, port->port_num);
    return status;
}
#endif

static int set_read_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    unsigned long read_cb_null;
    unsigned long flags;

    spin_lock_irqsave(&port->port_lock, flags);
    port->read_done_cb = (pcdev_read_done_cb_t)(uintptr_t)arg;
    read_cb_null = port->pstats.read_cb_null;
    port->pstats.read_cb_null = 0;
    spin_unlock_irqrestore(&port->port_lock, flags);
    if (read_cb_null) {
        pcie_cdev_evt_push(port, &g_pcdev_read_evt_manage);
        queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);
    }
    return 0;
}

static int set_write_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    port->write_done_cb = (pcdev_write_done_cb_t)(uintptr_t)arg;
    return 0;
}

static int set_write_info_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    port->write_info_done_cb = (pcdev_write_info_done_cb_t)(uintptr_t)arg;
    return 0;
}

static int set_evt_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    port->event_cb = (pcdev_event_cb_t)(uintptr_t)arg;
    return 0;
}

void pcdev_send_evt_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;

    PCDEV_TRACE("in port(%d)\n", port->port_num);

    if (pcie_cdev_vote(SEND_EVT_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        goto out;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        goto out;
    }

    memcpy_toio((void *)port->event_type, &port->event_type_arg, sizeof(pcdev_evt_e));

    pcie_cdev_event_send(port);
    port->pstats.event_send++;

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(SEND_EVT_MODE, port->port_num);
    return;
}

int pcdev_send_evt(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    PCDEV_TRACE("in port(%d)\n", port->port_num);

    port->event_type_arg = *((unsigned int *)((uintptr_t)arg));

    pcie_cdev_evt_push(port, &g_pcdev_evt_send_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);

    return 0;
}

void pcdev_send_read_sig_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    int ret;
    PCDEV_ERR("in port(%d)\n", port->port_num);

    if (pcie_cdev_vote(SEND_EVT_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        goto out;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        goto out;
    }

    memcpy_toio((void *)port->read_sig, &port->read_sig_arg, sizeof(pcdev_mdm_msc_stru));

    if (!port->remote_stat->bits.open) {
        port->pstats.remote_close++;
        PCDEV_ERR("remote close\n");
        goto out;
    }

    writel(1, port->pcie_cdev_send_irq[READ_SIG_SEND]);
    (void)readl(port->pcie_cdev_send_irq[READ_SIG_SEND]);

    ret = g_pcdev_ctx.send_irq();
    if (!ret) {
        port->pstats.irq_send++;
    }

    port->pstats.rel_ind_send++;

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(SEND_EVT_MODE, port->port_num);
    return;
}

int pcdev_send_read_sig(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    int ret;
    PCDEV_ERR("in port(%d)\n", port->port_num);

    ret = memcpy_s(&port->read_sig_arg, sizeof(port->read_sig_arg), (void *)(uintptr_t)arg, sizeof(pcdev_mdm_msc_stru));
    if (ret) {
        PCDEV_ERR("memcpy_s failed, line: %d \n", __LINE__);
        return -1;
    }

    pcie_cdev_evt_push(port, &g_pcdev_read_sig_send_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);

    return 0;
}

void pcdev_send_rel_ind_cb(struct pcie_cdev_port *port)
{
    unsigned long flags;
    int ret;
    PCDEV_ERR("in port(%d)\n", port->port_num);

    if (pcie_cdev_vote(SEND_EVT_MODE, port->port_num)) {
        return;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        goto out;
    }

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        goto out;
    }

    memcpy_toio((void *)port->rel_ind, &port->rel_ind_arg, sizeof(unsigned int));

    if (!port->remote_stat->bits.open) {
        port->pstats.remote_close++;
        PCDEV_ERR("remote close\n");
        goto out;
    }

    writel(1, port->pcie_cdev_send_irq[REL_IND_SEND]);
    (void)readl(port->pcie_cdev_send_irq[REL_IND_SEND]);

    ret = g_pcdev_ctx.send_irq();
    if (!ret) {
        port->pstats.irq_send++;
    }

    port->pstats.read_sig_send++;
out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(SEND_EVT_MODE, port->port_num);
    return;
}

int pcdev_send_rel_ind(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    PCDEV_ERR("in port(%d)\n", port->port_num);

    port->rel_ind_arg = *((unsigned int *)((uintptr_t)arg));

    pcie_cdev_evt_push(port, &g_pcdev_rel_ind_send_evt_manage);
    queue_delayed_work(g_cdev_driver->pcdev_work_queue, &g_pcdev_access_work, 0);

    return 0;
}

static int write_async_ioctl(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    pcdev_wr_async_info_s *rw_info = NULL;
    int status;

    rw_info = (pcdev_wr_async_info_s *)((uintptr_t)arg);
    port->pstats.write_async_call++;
    PCDEV_TRACE("PCDEV_IOCTL_WRITE_ASYNC\n");
    status = pcie_cdev_write_base(port, rw_info, 0);
    return status;
}

int pcdev_get_read_buf(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    int status;
    if (arg == 0) {
        PCDEV_ERR("PCDEV_IOCTL_GET_RD_BUFF invalid param\n");
        return -EFAULT;
    }
    port->pstats.get_buf_call++;
    status = pcie_cdev_get_read_buf(port, (pcdev_wr_async_info_s *)((uintptr_t)arg));
    return status;
}

int pcdev_ret_read_buf(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    int status;

    port->pstats.ret_buf_call++;
    status = pcie_cdev_ret_read_buf(port, (pcdev_wr_async_info_s *)((uintptr_t)arg));
    return status;
}

int realloc_read_buf(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_PCIE_CDEV
    int status;

    status = pcie_cdev_realloc_read_buf(port, (pcdev_read_buf_info_s *)((uintptr_t)arg));
    return status;
#else
    return 0;
#endif
}

static int set_msc_read_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    port->read_sig_cb = (pcdev_modem_msc_read_cb_t)(uintptr_t)arg;
    return 0;
}

static int set_rel_ind_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    port->rel_ind_cb = (pcdev_modem_rel_ind_cb_t)(uintptr_t)arg;
    return 0;
}

static int set_msc_write_cb(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    port->msc_stru_cb = (pcdev_modem_msc_write_cb_t)(uintptr_t)arg;
    return 0;
}

static void read_buf_process(struct pcie_cdev_port *port, pcdev_read_buf_set_info_s *buf_info)
{
    int i;
    dma_addr_t d_pbuf;
    struct pcdev_desc *desc = NULL;
    port->local_stat->bits.read_start = 0;
    if (!port->set_readbuf) {
        for (i = 0; i < (int)port->rx_num; i++) {
            desc = port->rx_desc + i;
            if (port->d_vaddr[i]) {
                kfree((void *)(uintptr_t)port->d_vaddr[i]);
                port->d_vaddr[i] = 0;
            }
        }
        port->rd_buf_alloced = 0;
    }

    *port->rx_buf_max_sz = buf_info->buf_size;
    port->rx_buf_max_size = buf_info->buf_size;

    for (i = 0; i < MAX_DESC_NUM; i++) {
        desc = port->rx_desc + i;
        d_pbuf = buf_info->phy_buf_base[i];
        desc->d_dma_low = (unsigned int)(d_pbuf & 0xFFFFFFFFU);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
        desc->d_dma_high = (unsigned int)((d_pbuf >> 32U) & 0xFFFFFFFFU);
#else
        desc->d_dma_high = 0;
#endif
        desc->d_vaddr = buf_info->vir_buf_base[i];
        desc->own = 1;
    }

    port->set_readbuf = 1;
    port->local_stat->bits.read_start = 1;
}

int set_read_buf(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
    pcdev_read_buf_set_info_s *buf_info = NULL;
    int status = 0;
    unsigned long flags;

    buf_info = (pcdev_read_buf_set_info_s *)(uintptr_t)arg;
    if (buf_info->buf_num != port->rx_num) {
        PCDEV_ERR("PCDEV_IOCTL_SET_READ_BUFF buf num %d err\n", buf_info->buf_num);
        return -EFAULT;
    }

    if (pcie_cdev_vote(REALLOC_MODE, port->port_num)) {
        return -ENOTBLK;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == false) {
        port->pstats.local_close++;
        status = -EBUSY;
        goto out;
    }

    read_buf_process(port, buf_info);

out:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(REALLOC_MODE, port->port_num);
    return status;
}

static struct pcie_cdev_port *pcdev_get_port(struct inode *inode)
{
    struct pcie_cdev_port *port = NULL;
    int port_num;
    int status;

    port_num = pcie_cdev_get_port_num(inode);
    if (port_num < 0) {
        return (struct pcie_cdev_port *)(-ENODEV);
    }

    status = fs_user_space_check(port_num);
    if (status) {
        return (struct pcie_cdev_port *)(-ENOTSUPP);
    }

    port = g_pcie_cdev_ports[port_num].port;
    if (port == NULL) {
        return (struct pcie_cdev_port *)(-ENODEV);
    }

    return port;
}

static int pcdev_set_diag_mode(struct pcie_cdev_port *port, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_PCIE_CDEV
    pcdev_set_diag_mode_ep(arg);
#endif
    return 0;
}

struct pcdev_ioctl_handler g_pcdev_ioctl_cb_table[PCIE_CDEV_MAX_IOCTL_CB] = {
    { PCDEV_IOCTL_SET_READ_CB, set_read_cb },
    { PCDEV_IOCTL_SET_WRITE_CB, set_write_cb },
    { PCDEV_IOCTL_SET_WRITE_INFO_CB, set_write_info_cb },
    { PCDEV_IOCTL_SET_EVT_CB, set_evt_cb },
    { PCDEV_IOCTL_SET_FREE_CB, NULL },
    { PCDEV_IOCTL_SEND_EVT, pcdev_send_evt },
    { PCDEV_IOCTL_SEND_MSC_READ, pcdev_send_read_sig },
    { PCDEV_IOCTL_SEND_REL_IND, pcdev_send_rel_ind },
    { PCDEV_IOCTL_WRITE_ASYNC, write_async_ioctl },
    { PCDEV_IOCTL_GET_RD_BUFF, pcdev_get_read_buf },
    { PCDEV_IOCTL_RETURN_BUFF, pcdev_ret_read_buf },
    { PCDEV_IOCTL_RELLOC_READ_BUFF, realloc_read_buf },
    { PCDEV_IOCTL_SET_READ_BUFF, set_read_buf },
    { PCDEV_IOCTL_WRITE_DO_COPY, NULL },
    { PCDEV_MODEM_IOCTL_SET_MSC_READ_CB, set_msc_read_cb },
    { PCDEV_MODEM_IOCTL_MSC_WRITE_CMD, pcdev_modem_msc_write },
    { PCDEV_MODEM_IOCTL_SET_REL_IND_CB, set_rel_ind_cb },
    { PCDEV_MODEM_IOCTL_SET_MSC_WRITE_CB, set_msc_write_cb },
    { PCDEV_MODEM_IOCTL_SET_DIAG_STATUS, pcdev_set_diag_mode },
};

long pcie_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct inode *inode = NULL;
    struct pcdev_ioctl_handler *ioctl_cb_table = &g_pcdev_ioctl_cb_table[0];
    struct pcie_cdev_port *port = NULL;
    int status = 0;
    int index = -1;
    int i;
    PCDEV_TRACE("in cmd:0x%x\n", cmd);

    if (file == NULL) {
        PCDEV_ERR("invalid paras cmd 0x%X, file is NULL\n", cmd);
        return -1;
    }

    inode = file->f_path.dentry->d_inode;
    port = pcdev_get_port(inode);
    if (IS_ERR_OR_NULL(port)) {
        PCDEV_ERR("get port fail\n");
        return (uintptr_t)port;
    }

    for (i = 0; i < PCIE_CDEV_MAX_IOCTL_CB; i++) {
        if (ioctl_cb_table[i].cmd == cmd) {
            index = i;
            break;
        }
    }

    if (index < 0 || index >= PCIE_CDEV_MAX_IOCTL_CB) {
        PCDEV_ERR("cmd 0x%X not found\n", cmd);
        return -1;
    }

    if (ioctl_cb_table[index].pcdev_ioctl_cb != NULL) {
        if (arg == 0) {
            PCDEV_ERR("invalid paras cmd 0x%X arg 0\n", cmd);
            return -1;
        }

        status = ioctl_cb_table[index].pcdev_ioctl_cb(port, cmd, arg);
        if (status) {
            PCDEV_ERR("index %d cmd 0x%X status %d error\n", index, cmd, status);
        }
    }

    return status;
}

static void ep_transfer_info_init(struct pcie_cdev_port *port)
{
    int i;

    port->transfer_info = kzalloc(sizeof(struct pcie_cdev_dma_list) * port->tx_num, GFP_ATOMIC);

    if (port->transfer_info == NULL) {
        PCDEV_ERR("transfer_info alloc fail\n");
        pcie_cdev_free_read_buf(port);
        return ;
    }

    for (i = 0; i < port->tx_num; i++) {
        port->transfer_info[i].dma_info.id = g_pcdev_ctx.pcie_id;
        port->transfer_info[i].dma_info.callback = pcie_cdev_dma_write_complete;
        port->transfer_info[i].dma_info.callback_args = (void *)port;
        port->transfer_info[i].dma_info.channel = g_pcdev_ctx.dma_ctx.dma_channel;
        port->transfer_info[i].dma_info.direction = 1;
        port->transfer_info[i].dma_info.transfer_type = PCIE_DMA_NORMAL_MODE;
        port->transfer_info[i].dma_info.element_cnt = 1;
    }
}

static int port_transfer_init(struct pcie_cdev_port *port)
{
#ifdef CONFIG_PCIE_CDEV_DMA_DOUBLE
    ep_transfer_info_init(port);
#endif

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    int i;
    if (g_pcdev_ctx.work_mode == PCIE_EP) {
        ep_transfer_info_init(port);

        port->transfer_info_rc = kzalloc(sizeof(struct pcie_cdev_dma_list) * port->rx_num, GFP_ATOMIC);
        if (port->transfer_info_rc == NULL) {
            return -1;
        }

        for (i = 0; i < port->rx_num; i++) {
            port->transfer_info_rc[i].dma_info.id = g_pcdev_ctx.pcie_id;
            port->transfer_info_rc[i].dma_info.callback = pcie_cdev_dma_read_complete;
            port->transfer_info_rc[i].dma_info.callback_args = (void *)port;
            port->transfer_info_rc[i].dma_info.channel = g_pcdev_ctx.dma_ctx.dma_channel;
            port->transfer_info_rc[i].dma_info.direction = 0;
            port->transfer_info[i].dma_info.transfer_type = PCIE_DMA_NORMAL_MODE;
            port->transfer_info[i].dma_info.element_cnt = 1;
        }
    } else {
        port->dma_callback = pcie_cdev_dma_write_complete;
    }
#endif
    return 0;
}

static struct pcie_cdev_port *pcie_cdev_open_check(int *port_num, struct inode *inode)
{
    int status;
    struct pcie_cdev_port *port = NULL;

    *port_num = pcie_cdev_get_port_num(inode);
    if (*port_num < 0) {
        return NULL;
    }

    port = g_pcie_cdev_ports[*port_num].port;
    if (port == NULL) {
        return NULL;
    }

    status = fs_user_space_check(*port_num);
    if (status) {
        return NULL;
    }

    if (pcie_cdev_vote(PORT_OPEN_MODE, port->port_num)) {
        return NULL;
    }
    return port;
}

static int pcie_cdev_open_process(struct pcie_cdev_port *port, int port_num)
{
    int status;

    if (port->rd_buf_alloced) {
        *port->rx_buf_max_sz = port->rx_buf_max_size;
    }
    port->rx_num = g_pcie_cdev_type_table[port_num].buf_num;
    port->tx_num = g_pcie_cdev_type_table[port_num].buf_num;

    status = pcie_cdev_alloc_read_buf(port);
    if (status) {
        PCDEV_ERR("alloc read buf fail\n");
        return status;
    }

    if (port_transfer_init(port)) {
        return status;
    }

    port->write_buf_list = kzalloc(sizeof(struct pcdev_buf_list) * port->tx_num, GFP_ATOMIC);
    if (port->write_buf_list == NULL) {
        PCDEV_ERR("write_buf_list alloc fail\n");
        return -1;
    }

    port->ret_buf_list = kzalloc(sizeof(struct pcdev_buf_list) * port->rx_num, GFP_ATOMIC);
    if (port->ret_buf_list == NULL) {
        PCDEV_ERR("ret_buf_list alloc fail\n");
        return -1;
    }
    return 0;
}

static void pcie_cdev_open_err(struct pcie_cdev_port *port)
{
    if (port->ret_buf_list) {
        kfree(port->ret_buf_list);
        port->ret_buf_list = NULL;
    }

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    if (port->transfer_info_rc) {
        kfree(port->transfer_info_rc);
        port->transfer_info_rc = NULL;
    }
#endif

    if (port->transfer_info) {
        kfree(port->transfer_info);
        port->transfer_info = NULL;
    }

    pcie_cdev_free_read_buf(port);

    port->pstats.openclose = false;
}

static int pcie_cdev_open(struct inode *inode, struct file *filp)
{
    int port_num;
    struct pcie_cdev_port *port = NULL;
    int status;
    unsigned long flags;
    unsigned int i;

    PCDEV_TRACE("in\n");
    port = pcie_cdev_open_check(&port_num, inode);
    if (port == NULL) {
        PCDEV_ERR("get port fail\n");
        return -ENODEV;
    }

    spin_lock_irqsave(&port->port_lock, flags);

    if (port->pstats.openclose == true) {
        PCDEV_ERR("openclose state err\n");
        status = -EBUSY;
        goto out1;
    } else {
        if (g_pcdev_ctx.init_count <= g_pcdev_ctx.exit_count) {
            PCDEV_ERR("pcdev not ready, can't open\n");
            status = -ENOENT;
            goto out1;
        }

        status = 0;
        port->pstats.openclose = true;
        port->pstats.open_count++;
    }

    status = pcie_cdev_open_process(port, port_num);
    if (status) {
        PCDEV_ERR("pcie_cdev_open_process error\n");
        goto outerr;
    }

    for (i = 0; i < port->rx_num; i++) {
        list_add(&port->ret_buf_list[i].list, &port->read_buf_head);
    }

    port->local_stat->bits.read_start = 1;
    port->local_stat->bits.open = 1;
    PCDEV_TRACE("port(%d) alloc success\n", port_num);
    goto out1;

outerr:
    pcie_cdev_open_err(port);
out1:
    spin_unlock_irqrestore(&port->port_lock, flags);
    pcie_cdev_unvote(PORT_OPEN_MODE, port->port_num);
    return status;
}

static void port_list_clean(struct pcie_cdev_port *port)
{
    struct list_head *head = NULL;
    struct pcdev_buf_list *list = NULL;
    struct pcdev_msc_list *msc_list = NULL;
    pcdev_write_info_s write_info;

    if (port->transfer_info != NULL) {
        kfree((void *)port->transfer_info);
        port->transfer_info = NULL;
    }
#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    if (port->transfer_info_rc != NULL) {
        kfree((void *)port->transfer_info_rc);
        port->transfer_info_rc = NULL;
    }
#endif

    head = &port->msc_stru_head;
    while (!list_empty(head)) {
        msc_list = list_entry(head->next, struct pcdev_msc_list, list);
        list_del(&msc_list->list);
        kfree(msc_list);
    }

    head = &port->write_buf_head;
    while (!list_empty(head)) {
        list = list_entry(head->next, struct pcdev_buf_list, list);
        if (port->write_done_cb) {
            port->write_done_cb(list->buf_info.p_vaddr, list->buf_info.p_paddr, -1);
        } else if (port->write_info_done_cb) {
            write_info.p_vaddr = list->buf_info.p_vaddr;
            write_info.p_paddr = list->buf_info.p_paddr;
            write_info.size = -1;
            port->write_info_done_cb(&write_info);
        }
        list_del(&list->list);
    }
}

static void pcie_cdev_port_exit(struct pcie_cdev_port *port)
{
    unsigned long flags;

    PCDEV_ERR("in port(%d)\n", port->port_num);

    spin_lock_irqsave(&port->port_lock, flags);
    if (port->pstats.openclose == false) {
        port->pstats.close_count_repeat++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return;
    }
    port_list_clean(port);

    INIT_LIST_HEAD(&port->ret_buf_head);
    INIT_LIST_HEAD(&port->read_buf_head);

    if (port->ret_buf_list != NULL) {
        kfree((void *)port->ret_buf_list);
        port->ret_buf_list = NULL;
    }
    if (port->write_buf_list != NULL) {
        kfree((void *)port->write_buf_list);
        port->write_buf_list = NULL;
    }

    port->pstats.openclose = false;
    port->pstats.close_count++;

    spin_unlock_irqrestore(&port->port_lock, flags);
    return;
}

static void pcie_cdev_port_close(struct pcie_cdev_port *port)
{
    unsigned long flags;

    PCDEV_ERR("in port(%d)\n", port->port_num);

    spin_lock_irqsave(&port->port_lock, flags);
    if (port->pstats.openclose == false) {
        port->pstats.close_count_repeat++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return;
    }

    port_list_clean(port);

    INIT_LIST_HEAD(&port->ret_buf_head);
    INIT_LIST_HEAD(&port->read_buf_head);

    if (port->ret_buf_list != NULL) {
        kfree((void *)port->ret_buf_list);
        port->ret_buf_list = NULL;
    }

    if (port->write_buf_list != NULL) {
        kfree((void *)port->write_buf_list);
        port->write_buf_list = NULL;
    }

    port->pstats.openclose = false;
    port->pstats.close_count++;

    spin_unlock_irqrestore(&port->port_lock, flags);
    return;
}

static int pcie_cdev_close(struct inode *inode, struct file *filp)
{
    int port_num;
    struct pcie_cdev_port *port = NULL;

    port_num = pcie_cdev_get_port_num(inode);
    PCDEV_ERR("in port(%d)\n", port_num);
    if (port_num < 0) {
        return port_num;
    }

    port = g_pcie_cdev_ports[port_num].port;
    if (port == NULL) {
        return -ENODEV;
    }

    pcie_cdev_port_close(port);
    return 0;
}

static const struct file_operations g_pcdev_fops = {
    .read = pcie_cdev_read,
    .write = pcie_cdev_write,
    .unlocked_ioctl = pcie_cdev_ioctl,
    .open = pcie_cdev_open,
    .release = pcie_cdev_close,
};

static int pcie_cdev_alloc_read_buf(struct pcie_cdev_port *port)
{
    int i;
    dma_addr_t d_pbuf;
    struct pcdev_desc *desc = NULL;

    for (i = 0; i < (int)port->rx_num; i++) {
        desc = port->rx_desc + i;
        if (!port->d_vaddr[i]) {
            port->d_vaddr[i] = (unsigned long long)(unsigned long)kzalloc(port->rx_buf_max_size, GFP_ATOMIC);

            if (!port->d_vaddr[i]) {
                PCDEV_ERR("alloc fail :%d!\n", i);
                return -ENOMEM;
            }
        }
        desc->d_vaddr = port->d_vaddr[i];
        if (port->d_vaddr[i] != desc->d_vaddr) {
            PCDEV_ERR("port->d_vaddr:%lld, desc->d_vaddr :%lld\n", port->d_vaddr[i], desc->d_vaddr);
        }

        d_pbuf = dma_map_single(port->dev, (void *)(uintptr_t)port->d_vaddr[i], port->rx_buf_max_size,
                                DMA_FROM_DEVICE);
        desc->d_dma_low = (unsigned int)(d_pbuf & 0xFFFFFFFFU);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
        desc->d_dma_high = (unsigned int)((d_pbuf >> 32U) & 0xFFFFFFFFU);
#else
        desc->d_dma_high = 0;
#endif
        desc->own = 1;
    }

    writel(port->rx_num, port->rx_wp);
    port->pstats.rx_packets += port->rx_num;
    port->rd_buf_alloced = 1;

    return 0;
}

static int pcie_cdev_free_read_buf(struct pcie_cdev_port *port)
{
    unsigned int i;

    for (i = 0; i < port->rx_num; i++) {
        if (port->d_vaddr[i]) {
            kfree((void *)(uintptr_t)port->d_vaddr[i]);
        }
        port->d_vaddr[i] = 0;
        port->rx_desc[i].own = 0;
        port->rx_desc[i].d_vaddr = 0;
        port->rx_desc[i].d_dma_low = 0;
        port->rx_desc[i].d_dma_high = 0;
    }

    *port->rx_wp = 0;
    *port->rx_rp = 0;
    port->rx_rp_toget = 0;
    port->rx_rp_process = 0;
    port->tx_rp_complete = 0;
    port->pstats.rx_packets -= port->rx_num;
    port->rd_buf_alloced = 0;

    return 0;
}

static int pcdev_port_init(struct pcie_cdev_port *port)
{
    port->rx_num = NUM_R2E_DESC;
    port->tx_num = NUM_E2R_DESC;
    port->dmamap_skip = 0;
    return 0;
}

static void port_pr_clear(struct pcie_cdev_port *port)
{
    port->local_stat = 0;
    port->remote_stat = 0;
    port->rx_rp_toget = 0;
    port->rx_rp_process = 0;

    port->event_type = 0;

    port->rx_wp = 0;
    port->rx_rp = 0;
    port->rx_rp_toget = 0;
    port->rx_rp_process = 0;
    port->rx_buf_max_sz = 0;

    port->rx_desc = 0;

    port->tx_wp = 0;
    port->tx_rp = 0;
    port->tx_rp_todo = 0;
    port->tx_rp_complete = 0;
    port->tx_buf_max_sz = 0;
    port->tx_desc = 0;
    port->write_sync = 0;
    port->write_blocked = 0;

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    port->tx_rp_todo_rc = 0;
    port->tx_rp_dma_cmp_rc = 0;
    port->tx_rp_dma_cfg_rc = 0;
    port->tx_rp_cmp = 0;
#endif
    return;
}

void pcie_cdev_port_alloc(unsigned int port_num)
{
    struct pcie_cdev_port *port = NULL;
    struct pcdev_ports_desc *ports_desc = NULL;

    PCDEV_TRACE("in\n");

    g_pcdev_ctx.ports_desc = (struct pcdev_ports_desc *)g_pcdev_ctx.virt_addr;
    ports_desc = g_pcdev_ctx.ports_desc;
#ifdef CONFIG_PCIE_CDEV_RC
    ports_desc = (struct pcdev_ports_desc *)g_pcdev_ctx.virt_addr;

    if (pcie_cdev_linkdown()) {
        PCDEV_ERR("ep ports not allocked \n");
        return;
    }

    if (pcdev_rc_port_desc_match(port_num)) {
        PCDEV_ERR("port(%d) desc match fail \n", port_num);
        return;
    }
#endif

    if (g_pcie_cdev_ports[port_num].is_alloc == 1) {
        port = g_pcie_cdev_ports[port_num].port;
        port_pr_clear(port);
    } else {
        port = (struct pcie_cdev_port *)kzalloc(sizeof(struct pcie_cdev_port), GFP_KERNEL);
        if (port == NULL) {
            PCDEV_ERR("port(%d) kzalloc fail \n", port_num);
            return;
        }

        spin_lock_init(&port->port_lock);

        init_waitqueue_head(&port->write_wait);
        init_waitqueue_head(&port->buf_free_wait);
        init_waitqueue_head(&port->read_wait);

        INIT_LIST_HEAD(&port->ret_buf_head);
        INIT_LIST_HEAD(&port->read_buf_head);
        INIT_LIST_HEAD(&port->write_buf_head);
        INIT_LIST_HEAD(&port->msc_stru_head);

        port->port_num = port_num;
        port->name = PCIE_CDEV_GET_NAME(port_num);
        port->dev = &g_pcdev_ctx.pdev->dev;

        g_pcie_cdev_ports[port_num].port = port;
        g_pcie_cdev_ports[port_num].is_alloc = 1;
    }

    pcdev_port_init(port);

    port_desc_map(port_num, g_pcdev_ctx.work_mode);
    port->local_stat->bits.init = 1;
    return;
}

static void pcie_cdev_ports_alloc(void)
{
    int i;
    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        pcie_cdev_port_alloc(i);
    }
}

static struct pcie_cdev_driver *pcie_cdev_alloc_driver(int lines)
{
    struct pcie_cdev_driver *driver;

    driver = kzalloc(sizeof(struct pcie_cdev_driver), GFP_KERNEL);
    if (driver != NULL) {
        kref_init(&driver->kref);
        driver->num = lines;
    }
    return driver;
}

static int pcie_cdev_register_driver(struct pcie_cdev_driver *driver)
{
    int error;
    dev_t dev;

    if (driver->pcdev_work_queue == NULL) {
        PCDEV_ERR("pcdev_work_queue NULL\n");
        return -1;
    }

    error = alloc_chrdev_region(&dev, driver->minor_start, driver->num, driver->name);
    if (error < 0) {
        return error;
    }

    driver->major = MAJOR(dev);
    driver->minor_start = MINOR(dev);

    cdev_init(&driver->cdev, &g_pcdev_fops);
    driver->cdev.owner = driver->owner;
    error = cdev_add(&driver->cdev, dev, driver->num);
    if (error) {
        unregister_chrdev_region(dev, driver->num);
        return error;
    }
    driver->dev_no = dev;

    return 0;
}

static void pcie_cdev_unregister_driver(struct pcie_cdev_driver *driver)
{
    cdev_del(&driver->cdev);
    unregister_chrdev_region(driver->dev_no, driver->num);
    return;
}

struct device *pcie_cdev_register_device(struct pcie_cdev_driver *driver, unsigned index, struct device *device)
{
    char name[PCIE_CDEV_NAME_MAX];
    dev_t dev = MKDEV(driver->major, driver->minor_start) + index;
    int ret;

    if ((int)index >= driver->num) {
        PCDEV_ERR("Attempt to register invalid tty line number (%d).\n", index);
        return ERR_PTR(-EINVAL);
    }
    ret = snprintf_s(name, PCIE_CDEV_NAME_MAX, PCIE_CDEV_NAME_MAX - 1, "%s", PCIE_CDEV_GET_NAME(index));
    if (ret < 0) {
        PCDEV_ERR("snprintf_s fail ret:%d\n", ret);
    }

    return device_create(g_pcdev_class, device, dev, NULL, name);
}

static void pcie_cdev_unregister_device(struct pcie_cdev_driver *driver, unsigned index)
{
    device_destroy(g_pcdev_class, MKDEV(driver->major, driver->minor_start) + index);
    return;
}

int pcie_cdev_register_devices(void)
{
    int i;
    struct device *cdev = NULL;

    g_pcdev_class = class_create(THIS_MODULE, "g_pcdev_class");
    if (IS_ERR(g_pcdev_class)) {
        return PTR_ERR(g_pcdev_class);
    }

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        /* register devices ... */
        if (!g_pcie_cdev_type_table[i].port_make_device) {
            continue;
        }
        cdev = pcie_cdev_register_device(g_cdev_driver, i, NULL);
        if (IS_ERR(cdev)) {
            PCDEV_ERR("no classdev for port(%d), err %ld\n", i, PTR_ERR(cdev));
            goto setup_fail;
        }

        dma_set_mask_and_coherent(cdev, g_pcdev_dma_mask);
        of_dma_configure(cdev, NULL, true);
        g_pcie_cdev_ports[i].cdev = cdev;
    }
    return 0;

setup_fail:
    while (i > 0) {
        i--;
        /* start sysfs and /dev/ttyGS* node removal */
        if (g_pcie_cdev_ports[i].cdev) {
            pcie_cdev_unregister_device(g_cdev_driver, i);
            g_pcie_cdev_ports[i].cdev = NULL;
        }
    }

    class_destroy(g_pcdev_class);
    return -ENXIO;
}

static void pcie_cdev_unregister_devices(void)
{
    int i;

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        if (g_pcie_cdev_ports[i].cdev) {
            pcie_cdev_unregister_device(g_cdev_driver, i);
            g_pcie_cdev_ports[i].cdev = NULL;
        }
    }

    if (g_pcdev_class) {
        class_destroy(g_pcdev_class);
        g_pcdev_class = NULL;
    }

    return;
}

static int pcdev_enmudone_check(void *data)
{
    int i;
    char pcdev_name[PCIE_CDEV_NAME_MAX];
    mm_segment_t old_fs;
    int ret;

    PCDEV_TRACE("in\n");
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        if (!g_pcie_cdev_ports[i].port) {
            PCDEV_ERR("port(%d) is NULL\n", i);
            set_fs(old_fs);
            return -1;
        }

        if (!g_pcie_cdev_type_table[i].port_make_device) {
            continue;
        }

        ret = snprintf_s(pcdev_name, PCIE_CDEV_NAME_MAX, PCIE_CDEV_NAME_MAX - 1, "/dev/%s",
                         g_pcie_cdev_ports[i].port->name);
        if (ret < 0) {
            PCDEV_ERR("snprintf_s fail ret:%d\n", ret);
        }

        while (bsp_eaccess(pcdev_name)) {
            msleep(100); /* sleep 100 miliseconds */
            PCDEV_ERR("port(%d) sleep\n", i);
        }
    }
    set_fs(old_fs);

    pcie_cdev_notify_enum_done();
    return 0;
}

static void pcdev_evt_init(void)
{
    pcie_cdev_evt_init(&g_pcdev_write_evt_manage, "write_evt");
    pcie_cdev_evt_init(&g_pcdev_read_evt_manage, "read_evt");
    pcie_cdev_evt_init(&g_pcdev_sig_stat_evt_manage, "sig_stat_evt");
    pcie_cdev_evt_init(&g_pcdev_read_sig_evt_manage, "read_sig_evt");
    pcie_cdev_evt_init(&g_pcdev_rel_ind_evt_manage, "rel_ind_evt");
    pcie_cdev_evt_init(&g_pcdev_msc_stru_evt_manage, "msc_stru_evt");
    pcie_cdev_evt_init(&g_pcdev_evt_send_evt_manage, "evt_send_evt");
    pcie_cdev_evt_init(&g_pcdev_read_sig_send_evt_manage, "read_sig_send_evt");
    pcie_cdev_evt_init(&g_pcdev_rel_ind_send_evt_manage, "rel_ind_send_evt");
    pcie_cdev_evt_init(&g_pcdev_msc_stru_send_evt_manage, "msc_stru_send_evt");
}

static int pcdev_driver_init(void)
{
    g_cdev_driver = pcie_cdev_alloc_driver(PCIE_CDEV_COUNT);
    if (g_cdev_driver == NULL) {
        PCDEV_ERR("pcie_cdev_alloc_driver fail\n");
        return -1;
    }

    g_cdev_driver->owner = THIS_MODULE;
    g_cdev_driver->driver_name = PCIE_CDEV_DRV_NAME;
    g_cdev_driver->name = PCIE_CDEV_PREFIX;
    g_cdev_driver->pcdev_work_queue = create_singlethread_workqueue("pcie_cdev");

    INIT_DELAYED_WORK(&g_pcdev_access_work, pcie_cdev_rw_push);
    g_pcdev_ctx.irq_handler = pcie_cdev_irq_handler;
    g_pcdev_ctx.irq_table = g_pcdev_intr_table;
    return 0;
}

static void pcdev_ctx_init(void)
{
#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    if (g_pcdev_ctx.work_mode == PCIE_EP) {
        spin_lock_init(&g_pcdev_ctx.dma_ctx.dma_lock);
        spin_lock_init(&g_pcdev_ctx.dma_ctx.dma_lock_rc);
        INIT_LIST_HEAD(&g_pcdev_ctx.dma_ctx.head);
        INIT_LIST_HEAD(&g_pcdev_ctx.dma_ctx.head_rc);
    }
#endif
#ifdef CONFIG_PCIE_CDEV_DMA_DOUBLE
    spin_lock_init(&g_pcdev_ctx.dma_ctx.dma_lock);
    INIT_LIST_HEAD(&g_pcdev_ctx.dma_ctx.head);
#endif

    g_pcdev_ctx.dma_ctx.dma_channel = PCIE_DMA_CHAR_DEV;
    if (g_pcdev_dump) {
        g_pcdev_dump->phys_addr = g_pcdev_ctx.phys_addr;
        g_pcdev_dump->virt_addr = g_pcdev_ctx.virt_addr;
        g_pcdev_dump->buffer_size = g_pcdev_ctx.buffer_size;
    }

    g_pcdev_ctx.diag_mode = 0;
    g_pcdev_ctx.init_count++;
    g_pcdev_enumdone_check = kthread_run(pcdev_enmudone_check, NULL, "PCDEV_ENMU_CHECK");
}

static int retiest_driver(void)
{
    int ret;
    if (g_cdev_driver->pcdev_work_queue == NULL) {
        PCDEV_ERR("pcdev_work_queue NULL\n");
        return -1;
    }

    ret = pcie_cdev_register_driver(g_cdev_driver);
    if (ret) {
        PCDEV_ERR("register driver fail\n");
        return -1;
    }
    return 0;
}

void pcdev_init(struct work_struct *work)
{
    bsp_err("[init]start\n");

    pcdev_evt_init();

    if (g_cdev_driver == NULL) {
        if (pcdev_driver_init()) {
            return;
        }

        if (retiest_driver()) {
            goto PRB_ERROR0;
        }

        if (pcie_cdev_register_devices()) {
            PCDEV_ERR("register devices fail\n");
            goto PRB_ERROR1;
        }
    }

    if (pcie_cdev_vote(INIT_MODE, PCIE_CDEV_COUNT)) {
        PCDEV_ERR("pcdev init vote failed\n");
        goto PRB_ERROR2;
    }

    if (g_pcdev_ctx.pcdev_hw_init()) {
        PCDEV_ERR("pcdev hw init failed\n");
        goto PRB_ERROR3;
    }

    pcie_cdev_ports_alloc();
    pcdev_ctx_init();

    pcie_cdev_unvote(INIT_MODE, PCIE_CDEV_COUNT);
    bsp_err("[init]ok\n");
    return;

PRB_ERROR3:
    pcie_cdev_unvote(INIT_MODE, PCIE_CDEV_COUNT);
PRB_ERROR2:
    pcie_cdev_unregister_devices();
PRB_ERROR1:
    pcie_cdev_unregister_driver(g_cdev_driver);
PRB_ERROR0:
    if (g_cdev_driver->pcdev_work_queue != NULL) {
        destroy_workqueue(g_cdev_driver->pcdev_work_queue);
    }
    kfree(g_cdev_driver);
    g_cdev_driver = NULL;

    return;
}

static void pcie_dma_free(void)
{
#ifdef CONFIG_PCIE_CDEV_DMA_DOUBLE
    head = &g_pcdev_ctx.dma_ctx.head;
    spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
    while (!list_empty(head)) {
        list = list_entry(head->next, struct pcie_cdev_dma_list, list);
        list_del(&list->list);
        g_pcdev_ctx.dma_ctx.kick_dma_trans_fail++;
    }
    spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
#endif

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    struct list_head *head = NULL;
    struct pcie_cdev_dma_list *list = NULL;
    unsigned long dma_flags;
    if (g_pcdev_ctx.work_mode == PCIE_EP) {
        head = &g_pcdev_ctx.dma_ctx.head;
        spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);
        while (!list_empty(head)) {
            list = list_entry(head->next, struct pcie_cdev_dma_list, list);
            list_del(&list->list);
            g_pcdev_ctx.dma_ctx.kick_dma_trans_fail++;
        }
        spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock, dma_flags);

        head = &g_pcdev_ctx.dma_ctx.head_rc;
        spin_lock_irqsave(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);
        while (!list_empty(head)) {
            list = list_entry(head->next, struct pcie_cdev_dma_list, list);
            list_del(&list->list);
            g_pcdev_ctx.dma_ctx.kick_dma_trans_fail_rc++;
        }
        spin_unlock_irqrestore(&g_pcdev_ctx.dma_ctx.dma_lock_rc, dma_flags);
    }
#endif
}

void pcdev_exit(void)
{
    int i;
    struct pcie_cdev_port *port = NULL;

    PCDEV_ERR("in\n");
    if (g_pcdev_ctx.exit_count >= g_pcdev_ctx.init_count) {
        return;
    }

    g_pcdev_ctx.exit_count++;

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        port = g_pcie_cdev_ports[i].port;
        if (port == NULL) {
            continue;
        }
        pcie_cdev_port_exit(port);
    }

    pcie_cdev_notify_disable();

    pcie_dma_free();
    if (g_pcdev_ctx.pcdev_hw_exit != NULL) {
        g_pcdev_ctx.pcdev_hw_exit();
    }
}

int pcdev_initwork_init(void)
{
    g_pcdev_ctx.pcdev_init_work_queue = create_singlethread_workqueue("pcie_cdev_init");
    if (g_pcdev_ctx.pcdev_init_work_queue == NULL) {
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&g_pcdev_init_work, pcdev_init);
    return 0;
}

int pcdev_init_cb(void)
{
    PCDEV_TRACE("in\n");
    if (g_pcdev_ctx.pcdev_init_work_queue == NULL) {
        PCDEV_ERR("pcdev_init_work_queue is NULL\n");
        return -ENOMEM;
    }
    queue_delayed_work(g_pcdev_ctx.pcdev_init_work_queue, &g_pcdev_init_work, 0);
    return 0;
}

static void dump_port_rx_tx_save(struct pcie_cdev_port_dump_s *port_info,
                                 struct pcie_cdev_port *port)
{
    port_info->tx_empty = (unsigned int)port->pstats.tx_empty;
    port_info->tx_full = (unsigned int)port->pstats.tx_full;
    port_info->tx_todo_full = (unsigned int)port->pstats.tx_todo_full;
    port_info->tx_desc_null = (unsigned int)port->pstats.tx_desc_null;
    port_info->tx_desc_err = (unsigned int)port->pstats.tx_desc_err;
    port_info->tx_buf_size_err = (unsigned int)port->pstats.tx_buf_size_err;
    port_info->kick_dma_transfer = (unsigned int)port->pstats.kick_dma_transfer;
    port_info->kick_dma_transfer_fail = (unsigned int)port->pstats.kick_dma_transfer_fail;
    port_info->tx_dma_send_complete = (unsigned int)port->pstats.tx_dma_send_complete;
    port_info->write_base = (unsigned int)port->pstats.write_base;

    port_info->tx_packets = (unsigned int)port->pstats.tx_packets;
    port_info->tx_packets_finish = (unsigned int)port->pstats.tx_packets_finish;
    port_info->tx_packets_cb = (unsigned int)port->pstats.tx_packets_cb;
    port_info->tx_packets_fail = (unsigned int)port->pstats.tx_packets_fail;
    port_info->write_cb_call = (unsigned int)port->pstats.write_cb_call;
    port_info->write_info_cb_call = (unsigned int)port->pstats.write_info_cb_call;
    port_info->write_cb_null = (unsigned int)port->pstats.write_cb_null;
    port_info->write_async_call = (unsigned int)port->pstats.write_async_call;
    port_info->sync_tx_submit = (unsigned int)port->pstats.sync_tx_submit;
    port_info->sync_tx_wait_fail = (unsigned int)port->pstats.sync_tx_wait_fail;
    port_info->sync_tx_done = (unsigned int)port->pstats.sync_tx_done;
    port_info->sync_tx_fail = (unsigned int)port->pstats.sync_tx_fail;

    port_info->rx_close = (unsigned int)port->pstats.rx_close;
    port_info->rx_empty = (unsigned int)port->pstats.rx_empty;
    port_info->rx_todo_empty = (unsigned int)port->pstats.rx_todo_empty;
    port_info->rx_process_empty = (unsigned int)port->pstats.rx_process_empty;
    port_info->rx_full = (unsigned int)port->pstats.rx_full;
    port_info->rx_desc_null = (unsigned int)port->pstats.rx_desc_null;
    port_info->rx_desc_err = (unsigned int)port->pstats.rx_desc_err;
    port_info->rx_get_bur_err = (unsigned int)port->pstats.rx_get_bur_err;
    port_info->rx_packets = (unsigned int)port->pstats.rx_packets;
    port_info->rx_packets_finish = (unsigned int)port->pstats.rx_packets_finish;

    port_info->rx_packets_fail = (unsigned int)port->pstats.rx_packets_fail;
    port_info->rx_bytes = (unsigned int)port->pstats.rx_bytes;
    port_info->rx_get_buf_pram_err = (unsigned int)port->pstats.rx_get_buf_pram_err;
    port_info->get_buf_call = (unsigned int)port->pstats.get_buf_call;
    port_info->ret_buf_call = (unsigned int)port->pstats.ret_buf_call;
    port_info->read_cb_call = (unsigned int)port->pstats.read_cb_call;
    port_info->read_cb_null = (unsigned int)port->pstats.read_cb_null;
}

static void pcdev_dump_port_cominfo_save(struct pcie_cdev_port_dump_s *port_info,
                                         struct pcie_cdev_port *port)
{
    port_info->open_count = (unsigned int)port->pstats.open_count;
    port_info->close_count = (unsigned int)port->pstats.close_count;
    port_info->close_count_repeat = (unsigned int)port->pstats.close_count_repeat;
    port_info->openclose = (unsigned int)port->pstats.openclose;
    port_info->remote_close = (unsigned int)port->pstats.remote_close;
    port_info->local_close = (unsigned int)port->pstats.local_close;

    port_info->irq_send = (unsigned int)port->pstats.irq_send;
    port_info->event_send = (unsigned int)port->pstats.event_send;
    port_info->event_cb_call = (unsigned int)port->pstats.event_cb_call;
    port_info->event_cb_null = (unsigned int)port->pstats.event_cb_null;

    dump_port_rx_tx_save(port_info, port);
}

static void pcdev_dump_port_info_save(void)
{
    struct pcie_cdev_port_dump_s *port_info = NULL;
    struct pcie_cdev_port *port = NULL;
    int i;

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        port_info = &g_pcdev_dump->pcdev_port_dump[i];
        port = g_pcie_cdev_ports[i].port;
        if (port == NULL) {
            PCDEV_ERR("dump port(%d) is NULL\n", i);
            continue;
        }

        port_info->port_start = PCIE_DUMP_PORT_MARK;
        port_info->port_num = port->port_num;
        if (g_pcdev_ctx.work_mode == PCIE_EP) {
            port_info->local_stat.u32 = port->local_stat->u32;
            port_info->remote_stat.u32 = port->remote_stat->u32;
            port_info->dmamap_skip = port->dmamap_skip;

            port_info->rx_wp = *port->rx_wp;
            port_info->rx_rp = *port->rx_rp;
            port_info->rx_rp_toget = port->rx_rp_toget;
            port_info->rx_rp_process = port->rx_rp_process;
            port_info->rx_buf_max_size = port->rx_buf_max_size;
            port_info->rx_num = port->rx_num;

            port_info->tx_wp = *port->tx_wp;
            port_info->tx_rp = *port->tx_rp;
            port_info->tx_rp_todo = *port->tx_rp_todo;
            port_info->tx_rp_complete = port->tx_rp_complete;
            port_info->tx_buf_max_sz = *port->tx_buf_max_sz;
            port_info->tx_num = port->tx_num;
            port_info->write_sync = port->write_sync;
            port_info->write_blocked = port->write_blocked;
#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
            port_info->tx_rp_todo_rc = *port->tx_rp_todo_rc;
            port_info->tx_rp_dma_cmp_rc = *port->tx_rp_dma_cmp_rc;
            port_info->tx_rp_dma_cfg_rc = port->tx_rp_dma_cfg_rc;
            port_info->tx_rp_cmp = port->tx_rp_cmp;
#endif
        }

        pcdev_dump_port_cominfo_save(port_info, port);
    }
}

static void pcdev_dump_hook(void)
{
    int i, ret;

    if (g_pcdev_dump == NULL) {
        return;
    }

    if (g_pcdev_ctx.work_mode == PCIE_RC) {
        for (i = 0; i < PCIE_CDEV_COUNT; i++) {
            PCDEV_ERR("tx port(%d) vaddr:0x%llx dma_high:0x%x dma_low:dma_high:0x%x\n", i,
                      g_pcdev_dump->tx_desc_dump[i].vaddr, g_pcdev_dump->tx_desc_dump[i].dma_high,
                      g_pcdev_dump->tx_desc_dump[i].dma_low);
            PCDEV_ERR("rx port(%d) vaddr:0x%llx dma_high:0x%x dma_low:dma_high:0x%x\n", i,
                      g_pcdev_dump->rx_desc_dump[i].vaddr, g_pcdev_dump->rx_desc_dump[i].dma_high,
                      g_pcdev_dump->rx_desc_dump[i].dma_low);
        }
    }

    ret = memcpy_s(g_pcdev_dump->vote_dbg, sizeof(g_pcdev_dump->vote_dbg), g_pcdev_ctx.vote_dbg,
                   sizeof(struct pcdev_vote_dbg_s) * (PCIE_CDEV_COUNT + 1));
    if (ret) {
        PCDEV_ERR("dump memset_s fail\n");
    }

    pcdev_dump_port_info_save();
}

void pcdev_dump_init(void)
{
    int ret;

    g_pcdev_dump = (struct pcie_cdev_dump_s *)bsp_dump_register_field(DUMP_MODEMAP_PCDEV, "PCDEV",
                                                                      PCIE_CDEV_DUMP_SIZE, 0);
    if (g_pcdev_dump == NULL) {
        PCDEV_ERR("dump mem alloc fail\n");
        return;
    }
    ret = (int)memset_s(g_pcdev_dump, PCIE_CDEV_DUMP_SIZE, 0, PCIE_CDEV_DUMP_SIZE);
    if (ret) {
        PCDEV_ERR("dump memset_s fail\n");
    }

    ret = bsp_dump_register_hook("PCDEV", pcdev_dump_hook);
    if (ret == BSP_ERROR) {
        PCDEV_ERR("register om fail\n");
    }
}
