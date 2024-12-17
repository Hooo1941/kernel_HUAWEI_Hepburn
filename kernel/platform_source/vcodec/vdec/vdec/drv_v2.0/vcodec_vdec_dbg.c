/*
 * vcodec_vdec_dbg.c
 *
 * This is for vdec debug
 *
 * Copyright (c) 2020-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "vcodec_vdec_dbg.h"
#include "securec.h"
#include "dbg.h"
#include <linux/kernel.h>
#include <stddef.h>

#define VDEC_MIN_RESERVED_PHYADDR 0x80000000
#define VDEC_MAX_RESERVED_PHYADDR 0xA0000000
const struct vdec_setting_option setting_options[] = {
	{ "smmu_glb_bypass", "smmu glb bypass",
		offsetof(struct vdec_setting_info, smmu_glb_bypass),
		SETTING_OPT_TYPE_BOOL, {.enable = false}, 0, 1},

	{ "vdec_reserved_phyaddr", "vdec reserved memory",
		offsetof(struct vdec_setting_info, vdec_reserved_phyaddr),
		SETTING_OPT_TYPE_UINT64, {.i64 = VDEC_MIN_RESERVED_PHYADDR}, VDEC_MIN_RESERVED_PHYADDR, VDEC_MAX_RESERVED_PHYADDR},
};

void vdec_init_setting_info(const struct vdec_setting_info *info)
{
	size_t i;
	void *dst = NULL;

	for (i = 0; i < ARRAY_SIZE(setting_options); i++) {
		const struct vdec_setting_option *option = &setting_options[i];
		dst = ((unsigned char *)info) + option->offset;
		switch (option->type) {
		case SETTING_OPT_TYPE_BOOL:
			*(bool *)dst = option->default_value.enable;
			break;
		case SETTING_OPT_TYPE_INT:
			*(int *)dst = option->default_value.i32;
			break;
		case SETTING_OPT_TYPE_UINT:
			*(unsigned int *)dst = (unsigned int)option->default_value.i32;
			break;
		case SETTING_OPT_TYPE_INT64:
			*(long long *)dst = (long long)option->default_value.i64;
			break;
		case SETTING_OPT_TYPE_UINT64:
			*(unsigned long long *)dst = (unsigned long long)option->default_value.i64;
			break;
		case SETTING_OPT_TYPE_STRING: {
				int ret = strcpy_s((unsigned char *)dst, MAX_SETTING_STRING_LEN, option->default_value.str);
				if (ret != 0)
					dprint(PRN_ERROR, "Failed to init string setting value. ret:%d.", ret);
		}
			break;
		default:
			break;
		}
	}
}

