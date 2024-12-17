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

#include "chip/mp16/pcie_soc_mp16.h"


OAL_STATIC oal_pcie_region g_pcie_fpga_regions[] = {
    {   .pci_start = 0x00000000 + 0x3000000,  /* 0x3000000 : 外设ACP地址固定偏移 */
        .pci_end   = 0x00003FFF + 0x3000000,
        .cpu_start = 0x00000000,
        .cpu_end   = 0x00003FFF, /* 16K */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BOOT"
    },
    {   .pci_start = 0x00040000 + 0x3000000,
        .pci_end   = 0x001bFFFF + 0x3000000,
        .cpu_start = 0x00040000,
        .cpu_end   = 0x001bFFFF, /* 1536KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_WRAM_ACP"
    },
    {   .pci_start = 0x02000000 + 0x3000000,
        .pci_end   = 0x020A1FFF + 0x3000000,
        .cpu_start = 0x02000000,
        .cpu_end   = 0x020A1FFF, /* 648KB */
        .flag      = OAL_IORESOURCE_REG,
        /* 和芯片约束，PCIE不使用S7端口 */
        .name      = "MP16_SHARE_MEM_S6_ACP"
    },
    {   .pci_start = 0x40000000,
        .pci_end   = 0x4013FFFF,
        .cpu_start = 0x40000000,
        .cpu_end   = 0x4013FFFF, /* 1280KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_AON_APB"
    },
    {   .pci_start = 0x44000000,
        .pci_end   = 0x44121FFF,
        .cpu_start = 0x44000000,
        .cpu_end   = 0x44121FFF, /* 1160KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BITCM"
    },
    {   .pci_start = 0x44200000,
        .pci_end   = 0x442FFFFF,
        .cpu_start = 0x44200000,
        .cpu_end   = 0x442FFFFF, /* 1024KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BDTCM"
    },
    {   .pci_start = 0x44400000,
        .pci_end   = 0x4446FFFF,
        .cpu_start = 0x44400000,
        .cpu_end   = 0x4446FFFF, /* 448KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_GITCM"
    },
    {   .pci_start = 0x44500000,
        .pci_end   = 0x44601FFF,
        .cpu_start = 0x44500000,
        .cpu_end   = 0x44601FFF, /* 1032KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_GDTCM"
    },
    {   .pci_start = 0x4021B000,
        .pci_end   = 0x4021EFFF,
        .cpu_start = 0x4021B000,
        .cpu_end   = 0x4021EFFF, /* 16KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BCPU_CPUTRACE"
    },
    {   .pci_start = 0x40312000,
        .pci_end   = 0x40315FFF,
        .cpu_start = 0x40312000,
        .cpu_end   = 0x40315FFF, /* 16KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_GCPU_CPUTRACE"
    }
};

OAL_STATIC oal_pcie_region g_pcie_asic_regions[] = {
    {   .pci_start = 0x00000000 + 0x3000000,  /* 0x3000000 : 外设ACP地址固定偏移 */
        .pci_end   = 0x00003FFF + 0x3000000,
        .cpu_start = 0x00000000,
        .cpu_end   = 0x00003FFF, /* 16K */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BOOT"
    },
    {   .pci_start = 0x00040000 + 0x3000000,
        .pci_end   = 0x001bFFFF + 0x3000000,
        .cpu_start = 0x00040000,
        .cpu_end   = 0x001bFFFF, /* 1536KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_WRAM_NON_ACP"
    },
    {   .pci_start = 0x02000000 + 0x3000000,
        .pci_end   = 0x020A1FFF + 0x3000000,
        .cpu_start = 0x02000000,
        .cpu_end   = 0x020A1FFF, /* 648KB */
        .flag      = OAL_IORESOURCE_REG,
        /* 和芯片约束，PCIE不使用S7端口 */
        .name      = "MP16_SHARE_MEM_S6_NON_ACP"
    },
    {   .pci_start = 0x40000000,
        .pci_end   = 0x4013FFFF,
        .cpu_start = 0x40000000,
        .cpu_end   = 0x4013FFFF, /* 1280KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_AON_APB"
    },
    {   .pci_start = 0x44000000,
        .pci_end   = 0x44121FFF,
        .cpu_start = 0x44000000,
        .cpu_end   = 0x44121FFF, /* 1160KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BITCM"
    },
    {   .pci_start = 0x44200000,
        .pci_end   = 0x442FFFFF,
        .cpu_start = 0x44200000,
        .cpu_end   = 0x442FFFFF, /* 1024KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BDTCM"
    },
    {   .pci_start = 0x44400000,
        .pci_end   = 0x4446FFFF,
        .cpu_start = 0x44400000,
        .cpu_end   = 0x4446FFFF, /* 448KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_GITCM"
    },
    {   .pci_start = 0x44500000,
        .pci_end   = 0x44601FFF,
        .cpu_start = 0x44500000,
        .cpu_end   = 0x44601FFF, /* 1032KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_GDTCM"
    },
    {   .pci_start = 0x4021B000,
        .pci_end   = 0x4021EFFF,
        .cpu_start = 0x4021B000,
        .cpu_end   = 0x4021EFFF, /* 16KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_BCPU_CPUTRACE"
    },
    {   .pci_start = 0x40312000,
        .pci_end   = 0x40315FFF,
        .cpu_start = 0x40312000,
        .cpu_end   = 0x40315FFF, /* 16KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16_GCPU_CPUTRACE"
    }
};

int32_t oal_pcie_get_bar_region_info_mp16(oal_pcie_res *pst_pci_res,
                                          oal_pcie_region **region_base, uint32_t *region_num)
{
    oal_print_mpxx_log(MPXX_LOG_INFO, "mp16 %s region map", mpxx_is_asic() ? "asic" : "fpga");
    if (mpxx_is_asic()) {
        /* mp16 Asic, same as mp16 Fpga */
        *region_num = (uint32_t)oal_array_size(g_pcie_asic_regions);
        *region_base = &g_pcie_asic_regions[0];
    } else {
        /* mp16 Fpga */
        *region_num = (uint32_t)oal_array_size(g_pcie_fpga_regions);
        *region_base = &g_pcie_fpga_regions[0];
    }
    return OAL_SUCC;
}

int32_t oal_pcie_set_outbound_membar_mp16(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar)
{
    int32_t ret;
    ret = oal_pcie_set_outbound_iatu_by_membar(pst_iatu_bar->st_region.vaddr,
                                               0, HISI_PCIE_SLAVE_START_ADDRESS,
                                               HISI_PCIE_MASTER_START_ADDRESS,
                                               HISI_PCIE_IP_REGION_SIZE);
    if (ret) {
        return ret;
    }
    return OAL_SUCC;
}

#endif

