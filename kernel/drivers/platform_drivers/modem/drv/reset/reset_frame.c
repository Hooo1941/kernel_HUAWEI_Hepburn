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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/version.h>
#if defined(CONFIG_IPC_RPROC)
#include <linux/platform_drivers/ipc_rproc.h>
#include <linux/platform_drivers/hifidrvinterface.h>
#include <adrv_reset.h>
#endif
#include <product_config.h>
#include <bsp_nva.h>
#include <bsp_msg.h>
#include <msg_id.h>
#include <bsp_fipc.h>
#include <bsp_fiq.h>
#include <bsp_ipc_fusion.h>
#include <bsp_dt.h>
#include <bsp_dump.h>
#include <bsp_nvim.h>
#include <bsp_reset.h>
#include <bsp_dra.h>
#include <bsp_sec_call.h>
#include <bsp_mloader.h>
#include <bsp_power_state.h>
#include <bsp_wdt.h>
#include <mdrv_memory.h>
#include "bsp_pfa_tft.h"
#include "reset_frame.h"
#include "reset_debug.h"
#include "reset_dev.h"
#include <securec.h>
/*lint --e{746, 732, 516, 958, 666} */

struct modem_reset_ctrl g_modem_reset_ctrl = { 0 };
struct modem_reset_priv_s g_mdm_rst_priv[MODEM_RESET_NUM] = {
    {
        .name = "modem0_reset",
        .modem_reset_stat = MODEM_READY,
        .flag = 0,
    },
    {
        .name = "modem1_reset",
        .modem_reset_stat = MODEM_READY,
        .flag = 0,
    },
    {
        .name = "modem2_reset",
        .modem_reset_stat = MODEM_READY,
        .flag = 0,
    },
};

static int rild_app_reset_notify_init(void);
static void reset_notify_app(drv_reset_cb_moment_e stage);
static int reset_init_later(void);

static uintptr_t reset_get_scbakdata13(void)
{
    return ((uintptr_t)bsp_sysctrl_addr_byindex(SYSCTRL_AO) + g_modem_reset_ctrl.scbakdata13_offset);
}

static void reset_set_bootflag(u32 value)
{
    writel((value), (volatile void *)(uintptr_t)reset_get_scbakdata13());
}

static void reset_reboot_system(reset_stage_e stage)
{
    unsigned long flags = 0;

    reset_set_bootflag(CCORE_BOOT_NORMAL);
    reset_timestamp_dump(stage, RECORD_ABNORMAL);
    spin_lock_irqsave(&g_modem_reset_ctrl.action_lock, flags);
    g_modem_reset_ctrl.modem_action = INVALID_MODEM_ACTION;
    g_modem_reset_ctrl.reset_action_status = !RESET_IS_SUCC;
    spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);

    osl_sem_up(&(g_modem_reset_ctrl.action_sem));
}

u32 bsp_reset_ccore_is_reboot(void)
{
    return (u32)((u32)CCORE_IS_REBOOT == (readl((const volatile void *)reset_get_scbakdata13())) ? 1 : 0);
}

// 单独复位前清理boot log内存
static int reset_ccpu_boot_logmem(void)
{
    syslog_logmem_area_info_s info = { 0 };
    int ret = bsp_syslog_get_logmem_info(LOGMEM_CCPUBOOT, &info);
    if ((ret != 0) || (info.virt_addr == NULL) || (info.length == 0)) {
        reset_print_err("reset, get ccpu boot logmem[len: 0x%x] failed!\n", info.length);
        return -1;
    }

    ret = memset_s((void *)info.virt_addr, info.length, 0, info.length);
    if (ret != 0) {
        reset_print_err("reset, memset ccpu boot logmem[len: 0x%x] failed!\n", info.length);
        return -1;
    }

    return 0;
}

s32 reset_prepare(enum MODEM_ACTION action)
{
    unsigned long flags = 0;
    u32 current_action = (u32)action;
    u32 global_action;
    spin_lock_irqsave(&g_modem_reset_ctrl.action_lock, flags);
    global_action = g_modem_reset_ctrl.modem_action;

    if (current_action == global_action || (global_action == (u32)INVALID_MODEM_ACTION)) {
        spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
        return RESET_OK;
    } else if ((global_action == (u32)MODEM_NORMAL) ||
        ((global_action == (u32)MODEM_POWER_OFF) && (current_action == (u32)MODEM_POWER_ON))) {
        // 可进行reset
    } else {
        spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
        reset_print_err("action(%d) is done, abandon action(%d)\n", global_action, action);
        return RESET_ERROR;
    }
    reinit_main_stage();
    reset_reinit_acore_dump();
    g_modem_reset_ctrl.modem_action = action;
    spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
    __pm_stay_awake(g_modem_reset_ctrl.wake_lock);
    reset_print_err("(%d) wake_lock\n", get_updated_main_stage());

    if ((current_action == MODEM_POWER_OFF) || (current_action == MODEM_RESET)) {
        bsp_ccore_ipc_disable();
    }
    osl_sem_up(&(g_modem_reset_ctrl.task_sem));
    reset_timestamp_dump(RESET_DUMP_PREPARE, RECORD_NORMAL);
    return RESET_OK; /*lint !e454 */
}

static s32 reset_drv_cb_before(s32 stage)
{
    s32 ret;
    reset_print_err("before reset rfile cb\n");
    ret = bsp_rfile_reset_cb(stage, 0);
    if (ret != 0) {
        return RESET_ERROR;
    }

    reset_print_err("before reset dump cb\n");
    (void)bsp_dump_sec_channel_free(stage, 0);

    ret = reset_ccpu_boot_logmem();
    if (ret != 0) {
        reset_print_err("ccpu boot log mem clear fail!\n");
    }
    return RESET_OK;
}

static s32 reset_drv_cb_after(s32 stage)
{
    reset_print_err("after reset wdt cb\n");
    bsp_wdt_reinit();
    return RESET_OK;
}

static s32 reset_drv_cb(drv_reset_cb_moment_e stage, int userdata)
{
    switch (stage) {
        case MDRV_RESET_CB_BEFORE:
            if (reset_drv_cb_before(stage) != RESET_OK) {
                reset_print_err("drv cb before func run fail!\n");
                return RESET_ERROR;
            }
            break;

        case MDRV_RESET_RESETTING:
            // 提前到复位前处理bsp_ipc_modem_reset(stage, 0);
            break;

        case MDRV_RESET_CB_AFTER:
            if (reset_drv_cb_after(stage) != RESET_OK) {
                reset_print_err("drv cb after func run fail!\n");
                return RESET_ERROR;
            }
            break;

        default:
            break;
    }

    return RESET_OK;
}

