/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Api for crl update
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

#ifndef TZ_UPDATE_CRL_H
#define TZ_UPDATE_CRL_H
#include "teek_ns_client.h"

#define DEVICE_CRL_MAX 0x4000 /* 16KB */
int send_crl_to_tee(const char *crl_buffer, uint32_t crl_len, const struct tc_ns_dev_file *dev_file);
int tc_ns_update_ta_crl(const struct tc_ns_dev_file *dev_file, void __user *argp);
int tz_update_crl(const char *file_path, const struct tc_ns_dev_file *dev_file);
#endif
