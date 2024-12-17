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


#include <linux/version.h>
#include <securec.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <osl_malloc.h>
#include <osl_sem.h>
#include <osl_sem.h>
#include "bsp_sn.h"
#include "bsp_version.h"
#include <mdrv_timer.h>
#include "mprobe_service.h"
#include "mprobe_message.h"
#include "mprobe_channel_manager.h"
#include "mprobe_debug.h"

#undef THIS_MODU
#define THIS_MODU mod_mprobe

mprobe_srv_ctrl_s g_mprobe_srv_ctrl;
mprobe_service_s g_mprobe_service[MPROBE_COMPID_BUTT] = {0};
mprobe_conn_state_e g_mprobe_conn_state;

int mprobe_get_conn_state(void)
{
    return g_mprobe_conn_state;
}

int mprobe_send_to_ccpu(void *data, u32 len)
{
    int ret;

    ret = mprobe_msg_send(data, len);
    if (ret) {
        mprobe_error("send msg to ccpu mprobe fail");
        return BSP_ERROR;
    }

    return ret;
}

void mprobe_get_board_info(mprobe_get_board_info_s *conn_msg, mprobe_conn_info_s *ver_msg)
{
    s32 ret;
    char serial_number[MPROBE_SN_LEN] = {0};

    ver_msg->auid = conn_msg->auid;

#ifndef BSP_CONFIG_PHONE_TYPE
    ret = bsp_usb_get_serial_no(serial_number, MPROBE_SN_LEN);
    if (ret != 0) {
        mprobe_error("get sn fail:%x\n", ret);
    }
    ret = memcpy_s(ver_msg->sn, MPROBE_SN_LEN, serial_number, MPROBE_SN_LEN - 1);
    if (ret != EOK) {
        mprobe_error("memcpy sn fail, ret=0x%x\n", ret);
    }
#endif
    mprobe_crit("serial_number: %s\n", ver_msg->sn);

    (void)mdrv_timer_get_accuracy_timestamp((u32 *)&ver_msg->time_stamp[MPROBE_TMST_LEN], (u32 *)&ver_msg->time_stamp[0]);

    ver_msg->authver = 0;
    ver_msg->authflag = 0;

    ret = memset_s(ver_msg->product_info, (u32)sizeof(ver_msg->product_info), 0, (u32)sizeof(ver_msg->product_info));
    if (ret != EOK) {
        mprobe_error("memset fail:%x\n", ret);
    }

    ret = memcpy_s(ver_msg->product_info, (u32)sizeof(ver_msg->product_info), bsp_version_get_release_ver(),
        strnlen(bsp_version_get_release_ver(), sizeof(ver_msg->product_info) - 1));
    if (ret != EOK) {
        mprobe_error("memcpy product fail, ret=0x%x\n", ret);
    }
}


void bsp_mprobe_fill_srv_head(mprobe_srv_head_s *mprobe_head)
{
    mprobe_head->header.magic = MPROBE_ODT_HEAD_MAGIC;
    mprobe_head->header.data_len = 0;

    mprobe_head->frame_header.srv_head.sid = OMP_SERVER_ID_MPROBE;
    mprobe_head->frame_header.srv_head.ver = 0;
    mprobe_head->frame_header.srv_head.mdmid = 0;
    mprobe_head->frame_header.srv_head.rsv = 0;
    mprobe_head->frame_header.srv_head.ssid = MPROBE_SSID_ACPU;

    mprobe_head->frame_header.srv_head.msgtyp = 0;
    mprobe_head->frame_header.srv_head.sidx = 0;
    mprobe_head->frame_header.srv_head.esflag = 0;
    /* 默认不分包 */
    mprobe_head->frame_header.srv_head.sflag = 0;
    mprobe_head->frame_header.srv_head.srctransid = 0;
    mprobe_head->frame_header.srv_head.odttransid = 0;
    mprobe_head->frame_header.srv_head.timestamp[0] = mdrv_timer_get_normal_timestamp();
}

void bsp_mprobe_notify_reg(mprobe_compid_e compid, mprobe_notify_func notify_fn)
{
    if (compid >= MPROBE_COMPID_BUTT) {
        return;
    }

    g_mprobe_service[compid].compid = compid;
    g_mprobe_service[compid].notify_fn = notify_fn;
}

