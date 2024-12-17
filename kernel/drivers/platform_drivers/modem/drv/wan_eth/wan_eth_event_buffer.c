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

#include <linux/netdevice.h>
#include <linux/jiffies.h>
#include <linux/version.h>

#include "wan_eth_event_buffer.h"
#include "wan_eth_common.h"
#include "bsp_pcie.h"
#include "bsp_dump.h"
#include "mdrv_errno.h"

#include "product_config.h"

struct event_buff_ctx g_weth_event_ctx;

#ifdef CONFIG_WETH
#define CONFIG_WETH_SMP_EVENT
#endif

struct event_buff_ctx *weth_get_event_buff_ctx(void)
{
    return &g_weth_event_ctx;
}

int weth_process_send_event(int port_no, unsigned int num, unsigned int dma_desc_idx, unsigned int event_type,
                            int send_irq);
/* for event balence */
#ifdef CONFIG_WETH_SMP_EVENT

#define EVENT_QLEN_CALC_TIME 100

#define MAX_LOW_PERF_CORE 4

struct event_pcpu_ctx {
    union event_element *event;
    union event_element *event_handle;
    int handle_index;
    unsigned int handle_event_w;
    unsigned int handle_event_r;
    unsigned int event_w;
    unsigned int event_r;
    unsigned int cpu;
    unsigned int evt_weight;
    unsigned int evt_order;
    unsigned int evt_num;
    int cpu_run_max;
    int cpu_run_cnt;
    unsigned long state;
    struct __call_single_data csd ____cacheline_aligned_in_smp;
    struct tasklet_struct evt_smp_push;
    spinlock_t event_process_lock;

    unsigned int stat_smp_call;
    unsigned int stat_direct_call;
    unsigned int stat_event_handled;
    unsigned int stat_process_end;
    unsigned int stat_process_call;
    unsigned int stat_other_cpu_call;
    unsigned int stat_find_cpu_ok;
    unsigned int stat_find_cpu_fail;
    unsigned int stat_find_min_len_fail;
    unsigned int stat_smp_call_fail;
    unsigned int stat_smp_call_function_fail;
    unsigned int state_cpu_busy;
    unsigned int smp_recall;
    unsigned int state_smp_recall_failed;
    unsigned int state_smp_call_failed;
};

DEFINE_PER_CPU_ALIGNED(struct event_pcpu_ctx, g_epcpu_ctx);

unsigned int g_evt_dft_weight[] = {
    1, 2, 2, 2, 8, 8, 16, 16
};

unsigned int g_evt_dft_order[] = {
    1, 1, 1, 1, 4, 4, 8, 8
};

unsigned int g_evt_run_max[] = {
    1, 2, 2, 2, 4, 4, 8, 8
};

int weth_event_set_weight(int cpu, unsigned int weight)
{
    if (cpu >= nr_cpu_ids) {
        return -1;
    }
    g_evt_dft_weight[cpu] = weight;
    return 0;
}

int weth_event_set_pcpu_packets_thru(int value)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    ectx->pcpu_packets_thru = value;

    return ectx->pcpu_packets_thru;
}

int weth_event_set_run_max(int cpu, unsigned int max)
{
    if (cpu >= nr_cpu_ids) {
        return -1;
    }
    g_evt_run_max[cpu] = max;
    return 0;
}

int weth_event_set_low_perf(int value)
{
    g_weth_event_ctx.low_perf = value;
    return g_weth_event_ctx.low_perf;
}

STATIC void weth_event_process(void);

STATIC unsigned int weth_event_get_next_cpu(unsigned int cur_cpu)
{
    return cpumask_next_wrap(cur_cpu, cpu_online_mask, cur_cpu, 1);
}

