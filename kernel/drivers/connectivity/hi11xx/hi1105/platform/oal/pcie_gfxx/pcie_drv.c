/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie driver
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#include "oal_hardware.h"
#include "oal_hcc_host_if.h"
#include "oal_util.h"
#include "oam_ext_if.h"
#include "plat_pm.h"
#include "plat_exception_rst.h"
#include "oal_pcie_interface.h"
#include "chip/gf61/pcie_chip_gf61.h"
#include "chip/comm/pcie_pm.h"

oal_completion g_probe_complete; /* 初始化信号量 */
OAL_VOLATILE int32_t g_probe_ret; /* probe 返回值 */
int32_t g_pcie_enum_fail_reg_dump_flag = 0;
int32_t g_hipci_sync_flush_cache_enable = 0; // for device
int32_t g_hipci_sync_inv_cache_enable = 0;   // for cpu
oal_debug_module_param(g_hipci_sync_flush_cache_enable, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hipci_sync_inv_cache_enable, int, S_IRUGO | S_IWUSR);
OAL_STATIC int32_t g_pcie_shutdown_panic = 0;
oal_debug_module_param(g_pcie_shutdown_panic, int, S_IRUGO | S_IWUSR);

OAL_STATIC const oal_pci_device_id_stru g_pcie_id_tab[] = {
    { PCIE_HISI_VENDOR_ID_0, PCIE_HISI_DEVICE_ID_GF61, PCI_ANY_ID, PCI_ANY_ID }, /* gf61 tmp PCIE */
    { 0, }
};

OAL_STATIC oal_pcie_linux_res *g_pcie_linux_res_tab[HCC_CHIP_MAX_DEV];

int32_t g_pcie_speed_change = 0;
oal_debug_module_param(g_pcie_speed_change, int, S_IRUGO | S_IWUSR);

static int32_t g_pcie_preset_config = 0; /* chip default 0, 0: -6db, 1: -3.5db */
oal_debug_module_param(g_pcie_preset_config, int, S_IRUGO | S_IWUSR);

static int32_t g_pcie_tx_swing_config = 0; /* chip default 0, 0: tx_swing_full, 1: tx_swing_slow */
oal_debug_module_param(g_pcie_tx_swing_config, int, S_IRUGO | S_IWUSR);

static void oal_pcie_preset_init(void)
{
    int32_t l_cfg_value = 0;
    int32_t l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_PLAT, "pcie_preset", &l_cfg_value);
    if (l_ret == INI_FAILED) {
        pci_print_log(PCI_LOG_INFO, "pcie_preset not set\n");
        return;
    }

    if ((g_pcie_preset_config < PCIE_PRESET_6DB) || (g_pcie_preset_config >= PCIE_PRESET_MAX)) {
        pci_print_log(PCI_LOG_ERR, "g_pcie_preset_config target:%d invalid, no set\n", g_pcie_preset_config);
        g_pcie_preset_config = 0;
        return;
    }

    g_pcie_preset_config = (uint16_t)l_cfg_value;
    pci_print_log(PCI_LOG_INFO, " g_pcie_preset_config target:%d[0:-6db 1:-3.5db]", g_pcie_preset_config);
    return;
}

int32_t oal_pcie_get_preset_val(void)
{
    return g_pcie_preset_config;
}

static void oal_pcie_tx_swing_init(void)
{
    int32_t l_cfg_value = 0;
    int32_t l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_PLAT, "pcie_tx_swing", &l_cfg_value);
    if (l_ret == INI_FAILED) {
        pci_print_log(PCI_LOG_INFO, "pcie_tx_swing not set\n");
        return;
    }

    if ((g_pcie_tx_swing_config < PCIE_TX_SWING_FULL) || (g_pcie_tx_swing_config >= PCIE_TX_SWING_MAX)) {
        pci_print_log(PCI_LOG_ERR, "g_pcie_tx_swing_config target:%d invalid, no set\n", g_pcie_tx_swing_config);
        g_pcie_tx_swing_config = 0;
        return;
    }

    g_pcie_tx_swing_config = (uint16_t)l_cfg_value;
    pci_print_log(PCI_LOG_INFO, " g_pcie_tx_swing_config target:%d[0:full 1:slow]", g_pcie_tx_swing_config);
    return;
}

