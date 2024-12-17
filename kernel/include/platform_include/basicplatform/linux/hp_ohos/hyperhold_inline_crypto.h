/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Head file for hyperhold inline crypto
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#ifndef _HYPERHOLD_INLINE_CRYPTO_H
#define _HYPERHOLD_INLINE_CRYPTO_H
ssize_t hyperhold_inline_crypto_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t hyperhold_inline_crypto_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len);
#endif
