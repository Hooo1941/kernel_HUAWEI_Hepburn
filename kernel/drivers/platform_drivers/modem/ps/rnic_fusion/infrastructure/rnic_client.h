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

#ifndef RNIC_CLIENT_H
#define RNIC_CLIENT_H

#include "product_config_ps_ap.h"
#include "mdrv_pfa.h"
#include "rnic_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rnic_handle_s;

enum rnic_client_type_e {
	RNIC_CLIENT_VNET,
#ifdef CONFIG_RNIC_LTEV
	RNIC_CLIENT_LTEV,
#endif
#ifdef LLT_OS_VER
	RNIC_CLIENT_TEST,
#endif
	RNIC_CLIENT_MAX,
};

struct rnic_link_info_s {
	u32 pif_id;
	bool link_up;
	enum rnic_addr_type_e type;
	struct {
		bool copy_en;
		struct pfa_tft_info_s tft_info;
	};
};

struct rnic_client_ops_s {
	int (*init_inst)(struct rnic_handle_s *);
	void (*deinit_inst)(struct rnic_handle_s *);
	int (*rx_data)(struct rnic_handle_s *, struct sk_buff *);
	void (*rx_cmplt)(struct rnic_handle_s *);
	int (*link_change)(struct rnic_handle_s *, const struct rnic_link_info_s *);
	void (*feature_cfg)(struct rnic_handle_s *, const void *);
	void (*reset_notify)(struct rnic_handle_s *);
	void (*rpt_stats)(struct rnic_handle_s *);
};

struct rnic_wwan_ops_s {
	int (*tx_data)(struct rnic_handle_s *, struct sk_buff *);
	void (*rx_cb_register)(u32 pif_id, int unmap_en);
};

struct rnic_client_s {
	struct list_head list;
	unsigned long long pif_bitmask;
	unsigned long long frag_pifmask;
	enum rnic_client_type_e type;
	const struct rnic_client_ops_s *ops;
};

struct rnic_handle_s {
	struct device *dev;
	struct rnic_client_s *client;
	void *priv;
	const struct rnic_wwan_ops_s *ops;
	pfa_version_type pfa_ver;
};

#ifdef __cplusplus
}
#endif

#endif /* RNIC_CLIENT_H */