int32_t oal_pcie_get_tx_swing_val(void)
{
    return g_pcie_tx_swing_config;
}

static void oal_pcie_speed_change_init(void)
{
    int32_t l_cfg_value = 0;
    int32_t l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_PLAT, "pcie_speed_change", &l_cfg_value);
    if (l_ret == INI_FAILED) {
        pci_print_log(PCI_LOG_INFO, "pcie_speed_change not set, no link speed change\n");
        return;
    }

    if ((g_pcie_speed_change < PCIE_GEN1) || (g_pcie_speed_change >= PCIE_GEN_MAX)) {
        pci_print_log(PCI_LOG_ERR, "pcie_speed_change target:%d invalid, no link speed change\n", g_pcie_speed_change);
        g_pcie_speed_change = 0;
        return;
    }

    g_pcie_speed_change = (uint16_t)l_cfg_value;
    pci_print_log(PCI_LOG_INFO, " pcie_speed_change target:%d[0:GEN1 1:GEN2 2:GEN3]", g_pcie_speed_change);
    return;
}

int32_t oal_pcie_get_pcie_speed_change_val(void)
{
    return g_pcie_speed_change;
}

static void oal_pcie_aspm_switch_init(void)
{
    int32_t value = 0;
    int32_t ret;

    ret = get_cust_conf_int32(INI_MODU_PLAT, "plat_pcie_aspm_enable", &value);
    if ((ret == INI_SUCC) && (value == 0)) {
        pci_print_log(PCI_LOG_INFO, "plat_pcie_aspm disable by ini");
        g_pcie_aspm_enable = 0;
    }
}

oal_pcie_linux_res *oal_pcie_get_linux_res_by_id(uint32_t id)
{
    if (id >= HCC_CHIP_MAX_DEV) {
        return NULL;
    }
    return g_pcie_linux_res_tab[id];
}

OAL_STATIC hcc_bus *oal_pcie_bus_init(oal_pcie_linux_res *pcie_res, uint32_t ep_id)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;

    pst_bus = hcc_alloc_bus();
    if (pst_bus == NULL) {
        oal_io_print("alloc pcie hcc bus failed, size:%u\n", (uint32_t)sizeof(hcc_bus));
        return NULL;
    }

    pst_bus->bus_type = HCC_BUS_PCIE;
    pst_bus->bus_id = ep_id - HCC_EP_WIFI_DEV;
    pst_bus->dev_id = ep_id;

    /* PCIE 只需要4字节对齐, burst大小对性能的影响有限 */
    pst_bus->cap.align_size[HCC_TX] = (uint32_t)sizeof(uint32_t);
    pst_bus->cap.align_size[HCC_RX] = (uint32_t)sizeof(uint32_t);
    pst_bus->cap.max_trans_size = 0x7fffffff;

    if (pcie_res->ep_res.chip_info.ete_support == OAL_TRUE) {
        oal_pcie_ete_bus_ops_init(pst_bus);
    } else {
        pst_bus->opt_ops = NULL;
        oal_print_mpxx_log(MPXX_LOG_ERR, "unsupport chip, bus init failed!\n");
        hcc_free_bus(pst_bus);
        return NULL;
    }

    pst_bus->data = (void *)pcie_res;

    ret = hcc_add_bus(pst_bus, "pcie");
    if (ret) {
        oal_io_print("add pcie bus failed, ret=%d\n", ret);
        hcc_free_bus(pst_bus);
        return NULL;
    }

    pcie_res->pst_bus = pst_bus;
    return pst_bus;
}

OAL_STATIC void oal_pcie_bus_exit(hcc_bus *pst_bus)
{
    hcc_remove_bus(pst_bus);
    hcc_free_bus(pst_bus);
}

