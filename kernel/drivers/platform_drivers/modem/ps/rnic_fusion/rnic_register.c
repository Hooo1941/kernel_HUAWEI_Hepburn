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

#include <linux/module.h>
#include <linux/list.h>
#include "product_config_ps_ap.h"
#include "rnic_private.h"
#include "rnic_client.h"
#include "rnic_vnet_enet_i.h"
#ifdef CONFIG_RNIC_LTEV
#include "rnic_ltev_i.h"
#endif

STATIC LIST_HEAD(rnic_client_list);

STATIC int rnic_register_client(struct rnic_client_s *client)
{
	struct rnic_client_s *client_tmp = NULL;
	struct list_head *tmp = NULL;

	list_for_each(tmp, &rnic_client_list) {
		client_tmp = list_entry(tmp, struct rnic_client_s, list);
		if (client_tmp->type == client->type)
			goto out;
	}

	list_add_tail(&client->list, &rnic_client_list);
out:
	return 0;
}

STATIC void rnic_deregister_client(struct rnic_client_s *client)
{
	struct rnic_client_s *client_tmp = NULL;
	struct list_head *tmp = NULL;
	bool found = false;

	list_for_each(tmp, &rnic_client_list) {
		client_tmp = list_entry(tmp, struct rnic_client_s, list);
		if (client_tmp->type == client->type) {
			found = true;
			break;
		}
	}

	if (found)
		list_del(&client->list);
}

struct list_head *rnic_get_client_list(void)
{
	return &rnic_client_list;
}

int rnic_register(void)
{
	int ret;

	ret = rnic_register_client(rnic_vnet_get_client());
	if (ret)
		goto err_reg_vnet;

#ifdef CONFIG_RNIC_LTEV
	ret = rnic_register_client(ltev_get_client());
	if (ret)
		goto err_reg_ltev;
#endif
	return 0;

#ifdef CONFIG_RNIC_LTEV
err_reg_ltev:
#endif

	rnic_deregister_client(rnic_vnet_get_client());
err_reg_vnet:

	return ret;
}

void rnic_deregister(void)
{
#ifdef CONFIG_RNIC_LTEV
	rnic_deregister_client(ltev_get_client());
#endif

	rnic_deregister_client(rnic_vnet_get_client());
}