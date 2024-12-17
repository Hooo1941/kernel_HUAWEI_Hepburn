/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver,synophys edma
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
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
#include "oam_ext_if.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"

uint32_t g_pcie_pcs_tracer_enable = 1;
oal_debug_module_param(g_pcie_pcs_tracer_enable, uint, S_IRUGO | S_IWUSR);

int32_t g_pcie_auxclk_switch = 1;
oal_debug_module_param(g_pcie_auxclk_switch, int, S_IRUGO | S_IWUSR);

static pcie_chip_id_stru g_pcie_pm_chip_id[] = {
    {PCIE_HISI_DEVICE_ID_GF61,     pcie_pm_chip_init_gf61}
};

uint32_t pcs_trace_linkup_flag(void)
{
    return (uint32_t)((uint32_t)(g_pcie_pcs_tracer_enable >> 4) & 0xf); /* 右移4位，获取bit[7]的值 */
}

uint32_t pcs_trace_speedchange_flag(void)
{
    return (uint32_t)(g_pcie_pcs_tracer_enable & 0xf);
}

void oal_pcie_pcs_ini_config_init(void)
{
    int32_t l_cfg_value = 0;
    int32_t l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_PLAT, "pcs_trace_enable", &l_cfg_value);
    if (l_ret == INI_FAILED) {
        pci_print_log(PCI_LOG_INFO, "pcs_trace_enable not set, use defalut value=%d\n", g_pcie_pcs_tracer_enable);
        return;
    }

    /* 这里不判断有效性，在PHY初始化时判断，如果无效，不设置FFE */
    g_pcie_pcs_tracer_enable = (uint32_t)l_cfg_value;
    pci_print_log(PCI_LOG_INFO, "g_pcie_pcs_tracer_enable=0x%x", g_pcie_pcs_tracer_enable);
    return;
}

int32_t oal_pcie_pm_chip_init(oal_pcie_linux_res *pcie_res, int32_t device_id)
{
    uint32_t i;
    uint32_t chip_num = oal_array_size(g_pcie_pm_chip_id);
    for (i = 0; i < chip_num; i++) {
        if (g_pcie_pm_chip_id[i].device_id == device_id) {
            if (g_pcie_pm_chip_id[i].func != NULL) {
                return g_pcie_pm_chip_id[i].func(pcie_res, device_id);
            } else {
                oal_print_mpxx_log(MPXX_LOG_INFO, "pm chip init 0x%x init null func", device_id);
                return OAL_SUCC;
            }
        }
    }
    oal_print_mpxx_log(MPXX_LOG_ERR, "not implement pm chip init device_id=0x%x", device_id);
    return -OAL_ENODEV;
}

int32_t oal_pcie_device_aspm_init(oal_pcie_linux_res *pcie_res)
{
    if (pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_aspm_init != NULL) {
        return pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_aspm_init(pcie_res);
    }
    return -OAL_ENODEV;
}

int32_t oal_pcie_device_auxclk_switch(oal_pcie_linux_res *pcie_res, aux_clk_mode clk_mode)
{
    if ((g_pcie_auxclk_switch != 0) && (pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_auxclk_switch != NULL)) {
        return pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_auxclk_switch(pcie_res, clk_mode);
    }

    return -OAL_ENODEV;
}

/* 时钟分频要在低功耗关闭下配置 */
int32_t oal_pcie_device_auxclk_init(oal_pcie_linux_res *pcie_res)
{
    if (pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_auxclk_init != NULL) {
        return pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_auxclk_init(pcie_res);
    }
    return -OAL_ENODEV;
}

void oal_pcie_device_xfer_pending_sig_clr(oal_pcie_linux_res *pcie_res, oal_bool_enum clear)
{
    if (pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_aspm_ctrl != NULL) {
        pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_aspm_ctrl(pcie_res, clear);
    }
}

/*
* Prototype    : oal_pcie_device_phy_config
* Description  : PCIE PHY配置
* Output       : int32_t : OAL_TURE means set succ
*/
int32_t oal_pcie_device_phy_config(oal_pcie_linux_res *pcie_res)
{
    if (pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_phy_config != NULL) {
        return pcie_res->ep_res.chip_info.cb.pm_cb.pcie_device_phy_config(pcie_res);
    }
    return -OAL_ENODEV;
}
#endif
