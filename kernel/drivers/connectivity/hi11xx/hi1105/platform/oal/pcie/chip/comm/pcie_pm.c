/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver,synophys edma
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_pm.h"
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
#include "oal_thread.h"
#include "oam_ext_if.h"
#include "pcie_linux.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "oal_kernel_file.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"
#include "securec.h"

uint32_t g_pcie_pcs_tracer_enable = 1;
oal_debug_module_param(g_pcie_pcs_tracer_enable, uint, S_IRUGO | S_IWUSR);

int32_t g_pcie_auxclk_switch = 1;
oal_debug_module_param(g_pcie_auxclk_switch, int, S_IRUGO | S_IWUSR);

static pcie_chip_id_stru g_pcie_pm_chip_id[] = {
    {PCIE_HISI_DEVICE_ID_MP13,     pcie_pm_chip_init_mp13},
    {PCIE_HISI_DEVICE_ID_MP15,     pcie_pm_chip_init_mp15},
    {PCIE_HISI_DEVICE_ID_MP16,     pcie_pm_chip_init_mp16},
    {PCIE_HISI_DEVICE_ID_MP16FPGA, pcie_pm_chip_init_mp16},
    {PCIE_HISI_DEVICE_ID_MP16C,    pcie_pm_chip_init_mp16c}
};

int32_t oal_pcie_pm_chip_init(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    int32_t i;
    int32_t chip_num = (int32_t)oal_array_size(g_pcie_pm_chip_id);
    for (i = 0; i < chip_num; i++) {
        if (g_pcie_pm_chip_id[i].device_id == device_id) {
            if (g_pcie_pm_chip_id[i].func != NULL) {
                return g_pcie_pm_chip_id[i].func(pst_pci_res, device_id);
            } else {
                oal_print_mpxx_log(MPXX_LOG_INFO, "pm chip init 0x%x init null func", device_id);
                return OAL_SUCC;
            }
        }
    }
    oal_print_mpxx_log(MPXX_LOG_ERR, "not implement pm chip init device_id=0x%x", device_id);
    return -OAL_ENODEV;
}

int32_t oal_pcie_device_aspm_init(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_init != NULL) {
        return pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_init(pst_pci_res);
    }
    return -OAL_ENODEV;
}

/* 时钟分频要在低功耗关闭下配置 */
int32_t oal_pcie_device_auxclk_init(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_init != NULL) {
        return pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_init(pst_pci_res);
    }
    return -OAL_ENODEV;
}

void oal_pcie_device_xfer_pending_sig_clr(oal_pcie_res *pst_pci_res, oal_bool_enum clear)
{
    if (pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl != NULL) {
        pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl(pst_pci_res, clear);
    }
}

int32_t oal_pcie_device_sr_debug(oal_pcie_res *pst_pci_res, oal_bool_enum in_sleep)
{
    if (pst_pci_res->chip_info.cb.pm_cb.pcie_device_pm_debug != NULL) {
        return pst_pci_res->chip_info.cb.pm_cb.pcie_device_pm_debug(pst_pci_res, in_sleep);
    }

    return -OAL_ENODEV;
}

int32_t oal_pcie_device_auxclk_switch(oal_pcie_res *pst_pci_res, aux_clk_mode clk_mode)
{
    if ((g_pcie_auxclk_switch != 0) && (pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_switch != NULL)) {
        return pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_switch(pst_pci_res, clk_mode);
    }

    return -OAL_ENODEV;
}

void oal_pcie_enable_master(oal_pcie_res *pst_pci_res, oal_pci_dev_stru *pst_pcie_dev, oal_bool_enum enable)
{
    if (pst_pci_res->chip_info.cb.pm_cb.pcie_enable_master != NULL) {
        pst_pci_res->chip_info.cb.pm_cb.pcie_enable_master(pst_pci_res, pst_pcie_dev, enable);
    }
}

/*
* Prototype    : oal_pcie_device_phy_config
* Description  : PCIE PHY配置
* Output       : int32_t : OAL_TURE means set succ
*/
int32_t oal_pcie_device_phy_config(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.cb.pm_cb.pcie_device_phy_config != NULL) {
        return pst_pci_res->chip_info.cb.pm_cb.pcie_device_phy_config(pst_pci_res);
    }
    return -OAL_ENODEV;
}

#endif