s32 invoke_reset_cb(drv_reset_cb_moment_e stage)
{
    struct reset_cb_list *p = g_modem_reset_ctrl.list_head;
    int ret;

    reset_print_err("(%d) @reset %d\n", get_updated_main_stage(), (u32)stage);

    /* 根据回调函数优先级，调用回调函数 */
    while (p != NULL) {
        if (p->cb_info.cbfun == NULL) {
            p = p->next;
            continue;
        }
        reset_print_err("cb stage %d %d %s callback invoked\n", stage, p->cb_info.priolevel, p->cb_info.name);
        ret = p->cb_info.cbfun(stage, p->cb_info.userdata);
        if (ret != RESET_OK) {
            if (p->cb_info.priolevel != DRV_RESET_CB_PIOR_PFA_TFT)
                reset_print_err("fail to run cbfunc of %s, at stage%d return %d\n", p->cb_info.name, stage, ret);
            return RESET_ERROR;
        }
        p = p->next;
    }

    return RESET_OK;
}

#if defined(CONFIG_IPC_RPROC)
#define MODEM_IPC_MSG_BUFF_SIZE 2
#define MODEM_IPC_MSG_HEAD (0 << 24 | 9 << 16 | 3 << 8)
#define MODEM_IPC_INT_NUM (32 << 8)
#define MODEM_IPC_MSG_TIMEOUT 5000

static s32 send_sync_msg_to_mcore(u32 reset_info, u32 *ack_val)
{
    s32 ret;
    /* 如果是modem处于复位状态，则调用AP侧IPC */
    /* ap receive mabx 0,send mbx 13 */
    rproc_msg_t tx_buffer[MODEM_IPC_MSG_BUFF_SIZE];
    rproc_msg_t ack_buffer[MODEM_IPC_MSG_BUFF_SIZE];
    /* 发送标志，用于LPM3上接收时解析 */
    tx_buffer[0] = MODEM_IPC_MSG_HEAD;
    tx_buffer[1] = reset_info;
    ret = RPROC_SYNC_SEND(LPM3_RRPOC_ID, tx_buffer, MODEM_IPC_MSG_BUFF_SIZE, ack_buffer, MODEM_IPC_MSG_BUFF_SIZE);
    if (ret) {
        *ack_val = (u32)-1;
    } else {
        *ack_val = (u32)ack_buffer[1];
    }
    return ret;
}
static int send_sync_msg_to_hifi(BSP_CORE_TYPE_E ecoretype, unsigned int cmdtype, unsigned int timeout_ms,
    unsigned int *retval)
{
    int ret;
    rproc_id_t rproc_id;
    rproc_msg_t tx_buffer[MODEM_IPC_MSG_BUFF_SIZE];
    rproc_msg_t ack_buffer[MODEM_IPC_MSG_BUFF_SIZE];

    if (ecoretype == BSP_HIFI) {
        tx_buffer[0] = MODEM_IPC_INT_NUM; /* the INT_SRC_NUM to hifi */
        rproc_id = IPC_ACPU_HIFI_MBX_2;
    } else {
        reset_print_err("wrong ecoretype\n");
        return -1;
    }

    tx_buffer[1] = cmdtype;

    ret = RPROC_SYNC_SEND(rproc_id, tx_buffer, MODEM_IPC_MSG_BUFF_SIZE, ack_buffer, MODEM_IPC_MSG_BUFF_SIZE);
    if (ret == 0) {
        /* the send is reponsed by the remote process, break out */
        *retval = ack_buffer[1];
    } else if (ret == -ETIMEOUT) {
        ret = BSP_RESET_NOTIFY_TIMEOUT;
        reset_print_err("to hifi's ipc timeout\n");
    } else {
        ret = BSP_RESET_NOTIFY_SEND_FAILED;
        reset_print_err("bad parameter or other error\n");
    }

    return ret;
}
static void hifi_reply_ok_process(u32 val)
{
    if (!val) {
        reset_print_err("(%d) has received the reply from hifi\n", get_updated_main_stage());
    } else {
        reset_print_err("unkown msg from hifi\n");
    }
    return;
}

s32 send_msg_to_hifi(drv_reset_cb_moment_e stage)
{
    s32 ret = RESET_ERROR;
    u32 val = ~0x0;
    u32 cmd_type = 0;

    reset_print_err("(%d) stage%d msg to hifi\n", get_updated_main_stage(), (s32)stage);

    if (stage == MDRV_RESET_CB_BEFORE) {
        cmd_type = ID_AP_HIFI_CCPU_RESET_REQ;
        reset_print_err("reset before cmd type:0x%x\n", cmd_type);
    } else if (stage == MDRV_RESET_CB_AFTER) {
        cmd_type = ID_AP_HIFI_CCPU_RESET_DONE_IND;
        reset_print_err("reset after cmd type:0x%x\n", cmd_type);
    } else if (stage == MDRV_RESET_RESETTING) {
        cmd_type = ID_AP_HIFI_CCPU_SUSPEND_IND;
        reset_print_err("reseting cmd type:0x%x\n", cmd_type);
    } else {
        reset_print_err("stage%d unexpected!");
        return ret;
    }

    ret = send_sync_msg_to_hifi(BSP_HIFI, cmd_type, MODEM_IPC_MSG_TIMEOUT, &val);
    switch (ret) {
        case BSP_RESET_NOTIFY_REPLY_OK:
            hifi_reply_ok_process(val);
            break;
        case BSP_RESET_NOTIFY_SEND_FAILED:
            reset_print_err("send_msg_to_hifi=0x%x fail\n", ret);
            break;
        case BSP_RESET_NOTIFY_TIMEOUT:
            reset_print_err("waiting the reply from hifi timeout(%d), but not reboot system!\n",
                RESET_WAIT_RESP_TIMEOUT);
            break;
        default:
            reset_print_err("unexpect scenario\n");
            break;
    }
    /* 如果有必要，其他阶段也通知hifi */
    return ret;
}
#else
static msg_chn_t g_modem_reset_msghdl;
#define LPMCU_ACK_TIMEOUT 5000
static int mdmreset_msg_lite_proc(const struct msg_addr *src, void *msg, u32 len)
{
    if (msg == NULL || len != sizeof(u32)) {
        return -1;
    }
    g_modem_reset_ctrl.reset_state = *(u32 *)msg;
    osl_sem_up(&(g_modem_reset_ctrl.wait_mcore_reply_sem));
    return 0;
}
s32 send_sync_msg_to_mcore(u32 reset_info, u32 *ack_val)
{
    int ret;
    struct msg_addr dst;
    dst.core = MSG_CORE_LPM;
    dst.chnid = MSG_CHN_MDM_RST;
    reset_print_err("use msg lpmcu\n");
    ret = bsp_msg_lite_sendto(g_modem_reset_msghdl, &dst, &reset_info, sizeof(reset_info));
    if (ret != 0) {
        reset_print_err("memcpy fail in reset, ret = 0x%x.\n", ret);
        return ret;
    }
    reset_print_err("send msg lpmcu success.\n");

    ret = osl_sem_downtimeout(&(g_modem_reset_ctrl.wait_mcore_reply_sem), msecs_to_jiffies(LPMCU_ACK_TIMEOUT));
    if (ret != 0) { /*lint !e713 */
        reset_print_err("Get response from lpmcu reset timeout within 5s\n");
        return -1;
    }
    reset_print_err("reset msg back ok.\n");
    *ack_val = g_modem_reset_ctrl.reset_state;

    return 0;
}

