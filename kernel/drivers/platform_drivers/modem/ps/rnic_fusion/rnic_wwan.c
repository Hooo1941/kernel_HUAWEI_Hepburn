/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 */

#include <linux/skbuff.h>
#include "msg_id.h"
#include "mdrv_diag.h"
#include "mdrv_pfa.h"
#include "mdrv_sysboot.h"
#include "rnic_data.h"
#include "rnic_client.h"
#include "rnic_dsm_msg_pif.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "rnic_multi_if.h"
#include "rnic_wakeup.h"
#include "rnic_pkterr_i.h"
#include "rnic_wwan.h"

STATIC struct rnic_wwan_ctx_s *rnic_wwan_ctx_ptr;

STATIC int rnic_wwan_tx_data(struct rnic_handle_s *handle, struct sk_buff *skb)
{
	struct rnic_wwan_ctx_s *wctx = container_of(handle, struct rnic_wwan_ctx_s,
						    client_hdls[handle->client->type]);
	u32 flag = rnic_mem_flag(skb);

	wctx->stats.tx_bytes += skb->len;
	wctx->stats.tx_pkts++;

	mdrv_wan_tx(skb, flag);
	return 0;
}

unsigned int rnic_wwan_rx_get_pifid(struct sk_buff* skb)
{
	return rnic_map_pifid(skb);
}

STATIC void rnic_wwan_rx_data(struct sk_buff *skb)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;
	struct rnic_handle_s *handle = NULL;
	u8 dra_mem = !(rnic_map_skb_cb(skb)->packet_info.bits.unmapped);

	rnic_set_mem_flag(skb, dra_mem);

	rnic_pkterr_statistic_err(skb, wctx->pfa_ver);

	handle = wctx->pif_hdls[rnic_map_pifid(skb)];
	if (unlikely(handle == NULL)) {
		wctx->stats.rx_drop++;
		rnic_kfree_skb(skb);
		return;
	}

	wctx->stats.rx_bytes += skb->len;
	wctx->stats.rx_pkts++;

	handle->client->ops->rx_data(handle, skb);
	return;
}

STATIC void rnic_wwan_rx_cmplt(void)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;
	struct rnic_client_s *client = NULL;
	u32 i;

	for (i = 0; i < RNIC_CLIENT_MAX; i++) {
		client = wctx->client_hdls[i].client;
		if (client != NULL)
			client->ops->rx_cmplt(&wctx->client_hdls[i]);
	}

	if (!rnic_wakeup())
		__pm_wakeup_event(wctx->wake_lock, RNIC_WWAN_WAKELOCK_TIME);
}

void rnic_wwan_rx_cb_register(u32 pif_id, int unmap_en)
{
	struct wan_callback_s wan_cb = {
		.unmap_en = unmap_en,
		.rx = rnic_wwan_rx_data,
		.rx_complete = rnic_wwan_rx_cmplt,
	};

	mdrv_wan_callback_register(pif_id, &wan_cb);
}

STATIC void rnic_wwan_link_change(struct rnic_wwan_ctx_s *wctx, bool link_up,
				  const struct rnic_data_link_cfg_s *cfg)
{
	struct rnic_handle_s *handle = NULL;
	struct rnic_client_s *client = NULL;
	struct rnic_link_info_s link_info = {0};
	struct pfa_tft_info_s *tft_info = &link_info.tft_info;

	if (cfg->ps_iface_id >= RNIC_MAX_PIF_NUM) {
		RNIC_LOGE("pif_id %d invalid", cfg->ps_iface_id);
		return;
	}

	handle = wctx->pif_hdls[cfg->ps_iface_id];
	if (handle == NULL) {
		RNIC_LOGE("client of pif_id %d is NULL", cfg->ps_iface_id);
		return;
	}

	client = handle->client;
	if (client->ops->link_change == NULL) {
		RNIC_LOGE("link_change pif_id %d is NULL", cfg->ps_iface_id);
		return;
	}

	if (link_up)
		rnic_pkterr_clear_err_data(cfg->modem_id, cfg->pdu_sess_id);

	link_info.pif_id = cfg->ps_iface_id;
	link_info.link_up = link_up;
	link_info.copy_en = !!cfg->copy_en;

	tft_info->pdu_session_id = cfg->pdu_sess_id;
	tft_info->modem_id = cfg->modem_id;
	tft_info->fc_head  = cfg->fc_head;

	if ((cfg->pdu_sess_type & RNIC_PDU_SESS_TYPE_IPV4) != 0) {
		link_info.type = RNIC_ADDR_IPV4;
		client->ops->link_change(handle, &link_info);
	}

	if ((cfg->pdu_sess_type & RNIC_PDU_SESS_TYPE_IPV6) != 0) {
		link_info.type = RNIC_ADDR_IPV6;
		client->ops->link_change(handle, &link_info);
	}

	if ((cfg->pdu_sess_type & RNIC_PDU_SESS_TYPE_ETHER) != 0) {
		link_info.type = RNIC_ADDR_ETH;
		client->ops->link_change(handle, &link_info);
	}

	if ((cfg->pdu_sess_type & RNIC_PDU_SESS_TYPE_MBS) != 0) {
		link_info.type = RNIC_ADDR_MBS;
		client->ops->link_change(handle, &link_info);
	}
}

