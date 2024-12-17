/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : UAPSD hmac层处理
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#include "mac_vap.h"
#include "mac_frame.h"
#include "hmac_low_latency.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_rx_data.h"
#include "hmac_uapsd.h"
#include "hmac_config.h"
#include "securec.h"
#include "external/oal_pm_qos_interface.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/pm_qos.h>
#endif
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_LOW_LATENCY_C

#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
/*
 * This mutex is used to serialize latency switching actions between two
 * subsystems: the driver PM notifications and the linux PMQoS.
 */
static DEFINE_MUTEX(pmqos_network_latency_lock);

/* The flags are used to track state in case of concurrent execution */
static bool g_hmac_low_latency_request_on;
static bool g_hmac_low_latency_wifi_on;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && defined(CONFIG_PLATFORM_QOS)
struct platform_qos_request g_pmqos_cpu_dma_latency_request;
#else
static struct pm_qos_request g_pmqos_cpu_dma_latency_request;
#endif


/*
 * This function works like `hmac_userctl_bindcpu_get_cmd` but it is called for
 * low-latency binding reasons
 */
static void hmac_low_latency_bindcpu(uint32_t cmd)
{
    uint8_t irq_cmd;
    uint8_t thread_cmd;

    /* IRQ binding is encoded in upper bits, thread binding in lower */
    irq_cmd = (uint8_t)((uint16_t)cmd >> 8); /* 右移8位获取高位硬中断命令 */
    thread_cmd = (uint8_t)((uint16_t)cmd & 0xff);
    /* IRQ affinity must be set to a single CPU core */
    if (irq_cmd_is_onecpu(irq_cmd)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_low_latency_bindcpu::"
                         "irq_cmd[0x%x] is not single core}",
                         irq_cmd);
        return;
    }

    if (!irq_cmd && !thread_cmd) {
        /*
         * This branch is used to return back to auto-adjusting based on PPS
         * packet load and throughput estimations
         */
        g_freq_lock_control.uc_low_latency_irqbind    = 0;
        g_freq_lock_control.uc_low_latency_threadbind = 0;
        g_freq_lock_control.en_low_latency_bindcpu = OAL_FALSE;
    } else {
        /* This branch is used to set low-latency binding */
        g_freq_lock_control.uc_low_latency_irqbind    = irq_cmd;
        g_freq_lock_control.uc_low_latency_threadbind = thread_cmd;
        g_freq_lock_control.en_low_latency_bindcpu    = OAL_TRUE;
    }
}

void hmac_low_latency_bindcpu_fast(void)
{
    uint8_t irq_cmd;
    uint8_t thread_cmd;
    uint32_t command;

    /* set IRQ affinity to special CPU core */
    irq_cmd = 1 << WLAN_IRQ_AFFINITY_BUSY_CPU;

    /*
     * Note: bind driver threads to middle CPU cores (4-5) to achieve lower
     * latency with slightly increased power consumption but check the
     * possibility of scenarios which require both low latency and high
     * throughput. In these scenarios we should consider binding to fast cores
     * here.
     * latency with slightly increased power consumption.
     */
    thread_cmd = 0x30; /* 0x30表示cpu中核4-5 */

    /* construct bit command */
    command = (irq_cmd << 8) | thread_cmd; /* 右移8位获取高位硬中断命令 */
    hmac_low_latency_bindcpu(command);
}

void hmac_low_latency_bindcpu_default(void)
{
    hmac_low_latency_bindcpu(0);
}

