/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: efuse source file to provide custom service with ffa
 * Create: 2023/05/08
 */

#include <platform_include/see/efuse_driver.h>
#include <securec.h>           /* memset_s */
#include <linux/mutex.h>       /* mutex_lock */
#include "../efuse_plat.h"     /* efuse_err */
#include "../efuse_call.h"     /* efuse_call_args */

static DEFINE_MUTEX(g_efuse_rw_lock);

u32 efuse_custom_read(u8 *buf, u32 *size, u32 fid)
{
	u32 ret;
	struct efuse_call_args args = {0};
	u8 tmp_buf[EFUSEC_GROUP_MAX_COUNT * sizeof(u32)] = { 0 };

	args.arg0 = fid;

	if (!buf || !size) {
		efuse_err("error, param is null\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	mutex_lock(&g_efuse_rw_lock);
	ret = efuse_ffa_rd_shmem(&args, (u8 *)&tmp_buf[0]);
	mutex_unlock(&g_efuse_rw_lock);

	if (args.arg4 > *size) {
		efuse_err("error, buf overflow\n");
		return EFUSE_OVERFLOW_ERR;
	}

	if (memcpy_s((void *)buf, *size, (void *)&tmp_buf[0], args.arg4) != EOK) {
		efuse_err("error, memcpy_s failed\n");
		return EFUSE_OVERFLOW_ERR;
	}

	*size = args.arg4;

	return ret;
}

u32 efuse_custom_write(u8 *buf, u32 size, u32 fid)
{
	u32 ret;
	struct efuse_call_args args = {0};
	u8 tmp_buf[EFUSEC_GROUP_MAX_COUNT * sizeof(u32)] = { 0 };

	if (!buf) {
		efuse_err("error, buf is null\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	if (memcpy_s((void *)&tmp_buf[0], sizeof(tmp_buf), (void *)buf, size) != EOK) {
		efuse_err("error, memcpy_s failed\n");
		return EFUSE_OVERFLOW_ERR;
	}

	args.arg0 = fid;
	args.arg4 = size;

	mutex_lock(&g_efuse_rw_lock);
	ret = efuse_ffa_wr_shmem(&args, (u8 *)&tmp_buf[0]);
	mutex_unlock(&g_efuse_rw_lock);

	return ret;
}
