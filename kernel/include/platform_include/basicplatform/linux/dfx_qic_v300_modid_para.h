/*
 *
 * qic v300 modid register.
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
 *
 */
#ifndef __DFX_QIC_V300_MODID_PARA_H__
#define __DFX_QIC_V300_MODID_PARA_H__

#ifdef CONFIG_DFX_SEC_QIC_V300_MODID_REGISTER
void dfx_qic_modid_register(u32 mid_start, u32 mid_end, u32 bus, u32 modid);
#else
static inline void dfx_qic_modid_register(u32 mid_start, u32 mid_end, u32 bus, u32 modid)
{
	(void)(mid_start);
	(void)(mid_end);
	(void)(bus);
	(void)(modid);
}
#endif

#endif
