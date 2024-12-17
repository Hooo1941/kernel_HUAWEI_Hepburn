/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#include <linux/netdevice.h>
#include <linux/uaccess.h>
#include "product_config_ps_ap.h"
#include "rnic_vnet_cfg.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_vnet_ioctl.h"

struct rnic_vnet_ioctl_hdlr_s {
	int cmd;
	int (*func)(struct rnic_vnet_priv_s *priv, struct rnic_ioctl_priv_args_s *args);
};

void (*rnic_gro_set_fix_weight_entry)(u32) __rcu __read_mostly = NULL;
void (*rnic_gro_disable_gro_entry)(u32) __rcu __read_mostly = NULL;


STATIC int rnic_vnet_ioctl_set_drop_time(struct rnic_vnet_priv_s *priv,
					 struct rnic_ioctl_priv_args_s *args)
{
	if (priv->drop_code != args->drop_time.code) {
		priv->drop_code = args->drop_time.code;
	}

	return 0;
}

STATIC int rnic_vnet_ioctl_get_drop_time(struct rnic_vnet_priv_s *priv,
					 struct rnic_ioctl_priv_args_s *args)
{
	args->drop_time.code = (u8)priv->drop_code;
	return 0;
}

STATIC int rnic_vnet_ioctl_set_gro_weight(struct rnic_vnet_priv_s *priv,
					  struct rnic_ioctl_priv_args_s *args)
{
	void (*gro_fix_weight_entry)(u32) = NULL;

	rcu_read_lock();
	gro_fix_weight_entry = rcu_dereference(rnic_gro_set_fix_weight_entry);
	if (gro_fix_weight_entry != NULL)
		gro_fix_weight_entry(args->gro_weight);
	rcu_read_unlock();

	return 0;
}

STATIC int rnic_vnet_ioctl_disable_gro(struct rnic_vnet_priv_s *priv,
				       struct rnic_ioctl_priv_args_s *args)
{
	struct rnic_vnet_ctx_s *vctx = priv->back;
	void (*gro_disable_gro_entry)(u32) = NULL;

	if (vctx->srs_ops != NULL && vctx->srs_ops->disable_gro != NULL)
		vctx->srs_ops->disable_gro(args->gro_disable);

	rcu_read_lock();
	gro_disable_gro_entry = rcu_dereference(rnic_gro_disable_gro_entry);
	if (gro_disable_gro_entry != NULL)
		gro_disable_gro_entry(args->gro_disable);
	rcu_read_unlock();

	return 0;
}

STATIC int rnic_vnet_ioctl_boost_cpu(struct rnic_vnet_priv_s *priv,
				     struct rnic_ioctl_priv_args_s *args)
{
	struct rnic_vnet_ctx_s *vctx = priv->back;

	if (vctx->srs_ops != NULL && vctx->srs_ops->boost_cpu != NULL)
		vctx->srs_ops->boost_cpu(args->boost_en);

	return 0;
}


STATIC const struct rnic_vnet_ioctl_hdlr_s rnic_vnet_ioctl_hdlrs[] = {
	{RNIC_IOCTL_PRIV_SET_DROP_TIME, rnic_vnet_ioctl_set_drop_time},
	{RNIC_IOCTL_PRIV_GET_DROP_TIME, rnic_vnet_ioctl_get_drop_time},
	{RNIC_IOCTL_PRIV_SET_GRO_WT, rnic_vnet_ioctl_set_gro_weight},
	{RNIC_IOCTL_PRIV_DIS_GRO, rnic_vnet_ioctl_disable_gro},
	{RNIC_IOCTL_PRIV_BOOST_CPU, rnic_vnet_ioctl_boost_cpu},
};

STATIC const struct rnic_vnet_ioctl_hdlr_s *rnic_vnet_ioctl_get_handler(int cmd)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(rnic_vnet_ioctl_hdlrs); i++) {
		if (cmd == rnic_vnet_ioctl_hdlrs[i].cmd)
			return &rnic_vnet_ioctl_hdlrs[i];
	}

	return NULL;
}

int rnic_vnet_ioctl_priv_handler(struct rnic_vnet_priv_s *priv, void __user *data)
{
	const struct rnic_vnet_ioctl_hdlr_s *handler = NULL;
	struct rnic_ioctl_priv_args_s args = {0};
	int ret;

	if (copy_from_user(&args, data, (unsigned long)sizeof(args))) {
		RNIC_LOGE("copy from priv ioctl data failed");
		return -EFAULT;
	}

	handler = rnic_vnet_ioctl_get_handler(args.cmd);
	if (handler == NULL) {
		RNIC_LOGE("unsupported cmd %d", args.cmd);
		return -EOPNOTSUPP;
	}

	ret = handler->func(priv, &args);
	if (!ret && copy_to_user(data, &args, (unsigned long)sizeof(args))) {
		RNIC_LOGE("copy to priv ioctl data failed");
		ret = -EFAULT;
	}

	return ret;
}
