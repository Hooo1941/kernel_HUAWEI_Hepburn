/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include <securec.h>
#include <mdrv_msg.h>
#include <mdrv_timer.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <msg_id.h>
#include <bsp_slice.h>
#include <bsp_chr.h>
#include "chr_core.h"
#include "chr_cfg.h"
#include "chr_report.h"
#include "chr_debug.h"

chr_cfg_info_s g_chr_cfg_info;

unsigned int chr_get_connect_state(void)
{
    return g_chr_cfg_info.conn_state;
}

int chr_app_query_proc(unsigned char *data, unsigned int data_len)
{
    int ret;

    ret  = chr_msglite_send(data, data_len);
    if (ret) {
        return chr_cnf_msg_report((chr_comm_head_s *)data, (unsigned int)ret);
    }

    return 0;
}

int chr_connect_cfg_proc(unsigned char *data, unsigned int data_len)
{
    chr_app_conn_info_s *conn_state = NULL;
    int ret;

    if (data_len <= sizeof(chr_app_req_s)) {
        chr_error("data_len invalid, len=0x%x\n", data_len);
        ret = CHR_INVALID_LEN;
        goto CHR_ERROR;
    }

    ret = chr_msglite_send(data, data_len);
    if (ret) {
        chr_error("send conn cfg fail, ret=0x%x\n", ret);
        goto CHR_ERROR;
    }

    conn_state = (chr_app_conn_info_s *)((chr_app_req_s *)data)->data;
    g_chr_cfg_info.conn_state = conn_state->conn_state;
    chr_error("conn state: 0x%x\n", g_chr_cfg_info.conn_state);
    return 0;

CHR_ERROR:
    return chr_cnf_msg_report((chr_comm_head_s *)data, (unsigned int)ret);
}

static void chr_conn_state_send_to_tsp(void)
{
    chr_app_req_s *send_data = NULL;
    chr_app_conn_info_s *conn_info = NULL;
    unsigned int len;
    int ret;

    len = sizeof(chr_app_req_s) + sizeof(chr_app_conn_info_s);
    send_data = (chr_app_req_s *)osl_malloc(len);
    if (send_data == NULL) {
        chr_error("osl malloc fail, len=0x%x\n", len);
        return;
    }
    (void)memset_s(send_data, len, 0, len);

    send_data->oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    send_data->oam_head.mt_2b = MSP_OAM_FRAME_MT_REQ;
    send_data->oam_head.timestamp = bsp_get_slice_value();
    send_data->info_head.msg_type = OM_ERR_LOG_MSG_CONNECT_CFG;
    send_data->info_head.msg_len = sizeof(chr_app_conn_info_s) + CHR_APP_REQ_MODULE_INFO_LEN;
    conn_info = (chr_app_conn_info_s *)send_data->data;
    conn_info->conn_state = g_chr_cfg_info.conn_state;

    ret = chr_msglite_send(send_data, len);
    if (ret) {
        chr_error("send conn state fail, ret=0x%x\n", ret);
    }

    osl_free(send_data);
    return;
}

static void chr_blocklist_send_to_tsp(void)
{
    chr_app_blocklist_cfg_s *send_data = NULL;
    unsigned int data_len = g_chr_cfg_info.blocklist_cfg.blocklist_num * sizeof(chr_blocklist_info_s);
    unsigned int total_len = sizeof(chr_app_blocklist_cfg_s) + data_len;
    int ret;

    send_data = (chr_app_blocklist_cfg_s *)osl_malloc(total_len);
    if (send_data == NULL) {
        chr_error("send_data malloc fail\n");
        return;
    }
    (void)memset_s(send_data, total_len, 0, total_len);

    send_data->oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    send_data->oam_head.mt_2b = MSP_OAM_FRAME_MT_REQ;
    send_data->oam_head.timestamp = bsp_get_slice_value();

    send_data->info_head.msg_type = OM_ERR_LOG_MSG_BLOCKLIST_CFG;
    send_data->info_head.msg_len = data_len;

    ret = memcpy_s((unsigned char *)send_data->blocklist_cfg, data_len,
        g_chr_cfg_info.blocklist_cfg.blocklist_info , data_len);
    if (ret) {
        chr_error("blocklist info copy fail, ret=0x%x,len=0x%x,\n",ret, data_len);
        goto CHR_END;
    }

    ret = chr_msglite_send(send_data, total_len);
    if (ret) {
        chr_error("send blocklist info to tsp fail, ret=0x%x\n", ret);
    }

CHR_END:
    osl_free(send_data);
    return;
}