s32 send_msg_to_hifi(drv_reset_cb_moment_e stage)
{
    // mbb hifi use fipc to comm.
    return 0;
}
#endif

s32 wait_for_ccore_reset_done(u32 timeout)
{
    reset_print_err("(%d) waiting the reply from ccore\n", get_updated_main_stage());

    if (osl_sem_downtimeout(&(g_modem_reset_ctrl.wait_ccore_reset_ok_sem), msecs_to_jiffies(timeout))) { /*lint !e713 */
        reset_print_err("Get response from ccore reset timeout within %d\n", timeout);
        return RESET_ERROR;
    }

    reset_print_err("(%d) has received the reply from modem\n", get_updated_main_stage());
    return RESET_OK;
}

void do_power_off_wakelock_send(void)
{
    s32 ret = 0;
    int i;
    for (i = 0; i < RESET_IPC_NUM; i++) {
        if ((g_modem_reset_ctrl.ipc_valid_mask & (1 << i)) == 0) {
            reset_print_err("ipc channel %d Invalid!\n", i);
            continue;
        }
        ret = bsp_ipc_send(g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore_handle[i]);
        if (ret) {
            reset_print_err("ipc send to ccore fail! channel:%d\n", i);
        } else {
            reset_print_err("ipc send wakeup success: %d, 0x%x\n", i, g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[i]);
        }
    }
    if (ret != 0) {
        reset_print_err("wakeup ccore fail\n");
    }
}

s32 do_power_off_fiq_idle_send(void)
{
    s32 ret;

    /* 通知modem host进idle，并等待ccore回复 */
    if (BSP_OK != bsp_send_cp_fiq(FIQ_RESET)) {
        reset_print_err("fiq trigger fail\n");
    }
    ret = osl_sem_downtimeout(&(g_modem_reset_ctrl.wait_modem_host_in_idle_sem),
        msecs_to_jiffies(RESET_WAIT_MODEM_IN_IDLE_TIMEOUT)); /*lint !e713 */
    if (ret) {
        reset_print_err("(%d) let modem host in idle timeout\n", get_updated_main_stage());
    } else {
        reset_print_err("(%d) let modem host in idle successfully\n", get_updated_main_stage());
    }
    return ret;
}

s32 do_power_off_mdm_reset_unreset(u16 action)
{
    u32 msg;
    u32 ack_val;
    s32 ret;

    reset_print_err("(%d) before reset stage communicate with lpm3 \n", get_updated_main_stage());
    /* 通知m3进行复位前辅助处理 */
    msg = RESET_INFO_MAKEUP(action, MDRV_RESET_CB_BEFORE); /*lint !e701 */

    /* 复位解复位modem子系统 */
    ret = send_sync_msg_to_mcore(msg, &ack_val);
    if (ret) {
        reset_print_err("send_sync_msg_to_mcore(0x%x) before reset fail!\n", ret);
        return RESET_ERROR;
    } else if (ack_val != RESET_MCORE_BEFORE_RESET_OK) {
        reset_print_err("cp reset before error probed on m3, ack_val=0x%x\n", ack_val);
        return RESET_ERROR;
    }
    reset_print_err("(%d) before reset stage has communicated with lpm3 succeed\n", get_updated_main_stage());
    return RESET_OK;
}

void do_power_off_ap_host_reinit(void)
{
    reset_print_err("bsp_dra_reinit ++\n");
    bsp_dra_reinit();
    reset_print_err("bsp_dra_reinit --\n");
    reset_print_err("bsp_pfa_reinit ++\n");
    bsp_pfa_reinit();
    reset_print_err("bsp_pfa_reinit --\n");
    reset_print_err("bsp_pfa_tft_reinit ++\n");
    bsp_pfa_tft_reinit();
    reset_print_err("bsp_pfa_tft_reinit --\n");
}

s32 do_power_off(u16 action)
{
    s32 ret;

    // 通知APP层复位开始
    reset_timestamp_dump(RESET_DUMP_POWER_OFF, RECORD_NORMAL);
    reset_notify_app(MDRV_RESET_CB_BEFORE);

    /* 设置启动模式为C核单独复位 */
    reset_set_bootflag(CCORE_IS_REBOOT);
    g_modem_reset_ctrl.boot_mode = readl((volatile const void *)(uintptr_t)reset_get_scbakdata13());
    reset_print_err("(%d) set boot mode:0x%x\n", get_updated_main_stage(), g_modem_reset_ctrl.boot_mode);

    do_power_off_wakelock_send();

    /* 复位前各组件回调 */
    if (invoke_reset_cb(MDRV_RESET_CB_BEFORE) < 0) {
        reset_reboot_system(RESET_DUMP_CB_BEFORE);
        return RESET_ERROR;
    }
    reset_timestamp_dump(RESET_DUMP_CB_BEFORE, RECORD_NORMAL);

    /* 通知hifi停止与modem交互并等待hifi处理完成 */
    if (send_msg_to_hifi(MDRV_RESET_CB_BEFORE) == BSP_ERROR) {
        reset_timestamp_dump(RESET_DUMP_MSG_TO_HIFI, RECORD_ABNORMAL);
        reset_print_err("send msg to hifi fail %d\n", get_updated_main_stage());
    }
    reset_timestamp_dump(RESET_DUMP_MSG_TO_HIFI, RECORD_NORMAL);

    bsp_msg_act_before_rst(MSG_CORE_TSP);

    reset_dump_clear_ccore_mem();
    // mdrv_sync_wait fail judge. use ipc feedback or sharemem.
    msleep(WAIT_CCORE_UP_MS);

    ret = do_power_off_fiq_idle_send();
    /* 开关控制idle超时是否进行整机复位 */
    if ((ret != 0) && (g_modem_reset_ctrl.nv_config.is_idle_timerout_reset == 1)) {
        reset_reboot_system(RESET_DUMP_CCOER_IN_IDLE);
        return RESET_ERROR;
    }
    reset_timestamp_dump(RESET_DUMP_CCOER_IN_IDLE, RECORD_NORMAL);

    /* acore fipc跟modem侧协商下电 */
    if (bsp_fipc_chn_before_resetting() != 0) {
        reset_reboot_system(RESET_DUMP_FIPC_BEFORE);
        return RESET_ERROR;
    }

    bsp_ipc_modem_reset(MDRV_RESET_CB_BEFORE, 0);

    reset_cp_dump_parse();

    /* 通知hifi完成核间通信IP协商下电,防止后续功能不可用,放在modem idle之后 */
    if (send_msg_to_hifi(MDRV_RESET_RESETTING) == BSP_ERROR) {
        reset_timestamp_dump(RESET_DUMP_MSG_TO_HIFI, RECORD_ABNORMAL);
        reset_print_err("send msg to hifi fail %d\n", get_updated_main_stage());
    }
    reset_timestamp_dump(RESET_DUMP_MSG_TO_HIFI, RECORD_NORMAL);

    if (bsp_mloader_load_reset() != 0) {
        reset_reboot_system(RESET_DUMP_MLOADER_LOAD_RESET);
        return RESET_ERROR;
    }

    if (do_power_off_mdm_reset_unreset(action) != RESET_OK) {
        reset_reboot_system(RESET_DUMP_MDMPOWER_DOWN);
        return RESET_ERROR;
    }
    reset_timestamp_dump(RESET_DUMP_MDMPOWER_DOWN, RECORD_NORMAL);

    do_power_off_ap_host_reinit();
    return RESET_OK;
}