#define WETH_EVENT_LEN_MIN 0xFFFFFF
STATIC void weth_event_find_cpu(struct event_pcpu_ctx *find_cpu, struct event_pcpu_ctx *min_pcpu)
{
    struct event_pcpu_ctx *cur_pcpu = NULL;
    unsigned int cur_cpu;
    unsigned int i = 0;
    unsigned int evt_min = WETH_EVENT_LEN_MIN;
    unsigned int cpu_online = num_online_cpus();
    unsigned int evt_len;

    cur_cpu = g_weth_event_ctx.cur_cpu;
    do {
        if (!cpu_online(cur_cpu)) {
            cur_cpu = weth_event_get_next_cpu(cur_cpu);
            g_weth_event_ctx.stat_cpu_not_online++;
            continue;
        }
        cur_pcpu = &per_cpu(g_epcpu_ctx, cur_cpu);
        evt_len = cur_pcpu->evt_num;
        if (evt_len < cur_pcpu->evt_weight) {
            if (cur_pcpu->cpu_run_cnt < 0) {
                cur_pcpu->cpu_run_cnt = cur_pcpu->cpu_run_max;
                cur_cpu = weth_event_get_next_cpu(cur_cpu);
                continue;
            } else {
                cur_pcpu->cpu_run_cnt--;
            }
            find_cpu = cur_pcpu;
            g_weth_event_ctx.cur_cpu = find_cpu->cpu;
            g_weth_event_ctx.stat_find_cpu_ok++;
            break;
        } else {
            evt_len = evt_len >> cur_pcpu->evt_order;
            if (evt_min > evt_len) {
                min_pcpu = cur_pcpu;
                evt_min = evt_len;
            }
        }
        cur_cpu = weth_event_get_next_cpu(cur_cpu);
    } while ((find_cpu == NULL) && i++ < cpu_online);

    return;
}

STATIC struct event_pcpu_ctx *weth_event_get_cpu(void)
{
    struct event_pcpu_ctx *min_pcpu = NULL;
    struct event_pcpu_ctx *find_cpu = NULL;

    weth_event_find_cpu(find_cpu, min_pcpu);

    if (unlikely(find_cpu == NULL)) {
        if (min_pcpu != NULL) {
            g_weth_event_ctx.stat_find_cpu_fail++;
            find_cpu = min_pcpu;
        } else {
            g_weth_event_ctx.stat_find_min_len_fail++;
            find_cpu = this_cpu_ptr(&g_epcpu_ctx);
        }
    }

    return find_cpu;
}

STATIC void weth_event_dispatch_push(unsigned long data)
{
    union event_element *rx_event = NULL;
    unsigned int event_type;
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    struct event_pcpu_ctx *pcpu = (struct event_pcpu_ctx *)data;
    unsigned long flags;
    unsigned int handle_index = 0;

    pcpu->stat_direct_call++;

    spin_lock_irqsave(&pcpu->event_process_lock, flags);
    while (weth_rw_get_busy(pcpu->event_w, pcpu->event_r, ectx->event_buff_size)) {
        pcpu->stat_process_call++;
        *(pcpu->event_handle + pcpu->handle_event_w) = *(pcpu->event + pcpu->event_r);
        pcpu->handle_event_w = weth_rw_get_pos(pcpu->handle_event_w + 1, ectx->handle_event_buff_size);
        pcpu->event_r = weth_rw_get_pos(pcpu->event_r + 1, ectx->event_buff_size);
        handle_index++;
    }
    spin_unlock_irqrestore(&pcpu->event_process_lock, flags);

    pcpu->handle_index = handle_index;
    while (weth_rw_get_busy(pcpu->handle_event_w, pcpu->handle_event_r, ectx->handle_event_buff_size)) {
        rx_event = pcpu->event_handle + pcpu->handle_event_r;
        pcpu->handle_event_r = weth_rw_get_pos(pcpu->handle_event_r + 1, ectx->handle_event_buff_size);
        event_type = (unsigned int)rx_event->bits.event_type;
        if (event_type >= WETH_EVENT_TYPE_NUM) {
            ectx->rx_stats.invalid_event++;
            continue;
        }
        pcpu->stat_event_handled++;
        ectx->event_cb[event_type]((unsigned int)rx_event->bits.dma_desc_idx,
                                   (unsigned int)rx_event->bits.num_per_table);
        rx_event->bits.event_type = WETH_EVENT_TYPE_NUM;
    }

    return;
}

STATIC void weth_event_push(unsigned long data)
{
    struct common_info *comm_info = &g_common_info;

    atomic_add(1, &comm_info->bar_access);
    if (unlikely(!comm_info->link_status)) {
        if (comm_info->init_finish) {
            comm_info->link_down_rx++;
            atomic_sub(1, &comm_info->bar_access);
            return;
        }
    }

    weth_event_dispatch_push(data);
    atomic_sub(1, &comm_info->bar_access);

    return;
}

