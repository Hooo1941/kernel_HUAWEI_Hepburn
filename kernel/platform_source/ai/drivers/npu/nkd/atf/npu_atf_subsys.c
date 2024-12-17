/*
 * npu_atf_subsys.c
 *
 * about npu atf subsys
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "npu_atf_subsys.h"

noinline int atfd_service_npu_smc(u64 _service_id, u64 _cmd, u64 _arg1,
	u64 _arg2)
{
#if defined (CONFIG_ARM_FFA_TRANSPORT) && defined (CONFIG_NPU_FFA_SUPPORT)
	int ret;
	struct ffa_send_direct_data args = {
		.data0 = (unsigned long)_service_id,
		.data1 = (unsigned long)_cmd,
		.data2 = (unsigned long)_arg1,
		.data3 = (unsigned long)_arg2,
		.data4 = 0
	};

	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0 || args.data4 != 0) {
		npu_drv_err("ffa platdrv send msg error, service_id = 0x%llx, ret = %d, data4 = 0x%llx\n",
			_service_id, ret, args.data4);
		return (int)args.data4;
	}
	return ret;
#else
	struct arm_smccc_res res = {0};
	arm_smccc_smc(_service_id, _cmd, _arg1, _arg2, 0, 0, 0, 0, &res);
	return (int)res.a0;
#endif
}
EXPORT_SYMBOL(atfd_service_npu_smc);