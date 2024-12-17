/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:Test code of the device pcie driver chip
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
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
#include "securec.h"
#include "plat_pm.h"

int32_t oal_pcie_rc_slt_chip_transfer(hcc_bus *pst_bus, void *ddr_address,
                                      uint32_t data_size, int32_t direction)
{
    oal_pcie_linux_res *pst_pci_lres = NULL;
    if (pst_bus == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pst_bus is null");
        return -OAL_ENODEV;
    }

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pst_pci_lres is null");
        return -OAL_ENODEV;
    }

    if (direction == 1) { /* 1表示 Host to Device */
        return oal_pcie_copy_to_device_by_dword(pst_pci_lres, ddr_address,
                                                oal_pcie_slt_get_deivce_ram_cpuaddr(pst_pci_lres),
                                                data_size);
    } else if (direction == 2) { /* 2表示 Device to Host */
        return oal_pcie_copy_from_device_by_dword(pst_pci_lres, ddr_address,
                                                  oal_pcie_slt_get_deivce_ram_cpuaddr(pst_pci_lres),
                                                  data_size);
    } else {
        oal_print_mpxx_log(MPXX_LOG_ERR, "invaild direction:%d", direction);
        return -OAL_EINVAL;
    }
}

int32_t oal_pcie_get_vol_reg_0v9_value(int32_t request_vol, uint32_t *pst_value)
{
    return OAL_SUCC;
}

int32_t oal_pcie_get_vol_reg_1v8_value(int32_t request_vol, uint32_t *pst_value)
{
    return OAL_SUCC;
}
#endif