irqreturn_t oal_pcie_intx_ete_isr(int irq, void *dev_id)
{
    uint32_t i;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    /*
     * 中断处理内容太多，目前无法下移，因为中断需要每次读空 ，而不在中断读的话，
     * 要先锁住中断，否则中断会狂报
     */
    for (i = 0; i < HCC_CHIP_MAX_DEV; i++) {
        pst_pci_lres = oal_pcie_get_linux_res_by_id(i);
        if ((pst_pci_lres == NULL) || (!pst_pci_lres->inited)) {
            continue;
        }
        if (oal_unlikely(oal_pcie_transfer_ete_done(pst_pci_lres) < 0)) {
            pci_print_log(PCI_LOG_ERR, "froce to disable pcie intx");
            declare_dft_trace_key_info("transfer_done_fail", OAL_DFT_TRACE_EXCEP);
            oal_disable_pcie_irq(pst_pci_lres);

            /* DFR trigger */
            oal_pcie_change_link_state(pst_pci_lres, PCI_WLAN_LINK_DOWN);
            hcc_bus_exception_submit(pst_pci_lres->pst_bus, HCC_BUS_TRANS_FAIL);

            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV, CHR_PLT_DRV_EVENT_PCIE,
                                 CHR_PLAT_DRV_ERROR_INTX_ISR_PCIE_LINK_DOWN);
            /* 关闭低功耗 */
        }
    }
    return IRQ_HANDLED;
}

