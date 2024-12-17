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
 * Description: PCIE IOT Debug function for recording pcie running trace
 */
#include "bsp_dump.h"
#include "bsp_slice.h"
#include "pcie_iot_dev.h"
#include "pcie_iot_debug.h"
#include "securec.h"

#define PRINT_BUFF_SIZE 400

struct pcie_dbg_info *g_pcie_dbg;
int g_pcie_debug_init_done = 0;

void pcie_pm_vote_dbg(unsigned int user_id, enum vote_action action)
{
    struct pcie_lock_dbg *lock_dbg = NULL;
    struct pcie_lock_debug *debug = NULL;
    struct pcie_lock_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int try_time;
    unsigned int force_time;
    unsigned int timeout_time;
    unsigned int time_temp;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        lock_dbg = &(g_pcie_dbg->lock_dbg);
        debug = &(lock_dbg->debug[user_id]);
        timecnt = &(debug->timecnt[action]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        if (action == UNLOCK) {
            try_time = debug->timecnt[TRY_LOCK].time;
            force_time = debug->timecnt[FORCE_LOCK].time;
            timeout_time = debug->timecnt[TIMEOUT_LOCK].time;

            time_temp = (try_time > force_time) ? try_time : force_time;
            time_temp = (time_temp > timeout_time) ? time_temp : timeout_time;

            debug->lock_time_cost += (timestamp - time_temp);
        }

        i = lock_dbg->trace_index % PCIE_DBG_LOCK_TRACE_MAX;
        trace = &(lock_dbg->trace[i]);
        lock_dbg->trace_index++;

        trace->timestamp = timestamp;
        trace->user_id = user_id;
        trace->act_id = action;
    }
}
void pcie_pm_state_dbg(unsigned int state_type, unsigned int state_id)
{
    struct pcie_state_dbg *state_dbg = NULL;
    struct pcie_state_debug *debug = NULL;
    struct pcie_state_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        state_dbg = &(g_pcie_dbg->state_dbg);
        debug = &(state_dbg->debug[state_type]);
        timecnt = &(debug->timecnt[state_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = state_dbg->trace_index % PCIE_DBG_STATE_TRACE_MAX;
        trace = &(state_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->state_id = state_id;
        trace->state_type = state_type;
        state_dbg->trace_index++;
    }
}
void pcie_pm_event_dbg(unsigned int event_id)
{
    struct pcie_event_dbg *event_dbg = NULL;
    struct pcie_event_debug *debug = NULL;
    struct pcie_event_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        event_dbg = &(g_pcie_dbg->event_dbg);

        debug = &(event_dbg->debug[event_id]);
        timecnt = &(debug->timecnt);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = event_dbg->trace_index % PCIE_DBG_EVENT_TRACE_MAX;
        trace = &(event_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->event_id = event_id;
        event_dbg->trace_index++;
    }
}
void pcie_pm_gpio_dbg(unsigned int enum_id, enum gpio_action act_id)
{
    struct pcie_gpio_dbg *gpio_dbg = NULL;
    struct pcie_gpio_debug *debug = NULL;
    struct pcie_gpio_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        gpio_dbg = &(g_pcie_dbg->gpio_dbg);
        debug = &(gpio_dbg->debug[enum_id]);
        timecnt = &(debug->timecnt[act_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = gpio_dbg->trace_index % PCIE_DBG_GPIO_TRACE_MAX;
        trace = &(gpio_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->enum_id = enum_id;
        trace->act_id = act_id;
        gpio_dbg->trace_index++;
    }
}
void pcie_msi_dbg(unsigned int msi_id, enum msi_type type_id, enum msi_action act_id)
{
    struct pcie_msi_dbg *msi_dbg = NULL;
    struct pcie_msi_debug *debug = NULL;
    struct pcie_msi_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        msi_dbg = &(g_pcie_dbg->msi_dbg);
        debug = &(msi_dbg->debug[msi_id]);
        timecnt = &(debug->timecnt[type_id][act_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = msi_dbg->trace_index % PCIE_DBG_MSI_TRACE_MAX;
        trace = &(msi_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->msi_id = msi_id;
        trace->type_id = type_id;
        trace->act_id = act_id;
        msi_dbg->trace_index++;
    }
}

void pcie_pm_msg_dbg(unsigned int msg_id, enum msg_action act_id)
{
    struct pcie_msg_dbg *msg_dbg = NULL;
    struct pcie_msg_debug *debug = NULL;
    struct pcie_msg_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        msg_dbg = &(g_pcie_dbg->msg_dbg);
        debug = &(msg_dbg->debug[msg_id]);
        timecnt = &(debug->timecnt[act_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = msg_dbg->trace_index % PCIE_DBG_MSG_TRACE_MAX;
        trace = &(msg_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->msg_id = msg_id;
        trace->act_id = act_id;
        msg_dbg->trace_index++;
    }
}

void pcie_wakelock_dbg(unsigned int user_id, enum wakelock_action act_id)
{
    struct pcie_wakelock_dbg *wakelock_dbg = NULL;
    struct pcie_wakelock_debug *debug = NULL;
    struct pcie_wakelock_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        wakelock_dbg = &(g_pcie_dbg->wakelock_dbg);
        debug = &(wakelock_dbg->debug[user_id]);
        timecnt = &(debug->timecnt[act_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = wakelock_dbg->trace_index % PCIE_DBG_WAKELOCK_TRACE_MAX;
        trace = &(wakelock_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->user_id = user_id;
        trace->act_id = act_id;
        wakelock_dbg->trace_index++;
    }
}
void pcie_pm_vote_edge_dbg(unsigned int user_id, enum edge_type type_id)
{
    struct pcie_vote_edge_dbg *vote_edge_dbg = NULL;
    struct pcie_vote_edge_debug *debug = NULL;
    struct pcie_vote_edge_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        vote_edge_dbg = &(g_pcie_dbg->vote_edge_dbg);
        debug = &(vote_edge_dbg->debug[user_id]);
        timecnt = &(debug->timecnt[type_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = vote_edge_dbg->trace_index % PCIE_DBG_VOTE_EDGE_TRACE_MAX;
        trace = &(vote_edge_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->user_id = user_id;
        trace->type_id = type_id;
        vote_edge_dbg->trace_index++;
    }
}

void pcie_bar_dbg(unsigned int bar_id, unsigned int bar_size, unsigned long bar_addr)
{
    struct pcie_bar_debug *debug = NULL;

    if (g_pcie_dbg != NULL) {
        debug = &(g_pcie_dbg->bar_dbg.debug[bar_id]);

        debug->bar_addr = bar_addr;
        debug->bar_size = bar_size;
    }
}

void pcie_dma_dbg(unsigned int chn_id, enum dma_type type_id, enum dma_action act_id)
{
    struct pcie_dma_dbg *dma_dbg = NULL;
    struct pcie_dma_debug *debug = NULL;
    struct pcie_dma_trace *trace = NULL;
    struct timecnt_dbg *timecnt = NULL;
    unsigned int timestamp = bsp_get_slice_value();
    int i;

    if (g_pcie_dbg != NULL) {
        dma_dbg = &(g_pcie_dbg->dma_dbg);

        debug = &(dma_dbg->debug[chn_id]);
        timecnt = &(debug->timecnt[type_id][act_id]);
        timecnt->cnt++;
        timecnt->time = timestamp;

        i = dma_dbg->trace_index % PCIE_DBG_DMA_TRACE_MAX;
        trace = &(dma_dbg->trace[i]);
        trace->timestamp = timestamp;
        trace->chn_id = chn_id;
        trace->type_id = type_id;
        trace->act_id = act_id;
        dma_dbg->trace_index++;
    }
}
void bsp_pcie_boot_dbg(unsigned int boot_stage)
{
    unsigned int timestamp = bsp_get_slice_value();

    if (g_pcie_dbg != NULL) {
        g_pcie_dbg->boot_dbg.debug[boot_stage].timestamp = timestamp;
    }
}

void pcie_callback_dbg(unsigned int user_id, unsigned int cb_stage, unsigned int cb_result)
{
    struct pcie_cb_stage *stage = NULL;
    unsigned int timestamp = bsp_get_slice_value();

    if (g_pcie_dbg != NULL) {
        stage = &(g_pcie_dbg->cb_dbg.debug[user_id].stage[cb_stage]);
        stage->timestamp = timestamp;
        stage->cb_result = cb_result;
    }
}

void pcie_config_space_dbg(enum cfg_type type, unsigned int func_num, unsigned int reg_num, unsigned int reg_val)
{
    struct pcie_config_space *cfg_dbg = NULL;

    if (func_num >= PCIE_FUNC_NUM || reg_num >= PCIE_CFG_REG_NUM) {
        return;
    }

    if (g_pcie_dbg != NULL) {
        cfg_dbg = &(g_pcie_dbg->cfg_dbg.ep_config_space[type]);
        cfg_dbg->cfg_space[func_num][reg_num] = reg_val;
    }
}

int pcie_check_restored_config_space_dbg(void)
{
    int ret = 0;
    struct pcie_config_space *save_cfg = NULL;
    struct pcie_config_space *restore_cfg = NULL;
    if (g_pcie_dbg != NULL) {
        save_cfg = &(g_pcie_dbg->cfg_dbg.ep_config_space[SAVE_CONFIG]);
        restore_cfg = &(g_pcie_dbg->cfg_dbg.ep_config_space[RESTORE_CONFIG]);
        ret = memcmp(save_cfg, restore_cfg, PCIE_CONFIG_SPACE_SIZE);
    }
    return ret;
}

void pcie_dma_addr_dbg(struct pcie_dma_transfer_info *transfer_info)
{
    int i;
    struct pcie_dma_addr_dbg *addr_dbg = NULL;
    struct pcie_dma_addr_trace *trace = NULL;
    unsigned int timestamp = bsp_get_slice_value();

    if (g_pcie_dbg != NULL) {
        addr_dbg = &(g_pcie_dbg->dma_addr_dbg);
        i = addr_dbg->trace_index % PCIE_DBG_DMA_ADDR_TRACE_MAX;
        trace = &(addr_dbg->trans_info[i]);
        trace->timestamp = timestamp;
        trace->channel_id = transfer_info->channel;
        trace->direction = transfer_info->direction;
        trace->transfer_type = transfer_info->transfer_type;
        if (trace->transfer_type == PCIE_DMA_NORMAL_MODE) {
            trace->transfer_size = transfer_info->element.transfer_size;
            trace->sar_low = transfer_info->element.sar_low;
            trace->sar_high = transfer_info->element.sar_high;
            trace->dar_low = transfer_info->element.dar_low;
            trace->dar_high = transfer_info->element.dar_high;
            trace->dma_lli_addr = 0;
        } else if (trace->transfer_type == PCIE_DMA_LINK_MODE) {
            trace->transfer_size = 0;
            trace->sar_low = 0;
            trace->sar_high = 0;
            trace->dar_low = 0;
            trace->dar_high = 0;
            trace->dma_lli_addr = transfer_info->dma_lli_addr;
        }
        addr_dbg->trace_index++;
    }
}

void print_last_msi_info(const char *name, enum msi_type type_id, enum msi_action act_id)
{
    u32 i;
    u32 cnt;
    char print_buf[PRINT_BUFF_SIZE] = {0};
    u32 last_user_id;
    unsigned int last_action_time = 0;

    if (strlen(name) >= PRINT_BUFF_SIZE) {
        PCIE_DBG_TRACE("invalid input name");
        return;
    }
    cnt += snprintf_s((char *)print_buf, (size_t)PRINT_BUFF_SIZE, (size_t)(PRINT_BUFF_SIZE - 1), name);
    for (i = 0; i < PCIE_DBG_MSI_DEBUG_MAX; i++) {
        cnt += snprintf_s((char *)print_buf + cnt, (size_t)(PRINT_BUFF_SIZE - cnt), (size_t)(PRINT_BUFF_SIZE - cnt - 1),
                          "%d,", g_pcie_dbg->msi_dbg.debug[i].timecnt[type_id][act_id].cnt);
    }
    cnt += snprintf_s((char *)print_buf + cnt, (size_t)(PRINT_BUFF_SIZE - cnt), (size_t)(PRINT_BUFF_SIZE - cnt - 1),
                      "\n");
    printk(KERN_ERR "%s", print_buf);

    for (i = 0; i < PCIE_DBG_MSI_DEBUG_MAX; i++) {
        if (g_pcie_dbg->msi_dbg.debug[i].timecnt[type_id][act_id].time > last_action_time) {
            last_action_time = g_pcie_dbg->msi_dbg.debug[i].timecnt[type_id][act_id].time;
            last_user_id = i;
        }
    }

    printk(KERN_ERR "last irq user_id: %d, timestamp: 0x%x\n", last_user_id, last_action_time);
    return;
}

void print_last_vote_info(const char *name, enum vote_action action)
{
    u32 i;
    u32 cnt = 0;
    char print_buf[PRINT_BUFF_SIZE] = {0};
    u32 last_user_id;
    unsigned int last_action_time = 0;

    if (strlen(name) >= PRINT_BUFF_SIZE) {
        PCIE_DBG_TRACE("invalid input name");
        return;
    }
    cnt += snprintf_s((char *)print_buf, (size_t)PRINT_BUFF_SIZE, (size_t)(PRINT_BUFF_SIZE - 1), name);
    for (i = 0; i < PCIE_USER_NUM; i++) {
        cnt += snprintf_s((char *)print_buf + cnt, (size_t)(PRINT_BUFF_SIZE - cnt), (size_t)(PRINT_BUFF_SIZE - cnt - 1),
                          "%d,", g_pcie_dbg->lock_dbg.debug[i].timecnt[action].cnt);
    }
    cnt += snprintf_s((char *)print_buf + cnt, (size_t)(PRINT_BUFF_SIZE - cnt), (size_t)(PRINT_BUFF_SIZE - cnt - 1),
                      "\n");
    printk(KERN_ERR "%s", print_buf);

    for (i = 0; i < PCIE_USER_NUM; i++) {
        if (g_pcie_dbg->lock_dbg.debug[i].timecnt[action].time > last_action_time) {
            last_action_time = g_pcie_dbg->lock_dbg.debug[i].timecnt[action].time;
            last_user_id = i;
        }
    }

    printk(KERN_ERR "last user_id: %d, timestamp: 0x%x\n", last_user_id, last_action_time);
    return;
}

void print_last_pm_state_info(enum pcie_pm_state_type state_type)
{
    int i;
    u32 last_state_id;
    unsigned int last_action_time = 0;

    for (i = 0; i < PCIE_PM_STATE_MAX_NUM; i++) {
        if (g_pcie_dbg->state_dbg.debug[state_type].timecnt[i].time > last_action_time) {
            last_action_time = g_pcie_dbg->state_dbg.debug[state_type].timecnt[i].time;
            last_state_id = i;
        }
    }

    printk(KERN_ERR "state type: %d, last state id : %d, timestamp: 0x%x\n", state_type, last_state_id,
           last_action_time);
}

void bsp_pcie_print_last_status(void)
{
    if (g_pcie_dbg != NULL) {
        printk(KERN_ERR "-----------PCIE_DUMP START-----------\n");

        /* ---------last msi info---------- */
        print_last_msi_info("send msi cnt:      ", MSI_SEND, MSI_NA);
        print_last_msi_info("read msi in cnt:   ", MSI_READ, MSI_IN);
        print_last_msi_info("read msi out cnt: ", MSI_READ, MSI_OUT);
        /* ---------last pcie pm vote info---------- */
        print_last_vote_info("pm try lock cnt:        ", TRY_LOCK);
        print_last_vote_info("pm force lock cnt:     ", FORCE_LOCK);
        print_last_vote_info("pm timeout lock cnt: ", TIMEOUT_LOCK);
        print_last_vote_info("pm unlock cnt:          ", UNLOCK);
        print_last_vote_info("pm err lock cnt:         ", ERR_LOCK);
        /* ---------last pcie pm state info---------- */
        print_last_pm_state_info(CURRENT_STATE);
        print_last_pm_state_info(INNER_EVENT);
        print_last_pm_state_info(OUTER_EVENT);

        printk(KERN_ERR "-----------PCIE_DUMP END-----------\n");
    }
}

#define PCIE_OM_DUMP_SIZE 0x3C00
#define PCIE_OM_DUMP_ID DUMP_MODEMAP_PCIE_RC

int bsp_pcie_dump_init(void)
{
    int ret = 0;
    int pcie_dbg_size = sizeof(struct pcie_dbg_info);

    if (pcie_dbg_size > PCIE_OM_DUMP_SIZE) {
        PCIE_DBG_TRACE("pcie debug size too big!\n");
        return -EINVAL;
    }
    if (!g_pcie_debug_init_done) {
        g_pcie_dbg = (struct pcie_dbg_info *)bsp_dump_register_field(PCIE_OM_DUMP_ID, "PCIE_RC_DBG",
                                                                     pcie_dbg_size, 1);
        if (g_pcie_dbg == NULL) {
            PCIE_DBG_TRACE("malloc for dump fail,malloc form ddr!\n");
            g_pcie_dbg = (struct pcie_dbg_info *)kzalloc(pcie_dbg_size, GFP_KERNEL);
            if (g_pcie_dbg == NULL) {
                PCIE_DBG_TRACE("malloc for ddr fail,error!\n");
                goto err_ret; /*lint !e801 */
            }
        }
        PCIE_DBG_TRACE("pcie dump init ok,size is 0x%x!\n", pcie_dbg_size);
        (void)bsp_dump_register_hook("rc_pcie", bsp_pcie_print_last_status);
        g_pcie_debug_init_done = 1;
    }

    if (g_pcie_dbg != NULL) {
        ret = memset_s(g_pcie_dbg, pcie_dbg_size, 0, pcie_dbg_size);
        g_pcie_dbg->dbg_start_byte = 0x52525252;  // special start byte for rc dump index
        PCIE_DBG_TRACE("pcie dump memset done, ret %d!\n", ret);
    }

    return ret;

err_ret:
    PCIE_DBG_TRACE("pcie dump init fail,can not debug!\n");
    return -ENOSPC;
}
