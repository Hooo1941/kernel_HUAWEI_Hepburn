/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: User Control System File Node
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include <linux/arm-smccc.h>
#include "bfgx_user_ctrl.h"
#include "plat_debug.h"
#include "plat_pm.h"
#include "bfgx_exception_rst.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "oal_kernel_file.h"
#include "oal_hcc_host_if.h"
#include "plat_pm_wlan.h"
#include "hisi_ini.h"
#include "plat_uart.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "oam_rdr.h"
#include "securec.h"
#include "bfgx_data_parse.h"
#include "pmu_rtc_interface.h"
#include "factory_mem.h"

typedef struct {
    int32_t cmd;
    int32_t (*exception_dbg_func)(uint32_t bus_id);
} dbg_cmd_t;

/* 全局变量定义 */
static struct kobject *g_sysfs_hi110x_bfgx = NULL;
static struct kobject *g_sysfs_hi110x_pmdbg = NULL;

#ifdef PLATFORM_DEBUG_ENABLE
static struct kobject *g_sysfs_hi110x_debug = NULL;
#endif

atomic_t g_hi110x_loglevel = ATOMIC_INIT(PLAT_LOG_INFO);
oal_module_symbol(g_hi110x_loglevel);

STATIC ssize_t show_wifi_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int32_t ret;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "wifi_pm_debug usage: \n"
                     " 1:dump host info 2:dump device info\n"
                     " 3:open wifi      4:close wifi \n"
                     " 5:enable pm      6:disable pm \n");
#else
    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "wifi_pm_debug usage: \n"
                     " 1:dump host info 2:dump device info\n");
#endif
    if (ret < 0) {
        return ret;
    }

    return ret;
}

STATIC ssize_t store_wifi_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    int input;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    input = oal_atoi(buf);

    if (pst_wlan_pm == NULL) {
        oal_io_print("pm_data is NULL!\n");
        return -FAILURE;
    }
    ps_print_info("%s->%d\n", __func__, input);
    /* case x => echo x->测试节点用 */
    switch (input) {
        case 1: /* cmd: 1 dump host info */
            wlan_pm_dump_host_info();
            break;
        case 2: /* cmd: 2 dump device info */
            wlan_pm_dump_device_info();
            break;
#ifdef PLATFORM_DEBUG_ENABLE
        case 3: /* cmd: 3 open wifi */
            wlan_pm_open();
            break;
        case 4: /* cmd: 4 close wifi */
            wlan_pm_close();
            break;
        case 5: /* cmd: 5 enalbe pm */
            wlan_pm_set_host_pm_switch(1);
            break;
        case 6: /* cmd: 6 disalbe pm */
            wlan_pm_set_host_pm_switch(0);
            break;
#endif
        default:
            break;
    }

    return count;
}

STATIC ssize_t show_bfgx_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "cmd       func     \n"
                      "  1  set pmdbg bfgx\n  2  set pmdbg gnss\n"
                      "  3  plat pm enable\n  4  plat pm disable\n"
                      "  5   bt  pm enable\n  6   bt  pm disable\n"
                      "  7  gnss pm enable\n  8  gnss pm disable\n"
                      "  9  pm ctrl enable\n  10 pm ctrl disable\n"
                      "  11 open bt\n  12 close bt\n"
                      "  13 open gnss\n  14 close gnss\n"
                      "  15 pm debug switch\n");
}

