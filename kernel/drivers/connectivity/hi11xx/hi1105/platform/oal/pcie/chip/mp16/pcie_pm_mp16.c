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
#include "pcie_chip.h"
#include "pcie_chip_mp16.h"
#include "pcie_linux.h"
#include "mp16/pcie_pcs_rb_regs.h"
#include "pcie_pcs_host.h"

int32_t oal_pcie_device_aspm_init_mp13(oal_pcie_res *pst_pci_res);
void oal_pcie_device_aspm_ctrl_mp13(oal_pcie_res *pst_pci_res, oal_bool_enum clear);

int32_t oal_pcie_device_phy_config_mp16(oal_pcie_res *pst_pci_res)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res));

    /* pcie0 phy寄存器优化 */
    oal_pcie_device_phy_config_single_lane(pst_pci_res, MP16_PA_PCIE0_PHY_BASE_ADDRESS);

    /* ASPM L1sub power_on_time bias */
    oal_pcie_config_l1ss_poweron_time(pst_pci_lres->pst_pcie_dev);

    /* GEN1->GENx link speed change */
    (void)oal_pcie_link_speed_change(pst_pci_lres->pst_pcie_dev, pst_pci_res);

    oal_pcie_device_phy_disable_l1ss_rekey(pst_pci_res, MP16_PA_PCIE0_PHY_BASE_ADDRESS);

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    /* dual pcie */
    if (pst_pci_lres->pst_pcie_dev_dual != NULL) {
        /* pcie1 phy寄存器优化 */
        oal_pcie_device_phy_config_single_lane(pst_pci_res, MP16_PA_PCIE1_PHY_BASE_ADDRESS);

        /* ASPM L1sub power_on_time bias */
        oal_pcie_config_l1ss_poweron_time(pst_pci_lres->pst_pcie_dev_dual);

        /* GEN1->GENx link speed change */
        (void)oal_pcie_link_speed_change(pst_pci_lres->pst_pcie_dev_dual, pst_pci_res);

        oal_pcie_device_phy_disable_l1ss_rekey(pst_pci_res, MP16_PA_PCIE1_PHY_BASE_ADDRESS);
    }
#endif

    return OAL_SUCC;
}

/* 时钟分频要在低功耗关闭下配置 */
int32_t oal_pcie_device_auxclk_init_mp16(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    uint32_t value;
    pci_addr_map st_map;

    value = oal_readl(pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);
    /* 保留value bit[10]以上, aux_clk 1M, synophys set */
    value &= (~((1 << 10) - 1));
    value |= 0x1;
    oal_writel(value, pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);

    /* tcxo 38.4M 39分频 = 0.98M 接近1M */
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, (0x40000000 + 0x2c), &st_map);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wcpu address  convert failed, ret=%d", ret);
        return ret;
    }

    value = oal_readl((void *)st_map.va);
    value &= (~(((1 << 6) - 1) << 8)); // 6, 8, bit pos
    value |= (0x27 << 8); // 8, bit pos
    oal_writel(value, (void *)st_map.va);

    oal_print_mpxx_log(MPXX_LOG_INFO, "clk_freq reg:0x%x, freq_div reg:0x%x",
                       oal_readl(pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF),
                       oal_readl((void *)st_map.va));
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_enable_master_mp16(oal_pcie_res *pst_pci_res, oal_pci_dev_stru *pst_pcie_dev,
                                            oal_bool_enum enable)
{
    // mp16/mp16c切32k特性不配置set/clear master
}

int32_t pcie_pm_chip_init_mp16(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_init = oal_pcie_device_aspm_init_mp13;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_init = oal_pcie_device_auxclk_init_mp16;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl = oal_pcie_device_aspm_ctrl_mp13;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_phy_config = oal_pcie_device_phy_config_mp16;
    pst_pci_res->chip_info.cb.pm_cb.pcie_enable_master = oal_pcie_enable_master_mp16;
    return OAL_SUCC;
}
#endif
