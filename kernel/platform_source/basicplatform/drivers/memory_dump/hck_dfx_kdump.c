 /*
 *
 * hck kerneldump module
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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

#include <linux/page-flags.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <platform_include/basicplatform/linux/mntn_dump.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_kdump.h>
#include "kernel_dump.h"

static void kdump_cma_set_page_flag(struct page *page, size_t cnt)
{
	u64 k;

	if (page != NULL)
		for (k = 0; k < cnt; k++)
			SetPageMemDump(page++);
}

static void kdump_ion_set_page_flag(gfp_t gft_highmem, struct page *page)
{
	if (gft_highmem != 0 && page != NULL)
		SetPageMemDump(page);
}

void hck_dfx_kdump_register(void)
{
	REGISTER_HCK_VH(hck_dfx_kdump_cma_set_page_flag, kdump_cma_set_page_flag);
	REGISTER_HCK_VH(hck_dfx_kdump_reserved_addr_save, kdump_reserved_addr_save);
	REGISTER_HCK_VH(hck_dfx_kdump_ion_set_page_flag, kdump_ion_set_page_flag);
}
EXPORT_SYMBOL(hck_dfx_kdump_register);