OAL_STATIC int32_t oal_pcie_register_int(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pcie_res)
{
    int32_t ret = -OAL_EFAIL;

    if (atomic_read(&pcie_res->comm_res->refcnt) > 1) {
        return OAL_SUCC;
    }

    /* interrupt register */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_io_print("raw irq: %d\n", pst_pci_dev->irq);

    if (pcie_res->ep_res.chip_info.ete_support == OAL_TRUE) {
        ret = request_irq(pst_pci_dev->irq, oal_pcie_intx_ete_isr, IRQF_SHARED, "hisi_ete_intx", (void *)pcie_res);
    }

    if (ret < 0) {
        oal_io_print("request intx failed, ret=%d\n", ret);
        return ret;
    }

    pcie_res->comm_res->irq_stat = 0;

#endif
    oal_io_print("request pcie intx irq %d succ\n", pst_pci_dev->irq);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_dev_probe(oal_pci_dev_stru *pst_pci_dev)
{
    int32_t i;
    int32_t ret = OAL_SUCC;
    oal_pcie_linux_res *pcie_res = NULL;
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *bus_dev = NULL;

    for (i = 0; i < HCC_CHIP_MAX_DEV; i++) {
        bus_dev = hcc_get_bus_dev(i);
        if (bus_dev == NULL || (bus_dev->bus_cap & HCC_BUS_PCIE_CAP) == 0) {
            continue;
        }
        pcie_res = oal_pcie_res_init(pst_pci_dev, bus_dev->dev_id);
        if (pcie_res == NULL) {
            printk("pcie res init fail, dev id:%d\n", bus_dev->dev_id);
            return -OAL_EINTR;
        }

        if (pcie_res->inited) {
            printk("pcie dev inited, dev_id:%d\n", bus_dev->dev_id);
            continue;
        }

        /* soft resource init */
        ret = oal_pcie_host_init(pcie_res);
        if (ret != OAL_SUCC) {
            oal_io_print("pci: oal_pcie_host_init failed\n");
            goto failed_pci_host_init;
        }

        pst_bus = oal_pcie_bus_init(pcie_res, bus_dev->dev_id);
        if (pst_bus == NULL) {
            ret = -OAL_EBUSY;
            printk("pcie bus init fail\n");
            goto failed_pci_bus_init;
        }

        /* 硬件设备资源初始化, 5610+mp13 FPGA 没有上下电接口 */
        ret = oal_pcie_dev_init(pcie_res);
        if (ret != OAL_SUCC) {
            printk("pcie dev init fail\n");
            goto failed_pci_dev_init;
        }

        ret = oal_pcie_register_int(pst_pci_dev, pcie_res);
        if (ret != OAL_SUCC) {
            printk("pcie register int fail\n");
            goto failed_register_int;
        }
        pcie_res->inited = 1;
        g_pcie_linux_res_tab[i] = pcie_res;
        oal_pcie_register_event(pst_pci_dev, pcie_res);
        printk("pcie probe dev succ, id=%d\n", bus_dev->dev_id);
    }

    oal_complete(&g_probe_complete);
    g_probe_ret = ret;
    return OAL_SUCC;
failed_register_int:
    oal_pcie_dev_deinit(pcie_res);
failed_pci_dev_init:
    oal_pcie_bus_exit(pst_bus);
failed_pci_bus_init:
    oal_pcie_host_exit(pcie_res);
failed_pci_host_init:
    oal_pcie_res_deinit(pcie_res);
    g_probe_ret = ret;
    return ret;
}

/* 探测到一个PCIE设备, probe 函数可能会触发多次 */
OAL_STATIC int32_t oal_pcie_probe(oal_pci_dev_stru *pst_pci_dev, const oal_pci_device_id_stru *pst_id)
{
    int32_t ret;
    unsigned short device_id;

    if (oal_any_null_ptr2(pst_pci_dev, pst_id)) {
        /* never touch here */
        oal_io_print("oal_pcie_probe failed, pst_pci_dev:%p, pst_id:%p\n", pst_pci_dev, pst_id);
        oal_complete(&g_probe_complete);
        return -OAL_EIO;
    }
    device_id = oal_pci_get_dev_id(pst_pci_dev);
    /* 设备ID 和 产品ID */
    oal_print_mpxx_log(MPXX_LOG_INFO,
                       "[PCIe][%s]devfn:0x%x , vendor:0x%x , device:0x%x , "
                       "subsystem_vendor:0x%x , subsystem_device:0x%x , class:0x%x \n",
                       dev_name(&pst_pci_dev->dev), pst_pci_dev->devfn, pst_pci_dev->vendor, pst_pci_dev->device,
                       pst_pci_dev->subsystem_vendor, pst_pci_dev->subsystem_device, pst_pci_dev->class);

    ret = oal_pci_enable_device(pst_pci_dev);
    if (ret) {
        oal_io_print("pci: pci_enable_device error ret=%d\n", ret);
        oal_complete(&g_probe_complete);
        return ret;
    }

    return oal_pcie_dev_probe(pst_pci_dev);
}

OAL_STATIC void oal_pcie_remove(oal_pci_dev_stru *pst_pci_dev)
{
    unsigned short device_id;
    oal_pcie_linux_res *pst_pci_lres = NULL;

    device_id = oal_pci_get_dev_id(pst_pci_dev);

    oal_io_print("pcie driver remove 0x%x, name:%s\n", device_id, dev_name(&pst_pci_dev->dev));

    pst_pci_lres = (oal_pcie_linux_res *)oal_get_default_pcie_handler();
    if (pst_pci_lres == NULL) {
        oal_io_print("oal_pcie_remove lres is null\n");
        return;
    }
    memset_s(g_pcie_linux_res_tab, sizeof(g_pcie_linux_res_tab), 0, sizeof(g_pcie_linux_res_tab));
    oal_pcie_deregister_event(pst_pci_lres);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    free_irq(pst_pci_dev->irq, pst_pci_lres);
#endif

    mutex_destroy(&pst_pci_lres->comm_res->sr_lock);

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    oal_pcie_dual_pci_exit(pst_pci_lres);
#endif
    oal_pcie_dev_deinit(pst_pci_lres);
    oal_pcie_bus_exit(pst_pci_lres->pst_bus);
    oal_pcie_host_exit(pst_pci_lres);

    pst_pci_lres->comm_res->pcie_dev = NULL;
    oal_free(pst_pci_lres);

    oal_pci_disable_device(pst_pci_dev);
}

OAL_STATIC int32_t oal_pcie_suspend(oal_pci_dev_stru *pst_pci_dev, oal_pm_message_t state)
{
    int32_t ret = OAL_SUCC;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_get_default_pcie_handler();
    if (pst_pci_lres == NULL) {
        oal_io_print("pcie_suspend lres is null,pci_dev_name=%s\n", dev_name(&pst_pci_dev->dev));
        return 0;
    }

    if (pst_pci_lres->ep_res.chip_info.ete_support == OAL_TRUE) {
        ret = oal_pcie_ete_suspend(pst_pci_dev, state);
    } else {
        ret = -OAL_ENODEV;
    }
    return ret;
}

OAL_STATIC int32_t oal_pcie_resume(oal_pci_dev_stru *pst_pci_dev)
{
    int32_t ret = OAL_SUCC;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_get_default_pcie_handler();
    if (pst_pci_lres == NULL) {
        oal_io_print("pcie_suspend lres is null,pci_dev_name=%s\n", dev_name(&pst_pci_dev->dev));
        return 0;
    }

    if (pst_pci_lres->ep_res.chip_info.ete_support == OAL_TRUE) {
        ret = oal_pcie_ete_resume(pst_pci_dev);
    } else {
        ret = -OAL_ENODEV;
    }

    return ret;
}

OAL_STATIC void oal_pcie_shutdown(oal_pci_dev_stru *pst_pci_dev)
{
    declare_time_cost_stru(cost);

    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_get_default_pcie_handler();
    g_chip_err_block = 0;
    hcc_bus_set_shutdown_flag(OAL_TRUE);
    if (pst_pci_lres == NULL) {
        return;
    }

    ssi_excetpion_dump_disable();
    /* system shutdown, should't write sdt file */
    oam_set_output_type_all_sys(OAM_OUTPUT_TYPE_CONSOLE);

    if (pst_pci_lres->pst_bus == NULL) {
        pci_print_log(PCI_LOG_INFO, "pcie bus is null");
        return;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    time_cost_var_start(cost) = 0;
#else
    time_cost_var_start(cost) = (ktime_t) { .tv64 = 0 };
#endif

    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_pci_lres->pst_bus), OAL_FALSE);
    oal_disable_pcie_irq(pst_pci_lres);

    /* power off wifi */
    if (g_pcie_shutdown_panic) {
        oal_get_time_cost_start(cost);
    }

    if (in_interrupt() || irqs_disabled() || in_atomic()) {
        pci_print_log(PCI_LOG_INFO, "no call wlan_pm_close_by_shutdown interrupt");
    } else {
        wlan_pm_close_by_shutdown();
    }

    if (g_pcie_shutdown_panic) {
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_mpxx_log(MPXX_LOG_INFO, "pcie shutdown cost %llu us", time_cost_var_sub(cost));
#ifdef PLATFORM_DEBUG_ENABLE
        /* debug shutdown process when after wifi shutdown */
        if (time_cost_var_sub(cost) > (uint64_t)(uint32_t)g_pcie_shutdown_panic) {
            oal_print_mpxx_log(MPXX_LOG_INFO, "pcie shutdown panic");
            BUG_ON(1);
        }
#endif
    }
}