STATIC int32_t bfgx_pmdbg_plat_pm_en(struct ps_core_s *ps_core_d, uint8_t en)
{
    struct pm_drv_data *pm_data = ps_core_d->pm_data;
    int32_t prepare_result;

    if (ps_core_chk_bfgx_active(ps_core_d) != true) {
        ps_print_err("device not open\n");
        return -FAILURE;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    if (en) {
        pm_data->bfgx_lowpower_enable = BFGX_PM_ENABLE;
        ps_print_info("bfgx platform pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_PL_ENABLE_PM);
        mod_timer(&pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
        oal_atomic_inc(&pm_data->bfg_timer_mod_cnt);
    } else {
        pm_data->bfgx_lowpower_enable = BFGX_PM_DISABLE;
        ps_print_info("bfgx platform pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_PL_DISABLE_PM);
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

STATIC int32_t bfgx_pmdbg_bt_pm_en(struct ps_core_s *ps_core_d, uint8_t en)
{
    struct pm_drv_data *pm_data = ps_core_d->pm_data;
    int32_t prepare_result;

    if (ps_core_chk_bfgx_active(ps_core_d) != true) {
        ps_print_err("device not open\n");
        return -FAILURE;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    if (en) {
        pm_data->bfgx_bt_lowpower_enable = BFGX_PM_ENABLE;
        ps_print_info("bfgx bt pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_BT_ENABLE_PM);
    } else {
        pm_data->bfgx_bt_lowpower_enable = BFGX_PM_DISABLE;
        ps_print_info("bfgx bt pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_BT_DISABLE_PM);
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

STATIC int32_t bfgx_pmdbg_gnss_pm_en(struct ps_core_s *ps_core_d, uint8_t en)
{
    struct pm_drv_data *pm_data = ps_core_d->pm_data;
    int32_t prepare_result;

    if (ps_core_chk_bfgx_active(ps_core_d) != true) {
        ps_print_err("device not open\n");
        return -FAILURE;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    if (en) {
        pm_data->bfgx_gnss_lowpower_enable = BFGX_PM_ENABLE;
        ps_print_info("bfgx gnss pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_GNSS_ENABLE_PM);
    } else {
        pm_data->bfgx_gnss_lowpower_enable = BFGX_PM_DISABLE;
        ps_print_info("bfgx gnss pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_GNSS_DISABLE_PM);
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

STATIC int32_t bfgx_pmdbg_pm_ctrl_en(struct ps_core_s *ps_core_d, uint8_t en)
{
    struct pm_drv_data *pm_data = ps_core_d->pm_data;
    if (en) {
        pm_data->bfgx_pm_ctrl_enable = BFGX_PM_ENABLE;
    } else {
        pm_data->bfgx_pm_ctrl_enable = BFGX_PM_DISABLE;
    }
    return 0;
}

STATIC int32_t bfgx_pmdbg_bt_power(struct ps_core_s *ps_core_d, uint8_t en)
{
    int32_t prepare_result;
    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    if (en) {
        ps_print_info("open bt\n");
        ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_BT, BFG_POWER_GPIO_UP);
    } else {
        ps_print_info("close bt\n");
        ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_BT, BFG_POWER_GPIO_DOWN);
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

STATIC int32_t bfgx_pmdbg_gnss_power(struct ps_core_s *ps_core_d, uint8_t en)
{
    int32_t prepare_result;
    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    if (en) {
        ps_print_info("open gnss\n");
        ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_GNSS, BFG_POWER_GPIO_UP);
    } else {
        ps_print_info("close gnss\n");
        ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_GNSS, BFG_POWER_GPIO_DOWN);
    }

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

STATIC int32_t bfgx_pmdbg_plat_pm_debug(struct ps_core_s *ps_core_d)
{
    int32_t prepare_result;
    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    ps_tx_sys_cmd(ps_core_d, SYS_MSG, PL_PM_DEBUG);

    post_to_visit_node(ps_core_d, prepare_result);
    return 0;
}

int32_t g_test_uart = BUART;
STATIC ssize_t store_bfgx_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t cmd;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    cmd = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    ps_print_info("cmd:%d\n", cmd);

    ps_core_d = ps_get_core_reference(g_test_uart);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -FAILURE;
    }

    /* case x => echo x->测试节点用 */
    switch (cmd) {
        case 1: /* 1-set pmdbg bfgx */
            g_test_uart = BUART;
            ps_print_info("pmdbg from [%s] to [%s]\n", index2name(ps_core_d->pm_data->index), index2name(g_test_uart));
            break;
        case 2: /* 2-set pmdbg gnss */
            g_test_uart = GUART;
            ps_print_info("pmdbg from [%s] to [%s]\n", index2name(ps_core_d->pm_data->index), index2name(g_test_uart));
            break;
        case 3: /* 3-enable plat lowpower function */
            bfgx_pmdbg_plat_pm_en(ps_core_d, OAL_TRUE);
            break;
        case 4: /* 4-disable plat lowpower function */
            bfgx_pmdbg_plat_pm_en(ps_core_d, OAL_FALSE);
            break;
        case 5: /* 5-enable bt lowpower function */
            bfgx_pmdbg_bt_pm_en(ps_core_d, OAL_TRUE);
            break;
        case 6: /* 6-disable bt lowpower function */
            bfgx_pmdbg_bt_pm_en(ps_core_d, OAL_FALSE);
            break;
        case 7: /* 7-enable gnss lowpower function */
            bfgx_pmdbg_gnss_pm_en(ps_core_d, OAL_TRUE);
            break;
        case 8: /* 8-disable gnss lowpower function */
            bfgx_pmdbg_gnss_pm_en(ps_core_d, OAL_FALSE);
            break;
        case 9: /* 9-enable pm ctrl function */
            bfgx_pmdbg_pm_ctrl_en(ps_core_d, OAL_TRUE);
            break;
        case 10: /* 10-disable pm ctrl function */
            bfgx_pmdbg_pm_ctrl_en(ps_core_d, OAL_FALSE);
            break;
        case 11: /* 11-open bt */
            bfgx_pmdbg_bt_power(ps_core_d, OAL_TRUE);
            break;
        case 12: /* 12-close bt */
            bfgx_pmdbg_bt_power(ps_core_d, OAL_FALSE);
            break;
        case 13: /* 13-open gnss */
            bfgx_pmdbg_gnss_power(ps_core_d, OAL_TRUE);
            break;
        case 14: /* 14-close gnss */
            bfgx_pmdbg_gnss_power(ps_core_d, OAL_FALSE);
            break;
        case 15: /* 15-send cmd to dev */
            bfgx_pmdbg_plat_pm_debug(ps_core_d);
            break;
        default:
            ps_print_err("unknown cmd %d\n", cmd);
            break;
    }

    return count;
}

STATIC struct kobj_attribute g_wifi_pmdbg =
    __ATTR(wifi_pm, 0644, (void *)show_wifi_pmdbg, (void *)store_wifi_pmdbg);

STATIC struct kobj_attribute g_bfgx_pmdbg =
    __ATTR(bfgx_pm, 0644, (void *)show_bfgx_pmdbg, (void *)store_bfgx_pmdbg);

STATIC struct attribute *g_pmdbg_attrs[] = {
    &g_wifi_pmdbg.attr,
    &g_bfgx_pmdbg.attr,
    NULL,
};

STATIC struct attribute_group g_pmdbg_attr_grp = {
    .attrs = g_pmdbg_attrs,
};

STATIC ssize_t show_install(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}

/* read by octty from hal to decide open which uart */
STATIC ssize_t show_dev_name(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", pm_data->uart_name);
}

/* read by octty from hal to decide what baud rate to use */
STATIC ssize_t show_baud_rate(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%lu\n", pm_data->baud_rate);
}

/* read by octty from hal to decide whether or not use flow cntrl */
STATIC ssize_t show_flow_cntrl(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", pm_data->flow_cntrl);
}

/* show curr bfgx proto yes or not opened state */
STATIC ssize_t show_bfgx_active_state(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;
    uint8_t bt_state = POWER_STATE_SHUTDOWN;
    uint8_t fm_state = POWER_STATE_SHUTDOWN;
    uint8_t gnss_state = POWER_STATE_SHUTDOWN;
#ifdef HAVE_HISI_IR
    uint8_t ir_state = POWER_STATE_SHUTDOWN;
#endif

    ps_print_dbg("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EFAULT;
    }

    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_BT].subsys_state) == true) {
        bt_state = POWER_STATE_OPEN;
    }

    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_FM].subsys_state) == true) {
        fm_state = POWER_STATE_OPEN;
    }

    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_GNSS].subsys_state) == true) {
        gnss_state = POWER_STATE_OPEN;
    }

#ifdef HAVE_HISI_IR
    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_IR].subsys_state) == true) {
        ir_state = POWER_STATE_OPEN;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "bt:%u; fm:%u; gnss:%u; ir:%u;\n",
                      bt_state, fm_state, gnss_state, ir_state);
#else
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "bt:%u; fm:%u; gnss:%u;\n",
                      bt_state, fm_state, gnss_state);
#endif
}

STATIC ssize_t show_ini_file_name(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, INI_FILE_PATH_LEN, "%s", get_ini_file_name());
}

STATIC ssize_t show_exception_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return hisi_conn_save_stat_info(buf, plat_get_dfr_sinfo(buf, 0), PAGE_SIZE);
}

STATIC ssize_t show_exception_count(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    excp_cnt_t excp_cnt = {0};

    ps_print_dbg("exception debug: wifi rst count is %d\n", plat_get_excp_total_cnt(&excp_cnt));
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", plat_get_excp_total_cnt(&excp_cnt));
}

