/*
 * smc_abi.h
 *
 * function declarations for legacy tzdriver smc transport
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

#ifndef SMC_ABI_H
#define SMC_ABI_H

#include "smc_call.h"

void do_smc_transport(struct smc_in_params *in, struct smc_out_params *out, uint8_t wait);
#endif