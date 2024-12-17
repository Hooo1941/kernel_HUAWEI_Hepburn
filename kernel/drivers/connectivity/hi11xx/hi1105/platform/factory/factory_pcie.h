/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: factory_pcie.c header file
 * Author: @CompanyNameTag
 */

#ifndef FACTORY_PCIE_H
#define FACTORY_PCIE_H

/* 其他头文件包含 */
#if (defined(CONFIG_PCIE_KIRIN_SLT_HI110X)|| defined(CONFIG_PCIE_KPORT_SLT_DEVICE)) && defined(CONFIG_HISI_DEBUG_FS)
int32_t mp13_pcie_chip_rc_slt_register(void);
int32_t mp13_pcie_chip_rc_slt_unregister(void);
#endif

#endif
