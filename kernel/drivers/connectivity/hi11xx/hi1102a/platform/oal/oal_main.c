

/* 头文件包含 */
#include "wlan_types.h"
#include "oal_main.h"
#include "oal_schedule.h"
#include "oal_net.h"
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#include "oal_hcc_host_if.h"
#endif
#include "oal_kernel_file.h"

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#ifndef WIN32
#include "plat_firmware.h"
#endif
#endif

#ifdef _PRE_MEM_TRACE
#include "mem_trace.h"
#endif
#include "securec.h"

#define OAL_MAX_TRACE_ENTRY 64

typedef struct {
    oal_spin_lock_stru st_lock;
    unsigned long aul_trace_entries[OAL_MAX_TRACE_ENTRY];
} oal_stacktrace_stru;

/* 全局变量定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void __iomem *l2cache_base;
#endif

/* 动态/静态DBDC，其中一个默认使能 */
oal_uint8 g_wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = { WLAN_INIT_DEVICE_RADIO_CAP };

oal_uint8 g_hitalk_status = 0; /* 记录当前固件类型 */
#ifdef _PRE_WLAN_NARROW_BAND
extern oal_uint8 g_narrow_bw; /* 窄带默认不开 */
#endif
/* 函数实现 */
/* 有些内核版本可能没开启CONFIG_STACKTRACE, 增加CONFIG_STACKTRACE的宏 */
#define oal_dump_stack_str_init()
oal_int32 oal_dump_stack_str(oal_uint8 *puc_str, oal_uint32 ul_max_size)
{
    return 0;
}

/*
 * 函 数 名  : oal_main_init
 * 功能描述  : OAL模块初始化总入口，包含OAL模块内部所有特性的初始化。
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_int32 ATTR_OAL_NO_FUNC_TRACE oal_main_init(oal_void)
{
    oal_uint32 ul_rslt;
#ifdef _PRE_MEM_TRACE
    mem_trace_init();
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if (oal_conn_sysfs_root_obj_init() == NULL) {
        OAL_IO_PRINT("hisi root sysfs init failed\n");
    }
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && \
    defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /* 110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功 */
#else
    /* 内存池初始化 */
    ul_rslt = oal_mem_init_pool();
    if (ul_rslt != OAL_SUCC) {
        OAL_IO_PRINT("oal_main_init: oal_mem_init_pool return error code: %d", ul_rslt);
        return -OAL_EFAIL;  //lint !e527
    }
#endif

#if ((defined(_PRE_PRODUCT_ID_HI110X_HOST)) || (defined(_PRE_PRODUCT_ID_HI110X_DEV)))

    /* 初始化: 总线上的chip数量增加1 */
    oal_bus_init_chip_num();
    ul_rslt = oal_bus_inc_chip_num();
    if (ul_rslt != OAL_SUCC) {
        OAL_IO_PRINT("oal_pci_probe: oal_bus_inc_chip_num failed!\n");
        return -OAL_EIO;
    }
#endif

    /* 启动成功 */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    memset_s(g_past_net_device, WLAN_NET_DEVICE_MAX_NUM * OAL_SIZEOF(oal_net_device_stru *),
             0, WLAN_NET_DEVICE_MAX_NUM * OAL_SIZEOF(oal_net_device_stru *));
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* HCC初始化 */
    if (oal_unlikely(hcc_dev_init() != OAL_SUCC)) {
        OAL_IO_PRINT("[ERROR]hcc_module_init return err null\n");
        return -OAL_EFAIL;
    }
#if defined(_PRE_PLAT_FEATURE_HI110X_PCIE) && defined(CONFIG_ARCH_SD56XX)
    /* 5610 udp pcie chip test */
    hcc_enable(hcc_get_110x_handler(), OAL_FALSE);
#endif
#endif

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
    if (oal_unlikely(oal_softwdt_init() != OAL_SUCC)) {
        OAL_IO_PRINT("oal_softwdt_init init failed!\n");
        return -OAL_EFAIL;
    }
#endif

#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
    if (oal_unlikely(oal_dft_init() != OAL_SUCC)) {
        OAL_IO_PRINT("oal_dft_init init failed!\n");
        return -OAL_EFAIL;
    }
#endif

    oal_dump_stack_str_init();

    oal_workqueue_init();

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    oal_register_syscore_ops();
#endif
    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_main_exit
 * 功能描述  : OAL模块卸载
 * 返 回 值  : 模块卸载返回值，成功或失败原因
 */
oal_void ATTR_OAL_NO_FUNC_TRACE oal_main_exit(oal_void)
{
#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
    oal_dft_exit();
#endif

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
    oal_softwdt_exit();
#endif

#if ((defined(_PRE_PRODUCT_ID_HI110X_HOST)) || (defined(_PRE_PRODUCT_ID_HI110X_DEV)))

    /* chip num初始化:0 */
    oal_bus_init_chip_num();
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && \
    defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /* 110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功 */
#else
    /* 内存池卸载 */
    oal_mem_exit();
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    oal_conn_sysfs_root_boot_obj_exit();
    oal_conn_sysfs_root_obj_exit();
#endif
    oal_workqueue_exit();

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    oal_unregister_syscore_ops();
#endif

#ifdef _PRE_MEM_TRACE
    mem_trace_exit();
#endif

    return;
}

/*
 * 函 数 名  : oal_device_check_enable_num
 * 功能描述  : 检查单chip中mac device使能个数
 * 返 回 值  : 使能device个数
 */
OAL_STATIC oal_uint8 oal_device_check_enable_num(oal_void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_uint8 uc_device_num = 0;
    oal_uint8 uc_device_id;

    for (uc_device_id = 0; uc_device_id < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_id++) {
        if (g_wlan_service_device_per_chip[uc_device_id]) {
            uc_device_num++;
        }
    }
    return uc_device_num;
#else
    return WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP;
#endif  // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
}

/*
 * 函 数 名  : oal_chip_get_version
 * 功能描述  : 根据chip version获取device num
 */
oal_uint8 oal_chip_get_device_num(oal_uint32 ul_chip_ver)
{
    return oal_device_check_enable_num();
}
/*
 * 函 数 名  : oal_board_get_service_vap_start_id
 * 功能描述  : 获取board上的业务vap其实idx
 */
oal_uint8 oal_board_get_service_vap_start_id(oal_void)
{
    oal_uint8 uc_device_num_per_chip = oal_device_check_enable_num();

    /* 配置vap个数 = mac device个数,vap idx分配先配置vap,后业务vap */
    return (oal_uint8)(WLAN_CHIP_MAX_NUM_PER_BOARD * uc_device_num_per_chip);
}

/*lint -e578*/ /*lint -e19*/
oal_module_symbol(oal_dump_stack_str);
oal_module_symbol(oal_chip_get_device_num);
oal_module_symbol(oal_board_get_service_vap_start_id);
oal_module_symbol(oal_main_init);
oal_module_symbol(oal_main_exit);
oal_module_symbol(l2cache_base);
oal_module_symbol(g_wlan_service_device_per_chip);
oal_module_symbol(g_hitalk_status);
oal_module_license("GPL");
