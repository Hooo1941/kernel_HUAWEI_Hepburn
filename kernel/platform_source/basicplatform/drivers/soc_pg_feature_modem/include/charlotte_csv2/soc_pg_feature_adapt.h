/*
 * parse soc pg feature adapt
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
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
#ifndef _SOC_PG_FEATURE_ADAPT_H_
#define _SOC_PG_FEATURE_ADAPT_H_

#include <soc_efuse_chip_level_flag.h>
#include <linux/string.h>

#define EFUSE_SOC_ERROR "efuse_soc_error"

#define EFUSE_PARTIALGOOD_FLAG6_BUFF_SIZE 4
#define EFUSE_PARTIALGOOD_FLAG7_BUFF_SIZE 4

#define PG_BIT    CHIP_LEVEL_EFUSE_FLAG_MODEM_START
#define MODEM_BIT CHIP_LEVEL_EFUSE_FLAG_MODEM_VOL_ADJUST_START
#define BBP_BIT   CHIP_LEVEL_EFUSE_FLAG_BBP_VOL_ADJUST_START
#define AVS_BIT   CHIP_LEVEL_EFUSE_FLAG_AVS_ADJUST_MODEM_START

#define EFUSE_PG_INDEX    3
#define EFUSE_PG_BIT      18
#define EFUSE_PG_MASK     (1 << EFUSE_PG_BIT)

#define EFUSE_MODEM_INDEX 1
#define EFUSE_MODEM_BIT   23
#define EFUSE_MODEM_MASK  (1 << EFUSE_MODEM_BIT)

#define EFUSE_BBP_INDEX   1
#define EFUSE_BBP_BIT     24
#define EFUSE_BBP_MASK    (1 << EFUSE_BBP_BIT)

#define EFUSE_AVS_INDEX   2
#define EFUSE_AVS_BIT     10
#define EFUSE_AVS_MASK    (1 << EFUSE_AVS_BIT)

#define LEVEL_1           0xE
#define LEVEL_2           0x9
#define LEVEL_AVS         0x8
#define NOT_SUPPORT       0x7
#define LEVLE_1_MODEM     0xF

static inline unsigned int is_have_modem(const char *dts_str)
{
	if (strncmp(dts_str, "ap_pad", strlen("ap_pad")) == 0 ||
		strncmp(dts_str, "pc", strlen("pc")) == 0 ||
		strncmp(dts_str, "unknown", strlen("unknown")) == 0)
		return 0;
	return 1;
}

#endif /* _SOC_PG_FEATURE_ADAPT_H_ */