OAL_STATIC void oal_pcie_disable_device(oal_pcie_linux_res *pst_pci_lres)
{
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    if (oal_warn_on(pst_pci_lres->comm_res->pcie_dev == NULL)) {
        return;
    }

    oal_pci_disable_device(pst_pci_lres->comm_res->pcie_dev);
}

OAL_STATIC oal_pci_driver_stru g_pcie_drv = {
    .name = "hi110x_pci",
    .id_table = g_pcie_id_tab,
    .probe = oal_pcie_probe,
    .remove = oal_pcie_remove,
    .suspend = oal_pcie_suspend,
    .resume = oal_pcie_resume,
    .shutdown = oal_pcie_shutdown,
};

int32_t wlan_first_power_off_callback(void *data)
{
    oal_reference(data);
    hi_wlan_power_set(0);
    return 0;
}

OAL_STATIC void oal_pci_disable_hcc(oal_pcie_linux_res *pst_wl_pci_lres, oal_pcie_linux_res *pst_gt_pci_lres)
{
    if (pst_wl_pci_lres != NULL) {
        hcc_bus_disable_state(pst_wl_pci_lres->pst_bus, OAL_BUS_STATE_ALL);
    }
    if (pst_gt_pci_lres != NULL) {
        hcc_bus_disable_state(pst_gt_pci_lres->pst_bus, OAL_BUS_STATE_ALL);
    }
}

