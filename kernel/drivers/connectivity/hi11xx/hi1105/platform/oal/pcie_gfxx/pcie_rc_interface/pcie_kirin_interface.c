/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#ifdef CONFIG_ARCH_KIRIN_PCIE
#include "oal_hardware.h"
#include "external/lpcpu_feature.h"
#include "oal_util.h"
#include "oam_ext_if.h"
#include "plat_pm.h"
#include "arch/oal_mem.h"
#include "plat_exception_rst.h"
#include "oal_pcie_interface.h"

OAL_STATIC void oal_pcie_linkdown_callback(struct kirin_pcie_notify *noti)
{
    oal_pci_dev_stru *pst_pci_dev = (oal_pci_dev_stru *)noti->user;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    uint32_t i;
    if (pst_pci_dev == NULL) {
        oal_io_print("pcie linkdown, pci dev is null!!\n");
        return;
    }
    for (i = 0; i < HCC_CHIP_MAX_DEV; i++) {
        pst_pci_lres = oal_get_pcie_handler(i);
        if (pst_pci_lres == NULL) {
            oal_io_print("pcie linkdown, pci res is null!!\n");
            continue;
        }
        if (pst_pci_lres->pst_bus == NULL) {
            oal_io_print("pcie linkdown, pst_bus is null\n");
            continue;
        }
        oal_io_print("pcie dev[%s] [%d:%d] devid %d linkdown\n", dev_name(&pst_pci_dev->dev),
                     pst_pci_dev->vendor, pst_pci_dev->device, i);
        declare_dft_trace_key_info("pcie_link_down", OAL_DFT_TRACE_EXCEP);

        oal_pcie_change_link_state(pst_pci_lres, PCI_WLAN_LINK_DOWN);

        if (((conn_get_gpio_level(W_WKUP_HOST) == 0) && (i == HCC_EP_WIFI_DEV)) ||
            ((conn_get_gpio_level(GT_WKUP_HOST) == 0) && (i == HCC_EP_GT_DEV))) {
            hcc_bus_exception_submit(pst_pci_lres->pst_bus, HCC_BUS_TRANS_FAIL);
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
            hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_WIFI_PCIE_LINKDOWN,
                "%s: pcie linkdown\n", __FUNCTION__);
#endif
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_WKUP_GPIO_PCIE_LINK_DOWN);
        } else {
            pci_print_log(PCI_LOG_INFO, "dev wakeup gpio is high, dev maybe panic");
        }
    }
}

void oal_pcie_deregister_event(oal_pcie_linux_res *pst_pci_lres)
{
#ifndef _PRE_HI375X_PCIE
    kirin_pcie_deregister_event((struct kirin_pcie_register_event *)pst_pci_lres->comm_res->pcie_event);
    if (pst_pci_lres->comm_res->pcie_event != NULL) {
        oal_free(pst_pci_lres->comm_res->pcie_event);
        pst_pci_lres->comm_res->pcie_event = NULL;
    }
#endif
    return;
}

void oal_pcie_register_event(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres)
{
#ifndef _PRE_HI375X_PCIE
    struct kirin_pcie_register_event *pcie_event = NULL;
    if (pst_pci_lres->comm_res->pcie_event == NULL) {
        pcie_event = (struct kirin_pcie_register_event *)oal_memalloc(sizeof(struct kirin_pcie_register_event));
        if (pcie_event == NULL) {
            pci_print_log(PCI_LOG_ERR, "pcie alloc event fail!\n");
            return;
        }
        pcie_event->events = KIRIN_PCIE_EVENT_LINKDOWN;
        pcie_event->user = pst_pci_dev;
        pcie_event->mode = KIRIN_PCIE_TRIGGER_CALLBACK;
        pcie_event->callback = oal_pcie_linkdown_callback;
        pst_pci_lres->comm_res->pcie_event = pcie_event;
    }
    kirin_pcie_register_event((struct kirin_pcie_register_event *)pst_pci_lres->comm_res->pcie_event);
#endif
    return;
}

