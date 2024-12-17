/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:Test code of the device pcie driver chip
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
#include "oal_thread.h"
#include "pcie_linux.h"

#define GLB_CTL_RB_BASE                            0x40000000
#define W_CTL_RB_BASE                              0x40105000
#define W_CTL_RB_CMU_CLK_SEL_REG                   (W_CTL_RB_BASE + 0x1734) /* 时钟配置寄存器 */
#define W_CTL_RB_CMU_CPU_CLKEN_REG                 (W_CTL_RB_BASE + 0x1468)
#define GLB_CTL_RB_AON_PERP_CLKSEL_W_REG           (GLB_CTL_RB_BASE + 0x70)
#define W_CTL_RB_WCPU_CLK_DIV_CFG_REG              (W_CTL_RB_BASE + 0x200)
#define W_CTL_RB_SDIO_CLK_DIV_CFG_REG              (W_CTL_RB_BASE + 0x173C)
#define W_CTL_RB_BUS_CLK_DIV_CFG_REG               (W_CTL_RB_BASE + 0x204)
#define W_CTL_RB_CLKMUX_SEL_REG                    (W_CTL_RB_BASE + 0x210)
#define W_CTL_RB_WTOP_MUX_CLK_STS_REG              (W_CTL_RB_BASE + 0x214)
#define W_CTL_RB_TCXO_SSI_CPU_MUX_CLKB_STS_MASK    0x2
#define RF_CLK_PLL_RB_BASE                         0x4011F000
#define CLKPLL_REG_WIDE                            4
#define RF_CLK_PLL_RB_REG9_TESTMODE_REG            (RF_CLK_PLL_RB_BASE + CLKPLL_REG_WIDE * 9)  // reg9
#define RF_CLK_PLL_RB_REG17_LO_2_REG              (RF_CLK_PLL_RB_BASE + CLKPLL_REG_WIDE * 17)  // reg17
/* 函数声明 */
int32_t oal_pcie_ip_l1pm_check(oal_pcie_linux_res *pst_pci_lres);
int32_t oal_pcie_unmask_device_link_erros(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_mem_scanall(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_check_device_link_errors(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_read32_timeout(void *pst_va, uint32_t target,
    uint32_t mask, unsigned long timeout);

/* 函数定义 */
int32_t oal_pcie_get_gen_mode_mp16(oal_pcie_res *pst_pci_res)
{
    uint32_t value = (uint32_t)oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_STATUS2_OFF);
    value = ((value >> 20) & 0x3); /* bit[21:20] link signaling rate */
    if (value == 0) {
        return PCIE_GEN1;
    } else {
        return PCIE_GEN2;
    }
}

int32_t oal_pcie_gen_mode_check_mp16(oal_pcie_linux_res *pst_pci_lres)
{
    /* 根据实际产品来判断当前链路状态是否正常 */
    int32_t gen_select = oal_pcie_get_gen_mode_mp16(pst_pci_lres->pst_pci_res);

    if (g_ft_pcie_gen_check_bypass) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "pcie_gen_check_bypass");
    } else {
        if (g_hipci_gen_select != gen_select) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "expect link mode is %d, but current is %d",
                               g_hipci_gen_select, gen_select);
            return -OAL_EFAIL;
        }
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "current link is %s", (gen_select == PCIE_GEN2) ? "GEN2" : "GEN1");

    return OAL_SUCC;
}

int32_t oal_pcie_device_changeto_high_cpufreq_mp16(oal_pcie_res *pst_pci_res)
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

static int32_t oal_pcie_ip_factory_test_init_mp16(hcc_bus *pst_bus, oal_pcie_linux_res *pst_pci_lres)
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

    ret = oal_pcie_device_changeto_high_cpufreq_mp16(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }
    return OAL_SUCC;
}


int32_t oal_pcie_ip_factory_test_mp16(hcc_bus *pst_bus, int32_t test_count)
{
    int32_t i;
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    ret = oal_pcie_ip_factory_test_init_mp16(pst_bus, pst_pci_lres);
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

    oal_msleep(5); /* wait 5ms to enter pcie  L1.2 */

    ret = oal_pcie_ip_l1pm_check(pst_pci_lres);
    if (ret) {
        return ret;
    }

    return OAL_SUCC;
}

int32_t oal_pcie_chiptest_init_mp16(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pcie_ip_factory_test = oal_pcie_ip_factory_test_mp16;
    return OAL_SUCC;
}
#endif
