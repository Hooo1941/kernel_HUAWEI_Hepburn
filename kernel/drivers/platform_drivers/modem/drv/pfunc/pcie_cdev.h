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

#ifndef __PCIE_CDEV_H__
#define __PCIE_CDEV_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/pci.h>
#include "mdrv_pcdev.h"
#include "bsp_dump.h"
#include <bsp_pcdev.h>
#include "pcie_cdev_desc.h"
#include <bsp_pcie.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DRV_NAME "PCIE_CDEV"
#define PCIE_CDEV_NAME_MAX 64
#define PCIE_CDEV_ALLOCED 0X5A5A
#define VOTE_FLAG 0x1221
#define PCIE_CDEV_DUMP_SIZE 0x2000
#define PCIE_VOLUME_SPEED_SET 1500

enum pcie_cdev_mode {
    PCIE_EP = 0,
    PCIE_RC = 4,
};

enum pcie_cdev_vote_mode {
    NOTIFY_CB = 0,
    READ_CB_MODE = 1,
    WRITE_BASE_MODE = 2,
    WRITE_CB_MODE = 3,
    GET_READBUF_MODE = 4,
    SEND_EVT_MODE = 6,
    PORT_OPEN_MODE = 7,
    INIT_MODE = 9,
    REALLOC_MODE = 10,
    READ_SIG_MODE = 11,
    REL_IND_MODE = 12,
    MSC_WRITE_MODE = 13,
    PRINT_MODE = 14,    
    VOTE_MODE_BOTTOM,
};

struct pcie_cdev_name_type_tbl {
    char *name;
    UNIFY_IF_PROT type;
    unsigned int ep2rc_max_buf_sz;
    unsigned int rc2ep_max_buf_sz;
    unsigned int buf_num;
    unsigned int user_space_check;
    unsigned int port_make_device;
};

struct pcie_cdev_driver {
    struct kref kref; /* Reference management */
    struct cdev cdev;
    struct module *owner;
    const char *driver_name;
    const char *name;
    dev_t dev_no;
    int name_base;            /* offset of printed name */
    unsigned int major;       /* major device number */
    unsigned int minor_start; /* start of minor device number */
    int num;                  /* number of devices allocated */

    struct workqueue_struct *pcdev_work_queue;
};

struct pcdev_port_stats {
    unsigned long open_count;
    unsigned long close_count;
    unsigned long close_count_repeat;
    unsigned long openclose;
    unsigned long remote_close;
    unsigned long local_close;

    unsigned long irq_send;
    unsigned long event_send;
    unsigned long event_cb_call;
    unsigned long event_cb_null;
    unsigned long read_sig_send;
    unsigned long read_sig_cb_call;
    unsigned long read_sig_cb_null;
    unsigned long rel_ind_send;
    unsigned long rel_ind_cb_call;
    unsigned long rel_ind_cb_null;
    unsigned long msc_stru_send;
    unsigned long msc_stru_cb_call;
    unsigned long msc_stru_cb_null;

    unsigned long tx_empty;
    unsigned long tx_full;
    unsigned long tx_todo_full;
    unsigned long tx_desc_null;
    unsigned long tx_desc_err;
    unsigned long tx_buf_size_err;
    unsigned long kick_dma_1;
    unsigned long kick_dma_2;
    unsigned long kick_dma_transfer;
    unsigned long kick_dma_transfer_fail;
    unsigned long tx_dma_send_complete;
    unsigned long write_base;
    unsigned long tx_packets;
    unsigned long tx_packets_finish;
    unsigned long tx_packets_cb;
    unsigned long tx_packets_fail;
    unsigned long tx_bytes;
    unsigned long write_cb_call;
    unsigned long write_info_cb_call;
    unsigned long write_cb_null;
    unsigned long write_async_call;
    unsigned long sync_tx_submit;
    unsigned long sync_tx_wait_fail;
    unsigned long sync_tx_done;
    unsigned long sync_tx_fail;

    unsigned long rx_close;
    unsigned long rx_empty;
    unsigned long rx_todo_empty;
    unsigned long rx_process_empty;
    unsigned long rx_full;
    unsigned long rx_desc_null;
    unsigned long rx_desc_err;
    unsigned long rx_get_bur_err;
    unsigned long rx_packets;
    unsigned long rx_packets_finish;
    unsigned long rx_packets_fail;
    unsigned long rx_bytes;
    unsigned long rx_get_buf_pram_err;
    unsigned long get_buf_call;
    unsigned long ret_buf_call;
    unsigned long read_cb_call;
    unsigned long read_cb_null;
    unsigned long sync_read_wait_fail;

    unsigned long callback_null;
};

struct pcie_cdev_dma_ctx {
    unsigned int dma_channel;
    spinlock_t dma_lock;
    struct list_head head;
    unsigned long kick_dma_trans_succ;
    unsigned long kick_dma_trans_fail;
    unsigned long dma_trans_cmp;
#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    spinlock_t dma_lock_rc;
    struct list_head head_rc;
    unsigned long kick_dma_trans_succ_rc;
    unsigned long kick_dma_trans_fail_rc;
    unsigned long dma_trans_cmp_rc;
#endif
};