int32_t oal_pcie_pm_control(oal_pcie_linux_res *pst_pci_lres, u32 rc_idx, int power_option)
{
    int32_t ret;
    if (power_option == PCIE_POWER_ON) {
        wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);
    }
    ret = kirin_pcie_pm_control(power_option, rc_idx);
    pci_print_log(PCI_LOG_INFO, "host_pcie_pm_control ret=%d,dev id %d\n", ret, pst_pci_lres->dev_id);
    if ((power_option == PCIE_POWER_OFF_WITH_L3MSG) || (power_option == PCIE_POWER_OFF_WITHOUT_L3MSG)) {
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);
    }
    if (power_option == PCIE_POWER_ON) {
        kirin_pcie_lp_ctrl(g_rc_idx, 0);
    }
    return ret;
}

int32_t oal_pcie_power_notifiy_register(u32 rc_idx, int (*poweron)(void *data),
                                        int (*poweroff)(void *data), void *data)
{
    int32_t ret = kirin_pcie_power_notifiy_register(rc_idx, poweron, poweroff, data);
    pci_print_log(PCI_LOG_INFO, "host_pcie_power_notifiy_register ret=%d\n", ret);
    return ret;
}

int32_t oal_pcie_host_aspm_init(oal_pcie_linux_res *pst_pci_lres)
{
    uint16_t val = 0;
    uint32_t reg;
    oal_pci_dev_stru *pst_rc_dev;
    if ((pst_pci_lres == NULL) || (pst_pci_lres->comm_res == NULL)) {
        return -OAL_EFAUL;
    }
    pst_rc_dev = pci_upstream_bridge(pst_pci_lres->comm_res->pcie_dev);

    /* 使能/去使能ASPM，RC & EP */
    kirin_pcie_lp_ctrl(g_rc_idx, 0);
    oal_pcie_device_phy_config(pst_pci_lres);
    if (g_pcie_aspm_enable) {
        /* L1SS config */
        if (pst_pci_lres->ep_res.pst_pci_dbi_base != NULL) {
            reg = oal_readl(pst_pci_lres->ep_res.pst_pci_dbi_base + PCIE_ACK_F_ASPM_CTRL_OFF);
#ifdef _PRE_COMMENT_CODE_
            reg &= ~((0x7 << 24) | (0x7 << 27)); /* 将bit[29:27]、bit[26：24]清0，L1 and L0s Entrance Latency disable */
#endif
            reg &= ~((0x7 << 27)); /* 将bit[29:27]清0，L1 Entrance Latency disable */
#ifdef _PRE_COMMENT_CODE_
            /* L0s 7us entry, L1 64us entery, tx & rx */
            reg |= (0x7 << 24) | (0x7 << 27); /* 将bit[29:27]、bit[26：24]置1，L1 and L0s Entrance Latency enable */
#endif
            reg |= (0x3 << 27); /* 将bit[29:27]置1，L1 Entrance Latency enable */
            oal_writel(reg, pst_pci_lres->ep_res.pst_pci_dbi_base + PCIE_ACK_F_ASPM_CTRL_OFF);
            pci_print_log(PCI_LOG_INFO, "ack aspm ctrl val:0x%x", reg);
#ifdef _PRE_COMMENT_CODE_
            reg = oal_readl(pst_pci_lres->cep_res.pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);
            reg &= ~(0x3ff);
            reg |= (0x2 << 0); /* aux_clk 2m,actual 1.92M  38.4M/20 */
            oal_writel(reg, pst_pci_lres->ep_res.pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);
            pci_print_log(PCI_LOG_INFO, "aux_clk_freq val:0x%x", reg);
#endif
        } else {
            pci_print_log(PCI_LOG_ERR, "pci res null or ctrl_base is null");
        }

        kirin_pcie_lp_ctrl(g_rc_idx, 1);
    }
    pci_print_log(PCI_LOG_INFO, "g_pcie_aspm_enable:%d", g_pcie_aspm_enable);

    /* rc noc protect */
    if (pst_rc_dev != NULL) {
        oal_pci_read_config_word(pst_rc_dev, oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_DEVCTL2, &val);
#ifdef _PRE_COMMENT_CODE_
        /* 调整rc pcie rsp 超时时间，对kirin noc有影响 */
        val |= (0xe << 0);
        oal_pci_write_config_word(pst_rc_dev, oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_DEVCTL2, val);
#endif
        pci_print_log(PCI_LOG_INFO, "devctrl:0x%x", val);
    }
    return OAL_SUCC;
}

