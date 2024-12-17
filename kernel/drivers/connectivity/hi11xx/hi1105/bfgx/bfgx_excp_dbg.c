/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: User Control System File Node
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "plat_debug.h"
#include "plat_pm.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "oal_kernel_file.h"
#include "bfgx_user_ctrl.h"
#include "bfgx_data_parse.h"
#include "plat_exception_rst.h"
#include "bfgx_excp_dbg.h"

#ifdef PLATFORM_DEBUG_ENABLE
typedef struct {
    int32_t cmd;
    int32_t (*exception_dbg_func)(struct ps_core_s *ps_core_d);
} excp_dbg_cmd_t;

STATIC int32_t bfgx_timeout_dbg(struct ps_core_s *ps_core_d);
STATIC int32_t bfgx_power_on_fail_test(struct ps_core_s *ps_core_d);
STATIC int32_t bfgx_power_off_fail_test(struct ps_core_s *ps_core_d);
STATIC int32_t bfgx_device_panic(struct ps_core_s *ps_core_d);
STATIC int32_t bfgx_arp_timeout(struct ps_core_s *ps_core_d);
STATIC int32_t bfgx_wkup_fail_test(struct ps_core_s *ps_core_d);
STATIC int32_t dfr_sys_rst_download_fail_test(struct ps_core_s *ps_core_d);
STATIC int32_t dfr_sys_rst_uart_suspend_test(struct ps_core_s *ps_core_d);
STATIC int32_t dfr_sys_rst_uart_suspend_clean(struct ps_core_s *ps_core_d);


/* cmd功能与原来保持一致，便于测试切换 */
excp_dbg_cmd_t g_bfgx_excp_dbg[] = {
    {1, NULL},
    {2, bfgx_timeout_dbg},                /* cmd: 2, trigger bfgx heartbeat timeout */
    {3, NULL},
    {4, NULL},
    {5, bfgx_power_on_fail_test},         /* cmd: 5, trigger bfgx_power_on_fail */
    {6, bfgx_power_off_fail_test},        /* cmd: 6, trigger bfgx_power_off_fail */
    {7, NULL},
    {8, bfgx_device_panic},               /* cmd: 8, trigger bfgx_device_panic */
    {9, bfgx_arp_timeout},                /* cmd: 9, trigger bfgx_arp_timeout excp, dfr_system_rst */
    {10, bfgx_wkup_fail_test},            /* cmd: 10, trigger bfgx_wkup_fail */
    {11, NULL},
    {12, NULL},
    {13, dfr_sys_rst_download_fail_test}, /* cmd: 13, trigger dfr_sys_rst_download_fail */
    {14, dfr_sys_rst_uart_suspend_test},  /* cmd: 14, 模拟uart_suspend场景，构造异常，进入dfr下电流程 */
    {15, dfr_sys_rst_uart_suspend_clean}  /* cmd: 15, 清理uart_suspend标志位 */
};

STATIC int32_t bfgx_timeout_dbg(struct ps_core_s *ps_core_d)
{
    int32_t ret = 0;
    int32_t prepare_result;

    ps_print_info("[dfr_test]bfgx device timeout cause ++\n");
    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("[dfr_test]prepare work FAIL\n");
        return OAL_FAIL;
    }

    set_excp_test_en(HEARTBEATER_TIMEOUT_FAULT, (uintptr_t)ps_core_d);

    post_to_visit_node(ps_core_d, prepare_result);
    ps_print_info("[dfr_test]bfgx device timeout cause --\n");
    return OAL_SUCC;
}

STATIC int32_t bfgx_power_on_fail_test(struct ps_core_s *ps_core_d)
{
    ps_print_info("[dfr_test]bfgx powon fail dfr test en, next poweron will fail\n");
    set_excp_test_en(BFGX_POWEON_FAULT, (uintptr_t)ps_core_d);
    return OAL_SUCC;
}

STATIC int32_t bfgx_power_off_fail_test(struct ps_core_s *ps_core_d)
{
    ps_print_info("[dfr_test]bfgx pwr off dfr test en, next poweroff will fail\n");
    set_excp_test_en(BFGX_POWEOFF_FAULT, (uintptr_t)ps_core_d);
    return OAL_SUCC;
}