STATIC ssize_t show_loglevel(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
        "curr loglevel=%d\nerr:1\nwarning:2\nfunc|succ|info:3\ndebug:4\n", oal_atomic_read(&g_hi110x_loglevel));
}

STATIC ssize_t store_loglevel(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t loglevel;

    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (*buf == 'r') {
        ps_set_device_log_status(true);
        ps_print_info("enable device log\n");
        return count;
    } else if (*buf == 'R') {
        ps_set_device_log_status(false);
        ps_print_info("disable device log\n");
        return count;
    }

    loglevel = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    if (loglevel < PLAT_LOG_ALERT) {
        oal_atomic_set(&g_hi110x_loglevel, PLAT_LOG_ALERT);
    } else if (loglevel > PLAT_LOG_DEBUG) {
        oal_atomic_set(&g_hi110x_loglevel, PLAT_LOG_DEBUG);
    } else {
        oal_atomic_set(&g_hi110x_loglevel, loglevel);
    }

    return count;
}

#ifdef HAVE_HISI_IR
STATIC ssize_t show_ir_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (!g_st_board_info.have_ir) {
        ps_print_err("board have no ir module");
        return -FAILURE;
    }

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (g_st_board_info.irled_power_type == IR_GPIO_CTRL) {
        return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", gpio_get_value(g_st_board_info.bfgx_ir_ctrl_gpio));
    } else if (g_st_board_info.irled_power_type == IR_LDO_CTRL) {
        return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
                          (regulator_is_enabled(g_st_board_info.bfgn_ir_ctrl_ldo)) > 0 ? 1 : 0);
    }

    return -FAILURE;
}

STATIC ssize_t store_ir_mode(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t ir_ctrl_level;
    int ret;

    ps_print_info("into %s,irled_power_type is %d\n", __func__, g_st_board_info.irled_power_type);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (!g_st_board_info.have_ir) {
        ps_print_err("board have no ir module");
        return -FAILURE;
    }

    if (g_st_board_info.irled_power_type == IR_GPIO_CTRL) {
        ir_ctrl_level = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
        if (ir_ctrl_level == GPIO_LOWLEVEL) {
            gpio_direction_output(g_st_board_info.bfgx_ir_ctrl_gpio, GPIO_LOWLEVEL);
        } else if (ir_ctrl_level == GPIO_HIGHLEVEL) {
            gpio_direction_output(g_st_board_info.bfgx_ir_ctrl_gpio, GPIO_HIGHLEVEL);
        } else {
            ps_print_err("gpio level should be 0 or 1, cur value is [%d]\n", ir_ctrl_level);
            return -FAILURE;
        }
    } else if (g_st_board_info.irled_power_type == IR_LDO_CTRL) {
        if (IS_ERR(g_st_board_info.bfgn_ir_ctrl_ldo)) {
            ps_print_err("ir_ctrl get ird ldo failed\n");
            return -FAILURE;
        }

        ir_ctrl_level = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
        if (ir_ctrl_level == GPIO_LOWLEVEL) {
            ret = regulator_disable(g_st_board_info.bfgn_ir_ctrl_ldo);
            if (ret) {
                ps_print_err("ir_ctrl disable ldo failed\n");
            }
        } else if (ir_ctrl_level == GPIO_HIGHLEVEL) {
            ret = regulator_enable(g_st_board_info.bfgn_ir_ctrl_ldo);
            if (ret) {
                ps_print_err("ir_ctrl enable ldo failed\n");
            }
        } else {
            ps_print_err("ir_ctrl level should be 0 or 1, cur value is [%d]\n", ir_ctrl_level);
            return -FAILURE;
        }
    } else {
        ps_print_err("get ir_ldo_type error! ir_ldo_type is %d!\n", g_st_board_info.irled_power_type);
        return -FAILURE;
    }
    ps_print_info("set ir ctrl mode as %d!\n", (int)ir_ctrl_level);
    return count;
}
#endif

STATIC ssize_t bfgx_sleep_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", pm_data->bfgx_dev_state);
}

STATIC ssize_t bfgx_wkup_host_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", pm_data->bfg_wakeup_host);
}

#if defined(CONFIG_HISI_PMU_RTC_READCOUNT) || defined(CONFIG_PMU_RTC_READCOUNT)
STATIC ssize_t show_hisi_gnss_ext_rtc_count(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned long rtc_count;

    if ((kobj == NULL) || (attr == NULL) || (buf == NULL)) {
        ps_print_err("paramater is NULL\n");
        return 0;
    }

    rtc_count = external_pmu_rtc_readcount();
    ps_print_info("show_hisi_gnss_ext_rtc_count: %ld\n", rtc_count);
    if (memcpy_s(buf, PAGE_SIZE, (char *)&rtc_count, sizeof(rtc_count)) != EOK) {
        ps_print_err("memcpy_s error, destlen=%lu, srclen=%lu\n ", PAGE_SIZE, sizeof(rtc_count));
        return 0;
    }
    return sizeof(rtc_count);
}
#endif

STATIC struct kobj_attribute g_plat_exception_info =
    __ATTR(excp_info, 0444, (void *)show_exception_info, NULL);

#ifdef CONFIG_HI110X_GPS_SYNC
STATIC int32_t gnss_sync_convert_mode_modem0(int32_t rat_mode, uint32_t version)
{
    int32_t sync_mode = GNSS_SYNC_MODE_UNKNOWN;
    switch (rat_mode) {
        case GNSS_RAT_MODE_GSM:
            sync_mode = GNSS_SYNC_MODE_G1;
            break;
        case GNSS_RAT_MODE_WCDMA:
            sync_mode = GNSS_SYNC_MODE_PW;
            break;
        case GNSS_RAT_MODE_LTE:
            sync_mode = GNSS_SYNC_MODE_LTE;
            break;
        case GNSS_RAT_MODE_CDMA:
            sync_mode = GNSS_SYNC_MODE_CDMA;
            break;
        case GNSS_RAT_MODE_NR:
            if (version >= GNSS_SYNC_VERSION_5G) {
                sync_mode = GNSS_SYNC_MODE_NSTU;
            }
            break;
        default:
            sync_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }
    return sync_mode;
}