int chr_blocklist_cfg_proc(unsigned char *data, unsigned int data_len)
{
    chr_app_blocklist_cfg_s *blocklist_data = (chr_app_blocklist_cfg_s *)data;
    unsigned int msg_len = blocklist_data->info_head.msg_len;
    unsigned int blocklist_num;
    unsigned char *blocklist_info;
    unsigned long lock_flag;
    int ret;

    if (msg_len < sizeof(chr_blocklist_info_s)) {
        chr_error("blocklist info len invalid, len=0x%x\n", msg_len);
        ret = CHR_INVALID_LEN;
        goto CHR_ERROR;
    }

    blocklist_num = msg_len / sizeof(chr_blocklist_info_s);
    blocklist_info = (unsigned char *)osl_malloc(msg_len);
    if (blocklist_info == NULL) {
        chr_error("malloc blocklist info fail, len=0x%x\n", msg_len);
        ret = CHR_MEM_ALLOC_ERR;
        goto CHR_ERROR;
    }

    ret = memcpy_s(blocklist_info, msg_len,
        (unsigned char *)blocklist_data->blocklist_cfg, msg_len);
    if (ret) {
        chr_error("memcpy blocklist info fail, msg_len=0x%x\n", msg_len);
        osl_free(blocklist_info);
        ret = CHR_MEM_COPY_ERR;
        goto CHR_ERROR;
    }

    spin_lock_irqsave(&g_chr_cfg_info.blocklist_cfg.blocklist_lock, lock_flag);
    if (g_chr_cfg_info.blocklist_cfg.blocklist_info != NULL) {
        osl_free(g_chr_cfg_info.blocklist_cfg.blocklist_info);
        g_chr_cfg_info.blocklist_cfg.blocklist_info = NULL;
    }
    g_chr_cfg_info.blocklist_cfg.blocklist_info = blocklist_info;
    g_chr_cfg_info.blocklist_cfg.blocklist_num = blocklist_num;
    spin_unlock_irqrestore(&g_chr_cfg_info.blocklist_cfg.blocklist_lock, lock_flag);

    ret = chr_msglite_send(data, data_len);
    if (ret) {
        chr_error("blocklist info send fail, ret=0x%x\n", ret);
        goto CHR_ERROR;
    }
    return 0;

CHR_ERROR:
    return chr_cnf_msg_report((chr_comm_head_s *)data, (unsigned int)ret);
}

static void chr_prio_info_send_to_tsp(void)
{
    chr_app_priority_cfg_s *send_data = NULL;
    unsigned int data_len = g_chr_cfg_info.prio_cfg.prio_num * sizeof(chr_priority_info_s);
    unsigned int total_len = sizeof(chr_app_priority_cfg_s) + data_len;
    int ret;

    send_data = (chr_app_priority_cfg_s *)osl_malloc(total_len);
    if (send_data == NULL) {
        chr_error("send_data malloc fail\n");
        return;
    }
    (void)memset_s(send_data, total_len, 0, total_len);

    send_data->oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    send_data->oam_head.mt_2b = MSP_OAM_FRAME_MT_REQ;
    send_data->oam_head.timestamp = bsp_get_slice_value();

    send_data->info_head.msg_type = OM_ERR_LOG_MSG_PRIORITY_CFG;
    send_data->info_head.msg_len = data_len;

    ret = memcpy_s((unsigned char *)send_data->priority_cfg, data_len,
        g_chr_cfg_info.prio_cfg.prio_info , data_len);
    if (ret) {
        chr_error("priority info copy fail, ret=0x%x,len=0x%x,\n",ret, data_len);
        goto CHR_END;
    }

    ret = chr_msglite_send(send_data, total_len);
    if (ret) {
        chr_error("send prio info to tsp fail, ret=0x%x\n", ret);
    }

CHR_END:
    osl_free(send_data);
    return;
}