int32_t oal_pcie_get_mps(oal_pci_dev_stru *pst_pcie_dev)
{
    int32_t ret;
    int32_t pos;
    uint16_t reg16 = 0;

    pos = pci_find_capability(pst_pcie_dev, PCI_CAP_ID_EXP);
    if (!pos) {
        return -1;
    }

    ret = oal_pci_read_config_word(pst_pcie_dev, pos + PCI_EXP_DEVCAP, &reg16);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "pci read devcap failed ret=%d", ret);
        return -1;
    }

    return (int32_t)(reg16 & PCI_EXP_DEVCAP_PAYLOAD);
}

/* Max Payload Size Supported,  must config beofre pcie access */
int32_t oal_pcie_mps_init(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t rc_mps, ep_mps;
    uint32_t mps, mrq;
    oal_pci_dev_stru *pst_rc_dev = NULL;
    /* 默认128, 开启后可协商256, 一次Burst多一倍对峰值性能有收益 */
    if (!g_pcie_performance_mode) {
        return OAL_SUCC;
    }

    pst_rc_dev = pci_upstream_bridge(pst_pci_lres->comm_res->pcie_dev);
    if (pst_rc_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "no upstream dev");
        return -OAL_ENODEV;
    }

    rc_mps = oal_pcie_get_mps(pst_rc_dev);
    ep_mps = oal_pcie_get_mps(pst_pci_lres->comm_res->pcie_dev);
    if (rc_mps < 0 || ep_mps < 0) {
        pci_print_log(PCI_LOG_ERR, "mps get failed, rc_mps:%d  , ep_mps:%d", rc_mps, ep_mps);
        return -OAL_EFAIL;
    }

    mps = (uint32_t)oal_min(rc_mps, ep_mps);
    mrq = mps;
    pci_print_log(PCI_LOG_INFO, "rc/ep max payload size, rc_mps:%d  , ep_mps:%d, select mps:%d bytes", rc_mps, ep_mps,
                  OAL_PCIE_MIN_MPS << mps);
    mps <<= 5;  /* 右移5位，PCI_EXP_DEVCTL_PAYLOAD = 0b1110 0000 */
    mrq <<= 12; /* 右移12位，PCI_EXP_DEVCTL_READRQ  = 0b0111 0000 0000 0000 */

    pcie_capability_clear_and_set_word(pst_pci_lres->comm_res->pcie_dev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_READRQ, mrq);
    pcie_capability_clear_and_set_word(pst_pci_lres->comm_res->pcie_dev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_PAYLOAD, mps);
    pcie_capability_clear_and_set_word(pst_rc_dev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_PAYLOAD, mps);
    pcie_capability_clear_and_set_word(pst_rc_dev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_READRQ, mrq);
    return OAL_SUCC;
}

int32_t oal_pcie_enumerate(int32_t rc_idx)
{
    int32_t ret;
    oal_io_print("notify host to scan rc:%d\n", rc_idx);
    ret = kirin_pcie_enumerate(rc_idx);
    if (ret != 0) {
        return ret;
    }
    kirin_pcie_lp_ctrl(g_rc_idx, 0);
    return ret;
}

#endif // CONFIG_ARCH_KIRIN_PCIE
#endif // _PRE_PLAT_FEATURE_GF6X_PCIE