void mprobe_conn_proc(u8 *conn_msg, u32 len)
{
    int ret;
    mprobe_frame_head_s *mprobe_head = NULL;
    mprobe_srv_head_s mprobe_srv_head;
    mprobe_msg_report_head_s mprobe_msg;
    mprobe_get_board_info_s *cmd_info = NULL;
    mprobe_conn_info_s cnf_msg;

    mprobe_head = (mprobe_frame_head_s *)conn_msg;
    cmd_info = (mprobe_get_board_info_s *)mprobe_head->data;

    (void)memset_s(&cnf_msg, sizeof(cnf_msg), 0, sizeof(cnf_msg));

    mprobe_get_board_info(cmd_info, &cnf_msg);

    mprobe_crit("receive tool conn cmd\n");

    ((mprobe_frame_head_s *)conn_msg)->srv_head.ssid = MPROBE_SSID_CCPU;
    cnf_msg.result = BSP_OK;
    cnf_msg.result = mprobe_send_to_ccpu((void *)conn_msg, len);

    ret = mprobe_channel_start();
    if (ret) {
        mprobe_error("mprobe channel start fail, ret:0x%x\n", ret);
        return;
    }

    g_mprobe_conn_state = MPROBE_STATE_CONNECT;

    bsp_mprobe_fill_srv_head(&mprobe_srv_head);

    mprobe_srv_head.frame_header.msg_id.msgid_16b = MPROBE_MSGID_HOST_CONNECT;
    mprobe_srv_head.frame_header.msg_id.comp_8b = MPROBE_COMPID_DEFAULT;
    mprobe_srv_head.frame_header.msg_id.rsv_8b = 0;

    mprobe_srv_head.frame_header.msg_len.msg_len = sizeof(mprobe_conn_info_s);
    mprobe_srv_head.frame_header.msg_len.resv = 0;

    mprobe_msg.header = &mprobe_srv_head;
    mprobe_msg.header_size = sizeof(mprobe_srv_head_s);
    mprobe_msg.data = (u8 *)&cnf_msg;
    mprobe_msg.data_size = sizeof(mprobe_conn_info_s);

    ret = bsp_mprobe_data_report(&mprobe_msg);
    if (ret) {
        mprobe_error("mprobe connect msg report fail, ret=0x%x\n", ret);
    }

    return;
}

void mprobe_disconn_proc(u8 *disconn_msg, u32 len)
{
    int i;
    int ret;
    int conn_state;

    conn_state = mprobe_get_conn_state();
    if (conn_state == MPROBE_STATE_DISCONNECT) {
        return;
    }

    mprobe_crit("mprobe acore disconnect\n");

    ((mprobe_frame_head_s *)disconn_msg)->srv_head.ssid = MPROBE_SSID_CCPU;
    ret = mprobe_send_to_ccpu((void *)disconn_msg, len);
    if (ret) {
        mprobe_error("send disconn msg to ccpu fail");
    }

    g_mprobe_conn_state = MPROBE_STATE_DISCONNECT;
    // 通知各组件回调
    for (i = 0; i < MPROBE_COMPID_BUTT; i++) {
        if (g_mprobe_service[i].notify_fn != NULL) {
            (void)g_mprobe_service[i].notify_fn(MPROBE_STATE_DISCONNECT);
        }
    }

    mprobe_channel_stop();

    return;
}

int mprobe_notify_port_disconn(void)
{
    u32 len = sizeof(mprobe_frame_head_s);
    mprobe_frame_head_s *mprobe_msg;

    mprobe_msg = (mprobe_frame_head_s *)osl_malloc(len);
    if (mprobe_msg == NULL) {
        return BSP_ERROR;
    }
    (void)memset_s(mprobe_msg, len, 0, len);

    mprobe_msg->srv_head.sid = OMP_SERVER_ID_MPROBE;
    mprobe_msg->srv_head.ssid = MPROBE_SSID_CCPU;

    mprobe_msg->msg_id.msgid_16b = MPROBE_MSGID_HOST_DISCONNECT;
    mprobe_msg->msg_id.comp_8b = MPROBE_COMPID_DEFAULT;

    mprobe_disconn_proc((void *)mprobe_msg, len);

    osl_free(mprobe_msg);
    return BSP_OK;
}

