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
#ifndef __WAN_ETH_HAL_EVENT_BUFFER_H__
#define __WAN_ETH_HAL_EVENT_BUFFER_H__

#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/threads.h>

#define MAX_EVENT_NUM 1024
#define MAX_HANDLE_EVENT_NUM 1024

#define TYPE_LEN 4
#define PORT_NO_LEN 4
#define INDEX_LEN 10
#define NUM_LEN 14
#define MAX_LITTLE_CORE 3
#define MAX_MID_CORE 5
#define MAX_LAGRE_CORE 7
#define MAX_QLEN_PCPU 1024

union event_element {
    struct {
        unsigned int event_type : 4;
        unsigned int port_no : 4;
        unsigned int dma_desc_idx : 10;
        unsigned int num_per_table : 14;
    } bits;
    unsigned int bit32;
};

union write_read_pointer {
    struct {
        unsigned int wp;
        unsigned int rp;
    } bits;
    unsigned long long bit64;
};

struct event_buffer {
    union write_read_pointer wr_rd_p;
    union event_element event[MAX_EVENT_NUM];
};

struct event_buffer_debug {
    union write_read_pointer wr_rd_p[MAX_EVENT_NUM];
    union event_element event[MAX_EVENT_NUM];
};

enum weth_event_type {
    WETH_LINK_UP,
    WETH_LINK_DOWN,
    WETH_CONFIG_DMA,
    WETH_TX_DMA_COMPLETE,
    WETH_RX_DMA_COMPLETE,
    WETH_REFILL_COMPLETE,
    WETH_SYNC_TABLE,
    WETH_SYNC_TX_TABLE_COMPLETE,
    WETH_SYNC_RX_TABLE_COMPLETE,
    WETH_EVENT_TYPE_NUM,
};

typedef void (*weth_event_cb_t)(unsigned int index, unsigned num);

struct event_buff_tx_stats {
    unsigned int send_event;
    unsigned int event_buffer_full;
    unsigned int send_irq;
    unsigned int event_buffer_nearly_full;
    unsigned int invalid_tx_wp;
    unsigned int invalid_rx_rp;
    unsigned int event_count[WETH_EVENT_TYPE_NUM];
};

struct event_buff_rx_stats {
    unsigned int handle_event_start;
    unsigned int invalid_event;
    unsigned int invalid_index;
    unsigned int event_handled;
    unsigned int event_count[WETH_EVENT_TYPE_NUM];
};

enum weth_pcie_mode {
    WETH_PCIE_EP = 0,
    WETH_PCIE_RC = 4,
};

struct event_buff_ctx {
    struct event_buffer *tx_event;
    struct event_buffer *rx_event;
    struct event_buffer_debug *rx_event_debug;
    int wr_rd_index;
    unsigned int cur_cpu;
    struct event_buff_tx_stats tx_stats;
    struct event_buff_rx_stats rx_stats;

    enum weth_pcie_mode work_mode;
    weth_event_cb_t event_cb[WETH_EVENT_TYPE_NUM];

    spinlock_t rx_lock;
    spinlock_t tx_lock;
    unsigned int event_buff_size;
    unsigned int handle_event_buff_size;
    int cpu_run_max;
    int low_perf;

    struct tasklet_struct evt_smp_recall;

    unsigned int stat_cpu_not_online;
    unsigned int stat_find_cpu_ok;
    unsigned int stat_find_cpu_fail;
    unsigned int stat_find_min_len_fail;
    unsigned int stat_smp_call_fail;
    unsigned int invalid_port_no;
    unsigned int debug_port_no;
    unsigned int valid_port_no;
    unsigned int extra_port_no;
    unsigned int smp_recall;
    unsigned int cpu_not_online;
    unsigned int pcpu_packets_thru;
    int cpu_per_port[NR_CPUS];
};

int weth_send_event(unsigned int num, unsigned int dma_desc_idx, unsigned int event_type, int send_irq);
int weth_send_event_by_port(int port_no, unsigned int num, unsigned int dma_desc_idx, unsigned int event_type,
                            int send_irq);
int weth_event_buff_init(enum weth_pcie_mode work_mode, struct event_buffer *tx_event, struct event_buffer *rx_event);
void weth_event_buff_set_cb(weth_event_cb_t ops[]);
void weth_event_buff_reinit(struct event_buffer *tx_event, struct event_buffer *rx_event);

void weth_event_buff_exit(void);
int weth_event_get_status(void);
struct event_buff_ctx *weth_get_event_buff_ctx(void);
#endif