STATIC void rnic_wwan_vnet_fea_cfg(struct rnic_wwan_ctx_s *wctx,
				   const struct rnic_vnet_fea_cfg_s *cfg)
{
	struct rnic_handle_s *handle = &wctx->client_hdls[RNIC_CLIENT_VNET];
	struct rnic_client_s *client = handle->client;

	if (client->ops->feature_cfg != NULL)
		client->ops->feature_cfg(handle, cfg);
}

STATIC bool rnic_wwan_msg_valid(const struct msg_addr *src, void *data, unsigned len)
{
	struct rnic_dsm_msg_s *msg = data;
	u32 payload_len;

	if (unlikely(src == NULL)) {
		RNIC_LOGE("src null");
		return false;
	}

	if (unlikely(msg == NULL)) {
		RNIC_LOGE("msg null");
		return false;
	}

	if (unlikely(len < RNIC_DSM_MSG_FIX_LEN)) {
		RNIC_LOGE("msg_len %d", len);
		return false;
	}

	payload_len = len - RNIC_DSM_MSG_HDR_LEN;
	if (unlikely(payload_len < msg->args_length)) {
		RNIC_LOGE("msg_len %d, msg_type %d, args_len %d",
			  len, msg->msg_type, msg->args_length);
		return false;
	}

	return true;
}

int rnic_wwan_msg_snd(struct msg_addr *chan_dst, void *msg, unsigned len)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;

	if (wctx->chn_hdl == NULL) {
		RNIC_LOGE("rnic_wwan_msg_snd: handle is null");
		return -EINVAL;
	}

	rnic_trace_report(msg, len);

	return mdrv_msgchn_lite_sendto(wctx->chn_hdl, chan_dst, msg, len);
}

STATIC int rnic_wwan_msg_rcv(const struct msg_addr *src, void *data, unsigned len)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;
	struct rnic_dsm_msg_s *msg = data;
	struct rnic_vnet_fea_cfg_s *cfg = NULL;

	rnic_trace_report(data, len);

	if (!rnic_wwan_msg_valid(src, data, len))
		return -EINVAL;

	switch (msg->msg_type) {
	case ID_RNIC_DATA_LINK_CFG_IND:
		rnic_wwan_link_change(wctx, true, &msg->data_cfg);
		break;
	case ID_RNIC_DATA_LINK_REL_IND:
		rnic_wwan_link_change(wctx, false, &msg->data_cfg);
		break;
	case ID_RNIC_VNET_FEA_CFG_IND:
		cfg = &msg->vnet_fea_cfg;
		rnic_wwan_vnet_fea_cfg(wctx, cfg);
		rnic_pkterr_fea_cfg(cfg->pkterr_period, cfg->pkterr_cycle,
				    cfg->pkterr_num_threshhold);
		break;
	default:
		break;
	}

	return 0;
}

STATIC void rnic_wwan_notify_client(struct rnic_wwan_ctx_s *wctx)
{
	struct rnic_client_s *client = NULL;
	u32 i;

	for (i = 0; i < RNIC_CLIENT_MAX; i++) {
		client = wctx->client_hdls[i].client;
		if (client == NULL || client->ops->reset_notify == NULL)
			continue;

		client->ops->reset_notify(&wctx->client_hdls[i]);
	}
}

