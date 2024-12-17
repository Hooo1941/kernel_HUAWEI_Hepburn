/*
 * zodiac mmc sdhci controller interface.
 * Copyright (c) Zodiac Technologies Co., Ltd. 2017-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SDHCI_ZODIAC_ANTLIA_H
#define __SDHCI_ZODIAC_ANTLIA_H
#ifdef CONFIG_MMC_SDHCI_ZODIAC_ANTLIA

/* hsdt1 sd */
#define PERDIS0	0x4
#define PERCLKEN0	0x8
#define PEREN4	0x40
#define PERDIS4	0x44
#define PERRSTEN0	0x60
#define PERRSTDIS0	0x64
#define PERRSTEN1	0x6c
#define PERRSTDIS1	0x70

#define IP_RST_SD	(0x1 << 1)
#define IP_HRST_SD	(0x1 << 0)

#define SMU_APB_M	(0x1 << 2)
#define IP_ARST_TBU_SD	(0x1 << 3)

#define GT_HCLK_SD	0x1
#define GT_TBU_SD	(0x1 << 6)
#define GT_APB_S	(0x1 << 5)
#define GT_APB_M	(0x1 << 4)

#define PMC_CONFIG_ENABLE	(0x1 << 11)
#define PMC_CONFIG_ENABLE_MASK	(0x1 << 27)

#define QIC_POWER_IDLEREQ_0	0x380
#define MAX_TUNING_LOOP 40

void sdhci_zodiac_hardware_reset(struct sdhci_host *host);
void sdhci_zodiac_hardware_disreset(struct sdhci_host *host);
int sdhci_zodiac_get_resource(struct platform_device *pdev, struct sdhci_host *host);

#define sdhci_pmctrl_writel(host, val, reg)                                   \
	writel((val), (host)->pmctrl + (reg))
#define sdhci_pmctrl_readl(host, reg) readl((host)->pmctrl + (reg))
#endif
#endif