s32 do_power_on_after(u16 action)
{
    s32 ret;
    u32 msg;
    u32 ack_val;

    /* 复位后相关处理 */
    ret = invoke_reset_cb(MDRV_RESET_CB_AFTER);
    if (ret < 0) {
        reset_reboot_system(RESET_DUMP_CB_INVOKE_AFTER);
        return RESET_ERROR;
    }

    /* 复位后通知M3进行相关处理 */
    msg = RESET_INFO_MAKEUP(action, MDRV_RESET_CB_AFTER); /*lint !e701 */

    ret = send_sync_msg_to_mcore(msg, &ack_val);
    if (ret) {
        reset_print_err("send_sync_msg_to_mcore(0x%x) after reset stage fail!\n", ret);
        reset_reboot_system(RESET_DUMP_SEND_MSG2_M3_FAIL_AFTER);
        return RESET_ERROR;
    } else if (ack_val != RESET_MCORE_AFTER_RESET_OK) {
        reset_print_err("after reset handle failed on m3, ack_val=0x%x, fail!\n", ack_val);
        reset_reboot_system(RESET_DUMP_RECV_WRONG_MSG_FROM_M3_AFTER);
        return RESET_ERROR;
    }
    reset_print_err("(%d) after reset stage has communicated with lpm3 succeed\n", get_updated_main_stage());

    /* 将启动模式置回普通模式 */
    reset_set_bootflag(CCORE_BOOT_NORMAL);
    return RESET_OK;
}

s32 do_power_on_loadimage(void)
{
    int i;
    s32 ret;
    for (i = 0; i < RESET_RETRY_TIMES; i++) {
        ret = bsp_load_modem_images();
        if (ret == 0)
            break;
        else
            reset_print_err("Retry %d times to load modem image also failed\n", i);
    }
    if (ret < 0) {
        return RESET_ERROR;
    }
    return RESET_OK;
}

s32 do_power_on_resetting(u16 action)
{
    s32 ret, ack_val, msg;

    /* 通知m3进行复位中相关处理 */
    msg = RESET_INFO_MAKEUP(action, (u32)MDRV_RESET_RESETTING); /*lint !e701 */

    ret = send_sync_msg_to_mcore(msg, &ack_val);
    if (ret) {
        reset_print_err("send_sync_msg_to_mcore(0x%x) at resting stage fail!\n", ret);
        reset_reboot_system(RESET_DUMP_SEND_MSG2_M3_FAIL_RESTING);
        return RESET_ERROR;
    } else if (ack_val != RESET_MCORE_RESETING_OK) {
        reset_print_err("modem unreset fail on m3, ack_val=0x%x\n", ack_val);
        reset_reboot_system(RESET_DUMP_RECV_WRONG_MSG_FROM_M3_RESTING);
        return RESET_ERROR;
    }
    reset_print_err("(%d) at reseting stage has communicated with lpm3 succeed\n", get_updated_main_stage());

    /* C核和dsp镜像加载 */
    /* 如出现错误，重试3次，直至每次都错误，则复位系统 */
    if (do_power_on_loadimage() != RESET_OK) {
        reset_reboot_system(RESET_DUMP_LOAD_MODEM_IMG);
        return RESET_ERROR;
    }
    reset_print_err("(%d) reseting stage start load image end\n", get_updated_main_stage());

    ret = reset_drv_cb(MDRV_RESET_RESETTING, 0);
    if (ret < 0) {
        reset_reboot_system(RESET_DUMP_CB_INVOKE_RESTING);
        return RESET_ERROR;
    }

    ret = wait_for_ccore_reset_done(RESET_WAIT_CCPU_STARTUP_TIMEOUT);
    if (ret < 0) {
        reset_reboot_system(RESET_DUMP_WAIT_CCORE_RELAY_TIMEOUT);
        return RESET_ERROR;
    }
    return RESET_OK;
}

s32 do_power_on(u16 action)
{
    s32 ret;
    reset_print_err("(%d) reseting stage icc channel reset \n", get_updated_main_stage());

    reset_timestamp_dump(RESET_DUMP_POWER_ON_START, RECORD_NORMAL);
    ret = bsp_fipc_chn_after_resetting();
    if (ret != RESET_OK) {
        reset_reboot_system(RESET_DUMP_FIPC_RESETTING);
        return RESET_ERROR;
    }

    reset_print_err("(%d) reseting stage ipc reset \n", get_updated_main_stage());

    /* 在C核drv启动前，icc需要准备好 */
    bsp_ipc_modem_reset(MDRV_RESET_RESETTING, 0);
    bsp_ccore_ipc_enable();

    bsp_msg_act_after_rst(MSG_CORE_TSP);

    /* 镜像加载前，rfile需要准备好 */
    ret = bsp_rfile_reset_cb(MDRV_RESET_RESETTING, 0);
    if (ret != 0) {
        reset_reboot_system(RESET_DUMP_RFILE_RESETTING);
        return RESET_ERROR;
    }
    reset_print_err("(%d) reseting stage start load image\n", get_updated_main_stage());

    if (do_power_on_resetting(action) != RESET_OK) {
        reset_print_err("do_power_on_resetting fail!\n");
        reset_reboot_system(RESET_DUMP_WAIT_CCORE_RELAY_TIMEOUT);
        return RESET_ERROR;
    }
    reset_timestamp_dump(RESET_DUMP_POWER_ON_RESETTING, RECORD_NORMAL);

    if (do_power_on_after(action) != RESET_OK) {
        reset_print_err("do_power_on_after fail!\n");
        reset_reboot_system(RESET_DUMP_CB_INVOKE_AFTER);
        return RESET_ERROR;
    }

    /* hifi核在融合架构有独立复位能力，跟rild有交互，可以独立恢复，和hifi消息不通有以下场景hifi没上电，hifi异常进入单独复位 */
    /* 因此消息失败后忽略，直接上报单独复位成功，hifi进行自恢复即可 */
    ret = send_msg_to_hifi(MDRV_RESET_CB_AFTER);
    if (ret != RESET_OK) {
        reset_print_err("reset after send msg to hifi fail!\n");
    }
    // 通知APP层复位结束
    reset_notify_app(MDRV_RESET_CB_AFTER);

    return RESET_OK;
}

