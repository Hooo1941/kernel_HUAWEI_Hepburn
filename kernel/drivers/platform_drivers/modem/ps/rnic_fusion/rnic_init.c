/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include "product_config_ps_ap.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_register.h"
#include "rnic_wwan_i.h"

#define RNIC_DMA_MASK	DMA_BIT_MASK(64)

STATIC int rnic_probe(struct platform_device *pdev)
{
	int ret;

	dma_set_mask_and_coherent(&pdev->dev, RNIC_DMA_MASK);

	ret = rnic_wwan_init(pdev, rnic_get_client_list());
	if (ret)
		RNIC_LOGE("wwan init failed");

	return ret;
}

STATIC int rnic_remove(struct platform_device *pdev)
{
	rnic_wwan_exit(pdev, rnic_get_client_list());
	return 0;
}

STATIC const struct of_device_id rnic_match[] = {
	{.compatible = "modem,rnic", },
	{ },
};

STATIC struct platform_driver rnic_driver = {
	.probe  = rnic_probe,
	.remove = rnic_remove,
	.driver = {
		.name = "rnic",
		.of_match_table = of_match_ptr(rnic_match),
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
};

int rnic_init(void)
{
	int ret;
	ret = rnic_register();
	if (ret) {
		RNIC_LOGE("register client failed");
		return ret;
	}

	ret = platform_driver_register(&rnic_driver);
	if (ret) {
		RNIC_LOGE("register driver failed");
		rnic_deregister();
		return ret;
	}

	return 0;
}

void rnic_exit(void)
{
	rnic_deregister();
	platform_driver_unregister(&rnic_driver);
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(rnic_init);
module_exit(rnic_exit);
#endif

void rnic_help(void)
{
	pr_err("[nas_rnic] <rnic_vnet_add_vlan_stub(vnet_id,vlan_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_del_vlan_stub(vnet_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_set_drop_time_stub(vnet_id,code)>\n");
	pr_err("[nas_rnic] <rnic_vnet_show_vlan_info(vnet_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_show_drop_time(vnet_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_link_up_stub(vnet_id,addr_type,sess_id,modem_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_link_down_stub(vnet_id,addr_type,sess_id,modem_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_show_pkt_stats(vnet_id)>\n");
	pr_err("[nas_rnic] <rnic_vnet_show_state_info(vnet_id)>\n");
	pr_err("[nas_rnic] <rnic_wwan_show_pkt_stats()>\n");
	pr_err("[nas_rnic] <rnic_frag_show_stats()>\n");
#ifdef CONFIG_RNIC_SRS
	pr_err("[nas_rnic] <rnic_srs_show_comm_info()>\n");
	pr_err("[nas_rnic] <rnic_srs_show_napi_cfg(pif_id)>\n");
	pr_err("[nas_rnic] <rnic_srs_show_napi_info(pif_id)>\n");
	pr_err("[nas_rnic] <rnic_srs_show_lb_info()>\n");
	pr_err("[nas_rnic] <rnic_srs_show_cc_info()>\n");
	pr_err("[nas_rnic] <rnic_srs_show_boost_info()>\n");
#endif
#ifdef CONFIG_RNIC_GRO
	pr_err("[nas_rnic] <rnic_gro_dbg_switch(mode)>\n");
	pr_err("[nas_rnic] <rnic_gro_dbg_set_fix_weight(weight)>\n");
	pr_err("[nas_rnic] <rnic_gro_dbg_set_pps_weight(index,pps,weight)>\n");
	pr_err("[nas_rnic] <rnic_gro_dbg_set_blk_per_flow(num)>\n");
	pr_err("[nas_rnic] <rnic_gro_show_weight_config()>\n");
	pr_err("[nas_rnic] <rnic_gro_show_segs_stats()>\n");
	pr_err("[nas_rnic] <rnic_gro_show_proto_stats(gro_type)>\n");
#endif
}
