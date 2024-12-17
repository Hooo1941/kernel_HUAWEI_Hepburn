/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver,synophys edma
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"
#include "oam_ext_if.h"

typedef enum {
    REGION_WCPU_ITCM,
    REGION_WCPU_DTCM,
    REGION_W_PKTRAM,
    REGION_W_PERP_AHB,
    REGION_BCPU_ITCM,
    REGION_BCPU_DTCM,
    REGION_B_PERP_AHB,
    REGION_AON_APB,
    REGION_BUTT
} mp13_regions;

/* Region大小必须为4KB的倍数，iATU要求 */
/* 这里的分段都是对应iATU inbound */
OAL_STATIC oal_pcie_region g_pcie_mpw2_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x0008BFFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x0008BFFF, /* 560KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_WCPU_ITCM)
    },
    {   .pci_start = 0x20000000,
        .pci_end = 0x20067FFF,
        .cpu_start = 0x20000000,
        .cpu_end = 0x20067FFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_WCPU_DTCM)
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x6008FFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x6008FFFF, /* 576KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_W_PKTRAM)
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40102FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40102FFF, /* 1036KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_W_PERP_AHB)
    },
    {   .pci_start = 0x80000000,
        .pci_end = 0x800FFFFF,
        .cpu_start = 0x80000000,
        .cpu_end = 0x800FFFFF, /* 1024KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_BCPU_ITCM)
    },
    {   .pci_start = 0x80100000,
        .pci_end = 0x801DFFFF,
        .cpu_start = 0x80100000,
        .cpu_end = 0x801DFFFF, /* 896KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_BCPU_DTCM)
    },
    {   .pci_start = 0x48000000,
        .pci_end = 0x48122FFF,
        .cpu_start = 0x48000000,
        .cpu_end = 0x48122FFF, /* 1164KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_B_PERP_AHB)
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000EDFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000EDFF, /* 59KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_AON_APB)
    },
};

OAL_STATIC oal_pcie_region g_pcie_pilot_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x000A7FFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x000A7FFF, /* 592KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_WCPU_ITCM)
    },
    {   .pci_start = 0x20018000,
        .pci_end = 0x2007FFFF,
        .cpu_start = 0x20018000,
        .cpu_end = 0x2007FFFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_WCPU_DTCM)
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x6007FFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x6007FFFF, /* 512KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_W_PKTRAM)
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40107FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40107FFF,
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_W_PERP_AHB)
    },
    {   .pci_start = 0x80040000,
        .pci_end = 0x8010FFFF,
        .cpu_start = 0x80040000,
        .cpu_end = 0x8010FFFF, /* 832KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_BCPU_ITCM)
    },
    {   .pci_start = 0x80200000,
        .pci_end = 0x8030FFFF,
        .cpu_start = 0x80200000,
        .cpu_end = 0x8030FFFF, /* 1088KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_BCPU_DTCM)
    },
    {   .pci_start = 0x48000000,
        .pci_end = 0x48122FFF,
        .cpu_start = 0x48000000,
        .cpu_end = 0x48122FFF, /* 1164KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_B_PERP_AHB)
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000EDFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000EDFF, /* 59KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(REGION_AON_APB)
    },
};


int32_t oal_pcie_get_bar_region_info_mp13(oal_pcie_res *pst_pci_res,
                                          oal_pcie_region **region_base, uint32_t *region_num)
{
    if (pst_pci_res->revision == PCIE_REVISION_4_70A) {
        *region_num = (uint32_t)oal_array_size(g_pcie_mpw2_regions);
        *region_base = &g_pcie_mpw2_regions[0];
    } else if (pst_pci_res->revision == PCIE_REVISION_5_00A) {
        *region_num = (uint32_t)oal_array_size(g_pcie_pilot_regions);
        *region_base = &g_pcie_pilot_regions[0];
    } else {
        pci_print_log(PCI_LOG_ERR, "unkown pcie ip revision :0x%x\n", pst_pci_res->revision);
        return -OAL_ENODEV;
    }
    return OAL_SUCC;
}

int32_t oal_pcie_set_outbound_membar_mp13(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar)
{
    pci_print_log(PCI_LOG_INFO, "no outbound!");
    return OAL_SUCC;
}

#endif

