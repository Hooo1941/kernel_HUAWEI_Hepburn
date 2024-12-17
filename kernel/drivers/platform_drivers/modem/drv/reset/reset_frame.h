/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef RESET_FRAME_H
#define RESET_FRAME_H

#include <linux/pm_wakeup.h>
#include <linux/printk.h>
#include <linux/suspend.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <bsp_print.h>
#include <osl_sem.h>
#include <osl_spinlock.h>
#include <bsp_reset.h>
#include <bsp_om_enum.h>
#include <nva_stru.h>
#include <nva_id.h>

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

/* ************************* 宏和枚举定义 ************************* */
#define RESET_WAIT_RESP_TIMEOUT (5000)              /* ms, time of wating for reply from hifi/mcu */
#define RESET_WAIT_MODEM_IN_IDLE_TIMEOUT (3100)     /* ms, time of wating for reply from modem in idle */
#define RESET_WAIT_M3_RESETING_REPLY_TIMEOUT (200)  /* ms, time of wating for reply from modem in idle */
#define RESET_WAIT_CCORE_WAKEUP_REPLY_TIMEOUT (100) /* ms, time of wating for modem wakeup */
#define RESET_WAIT_CCPU_STARTUP_TIMEOUT (20000)
#define RESET_WAIT_MODEM_STARTUP_TIMEOUT (450)
#define THIS_MODU mod_reset
#define reset_print_err(fmt, ...) (bsp_err("[%s] " fmt, __FUNCTION__, ##__VA_ARGS__))
#define reset_print_shorterr(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))
#define RESET_RETRY_TIMES (3)
#define RESET_IS_SUCC (1)
#define RESET_IS_FAIL (0)
#define RESET_ACTION_HZ_TIMEOUT 10
#define RESET_IPC_NUM 4

#define LPM3_RRPOC_ID IPC_ACPU_LPM3_MBX_4
#define HIFI_MSG_NUM 6
#define MODEM_POWER_TIMEOUT 35000
#define WAIT_CCORE_UP_MS 200
#define MODEM_RESET_NUM 3

enum RESET_IPC_ID {
    RESET_IPC_0 = 0,
    RESET_IPC_1 = 1,
    RESET_IPC_2 = 2,
    RESET_IPC_3 = 3,
};

enum RESET_STATE_E { RESET_UNINIT = 0, RESET_EARLY_INIT_OK = 1, RESET_INIT_OK = 2};
/* 结构体定义 */
#define DRV_RESET_MODULE_NAME_LEN 9
/* Record information of callback functions */
struct reset_cb_info {
    char name[DRV_RESET_MODULE_NAME_LEN + 1];
    u32 priolevel;
    pdrv_reset_cbfun cbfun;
    int userdata;
};

struct reset_cb_list {
    struct reset_cb_info cb_info;
    struct reset_cb_list *next;
};

struct modem_reset_ctrl {
    u32 boot_mode;            /* 表示ccore启动状态: 正常启动/单独复位后启动 */
    struct task_struct *task;
    u32 modem_action;
    spinlock_t action_lock;
    osl_sem_id action_sem;
    osl_sem_id task_sem;
    osl_sem_id wait_mcore_reply_sem;
    osl_sem_id wait_mcore_reply_reseting_sem;
    osl_sem_id wait_modem_status_sem;
    osl_sem_id wait_ccore_reset_ok_sem;
    osl_sem_id wait_modem_host_in_idle_sem;
    struct reset_cb_list *list_head;
    struct workqueue_struct *reset_wq;
    struct work_struct work_reset;
    struct work_struct work_power_off;
    struct work_struct work_power_on;
    struct wakeup_source *wake_lock;
    struct notifier_block pm_notify;
    struct completion suspend_completion;
    ipc_res_id_e ipc_send_irq_wakeup_ccore[RESET_IPC_NUM];
    ipc_res_id_e ipc_recv_irq_idle;
    ipc_handle_t ipc_send_irq_wakeup_ccore_handle[RESET_IPC_NUM];
    ipc_handle_t ipc_recv_irq_idle_handle;
    u32 ipc_valid_mask;     /* 用0~3四个bit位标识IPC通道是否真实存在 */
    s32 in_suspend_state;
    u32 list_use_cnt;
    u32 reset_cnt;
    void *crg_base;
    u32 exec_time;
    drv_ccore_reset_stru_s nv_config;
    u32 state;
    u32 lpmcu_ack;
    u32 reset_state;
    u32 reset_action_status;
    u32 reset_wait_dump_state;
    u32 reset_wait_modem_state;
    u32 scbakdata13_offset;
};

struct modem_reset_priv_s {
    char *name;
    dev_t devt;
    struct cdev reset_cdev;
    struct class *class;
    unsigned int modem_reset_stat;
    unsigned int flag;
    state_cb cb;
    wait_queue_head_t wait;
};

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /*  RESET_FRAME_H */