OAL_STATIC void oal_pci_disable_common_irq(oal_pcie_linux_res *pst_pci_lres)
{
#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
    oal_disable_pcie_irq_with_free(pst_pci_lres);
#else
    oal_disable_pcie_irq(pst_pci_lres);
#endif
}

OAL_STATIC OAL_INLINE void hi_gt_power_set(int32_t on)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (on) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "hcc probe:pull up power on gpio");
        board_host_gt_wakeup_dev_set(0);
        board_power_on(GT_SYS);
    } else {
        oal_print_mpxx_log(MPXX_LOG_INFO, "hcc probe:pull down power on gpio");
        board_power_off(GT_SYS);
        board_host_gt_wakeup_dev_set(0);
    }
#endif
}

OAL_STATIC void oal_pci_power_down_hcc(oal_pcie_linux_res *pst_wl_pci_lres, oal_pcie_linux_res *pst_gt_pci_lres)
{
    if (pst_wl_pci_lres != NULL) {
        hcc_bus_power_action(pst_wl_pci_lres->pst_bus, HCC_BUS_POWER_DOWN);
        hi_wlan_power_set(0);
    }
    if (pst_gt_pci_lres != NULL) {
        hcc_bus_power_action(pst_gt_pci_lres->pst_bus, HCC_BUS_POWER_DOWN);
        hi_gt_power_set(0);
    }
}

OAL_STATIC void oal_pcie_save_resource_fail_handle(oal_pcie_linux_res *pst_pci_lres)
{
    oal_pcie_disable_device(pst_pci_lres);
    oal_pci_disable_common_irq(pst_pci_lres);
    oal_pci_unregister_driver(&g_pcie_drv);
    hi_wlan_power_set(0);
    hi_gt_power_set(0);
    oal_pcie_power_notifiy_register(g_rc_idx, NULL, NULL, NULL);
    oal_atomic_set(&pst_pci_lres->comm_res->refcnt, 0);
    oal_io_print("pcie save resource fail\n");
}

OAL_STATIC int32_t oal_pci_power_down(void)
{
    int32_t ret;
    oal_pcie_linux_res *pst_wl_pci_lres = oal_get_pcie_handler(HCC_EP_WIFI_DEV);
    oal_pcie_linux_res *pst_gt_pci_lres = oal_get_pcie_handler(HCC_EP_GT_DEV);
    oal_pcie_linux_res *pst_pci_lres = (pst_gt_pci_lres == NULL) ? pst_wl_pci_lres : pst_gt_pci_lres;

    if ((pst_gt_pci_lres == NULL) && (pst_wl_pci_lres == NULL)) {
        oal_io_print("oal_get_pcie_handler is null\n");
        oal_pcie_power_notifiy_register(g_rc_idx, NULL, NULL, NULL);
        return -OAL_ENODEV;
    }
    oal_pci_disable_hcc(pst_wl_pci_lres, pst_gt_pci_lres);

    /* 保存PCIE 配置寄存器 */
    ret = oal_pcie_save_default_resource(pst_pci_lres);
    if (ret != OAL_SUCC) {
        oal_pcie_save_resource_fail_handle(pst_pci_lres);
        return ret;
    }
    oal_pcie_power_notifiy_register(g_rc_idx, NULL, wlan_first_power_off_callback, NULL);
    oal_pci_disable_common_irq(pst_pci_lres);
    oal_pci_power_down_hcc(pst_wl_pci_lres, pst_gt_pci_lres);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_dts_init(void)
{
    int ret;
    u32 pcie_rx_idx = 0;
    struct device_node *np = NULL;
    np = of_find_compatible_node(NULL, NULL, DTS_NODE_MPXX_WIFI);
    if (np == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "can't find node [%s]", DTS_NODE_MPXX_WIFI);
        return -OAL_ENODEV;
    }

    ret = of_property_read_u32(np, DTS_PROP_MPXX_PCIE_RC_IDX, &pcie_rx_idx);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "read prop [%s] fail, ret=%d", DTS_PROP_MPXX_PCIE_RC_IDX, ret);
        return ret;
    }
    g_rc_idx = (int32_t)pcie_rx_idx;
    oal_print_mpxx_log(MPXX_LOG_INFO, "%s=%d", DTS_PROP_MPXX_PCIE_RC_IDX, pcie_rx_idx);
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_mpxx_enum_fail_proc(void)
{
    if (!g_pcie_enum_fail_reg_dump_flag) {
        (void)ssi_dump_err_regs(SSI_ERR_PCIE_ENUM_FAIL);
    }
}

