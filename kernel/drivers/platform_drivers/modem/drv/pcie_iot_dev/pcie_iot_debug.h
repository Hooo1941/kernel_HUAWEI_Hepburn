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
 * Description: PCIE IOT Debug Driver Header
 */
#include <bsp_pcie.h>
/* ****************************************lock dbg start*************************************** */
enum vote_action {
    TRY_LOCK,
    FORCE_LOCK,
    TIMEOUT_LOCK,
    UNLOCK,
    ERR_LOCK,
    LOCK_MAX,
};
/* 投票动态缓冲区 */
struct pcie_lock_trace {
    unsigned int timestamp;
    unsigned short user_id;
    unsigned short act_id;
};
/* 投票静态缓冲区 */
struct timecnt_dbg {
    unsigned int cnt;
    unsigned int time;
};
struct pcie_lock_debug {
    struct timecnt_dbg timecnt[LOCK_MAX];
    unsigned int lock_time_cost;
};
#define PCIE_DBG_LOCK_TRACE_MAX 0x20
#define PCIE_DBG_LOCK_DEBUG_MAX PCIE_USER_NUM
struct pcie_lock_dbg {
    struct pcie_lock_debug debug[PCIE_DBG_LOCK_DEBUG_MAX];
    struct pcie_lock_trace trace[PCIE_DBG_LOCK_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_pm_vote_dbg(unsigned int user_id, enum vote_action action);
/* *************************************lock dbg end**************************************** */
/* **********************************state_machine dbg start******************************** */
/* 状态机动态缓冲区 */
struct pcie_state_trace {
    unsigned int timestamp;
    unsigned short state_id;
    unsigned short state_type;
};
/* 状态机静态缓冲区 */
struct pcie_state_debug {
    struct timecnt_dbg timecnt[PCIE_PM_STATE_MAX_NUM];
};
#define PCIE_DBG_STATE_TRACE_MAX 0x20
#define PCIE_DBG_STATE_DEBUG_MAX PCIE_PM_STATE_TYPE_MAX
struct pcie_state_dbg {
    struct pcie_state_debug debug[PCIE_DBG_STATE_DEBUG_MAX];
    struct pcie_state_trace trace[PCIE_DBG_STATE_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_pm_state_dbg(unsigned int state_type, unsigned int state_id);
/* 事件动态缓冲区 */
struct pcie_event_trace {
    unsigned int timestamp;
    unsigned int event_id;
};
/* 事件静态缓冲区 */
struct pcie_event_debug {
    struct timecnt_dbg timecnt;
};
#define PCIE_DBG_EVENT_TRACE_MAX 0x20
#define PCIE_DBG_EVENT_DEBUG_MAX PCIE_PM_EVENT_MAX
struct pcie_event_dbg {
    struct pcie_event_debug debug[PCIE_DBG_EVENT_DEBUG_MAX];
    struct pcie_event_trace trace[PCIE_DBG_EVENT_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_pm_event_dbg(unsigned int event_id);
/* **************************state_machine end start************************* */
/* **************************gpio dbg start************************* */
enum gpio_action {
    GPIO_UP,
    GPIO_DOWN,
    GPIO_RECEIVE,
    GPIO_ACTMAX,
};
/* GPIO动态缓冲区 */
struct pcie_gpio_trace {
    unsigned int timestamp;
    unsigned short enum_id;
    unsigned short act_id;
};
/* GPIO静态缓冲区 */
struct pcie_gpio_debug {
    struct timecnt_dbg timecnt[GPIO_ACTMAX];
};
#define PCIE_DBG_GPIO_TRACE_MAX 0x20
#define PCIE_DBG_GPIO_DEBUG_MAX PCIE_PM_GPIO_MAX_NUM
struct pcie_gpio_dbg {
    struct pcie_gpio_debug debug[PCIE_DBG_GPIO_DEBUG_MAX];
    struct pcie_gpio_trace trace[PCIE_DBG_GPIO_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_pm_gpio_dbg(unsigned int enum_id, enum gpio_action act_id);
/* **************************gpio dbg end************************* */
/* **************************msi dbg start************************* */
enum msi_type {
    MSI_SEND,
    MSI_READ,
    MSI_TYPE_MAX,
};
enum msi_action {
    MSI_IN,
    MSI_OUT,
    MSI_NA,
    MSI_ACT_MAX,
};
/* MSI动态缓冲区 */
struct pcie_msi_trace {
    unsigned int timestamp;
    unsigned short msi_id;
    unsigned short type_id;
    unsigned short act_id;
    unsigned short reserved;
};
/* MSI静态缓冲区 */
struct pcie_msi_debug {
    struct timecnt_dbg timecnt[MSI_TYPE_MAX][MSI_ACT_MAX];
};
#define PCIE_DBG_MSI_TRACE_MAX 0x20
#define PCIE_DBG_MSI_DEBUG_MAX ((PCIE_RC_MSI_NUM > PCIE_EP_MSI_NUM) ? PCIE_RC_MSI_NUM : PCIE_EP_MSI_NUM)
struct pcie_msi_dbg {
    struct pcie_msi_debug debug[PCIE_DBG_MSI_DEBUG_MAX];
    struct pcie_msi_trace trace[PCIE_DBG_MSI_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_msi_dbg(unsigned int msi_id, enum msi_type type_id, enum msi_action act_id);
/* ****************************msi dbg end**************************** */
/* **************************msg dbg start************************* */
enum msg_action {
    MSG_SEND,
    MSG_READ,
    MSG_ACTMAX,
};
/* GPIO动态缓冲区 */
struct pcie_msg_trace {
    unsigned int timestamp;
    unsigned short msg_id;
    unsigned short act_id;
};
/* GPIO静态缓冲区 */
struct pcie_msg_debug {
    struct timecnt_dbg timecnt[MSG_ACTMAX];
};
#define PCIE_DBG_MSG_TRACE_MAX 0x20
#define PCIE_DBG_MSG_DEBUG_MAX \
    (((int)RC_PCIE_PM_MSG_NUM > (int)EP_PCIE_PM_MSG_NUM) ? RC_PCIE_PM_MSG_NUM : EP_PCIE_PM_MSG_NUM)
struct pcie_msg_dbg {
    struct pcie_msg_debug debug[PCIE_DBG_MSG_DEBUG_MAX];
    struct pcie_msg_trace trace[PCIE_DBG_MSG_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_pm_msg_dbg(unsigned int msg_id, enum msg_action act_id);
/* ****************************msg dbg end**************************** */
/* **************************wakelock dbg start************************* */
enum wakelock_action {
    WAKE_LOCK,
    WAKE_UNLOCK,
    WAKELOCK_ACTION_MAX,
};
/* WAKELOCK动态缓冲区 */
struct pcie_wakelock_trace {
    unsigned int timestamp;
    unsigned short user_id;
    unsigned short act_id;
};
/* WAKELOCK静态缓冲区 */
struct pcie_wakelock_debug {
    struct timecnt_dbg timecnt[WAKELOCK_ACTION_MAX];
};
#define PCIE_DBG_WAKELOCK_TRACE_MAX 0x20
#define PCIE_DBG_WAKELOCK_DEBUG_MAX PCIE_WAKELOCK_ID_MAX
struct pcie_wakelock_dbg {
    struct pcie_wakelock_debug debug[PCIE_DBG_WAKELOCK_DEBUG_MAX];
    struct pcie_wakelock_trace trace[PCIE_DBG_WAKELOCK_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_wakelock_dbg(unsigned int user_id, enum wakelock_action act_id);
/* **************************wakeup dbg end************************* */
/* *********************vote_edge dbg start*********************** */
enum edge_type {
    FIRST_LOCK,
    LAST_UNLOCK,
    VOTE_EDGE_MAX,
};
/* 首包唤醒末包睡眠动态缓冲区 */
struct pcie_vote_edge_trace {
    unsigned int timestamp;
    unsigned short type_id;
    unsigned short user_id;
};
/* 首包唤醒末包睡眠静态缓冲区 */
struct pcie_vote_edge_debug {
    struct timecnt_dbg timecnt[VOTE_EDGE_MAX];
};
#define PCIE_DBG_VOTE_EDGE_TRACE_MAX 0x20
#define PCIE_DBG_VOTE_EDGE_DEBUG_MAX PCIE_USER_NUM
struct pcie_vote_edge_dbg {
    struct pcie_vote_edge_debug debug[PCIE_DBG_VOTE_EDGE_DEBUG_MAX];
    struct pcie_vote_edge_trace trace[PCIE_DBG_VOTE_EDGE_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_pm_vote_edge_dbg(unsigned int user_id, enum edge_type type_id);
/* *********************vote_edge dbg end*********************** */
/* *********************bar dbg start*********************** */
/* bar静态缓冲区 */
struct pcie_bar_debug {
    unsigned long bar_addr;
    unsigned int bar_size;
};
#define PCIE_DBG_BAR_DEBUG_MAX PCIE_BAR_MAX_NUM
struct pcie_bar_dbg {
    struct pcie_bar_debug debug[PCIE_DBG_BAR_DEBUG_MAX];
};
extern void pcie_bar_dbg(unsigned int bar_id, unsigned int bar_size, unsigned long bar_addr);
/* *****************************bar dbg end******************************* */
/* ***************************dma dbg start***************************** */
enum dma_type {
    PCIE_DMA_WRITE,
    PCIE_DMA_READ,
    PCIE_DMA_WRITE_IRQ,
    PCIE_DMA_READ_IRQ,
    PCIE_DMA_TYPE_MAX,
};
enum dma_action {
    PCIE_DMA_CB_IN,
    PCIE_DMA_CB_OUT,
    PCIE_DMA_NA,
    PCIE_DMA_ACT_MAX,
};
/* dma动态缓冲区 */
struct pcie_dma_trace {
    unsigned int timestamp;
    unsigned short chn_id;
    unsigned short type_id;
    unsigned short act_id;
    unsigned short reserved;
};
/* dma静态缓冲区 */
struct pcie_dma_debug {
    struct timecnt_dbg timecnt[PCIE_DMA_TYPE_MAX][PCIE_DMA_ACT_MAX];
};
#define PCIE_DBG_DMA_DEBUG_MAX PCIE_DMA_CHN_NUM
#define PCIE_DBG_DMA_TRACE_MAX 0x20
struct pcie_dma_dbg {
    struct pcie_dma_debug debug[PCIE_DBG_DMA_DEBUG_MAX];
    struct pcie_dma_trace trace[PCIE_DBG_DMA_TRACE_MAX];
    unsigned int trace_index;
};
extern void pcie_dma_dbg(unsigned int chn_id, enum dma_type type_id, enum dma_action act_id);
/* ***********************************dma dbg end************************************* */
/* ***********************************boot dbg start********************************** */
/* boot静态缓冲区 */
struct pcie_boot_debug {
    unsigned int timestamp;
};
#define PCIE_DBG_BOOT_DEBUG_MAX PCIE_BOOT_STAGE_NUM
struct pcie_boot_dbg {
    struct pcie_boot_debug debug[PCIE_DBG_BOOT_DEBUG_MAX];
};
/* ***********************************boot dbg end********************************** */
/* ***********************************cb dbg start********************************** */
/* cb静态缓冲区 */
struct pcie_cb_stage {
    unsigned int timestamp;
    unsigned int cb_result;
};
#define PCIE_DBG_CB_MAX (((int)PCIE_EP_CB_NUM > (int)PCIE_RC_CB_NUM) ? PCIE_EP_CB_NUM : PCIE_RC_CB_NUM)
struct pcie_cb_debug {
    struct pcie_cb_stage stage[PCIE_DBG_CB_MAX];
};
#define PCIE_DBG_CB_DEBUG_MAX PCIE_USER_NUM
struct pcie_cb_dbg {
    struct pcie_cb_debug debug[PCIE_DBG_CB_DEBUG_MAX];
};
extern void pcie_callback_dbg(unsigned int user_id, unsigned int cb_stage, unsigned int cb_result);
/* ***********************************cb dbg end********************************** */
/* ***********************************pcie cfg space dbg start********************************** */
enum cfg_type {
    SAVE_CONFIG,
    RESTORE_CONFIG,
    CFG_TYPE_MAX,
};

#define PCIE_CONFIG_SPACE_SIZE (PCIE_FUNC_NUM * PCIE_CFG_REG_NUM * 4)

struct pcie_config_space {
    unsigned int cfg_space[PCIE_FUNC_NUM][PCIE_CFG_REG_NUM];
};

struct pcie_cfg_dbg {
    struct pcie_config_space ep_config_space[CFG_TYPE_MAX];
};

extern void pcie_config_space_dbg(enum cfg_type type, unsigned int func_num, unsigned int reg_num,
                                  unsigned int reg_val);
extern int pcie_check_restored_config_space_dbg(void);
/* ***********************************pcie cfg space dbg end********************************** */
/* ***********************************dma addr dbg start********************************** */
struct pcie_dma_addr_trace {
    void *dma_lli_addr;
    unsigned int transfer_type;
    unsigned int timestamp;
    unsigned int channel_id;
    unsigned int direction;
    unsigned int transfer_size;
    unsigned int sar_low;
    unsigned int sar_high;
    unsigned int dar_low;
    unsigned int dar_high;
};

#define PCIE_DBG_DMA_ADDR_TRACE_MAX 0x20

struct pcie_dma_addr_dbg {
    struct pcie_dma_addr_trace trans_info[PCIE_DBG_DMA_ADDR_TRACE_MAX];
    unsigned int trace_index;
};

extern void pcie_dma_addr_dbg(struct pcie_dma_transfer_info *transfer_info);
/* ***********************************dma addr dbg end********************************** */
struct pcie_dbg_info {
    u64 dbg_start_byte;
    struct pcie_lock_dbg lock_dbg;
    struct pcie_state_dbg state_dbg;
    struct pcie_event_dbg event_dbg;
    struct pcie_gpio_dbg gpio_dbg;
    struct pcie_msi_dbg msi_dbg;
    struct pcie_wakelock_dbg wakelock_dbg;
    struct pcie_vote_edge_dbg vote_edge_dbg;
    struct pcie_bar_dbg bar_dbg;
    struct pcie_dma_dbg dma_dbg;
    struct pcie_boot_dbg boot_dbg;
    struct pcie_cb_dbg cb_dbg;
    struct pcie_msg_dbg msg_dbg;
    struct pcie_cfg_dbg cfg_dbg;
    struct pcie_dma_addr_dbg dma_addr_dbg;
};

#define PCIE_DBG_TRACE(fmt, args...) do { \
    printk(KERN_ERR "[PCIE_DBG]%s:" fmt "\n", __FUNCTION__, ##args); \
} while (0)