STATIC int32_t gnss_sync_convert_mode_modem1(int32_t rat_mode)
{
    int32_t sync_mode;
    switch (rat_mode) {
        case GNSS_RAT_MODE_GSM:
            sync_mode = GNSS_SYNC_MODE_G2;
            break;
        case GNSS_RAT_MODE_WCDMA:
            sync_mode = GNSS_SYNC_MODE_SW;
            break;
        case GNSS_RAT_MODE_LTE:
            sync_mode = GNSS_SYNC_MODE_LTE2;
            break;
        case GNSS_RAT_MODE_CDMA:
            sync_mode = GNSS_SYNC_MODE_CDMA;
            break;
        default:
            sync_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }
    return sync_mode;
}

STATIC int32_t gnss_sync_convert_mode_modem2(int32_t rat_mode)
{
    int32_t sync_mode;
    switch (rat_mode) {
        case GNSS_RAT_MODE_GSM:
            sync_mode = GNSS_SYNC_MODE_G3;
            break;
        case GNSS_RAT_MODE_CDMA:
            sync_mode = GNSS_SYNC_MODE_CDMA;
            break;
        default:
            sync_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }
    return sync_mode;
}

/*
 * Driver与Host接口定义为以下几个参数，每个参数之间用‘,’分割
 * "modem_id,rat_mode"
 */
STATIC int32_t gnss_sync_convert_mode(const char *rcv_data, int32_t *set_mode, uint32_t version)
{
    char *pc_split = ",";
    char *pc_str1 = (char *)rcv_data;
    char *pc_str2 = NULL;
    int32_t modem_id;
    int32_t rat_mode;

    if (pc_str1 == NULL || set_mode == NULL) {
        ps_print_err("received data or set mode is null\n");
        return -FAILURE;
    }

    modem_id = oal_atoi(pc_str1);
    if (modem_id < 0) {
        ps_print_err("modem id is invalid\n");
        return -FAILURE;
    }

    pc_str2 = oal_strstr(pc_str1, pc_split);
    if (pc_str2 == NULL) {
        ps_print_err("cannot find the split\n");
        return -FAILURE;
    }

    pc_str2++;
    rat_mode = oal_atoi(pc_str2);
    if (rat_mode < GNSS_RAT_MODE_NO_SERVICE || rat_mode >= GNSS_RAT_MODE_BUTT) {
        ps_print_err("rat mode is invalid\n");
        return -FAILURE;
    }

    /* 根据输入的值判断 */
    switch (modem_id) {
        case 0: /* 0->modem0 */
            *set_mode = gnss_sync_convert_mode_modem0(rat_mode, version);
            break;
        case 1: /* 1->modem1 */
            *set_mode = gnss_sync_convert_mode_modem1(rat_mode);
            break;
        case 2: /* 2->modem2 */
            *set_mode = gnss_sync_convert_mode_modem2(rat_mode);
            break;
        default:
            *set_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }

    return SUCCESS;
}

STATIC int32_t gnss_set_sync_mode_modem(int32_t set_mode)
{
    struct arm_smccc_res res = {0};

    if (set_mode == GNSS_SYNC_MODE_UNKNOWN) {
        return -FAILURE;
    }

    arm_smccc_smc(MODEM_SYNC_FUNC_ID, (uint64_t)set_mode, 0, 0, 0, 0, 0, 0, &res);
    return SUCCESS;
}

STATIC int32_t gnss_set_sync_mode_ap(struct gnss_sync_data *sync_data, int32_t set_mode)
{
    int32_t curr_mode = 0;

    if (sync_data->addr_base_virt == NULL) {
        ps_print_err("register virutal address is NULL!\n");
        return -FAILURE;
    }

    if (set_mode == GNSS_SYNC_MODE_UNKNOWN) {
        return -FAILURE;
    }

    curr_mode = readl(sync_data->addr_base_virt + sync_data->addr_offset);
    curr_mode = (int32_t)(((uint32_t)curr_mode & 0xfffffff0) | (uint32_t)set_mode); // only bit 3:0
    writel(curr_mode, (sync_data->addr_base_virt + sync_data->addr_offset));         // model selection

    return SUCCESS;
}

STATIC ssize_t show_hisi_gnss_sync_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int32_t mode;
    struct gnss_sync_data *sync_data = gnss_get_sync_data();

    ps_print_dbg("show hisi gnss sync mode!\n");

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (sync_data == NULL) {
        ps_print_err("sync_data is NULL!\n");
        return -FAILURE;
    }

    if (sync_data->addr_base_virt == NULL) {
        ps_print_err("register virutal address is NULL!\n");
        return -FAILURE;
    }

    mode = readl(sync_data->addr_base_virt + sync_data->addr_offset);

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", mode);
}

STATIC ssize_t store_hisi_gnss_sync_mode(struct kobject *kobj, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    int32_t ret;
    int32_t set_mode = GNSS_SYNC_MODE_UNKNOWN;
    struct gnss_sync_data *sync_data = NULL;

    ps_print_info("store hisi gnss sync mode!\n");

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    sync_data = gnss_get_sync_data();
    if (sync_data == NULL) {
        ps_print_err("sync_data is NULL!\n");
        return -EACCES;
    }

    if (sync_data->version == GNSS_SYNC_VERSION_OFF) {
        ps_print_info("sync version off!!\n");
        return -EACCES;
    } else if (sync_data->version >= GNSS_SYNC_VERSION_BUTT) {
        ps_print_warning("sync version %d has not been realized!\n", sync_data->version);
        return -FAILURE;
    }

    ret = gnss_sync_convert_mode(buf, &set_mode, sync_data->version);
    if (ret != SUCCESS) {
        return ret;
    }
    ps_print_info("gnss sync mode \"%s\" convert to %d", buf, set_mode);

    if (sync_data->version == GNSS_SYNC_VERSION_5G_EN) {
        ret = gnss_set_sync_mode_modem(set_mode);
        if (ret != SUCCESS) {
            return ret;
        }
        return count;
    }

    ret = gnss_set_sync_mode_ap(sync_data, set_mode);
    if (ret != SUCCESS) {
        return ret;
    }

    return count;
}
#endif

STATIC struct kobj_attribute g_ldisc_install =
    __ATTR(install, 0444, (void *)show_install, NULL);

STATIC struct kobj_attribute g_uart_dev_name =
    __ATTR(dev_name, 0444, (void *)show_dev_name, NULL);

STATIC struct kobj_attribute g_uart_baud_rate =
    __ATTR(baud_rate, 0444, (void *)show_baud_rate, NULL);

STATIC struct kobj_attribute g_uart_flow_cntrl =
    __ATTR(flow_cntrl, 0444, (void *)show_flow_cntrl, NULL);

