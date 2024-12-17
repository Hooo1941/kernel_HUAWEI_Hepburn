/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver,synophys edma
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include "oam_ext_if.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"
#include "plat_pm.h"

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

OAL_STATIC oal_pcie_region g_pcie_asic_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x000BFFFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x000BFFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_WCPU_ITCM"
    },
    {   .pci_start = 0x20018000,
        .pci_end = 0x2007FFFF,
        .cpu_start = 0x20018000,
        .cpu_end = 0x2007FFFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_WCPU_DTCM"
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x600FFFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x600FFFFF, /* 512*2KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_W_PKTRAM"
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40107FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40107FFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_W_PERP_AHB"
    },
    {   .pci_start = 0x80000000,
        .pci_end = 0x8014FFFF,
        .cpu_start = 0x80000000,
        .cpu_end = 0x8014FFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_BCPU_ITCM"
    },
    {   .pci_start = 0x80200000,
        .pci_end = 0x8030FFFF,
        .cpu_start = 0x80200000,
        .cpu_end = 0x8030FFFF, /* 1088KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_BCPU_DTCM"
    },
    {   .pci_start = 0x48000000,
        .pci_end = 0x48001FFF,
        .cpu_start = 0x48000000,
        .cpu_end = 0x483FFFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_B_PERP_AHB"
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000FFFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000FFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_AON_APB"
    },
};

OAL_STATIC oal_pcie_region g_pcie_fpga_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x000BFFFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x000BFFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_WCPU_ITCM"
    },
    {   .pci_start = 0x20018000,
        .pci_end = 0x2007FFFF,
        .cpu_start = 0x20018000,
        .cpu_end = 0x2007FFFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_WCPU_DTCM"
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x6007FFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x6007FFFF, /* 512KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_W_PKTRAM"
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40107FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40107FFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_W_PERP_AHB"
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000FFFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000FFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "MP15_REGION_AON_APB"
    },
};

int32_t oal_pcie_get_bar_region_info_mp15(oal_pcie_res *pst_pci_res,
                                          oal_pcie_region **region_base, uint32_t *region_num)
{
    oal_print_mpxx_log(MPXX_LOG_INFO, "mp15 %s region map", mpxx_is_asic() ? "asic" : "fpga");
    if (mpxx_is_asic()) {
        /* mp15 Asic */
        *region_num = (uint32_t)oal_array_size(g_pcie_asic_regions);
        *region_base = &g_pcie_asic_regions[0];
    } else {
        /* mp15 Fpga */
        *region_num = (uint32_t)oal_array_size(g_pcie_fpga_regions);
        *region_base = &g_pcie_fpga_regions[0];
    }
    return OAL_SUCC;
}

#endif

