/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Host访问Device控制
 * 作    者 : wifi
 * 创建日期 : 2023年2月13日
 */

#include "host_hal_access_mgmt.h"
#include "host_hal_ext_if.h"
#include "plat_pm_wlan.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_ACCESS_MGMT_C

#define FORBIDE_SLEEP_RECORD_CNT 20
uint8_t g_forbid_sleep_record = 0;

// 在定时器中检查forbid_sleep的值，如果连续20次(2s)都是非0，输出error
void hal_pm_check_forbid_sleep_in_timer(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if (wlan_pm_info == NULL) {
        oam_error_log0(0, 0, "hal_pm_check_forbid_sleep_in_timer: wlan_pm is null \n");
        return;
    }
    if (oal_atomic_read(&wlan_pm_info->forbid_sleep) == 0) {
        g_forbid_sleep_record = 0;
        return;
    }
    g_forbid_sleep_record++;
    if (g_forbid_sleep_record >= FORBIDE_SLEEP_RECORD_CNT) {
        oam_error_log1(0, 0, "hal_pm_check_forbid_sleep_in_timer: forbid_sleep[%d] \n",
            oal_atomic_read(&wlan_pm_info->forbid_sleep));
        g_forbid_sleep_record = 0;
    }
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
typedef struct {
    uint8_t __rcu *access_dev_allowed;
} hal_host_access_device_mgmt_stru;

static DEFINE_PER_CPU(uint8_t, g_pcpu_access_dev_mem);
static DEFINE_PER_CPU(hal_host_access_device_mgmt_stru, g_pcpu_access_dev_mgmt) = { 0 };
oal_spin_lock_stru g_access_dev_lock;

/* need to be protected by rcu_read_lock! */
static inline uint8_t hal_host_pcpu_access_device_allowed(void)
{
    hal_host_access_device_mgmt_stru *pcpu_access_dev_mgmt = this_cpu_ptr(&g_pcpu_access_dev_mgmt);
    return *rcu_dereference(pcpu_access_dev_mgmt->access_dev_allowed);
}

static void hal_host_pcpu_update_access_device_status(uint8_t allowed)
{
    int32_t cpu;
    uint8_t *pcpu_access_dev_mem = NULL;

    oal_spin_lock_bh(&g_access_dev_lock);
    for_each_possible_cpu(cpu) {
        pcpu_access_dev_mem = per_cpu_ptr(&g_pcpu_access_dev_mem, cpu);
        *pcpu_access_dev_mem = allowed;
        rcu_assign_pointer(per_cpu_ptr(&g_pcpu_access_dev_mgmt, cpu)->access_dev_allowed, pcpu_access_dev_mem);
    }
    oal_spin_unlock_bh(&g_access_dev_lock);

    synchronize_rcu();
}

void hal_host_forbid_access_device(void)
{
    /* mask irq report */
    hal_host_mac_phy_irq_mask();
    hal_host_pcpu_update_access_device_status(OAL_FALSE);
    oam_warning_log0(0, 0, "{hal_host_forbid_access_device::access forbid}");
}

void hal_host_approve_access_device(void)
{
    hal_host_pcpu_update_access_device_status(OAL_TRUE);
    oam_warning_log0(0, 0, "{hal_host_approve_access_device::access approve}");
}

void hal_host_access_device_init(void)
{
    oal_spin_lock_init(&g_access_dev_lock);
    hal_host_approve_access_device();
}

uint32_t hal_host_readl(uintptr_t addr)
{
    uint32_t val = HAL_HOST_READL_INVALID_VAL; /* invalid val */

    rcu_read_lock();
    if (hal_host_pcpu_access_device_allowed()) {
        val = oal_readl(addr);
    } else {
        oam_warning_log0(0, 0, "{hal_host_readl::illegal readl}");
    }
    rcu_read_unlock();

    return val;
}

void hal_host_writel(uint32_t val, uintptr_t addr)
{
    rcu_read_lock();
    if (hal_host_pcpu_access_device_allowed()) {
        oal_writel(val, addr);
    } else {
        oam_warning_log0(0, 0, "{hal_host_writel::illegal writel}");
    }
    rcu_read_unlock();
}

uint32_t __hal_host_readl_irq(uintptr_t addr)
{
    uint32_t val = HAL_HOST_READL_INVALID_VAL; /* invalid val */

    rcu_read_lock();
    if (!hal_host_pcpu_access_device_allowed()) {
        oam_warning_log0(0, 0, "{hal_host_readl_irq::illegal readl}");
    }
    val = oal_readl(addr);
    rcu_read_unlock();

    return val;
}

uint32_t hal_host_readl_irq(uintptr_t addr)
{
    if (in_irq()) {
        return __hal_host_readl_irq(addr);
    } else {
        return hal_host_readl(addr);
    }
}

void __hal_host_writel_irq(uint32_t val, uintptr_t addr)
{
    rcu_read_lock();
    if (!hal_host_pcpu_access_device_allowed()) {
        oam_warning_log0(0, 0, "{hal_host_writel_irq::illegal writel}");
    }
    oal_writel(val, addr);
    rcu_read_unlock();
}

void hal_host_writel_irq(uint32_t val, uintptr_t addr)
{
    if (in_irq()) {
        __hal_host_writel_irq(val, addr);
    } else {
        hal_host_writel(val, addr);
    }
}

#else
void hal_host_forbid_access_device(void)
{
}

void hal_host_approve_access_device(void)
{
}

void hal_host_access_device_init(void)
{
}

uint32_t hal_host_readl(uintptr_t addr)
{
    return oal_readl(addr);
}

void hal_host_writel(uint32_t val, uintptr_t addr)
{
    oal_writel(val, addr);
}

uint32_t hal_host_readl_irq(uintptr_t addr)
{
    return oal_readl(addr);
}

void hal_host_writel_irq(uint32_t val, uintptr_t addr)
{
    oal_writel(val, addr);
}
#endif

uint32_t hal_pm_try_wakeup_dev_lock(void)
{
    if (wlan_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        return OAL_FAIL;
    }
    if (!oal_pcie_link_state_up(HCC_EP_WIFI_DEV)) {
        oam_error_log0(0, 0, "{hal_pm_try_wakeup_dev_lock::pcie is not work up");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

void hal_pm_try_wakeup_forbid_sleep(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    if (wlan_pm_info == NULL) {
        oam_error_log0(0, 0, "hal_pm_try_wakeup_forbid_sleep: wlan_pm is null \n");
        return;
    }
    oal_atomic_inc(&(wlan_pm_info->forbid_sleep));
}
void hal_pm_try_wakeup_allow_sleep(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if (wlan_pm_info == NULL) {
        oam_error_log0(0, 0, "hal_pm_try_wakeup_allow_sleep: wlan_pm is null \n");
        return;
    }
    // 判断为0时，不再减，做保护
    if (oal_atomic_read(&wlan_pm_info->forbid_sleep) == 0) {
        oam_error_log0(0, 0, "hal_pm_try_wakeup_allow_sleep: forbid_sleep is 0");
        return;
    }
    oal_atomic_dec(&(wlan_pm_info->forbid_sleep));
}

void hal_pm_try_wakeup_reset_forbid_sleep(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if (wlan_pm_info == NULL) {
        oam_error_log0(0, 0, "hal_pm_try_wakeup_reset_forbid_sleep: wlan_pm is null \n");
        return;
    }
    oal_atomic_set(&(wlan_pm_info->forbid_sleep), OAL_FALSE);
}