int chr_priority_cfg_proc(unsigned char *data, unsigned int data_len)
{
    chr_app_priority_cfg_s *prio_data = (chr_app_priority_cfg_s *)data;
    unsigned int msg_len = prio_data->info_head.msg_len;
    unsigned int prio_num;
    unsigned char *prio_info = NULL;
    int ret;

    if (msg_len < sizeof(chr_priority_info_s)) {
        chr_error("prio info len invalid, len=0x%x\n", msg_len);
        ret = CHR_INVALID_LEN;
        goto CHR_ERROR;
    }

    prio_num = msg_len / sizeof(chr_priority_info_s);
    prio_info = (unsigned char *)osl_malloc(msg_len);
    if (prio_info == NULL) {
        chr_error("malloc priority info fail, len=0x%x\n", msg_len);
        ret = CHR_MEM_ALLOC_ERR;
        goto CHR_ERROR;
    }

    ret = memcpy_s(prio_info, msg_len,
        (unsigned char *)prio_data->priority_cfg, msg_len);
    if (ret) {
        chr_error("memcpy priority info fail, msg_len=0x%x\n", msg_len);
        osl_free(prio_info);
        ret = CHR_MEM_COPY_ERR;
        goto CHR_ERROR;
    }

    if (g_chr_cfg_info.prio_cfg.prio_info != NULL) {
        osl_free(g_chr_cfg_info.prio_cfg.prio_info);
        g_chr_cfg_info.prio_cfg.prio_info = NULL;
    }
    g_chr_cfg_info.prio_cfg.prio_info = prio_info;
    g_chr_cfg_info.prio_cfg.prio_num = prio_num;

    ret = chr_msglite_send(data, data_len);
    if (ret) {
        goto CHR_ERROR;
    }
    return 0;

CHR_ERROR:
    return chr_cnf_msg_report((chr_comm_head_s *)data, (unsigned int)ret);
}

static void chr_period_info_send_to_tsp(void)
{
    chr_app_period_cfg_s *send_data = NULL;
    unsigned int data_len = g_chr_cfg_info.period_cfg.period_num * sizeof(chr_period_info_s);
    unsigned int total_len = sizeof(chr_app_period_cfg_s) + data_len;
    int ret;

    send_data = (chr_app_period_cfg_s *)osl_malloc(total_len);
    if (send_data == NULL) {
        chr_error("send_data malloc fail\n");
        return;
    }
    (void)memset_s(send_data, total_len, 0, total_len);

    send_data->oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    send_data->oam_head.mt_2b = MSP_OAM_FRAME_MT_REQ;
    send_data->oam_head.timestamp = bsp_get_slice_value();

    send_data->info_head.msg_type = OM_ERR_LOG_MSG_PERIOD_CFG;
    send_data->info_head.msg_len = data_len;

    ret = memcpy_s((unsigned char *)send_data->period_cfg, data_len,
        g_chr_cfg_info.period_cfg.period_info , data_len);
    if (ret) {
        chr_error("period info copy fail, ret=0x%x,len=0x%x,\n",ret, data_len);
        goto CHR_END;
    }

    ret = chr_msglite_send(send_data, total_len);
    if (ret) {
        chr_error("send period info to tsp fail, ret=0x%x\n", ret);
    }

CHR_END:
    osl_free(send_data);
    return;
}

