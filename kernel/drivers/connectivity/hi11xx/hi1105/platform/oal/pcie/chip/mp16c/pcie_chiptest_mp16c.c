/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:Test code of the device pcie driver chip
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "oal_hcc_host_if.h"
#include "pcie_linux.h"

int32_t oal_pcie_ip_l1pm_check(oal_pcie_linux_res *pst_pci_lres);
int32_t oal_pcie_unmask_device_link_erros(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_mem_scanall(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_check_device_link_errors(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_gen_mode_check_mp16(oal_pcie_linux_res *pst_pci_lres);

// erroror：mp16c 切高频暂未配置
int32_t oal_pcie_device_changeto_high_cpufreq_mp16c(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    if (mpxx_is_asic()) {
        oal_udelay(30); /* 30us */
    }
    ret = oal_pcie_device_check_alive(pst_pci_res);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "change 640M failed");
        ssi_dump_device_regs(SSI_MODULE_MASK_ARM_REG);
        return ret;
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "change 640M done");
    return OAL_SUCC;
}

static int32_t oal_pcie_ip_factory_test_init(hcc_bus *pst_bus, oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret = oal_pcie_ip_init(pst_bus);
    if (ret) {
        return ret;
    }

    oal_msleep(5); /* 5ms wait pcie enter L1.2 */

    ret = oal_pcie_ip_l1pm_check(pst_pci_lres);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_voltage_bias_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_device_changeto_high_cpufreq_mp16c(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }
    return OAL_SUCC;
}

int32_t oal_pcie_ip_factory_test_mp16c(hcc_bus *pst_bus, int32_t test_count)
{
    int32_t i;
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;

    oal_print_mpxx_log(MPXX_LOG_INFO, "hello factory!!");

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    ret = oal_pcie_ip_factory_test_init(pst_bus, pst_pci_lres);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_gen_mode_check_mp16(pst_pci_lres);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_unmask_device_link_erros(pst_pci_lres->pst_pci_res);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "unmask device link err failed = %d", ret);
        return ret;
    }

    for (i = 0; i < test_count; i++) {
        /* memcheck */
        ret = oal_pcie_device_mem_scanall(pst_pci_lres->pst_pci_res);
        if (ret) {
            return ret;
        }

        oal_print_mpxx_log(MPXX_LOG_INFO, "scan all mem done , test %d times", i + 1);

        ret = oal_pcie_check_device_link_errors(pst_pci_lres->pst_pci_res);
        if (ret) {
            return ret;
        }

        ret = oal_pcie_gen_mode_check_mp16(pst_pci_lres);
        if (ret) {
            return ret;
        }
    }

    oal_msleep(5); /* wait 5ms to enter pcie L1.2 */

    ret = oal_pcie_ip_l1pm_check(pst_pci_lres);
    if (ret) {
        return ret;
    }

    return OAL_SUCC;
}

int32_t oal_pcie_chiptest_init_mp16c(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pcie_ip_factory_test = oal_pcie_ip_factory_test_mp16c;
    return OAL_SUCC;
}
#endif
