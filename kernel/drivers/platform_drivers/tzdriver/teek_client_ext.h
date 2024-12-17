/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function declaration interface for kernel.
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
#ifndef TEEK_CLIENT_EXT_H
#define TEEK_CLIENT_EXT_H

#include <linux/types.h>

/* update crl */
#ifdef CONFIG_CMS_SIGNATURE
uint32_t teek_update_crl(uint8_t *crl, uint32_t crl_len);
#endif

#endif
