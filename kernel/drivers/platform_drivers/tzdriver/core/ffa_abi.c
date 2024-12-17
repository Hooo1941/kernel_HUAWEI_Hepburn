/*
 * ffa_abi.c
 *
 * functions for ffa settings
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/arm_ffa.h>
#include "ffa_abi.h"
#include "teek_ns_client.h"
#include "tz_pm.h"
#include "smc_call.h"

const struct ffa_ops *g_ffa_ops = NULL;
struct ffa_device *g_ffa_dev = NULL;

static void ffa_remove(struct ffa_device *ffa_dev)
{
	tlogd("stub remove ffa driver!\n");
}

static int ffa_probe(struct ffa_device *ffa_dev)
{
	g_ffa_ops = ffa_dev->ops;
	g_ffa_dev = ffa_dev;
	if (!g_ffa_ops) {
		tloge("failed to get ffa_ops!\n");
		return -ENOENT;
	}

	g_ffa_ops->mode_32bit_set(ffa_dev);
	return 0;
}

/* two sp uuid can be the same */
const struct ffa_device_id tz_ffa_device_id[] = {
    /* uuid = <0xe0786148 0xe311f8e7 0x02005ebc 0x1bc5d5a5> */
	{0x48, 0x61, 0x78, 0xe0, 0xe7, 0xf8, 0x11, 0xe3,
	 0xbc, 0x5e, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b},
	{}
};

static struct ffa_driver tz_ffa_driver = {
	.name = "iTrustee",
	.probe = ffa_probe,
	.remove = ffa_remove,
	.id_table = tz_ffa_device_id,
};

int ffa_abi_register(void)
{
	return ffa_register(&tz_ffa_driver);
}

void ffa_abi_unregister(void)
{
	ffa_unregister(&tz_ffa_driver);
}

void smc_req(struct smc_in_params *in, struct smc_out_params *out, uint8_t wait)
{
	ffa_forward_call(in, out, wait);
}

static void convert_smc_param_to_ffa_param(struct smc_in_params *in_param, struct ffa_send_direct_data *ffa_param)
{
	ffa_param->data0 = in_param->x1;
	ffa_param->data1 = in_param->x2;
	ffa_param->data2 = in_param->x3;
	ffa_param->data3 = in_param->x4;
	/* x0(smc id) need to be transported for tee dealing it directly */
	ffa_param->data4 = in_param->x0;
}

static void convert_ffa_param_to_smc_param(struct ffa_send_direct_data *ffa_param, struct smc_out_params *out_param)
{
	out_param->ret = ffa_param->data4;
	out_param->exit_reason = ffa_param->data0;
	out_param->ta = ffa_param->data1;
	out_param->target = ffa_param->data2;
}

int ffa_forward_call(struct smc_in_params *in_param, struct smc_out_params *out_param, uint8_t wait)
{
	if (in_param == NULL || out_param == NULL) {
		tloge("invalid parameter ffa forward!\n");
		return -1;
	}

	int ret;
	struct ffa_send_direct_data ffa_param = {};
	convert_smc_param_to_ffa_param(in_param, &ffa_param);

	do {
		ret = g_ffa_ops->sync_send_receive(g_ffa_dev, &ffa_param);
		convert_ffa_param_to_smc_param(&ffa_param, out_param);
	} while(out_param->ret == TSP_REQUEST && wait != 0);

	if (ret != 0)
		tloge("failed to call! ret is %d\n", ret);

	return ret;
}
