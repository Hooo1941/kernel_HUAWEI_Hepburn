/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oam_dsm.c
 * Author: @CompanyNameTag
 */

#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
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
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_DSM_C
#define DMD_EVENT_BUFF_SIZE 1024

#ifdef CONFIG_HISYSEVENT
struct error_no_map {
    int error_no;
    char *name;
};

struct error_no_map g_wifi_error_no_map[] = {
    {909030001, "DSM_WIFI_MODULE_INIT_ERROR"},
    {909030002, "DSM_WIFI_KSO_ERROR"},
    {909030003, "DSM_WIFI_CMD_FIVE_TWO_ERROR"},
    {909030004, "DSM_WIFI_CMD_FIVE_THREE_ERROR"},
    {909030005, "DSM_WIFI_SDIO_RESET_COMM_ERROR"},
    {909030006, "DSM_WIFI_SDIO_PROBE_ATTACH_ERR"},
    {909030007, "DSM_WIFI_SDIO_FIRMWARE_DL_ERR"},
    {909030008, "DSM_WIFI_DHD_DEV_INIT_IOCTL_ERR"},
    {909030009, "DSM_WIFI_WLC_SET_PSV_SCAN_ERR"},
    {909030010, "DSM_WIFI_WLC_SCAN_ERROR"},
    {909030011, "DSM_WIFI_WLC_SET_SCANSPRS_ERR"},
    {909030012, "DSM_WIFI_WLC_GET_CHANNEL_ERR"},
    {909030013, "DSM_WIFI_WLC_SCAN_RESULTS_ERR"},
    {909030014, "DSM_WIFI_WLC_DISASSOC_ERROR"},
    {909030015, "DSM_WIFI_WLC_SET_SSID_ERROR"},
    {909030016, "DSM_WIFI_SET_CIPHER_ERROR"},
    {909030017, "DSM_WIFI_SET_KEY_MGMT_ERROR"},
    {909030018, "DSM_WIFI_SET_SHAREDKEY_ERROR"},
    {909030019, "DSM_WIFI_OPEN_ERROR"},
    {909030020, "DSM_WIFI_CHIPSET_DAMAGE_WARNING"},
    {909030021, "DSM_WIFI_TEM_CTRL_EVENT"},
    {909030022, "DSM_WIFI_PMIC_CHECK_EVENT"},
    {909030023, "DSM_WIFI_OTP_ERROR"},
    {909030024, "DSM_WIFI_PCIE_LINKDOWN"},
    {909030025, "DSM_WIFI_PCIE_READ_ERROR"},
    {909030026, "DSM_WIFI_DEVWAKE_TIMEOUT"},
    {909030032, "DSM_PCIE_LINKFAIL"},
    {909030033, "DSM_WIFI_LOAD_FIRMWARE"},
    {909030034, "DSM_BUCK_PROTECTED"},
    {909030035, "DSM_MP_ONE_THREE_HALT"},
    {909030036, "DSM_WIFI_FEMERROR"},
    {909030037, "DSM_SDIO_TUNNING_ERR"},
    {909030038, "DSM_SDIO_DATA_TIMEOUT"},
    {909030039, "DSM_SDIO_CRC_ERR"},
    {909030040, "DSM_ONE_TWO_WIFI_SDIO_PB_AT_ERR"},
    {909030041, "DSM_MP_ONE_TWO_HALT"},
    {909030042, "DSM_ONE_TWO_DOWNLOAD_FIRMWARE"},
    {909030043, "DSM_PCIE_SWITCH_SDIO_FAIL"},
    {909030044, "DSM_PCIE_SWITCH_SDIO_SUCC"},
    {909030046, "DSM_WIFI_LNAERROR"},
};

int wifi_errorno_to_str(int error_no, char *str, int buff_len)
{
    int i;

    int isp_error_no_map_len = sizeof(g_wifi_error_no_map) / sizeof(struct error_no_map);
    for (i = 0; i < isp_error_no_map_len; i++) {
        if (g_wifi_error_no_map[i].error_no == error_no) {
            if (strlen(g_wifi_error_no_map[i].name) >= buff_len) {
                oal_io_print("[E]length error\n");
                return -1;
            }
            if (strncpy_s(str, buff_len, g_wifi_error_no_map[i].name, strlen(g_wifi_error_no_map[i].name)) != EOK) {
                oal_io_print("[E]cpy error\n");
                return -1;
            }
            str[buff_len - 1] = '\0';
            oal_io_print("[I]dsm wifi_errorno_to_str str=%s\n", str);
            return 0;
        }
    }
    return -1;
}

struct dsm_client_ops hisp_dsm_ops_wifi = {
    .poll_state = NULL,
    .dump_func = NULL,
    .errorno_to_str = wifi_errorno_to_str,
};
#endif