s32 do_reset(u16 action)
{
    s32 ret;

    ret = do_power_off(action);
    if (ret < 0) {
        return RESET_ERROR;
    }

    ret = do_power_on(action);
    if (ret < 0) {
        return RESET_ERROR;
    }

    return RESET_OK;
}

int modem_reset_task(void *arg)
{
    u16 action;
    unsigned long flags = 0;
    unsigned int err_action = 0x00;

    if (reset_init_later() != RESET_OK) {
        reset_print_err("reset init later fail, reset task exit.\n");
        return -1;
    }

    while (!err_action) {
        osl_sem_down(&(g_modem_reset_ctrl.task_sem));
        spin_lock_irqsave(&g_modem_reset_ctrl.action_lock, flags);
        action = (u16)g_modem_reset_ctrl.modem_action;
        spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
        reset_print_err("(%d)task_sem up, modem_action:%d\n", get_updated_main_stage(), action);

        if (action == MODEM_POWER_OFF) {
            (void)do_power_off(action);
        } else if (action == MODEM_POWER_ON) {
            (void)do_power_on(action);
        } else if (action == MODEM_RESET) {
            (void)do_reset(action);
            g_modem_reset_ctrl.reset_cnt++;
            reset_print_err("reset count: %d\n", g_modem_reset_ctrl.reset_cnt);
        } else if (action != MODEM_NORMAL && action != INVALID_MODEM_ACTION) {
            reset_print_err("reset action err, modem_reset_task exit:%d\n", action);
            err_action = 0x01;
        }

        spin_lock_irqsave(&g_modem_reset_ctrl.action_lock, flags);
        if ((action == MODEM_POWER_ON || action == MODEM_RESET) && (action == g_modem_reset_ctrl.modem_action)) {
            g_modem_reset_ctrl.modem_action = MODEM_NORMAL;
            g_modem_reset_ctrl.reset_action_status = RESET_IS_SUCC;
        }
        spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);

        osl_sem_up(&(g_modem_reset_ctrl.action_sem));
        __pm_relax(g_modem_reset_ctrl.wake_lock);
        g_modem_reset_ctrl.exec_time = get_timer_slice_delta(g_modem_reset_ctrl.exec_time, bsp_get_slice_value());
        reset_print_err("execute done, elapse time %d\n", g_modem_reset_ctrl.exec_time);
    }
    return -1;
}

struct reset_cb_list *sort_list_insert(struct reset_cb_list *list, struct reset_cb_list *node)
{
    struct reset_cb_list *head = list;
    struct reset_cb_list *curr = list;
    struct reset_cb_list *tail = list;

    if (list == NULL || node == NULL) {
        return NULL;
    }
    while (curr != NULL) {
        /* 由小到大, 按优先级插入 */
        if (curr->cb_info.priolevel > node->cb_info.priolevel) {
            if (head == curr) {
                node->next = curr;
                head = node;
            } else {
                tail->next = node;
                node->next = curr;
            }
            break;
        }
        tail = curr;
        curr = curr->next;
    }
    if (curr == NULL) {
        tail->next = node;
    }
    return head;
}

struct reset_cb_list *do_cb_func_register(struct reset_cb_list *list_head, const char *func_name, pdrv_reset_cbfun func,
    int user_data, int prior)
{
    struct reset_cb_list *cb_func_node = NULL;
    u32 name_len;
    int ret;

    if (func_name == NULL || func == NULL ||
        (prior < RESET_CBFUNC_PRIO_LEVEL_LOWT || prior > RESET_CBFUNC_PRIO_LEVEL_HIGH)) {
        reset_print_err("register fail, name:%s, prio=%d\n", func_name, prior);
        return list_head;
    }

    cb_func_node = (struct reset_cb_list *)kmalloc(sizeof(struct reset_cb_list), GFP_KERNEL);
    if (cb_func_node != NULL) {
        name_len = (u32)min((u32)DRV_RESET_MODULE_NAME_LEN, (u32)strlen(func_name));

        ret = memset_s((void *)(uintptr_t)cb_func_node, (sizeof(*cb_func_node)), 0, (sizeof(struct reset_cb_list)));
        if (ret) {
            reset_print_err("memset error%d\n", ret);
            kfree(cb_func_node);
            return NULL;
        }

        ret = memcpy_s((void *)cb_func_node->cb_info.name,
            (u32)min((u32)DRV_RESET_MODULE_NAME_LEN, (u32)strlen(func_name)), (void *)func_name, (int)name_len);
        if (ret) {
            reset_print_err("memcpy error%d\n", ret);
            kfree(cb_func_node);
            return NULL;
        }
        cb_func_node->cb_info.priolevel = prior;
        cb_func_node->cb_info.userdata = user_data;
        cb_func_node->cb_info.cbfun = func;
    } else {
        reset_print_err("register malloc fail, name:%s, prio=%d\n", func_name, prior);
        return list_head;
    }

    if (list_head == NULL) {
        list_head = cb_func_node;
    } else {
        list_head = sort_list_insert(list_head, cb_func_node);
    }
    ++g_modem_reset_ctrl.list_use_cnt;

    return list_head;
}

void modem_reset_do_work(struct work_struct *work)
{
    reset_print_err("\n");
    g_modem_reset_ctrl.exec_time = bsp_get_slice_value();
    if (g_modem_reset_ctrl.in_suspend_state == 1) {
        reset_print_err("reset in suspend state!");
        if (!wait_for_completion_timeout(&(g_modem_reset_ctrl.suspend_completion), HZ * RESET_ACTION_HZ_TIMEOUT)) {
            machine_restart("system halt"); /* 调systemError */
            return;
        }
    }
    if (reset_prepare(MODEM_RESET) != RESET_OK) {
        reset_print_err("reset prepare fail!");
    }
}

void modem_power_off_do_work(struct work_struct *work)
{
    reset_print_err("\n");
    if (g_modem_reset_ctrl.in_suspend_state == 1) {
        if (!wait_for_completion_timeout(&(g_modem_reset_ctrl.suspend_completion), HZ * RESET_ACTION_HZ_TIMEOUT)) {
            machine_restart("system halt"); /* */
            return;
        }
    }
    if (reset_prepare(MODEM_POWER_OFF) != RESET_OK) {
        reset_print_err("reset prepare fail!");
    }
}