STATIC struct kobj_attribute g_bfgx_active_state =
    __ATTR(bfgx_state, 0444, (void *)show_bfgx_active_state, NULL);

STATIC struct kobj_attribute g_hisi_ini_file_name =
    __ATTR(ini_file_name, 0444, (void *)show_ini_file_name, NULL);

STATIC struct kobj_attribute g_rst_count =
    __ATTR(rst_count, 0444, (void *)show_exception_count, NULL);

STATIC struct kobj_attribute g_bfgx_loglevel =
    __ATTR(loglevel, 0664, (void *)show_loglevel, (void *)store_loglevel);

#ifdef HAVE_HISI_IR
STATIC struct kobj_attribute g_bfgx_ir_ctrl =
    __ATTR(ir_ctrl, 0664, (void *)show_ir_mode, (void *)store_ir_mode);
#endif

#if defined(CONFIG_HISI_PMU_RTC_READCOUNT) || defined(CONFIG_PMU_RTC_READCOUNT)
STATIC struct kobj_attribute g_hisi_gnss_ext_rtc =
    __ATTR(gnss_ext_rtc, 0444, (void *)show_hisi_gnss_ext_rtc_count, NULL);
#endif

#ifdef CONFIG_HI110X_GPS_SYNC
STATIC struct kobj_attribute g_hisi_gnss_sync =
    __ATTR(gnss_sync, 0664, (void *)show_hisi_gnss_sync_mode, (void *)store_hisi_gnss_sync_mode);
#endif

STATIC struct kobj_attribute g_bfgx_sleep_attr =
    __ATTR(bfgx_sleep_state, 0444, (void *)bfgx_sleep_state_show, NULL);

STATIC struct kobj_attribute g_bfgx_wkup_host_count_attr =
    __ATTR(bfgx_wkup_host_count, 0444, (void *)bfgx_wkup_host_count_show, NULL);

STATIC struct attribute *g_bfgx_attrs[] = {
    &g_ldisc_install.attr,
    &g_uart_dev_name.attr,
    &g_uart_baud_rate.attr,
    &g_uart_flow_cntrl.attr,
    &g_bfgx_active_state.attr,
    &g_hisi_ini_file_name.attr,
    &g_rst_count.attr,
    &g_plat_exception_info.attr,
    &g_bfgx_loglevel.attr,
#ifdef HAVE_HISI_IR
    &g_bfgx_ir_ctrl.attr,
#endif
    &g_bfgx_sleep_attr.attr,
    &g_bfgx_wkup_host_count_attr.attr,
#if defined(CONFIG_HISI_PMU_RTC_READCOUNT) || defined(CONFIG_PMU_RTC_READCOUNT)
    &g_hisi_gnss_ext_rtc.attr,
#endif
#ifdef CONFIG_HI110X_GPS_SYNC
    &g_hisi_gnss_sync.attr,
#endif
    NULL,
};

STATIC struct attribute_group g_bfgx_attr_grp = {
    .attrs = g_bfgx_attrs,
};

#ifdef PLATFORM_DEBUG_ENABLE
STATIC ssize_t show_exception_dbg(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
                      "==========cmd  func             \n"
                      "   1  clear dfr info          \n"
                      "   2  cause bfgx beattimer timerout\n"
                      "   3  enable dfr subsystem rst\n"
                      "   4  disble dfr subsystem rst\n"
                      "   5  en dfr bfgx pwron fail\n"
                      "   6  en dfr bfgx pwroff fail\n"
                      "   7  en dfr wifi wkup fail\n"
                      "   8  cause bfgx panic\n"
                      "   9  cause bfgx arp exception system rst\n"
                      "   10 bfgx wkup fail\n"
                      "   11 wifi WIFI_TRANS_FAIL\n"
                      "   12 wifi WIFI_POWER ON FAIL\n");
}

STATIC int32_t dfr_info_clear(uint32_t bus_id)
{
    struct st_exception_info *pst_exception_data = NULL;

    oal_reference(bus_id);

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]clear dfr info\n");
    memset_s(&pst_exception_data->etype_info, sizeof(pst_exception_data->etype_info),
             0, sizeof(pst_exception_data->etype_info));
    return OAL_SUCC;
}

STATIC int32_t bfgx_timeout_dbg(uint32_t bus_id)
{
    int32_t prepare_result;
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = ps_get_core_reference(bus_id);
    if (ps_core_d == NULL) {
        ps_print_err("[dfr_test]ps_core_d is NULL\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]bfgx device timeout cause ++\n");
    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("[dfr_test]prepare work FAIL\n");
        return OAL_FAIL;
    }

    set_excp_test_en(HEARTBEATER_TIMEOUT_FAULT, (uintptr_t)ps_core_d);

    post_to_visit_node(ps_core_d, prepare_result);
    ps_print_info("[dfr_test]bfgx device timeout cause --\n");
    return OAL_SUCC;
}

STATIC int32_t dfr_subsystem_rst_enable(uint32_t bus_id)
{
    struct st_exception_info *pst_exception_data = NULL;

    oal_reference(bus_id);

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]enable dfr subsystem rst\n");
    pst_exception_data->subsystem_rst_en = DFR_TEST_ENABLE;
    return OAL_SUCC;
}

STATIC int32_t dfr_subsystem_rst_disable(uint32_t bus_id)
{
    struct st_exception_info *pst_exception_data = NULL;

    oal_reference(bus_id);

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]disable dfr subsystem rst\n");
    pst_exception_data->subsystem_rst_en = DFR_TEST_DISABLE;
    return OAL_SUCC;
}

STATIC int32_t bfgx_power_on_fail_test(uint32_t bus_id)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = ps_get_core_reference(bus_id);
    if (ps_core_d == NULL) {
        ps_print_err("[dfr_test]ps_core_d is NULL\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]bfgx powon fail dfr test en, next poweron will fail\n");
    set_excp_test_en(BFGX_POWEON_FAULT, (uintptr_t)ps_core_d);
    return OAL_SUCC;
}

STATIC int32_t bfgx_power_off_fail_test(uint32_t bus_id)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = ps_get_core_reference(bus_id);
    if (ps_core_d == NULL) {
        ps_print_err("[dfr_test]ps_core_d is NULL\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]bfgx pwr off dfr test en, next poweroff will fail\n");
    set_excp_test_en(BFGX_POWEOFF_FAULT, (uintptr_t)ps_core_d);
    return OAL_SUCC;
}