STATIC void weth_event_proc_smp(void *data)
{
    struct event_pcpu_ctx *pcpu = (struct event_pcpu_ctx *)data;

    tasklet_hi_schedule(&pcpu->evt_smp_push);
    pcpu->stat_smp_call++;
    clear_bit(WETH_SMP_BIT_SCHED, &pcpu->state);
    return;
}

STATIC int weth_event_init_pcpu(struct event_buff_ctx *ectx)
{
    int i;

    /* init rx per cpu ctx */
    for_each_possible_cpu(i) {
        struct event_pcpu_ctx *pcpu = &per_cpu(g_epcpu_ctx, i);

        pcpu->csd.func = weth_event_proc_smp;
        pcpu->csd.info = pcpu;
        pcpu->state = 0;
        pcpu->cpu = i;

        pcpu->evt_weight = g_evt_dft_weight[i];
        pcpu->evt_order = g_evt_dft_order[i];
        pcpu->cpu_run_max = g_evt_run_max[i];
        pcpu->cpu_run_cnt = pcpu->cpu_run_max;
        pcpu->stat_smp_call = 0;
        pcpu->stat_direct_call = 0;
        pcpu->handle_event_w = 0;
        pcpu->handle_event_r = 0;
        pcpu->event_w = 0;
        pcpu->event_r = 0;
        ectx->cpu_per_port[i] = i;

        spin_lock_init(&pcpu->event_process_lock);

        tasklet_init(&pcpu->evt_smp_push, weth_event_push, (uintptr_t)pcpu);
        pcpu->event = (union event_element *)kzalloc(MAX_EVENT_NUM * sizeof(union event_element), GFP_KERNEL);
        if (pcpu->event == NULL) {
            return -1;
        }

        pcpu->event_handle = (union event_element *)kzalloc(MAX_HANDLE_EVENT_NUM * sizeof(union event_element),
                                                            GFP_KERNEL);
        if (pcpu->event_handle == NULL) {
            kfree(pcpu->event);
            return -1;
        }
    }
    return 0;
}

STATIC void weth_event_deinit_pcpu(void)
{
    int i;

    /* init rx per cpu ctx */
    for_each_possible_cpu(i)
    {
        struct event_pcpu_ctx *pcpu = &per_cpu(g_epcpu_ctx, i);

        if (pcpu->event != NULL) {
            kfree((void *)pcpu->event);
        }
    }
}

static struct event_pcpu_ctx *weth_get_specified_cpu(struct event_buff_ctx *ectx, struct weth_common_ctx *comm_ctx,
    int portno, int cpu)
{
    struct event_pcpu_ctx *pcpu = NULL;

    if (cpu_online(cpu) && comm_ctx->rx_packets_per_cpu[cpu] <= ectx->pcpu_packets_thru) {
        pcpu = &per_cpu(g_epcpu_ctx, cpu);
        ectx->cpu_per_port[portno] = cpu;
    }

    return pcpu;
}

static struct event_pcpu_ctx *weth_get_alternative_cpu(struct event_buff_ctx *ectx, struct weth_common_ctx *comm_ctx, 
    int portno, int first_choosen_cpu)
{
    struct event_pcpu_ctx *pcpu = NULL;
    int i;

    for (i = first_choosen_cpu; i >= 1; i--) {
            pcpu = weth_get_specified_cpu(ectx, comm_ctx, portno, i);
            if (pcpu != NULL) {
                break;
            }
    }

    if (pcpu == NULL) {
        pcpu = &per_cpu(g_epcpu_ctx, 0);
    }

    return pcpu;
}

static struct event_pcpu_ctx *weth_get_online_cpu(struct event_buff_ctx *ectx,
    struct weth_common_ctx *comm_ctx, int portno)
{
    struct event_pcpu_ctx *pcpu = NULL;
    int last_used_cpu = ectx->cpu_per_port[portno];

    if (cpu_online(portno)) {
        pcpu = &per_cpu(g_epcpu_ctx, portno);
    } else if (cpu_online(last_used_cpu)) {
        pcpu = &per_cpu(g_epcpu_ctx, last_used_cpu);
    } else {
        int first_choosen_cpu = MAX_LITTLE_CORE;
        ectx->cpu_not_online++;

        if (portno <= MAX_LITTLE_CORE) {
            first_choosen_cpu = MAX_LITTLE_CORE;
        } else if (portno <= MAX_MID_CORE) {
            first_choosen_cpu = MAX_MID_CORE;
        } else if (portno <= MAX_LAGRE_CORE) {
            first_choosen_cpu = MAX_LAGRE_CORE;
        }

        pcpu = weth_get_alternative_cpu(ectx, comm_ctx, portno, first_choosen_cpu);
    }