struct pcdev_vote_dbg_s {
    unsigned long long vote_port;
    unsigned long long vote_fail_port;
    unsigned long long vote_lasttime;
    int vote_lastmode;
    unsigned long long unvote_port;
    unsigned long long unvote_lasttime;
    int unvote_lastmode;
};

struct pcdev_ctx {
    struct platform_device *pdev;
    unsigned int pcie_id;
    unsigned int work_mode;
    unsigned int init_count;
    unsigned int exit_count;
    unsigned int diag_mode;
    unsigned int vote_flag;
    struct pcdev_vote_dbg_s vote_dbg[PCIE_CDEV_COUNT + 1];

    void *phys_addr;
    void *virt_addr;
    unsigned long buffer_size;
    unsigned long vote_in_interrupt;
    struct pcdev_ports_desc *ports_desc;

    struct workqueue_struct *pcdev_init_work_queue;
    struct intr_handler *irq_table;
    irqreturn_t (*irq_handler)(void);
    int (*send_irq)(void);
    int (*dma_transfer)(unsigned int user_id, struct pcie_dma_transfer_info *transfer_info);
    int (*pcdev_hw_init)(void);
    void (*pcdev_hw_exit)(void);
    int (*pcdev_vote_lock)(int);
    int (*pcdev_vote_unlock)(int);
    int (*pcdev_vote_try_lock)(int);
    void (*get_curtime)(u64 *);
    int (*pcie_first_user)(enum pcie_ep_to_rc_msi_id);

    struct pcie_cdev_dma_ctx dma_ctx;

    unsigned int msg_level;
    unsigned int print_port;
};

struct pcie_cdev_dma_list {
    struct list_head list;
    struct pcie_dma_transfer_info dma_info;
};

struct pcdev_buf_list {
    struct list_head list;
    pcdev_wr_async_info_s buf_info;
};

struct pcdev_msc_list {
    struct list_head list;
    pcdev_mdm_msc_stru msc_stru;
};

struct pcie_cdev_port {
    char *name;
    unsigned int port_num;
    UNIFY_IF_PROT *port_id;
    struct device *dev;
    spinlock_t port_lock;
    wait_queue_head_t buf_free_wait;
    union pcie_cdev_status *local_stat;
    union pcie_cdev_status *remote_stat;
    u32 *pcie_cdev_send_irq[IRQ_NUM];
    u32 *pcie_cdev_rcv_irq[IRQ_NUM];
    unsigned int dmamap_skip;

    pcdev_event_cb_t event_cb;
    unsigned int *event_type;
    unsigned int event_type_arg;
    pcdev_modem_msc_read_cb_t read_sig_cb;
    pcdev_mdm_msc_stru *read_sig;
    pcdev_mdm_msc_stru read_sig_arg;
    pcdev_modem_rel_ind_cb_t rel_ind_cb;
    unsigned int *rel_ind;
    unsigned int rel_ind_arg;
    pcdev_modem_msc_write_cb_t msc_stru_cb;
    pcdev_mdm_msc_stru *msc_stru;
    pcdev_mdm_msc_stru msc_stru_arg;
    unsigned int *msc_busy;
    struct list_head msc_stru_head;

    unsigned int rd_buf_alloced;
    unsigned int set_readbuf;
    unsigned int *rx_wp;
    unsigned int *rx_rp;
    unsigned int rx_rp_toget;
    unsigned int rx_rp_process;
    unsigned int *rx_buf_max_sz;
    unsigned int rx_buf_max_size;
    unsigned int rx_num;
    pcdev_read_done_cb_t read_done_cb;
    struct pcdev_desc *rx_desc;
    unsigned long long d_vaddr[NUM_E2R_DESC];
    struct list_head ret_buf_head;
    struct list_head read_buf_head;
    struct list_head write_buf_head;
    wait_queue_head_t read_wait;
    unsigned int read_sync;
    unsigned int read_blocked;

    unsigned int *tx_wp;
    unsigned int *tx_rp;
    unsigned int *tx_rp_todo;
    unsigned int tx_rp_complete;
    unsigned int *tx_buf_max_sz;
    unsigned int tx_num;
    pcdev_write_done_cb_t write_done_cb;
    pcdev_write_info_done_cb_t write_info_done_cb;
    struct pcdev_desc *tx_desc;
    struct pcie_cdev_dma_list *transfer_info;
    struct pcdev_buf_list *ret_buf_list;
    struct pcdev_buf_list *write_buf_list;
    wait_queue_head_t write_wait;
    unsigned int write_sync;
    unsigned int write_blocked;

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    unsigned int *tx_rp_todo_rc;
    unsigned int *tx_rp_dma_cmp_rc;
    unsigned int tx_rp_dma_cfg_rc;
    unsigned int tx_rp_cmp;
    struct pcie_cdev_dma_list *transfer_info_rc;
    pcie_callback dma_callback;
#endif