int mprobe_msg_proc(void *data, u32 len)
{
    int msg_id;

    if (data == NULL || len == 0) {
        mprobe_error("process param err\n");
        return BSP_ERROR;
    }

    msg_id = MPROBE_PAYLOAD_MSG_ID(data);
    if (msg_id >= MPROBE_MSGID_BUTT) {
        mprobe_error("msg_id:0x%x is more tham max\n", msg_id);
        return BSP_ERROR;
    }

    switch (msg_id) {
        case MPROBE_MSGID_HOST_CONNECT:
            mprobe_conn_proc((u8 *)data, len);
            mprobe_crit("mprobe acore connect\n");
            break;

        case MPROBE_MSGID_HOST_DISCONNECT:
            mprobe_disconn_proc((u8 *)data, len);
            break;

        default:
            mprobe_error("msg_id:0x%x is not support\n", msg_id);
            return BSP_ERROR;
    }

    return BSP_OK;
}

void bsp_mprobe_func_reg(mprobe_compid_e compid, mprobe_service_func srv_fn)
{
    if (compid >= MPROBE_COMPID_BUTT) {
        return;
    }

    g_mprobe_service[compid].compid = compid;
    g_mprobe_service[compid].srv_func = srv_fn;
}

static u32 mprobe_distribute_comp_data(void *data, u32 len)
{
    int ret;
    int comp_id;

    if (data == NULL || len == 0) {
        mprobe_error("process param err\n");
        return BSP_ERROR;
    }

    comp_id = MPROBE_PAYLOAD_COMP_ID(data);
    mprobe_debug("comp_id:0x%x\n", comp_id);
    if (comp_id >= MPROBE_COMPID_BUTT) {
        mprobe_error("comp_id:0x%x is not support\n", comp_id);
        return BSP_ERROR;
    }

    if (g_mprobe_service[comp_id].srv_func == NULL) {
        mprobe_error("comp_id:0x%x have not register handler\n", comp_id);
        return BSP_ERROR;
    }

    ret = g_mprobe_service[comp_id].srv_func((void *)data, len);
    return ret;
}

static int mprobe_forward_data(void *data, u32 len)
{
    int ret;
    int ssid;

    if (data == NULL || len == 0) {
        mprobe_error("process param err\n");
        return BSP_ERROR;
    }

    ssid = MPROBE_SUB_SYS_ID(data);
    mprobe_debug("ssid:0x%x\n", ssid);
    switch (ssid) {
        case MPROBE_SSID_ACPU:
            /* 本核处理 */
            ret = mprobe_distribute_comp_data(data, len);
            break;
        case MPROBE_SSID_CCPU:
            /* 转交CCPU处理 */
            ret = mprobe_send_to_ccpu(data, len);
            break;
        default:
            mprobe_error("ssid:0x%x not regist\n", ssid);
            ret = BSP_ERROR;
            break;
    }

    return ret;
}

/*
 * Function Name: mprobe_srv_proc
 * Description: mprobe service 处理函数
 */
u32 mprobe_srv_proc(void *data, u32 len)
{
    int ret;

    if ((data == NULL) || (len < sizeof(omp_service_head_s))) {
        mprobe_error("para error:data_len=0x%x\n", len);
        return BSP_ERROR;
    }
    if (OMP_SERVER_ID(data) == OMP_SERVER_ID_MPROBE) {
        /* 开始处理，不允许睡眠 */
        __pm_stay_awake(g_mprobe_srv_ctrl.wakeup_lock);
        ret = mprobe_forward_data((void *)data, len);
        /* 处理结束，允许睡眠 */
        __pm_relax(g_mprobe_srv_ctrl.wakeup_lock);
    } else {
        ret = BSP_ERROR;
    }

    return ret;
}

void mprobe_srv_init(void)
{
    g_mprobe_srv_ctrl.wakeup_lock = wakeup_source_register(NULL, "mprobe_srv_lock");
    if (g_mprobe_srv_ctrl.wakeup_lock == NULL) {
        mprobe_error("wakeup_source_register err\n");
    }
    bsp_omp_server_reg(OMP_SERVER_ID_MPROBE, mprobe_srv_proc);

    bsp_mprobe_func_reg(MPROBE_COMPID_DEFAULT, mprobe_msg_proc);
}

int bsp_mprobe_data_report(mprobe_msg_report_head_s *data)
{
    int ret;

    ret = mprobe_get_conn_state();
    if (ret == MPROBE_STATE_DISCONNECT) {
        return BSP_ERROR;
    }

    if (data == NULL) {
        mprobe_error("report data check fail!\n");
        return BSP_ERROR;
    }

    ret = mprobe_srv_pack_data(data);
    if (ret != BSP_OK) {
        mprobe_error("report data error!");
        return BSP_ERROR;
    }

    return ret;
}