    return pcpu;
}

STATIC struct event_pcpu_ctx *weth_get_cpu_from_event(struct event_buff_ctx *ectx, union event_element rx_event)
{
    int port_no = rx_event.bits.port_no;
    int max_cpus = num_possible_cpus();
    struct event_pcpu_ctx *pcpu = NULL;
    struct weth_common_ctx *comm_ctx = &g_common_ctx;

    if (port_no == WETH_INVALID_PORT_NO) {
        ectx->invalid_port_no++;
        pcpu = &per_cpu(g_epcpu_ctx, 0);
    } else if (port_no >= WETH_DEBUG_PORT_NO) {
        ectx->invalid_port_no++;
        pcpu = weth_event_get_cpu();
    } else if (port_no >= max_cpus) {
        ectx->extra_port_no++;
        port_no = port_no % max_cpus;
        pcpu = weth_get_online_cpu(ectx, comm_ctx, port_no);
    } else {
        port_no = port_no + 1;
        if (ectx->low_perf == 1 && comm_ctx->cur_speed_level == WETH_RC_LOW_SPEED) {
            if (port_no >= MAX_LOW_PERF_CORE) {
                port_no = port_no - MAX_LOW_PERF_CORE;
            }
        }

        pcpu = weth_get_online_cpu(ectx, comm_ctx, port_no);
    }

    return pcpu;
}

STATIC int weth_event_smp_call(struct event_pcpu_ctx *pcpu)
{
    int ret;
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    struct event_pcpu_ctx *cur_pcpu = NULL;
    unsigned long flags;

    if (!test_and_set_bit(WETH_SMP_BIT_SCHED, &pcpu->state)) {
        ret = smp_call_function_single_async(pcpu->cpu, &pcpu->csd);
        /* smp call to target cpu exec fail (for exp. cpu not online) */
        if (unlikely(ret)) {
            pcpu->stat_smp_call_function_fail++;
            weth_pr_err("smp_call_function_single_async failed, current cpu: %d\n", pcpu->cpu);

            spin_lock_irqsave(&pcpu->event_process_lock, flags);
            while (weth_rw_get_busy(pcpu->event_w, pcpu->event_r, ectx->event_buff_size)) {
                cur_pcpu = this_cpu_ptr(&g_epcpu_ctx);
                *(cur_pcpu->event + cur_pcpu->event_w) = *(pcpu->event + pcpu->event_r);
                cur_pcpu->event_w = weth_rw_get_pos(cur_pcpu->event_w + 1, ectx->event_buff_size);
                pcpu->event_r = weth_rw_get_pos(pcpu->event_r + 1, ectx->event_buff_size);
            }
            spin_unlock_irqrestore(&pcpu->event_process_lock, flags);

            clear_bit(WETH_SMP_BIT_SCHED, &pcpu->state);
            tasklet_hi_schedule(&cur_pcpu->evt_smp_push);
            return -1;
        }
    } else {
        /* target cpu not exec complete, use default cpu */
        pcpu->state_cpu_busy++;
        tasklet_hi_schedule(&ectx->evt_smp_recall);

        return -1;
    }

    return 0;
}

STATIC void weth_event_smp_recall(unsigned long data)
{
    int i, ret;
    struct event_pcpu_ctx *pcpu = NULL;
    struct event_buff_ctx *ectx = (struct event_buff_ctx *)data;
    int event_num = 0;

    ectx->smp_recall++;

    for (i = 0; i < num_possible_cpus(); i++) {
        pcpu = &per_cpu(g_epcpu_ctx, i);

        event_num = weth_rw_get_busy(pcpu->event_w, pcpu->event_r, ectx->event_buff_size);
        if (event_num > 0) {
            pcpu->smp_recall++;
            ret = weth_event_smp_call(pcpu);
            if (ret) {
                pcpu->state_smp_recall_failed++;
            }
        }
    }
    return;
}

