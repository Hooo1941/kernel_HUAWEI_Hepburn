/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: PM function module
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include <linux/module.h> /* kernel module definitions */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>
#include <linux/mutex.h>
#include <linux/kernel.h>

#include "board.h"
#include "ssi_common.h"
#include "bfgx_dev.h"
#include "plat_debug.h"
#include "oam_ext_if.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "plat_pm_gt.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"

static int32_t g_firmware_download_process = OAL_FALSE;

int32_t get_firmware_download_flag(void)
{
    return g_firmware_download_process;
}

static void firmware_download_fail_proc(struct pm_top *pm_top_data, int which_cfg)
{
    ps_print_err("firmware download fail!\n");
    declare_dft_trace_key_info("patch_download_fail", OAL_DFT_TRACE_FAIL);
    if (which_cfg == BFGX_CFG) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_BFG_FW_DOWN);
    } else {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_WIFI_FW_DOWN);
    }

    if (pm_top_data->wlan_pm_info->pst_bus->bus_type == HCC_BUS_SDIO) {
        (void)ssi_dump_err_regs(SSI_ERR_FIRMWARE_DOWN_SDIO_FAIL);
    } else {
        (void)ssi_dump_err_regs(SSI_ERR_FIRMWARE_DOWN_FAIL);
    }

    if (mpxx_get_os_build_variant() != MPXX_OS_BUILD_VARIANT_USER) {
        /* dump bootloader rw dtcm */
        ssi_read_reg_info_test(MP13_BOOTLOAD_DTCM_BASE_ADDR, 0, MP13_BOOTLOAD_DTCM_SIZE, 1, SSI_RW_DWORD_MOD);
    }
}


/*
 * Description  : 加载前处理动作
 */
static int pre_download_process(struct pm_top *pm_top_data, firmware_downlaod_privfunc priv_func)
{
    int32_t ret;
    /* firmware_cfg_init(sdio) function should just be called once */
    if (!test_bit(FIRMWARE_CFG_INIT_OK, &pm_top_data->firmware_cfg_init_flag)) {
        ps_print_info("firmware_cfg_init begin\n");
        ret = firmware_cfg_init();
        if (ret) {
            ps_print_err("firmware_cfg_init failed, ret:%d!\n", ret);
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_CFG_FAIL_FIRMWARE_DOWN);
            return ret;
        }

        ps_print_info("firmware_cfg_init OK\n");
        set_bit(FIRMWARE_CFG_INIT_OK, &pm_top_data->firmware_cfg_init_flag);
    }

    /* do some private command before load cfg */
    if (priv_func != NULL) {
        ret = priv_func();
        if (ret) {
            ps_print_err("priv_func failed, ret:%d!\n", ret);
            return ret;
        }
    }

    return SUCCESS;
}

static int firmware_download_wifi_bus_prepare(hcc_bus *bus)
{
    int32_t ret;
    struct st_exception_info *excp_data = get_exception_info_reference();

    if (wlan_is_shutdown() || (in_plat_exception_reset() == OAL_TRUE &&
        (excp_data->record.subsys_type == EXCP_ALL_SYS ||
        excp_data->record.subsys_type == EXCP_W_SYS))) {
        ret = hcc_bus_reinit(bus);
        if (ret != OAL_SUCC) {
            ps_print_err("wifi hcc bus reinit failed, ret:%d!\n", ret);
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_FW_SDIO_INIT);
            return -FAILURE;
        }
        wlan_pm_init_dev();
    } else {
        wlan_pm_disable();
    }
    return SUCCESS;
}

static int firmware_download_gt_bus_prepare(hcc_bus *bus)
{
    int32_t ret;

    if (gt_is_shutdown() || (in_plat_exception_reset() == OAL_TRUE)) {
        ret = hcc_bus_reinit(bus);
        if (ret != OAL_SUCC) {
            ps_print_err("gt hcc bus reinit failed, ret:%d!\n", ret);
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_FW_SDIO_INIT);
            return -FAILURE;
        }
        gt_pm_init_dev();
    } else {
        gt_pm_disable();
    }
    return SUCCESS;
}

