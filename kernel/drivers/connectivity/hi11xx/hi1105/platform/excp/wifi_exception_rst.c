/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Entry of the wifi exception reset function
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "plat_pm.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "plat_firmware_download.h"
#include "board.h"
#include "oam_ext_if.h"
#include "oal_hcc_bus.h"
#include "chr_user.h"
#include "oal_hcc_host_if.h"
#include "bfgx_exception_rst.h"
#include "gt_exception_rst.h"
#include "exception_common.h"
#include "wifi_exception_rst.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WIFI_EXCEPTION_RST_C

memdump_info_t g_wcpu_memdump_cfg;

int32_t get_wifi_subsys_state(excp_comm_t *node_ptr)
{
    struct wlan_pm_s *wlan_pm = NULL;
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("subsys_res is NULL"NEWLINE);
        return POWER_STATE_SHUTDOWN;
    }

    wlan_pm = (struct wlan_pm_s*)subsys_res->subsys_pm;
    return wlan_pm->wlan_power_state;
}

OAL_STATIC int32_t wifi_shutdown_notify_for_excp(excp_comm_t *node_ptr)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (pst_exception_data->wifi_callback->wifi_recovery_complete == NULL) {
        ps_print_err("wifi recovery complete callback not regist\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->wifi_callback->wifi_recovery_complete(OAL_FAIL);
    return EXCEPTION_SUCCESS;
}

int32_t wifi_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (wlan_is_shutdown()) {
        oam_warning_log0(0, OAM_SF_DFR, "[MPXX_DFR] wifi is shutdown, do nothing\n");
        return EXCEPTION_FAIL;
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_DFR, CHR_PLAT_DRV_ERROR_EXCP_WIFI_HANDLE);

    /* notify_wifi_to_recovery会判断当前是否在做恢复, 是则立即返回, 不会重复做业务处理 */
    if (pst_exception_data->wifi_callback->notify_wifi_to_recovery != NULL) {
        ps_print_info("notify wifi start to power reset\n");
        pst_exception_data->wifi_callback->notify_wifi_to_recovery();
    }
    return EXCEPTION_SUCCESS;
}

int32_t wifi_power_reset(excp_comm_t *node_ptr)
{
    int32_t ret;

    oal_reference(node_ptr);
    ret = board_sys_disable(W_SYS);
    if (ret != OAL_SUCC) {
        return ret;
    }
    mdelay(WIFI_SUBSYS_RST_DELAY);
    return board_sys_enable(W_SYS);
}

void wifi_excp_power_off(excp_comm_t *node_ptr, uint8_t flag)
{
    if (flag == OAL_FALSE) {
        board_power_off(W_SYS);
        return;
    }

    wlan_pm_close_for_excp();
    wifi_shutdown_notify_for_excp(node_ptr);
}

int32_t wifi_firmware_download_pre(excp_comm_t *node_ptr)
{
    oal_reference(node_ptr);
    return hcc_bus_power_action(hcc_get_bus(HCC_EP_WIFI_DEV), HCC_BUS_POWER_PATCH_LOAD_PREPARE);
}

