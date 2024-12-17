/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#include "mdrv_pfa.h"
#include "mdrv_timer.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_data.h"

#define RNIC_PTS_IFACE_ID_MASK	0x0000000F
#define RNIC_PTS_PKT_NO_MASK	0xFFFFFFF0

/* pts: packet time stamp */
unsigned int rnic_pts_rpt_en = 0;

STATIC void rnic_vnet_dpl_pts_rpt(unsigned int pkt_id, enum fw_pos pos)
{
	struct packet_time_stamp time_info;
	time_info.pkt_id.iface_id = pkt_id & RNIC_PTS_IFACE_ID_MASK;
	time_info.pkt_id.id = (pkt_id & RNIC_PTS_PKT_NO_MASK) >> RNIC_VNET_PTS_PKT_NO_OFFSET;
	time_info.pos = pos;
	time_info.time_stamp = mdrv_timer_get_normal_timestamp();

	rnic_trans_report(ID_DIAG_RNIC_VNET_DPL_PTS, &time_info, sizeof(time_info));
}

void rnic_vnet_dpl_pts_tx_rpt(const struct sk_buff *skb)
{
	if (rnic_pts_rpt_en != 0) {
		unsigned int pkt_id = rnic_wan_skb_cb(skb)->userfield2;
		rnic_vnet_dpl_pts_rpt(pkt_id, RNIC_UP_RCV);
	}
}

void rnic_vnet_dpl_pts_rx_rpt(const struct sk_buff *skb)
{
	if (rnic_pts_rpt_en != 0) {
		unsigned int pkt_id = rnic_map_pktid(skb);
		rnic_vnet_dpl_pts_rpt(pkt_id, RNIC_DOWN_RCV);
	}
}

void rnic_vnet_dpl_set_pts_rpt(unsigned int enable)
{
	rnic_pts_rpt_en = enable;
	RNIC_LOGE("packet time stamp report enable is %d", rnic_pts_rpt_en);
}
