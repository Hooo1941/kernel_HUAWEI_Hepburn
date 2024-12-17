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
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <securec.h>
#include <mdrv_sysboot.h>
#include <mdrv_msg.h>
#include <mdrv_timer.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <msg_id.h>
#include <bsp_homi.h>
#include "chr_core.h"
#include "chr_cfg.h"
#include "chr_report.h"
#include "chr_debug.h"

chr_core_info_s g_chr_core_info; // 储存msg相关配置的全局变量
const chr_cmd_table_s g_chr_cmd_table[] = {
    {OM_ERR_LOG_MSG_SET_ERR_REPORT, chr_app_query_proc},
    {OM_ERR_LOG_MSG_BLOCKLIST_CFG, chr_blocklist_cfg_proc},
    {OM_ERR_LOG_MSG_PRIORITY_CFG, chr_priority_cfg_proc},
    {OM_ERR_LOG_MSG_PERIOD_CFG, chr_period_cfg_proc},
    {OM_ERR_LOG_MSG_CONNECT_CFG, chr_connect_cfg_proc},
};

int chr_app_data_handle(unsigned char *data, unsigned int data_len)
{
    chr_app_req_s *app_data = NULL;
    unsigned int ret;
    int i;

    if (data == NULL) {
        chr_error("data is null\n");
        ret = CHR_PARAMETER_NULL;
        goto CHR_ERROR;
    }

    if ((data_len < CHR_COMM_HEAD_LEN) || (data_len > CHR_MAX_CFG_LEN)) {
        chr_error("data_len invalid, len=0x%x\n", data_len);
        ret = CHR_INVALID_LEN;
        goto CHR_ERROR;
    }

    app_data = (chr_app_req_s *)data;
    if (app_data->info_head.msg_len + CHR_COMM_HEAD_LEN != data_len) {
        chr_error("msg_len is valid, msg_len=0x%x\n", app_data->info_head.msg_len);
        ret = CHR_INVALID_LEN;
        goto CHR_ERROR;
    }

    for (i = 0; i < ARRAY_SIZE(g_chr_cmd_table); i ++) {
        if (app_data->info_head.msg_type == g_chr_cmd_table[i].cmd) {
            return g_chr_cmd_table[i].cmd_handler(data, data_len);
        }
    }
    chr_error("cmd invalid, cmd=0x%x\n", app_data->info_head.msg_type);
    ret = CHR_INVALID_CMD;

CHR_ERROR:    
    chr_debug_set_fail_flag(ret);   
    return chr_cnf_msg_report((chr_comm_head_s *)data, ret);
}

int bsp_chr_genl_rcv_handler(struct sk_buff *skb, struct genl_info *info)
{
    unsigned char *data = NULL;
    unsigned int data_len;
    int ret;

    ret = bsp_homi_genl_rcv_para_check(skb, info);
    if (ret) {
        return ret;
    }

    data = genlmsg_data(nlmsg_data(nlmsg_hdr(skb)));
    data_len = skb->len - HOMI_GENL_HDR_LEN;

    bsp_homi_genl_set_port_info(GENL_CHAN_CHR, info);

    return chr_app_data_handle(data, data_len);
}

int chr_modem_reset(drv_reset_cb_moment_e param, int userdata)
{
    if (param == MDRV_RESET_CB_AFTER) {
        chr_crit("chr cfg send to modem\n");
        chr_cfg_send_to_modem();
        chr_reset_info_report();
    }
    return BSP_OK;
}


int chr_msglite_send(void *data, unsigned int len)
{
    struct msg_addr dst;

    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_HMI;

    return bsp_msg_lite_sendto(g_chr_core_info.msg_chan_hdl, &dst, data, len);
}

int chr_tsp_msg_proc(void *msg, unsigned int len)
{
    return bsp_homi_genl_send_data(GENL_CHAN_CHR, (u8 *)msg, len);
}

int chr_msglite_proc(const struct msg_addr *src, void *msg, unsigned int len)
{
    if ((src == NULL) || (msg == NULL) || (len == 0)) {
        chr_error("msglite para invalid\n");
        return -1;
    }

    if (src->core == MSG_CORE_TSP && src->chnid == MSG_CHN_HMI) {
        return chr_tsp_msg_proc(msg, len);
    }

    chr_error("src core or chan id invalid, core=0x%x, chan=0x%x\n", src->core, src->chnid);
    return -1;
}

int chr_msglite_init(void)
{
    struct msgchn_attr attr = { 0 };

    bsp_msgchn_attr_init(&attr);

    attr.chnid = MSG_CHN_HMI;
    attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    attr.lite_notify = chr_msglite_proc;

    return bsp_msg_lite_open(&g_chr_core_info.msg_chan_hdl, &attr);
}

int chr_init(void)
{
    int ret;
    char *reset_name = "CHR_FS";

    chr_crit("init start\n");

    chr_cfg_init();

    ret = chr_msglite_init();
    if (ret) {
        chr_error("msglite init fail, ret=0x%x\n", ret);
        return BSP_ERROR;
    }

    ret = mdrv_sysboot_register_reset_notify(reset_name, (pdrv_reset_cbfun)chr_modem_reset, 0, CHR_MDM_RESET_PRIO);
    if (ret != EOK) {
        chr_error("chr register modem reset failed.\n");
        return BSP_ERROR;
    }

    g_chr_core_info.init_state = 1;

    chr_crit("init ok\n");
    return BSP_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(chr_init);
#endif

EXPORT_SYMBOL(g_chr_core_info);
EXPORT_SYMBOL(chr_app_data_handle);
EXPORT_SYMBOL(chr_modem_reset);