STATIC void weth_event_process_percpu(struct event_buff_ctx *ectx, union event_element rx_event)
{
    int ret;
    struct event_pcpu_ctx *cur_pcpu = NULL;
    unsigned long flags;

    cur_pcpu = weth_get_cpu_from_event(ectx, rx_event);
    if (weth_rw_get_idle(cur_pcpu->event_w, cur_pcpu->event_r, ectx->event_buff_size) == 0) {
        system_error(DRV_ERRNO_WETH_SKBNULL, 0, 0, NULL, 0);
        return;
    }
    /* save event to per cpu event buffer */
    spin_lock_irqsave(&cur_pcpu->event_process_lock, flags);
    *(cur_pcpu->event + cur_pcpu->event_w) = rx_event;
    cur_pcpu->event_w = weth_rw_get_pos(cur_pcpu->event_w + 1, ectx->event_buff_size);
    spin_unlock_irqrestore(&cur_pcpu->event_process_lock, flags);

    ret = weth_event_smp_call(cur_pcpu);
    if (ret) {
        cur_pcpu->state_smp_call_failed++;
    }

    return;
}

STATIC void weth_event_process(void)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    union write_read_pointer wr_rd_p = ectx->rx_event->wr_rd_p;
    unsigned int rx_wp = wr_rd_p.bits.wp;
    unsigned int rx_rp = wr_rd_p.bits.rp;
    unsigned int rx_num;

    if (rx_wp >= ectx->event_buff_size || rx_rp >= ectx->event_buff_size) {
        ectx->tx_stats.invalid_rx_rp++;
        return;
    }

    ectx->rx_stats.handle_event_start++;
    rx_num = weth_rw_get_busy(rx_wp, rx_rp, ectx->event_buff_size);

    ectx->rx_event_debug->wr_rd_p[ectx->wr_rd_index] = wr_rd_p;
    ectx->wr_rd_index = weth_rw_get_pos(ectx->wr_rd_index + 1, ectx->event_buff_size);

    while (rx_num > 0) {
        union event_element rx_event = ectx->rx_event->event[rx_rp];
        unsigned int event_type = (unsigned int)rx_event.bits.event_type;
        unsigned int index = (unsigned int)rx_event.bits.dma_desc_idx;

        ectx->rx_event_debug->event[rx_rp] = rx_event;
        if (event_type >= WETH_EVENT_TYPE_NUM) {
            ectx->rx_stats.invalid_event++;
            rx_rp = weth_rw_get_pos(rx_rp + 1, ectx->event_buff_size);
            rx_num--;
            continue;
        }

        if (index >= WETH_RC_RX_TAB_MAX_NUM) {
            ectx->rx_stats.invalid_index++;
            rx_rp = weth_rw_get_pos(rx_rp + 1, ectx->event_buff_size);
            rx_num--;
            continue;
        }

        ectx->rx_stats.event_count[event_type]++;
        ectx->rx_stats.event_handled++;

        weth_event_process_percpu(ectx, rx_event);

        /* move w bar event ptr, after copy event to per cpu event */
        rx_rp = weth_rw_get_pos(rx_rp + 1, ectx->event_buff_size);
        rx_num--;
        ectx->rx_stats.event_handled++;
    }

    ectx->rx_event->wr_rd_p.bits.rp = rx_rp;
    /* read for order */
    rx_rp = ectx->rx_event->wr_rd_p.bits.rp;

    return;
}