STATIC int32_t bfgx_device_panic(struct ps_core_s *ps_core_d)
{
    int32_t ret = 0;
    int32_t prepare_result;

    ret = prepare_to_visit_node(ps_core_d, &prepare_result);
    if (ret < 0) {
        ps_print_err("[dfr_test]prepare work FAIL\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]bfgx device panic cause\n");
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_DEV_PANIC);
    post_to_visit_node(ps_core_d, prepare_result);
    return OAL_SUCC;
}

STATIC int32_t bfgx_arp_timeout(struct ps_core_s *ps_core_d)
{
    oal_reference(ps_core_d);

    ps_print_info("[dfr_test]trigger hal BFGX_ARP_TIMEOUT exception\n");
    plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, BFGX_ARP_TIMEOUT);
    return OAL_SUCC;
}

STATIC int32_t bfgx_wkup_fail_test(struct ps_core_s *ps_core_d)
{
    ps_print_info("[dfr_test]bfgx wkup dfr test en, next wkup will fail \n");
    set_excp_test_en(BFGX_WKUP_FAULT, (uintptr_t)ps_core_d);
    return OAL_SUCC;
}

STATIC int32_t dfr_sys_rst_download_fail_test(struct ps_core_s *ps_core_d)
{
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    oal_reference(ps_core_d);

    ps_print_info("[dfr_test]trigger dfr_sys_rst_download_fail_test\n");

    pst_exception_data->sys_rst_download_dbg = OAL_TRUE;
    plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, BFGX_ARP_TIMEOUT);
    return OAL_SUCC;
}

STATIC int32_t dfr_sys_rst_uart_suspend_test(struct ps_core_s *ps_core_d)
{
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    oal_reference(ps_core_d);

    ps_print_info("[dfr_test]trigger sys_rst_uart_suspend_test\n");

    pst_exception_data->sys_rst_uart_suspend = OAL_TRUE;

    return OAL_SUCC;
}

STATIC int32_t dfr_sys_rst_uart_suspend_clean(struct ps_core_s *ps_core_d)
{
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    oal_reference(ps_core_d);

    ps_print_info("[dfr_test]sys_rst_uart_suspend_clean\n");

    pst_exception_data->sys_rst_uart_suspend = OAL_FALSE;

    return OAL_SUCC;
}

STATIC int32_t exception_dbg_proc(struct ps_core_s *ps_core_d, int32_t cmd)
{
    if (cmd > oal_array_size(g_bfgx_excp_dbg) || cmd < 1) {
        ps_print_err("[dfr_test] unknown cmd\n");
        return OAL_FAIL;
    }

    if (g_bfgx_excp_dbg[cmd - 1].exception_dbg_func == NULL) {
        ps_print_err("[dfr_test] unsupport cmd, do nothing\n");
        return OAL_FAIL;
    }

    return g_bfgx_excp_dbg[cmd - 1].exception_dbg_func(ps_core_d);
}

ssize_t bfgx_excp_dbg_show_comm(char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
                      "==========cmd  func             \n"
                      "   1  cause bfgx beattimer timerout\n"
                      "   2  en dfr bfgx pwron fail\n"
                      "   3  en dfr bfgx pwroff fail\n"
                      "   4  cause bfgx panic\n"
                      "   5  cause bfgx arp exception system rst\n"
                      "   6  bfgx wkup fail\n"
                      "   7  download fail, trigger dfr sys rst fail, then dfr power off\n"
                      "   8  uart suspend, trigger dfr sys rst fail, then dfr power off\n"
                      "   9  uart suspend clean\n");
}

ssize_t bfgx_excp_dbg_store_comm(struct device *dev, const char *buf, size_t count)
{
    int32_t cmd;
    struct ps_core_s *ps_core_d = NULL;
    struct ps_plat_s *ps_plat_d = NULL;
    struct miscdevice *miscdev = dev_get_drvdata(dev);

    ps_core_d = get_ps_core_from_miscdev(miscdev);
    if (oal_unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (sscanf_s(buf, "%d", &cmd) != 1) {
        ps_print_err("input subsys name error\n");
        return -EINVAL;
    }

    ps_plat_d = (struct ps_plat_s *)ps_core_d->ps_plat;
    if (oal_unlikely(ps_plat_d == NULL)) {
        ps_print_err("ps_plat_d is NULL\n");
        return -EINVAL;
    }

    ps_print_info("[dfr_test] uart=%s, cmd=%d"NEWLINE, ps_plat_d->uart_name, cmd);
    if (exception_dbg_proc(ps_core_d, cmd) != OAL_SUCC) {
        return -FAILURE;
    }

    return count;
}
#endif
