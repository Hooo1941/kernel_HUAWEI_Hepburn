/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie pm
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"
#include "oal_types.h"
#include "pcie_chip_gf61.h"
#include "gf61/pcie_ctrl_rb_regs.h"
#include "gf61/pcie_pcs_rb_regs.h"

int32_t oal_pcie_device_phy_config_gf61(oal_pcie_linux_res *pcie_res)
{
    if (pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "ptr pcie_res is NULL");
        return OAL_FAIL;
    }
    if (oal_pcie_eye_param_change_gf61(pcie_res) != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "set pcie param error, not change pcie speed");
        return OAL_FAIL;
    }
    /* GEN1->GENx link speed change */
    (void)oal_pcie_link_speed_change_gf61(pcie_res);
    return OAL_SUCC;
}

int32_t pcie_pm_chip_init_gf61(oal_pcie_linux_res *pcie_res, int32_t device_id)
{
    oal_pcie_ep_res *ep_res = &pcie_res->ep_res;

    ep_res->chip_info.cb.pm_cb.pcie_device_aspm_init = NULL;
    ep_res->chip_info.cb.pm_cb.pcie_device_auxclk_init = NULL;
    ep_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl = NULL;
    ep_res->chip_info.cb.pm_cb.pcie_device_phy_config = oal_pcie_device_phy_config_gf61;
    return OAL_SUCC;
}
#endif