void weth_event_show_pcpu_info(int cpu_id)
{
    struct event_pcpu_ctx *pcpu = NULL;

    if (cpu_id >= num_possible_cpus()) {
        weth_pr_err("error cpu id: %d\n", cpu_id);
        return;
    }
    pcpu = &per_cpu(g_epcpu_ctx, cpu_id);
    weth_pr_err("cpu:                  %u\n", pcpu->cpu);
    weth_pr_err("state:                %lu\n", pcpu->state);
    weth_pr_err("evt_num:              %u\n", pcpu->evt_num);
    weth_pr_err("cpu_run_max:          %u\n", pcpu->cpu_run_max);
    weth_pr_err("qlen_weight:          %u\n", pcpu->evt_weight);
    weth_pr_err("stat_smp_call:        %u\n", pcpu->stat_smp_call);
    weth_pr_err("stat_direct_call:     %u\n", pcpu->stat_direct_call);
    weth_pr_err("stat_process_call:    %u\n", pcpu->stat_process_call);
    weth_pr_err("stat_process_end:     %u\n", pcpu->stat_process_end);
    weth_pr_err("stat_other_cpu_call:  %u\n", pcpu->stat_other_cpu_call);
    weth_pr_err("stat_event_handled:   %u\n", pcpu->stat_event_handled);
    weth_pr_err("stat_smp_call_function_fail:   %u\n", pcpu->stat_smp_call_function_fail);
    weth_pr_err("state_cpu_busy:   %u\n", pcpu->state_cpu_busy);
    weth_pr_err("state_smp_recall_failed:   %u\n", pcpu->state_smp_recall_failed);
    weth_pr_err("state_smp_call_failed:   %u\n", pcpu->state_smp_call_failed);
    weth_pr_err("smp_recall:   %u\n", pcpu->smp_recall);

    weth_pr_err("\n");

    return;
}

void weth_event_show_cpu_info(void)
{
    int i;

    weth_pr_err("cpu num:%d\n", NR_CPUS);
    weth_pr_err("cur_cpu:%d\n", g_weth_event_ctx.cur_cpu);
    weth_pr_err("stat_find_cpu_ok:%d\n", g_weth_event_ctx.stat_find_cpu_ok);
    weth_pr_err("stat_find_cpu_fail:%d\n", g_weth_event_ctx.stat_find_cpu_fail);
    weth_pr_err("stat_find_min_len_fail:%d\n", g_weth_event_ctx.stat_find_min_len_fail);
    weth_pr_err("stat_smp_call_fail:%d\n", g_weth_event_ctx.stat_smp_call_fail);
    weth_pr_err("invalid_port_no:%d\n", g_weth_event_ctx.invalid_port_no);
    weth_pr_err("valid_port_no:%d\n", g_weth_event_ctx.valid_port_no);
    weth_pr_err("extra_port_no:%d\n", g_weth_event_ctx.extra_port_no);
    weth_pr_err("smp_recall:%d\n", g_weth_event_ctx.smp_recall);
    weth_pr_err("debug_port_no:%d\n", g_weth_event_ctx.debug_port_no);
    weth_pr_err("cpu_not_online:%d\n", g_weth_event_ctx.cpu_not_online);

    for (i = 0; i < NR_CPUS; i++) {
        weth_event_show_pcpu_info(i);
    }
}

STATIC irqreturn_t weth_event_handler(int index, void *para)
{
    struct common_info *comm_info = &g_common_info;

    if (unlikely(!comm_info->link_status)) {
        if (comm_info->init_finish) {
            weth_pr_err("link is not up\n");
            comm_info->link_down_rx++;
            return IRQ_HANDLED;
        }
    }

    weth_event_process();

    return IRQ_HANDLED;
}

int weth_send_event_by_port(int port_no, unsigned int num, unsigned int dma_desc_idx, unsigned int event_type,
                            int send_irq)
{
    struct common_info *comm_info = &g_common_info;
    int ret;

    atomic_add(1, &comm_info->bar_access);
    if (unlikely(!comm_info->link_status)) {
        if (comm_info->init_finish) {
            comm_info->link_down_tx++;
            atomic_sub(1, &comm_info->bar_access);
            return -1;
        }
    }

    ret = weth_process_send_event(port_no, num, dma_desc_idx, event_type, send_irq);
    atomic_sub(1, &comm_info->bar_access);

    return ret;
}

void weth_show_pcpu_rx_packets_qlen(void)
{
    int i;

    for (i = 0; i < NR_CPUS; i++) {
        weth_pr_err("rx_packets_per_cpu[%d]: %d\n", i, g_common_ctx.rx_packets_per_cpu[i]);
    }
}

#else
STATIC int weth_event_init_pcpu(struct event_buff_ctx *ectx)
{
    return 0;
}

STATIC void weth_event_deinit_pcpu(void)
{
    return;
}