OAL_STATIC int32_t oal_wait_pcie_probe_complete(void)
{
    int32_t ret = OAL_SUCC;
    unsigned int timeout = hi110x_get_emu_timeout(10 * HZ); // 10 sec
    if (oal_wait_for_completion_timeout(&g_probe_complete, (uint32_t)timeout)) {
        if (g_probe_ret != OAL_SUCC) {
            oal_io_print("pcie driver probe failed ret=%d, driname:%s\n", g_probe_ret, g_pcie_drv.name);
#ifdef CONFIG_HWCONNECTIVITY_PC
            ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI |
                                 SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV, CHR_PLT_DRV_EVENT_INIT,
                                 CHR_PLAT_DRV_ERROR_PCIE_PROBE_FAIL);
#endif
            oal_pcie_power_notifiy_register(g_rc_idx, NULL, NULL, NULL);
            return g_probe_ret;
        }
    } else {
        oal_io_print("pcie driver probe timeout  driname:%s\n", g_pcie_drv.name);
        oal_pcie_mpxx_enum_fail_proc();
#ifdef CONFIG_HWCONNECTIVITY_PC
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV, CHR_PLT_DRV_EVENT_INIT,
                             CHR_PLAT_DRV_ERROR_PCIE_PROBE_TIMEOUT);
#endif
        oal_pcie_power_notifiy_register(g_rc_idx, NULL, NULL, NULL);
        return -OAL_ETIMEDOUT;
    }
    return ret;
}

int32_t wlan_first_power_on_callback(void *data)
{
    oal_reference(data);
    /* before wlan chip power up, rc clock must on. */
    /* bootloader had delay 20us before reset pcie, soc requet 10us delay, need't delay here */
    hi_wlan_power_set(1);
    return 0;
}

OAL_STATIC int32_t wlan_first_power_off_fail_callback(void *data)
{
    oal_reference(data);
    oal_io_print("wlan_first_power_off_fail_callback\n");
    (void)ssi_dump_err_regs(SSI_ERR_PCIE_FST_POWER_OFF_FAIL);

    return -1;
}