void modem_power_on_do_work(struct work_struct *work)
{
    reset_print_err("\n");
    if (g_modem_reset_ctrl.in_suspend_state == 1) {
        if (!wait_for_completion_timeout(&(g_modem_reset_ctrl.suspend_completion), HZ * RESET_ACTION_HZ_TIMEOUT)) {
            machine_restart("system halt");
            return;
        }
    }
    if (bsp_reset_is_successful(MODEM_POWER_TIMEOUT))
        reset_prepare(MODEM_POWER_ON);
}

static s32 reset_pm_notify(struct notifier_block *notify_block, unsigned long mode, void *reset_unused)
{
    switch (mode) {
        case PM_HIBERNATION_PREPARE:
        case PM_SUSPEND_PREPARE:
            g_modem_reset_ctrl.in_suspend_state = 1;
            break;

        case PM_POST_SUSPEND:
        case PM_POST_HIBERNATION:
        case PM_POST_RESTORE:
            complete(&(g_modem_reset_ctrl.suspend_completion));
            g_modem_reset_ctrl.in_suspend_state = 0;
            break;
        default:
            break;
    }

    return 0;
}

void reset_feature_on(u32 sw)
{
    g_modem_reset_ctrl.nv_config.is_feature_on = (sw ? 1 : 0);
}

void reset_ipc_isr_idle(void *data)
{
    reset_print_err("ccore idle ok\n");
    osl_sem_up(&(g_modem_reset_ctrl.wait_modem_host_in_idle_sem));
}

u32 bsp_reset_is_feature_on(void)
{
    if (g_modem_reset_ctrl.state == RESET_UNINIT) {
        if (bsp_nv_read(NVA_ID_DRV_CCORE_RESET, (u8 *)&(g_modem_reset_ctrl.nv_config),
            sizeof(drv_ccore_reset_stru_s)) != BSP_OK) {
            reset_feature_on(0);
            reset_print_err("nv read fail, reset feature set off!\n");
        }
    }
    return g_modem_reset_ctrl.nv_config.is_feature_on;
}

void bsp_reset_ccore_init_ok(void)
{
    if (bsp_reset_is_feature_on() == MDM_RESET_UNSUPPORT) {
        reset_print_err("modem reset feature is off!\n");
        return;
    }

    if (bsp_reset_ccore_is_reboot()) {
        osl_sem_up(&(g_modem_reset_ctrl.wait_ccore_reset_ok_sem));
        reset_print_err("receive modem reset ok,send irq to reset\n");
    }
}

int bsp_cp_reset(void)
{
    if (g_modem_reset_ctrl.state != RESET_INIT_OK) {
        reset_print_err("reset is not init\n");
        return -1;
    }

    reset_print_err("\n");
    if (g_modem_reset_ctrl.nv_config.is_feature_on) {
        queue_work(g_modem_reset_ctrl.reset_wq, &(g_modem_reset_ctrl.work_reset));
        return 0;
    } else {
        reset_print_err("reset fearute is off\n");
        return -1;
    }
}

void bsp_modem_power_off(void)
{
    reset_print_err("\n");
    if (g_modem_reset_ctrl.nv_config.is_feature_on) {
        queue_work(g_modem_reset_ctrl.reset_wq, &(g_modem_reset_ctrl.work_power_off));
    } else {
        reset_print_err("reset fearute is off\n");
    }
}

void bsp_modem_power_on(void)
{
    reset_print_err("\n");
    if (g_modem_reset_ctrl.nv_config.is_feature_on) {
        queue_work(g_modem_reset_ctrl.reset_wq, &(g_modem_reset_ctrl.work_power_on));
    } else {
        reset_print_err("reset fearute is off\n");
    }
}

int bsp_reset_cb_func_register(const char *name, pdrv_reset_cbfun func, drv_reset_cb_moment_e user_data, int prior)
{
    u32 use_cnt;
    struct reset_cb_list *head = g_modem_reset_ctrl.list_head;

    if (name != NULL) {
        reset_print_err("register info: name:%s, prio=%d\n", name, prior);
    }
    use_cnt = g_modem_reset_ctrl.list_use_cnt;
    g_modem_reset_ctrl.list_head = do_cb_func_register(head, name, func, user_data, prior);
    if (use_cnt == g_modem_reset_ctrl.list_use_cnt) {
        return RESET_ERROR;
    }

    return RESET_OK;
}
EXPORT_SYMBOL(bsp_reset_cb_func_register);

u32 bsp_reset_is_successful(u32 timeout_ms)
{
    s32 ret;
    unsigned long flags = 0;
    ret = osl_sem_downtimeout(&(g_modem_reset_ctrl.action_sem), msecs_to_jiffies(timeout_ms)); /*lint !e713 */

    spin_lock_irqsave(&g_modem_reset_ctrl.action_lock, flags);
    if ((ret == 0) && (g_modem_reset_ctrl.modem_action == MODEM_NORMAL)) {
        reset_print_err("modem reset success!\n");
        spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
        return RESET_IS_SUCC;
    }

    if (g_modem_reset_ctrl.modem_action == MODEM_POWER_OFF) {
        reset_print_err("modem reset success, power off!\n");
        spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
        return RESET_IS_SUCC;
    }

    spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);
    reset_print_err("modem reset fail, ret:%d!\n", ret);
    return RESET_IS_FAIL;
}

static int reset_of_node_init(void)
{
    device_node_s *node = NULL;
    int ret;
    node = bsp_dt_find_compatible_node(NULL, NULL, "cp_reset_app");
    if (node == NULL) {
        reset_print_err("dts node not found!\n");
        return RESET_ERROR;
    }

    ret = bsp_dt_property_read_u32(node, "scbakdata13", &g_modem_reset_ctrl.scbakdata13_offset);
    if (ret) {
        reset_print_err("read scbakdata13 from dts is failed,ret = %d!\n", ret);
        return RESET_ERROR;
    }

    return RESET_OK;
}

int reset_var_init(void)
{
    unsigned long flags = 0;

    reset_set_bootflag(CCORE_BOOT_NORMAL);

    spin_lock_init(&g_modem_reset_ctrl.action_lock);
    /* 置上acore与ccore之间通信状态可用标志 */
    spin_lock_irqsave(&g_modem_reset_ctrl.action_lock, flags);
    g_modem_reset_ctrl.modem_action = MODEM_NORMAL;
    g_modem_reset_ctrl.reset_action_status = RESET_IS_SUCC;
    spin_unlock_irqrestore(&g_modem_reset_ctrl.action_lock, flags);

    osl_sem_init(0, &g_modem_reset_ctrl.action_sem);
    osl_sem_init(0, &g_modem_reset_ctrl.task_sem);
    osl_sem_init(0, &g_modem_reset_ctrl.wait_mcore_reply_sem);
    osl_sem_init(0, &g_modem_reset_ctrl.wait_mcore_reply_reseting_sem);
    osl_sem_init(0, &g_modem_reset_ctrl.wait_modem_status_sem);
    osl_sem_init(0, &g_modem_reset_ctrl.wait_ccore_reset_ok_sem);
    osl_sem_init(0, &g_modem_reset_ctrl.wait_modem_host_in_idle_sem);

    g_modem_reset_ctrl.wake_lock = wakeup_source_register(NULL, "modem_reset wake");
    if (g_modem_reset_ctrl.wake_lock == NULL) {
        reset_print_err("wakeup source register fail!\n");
    }
    return RESET_OK;
}

