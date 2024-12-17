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

#ifndef RNIC_PKTERR_H
#define RNIC_PKTERR_H

#include <linux/skbuff.h>
#include "rnic_data.h"
#include "mdrv_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_PKTERR_CYCLE 5
#define RNIC_PKTERR_NUM_THRESHHOLD 700
#define RNIC_PKTERR_PERIOD 2000
#define RNIC_PKTERR_PIFMASK (RNIC_VNET_DATA_PIFMASK | RNIC_VNET_IMS_PIFMASK)

struct rnic_pkterr_fea_cfg_s {
	unsigned int pkterr_period;
	unsigned int pkterr_cycle;
	unsigned int pkterr_num_threshhold;
};

struct rnic_pkterr_info_s {
	unsigned short period_idx;
	unsigned short cur_pkterr_num;
	unsigned int total_pkterr_num;
	unsigned int period_start_time;
	unsigned int num_start_time;
};

struct rnic_pkterr_ctx_s {
	struct rnic_pkterr_fea_cfg_s fea_cfg;
	unsigned int timestamp_freq;
	struct rnic_pkterr_info_s pkterr_info[RNIC_MODEM_ID_MAX][RNIC_PDUSESSION_ID_MAX];
};

extern int rnic_wwan_msg_snd(struct msg_addr *chan_dst, void *msg, unsigned len);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_PKTERR_H */