OAL_STATIC int32_t wlan_try_power_off_fail_callback(void *data)
{
    oal_reference(data);
    g_pcie_enum_fail_reg_dump_flag = 1;
    /* 阻止麒麟枚举失败后下电关时钟 */
    oal_chip_error_task_block();
    return -1;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
int32_t oal_pcie_dual_power_on_callback(void *data)
{
    oal_reference(data);
    oal_io_print("board_set_pcie_reset\n");
    board_set_pcie_reset(OAL_FALSE, OAL_TRUE);
    mdelay(10); /* delay 10ms */
    return 0;
}

OAL_STATIC int32_t oal_pcie_dual_power_off_fail_callback(void *data)
{
    oal_reference(data);
    /* 阻止麒麟枚举失败后下电关时钟 */
    oal_io_print("oal_pcie_dual_power_off_fail_callback\n");
    g_pcie_enum_fail_reg_dump_flag = 1;
    (void)ssi_dump_err_regs(SSI_ERR_PCIE_FST_POWER_OFF_FAIL);
    oal_chip_error_task_block();
    return -1;
}

OAL_STATIC int32_t wlan_dual_first_power_off_callback(void *data)
{
    oal_reference(data);
    board_set_pcie_reset(OAL_FALSE, OAL_FALSE);
    return 0;
}
#endif

static void oal_pcie_ini_parameters_init(void)
{
    oal_pcie_preset_init();
    oal_pcie_tx_swing_init();
    oal_pcie_speed_change_init();
    oal_pcie_aspm_switch_init();
    oal_pcie_pcs_ini_config_init();
}

static void oal_pcie_mpxx_pre_init(void)
{
    /* init host pcie */
    oal_io_print("%s PCIe driver register start\n", g_pcie_drv.name); /* Debug */
    oal_init_completion(&g_probe_complete);

    g_probe_ret = -OAL_EFAUL;

    g_pcie_enum_fail_reg_dump_flag = 0;

    oal_pcie_dts_init();
    oal_pcie_ini_parameters_init();
}

OAL_STATIC int32_t oal_pcie_mpxx_enum(void)
{
    int32_t ret;

    /* 打开参考时钟 */
    ret = oal_pcie_power_notifiy_register(g_rc_idx, wlan_first_power_on_callback,
                                          wlan_first_power_off_fail_callback, NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("enumerate first power register failed ret=%d\n", ret);
        return ret;
    }

    ret = oal_pcie_enumerate(g_rc_idx);
    if (ret == OAL_SUCC) {
        oal_io_print("pcie enumerate succ\n");
        return ret;
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_PCIE_ENUM_FAIL);
    hi_wlan_power_set(0);
    msleep(1000); // 此时硬件状态可能有异常，预留1000ms 给硬件电源恢复再尝试
    // 尝试第二次枚举
    ret = oal_pcie_power_notifiy_register(g_rc_idx, wlan_first_power_on_callback,
                                          wlan_try_power_off_fail_callback, NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("enumerate try power register failed ret=%d\n", ret);
        return ret;
    }

    ret = oal_pcie_enumerate(g_rc_idx);
    if (ret == OAL_SUCC) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_PCIE_ENUM_TRY_SUCC);
        oal_io_print("pcie try enum succ\n");
        return ret;
    }

    oal_pcie_mpxx_enum_fail_proc();
    oal_io_print("enumerate try failed ret=%d\n", ret);
    oal_pcie_power_notifiy_register(g_rc_idx, NULL, NULL, NULL);
    return ret;
}

/* trigger pcie probe, alloc pcie resource */
int32_t oal_pcie_mpxx_init(void)
{
    int32_t ret;

    oal_pcie_mpxx_pre_init();
    ret = oal_pci_register_driver(&g_pcie_drv);
    if (ret) {
        oal_io_print("pcie driver register failed ret=%d, driname:%s\n", ret, g_pcie_drv.name);
        return ret;
    }

    ret = oal_pcie_mpxx_enum();
    if (ret != OAL_SUCC) {
        oal_pci_unregister_driver(&g_pcie_drv);
        return ret;
    }
    ret = oal_wait_pcie_probe_complete();
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_HOST_PCIE_ALWAYS_ON
    oal_reference(oal_pci_power_down);
#else
    ret = oal_pci_power_down();
#endif
    oal_io_print("%s PCIe driver register %s\n", g_pcie_drv.name, (ret != OAL_SUCC) ? "fail" : "succ");
    return ret;
}

void oal_pcie_mpxx_exit(void)
{
    oal_pcie_power_notifiy_register(g_rc_idx, NULL, NULL, NULL);
    oal_pci_unregister_driver(&g_pcie_drv);
}
#endif
