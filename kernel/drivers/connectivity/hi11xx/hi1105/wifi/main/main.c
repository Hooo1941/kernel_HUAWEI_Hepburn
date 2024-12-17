/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : main.c 文件
 * 作    者 : wifi
 * 创建日期 : 2014年10月10日
 */

/* 1 头文件包含 */
#define HISI_LOG_TAG "[WIFI_MAIN]"
#include "main.h"
#include "oal_kernel_file.h"
#include "oal_hcc_host_if.h"

#include "oneimage.h"
#include "wlan_chip_i.h"

#include "hmac_vap.h"
#include "hmac_cali_mgmt.h"
#include "wal_ext_if.h"
#include "oam_log_wifi.h"
#include "host_hal_ext_if.h"

#include "plat_pm_wlan.h"
#include "hisi_customize_wifi.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_C

void platform_module_exit(uint16_t us_bitmap);
OAL_STATIC void builder_module_exit(uint16_t us_bitmap);
#ifdef _PRE_WINDOWS_SUPPORT
oal_wait_queue_head_stru g_wlan_cali_complete_wq;
#endif

/* 3 函数实现 */
/*
 * 函 数 名  : host_mode_exit
 * 功能描述  : 选择性卸载不同模块
 * 1.日    期  : 2014年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void builder_module_exit(uint16_t us_bitmap)
{
    if (us_bitmap & BIT8) {
        wal_main_exit();
    }
    if (us_bitmap & BIT7) {
        hmac_main_exit();
    }

    return;
}

#if (defined(HI1102_EDA))
/*
 * 函 数 名  : device_test_create_cfg_vap
 * 功能描述  : device侧eda用例打桩使用
 * 1.日    期  : 2014年11月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t device_test_create_cfg_vap(void)
{
    uint32_t ret;
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;

    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oal_io_print("device_test_create_cfg_vap: hmac_init_event_process frw_event_alloc_m result=NULL.\n");
        return OAL_FAIL;
    }

    ret = dmac_init_event_process(pst_event_mem);
    if (ret != OAL_SUCC) {
        oal_io_print("device_test_create_cfg_vap: dmac_init_event_process result = fale.\n");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event->st_event_hdr.uc_device_id = 0;

    ret = dmac_cfg_vap_init_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}
#endif

#define MAIN_WAL_WAIT_CALI_COMPLETE_TIME  15000

/*
 * 函 数 名  : host_rf_cali_init
 * 功能描述  : 开机校准流程
 */
void host_rf_cali_init(void)
{
    oal_bool_enum_uint8 cali_first_pwr_on = OAL_TRUE;
    oal_init_completion(hmac_get_wlan_cali_completed_addr());

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_custom_cali_done = OAL_FALSE;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    cali_first_pwr_on = wlan_chip_first_power_on_mark();
#endif
    /* 开机校准和定制化参数下发 */
    if (cali_first_pwr_on == OAL_TRUE) {
        wlan_pm_open();
#ifndef _PRE_LINUX_TEST
        /*  PCIE总线工作模式:开机校准完成后，立即置位开机上电校准完成信号量  */
        if (hcc_is_pcie() == OAL_TRUE) {
            wlan_chip_first_powon_cali_completed();
        }
#endif
        /* 等待开机校准数据上传完成 */
        if (oal_wait_for_completion_timeout(hmac_get_wlan_cali_completed_addr(),
                                            (uint32_t)oal_msecs_to_jiffies(MAIN_WAL_WAIT_CALI_COMPLETE_TIME)) == 0) {
            oam_error_log1(0, OAM_SF_ANY, "{host_module_init::wait [%d]ms g_wlan_cali_completed timeout)!}",
                MAIN_WAL_WAIT_CALI_COMPLETE_TIME);
        }
        wlan_pm_close();
    }
    hmac_set_wlan_first_powon_mark(OAL_FALSE);
    oal_wait_queue_wake_up(&g_wlan_cali_complete_wq);
#endif
}
/*
 * 函 数 名  : host_main_init
 * 1.日    期  : 2014年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t host_module_init(void)
{
    int32_t l_return;
    uint16_t us_bitmap = 0;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_chip_custom_host_cali_data_init();
    register_wifi_customize(wlan_chip_hcc_customize_h2d_data_cfg);

    wlan_chip_custom_default_init();
    /* 读定制化配置文件&NVRAM */
    wlan_chip_custom_host_read_cfg_init();
    /* 配置host全局变量值 */
    wlan_chip_host_global_init_param();
#endif  // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

    l_return = hal_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_module_init: hal_main_init return error code:%d\r\n", l_return);
        return l_return;
    }

    l_return = hmac_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_module_init: hmac_main_init return error code:%d\r\n", l_return);
        return l_return;
    }

    l_return = wal_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_module_init:wal_main_init return error code:%d\r\n", l_return);
        us_bitmap = BIT7;
        builder_module_exit(us_bitmap);
        return l_return;
    }
    host_rf_cali_init();
    /* 启动完成后，输出打印 */
    oal_io_print("host_module_init:: host_main_init finish!\r\n");

    return OAL_SUCC;
}