STATIC int rnic_wwan_reset_notify(drv_reset_cb_moment_e param, int data)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;
	int ret = 0;

	RNIC_LOGH("modem reset stage: %d", param);

	switch (param) {
	case MDRV_RESET_CB_BEFORE:
		rnic_wwan_notify_client(wctx);
		break;
	case MDRV_RESET_CB_AFTER:
		break;
	default:
		ret = -ENOTSUPP;
		break;
	}

	return ret;
}

STATIC void rnic_wwan_diag_state_notify(mdrv_diag_state_e state)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;

	if (state == DIAG_STATE_CONN)
		mod_timer(&wctx->rpt_timer, RNIC_WWAN_RPT_TIMEOUT);
	else
		del_timer_sync(&wctx->rpt_timer);
}

STATIC void rnic_wwan_report_client_stats(struct rnic_wwan_ctx_s *wctx)
{
	struct rnic_client_s *client = NULL;
	u32 i;

	for (i = 0; i < RNIC_CLIENT_MAX; i++) {
		client = wctx->client_hdls[i].client;
		if (client == NULL || client->ops->rpt_stats == NULL)
			continue;

		client->ops->rpt_stats(&wctx->client_hdls[i]);
	}
}

STATIC void rnic_wwan_report_priv_stats(struct rnic_wwan_ctx_s *wctx)
{
	struct rnic_wwan_pkt_stats_rpt_s rpt_info = {{0}};

	rpt_info.hdr.ver = 101; /* version 101 */
	rpt_info.stats   = wctx->stats;
	rnic_trans_report(ID_DIAG_RNIC_WWAN_PKT_STATS, &rpt_info, sizeof(rpt_info));
}

void rnic_wwan_report_stats_cycle(struct timer_list *t)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;

	rnic_wwan_report_client_stats(wctx);
	rnic_wwan_report_priv_stats(wctx);
	mod_timer(&wctx->rpt_timer, RNIC_WWAN_RPT_TIMEOUT);
}

STATIC struct msg_chn_hdl *rnic_wwan_msgchn_open(void)
{
	struct msgchn_attr attr = {0};

	mdrv_msgchn_attr_init(&attr);
	attr.chnid = MSG_CHN_RNIC;
	attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
	attr.lite_notify = rnic_wwan_msg_rcv;

	return mdrv_msgchn_lite_open(&attr);
}

STATIC const struct rnic_wwan_ops_s rnic_wwan_ops = {
	.tx_data	= rnic_wwan_tx_data,
	.rx_cb_register	= rnic_wwan_rx_cb_register,
};

STATIC int rnic_wwan_init_client(struct rnic_wwan_ctx_s *wctx)
{
	struct rnic_handle_s *handle = NULL;
	struct rnic_client_s *client = NULL;
	struct list_head *tmp = NULL;
	unsigned long *bitmasks = NULL;
	unsigned long bit;
	int ret;

	if (list_empty(wctx->client_list)) {
		RNIC_LOGE("client not registered");
		return -ENODEV;
	}

	list_for_each(tmp, wctx->client_list) {
		client = list_entry(tmp, struct rnic_client_s, list);

		handle = &wctx->client_hdls[client->type];
		handle->dev     = wctx->dev;
		handle->client  = client;
		handle->ops     = &rnic_wwan_ops;
		handle->pfa_ver = wctx->pfa_ver;

		ret = client->ops->init_inst(handle);
		if (ret) {
			RNIC_LOGE("client init failed, type %d", client->type);
			break;
		}

		bitmasks = (unsigned long *)&client->pif_bitmask;
		for_each_set_bit(bit, bitmasks, (unsigned long)RNIC_MAX_PIF_NUM) {
			wctx->pif_hdls[bit] = handle;
			set_bit((int)bit, wctx->pif_masks);
		}
	}

	return ret;
}

STATIC void rnic_wwan_deinit_client(struct rnic_wwan_ctx_s *wctx)
{
	struct rnic_handle_s *handle = NULL;
	u32 i;

	for (i = 0; i < RNIC_CLIENT_MAX; i++) {
		handle = &wctx->client_hdls[i];
		if (handle->client != NULL)
			handle->client->ops->deinit_inst(handle);
	}
}