    pcie_callback dma_read_cb;
    pcie_callback dma_write_cb;

    struct pcdev_port_stats pstats;
    struct pcdev_port_timestamp timestamp[PCDEV_TIMESTAMP_COUNT];
};

struct pcie_cdev_port_dump_s {
    unsigned int port_start;
    unsigned int port_num;
    union pcie_cdev_status local_stat;
    union pcie_cdev_status remote_stat;
    unsigned int dmamap_skip;

    unsigned int rx_wp;
    unsigned int rx_rp;
    unsigned int rx_rp_toget;
    unsigned int rx_rp_process;
    unsigned int rx_buf_max_size;
    unsigned int rx_num;

    unsigned int tx_wp;
    unsigned int tx_rp;
    unsigned int tx_rp_todo;
    unsigned int tx_rp_complete;
    unsigned int tx_buf_max_sz;
    unsigned int tx_num;
    unsigned int write_sync;
    unsigned int write_blocked;

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    unsigned int tx_rp_todo_rc;
    unsigned int tx_rp_dma_cmp_rc;
    unsigned int tx_rp_dma_cfg_rc;
    unsigned int tx_rp_cmp;
#endif

    unsigned int open_count;
    unsigned int close_count;
    unsigned int close_count_repeat;
    unsigned int openclose;
    unsigned int remote_close;
    unsigned int local_close;

    unsigned int irq_send;
    unsigned int event_send;
    unsigned int event_cb_call;
    unsigned int event_cb_null;

    unsigned int tx_empty;
    unsigned int tx_full;
    unsigned int tx_todo_full;
    unsigned int tx_desc_null;
    unsigned int tx_desc_err;
    unsigned int tx_buf_size_err;
    unsigned int kick_dma_transfer;
    unsigned int kick_dma_transfer_fail;
    unsigned int tx_dma_send_complete;
    unsigned int write_base;
    unsigned int tx_packets;
    unsigned int tx_packets_finish;
    unsigned int tx_packets_cb;
    unsigned int tx_packets_fail;
    unsigned int write_cb_call;
    unsigned int write_info_cb_call;
    unsigned int write_cb_null;
    unsigned int write_async_call;
    unsigned int sync_tx_submit;
    unsigned int sync_tx_wait_fail;
    unsigned int sync_tx_done;
    unsigned int sync_tx_fail;

    unsigned int rx_close;
    unsigned int rx_empty;
    unsigned int rx_todo_empty;
    unsigned int rx_process_empty;
    unsigned int rx_full;
    unsigned int rx_desc_null;
    unsigned int rx_desc_err;
    unsigned int rx_get_bur_err;
    unsigned int rx_packets;
    unsigned int rx_packets_finish;
    unsigned int rx_packets_fail;
    unsigned int rx_bytes;
    unsigned int rx_get_buf_pram_err;
    unsigned int get_buf_call;
    unsigned int ret_buf_call;
    unsigned int read_cb_call;
    unsigned int read_cb_null;
};

struct pcie_cdev_desc_dump_s {
    unsigned long long vaddr;
    unsigned int dma_low;
    unsigned int dma_high;
};

struct pcie_cdev_dump_s {
    struct pcie_cdev_desc_dump_s tx_desc_dump[PCIE_CDEV_COUNT];
    struct pcie_cdev_desc_dump_s rx_desc_dump[PCIE_CDEV_COUNT];
    void *phys_addr;
    void *virt_addr;
    unsigned long buffer_size;
    struct pcdev_vote_dbg_s vote_dbg[PCIE_CDEV_COUNT + 1];
    struct pcie_cdev_port_dump_s pcdev_port_dump[PCIE_CDEV_COUNT];
};

struct pcie_cdev_port_manager {
    unsigned int is_alloc;
    struct pcie_cdev_port *port;
    struct device *cdev;
    union pcie_cdev_map *desc;
};

struct intr_handler {
    char *name;
    unsigned int cnt;
    void (*callback)(struct pcie_cdev_port *);
};

/* --- pcdev evt managment --- */
struct pcie_cdev_evt_manage {
    void *port_evt_array[PCIE_CDEV_COUNT];
    int port_evt_pos;
    char *name;
    spinlock_t evt_lock;
};

int pcdev_initwork_init(void);
int pcdev_init_cb(void);
void pcdev_exit(void);
long pcie_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
ssize_t pcie_cdev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t pcie_cdev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
void pcdev_dump_init(void);
void pcdev_ports_ctx_clear(void);
void pcdev_ctx_clear(void);
struct pcdev_ctx* get_pcdev_ctx(void);
struct pcie_cdev_port_manager* get_pcdev_ports(void);
struct pcie_cdev_name_type_tbl* get_pcdev_type_table(void);

#ifdef __cplusplus
}
#endif
#endif
