/* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "i2c_common.h"

int i2c_addr_access(struct i2c_adapter *adapter, unsigned char addr,
	ti_i2c_cmd_entry_t *accesses, int accesses_count)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < accesses_count; i++) {
		if (accesses[i].op_type == REG_OP_R) {
			cdc_err("read ops not support now");
		} else {
			ret = i2c_write_reg(adapter, addr, accesses[i].reg, accesses[i].val);
			if (ret)
				cdc_err("fail write slv_addr:0x%x reg: 0x%x val: 0x%x delaytime: %d i: %d",
					addr, accesses[i].reg, accesses[i].val, accesses[i].wait_ms, i);
		}

		if (accesses[i].wait_ms != 0)
			mdelay(accesses[i].wait_ms);
	}

	return ret;
}