STATIC void weth_event_process(void)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    union write_read_pointer wr_rd_p = ectx->rx_event->wr_rd_p;
    unsigned int rx_wp = wr_rd_p.bits.wp;
    unsigned int rx_rp = wr_rd_p.bits.rp;
    unsigned int rx_num;

    if ((rx_wp >= ectx->event_buff_size) || rx_rp >= ectx->event_buff_size) {
        ectx->rx_stats.invalid_event++;
        return;
    }

    ectx->rx_stats.handle_event_start++;
    rx_num = weth_rw_get_busy(rx_wp, rx_rp, ectx->event_buff_size);

    while (rx_num > 0) {
        union event_element rx_event = ectx->rx_event->event[rx_rp];
        unsigned int event_type = (unsigned int)rx_event.bits.event_type;
        if (event_type >= WETH_EVENT_TYPE_NUM) {
            ectx->rx_stats.invalid_event++;
            rx_rp = weth_rw_get_pos(rx_rp + 1, ectx->event_buff_size);
            rx_num--;
            continue;
        }
        ectx->rx_stats.event_count[event_type]++;
        ectx->event_cb[event_type]((unsigned int)rx_event.bits.dma_desc_idx, (unsigned int)rx_event.bits.num_per_table);

        rx_rp = weth_rw_get_pos(rx_rp + 1, ectx->event_buff_size);
        rx_num--;
        ectx->rx_stats.event_handled++;
    }

    ectx->rx_event->wr_rd_p.bits.rp = rx_rp;
    /* read for order */
    rx_rp = ectx->rx_event->wr_rd_p.bits.rp;

    return;
}

STATIC irqreturn_t weth_event_handler(int index, void *para)
{
    weth_event_process();
    return IRQ_HANDLED;
}

int weth_send_event_by_port(int port_no, unsigned int num, unsigned int dma_desc_idx, unsigned int event_type,
                            int send_irq)
{
    return weth_process_send_event(port_no, num, dma_desc_idx, event_type, send_irq);
}
#endif

int weth_send_event(unsigned int num, unsigned int dma_desc_idx, unsigned int event_type, int send_irq)
{
    return weth_send_event_by_port(WETH_INVALID_PORT_NO, num, dma_desc_idx, event_type, send_irq);
}

STATIC void weth_send_irq(struct event_buff_ctx *ectx)
{
#ifdef CONFIG_WETH_DEV
    bsp_pcie_ep_send_msi(PCIE_EP_MSI_ETH_DEV);
#endif

#ifdef CONFIG_WETH
    bsp_pcie_rc_send_msi(PCIE_RC_MSI_ETH_DEV);
#endif

    return;
}

int weth_process_send_event(int port_no, unsigned int num, unsigned int dma_desc_idx, unsigned int event_type,
                            int send_irq)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    union write_read_pointer wr_rd_p;
    unsigned int tx_wp;
    unsigned int tx_rp;
    unsigned long flags;

    ectx->tx_stats.send_event++;

    spin_lock_irqsave(&ectx->tx_lock, flags);
    wr_rd_p = ectx->tx_event->wr_rd_p;
    tx_wp = wr_rd_p.bits.wp;
    tx_rp = wr_rd_p.bits.rp;

    if (tx_wp >= ectx->event_buff_size || tx_wp < 0) {
        spin_unlock_irqrestore(&ectx->tx_lock, flags);
        ectx->tx_stats.invalid_tx_wp++;
        return -1;
    }

    if (weth_rw_get_idle(tx_wp, tx_rp, ectx->event_buff_size) <= 0) {
        weth_pr_err("event_buffer full, w:%d, r:%d, size:%d, num:%d, idx:%d, type:%d\n", tx_wp, tx_rp,
                    ectx->event_buff_size, (int)num, (int)dma_desc_idx, (int)event_type);

        spin_unlock_irqrestore(&ectx->tx_lock, flags);
        ectx->tx_stats.event_buffer_full++;
        return -1;
    }

    if (weth_rw_get_idle(tx_wp, tx_rp, ectx->event_buff_size) <= 1) {
        weth_pr_err("event_buffer full\n");
        ectx->tx_stats.event_buffer_nearly_full++;
        send_irq = 1;
    }

    ectx->tx_event->event[tx_wp].bit32 =
        (num << (TYPE_LEN + PORT_NO_LEN + INDEX_LEN)) |
        ((dma_desc_idx << (TYPE_LEN + PORT_NO_LEN)) & MASK(TYPE_LEN + PORT_NO_LEN + INDEX_LEN)) |
        (((unsigned int)port_no << TYPE_LEN) & MASK(TYPE_LEN + PORT_NO_LEN)) | (event_type & MASK(TYPE_LEN));

    ectx->tx_event->wr_rd_p.bits.wp = weth_rw_get_pos(tx_wp + 1, ectx->event_buff_size);
    ectx->tx_stats.event_count[event_type]++;
    spin_unlock_irqrestore(&ectx->tx_lock, flags);

    if (send_irq) {
        /* read for order */
        wr_rd_p = ectx->tx_event->wr_rd_p;
        weth_send_irq(ectx);
        ectx->tx_stats.send_irq++;
    }

    return 0;
}