/*
* wifi/gt没打开时, 需要初始化bus，否则，关闭wifi/gt低功耗，等加载完成后再开启低功耗
*/
static int firmware_download_bus_prepare(hcc_bus *bus)
{
    if (bus->bus_dev->dev_id == HCC_EP_WIFI_DEV) {
        return firmware_download_wifi_bus_prepare(bus);
    } else if (bus->bus_dev->dev_id == HCC_EP_GT_DEV) {
        return firmware_download_gt_bus_prepare(bus);
    }
    return SUCCESS;
}

static int firmware_download_process(uint32_t which_cfg, firmware_downlaod_privfunc priv_func, hcc_bus *bus)
{
    int32_t ret;
    struct pm_top *pm_top_data = pm_get_top();

    ps_print_info("enter firmware_download_function\n");

    if (pm_top_data == NULL || bus == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    if (which_cfg >= CFG_FILE_TOTAL) {
        ps_print_err("cfg file index [%d] outof range\n", which_cfg);
        return -FAILURE;
    }

    hcc_bus_and_wake_lock(bus);
    g_firmware_download_process = OAL_TRUE;

    if (firmware_download_bus_prepare(bus) != SUCCESS) {
        hcc_bus_and_wake_unlock(bus);
        return -FAILURE;
    }

    ps_print_info("firmware_download begin\n");

    ret = pre_download_process(pm_top_data, priv_func);
    if (ret != SUCCESS) {
        hcc_bus_and_wake_unlock(bus);
        return ret;
    }

    ret = firmware_download_cmd(which_cfg, bus);
    if (ret < 0) {
        hcc_bus_and_wake_unlock(bus);
        if (ret != -OAL_EINTR && ret != -OAL_ENOPERMI) {
            firmware_download_fail_proc(pm_top_data, which_cfg);
        } else {
            /* download firmware interrupt */
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_FILE_OPEN);
            ps_print_info("firmware download interrupt!\n");
            declare_dft_trace_key_info("patch_download_interrupt", OAL_DFT_TRACE_FAIL);
        }
        return ret;
    }
    declare_dft_trace_key_info("patch_download_ok", OAL_DFT_TRACE_SUCC);

    hcc_bus_and_wake_unlock(bus);

    g_firmware_download_process = OAL_FALSE;
    if (!wlan_is_shutdown()) {
        wlan_pm_enable();
    }

    ps_print_info("firmware_download success\n");
    return SUCCESS;
}

/*
 * Prototype    : firmware_download_function
 * Description  : download wlan patch
 * Return       : 0 means succeed,-1 means failed
 */
int firmware_download_function_priv(uint32_t which_cfg, firmware_downlaod_privfunc priv_func, hcc_bus *bus)
{
    int32_t ret;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_time = 0;
    static unsigned long long count = 0;

    start_time = ktime_get();

    ret = firmware_download_process(which_cfg, priv_func, bus);
    if (ret != SUCCESS) {
        return ret;
    }

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_time) {
        max_time = total_time;
    }

    count++;
    ps_print_warning("download firmware, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     count, total_time, max_time);
    return SUCCESS;
}

static uint32_t firware_download_get_cfg(uint32_t which_cfg)
{
    uint32_t cfg = which_cfg;
    if (hi110x_is_emu() != OAL_TRUE) {
        return cfg;
    }
    if (which_cfg == BFGX_AND_WIFI_CFG) { // 单WIFI 不下载BFG
        cfg = WIFI_CFG;
    } else {
        ps_print_err("only support wifi which_cfg=%u\n", which_cfg);
        cfg = CFG_FILE_TOTAL;
    }
    return cfg;
}

int firmware_download_function(uint32_t which_cfg, hcc_bus *bus)
{
    return firmware_download_function_priv(firware_download_get_cfg(which_cfg), NULL, bus);
}