s32 bsp_reset_ipc_init(void)
{
    int ret, i;
    g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[RESET_IPC_0] = IPC_TSP_INT_SRC_ACPU_DSS0_STAYUP;
    g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[RESET_IPC_1] = IPC_TSP_INT_SRC_ACPU_DSS1_STAYUP;
    g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[RESET_IPC_2] = IPC_TSP_INT_SRC_ACPU_DSS2_STAYUP;
    g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[RESET_IPC_3] = IPC_TSP_INT_SRC_ACPU_DSS3_STAYUP;

    g_modem_reset_ctrl.ipc_recv_irq_idle = IPC_ACPU_INT_SRC_TSP_RESET_DSS0_IDLE;

    for (i = 0; i < RESET_IPC_NUM; i++) {
        ret = bsp_ipc_open(g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[i],
            &g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore_handle[i]);
        if (ret) {
            reset_print_err("wakeup ccore ipc[0x%x] open fail! ret:%d\n",
                g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[i], ret);
        } else {
            reset_print_err("wakeup ccore ipc[0x%x] open success!\n", g_modem_reset_ctrl.ipc_send_irq_wakeup_ccore[i]);
            g_modem_reset_ctrl.ipc_valid_mask |= (1 << i);
        }
    }
    reset_print_err("ipc_valid_mask:[0x%x]", g_modem_reset_ctrl.ipc_valid_mask);
    ret = bsp_ipc_open(g_modem_reset_ctrl.ipc_recv_irq_idle, &g_modem_reset_ctrl.ipc_recv_irq_idle_handle);
    if (ret) {
        reset_print_err("recv irq idle ipc open idle fail! channel:0x%x\n", g_modem_reset_ctrl.ipc_recv_irq_idle);
        return RESET_ERROR;
    }

    ret = bsp_ipc_connect(g_modem_reset_ctrl.ipc_recv_irq_idle_handle, (ipc_cbk)reset_ipc_isr_idle,
        (void *)&g_modem_reset_ctrl.ipc_recv_irq_idle);
    if (ret) {
        reset_print_err("recv irq idle ipc open connect fail! channel:0x%x\n", g_modem_reset_ctrl.ipc_recv_irq_idle);
        return RESET_ERROR;
    }
    return RESET_OK;
}

s32 bsp_reset_msg_init(void)
{
#ifndef CONFIG_IPC_RPROC
    s32 ret;
    struct msgchn_attr lite_attr = { 0 };
    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_MDM_RST;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_LPM);
    lite_attr.lite_notify = mdmreset_msg_lite_proc;

    ret = bsp_msg_lite_open(&g_modem_reset_msghdl, &lite_attr);
    if (ret) {
        reset_print_err("msg reset failed, ret = 0x%x\n", ret);
        return ret;
    }
#endif
    return RESET_OK;
}

s32 reset_create_task(void)
{
    g_modem_reset_ctrl.task = kthread_run(modem_reset_task, NULL, "modem_reset");
    if (g_modem_reset_ctrl.task == NULL) {
        reset_print_err("create modem_reset thread fail!\n");
        return RESET_ERROR;
    }
    return RESET_OK;
}

void bsp_reset_wq_init(void)
{
    init_completion(&(g_modem_reset_ctrl.suspend_completion));
    g_modem_reset_ctrl.pm_notify.notifier_call = reset_pm_notify;
    register_pm_notifier(&g_modem_reset_ctrl.pm_notify);

    g_modem_reset_ctrl.reset_wq = create_singlethread_workqueue("reset_wq");
    if (g_modem_reset_ctrl.reset_wq == NULL) {
        reset_print_err("create reset wq fail!\n");
        system_error(DRV_ERRNO_MODEM_RST_FAIL, 0, 0, NULL, 0);
    }
    INIT_WORK(&(g_modem_reset_ctrl.work_reset), modem_reset_do_work);
    INIT_WORK(&(g_modem_reset_ctrl.work_power_off), modem_power_off_do_work);
    INIT_WORK(&(g_modem_reset_ctrl.work_power_on), modem_power_on_do_work);
}

void reset_feature_ctrl_init(void)
{
    int ret;

    ret = bsp_nv_read(NVA_ID_DRV_CCORE_RESET, (u8 *)&(g_modem_reset_ctrl.nv_config), sizeof(drv_ccore_reset_stru_s));
    if (ret != BSP_OK) {
        reset_feature_on(0);
        reset_print_err("nv read fail ret[%d], reset feature set off!\n", ret);
    }
    reset_print_err("mdm reset nv config:%d.\n", (int)(g_modem_reset_ctrl.nv_config.is_feature_on));
    return;
}

static int reset_init_later(void)
{
    s32 ret;
    ret = bsp_reset_cb_func_register("drv", reset_drv_cb, MDRV_RESET_CB_BEFORE, (int)DRV_RESET_CB_PIOR_ALL);
    if (ret != RESET_OK) {
        reset_print_err("register drv reset callback fail!\n");
        return RESET_ERROR;
    }

    bsp_reset_wq_init();

    if (bsp_reset_ipc_init()) {
        return RESET_ERROR;
    }

    if (bsp_reset_msg_init() != RESET_OK) {
        return RESET_ERROR;
    }

    if (reset_dump_ctrl_init() != RESET_OK) {
        return RESET_ERROR;
    }

    reset_dev_init();
    g_modem_reset_ctrl.state = RESET_INIT_OK;

    return RESET_OK;
}

int bsp_reset_init(void)
{
    s32 ret;

    reset_print_err("start\n");

    reset_feature_ctrl_init();

    if (RESET_OK != reset_of_node_init()) {
        reset_print_err("reset_of_node_init fail!\n");
        return RESET_ERROR;
    }

    ret = reset_var_init();
    if (ret) {
        reset_print_err("reset_var_init fail!\n");
        return ret;
    }

    ret = rild_app_reset_notify_init();
    if (ret) {
        reset_print_err("rild_reset_notify node create fail.\n");
        return RESET_ERROR;
    }

    if (reset_create_task() != RESET_OK) {
        return RESET_ERROR;
    }

    g_modem_reset_ctrl.state = RESET_EARLY_INIT_OK;
    reset_print_err("ok\n");
    return 0;
}

