/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie chip adaptation
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIEC]"
#define HISI_LOG_TAG           "[PCIEC]"
#include "pcie_linux.h"

/* mp13 & mp15 Registers */
#define PA_PCIE_DMA_CTRL_BASE_ADDR  0x40008000 /* 对应PCIE_DMA_CTRL页 */
#define PA_PCIE_CTRL_BASE_ADDR      0x40007000 /* 对应PCIE_CTRL页 */
#define PA_PCIE_CONFIG_BASE_ADDRESS 0x40102000
#define PA_GLB_CTL_BASE_ADDR        0x50000000
#define PA_PMU_CMU_CTL_BASE         0x50002000
#define PA_PMU2_CMU_IR_BASE         0x50003000
#define PA_W_CTL_BASE               0x40000000

#define DEV_VERSION_CPU_ADDR       0x00002700
#define DEV_VERSION_CPU_ADDR_PIOLT 0x00000180

int32_t oal_pcie_voltage_bias_init_mp13(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_get_bar_region_info_mp13(oal_pcie_res *pst_pci_res, oal_pcie_region **, uint32_t *);
int32_t oal_pcie_get_bar_region_info_mp15(oal_pcie_res *pst_pci_res, oal_pcie_region **, uint32_t *);
int32_t oal_pcie_set_outbound_membar_mp13(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar);

static uintptr_t oal_pcie_get_test_ram_address(void)
{
    return 0x20020000; // from 0x20020000, skip dtcm 32k bootrom stack & rw region
}

static void oal_pcie_chip_info_cb_init(pcie_chip_cb *cb, uint32_t device_id)
{
    cb->get_test_ram_address = oal_pcie_get_test_ram_address;
    cb->pcie_voltage_bias_init = oal_pcie_voltage_bias_init_mp13;
    cb->pcie_set_outbound_membar = oal_pcie_set_outbound_membar_mp13;
    cb->pcie_host_slave_address_switch = NULL;

    if (device_id == PCIE_HISI_DEVICE_ID_MP13) {
        cb->pcie_get_bar_region_info = oal_pcie_get_bar_region_info_mp13;
    }

    if (device_id == PCIE_HISI_DEVICE_ID_MP15) {
        cb->pcie_get_bar_region_info = oal_pcie_get_bar_region_info_mp15;
    }
}

int32_t oal_pcie_chip_info_init_mp13(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    oal_print_mpxx_log(MPXX_LOG_INFO, "oal_pcie_chip_info_init_MP13");
    pst_pci_res->chip_info.edma_support = OAL_TRUE;
    pst_pci_res->chip_info.addr_info.edma_ctrl = PA_PCIE_DMA_CTRL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.pcie_ctrl = PA_PCIE_CTRL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.dbi = PA_PCIE_CONFIG_BASE_ADDRESS;
    pst_pci_res->chip_info.addr_info.glb_ctrl = PA_GLB_CTL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.pmu_ctrl = PA_PMU_CMU_CTL_BASE;
    pst_pci_res->chip_info.addr_info.pmu2_ctrl = PA_PMU2_CMU_IR_BASE;
    pst_pci_res->chip_info.addr_info.sharemem_addr = PA_PCIE_CTRL_BASE_ADDR + 0x2e4; /* 0x2e4 device reg1 */
    if (pst_pci_res->revision == PCIE_REVISION_5_00A) {
        pst_pci_res->chip_info.membar_support = OAL_TRUE;
        pst_pci_res->chip_info.addr_info.boot_version = DEV_VERSION_CPU_ADDR_PIOLT;
    } else {
        pst_pci_res->chip_info.membar_support = OAL_FALSE;
        pst_pci_res->chip_info.addr_info.boot_version = DEV_VERSION_CPU_ADDR;
    }
    oal_pcie_chip_info_cb_init(&pst_pci_res->chip_info.cb, device_id);
    return OAL_SUCC;
}
#endif
