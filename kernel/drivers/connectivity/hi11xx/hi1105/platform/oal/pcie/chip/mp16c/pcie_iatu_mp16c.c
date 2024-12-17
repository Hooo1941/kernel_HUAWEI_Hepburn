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

#include "chip/mp16c/pcie_soc_mp16c.h"


OAL_STATIC oal_pcie_region g_pcie_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end   = 0x000DFFFF,
        .cpu_start = 0x00000000,
        .cpu_end   = 0x000DFFFF, /* 896KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_ITCM"
    },
    {   .pci_start = 0x20000000,
        .pci_end   = 0x2007FFFF,
        .cpu_start = 0x20000000,
        .cpu_end   = 0x2007FFFF, /* 512KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_DTCM_S1"
    },
    {   .pci_start = 0x02000000,
        .pci_end   = 0x0207FFFF,
        .cpu_start = 0x02000000,
        .cpu_end   = 0x0207FFFF, /* 512KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_SHARE_MEM_S5"
    },
    {   .pci_start = 0x40000000,
        .pci_end   = 0x40123FFF,
        .cpu_start = 0x40000000,
        .cpu_end   = 0x40123FFF, /* 1168KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_AON_WAPB"
    },
    {   .pci_start = 0x44600000,
        .pci_end   = 0x446CFFFF,
        .cpu_start = 0x44600000,
        .cpu_end   = 0x446CFFFF, /* 832KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_BITCM"
    },
    {   .pci_start = 0x44700000,
        .pci_end   = 0x44733FFF,
        .cpu_start = 0x44700000,
        .cpu_end   = 0x44733FFF, /* 208KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_BDTCM"
    },
    {   .pci_start = 0x44400000,
        .pci_end   = 0x44457FFF,
        .cpu_start = 0x44400000,
        .cpu_end   = 0x44457FFF, /* 352KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_SLEROM"
    },
    {   .pci_start = 0x44460000,
        .pci_end   = 0x444DFFFF,
        .cpu_start = 0x44460000,
        .cpu_end   = 0x444DFFFF, /* 512KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_SLERAM"
    },
    {   .pci_start = 0x42026000,
        .pci_end   = 0x42027FFF,
        .cpu_start = 0x42026000,
        .cpu_end   = 0x42027FFF, /* 8KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_SLE_TRACE_MEM"
    },
    {   .pci_start = 0x44000000,
        .pci_end   = 0x440B7FFF,
        .cpu_start = 0x44000000,
        .cpu_end   = 0x440B7FFF, /* 736KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_GFITCM"
    },
    {   .pci_start = 0x44100000,
        .pci_end   = 0x441C7FFF,
        .cpu_start = 0x44100000,
        .cpu_end   = 0x441C7FFF, /* 800KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_GFDTCM"
    },
    {   .pci_start = 0x43026000,
        .pci_end   = 0x43027FFF,
        .cpu_start = 0x43026000,
        .cpu_end   = 0x43027FFF, /* 8KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "MP16C_GF_TRACE_MEM"
    }
};

int32_t oal_pcie_get_bar_region_info_mp16c(oal_pcie_res *pst_pci_res,
                                           oal_pcie_region **region_base, uint32_t *region_num)
{
    oal_print_mpxx_log(MPXX_LOG_INFO, "mp16c %s region map", mpxx_is_asic() ? "asic" : "fpga");
    *region_num = (uint32_t)oal_array_size(g_pcie_regions);
    *region_base = &g_pcie_regions[0];

    return OAL_SUCC;
}

int32_t oal_pcie_set_outbound_membar_mp16c(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar)
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

