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
 *
 */
/*
 * File Name       : onoff_msg_msg.c
 * Description     : recv and send msg to tsp,run in acore
 * History         :
 */

#include <linux/sched.h>
#include <linux/kthread.h>
#include <mdrv_msg.h>
#include <bsp_onoff.h>
#include <bsp_print.h>
#include <bsp_slice.h>
#include "bsp_modem_boot.h"
#include <msg_id.h>
#include <bsp_msg.h>
#include <bsp_reset.h>
#include "onoff_msg.h"
#include "power_off_mbb.h"
#include "power_exchange.h"
#include "power_para.h"

#define THIS_MODU mod_onoff

static msg_chn_t g_onoff_msghdl;
static unsigned int g_modem_tsp_state = 0;
/*
 * 功能描述: tsp状态读取接口函数
 */
int mdrv_sysboot_is_modem_ready(void)
{
    return g_modem_tsp_state;
}

static int onoff_send_msg(unsigned int core, void *msg, u32 msg_len)
{
    int ret;
    struct msg_addr dst;
    dst.core = core;
    dst.chnid = MSG_CHN_ONOFF;
    ret = bsp_msg_lite_sendto(g_onoff_msghdl, &dst, msg, msg_len);
    if (ret) {
        bsp_err("fail to send msg to core(%d), ret = %d\n", core, ret);
    }
    return ret;
}

int onoff_msg_send_to_lpm(power_ctrl_type_e power_type, drv_shutdown_reason_e reason)
{
    tag_ctrl_msg_s msg = { 0 };
    msg.pwr_type = power_type;
    msg.reason = reason;
    return onoff_send_msg(MSG_CORE_LPM, &msg, sizeof(tag_ctrl_msg_s));
}

static void onoff_msg_send_state(void)
{
    tag_ctrl_msg_s msg = { 0 };
    msg.mode = bsp_start_mode_get();
    (void)onoff_send_msg(MSG_CORE_TSP, &msg, sizeof(tag_ctrl_msg_s));
}

static void onoff_msg_handle(tag_ctrl_msg_s *msg_info)
{
    switch (msg_info->pwr_type) {
        case E_POWER_ON_MODE_GET:
            onoff_msg_send_state();
            break;
        case E_MODE_LRCPU_DRV_READY:
            bsp_err("modem tsp drv ready, time slice %d\n", bsp_get_elapse_ms());
            break;
        case E_MODE_LRCPU_READY:
            g_modem_tsp_state = 1;
            bsp_err("onoff modem tsp ready, time slice %d\n", bsp_get_elapse_ms());
            (void)mdrv_set_modem_state(MODEM_INIT_OK);
            bsp_reset_ccore_init_ok();
            if (sysboot_get_monitor_flag() == SYSBOOT_MONITOR_DISABLED) {
                bsp_sysboot_clear_dpart_info();
            }
            sysboot_notifier_process(MODEM_INIT_DONE);
            break;
        case E_MODE_SET_MODEM_STATE:
            bsp_drv_set_modem_state(msg_info->modem_state_info.modem_id, msg_info->modem_state_info.state);
            break;
        case E_POWER_SHUT_DOWN:
            /* Here is to judge whether to shut down or restart */
            if (msg_info->reason == DRV_SHUTDOWN_RESET) {
                bsp_drv_power_reboot();
            } else {
                (void)drv_shut_down(msg_info->reason, POWER_OFF_MONITOR_TIMEROUT);
            }
            break;
        case E_POWER_POWER_OFF:
            /* To shut down right now */
            (void)drv_shut_down(DRV_SHUTDOWN_POWER_KEY, 0);
            break;
        default:
            bsp_err("invalid ctrl by ccore\n");
    }
}

static int onoff_msg_proc(const struct msg_addr *src, void *msg, u32 len)
{
    tag_ctrl_msg_s *msg_info = NULL;

    if (msg == NULL || src == NULL || src->chnid != MSG_CHN_ONOFF || len != sizeof(tag_ctrl_msg_s)) {
        bsp_err("msg is null or chnid(%d) is error or len(%u) is error.\n", src->chnid, len);
        return 0;
    }

    msg_info = (tag_ctrl_msg_s *)msg;
    onoff_msg_handle(msg_info);

    return 0;
}

int onoff_msg_init(void)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };

    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_ONOFF;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = onoff_msg_proc;
    ret = bsp_msg_lite_open(&g_onoff_msghdl, &lite_attr);
    if (ret) {
        bsp_err("onoff msg open err, ret=0x%x\n", ret);
        return -1;
    }
    return 0;
}
