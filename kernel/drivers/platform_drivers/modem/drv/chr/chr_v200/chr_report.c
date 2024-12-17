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

#include <product_config.h>
#include <securec.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <msg_id.h>
#include <mdrv_msg.h>
#include <mdrv_chr.h>
#include <msg_id.h>
#include <bsp_slice.h>
#include <bsp_homi.h>
#include <bsp_chr.h>
#include "chr_core.h"
#include "chr_cfg.h"
#include "chr_report.h"
#include "chr_debug.h"

int chr_cnf_msg_report(chr_comm_head_s *req_msg, unsigned int result)
{
    int ret;
    chr_app_cnf_s *cnf_msg = (chr_app_cnf_s *)osl_malloc(sizeof(chr_app_cnf_s));
    if (cnf_msg == NULL) {
        chr_error("cnf_msg malloc fail\n");
        return CHR_MEM_ALLOC_ERR;
    }
    (void)memset_s(cnf_msg, sizeof(chr_app_cnf_s), 0, sizeof(chr_app_cnf_s));

    cnf_msg->oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    cnf_msg->oam_head.mt_2b = MSP_OAM_FRAME_MT_CNF;
    cnf_msg->oam_head.timestamp = bsp_get_slice_value();

    cnf_msg->info_head.msg_type = OM_ERR_LOG_MSG_CFG_CNF;
    cnf_msg->info_head.msg_sn = req_msg->info_head.msg_sn;
    cnf_msg->info_head.msg_len = sizeof(chr_app_cnf_s) - CHR_COMM_HEAD_LEN;

    if (req_msg->info_head.msg_type == OM_ERR_LOG_MSG_SET_ERR_REPORT) {
        cnf_msg->module_id = ((chr_app_req_s *)req_msg)->module_id;
        cnf_msg->modem_id = ((chr_app_req_s *)req_msg)->modem_id;
        cnf_msg->alarm_id = ((chr_app_req_s *)req_msg)->alarm_id;
    }else {
        cnf_msg->module_id = req_msg->info_head.msg_type;
        cnf_msg->modem_id = 0;
        cnf_msg->alarm_id = 0;
    }
    cnf_msg->result = result;
    cnf_msg->msg_sn = DRV_MID_CHR_APSS;

    ret = bsp_homi_genl_send_data(GENL_CHAN_CHR, (unsigned char *)cnf_msg, sizeof(chr_app_cnf_s));
    osl_free(cnf_msg);
    return ret;
}
EXPORT_SYMBOL(chr_cnf_msg_report);

void chr_reset_info_report(void)
{
    chr_app_cnf_s reset_msg = {0};
    static unsigned int reset_cnt = 0;

    if (chr_get_connect_state() != CHR_STATE_CONNECT) {
        return;
    }
   
    reset_msg.oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    reset_msg.oam_head.mt_2b = MSP_OAM_FRAME_MT_CNF;
    reset_msg.oam_head.timestamp = bsp_get_slice_value();

    reset_msg.info_head.msg_type = OM_ERR_LOG_RESET_INFO_REPORT;
    reset_msg.info_head.msg_sn = ++reset_cnt;
    reset_msg.info_head.msg_len = sizeof(chr_app_cnf_s) - CHR_COMM_HEAD_LEN; 

    reset_msg.msg_sn = DRV_MID_CHR_APSS;
    reset_msg.result = MSG_ID_MODEM_RESET;

    (void)bsp_homi_genl_send_data(GENL_CHAN_CHR, (unsigned char *)&reset_msg, sizeof(chr_app_cnf_s));
}
EXPORT_SYMBOL(chr_reset_info_report);

int chr_report_to_app(chr_ind_info_s *chr_data)
{
    chr_app_ind_s *app_ind = NULL;
    int ret;
    unsigned int ind_len = sizeof(chr_app_ind_s) + chr_data->msg_len;

   app_ind = (chr_app_ind_s *)osl_malloc(ind_len);
    if (app_ind == NULL) {
        chr_error("app_ind malloc fail, ind_len=0x%x\n", ind_len);
        return BSP_ERROR;
    }
    (void)memset_s(app_ind, ind_len, 0, ind_len);

    app_ind->oam_head.sid_4b = MSP_OAM_FRAME_SID_CHR;
    app_ind->oam_head.mt_2b = MSP_OAM_FRAME_MT_IND;
    app_ind->oam_head.timestamp = bsp_get_slice_value();
    ret = memcpy_s((unsigned char *)(&app_ind->msg_type), ind_len - sizeof(chr_oam_head_s),
        (unsigned char *)(&chr_data->msg_type), ind_len - sizeof(chr_oam_head_s));
    if (ret) {
        chr_error("unipack memcpy_s fail, ret=0x%x\n", ret);
        osl_free(app_ind);
        return ret;
    }

    ret = bsp_homi_genl_send_data(GENL_CHAN_CHR, (u8 *)app_ind, ind_len);

    osl_free(app_ind);

    return ret;
}

unsigned int bsp_chr_report(chr_ind_info_s *chr_data)
{
    unsigned int ret;

    if ((chr_data == NULL) || (chr_data->msg_len > CHR_FRAGMENT_LEN_MAX)) {
        chr_error("parameter is null\n");
        return (unsigned int)BSP_ERROR;
    }

    ret = chr_get_connect_state();
    if (ret != CHR_STATE_CONNECT) {
        chr_info("chr disconnect\n");
        return (unsigned int)BSP_ERROR;
    }

    ret = chr_check_blocklist(chr_data->report_head.module_id,
        chr_data->report_head.alarm_id, chr_data->report_head.alarm_type);
    if (ret == CHR_TRUE) {
        chr_info("chr in blocklist: module_id=0x%x, alarm_id=0x%x, alarm_type=0x%x\n",
            chr_data->report_head.module_id, chr_data->report_head.alarm_id, chr_data->report_head.alarm_type);
        return CHR_BLOCKLIST_NOT_ALLOW;
    }

    return (unsigned int)chr_report_to_app(chr_data);
}
EXPORT_SYMBOL(bsp_chr_report);