int32_t wifi_firmware_download_do(excp_comm_t *node_ptr)
{
    oal_reference(node_ptr);
    return firmware_download_function(WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
}

int32_t wifi_firmware_download_post(excp_comm_t *node_ptr)
{
    int32_t ret;

    oal_reference(node_ptr);
    ret = hcc_bus_power_action(hcc_get_bus(HCC_EP_WIFI_DEV), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_DFR, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_WCPU_BOOTUP);
        oam_error_log1(0, OAM_SF_DFR, "[MPXX_DFR] HCC_BUS_POWER_PATCH_LAUCH failed, ret=%d\n", ret);
        return EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

int32_t wifi_reset_post_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (oal_unlikely(pst_exception_data == NULL || subsys_res == NULL)) {
        ps_print_err("exception info or subsys_res is NULL"NEWLINE);
        return -EXCEPTION_FAIL;
    }

    // 下发定制化参数到device去
    wifi_customize_h2d();

    subsys_res->subsys_excp_stats[excp_record->exception_type]++;
    subsys_res->subsys_excp_cnt++;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_DFR_SUCC);
    oam_warning_log0(0, OAM_SF_DFR, "[MPXX_DFR] wifi reset, plat dfr done\n");

    if (pst_exception_data->wifi_callback->wifi_recovery_complete == NULL) {
        ps_print_err("wifi recovery complete callback not regist\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->wifi_callback->wifi_recovery_complete(OAL_SUCC);

    ps_print_info("wifi recovery complete\n");
    return EXCEPTION_SUCCESS;
}

OAL_STATIC int32_t wifi_device_reset(void)
{
    ps_print_info("reset wifi device by w_en gpio\n");

    board_sys_disable(W_SYS);

    if (board_sys_enable(W_SYS)) {
        ps_print_err("MP13 wifi enable failed\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

OAL_STATIC int32_t wlan_exception_reinit_hcc_bus(uint32_t dev_id)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);
    int32_t ret = -OAL_EIO;
    if (pst_bus_dev != NULL) {
        ret = hcc_bus_reinit(pst_bus_dev->cur_bus);
    }
    return ret;
}

STATIC int32_t get_wifi_memdump(hcc_bus *hi_bus, int32_t excep_type)
{
    int32_t ret;
    int32_t reset_flag;
    int32_t is_gt = (hi_bus->dev_id == HCC_EP_GT_DEV) ? OAL_TRUE : OAL_FALSE;

    /* device panic不需要复位device，其他异常需要先复位device */
    reset_flag = (excep_type == HCC_BUS_PANIC) ? 0 : 1;

    forever_loop() {
        if (reset_flag == 1) {
            ret = (is_gt == OAL_TRUE) ? gt_device_reset() : wifi_device_reset();
            if (ret != EXCEPTION_SUCCESS) {
                break;
            }
            hcc_change_state_exception(hi_bus->bus_dev->hcc);
        }

        ret = wlan_exception_reinit_hcc_bus(hi_bus->dev_id);
        if (ret != OAL_SUCC) {
            ps_print_err("wlan mem dump:current bus reinit failed, ret=[%d]\n", ret);
            /* 如果panic恢复失败，会复位重试一次 */
            if (reset_flag == 0) {
                ps_print_err("reinit failed, try to reset wifi\n");
                reset_flag = 1;
                continue;
            }
            break;
        }

        ret = (is_gt == OAL_TRUE) ? gt_device_mem_dump(hi_bus, excep_type) :
                                    wifi_device_mem_dump(hi_bus, excep_type);
        hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_ALL);
        if (ret < 0) {
            ps_print_err("wlan device_mem_dump failed, ret=[%d]\n", ret);
            /* 如果panic恢复失败，会复位重试一次 */
            if (reset_flag == 0) {
                ps_print_err("memdump failed, try to reset wlan\n");
                reset_flag = 1;
                continue;
            }
        }
        break;
    }

    return ret;
}

/*
 * 函 数 名  : wifi_exception_mem_dump
 * 功能描述  : 全系统复位，firmware重新加载的时候，导出device指定地址的内存
 * 输入参数  : pst_mem_dump_info  : 需要读取的内存信息
 *             count              : 需要读取的内存块个数
 */
int32_t wifi_exception_mem_dump(hcc_bus *hi_bus)
{
    int32_t ret;
    int32_t excep_type = hi_bus->bus_excp_type;
    struct pm_top *pm_top_data = pm_get_top();

    if (excep_type != HCC_BUS_PANIC && excep_type != HCC_BUS_TRANS_FAIL) {
        ps_print_err("unsupport exception type :%d\n", excep_type);
        return -EXCEPTION_FAIL;
    }

    mutex_lock(&(pm_top_data->host_mutex));

    if (wlan_is_shutdown()) {
        ps_print_err("[E]dfr ignored, wifi shutdown before memdump\n");
        mutex_unlock(&(pm_top_data->host_mutex));
        return -OAL_EIO;
    }

    ret = get_wifi_memdump(hi_bus, excep_type);
    mutex_unlock(&(pm_top_data->host_mutex));
    return ret;
}

/*
 * 函 数 名  : wifi_exception_work_submit
 * 功能描述  : 异常恢复动作触发接口
 */
int32_t wifi_exception_work_submit(hcc_bus *hi_bus, uint32_t wifi_excp_type)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if ((pst_exception_data == NULL) || (hi_bus == NULL)) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (oal_work_is_busy(&pst_exception_data->wifi_excp_worker)) {
        ps_print_warning("WIFI DFR %pF Worker is Processing:doing wifi excp_work...need't submit\n",
                         (void *)pst_exception_data->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else if (oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker)) {
        ps_print_warning(
            "WIFI DFR %pF Recovery_Worker is Processing:doing wifi wifi_excp_recovery_worker ...need't submit\n",
            (void *)pst_exception_data->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else if ((in_plat_exception_reset() == OAL_TRUE) && (pst_exception_data->subsys_type == SUBSYS_WIFI)) {
        ps_print_warning("WIFI DFR %pF Recovery_Worker is Processing:doing  plat wifi recovery ...need't submit\n",
                         (void *)pst_exception_data->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else {
        int32_t ret = -OAL_EFAIL;
        ret = wifi_exception_mem_dump(hi_bus);
        if (ret < 0) {
            ps_print_err("Panic File Save Failed!");
        } else {
            ps_print_info("Panic File Save OK!");
        }

        /* 为了在没有开启DFR  的情况下也能mem dump, 故在此处作判断 */
        if (pst_exception_data->exception_reset_enable != OAL_TRUE) {
            ps_print_info("plat exception reset not enable!");
            return EXCEPTION_SUCCESS;
        }

        ps_print_err("WiFi DFR %pF Worker Submit, excp_type[%d]\n",
                     (void *)pst_exception_data->wifi_excp_worker.func, wifi_excp_type);
        pst_exception_data->wifi_excp_type = wifi_excp_type;
        hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_ALL);
        queue_work(pst_exception_data->wifi_exception_workqueue, &pst_exception_data->wifi_excp_worker);
    }
#else
    ps_print_warning("Geting WIFI DFR, but _PRE_WLAN_FEATURE_DFR not open!");
#endif  // _PRE_WLAN_FEATURE_DFR
    return OAL_SUCC;
}

oal_workqueue_stru *wifi_get_exception_workqueue(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (pst_exception_data == NULL) {
        return NULL;
    }
    return pst_exception_data->wifi_exception_workqueue;
}

int32_t plat_wifi_exception_rst_register(void *data)
{
    struct st_wifi_dfr_callback *pst_wifi_callback = NULL;

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (data == NULL) {
        ps_print_err("param data is null\n");
        return -EXCEPTION_FAIL;
    }

    /* wifi异常回调函数注册 */
    pst_wifi_callback = (struct st_wifi_dfr_callback *)data;
    pst_exception_data->wifi_callback = pst_wifi_callback;

    return EXCEPTION_SUCCESS;
}

void wifi_memdump_finish(void)
{
    g_wcpu_memdump_cfg.is_working = 0;
}

int32_t wifi_notice_hal_memdump(void)
{
    return notice_hal_memdump(&g_wcpu_memdump_cfg, CMD_READM_WIFI_SDIO);
}

int32_t wifi_memdump_enquenue(uint8_t *buf_ptr, uint16_t count)
{
    return excp_memdump_queue(buf_ptr, count, &g_wcpu_memdump_cfg);
}

EXPORT_SYMBOL_GPL(plat_wifi_exception_rst_register);
