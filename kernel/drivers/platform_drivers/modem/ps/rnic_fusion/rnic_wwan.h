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

#ifndef RNIC_WWAN_H
#define RNIC_WWAN_H

#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/version.h>
#include "mdrv_msg.h"
#include "rnic_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_WWAN_RPT_TIMEOUT	(jiffies + msecs_to_jiffies(1000))
#define RNIC_WWAN_RESET_CB_PIOR	4
#define RNIC_WWAN_WAKELOCK_TIME	500 /* ms */

struct rnic_wwan_stats_s {
	u64 tx_bytes;
	u64 tx_pkts;
	u64 tx_mif_loop_pkts;

	u64 rx_bytes;
	u64 rx_pkts;
	u64 rx_drop;
};

struct rnic_wwan_pkt_stats_rpt_s {
	struct {
		u8 ver;
		u8 reserved[7];
	} hdr;
	struct rnic_wwan_stats_s stats;
};

struct rnic_wwan_ctx_s {
	struct device *dev;
	struct list_head *client_list;
	struct rnic_wwan_stats_s stats;

	unsigned long pif_masks[BITS_TO_LONGS(RNIC_MAX_PIF_NUM)];
	struct rnic_handle_s *pif_hdls[RNIC_MAX_PIF_NUM];

	struct rnic_handle_s client_hdls[RNIC_CLIENT_MAX];
	struct wakeup_source *wake_lock;
	struct timer_list rpt_timer;
	struct msg_chn_hdl *chn_hdl;
	pfa_version_type pfa_ver;
};

#ifdef __cplusplus
}
#endif

#endif /* RNIC_WWAN_H */