STATIC int rnic_wwan_init_ctrl_service(struct rnic_wwan_ctx_s *wctx)
{
	int ret;

	wctx->chn_hdl = rnic_wwan_msgchn_open();
	if (wctx->chn_hdl == NULL) {
		RNIC_LOGE("msgchn open failed");
		return -EFAULT;
	}

	ret = mdrv_sysboot_register_reset_notify("rnic", rnic_wwan_reset_notify,
						 0, RNIC_WWAN_RESET_CB_PIOR);
	if (ret)
		RNIC_LOGE("register reset notify failed, %d", ret);

	mdrv_diag_conn_state_notify_fun_reg(rnic_wwan_diag_state_notify);
	timer_setup(&wctx->rpt_timer, rnic_wwan_report_stats_cycle, 0);
	return 0;
}

STATIC void rnic_wwan_deinit_ctrl_service(struct rnic_wwan_ctx_s *wctx)
{
	del_timer_sync(&wctx->rpt_timer);
	wctx->chn_hdl = NULL;
}

STATIC void rnic_wwan_init_wakelock(struct rnic_wwan_ctx_s *wctx)
{
	wctx->wake_lock = wakeup_source_register(NULL, "rnic_data_wake");
	if (wctx->wake_lock == NULL)
		RNIC_LOGE("wakeup_source_register failed");
}

STATIC void rnic_wwan_deinit_wakelock(struct rnic_wwan_ctx_s *wctx)
{
	wakeup_source_unregister(wctx->wake_lock);
}

int rnic_wwan_init(struct platform_device *pdev, struct list_head *client_list)
{
	struct rnic_wwan_ctx_s *wctx = NULL;
	int ret = 0;
	pfa_version_type pfa_ver = mdrv_pfa_get_version();

	if (pfa_ver == PFA_VER_ERR) {
		RNIC_LOGE("get pfa version error");
		ret = -ENODEV;
		goto err_out;
	}

	wctx = kzalloc(sizeof(*wctx), GFP_KERNEL);
	if (wctx == NULL) {
		RNIC_LOGE("alloc resource failed");
		ret = -ENOMEM;
		goto err_out;
	}

	rnic_wwan_ctx_ptr = wctx;
	wctx->dev = &pdev->dev;
	wctx->client_list = client_list;
	wctx->pfa_ver = pfa_ver;

	ret = rnic_pkterr_init();
	if (ret)
		goto err_pkterr_init;

	ret = rnic_wwan_init_client(wctx);
	if (ret)
		goto err_client_init;

	ret = rnic_wwan_init_ctrl_service(wctx);
	if (ret)
		goto err_ctrl_serv_init;

	rnic_wwan_init_wakelock(wctx);
	mdrv_wan_get_handle_cb_register(rnic_wwan_rx_get_pifid);

	platform_set_drvdata(pdev, wctx);
	return 0;

err_ctrl_serv_init:
	rnic_wwan_deinit_client(wctx);
err_client_init:
	rnic_pkterr_deinit();
err_pkterr_init:
	kfree(wctx);
	rnic_wwan_ctx_ptr = NULL;
err_out:
	return ret;
}

void rnic_wwan_exit(struct platform_device *pdev, struct list_head *client_list)
{
	struct rnic_wwan_ctx_s *wctx = platform_get_drvdata(pdev);

	if (wctx == NULL) {
		RNIC_LOGE("resource not exist");
		return;
	}

	mdrv_wan_get_handle_cb_register(NULL);
	rnic_wwan_deinit_wakelock(wctx);
	rnic_wwan_deinit_ctrl_service(wctx);
	rnic_wwan_deinit_client(wctx);
	rnic_pkterr_deinit();

	kfree(wctx);
	rnic_wwan_ctx_ptr = NULL;
}

void rnic_wwan_show_pkt_stats(void)
{
	struct rnic_wwan_ctx_s *wctx = rnic_wwan_ctx_ptr;

	if (wctx == NULL) {
		pr_err("wwan not exist\n");
		return;
	}

	pr_err("tx_bytes         %llu\n", wctx->stats.tx_bytes);
	pr_err("tx_pkts          %llu\n", wctx->stats.tx_pkts);
	pr_err("tx_mif_loop_pkts %llu\n", wctx->stats.tx_mif_loop_pkts);
	pr_err("rx_bytes         %llu\n", wctx->stats.rx_bytes);
	pr_err("rx_pkts          %llu\n", wctx->stats.rx_pkts);
	pr_err("rx_drop          %llu\n", wctx->stats.rx_drop);
}
