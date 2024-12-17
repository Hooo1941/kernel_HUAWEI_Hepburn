/*
 * mntn_dump.h
 *
 * dump the register of mntn.
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
#ifndef __MNTN_DUMP_H
#define __MNTN_DUMP_H

#include <linux/types.h>
#include <mntn_public_interface.h>
#include <linux/version.h>

#ifdef CONFIG_DFX_MNTNDUMP
extern int register_mntn_dump(int mod_id, unsigned int size, void **vaddr);
extern u32 checksum32(u32 *addr, u32 count);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
void dfx_set_phystart_addr(s64 addr);
#endif
#else
static inline int register_mntn_dump(int mod_id, unsigned int size, void **vaddr)
{
	return -1;
}
static inline u32 checksum32(u32 *addr, u32 count)
{
		return 0;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static inline void dfx_set_phystart_addr(s64 addr)
{
	return;
}
#endif
#endif

#endif
