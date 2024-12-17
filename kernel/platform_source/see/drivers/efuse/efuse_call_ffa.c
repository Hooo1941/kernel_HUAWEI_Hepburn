/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: efuse source file to provide ffa
 * Create: 2023/03/30
 */

#include <platform_include/see/efuse_driver.h>
#include <platform_include/see/ffa/ffa_plat_drv.h>
#include "efuse_call.h"
#include "efuse_plat.h"

uint32_t efuse_ffa(struct efuse_call_args *arg)
{
	uint32_t ret;
	struct ffa_send_direct_data ffa_data;

	if (!arg) {
		efuse_err("error, arg is null\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	ffa_data.data0 = arg->arg0;
	ffa_data.data1 = arg->arg1;
	ffa_data.data2 = arg->arg2;
	ffa_data.data3 = arg->arg3;
	ffa_data.data4 = arg->arg4;

	ret = (uint32_t)ffa_platdrv_send_msg(&ffa_data);
	if (ret != 0) {
		efuse_err("efuse ffa proc failed, ret=0x%x\n", ret);
		return EFUSE_FFA_PROC_ERR;
	}

	ret = (uint32_t)ffa_data.data0;
	if (ret != 0) {
		efuse_err("efuse ffa svc failed, ret=0x%x\n", ret);
		return EFUSE_FFA_PROC_ERR;
	}

	arg->arg0 = ffa_data.data0;
	arg->arg1 = ffa_data.data1;
	arg->arg2 = ffa_data.data2;
	arg->arg3 = ffa_data.data3;
	arg->arg4 = ffa_data.data4;

	return ret;
}

static uint32_t efuse_ffa_with_shmem(struct efuse_call_args *arg, uint8_t *buf, uint64_t flag)
{
	uint32_t ret;
	struct ffa_send_direct_data ffa_data;

	if (!arg || !buf) {
		efuse_err("error, param is null\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	ffa_data.data0 = arg->arg0;
	ffa_data.data1 = arg->arg1;
	ffa_data.data2 = arg->arg2;
	ffa_data.data3 = arg->arg3;
	ffa_data.data4 = arg->arg4;

	ret = (uint32_t)ffa_platdrv_send_msg_with_shmem(&ffa_data, buf, flag);
	if (ret != 0) {
		efuse_err("efuse ffa proc failed, proc_ret=0x%x, svc_ret=0x%x\n", ret, (uint32_t)ffa_data.data0);
		return EFUSE_FFA_PROC_ERR;
	}

	ret = (uint32_t)ffa_data.data0;
	if (ret != 0) {
		efuse_err("efuse ffa svc failed, ret=0x%x\n", ret);
		return EFUSE_FFA_PROC_ERR;
	}

	arg->arg0 = ffa_data.data0;
	arg->arg1 = ffa_data.data1;
	arg->arg2 = ffa_data.data2;
	arg->arg3 = ffa_data.data3;
	arg->arg4 = ffa_data.data4;

	return ret;
}

uint32_t efuse_ffa_rd_shmem(struct efuse_call_args *arg, uint8_t *buf)
{
	return efuse_ffa_with_shmem(arg, buf, SHMEM_OUT);
}

uint32_t efuse_ffa_wr_shmem(struct efuse_call_args *arg, uint8_t *buf)
{
	return efuse_ffa_with_shmem(arg, buf, SHMEM_IN);
}