int chr_period_cfg_proc(unsigned char *data, unsigned int data_len)
{
    chr_app_period_cfg_s *period_data = (chr_app_period_cfg_s *)data;
    unsigned int msg_len = period_data->info_head.msg_len;
    unsigned int period_num;
    unsigned char *period_info = NULL;
    int ret;

    if (msg_len < sizeof(chr_period_info_s)) {
        chr_error("period info len invalid, len=0x%x\n", msg_len);
        ret = CHR_INVALID_LEN;
        goto CHR_ERROR;
    }

    period_num = msg_len / sizeof(chr_period_info_s);
    period_info = (unsigned char *)osl_malloc(msg_len);
    if (period_info == NULL) {
        chr_error("malloc period info fail, len=0x%x\n", msg_len);
        ret = CHR_MEM_ALLOC_ERR;
        goto CHR_ERROR;
    }

    ret = memcpy_s(period_info, msg_len,
        (unsigned char *)period_data->period_cfg, msg_len);
    if (ret) {
        chr_error("memcpy period info fail, msg_len=0x%x\n", msg_len);
        ret = CHR_MEM_COPY_ERR;
        goto CHR_ERROR;
    }

    if (g_chr_cfg_info.period_cfg.period_info != NULL) {
        osl_free(g_chr_cfg_info.period_cfg.period_info);
        g_chr_cfg_info.period_cfg.period_info = NULL;
    }
    g_chr_cfg_info.period_cfg.period_info = period_info;
    g_chr_cfg_info.period_cfg.period_num = period_num;

    ret = chr_msglite_send(data, data_len);
    if (ret) {
        goto CHR_ERROR;
    }
    return 0;

CHR_ERROR:
    return chr_cnf_msg_report((chr_comm_head_s *)data, (unsigned int)ret);
}

void chr_cfg_send_to_modem(void)
{
    if (chr_get_connect_state() != CHR_STATE_CONNECT) {
        return;
    }   
    chr_conn_state_send_to_tsp();

    if ((g_chr_cfg_info.blocklist_cfg.blocklist_info != NULL) &&
        (g_chr_cfg_info.blocklist_cfg.blocklist_num != 0)) {
        chr_blocklist_send_to_tsp();
    }

    if ((g_chr_cfg_info.prio_cfg.prio_info != NULL) &&
        (g_chr_cfg_info.prio_cfg.prio_num != 0)) {
        chr_prio_info_send_to_tsp();
    }

    if ((g_chr_cfg_info.period_cfg.period_info != NULL) &&
        (g_chr_cfg_info.period_cfg.period_num != 0)) {
        chr_period_info_send_to_tsp();
    }

    return;
}
EXPORT_SYMBOL(g_chr_cfg_info);

unsigned int chr_check_blocklist(unsigned int module_id, unsigned short alarm_id, unsigned short alarm_type)
{
    chr_blocklist_info_s *cfg_ptr = NULL;
    unsigned long lock_flag;
    unsigned int i;

    spin_lock_irqsave(&g_chr_cfg_info.blocklist_cfg.blocklist_lock, lock_flag);
    for (i = 0; i < g_chr_cfg_info.blocklist_cfg.blocklist_num; i++) {
        cfg_ptr = (chr_blocklist_info_s *)(g_chr_cfg_info.blocklist_cfg.blocklist_info + i * sizeof(chr_blocklist_info_s));
        if (alarm_type == OM_DIRECT_REPORT_TYPE) {
            if ((cfg_ptr->module_id == module_id) && (cfg_ptr->alarm_id == alarm_id) &&
                (cfg_ptr->alarm_type == alarm_type)) {
                spin_unlock_irqrestore(&g_chr_cfg_info.blocklist_cfg.blocklist_lock, lock_flag);
                return CHR_TRUE;
            }
        } else {
            if ((cfg_ptr->module_id == module_id) && (cfg_ptr->alarm_id == alarm_id) &&
                (cfg_ptr->alarm_type != OM_DIRECT_REPORT_TYPE)) {
                spin_unlock_irqrestore(&g_chr_cfg_info.blocklist_cfg.blocklist_lock, lock_flag);
                return CHR_TRUE;
            }
        }
    }
    spin_unlock_irqrestore(&g_chr_cfg_info.blocklist_cfg.blocklist_lock, lock_flag);
    return CHR_FALSE;
}

void chr_cfg_init(void)
{
    spin_lock_init(&g_chr_cfg_info.blocklist_cfg.blocklist_lock);
}