STATIC int32_t wifi_wkup_fail_test(uint32_t bus_id)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    struct hcc_handler *pst_hcc = hcc_get_handler(HCC_EP_WIFI_DEV);

    oal_reference(bus_id);

    if (pst_hcc == NULL) {
        ps_print_info("pst_hcc is null\n");
        return OAL_FAIL;
    }
    if (oal_atomic_read(&pst_hcc->state) != HCC_ON) {
        ps_print_info("wifi is closed\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]wifi wkup dfr test en, next wkup will fail\n");
    if (pst_wlan_pm->wlan_dev_state == HOST_ALLOW_TO_SLEEP) {
        hcc_tx_transfer_lock(hcc_get_handler(HCC_EP_WIFI_DEV));
        set_excp_test_en(WIFI_WKUP_FAULT, (uintptr_t)hcc_get_handler(HCC_EP_WIFI_DEV));
        wlan_pm_wakeup_dev();
        hcc_tx_transfer_unlock(hcc_get_handler(HCC_EP_WIFI_DEV));
    } else {
        ps_print_warning("[dfr_test]wifi wkup dfr trigger fail, cur state:%ld\n",
                         pst_wlan_pm->wlan_dev_state);
    }

    return OAL_SUCC;
}

STATIC int32_t bfgx_device_panic(uint32_t bus_id)
{
    int32_t prepare_result;
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = ps_get_core_reference(bus_id);
    if (ps_core_d == NULL) {
        ps_print_err("[dfr_test]ps_core_d is NULL\n");
        return OAL_FAIL;
    }

    if (prepare_to_visit_node(ps_core_d, &prepare_result) < 0) {
        ps_print_err("[dfr_test]prepare work FAIL\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]bfgx device panic cause\n");
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_DEV_PANIC);
    post_to_visit_node(ps_core_d, prepare_result);
    return OAL_SUCC;
}

STATIC int32_t bfgx_arp_timeout(uint32_t bus_id)
{
    oal_reference(bus_id);

    ps_print_info("[dfr_test]trigger hal BFGX_ARP_TIMEOUT exception\n");
    plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, BFGX_ARP_TIMEOUT);
    return OAL_SUCC;
}

STATIC int32_t bfgx_wkup_fail_test(uint32_t bus_id)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = ps_get_core_reference(bus_id);
    if (ps_core_d == NULL) {
        ps_print_err("[dfr_test]ps_core_d is NULL\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]bfgx wkup dfr test en, next wkup will fail \n");
    set_excp_test_en(BFGX_WKUP_FAULT, (uintptr_t)ps_core_d);
    return OAL_SUCC;
}

STATIC int32_t wifi_trans_fail_test(uint32_t bus_id)
{
    hcc_bus *hi_bus = hcc_get_bus(HCC_EP_WIFI_DEV);
    struct hcc_handler *pst_hcc = hcc_get_handler(HCC_EP_WIFI_DEV);

    oal_reference(bus_id);

    if (pst_hcc == NULL || hi_bus == NULL) {
        ps_print_info("pst_hcc or hi_bus is null\n");
        return OAL_FAIL;
    }

    if (oal_atomic_read(&pst_hcc->state) != HCC_ON) {
        ps_print_info("wifi is closed\n");
        return OAL_FAIL;
    }

    ps_print_info("[dfr_test]wifi WIFI_TRANS_FAIL submit \n");
    hcc_tx_transfer_lock(pst_hcc);
    if (hcc_bus_pm_wakeup_device(hi_bus) == OAL_SUCC) {
        hcc_bus_exception_submit(hi_bus, HCC_BUS_TRANS_FAIL);
    }

    hcc_tx_transfer_unlock(pst_hcc);
    return OAL_SUCC;
}

STATIC int32_t wifi_power_on_fail_test(uint32_t bus_id)
{
    oal_reference(bus_id);

    ps_print_info("[dfr_test]wifi WIFI_power on failed\n");
    set_excp_test_en(WIFI_POWER_ON_FAULT, (uintptr_t)hcc_get_handler(HCC_EP_WIFI_DEV));
    return OAL_SUCC;
}

STATIC int32_t dfr_sys_rst_download_fail_test(uint32_t bus_id)
{
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    oal_reference(bus_id);

    ps_print_info("[dfr_test]trigger dfr_sys_rst_download_fail_test\n");

    pst_exception_data->sys_rst_download_dbg = OAL_TRUE;
    plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, BFGX_ARP_TIMEOUT);
    return OAL_SUCC;
}

STATIC int32_t dfr_sys_rst_uart_suspend_test(uint32_t bus_id)
{
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    oal_reference(bus_id);

    ps_print_info("[dfr_test]trigger sys_rst_uart_suspend_test\n");

    pst_exception_data->sys_rst_uart_suspend = OAL_TRUE;

    return OAL_SUCC;
}

STATIC int32_t dfr_sys_rst_uart_suspend_clean(uint32_t bus_id)
{
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (oal_unlikely(pst_exception_data == NULL)) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return OAL_FAIL;
    }

    oal_reference(bus_id);

    ps_print_info("[dfr_test]sys_rst_uart_suspend_clean\n");

    pst_exception_data->sys_rst_uart_suspend = OAL_FALSE;

    return OAL_SUCC;
}


dbg_cmd_t g_exception_dbg[] = {
    {1, dfr_info_clear},            /* cmd: 1, clear dfr info */
    {2, bfgx_timeout_dbg},          /* cmd: 2, trigger bfgx heartbeat timeout */
    {3, dfr_subsystem_rst_enable},  /* cmd: 3, enable dfr subsystem rst */
    {4, dfr_subsystem_rst_disable}, /* cmd: 4, disable dfr subsystem rst */
    {5, bfgx_power_on_fail_test},   /* cmd: 5, trigger bfgx_power_on_fail */
    {6, bfgx_power_off_fail_test},  /* cmd: 6, trigger bfgx_power_off_fail */
    {7, wifi_wkup_fail_test},       /* cmd: 7, trigger wifi_wkup_fail */
    {8, bfgx_device_panic},         /* cmd: 8, trigger bfgx_device_panic */
    {9, bfgx_arp_timeout},          /* cmd: 9, trigger bfgx_arp_timeout excp, dfr_system_rst */
    {10, bfgx_wkup_fail_test},      /* cmd: 10, trigger bfgx_wkup_fail */
    {11, wifi_trans_fail_test},     /* cmd: 11, trigger wifi_trans_fail */
    {12, wifi_power_on_fail_test},  /* cmd: 12, trigger wifi_power_on_fail */
    {13, dfr_sys_rst_download_fail_test}, /* cmd: 13, trigger dfr_sys_rst_download_fail */
    {14, dfr_sys_rst_uart_suspend_test},  /* cmd: 14, 模拟uart_suspend场景，构造异常，进入dfr下电流程 */
    {15, dfr_sys_rst_uart_suspend_clean}  /* cmd: 15, 清理uart_suspend标志位 */
};