/*
 * 函 数 名  : hw_mpxx_dsm_client_notify
 * 功能描述  : DMD事件上报
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
static struct dsm_dev g_dsm_wifi = {
#ifdef CONFIG_HISYSEVENT
    .name = "COMMUNICATION",
#else
    .name = "dsm_wifi",
#endif
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
#ifdef CONFIG_HISYSEVENT
    .fops = &hisp_dsm_ops_wifi,
#else
    .fops = NULL,
#endif
    .buff_size = DMD_EVENT_BUFF_SIZE,
};

static struct dsm_dev g_dsm_bt = {
    .name = "dsm_bt",
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
    .fops = NULL,
    .buff_size = DMD_EVENT_BUFF_SIZE,
};

static struct dsm_client *g_dsm_wifi_client = NULL;
static struct dsm_client *g_dsm_bt_client = NULL;

static inline struct dsm_client* get_dsm_client(int sub_sys)
{
    struct dsm_client *client_buf = NULL;

    if ((sub_sys == SYSTEM_TYPE_WIFI) || (sub_sys == SYSTEM_TYPE_PLATFORM)) {
        client_buf = g_dsm_wifi_client;
    } else if (sub_sys == SYSTEM_TYPE_BT) {
        client_buf = g_dsm_bt_client;
    }

    return client_buf;
}

void hw_mpxx_register_dsm_client(void)
{
    if (g_dsm_wifi_client == NULL) {
        g_dsm_wifi_client = dsm_register_client(&g_dsm_wifi);
    }

    if (g_dsm_bt_client == NULL) {
        g_dsm_bt_client = dsm_register_client(&g_dsm_bt);
    }
}

void hw_mpxx_unregister_dsm_client(void)
{
    if (g_dsm_wifi_client != NULL) {
        dsm_unregister_client(g_dsm_wifi_client, &g_dsm_wifi);
        g_dsm_wifi_client = NULL;
    }
    if (g_dsm_bt_client != NULL) {
        dsm_unregister_client(g_dsm_bt_client, &g_dsm_bt);
        g_dsm_bt_client = NULL;
    }
}
#define LOG_BUF_SIZE 512
static int g_last_dsm_id = 0;

void hw_mpxx_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...)
{
    char buf[LOG_BUF_SIZE] = {0};
    struct dsm_client *dsm_client_buf = NULL;
    va_list ap;
    int32_t ret = 0;

    dsm_client_buf = get_dsm_client(sub_sys);
    if (dsm_client_buf == NULL) {
        oam_error_log1(0, 0, "dsm subsys[%d] didn't suport\n", sub_sys);
        return;
    }

    if (dsm_client_buf != NULL) {
        if (fmt != NULL) {
            va_start(ap, fmt);
            ret = vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, fmt, ap);
            va_end(ap);
            if (ret < 0) {
                oam_error_log1(0, 0, "vsnprintf_s fail, line[%d]\n", __LINE__);
                return;
            }
        } else {
            oam_error_log1(0, 0, "dsm_client_buf is null, line[%d]\n", __LINE__);
            return;
        }
    }

    if (!dsm_client_ocuppy(dsm_client_buf)) {
        dsm_client_record(dsm_client_buf, buf);
        dsm_client_notify(dsm_client_buf, dsm_id);
        g_last_dsm_id = dsm_id;
        oam_error_log1(0, OAM_SF_PWR, "wifi dsm_client_notify success,dsm_id=%d", dsm_id);
        oal_io_print("[I]wifi dsm_client_notify success,dsm_id=%d[%s]\n", dsm_id, buf);
    } else {
        oam_error_log2(0, OAM_SF_PWR, "wifi dsm_client_notify failed,last_dsm_id=%d dsm_id=%d", g_last_dsm_id, dsm_id);
        oal_io_print("[E]wifi dsm_client_notify failed,last_dsm_id=%d dsm_id=%d\n", g_last_dsm_id, dsm_id);

        // retry dmd record
        dsm_client_unocuppy(dsm_client_buf);
        if (!dsm_client_ocuppy(dsm_client_buf)) {
            dsm_client_record(dsm_client_buf, buf);
            dsm_client_notify(dsm_client_buf, dsm_id);
            oam_error_log1(0, OAM_SF_PWR, "wifi dsm_client_notify success,dsm_id=%d", dsm_id);
            oal_io_print("[I]wifi dsm notify success, dsm_id=%d[%s]\n", dsm_id, buf);
        } else {
            oam_error_log1(0, OAM_SF_PWR, "wifi dsm client ocuppy, dsm notify failed, dsm_id=%d", dsm_id);
            oal_io_print("[E]wifi dsm client ocuppy, dsm notify failed, dsm_id=%d\n", dsm_id);
        }
    }
}
EXPORT_SYMBOL(hw_mpxx_dsm_client_notify);

#endif