static void hmac_pmqos_network_latency_enable(void)
{
    oam_warning_log0(0, OAM_SF_ANY, "[PMQOS]: enable low-latency");
    hmac_low_latency_bindcpu_fast();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#if defined(CONFIG_PLATFORM_QOS) && defined(CONFIG_DEVFREQ_GOV_PLATFORM_QOS)
    oal_pm_qos_add_request(&g_pmqos_cpu_dma_latency_request, PLATFORM_QOS_MEMORY_LATENCY, DDR_FREQ_INIT_VALUE);
#else
    oal_pm_qos_add_request(&g_pmqos_cpu_dma_latency_request, 0, DDR_FREQ_INIT_VALUE);
#endif
#else
    oal_pm_qos_add_request(&g_pmqos_cpu_dma_latency_request, PM_QOS_CPU_DMA_LATENCY, DDR_FREQ_INIT_VALUE);
#endif
    g_freq_lock_control.hcc_aspm_close_bitmap |= BIT(ASPM_CALL_PMQOS_NETWORK_LATENCY_ABLE);
    mpxx_hcc_ip_pm_ctrl(0, HCC_EP_WIFI_DEV);
    hmac_low_latency_freq_high();
}

static void hmac_pmqos_network_latency_disable(void)
{
    oam_warning_log0(0, OAM_SF_ANY, "[PMQOS]: disable low-latency");
    hmac_low_latency_freq_default();
    g_freq_lock_control.hcc_aspm_close_bitmap &= ~BIT(ASPM_CALL_PMQOS_NETWORK_LATENCY_ABLE);
    mpxx_hcc_ip_pm_ctrl(1, HCC_EP_WIFI_DEV);
    oal_pm_qos_remove_request(&g_pmqos_cpu_dma_latency_request);
    hmac_low_latency_bindcpu_default();
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_NETWORK_LATENCY_PLATFORM_QOS)
static int hmac_pmqos_network_latency_call(struct notifier_block *nb, unsigned long value, void *v)
{
    oam_warning_log1(0, OAM_SF_ANY, "[PMQOS]: low-latency call %lu", value);

    mutex_lock(&pmqos_network_latency_lock);
    /* perform latency switch only when wifi is active */
    if (g_hmac_low_latency_wifi_on) {
        if (!value) {
            hmac_pmqos_network_latency_enable();
        } else {
            hmac_pmqos_network_latency_disable();
        }
    }
    g_hmac_low_latency_request_on = !value;
    mutex_unlock(&pmqos_network_latency_lock);
    return NOTIFY_OK;
}
static struct notifier_block g_hmac_pmqos_network_latency_notifier = {
    .notifier_call = hmac_pmqos_network_latency_call,
};
#endif
void hmac_register_pmqos_network_latency_handler(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    pm_qos_add_notifier(PM_QOS_NETWORK_LATENCY, &g_hmac_pmqos_network_latency_notifier);
#elif defined(CONFIG_NETWORK_LATENCY_PLATFORM_QOS)
    platform_qos_add_notifier(PLATFORM_QOS_NETWORK_LATENCY, &g_hmac_pmqos_network_latency_notifier);
#endif
}

void hmac_unregister_pmqos_network_latency_handler(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    pm_qos_remove_notifier(PM_QOS_NETWORK_LATENCY, &g_hmac_pmqos_network_latency_notifier);
#elif defined(CONFIG_NETWORK_LATENCY_PLATFORM_QOS)
    platform_qos_remove_notifier(PLATFORM_QOS_NETWORK_LATENCY, &g_hmac_pmqos_network_latency_notifier);
#endif
}
static void hmac_low_latency_wifi_signal(bool wifi_on)
{
    oam_warning_log1(0, OAM_SF_ANY, "[PMQOS]: wifi is switched to %d", wifi_on);
    mutex_lock(&pmqos_network_latency_lock);
    /* perform latency switching only when request pending */
    if (g_hmac_low_latency_request_on) {
        if (wifi_on) {
            hmac_pmqos_network_latency_enable();
        } else {
            hmac_pmqos_network_latency_disable();
        }
    }
    g_hmac_low_latency_wifi_on = wifi_on;
    mutex_unlock(&pmqos_network_latency_lock);
}

void hmac_low_latency_wifi_enable(void)
{
    hmac_low_latency_wifi_signal(OAL_TRUE);
}

void hmac_low_latency_wifi_disable(void)
{
    hmac_low_latency_wifi_signal(OAL_FALSE);
}
#endif