int reset_set_modem_state(enum modem_state_node node, unsigned int state)
{
    int idx = (unsigned int)node - 1;
    if (state >= MODEM_STATE_INVALID) {
        reset_print_err("modem_reset %s:%d invalid state 0x%x.\n", __FUNCTION__, __LINE__, state);
        return -EINVAL;
    }
    reset_print_err("modem_reset  set state %d\n", state);
    if ((idx >= 0) && (idx < MODEM_RESET_NUM)) {
        g_mdm_rst_priv[idx].modem_reset_stat = state;
        g_mdm_rst_priv[idx].flag = 1;
        wake_up_interruptible(&g_mdm_rst_priv[idx].wait);
        return 0;
    } else {
        return -1;
    }
}

static void reset_notify_app(drv_reset_cb_moment_e stage)
{
    reset_print_err("modem_reset reset_notify_app enter. \n");
    if ((MDRV_RESET_CB_BEFORE == stage) && (g_modem_reset_ctrl.reset_action_status == RESET_IS_SUCC)) {
        reset_set_modem_state(modem_reset0_e, 0);
        reset_set_modem_state(modem_reset1_e, 0);
        reset_set_modem_state(modem_reset2_e, 0);
    } else if (MDRV_RESET_CB_AFTER == stage) {
        reset_set_modem_state(modem_reset0_e, 1);
        reset_set_modem_state(modem_reset1_e, 1);
        reset_set_modem_state(modem_reset2_e, 1);
    } else {
        reset_print_err("reset notify app abnormal!\n");
    }
}

static int modem_reset_open(struct inode *inode, struct file *filp)
{
    int i;
    if (inode == NULL) {
        return -EIO;
    }
    if (filp == NULL) {
        return -ENOENT;
    }
    for (i = 0; i < MODEM_RESET_NUM; i++) {
        if (MINOR(g_mdm_rst_priv[i].devt) == iminor(inode)) {
            filp->private_data = &g_mdm_rst_priv[i];
            reset_print_err("modem_reset %s:%d init. devt:%d\n", __FUNCTION__, __LINE__, g_mdm_rst_priv[i].devt);
            return 0;
        }
    }
    return 0;
}
static int modem_reset_release(struct inode *inode, struct file *filp)
{
    if (filp == NULL) {
        return -ENOENT;
    }
    return 0;
}
static ssize_t modem_reset_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t len = 0;
    struct modem_reset_priv_s *priv = NULL;
    if (filp == NULL) {
        return -ENOENT;
    }
    if (buf == NULL) {
        return -EFAULT;
    }
    if (ppos == NULL) {
        return -EFAULT;
    }
    reset_print_err("modem_reset read node. \n");
    priv = (struct modem_reset_priv_s *)filp->private_data;
    if (priv == NULL) {
        reset_print_err("modem_reset %s:%d not init.\n", __FUNCTION__, __LINE__);
        return 0;
    }
    if (priv->modem_reset_stat >= MODEM_STATE_INVALID) {
        reset_print_err("modem_reset : %s:%d Invalid state 0x%x now is set.\n", __FUNCTION__, __LINE__,
            priv->modem_reset_stat);
        return 0;
    }
    if (wait_event_interruptible(priv->wait, priv->flag)) {
        return -ERESTARTSYS;
    }
    priv->flag = 0;
    len = strlen(g_modem_state_str[priv->modem_reset_stat]) + 1;
    reset_print_err("modem_reset pos:%llu, count:%zu, len:%zd\n", *ppos, count, len);
    if (*ppos > 0) {
        reset_print_err("modem_reset ppos is %llu\n", *ppos);
        return 0;
    }
    if (count < len) {
        reset_print_err("modem_reset usr count need larger ,count is 0x%zd\n", len);
        return -EFAULT;
    }
    if (copy_to_user(buf, g_modem_state_str[priv->modem_reset_stat], len)) {
        reset_print_err("modem_reset copy to user fail\n");
        return -EFAULT;
    }
    return len;
}

static unsigned int modem_reset_poll(struct file *filp, poll_table *wait)
{
    struct modem_reset_priv_s *priv = filp->private_data;
    unsigned int mask = 0;
    if (priv != NULL) {
        reset_print_err("modem_reset wait before\n");
        poll_wait(filp, &priv->wait, wait);
        reset_print_err("modem_reset wait after\n");
    }
    if (priv->flag != 0) {
        mask = POLLIN | POLLRDNORM;
        reset_print_err("balong_reset read ready\n");
    }

    reset_print_err("modem_reset wait after1\n");
    return mask;
}

static struct file_operations modem_reset_fops = {
    .owner = THIS_MODULE,
    .open = modem_reset_open,
    .read = modem_reset_read,
    .release = modem_reset_release,
    .poll = modem_reset_poll,
};

static int rild_app_reset_notify_init(void)
{
    int ret;
    unsigned int i;
    dev_t devt;
    struct device *dev = NULL;
    ret = alloc_chrdev_region(&devt, 0, sizeof(g_mdm_rst_priv) / sizeof(struct modem_reset_priv_s), "modem_reset");
    if (ret) {
        reset_print_err("alloc_chrdev_region error");
        return -1;
    }
    for (i = 0; i < MODEM_RESET_NUM; i++) {
        init_waitqueue_head(&g_mdm_rst_priv[i].wait);
        g_mdm_rst_priv[i].devt = MKDEV(MAJOR(devt), i);
        cdev_init(&g_mdm_rst_priv[i].reset_cdev, &modem_reset_fops);
        ret = cdev_add(&g_mdm_rst_priv[i].reset_cdev, g_mdm_rst_priv[i].devt, 1);
        if (ret) {
            reset_print_err("cdev_add err\n");
            break;
        }
        g_mdm_rst_priv[i].class = class_create(THIS_MODULE, g_mdm_rst_priv[i].name);
        if (!g_mdm_rst_priv[i].class) {
            return -1;
        }
        dev = device_create(g_mdm_rst_priv[i].class, NULL, g_mdm_rst_priv[i].devt, &g_mdm_rst_priv[i],
            g_mdm_rst_priv[i].name);
        if (IS_ERR(dev)) {
            reset_print_err("device_create err\n");
            return RESET_ERROR;
        }
    }

    return ret;
}

EXPORT_SYMBOL(bsp_cp_reset);              /*lint !e19 */
EXPORT_SYMBOL(bsp_reset_is_successful);   /*lint !e19 */
EXPORT_SYMBOL(bsp_modem_power_off);       /*lint !e19 */
EXPORT_SYMBOL(bsp_reset_ccore_is_reboot); /*lint !e19 */
EXPORT_SYMBOL(bsp_reset_is_feature_on);   /*lint !e19 */
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_reset_init); /*lint !e19*/
#endif