/*
 * 函 数 名  : mp12_host_main_init
 * 功能描述  : mp12项目host侧初始化总入口
 * 1.日    期  : 2014年10月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t mpxx_host_main_init(void)
{
    int32_t l_return;

    hcc_flowctl_get_device_mode_register(hmac_flowctl_check_device_is_sta_mode, HCC_EP_WIFI_DEV);
    hcc_flowctl_operate_subq_register(hmac_vap_net_start_subqueue, hmac_vap_net_stop_subqueue,
                                      HCC_EP_WIFI_DEV);
    l_return = oam_wifi_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_main_init: oam_wifi_init fail\n");
        return l_return;
    }
    /* wifi相关编译选项初始化设置 */
    if (wlan_chip_feature_init() != OAL_SUCC) {
        oal_io_print("host_main_init: wlan_chip_feature_init return fail\n");
        return -OAL_EFAIL;
    }
    if (wlan_hal_ops_init() != OAL_SUCC) {
        oal_io_print("host_main_init: wlan_hal_ops_init return fail\r\n");
        return -OAL_EFAIL;
    }
    if (wlan_chip_ops_init() != OAL_SUCC) {
        oal_io_print("host_main_init: wlan_chip_ops_init return fail\r\n");
        return -OAL_EFAIL;
    }
    l_return = host_module_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_main_init: host_module_init return error code: %d\n", l_return);
        return l_return;
    }

    wal_hipriv_register_inetaddr_notifier();
    wal_hipriv_register_inet6addr_notifier();

    /* 启动完成后，输出打印 */
    oal_io_print("host_main_init:: host_main_init finish!\n");

    return OAL_SUCC;
}

/*
 * 函 数 名  : mp12_host_main_exit
 * 功能描述  : mp12项目host侧卸载总入口
 * 1.日    期  : 2014年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mpxx_host_main_exit(void)
{
    uint16_t us_bitmap;

    wal_hipriv_unregister_inetaddr_notifier();
    wal_hipriv_unregister_inet6addr_notifier();

    us_bitmap = BIT6 | BIT7 | BIT8;
    builder_module_exit(us_bitmap);

    return;
}

#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

#include "board.h"

int32_t g_wifi_init_flag = 0;
int32_t g_wifi_init_ret;
/* built-in */
OAL_STATIC ssize_t wifi_sysfs_set_init(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    char mode[128] = { 0 }; /* array lenth 128 */

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%20s", mode, sizeof(mode)) != 1)) {
        oal_io_print("set value one param!\n");
        return -OAL_EINVAL;
    }

    if (sysfs_streq("init", mode)) {
        /* init */
        if (g_wifi_init_flag == 0) {
            g_wifi_init_ret = mpxx_host_main_init();
            g_wifi_init_flag = 1;
        } else {
            oal_io_print("double init!\n");
        }
    } else {
        oal_io_print("invalid input:%s\n", mode);
    }

    return count;
}

OAL_STATIC ssize_t wifi_sysfs_get_init(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (oal_any_null_ptr3(buf, attr, dev)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (g_wifi_init_flag == 1) {
        if (g_wifi_init_ret == OAL_SUCC) {
            ret += snprintf_s(buf + ret, PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "running\n");
        } else {
            ret += snprintf_s(buf + ret, PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1,
                              "boot failed ret=%d\n", g_wifi_init_ret);
        }
    } else {
        ret += snprintf_s(buf + ret, PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "uninit\n");
    }

    return ret;
}
STATIC struct kobj_attribute g_dev_attr_wifi =
    __ATTR(wifi, S_IRUGO | S_IWUSR, wifi_sysfs_get_init, wifi_sysfs_set_init);
OAL_STATIC struct attribute *g_wifi_init_sysfs_entries[] = {
    &g_dev_attr_wifi.attr,
    NULL
};

OAL_STATIC struct attribute_group g_wifi_init_attribute_group = {
    .attrs = g_wifi_init_sysfs_entries,
};

int32_t wifi_sysfs_init(void)
{
    int32_t ret;
    oal_kobject *pst_root_boot_object = NULL;

    if ((is_hisi_chiptype(BOARD_VERSION_MP13) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_MP15) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_MP16) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_MP16C) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_GF61) == false)) {
        return OAL_SUCC;
    }

    pst_root_boot_object = oal_get_sysfs_root_boot_object();
    if (pst_root_boot_object == NULL) {
        oal_io_print("[E]get root boot sysfs object failed!\n");
        return -OAL_EBUSY;
    }

    ret = sysfs_create_group(pst_root_boot_object, &g_wifi_init_attribute_group);
    if (ret) {
        oal_io_print("sysfs create plat boot group fail.ret=%d\n", ret);
        ret = -OAL_ENOMEM;
        return ret;
    }

/* if TV && built-in, init dd here */
#ifdef _PRE_SUPPORT_LONG_CALI_COMPLETE_TIME
    oal_init_completion(hmac_get_wlan_cali_completed_addr());
#endif

    return ret;
}

void wifi_sysfs_exit(void)
{
    /* need't exit,built-in */
    return;
}
oal_module_init(wifi_sysfs_init);
oal_module_exit(wifi_sysfs_exit);
#endif
#else
oal_module_init(mpxx_host_main_init);
oal_module_exit(mpxx_host_main_exit);
#endif

oal_module_license("GPL");
