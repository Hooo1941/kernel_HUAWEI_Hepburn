/*
 *
 * hck mpam module
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <platform_include/cee/linux/mpam_sched.h>
#include <platform_include/cee/linux/hck/lpcpu/mpam/hck_mpam.h>


static void hck_mpam_sched_in(bool hook_flag)
{
#ifdef CONFIG_MPAM
	mpam_sched_in();
#endif
}

static int __init hck_mpam_register(void)
{
	REGISTER_HCK_VH(hck_mpam_sched_in, hck_mpam_sched_in);
	return 0;
}

early_initcall(hck_mpam_register);
