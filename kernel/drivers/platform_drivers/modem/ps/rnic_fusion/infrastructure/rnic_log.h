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

#ifndef RNIC_UTIL_H
#define RNIC_UTIL_H

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_LOG_LVL_HIGH	(1U << 0)
#define RNIC_LOG_LVL_ERR	(1U << 1)
#define RNIC_LOG_LVL_INFO	(1U << 2)
#define RNIC_LOG_LVL_DBG	(1U << 3)

#define RNIC_LOGH(fmt, ...) do { \
	if (rnic_log_level & RNIC_LOG_LVL_HIGH) \
		pr_err("[NAS_RNIC]: [HIGH] <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

#define RNIC_LOGE(fmt, ...) do { \
	if (rnic_log_level & RNIC_LOG_LVL_ERR) \
		pr_err("[NAS_RNIC]: [ERR] <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

#define RNIC_LOGI(fmt, ...) do { \
	if (rnic_log_level & RNIC_LOG_LVL_INFO) \
		pr_warn("[NAS_RNIC]: [INFO] <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

#define RNIC_LOGD(fmt, ...) do { \
	if (rnic_log_level & RNIC_LOG_LVL_DBG) \
		pr_notice("[NAS_RNIC]: [DBG] <%s> "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

extern unsigned int rnic_log_level;

/* RINC_FUSION[0xC928,0xC93F] */
enum {
	ID_DIAG_RNIC_WWAN_PKT_STATS       = 0xC928,
	ID_DIAG_RNIC_FRAG_PKT_STATS       = 0xC929,
	ID_DIAG_RNIC_VNET_DPL_PTS         = 0xC92A,
	ID_DIAG_RNIC_VNET_GRO_STATS       = 0xc92B,
	ID_DIAG_RNIC_VNET_SRS_BOOST_STATS = 0xc92C,

	ID_DIAG_RNIC_VNET_PKT_STATS = 0xC930,
	ID_DIAG_RNIC_LTEV_PKT_STATS = 0xC931,
	ID_DIAG_RNIC_VNET_SRS_STATS = 0xC932,
	ID_DIAG_RNIC_VNET_DPL_TX    = 0xC938,
	ID_DIAG_RNIC_VNET_DPL_RX    = 0xC939,
	ID_DIAG_RNIC_LTEV_DPL_TX    = 0xC93A,
	ID_DIAG_RNIC_LTEV_DPL_RX    = 0xC93B,
};

struct rnic_diag_trace_ind_s {
	u16 flag;
	u16 timestamp;
	u32 data_len;
	u32 src_id;
	u32 dst_id;
	u8  data[0];
};

void rnic_trace_report(const void *data, u32 len);
void rnic_trans_report(u32 msgid, void *data, u32 len);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_UTIL_H */