STATIC uint32_t find_bus_id(uint8_t *subsys)
{
    int32_t i;
    struct pm_top* top = pm_get_top();

    for (i = 0; i < BFGX_BUTT; i++) {
        if (oal_strcmp(top->srv_desc[i].name, subsys) == 0)  {
            return service_get_bus_id(i);
        }
    }

    return UART_BUTT;
}

STATIC int32_t exception_dbg_proc(uint32_t bus_id, int32_t cmd)
{
    if (cmd > oal_array_size(g_exception_dbg) || cmd < 1) {
        ps_print_err("[dfr_test] unknown cmd\n");
        return OAL_FAIL;
    }

    return g_exception_dbg[cmd - 1].exception_dbg_func(bus_id);
}

STATIC ssize_t store_exception_dbg(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t cmd;
    uint32_t bus_id;
    uint8_t subsys[SUBSYS_NAME_SIZE] = {0};

    if (buf == NULL) {
        ps_print_err("[dfr_test]buf or hi_bus is NULL\n");
        return -FAILURE;
    }

    if ((sscanf_s(buf, "%s %d", subsys, SUBSYS_NAME_SIZE, &cmd) != 2)) { /* 2为入参个数检查 */
        ps_print_err("param input is error"NEWLINE);
        return -FAILURE;
    }

    ps_print_err("[dfr_test]subsys=%s, cmd=%d"NEWLINE, subsys, cmd);
    bus_id = find_bus_id(subsys);
    if (exception_dbg_proc(bus_id, cmd) != OAL_SUCC) {
        return -FAILURE;
    }

    return count;
}

STATIC ssize_t show_dev_test(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
                      "cmd  func\n  1  cause bfgx panic\n  2  enable exception recovery\n  3  enable wifi open bcpu\n"
                      "  4  pull up power gpio\n  5  pull down power gpio\n  6  uart loop test\n"
                      "  7  wifi download test\n  8  open uart\n  9  close uart\n"
                      "  10 open bt\n  11 bfg download test\n  12 bfg wifi download test\n"
                      "  13 wlan_pm_open_bcpu\n  14 wlan_power_on \n  15 wlan_power_off\n"
                      "  16 wlan_pm_open\n  17 wlan_pm_close\n  18 bfg gpio power off\n"
                      "  19 bfg gpio power on\n  20 gnss monitor enable\n"
                      "  21 rdr test\n  22 bfgx power on\n");
}

oal_uint wlan_pm_open_bcpu(void);

STATIC ssize_t store_dev_test(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t cmd;
    int32_t ret, prepare_result;
    uint8_t send_data[] = { 0x7e, 0x0, 0x06, 0x0, 0x0, 0x7e };
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;
    mpxx_board_info *bd_info = NULL;

    ps_print_info("%s\n", __func__);

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return BFGX_POWER_FAILED;
    }

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return 0;
    }

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    cmd = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    /* case x => echo x->测试节点用 */
    switch (cmd) {
        case 1: /* cmd: 1 cause bfgx panic */
            ret = prepare_to_visit_node(ps_core_d, &prepare_result);
            if (ret < 0) {
                ps_print_err("prepare work FAIL\n");
                return ret;
            }

            ps_print_info("bfgx test cmd %d, cause device panic\n", cmd);
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_DEV_PANIC);

            post_to_visit_node(ps_core_d, prepare_result);
            break;
        case 2: /* cmd: 2 enable exception recovery */
            ps_print_info("cmd %d,enable platform dfr\n", cmd);
            plat_beat_timeout_reset_set(OAL_TRUE);
            break;
        case 3: /* cmd: 3 enable wifi open bcpu */
            ps_print_info("cmd %d,enable wifi open bcpu\n", cmd);
            break;
        case 4: /* cmd: 4 pull up power gpio */
            ps_print_info("cmd %d,test pull up power gpio\n", cmd);
            bd_info->bd_ops.board_power_on(W_SYS);
            break;
        case 5: /* cmd: 5 pull down power gpio */
            ps_print_info("cmd %d,test pull down power gpio\n", cmd);
            bd_info->bd_ops.board_power_off(W_SYS);
            break;
        case 6: /* cmd: 6 uart loop test */
            ps_print_info("cmd %d,start uart loop test\n", cmd);
            uart_loop_test(ps_core_d);
            break;
        case 7: /* cmd: 7 wifi download test */
            ps_print_info("cmd %d,firmware download wifi_cfg\n", cmd);
            firmware_download_function(WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
            break;
        case 8: /* cmd: 8 open uart */
            ps_print_info("cmd %d,open uart\n", cmd);
            open_tty_drv(ps_core_d);
            break;
        case 9: /* cmd: 9 close uart */
            ps_print_info("cmd %d,close uart\n", cmd);
            release_tty_drv(ps_core_d);
            break;
        case 10: /* cmd: 10 open bt */
            ps_print_info("cmd %d,uart cmd test\n", cmd);
            ps_write_tty(ps_core_d, send_data, sizeof(send_data));
            break;
        case 11: /* cmd: 11 bfg download test */
            ps_print_info("cmd %d,firmware download bfgx_cfg\n", cmd);
            firmware_download_function(BFGX_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
            break;
        case 12: /* cmd: 12 bfg wifi download test */
            ps_print_info("cmd %d,firmware download bfgx_and_wifi cfg\n", cmd);
            firmware_download_function(BFGX_AND_WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
            break;
        case 13: /* cmd: 13 wlan_pm_open_bcpu */
            ps_print_info("cmd %d,wlan_pm_open_bcpu\n", cmd);
            wlan_pm_open_bcpu();
            break;
        case 14: /* cmd: 14 wlan_power_on */
            ps_print_info("cmd %d,mp13_wlan_power_on\n", cmd);
            bd_info->bd_ops.wlan_power_on();
            break;
        case 15: /* cmd: 15 wlan_power_off */
            ps_print_info("cmd %d,mp13_wlan_power_off\n", cmd);
            bd_info->bd_ops.wlan_power_off();
            break;
        case 16: /* cmd: 16 wlan_pm_open */
            ps_print_info("cmd %d,wlan_pm_open\n", cmd);
            wlan_pm_open();
            break;
        case 17: /* cmd: 17 wlan_pm_close */
            ps_print_info("cmd %d,wlan_pm_close\n", cmd);
            wlan_pm_close();
            break;
        case 18: /* cmd: 18 bfg gpio power off */
            ps_print_info("cmd %d,bfgx gpio power off\n", cmd);
            bd_info->bd_ops.board_power_off(B_SYS);
            break;
        case 19: /* cmd: 19 bfg gpio power on */
            ps_print_info("cmd %d,bfgx gpio power on\n", cmd);
            bd_info->bd_ops.board_power_on(B_SYS);
            break;
        case 20: /* cmd: 20 gnss monitor enable */
            ps_print_info("cmd %d,g_device_monitor_enable set to 1\n", cmd);
            g_device_monitor_enable = 1;
            break;
        case 21: /* cmd: 21 rdr test */
            rdr_exception(MODID_CONN_WIFI_EXEC);
            rdr_exception(MODID_CONN_WIFI_CHAN_EXEC);
            rdr_exception(MODID_CONN_WIFI_WAKEUP_FAIL);
            rdr_exception(MODID_CONN_BFGX_EXEC);
            rdr_exception(MODID_CONN_BFGX_BEAT_TIMEOUT);
            rdr_exception(MODID_CONN_BFGX_WAKEUP_FAIL);
            break;
        case 22: /* cmd: 22 bfgx power on */
            ps_print_err("bfgx power on %d\n", cmd);
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_BT, BFG_POWER_GPIO_UP);
            break;
        default:
            ps_print_err("unknown cmd %d\n", cmd);
            break;
    }

    return count;
}