void weth_event_buff_set_cb(weth_event_cb_t ops[])
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    int i;

    for (i = 0; i < WETH_EVENT_TYPE_NUM; i++) {
        ectx->event_cb[i] = ops[i];
    }
}

STATIC void weth_event_buff_msi_init(struct event_buff_ctx *ectx)
{
#ifdef CONFIG_WETH_DEV
    bsp_pcie_ep_msi_request(PCIE_RC_MSI_ETH_DEV, weth_event_handler, "weth_hal_ep_irq", ectx);
#endif

#ifdef CONFIG_WETH
    bsp_pcie_rc_msi_request(PCIE_EP_MSI_ETH_DEV, weth_event_handler, "weth_hal_rc_irq", ectx);
#endif

    return;
}

STATIC void weth_event_buff_msi_deinit(struct event_buff_ctx *ectx)
{
#ifdef CONFIG_WETH_DEV
    bsp_pcie_ep_msi_free(PCIE_RC_MSI_ETH_DEV);
#endif

#ifdef CONFIG_WETH
    bsp_pcie_rc_msi_free(PCIE_EP_MSI_ETH_DEV);
#endif

    return;
}

int weth_event_get_status(void)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    union write_read_pointer wr_rd_p;
    unsigned int tx_wp;
    unsigned int tx_rp;
    unsigned long flags;

    ectx->tx_stats.send_event++;

    spin_lock_irqsave(&ectx->tx_lock, flags);
    wr_rd_p = ectx->tx_event->wr_rd_p;
    tx_wp = wr_rd_p.bits.wp;
    tx_rp = wr_rd_p.bits.rp;
    spin_unlock_irqrestore(&ectx->tx_lock, flags);

    return weth_rw_get_busy(tx_wp, tx_rp, ectx->event_buff_size);
}

void weth_event_buff_reinit(struct event_buffer *tx_event, struct event_buffer *rx_event)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    ectx->tx_event = tx_event;
    ectx->rx_event = rx_event;
    weth_event_buff_msi_init(ectx);
}

void weth_event_buff_exit(void)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    weth_event_buff_msi_deinit(ectx);
}

int weth_event_buff_init(enum weth_pcie_mode work_mode, struct event_buffer *tx_event, struct event_buffer *rx_event)
{
    struct event_buff_ctx *ectx = &g_weth_event_ctx;
    int ret;

    ectx->work_mode = work_mode;
    ectx->event_buff_size = MAX_EVENT_NUM;
    ectx->handle_event_buff_size = MAX_HANDLE_EVENT_NUM;
    ectx->pcpu_packets_thru = MAX_QLEN_PCPU;
    spin_lock_init(&ectx->tx_lock);
    spin_lock_init(&ectx->rx_lock);
    ectx->tx_event = tx_event;
    ectx->rx_event = rx_event;
#ifdef CONFIG_WETH_SMP_EVENT
    tasklet_init(&ectx->evt_smp_recall, weth_event_smp_recall, (uintptr_t)ectx);
    ectx->wr_rd_index = 0;
    ectx->rx_event_debug = (struct event_buffer_debug *)kzalloc(sizeof(struct event_buffer_debug), GFP_KERNEL);
    if (ectx->rx_event_debug == NULL) {
        weth_pr_err("rx_event_debug alloc failed\n");
        goto err;
    }
    ectx->low_perf = 1;
#endif
    ret = weth_event_init_pcpu(ectx);
    if (ret) {
        goto err;
    }
    weth_event_buff_msi_init(ectx);
    weth_pr_err("event buffer init ok\n");
    return 0;
err:
    weth_event_buff_msi_deinit(ectx);
    weth_event_deinit_pcpu();
    weth_pr_err("event buffer init fail\n");
    return -1;
}
