/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: HiPCIe phy interface define.
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef HSDT_BUS_API_H
#define HSDT_BUS_API_H
#include <linux/types.h>

enum hsdt_bus {
	HSDT_BUS_0 = 0,
	HSDT_BUS_1,
};

enum ecc_irq {
	DETECT_ECC_IRQ = 0,
	MULTPL_ECC_IRQ,
};

#ifdef CONFIG_HSDT_BUS_DOMAIN
int hsdt_bus_ecc_trigger_register(enum hsdt_bus bus_index, enum ecc_irq type,
		u32 triger, void (*handler)(void *data), void *data);
#else
static inline int hsdt_bus_ecc_trigger_register(enum hsdt_bus bus_index, enum ecc_irq type,
		u32 triger, void (*handler)(void *data), void *data)
{
	return 0;
}
#endif
#endif