STATIC ssize_t store_factory_test(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t ret;
    int32_t cmd;
    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    cmd = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    switch (cmd) {
        case 1:
            ret = device_gt_mem_check();
            if (ret != OAL_SUCC) {
                ps_print_err("gt_memory_test fail!\n");
                return -FAILURE;
            }
            ps_print_err("gt_memory_test succ!\n");
            break;
        default:
            ps_print_err("unknown cmd %d\n", cmd);
            break;
    }
    return count;
}

STATIC struct kobj_attribute g_plat_exception_dbg =
    __ATTR(exception_dbg, 0644, (void *)show_exception_dbg, (void *)store_exception_dbg);

STATIC struct kobj_attribute g_bfgx_dev_test =
    __ATTR(bfgx_test, 0644, (void *)show_dev_test, (void *)store_dev_test);

STATIC struct kobj_attribute g_factory_test =
    __ATTR(factory_test, 0220, NULL, (void *)store_factory_test);

STATIC struct attribute *g_mpxx_debug_attrs[] = {
    &g_plat_exception_dbg.attr,
    &g_bfgx_dev_test.attr,
    &g_factory_test.attr,
    NULL,
};

STATIC struct attribute_group g_mpxx_debug_attr_grp = {
    .attrs = g_mpxx_debug_attrs,
};
#endif

int32_t bfgx_user_ctrl_init(void)
{
    int status;
    struct kobject *pst_root_object = NULL;

    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        ps_print_err("[E]get root sysfs object failed!\n");
        return -EFAULT;
    }

    g_sysfs_hi110x_pmdbg = kobject_create_and_add("pmdbg", pst_root_object);
    if (g_sysfs_hi110x_pmdbg == NULL) {
        ps_print_err("Failed to creat sysfs_hisi_pmdbg !!!\n ");
        goto fail_g_sysfs_hisi_pmdbg;
    }

    status = oal_debug_sysfs_create_group(g_sysfs_hi110x_pmdbg, &g_pmdbg_attr_grp);
    if (status) {
        ps_print_err("failed to create g_sysfs_hi110x_pmdbg sysfs entries\n");
        goto fail_create_pmdbg_group;
    }

    g_sysfs_hi110x_bfgx = kobject_create_and_add("hi110x_ps", NULL);
    if (g_sysfs_hi110x_bfgx == NULL) {
        ps_print_err("Failed to creat sysfs_hi110x_ps !!!\n ");
        goto fail_g_sysfs_hi110x_bfgx;
    }

    status = sysfs_create_group(g_sysfs_hi110x_bfgx, &g_bfgx_attr_grp);
    if (status) {
        ps_print_err("failed to create g_sysfs_hi110x_bfgx sysfs entries\n");
        goto fail_create_bfgx_group;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    g_sysfs_hi110x_debug = kobject_create_and_add("hi110x_debug", NULL);
    if (g_sysfs_hi110x_debug == NULL) {
        ps_print_err("Failed to creat sysfs_hi110x_debug !!!\n ");
        goto fail_g_sysfs_hi110x_debug;
    }

    status = oal_debug_sysfs_create_group(g_sysfs_hi110x_debug, &g_mpxx_debug_attr_grp);
    if (status) {
        ps_print_err("failed to create g_sysfs_hi110x_debug sysfs entries\n");
        goto fail_create_hi110x_debug_group;
    }
#endif

    return 0;

#ifdef PLATFORM_DEBUG_ENABLE
fail_create_hi110x_debug_group:
    kobject_put(g_sysfs_hi110x_debug);
fail_g_sysfs_hi110x_debug:
#endif
    sysfs_remove_group(g_sysfs_hi110x_bfgx, &g_bfgx_attr_grp);
fail_create_bfgx_group:
    kobject_put(g_sysfs_hi110x_bfgx);
fail_g_sysfs_hi110x_bfgx:
    oal_debug_sysfs_remove_group(g_sysfs_hi110x_pmdbg, &g_pmdbg_attr_grp);
fail_create_pmdbg_group:
    kobject_put(g_sysfs_hi110x_pmdbg);
fail_g_sysfs_hisi_pmdbg:
    return -EFAULT;
}

void bfgx_user_ctrl_exit(void)
{
#ifdef PLATFORM_DEBUG_ENABLE
    oal_debug_sysfs_remove_group(g_sysfs_hi110x_debug, &g_mpxx_debug_attr_grp);
    kobject_put(g_sysfs_hi110x_debug);
#endif

    sysfs_remove_group(g_sysfs_hi110x_bfgx, &g_bfgx_attr_grp);
    kobject_put(g_sysfs_hi110x_bfgx);

    oal_debug_sysfs_remove_group(g_sysfs_hi110x_pmdbg, &g_pmdbg_attr_grp);
    kobject_put(g_sysfs_hi110x_pmdbg);
}
