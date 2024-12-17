/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie chip adaptation
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#define MPXX_LOG_MODULE_NAME     "[PCIEC]"
#define MPXX_LOG_MODULE_NAME_VAR pciec_loglevel
#define HISI_LOG_TAG               "[PCIEC]"

#include "chip/gf61/pcie_chip_gf61.h"

#include "pcie_pm.h"

static pcie_chip_id_stru g_pcie_chipinfo_id[] = {
    {PCIE_HISI_DEVICE_ID_GF61,     oal_pcie_chip_info_init_gf61}
};

static int32_t oal_pcie_chip_info_res_init(oal_pcie_linux_res *pcie_res, int32_t device_id)
{
    uint32_t i;
    uint32_t chip_num = oal_array_size(g_pcie_chipinfo_id);
    for (i = 0; i < chip_num; i++) {
        if (g_pcie_chipinfo_id[i].device_id == device_id) {
            if (g_pcie_chipinfo_id[i].func != NULL) {
                return g_pcie_chipinfo_id[i].func(pcie_res, device_id);
            } else {
                oal_print_mpxx_log(MPXX_LOG_INFO, "pcie chip init 0x%x init null func", device_id);
                return OAL_SUCC;
            }
        }
    }
    oal_print_mpxx_log(MPXX_LOG_ERR, "not implement chip info init device_id=0x%x", device_id);
    return -OAL_ENODEV;
}

int32_t oal_pcie_chip_info_init(oal_pcie_linux_res *pcie_res)
{
    int32_t ret;
    uint32_t device_id;
    oal_pci_dev_stru *pst_pci_dev = pcie_res->comm_res->pcie_dev;
    size_t len = sizeof(pcie_res->ep_res.chip_info.addr_info);

    (void)memset_s(&pcie_res->ep_res.chip_info.addr_info, len, 0, len);

    device_id = (uint32_t)pst_pci_dev->device;
    ret = oal_pcie_pm_chip_init(pcie_res, device_id);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_chip_info_res_init(pcie_res, device_id);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return ret;
}

unsigned long oal_pcie_slt_get_deivce_ram_cpuaddr(oal_pcie_linux_res *pcie_res)
{
    if (pcie_res->ep_res.chip_info.cb.get_test_ram_address == NULL) {
        return 0xffffffff; // invalid address
    }
    return (unsigned long)pcie_res->ep_res.chip_info.cb.get_test_ram_address();
}

int32_t oal_pcie_host_slave_address_switch(oal_pcie_linux_res *pcie_res, uint64_t src_addr,
                                           uint64_t* dst_addr, int32_t is_host_iova)
{
    if (oal_warn_on(pcie_res->ep_res.chip_info.cb.pcie_host_slave_address_switch == NULL)) {
        *dst_addr = 0;
        return -OAL_ENODEV; // needn't address switch
    }
    return pcie_res->ep_res.chip_info.cb.pcie_host_slave_address_switch(pcie_res, src_addr, dst_addr, is_host_iova);
}

#endif
