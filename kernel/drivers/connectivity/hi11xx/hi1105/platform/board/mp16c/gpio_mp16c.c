/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: MP16c gpio board-level implementation.
 * Author: @CompanyNameTag
 */

#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#ifdef CONFIG_HUAWEI_LOW_POWER
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#endif
#include "oal_hcc_bus.h"
#include "plat_debug.h"
#include "plat_gpio.h"
#include "oal_ext_if.h"
#include "plat_pm.h"
#include "plat_common_clk.h"
#include "plat_exception_rst.h"
#include "chr_user.h"
#include "bfgx_core.h"
#include "plat_pm_wlan.h"
#include "gpio_mp16c.h"

#define SPDT_GPIO_NUM 2
#define PA_GPIO_NUM   2

// gpio类型
typedef enum  {
    SPDT_VC1,
    SPDT_VC2,
    BBST_EN,
    BBST_VOUT,
    CLK38P4_PWR,
    WB_SWT,
    BBST_MOS_EN, // MOS EN FOR H+
    ANT_SW1_VC1,
    ANT_SW2_VC1,
    ANT_SW2_VC2,
    IO_MAX
} gpio_t;

// 特性开关
typedef enum  {
    SW_OFF,
    SW_ON
} switch_t;

// 特性类型
typedef enum  {
    TYPE_SPDT,
    TYPE_PA,
    TYPE_RSMC,
    TYPE_CLK,
    TYPE_QUERY,
    TYPE_FAC,
    TYPE_W_POWER
} feature_t;

struct gpio_gugong {
    char *dts_type;
    char *gpio_name;
    int32_t number;
    uint32_t init_mode;
    int32_t inited;
};

struct gugong_status_info {
    uint8_t gugong_cmd_config;
    uint8_t gugong_status_done;
    uint8_t gugong_factory_status;
    uint8_t gugong_on_last_entered;
    uint8_t gugong_off_last_entered;
};

STATIC void gugong_gpio_reinit(int32_t gpio_num);
STATIC struct gugong_status_info g_gugong_status_info;
STATIC plat_clk_mode_str g_clk_mod;

STATIC void gugong_status_info_init(void)
{
    g_gugong_status_info.gugong_cmd_config = OAL_FALSE;
    g_gugong_status_info.gugong_status_done = OAL_FALSE;
    g_gugong_status_info.gugong_factory_status = OAL_FALSE;
    g_gugong_status_info.gugong_on_last_entered = OAL_FALSE;
    g_gugong_status_info.gugong_off_last_entered = OAL_FALSE;

    g_clk_mod.as_dword = board_clk_mode();

    ps_print_info("gugong_status_info init\n");
}

STATIC struct gpio_gugong g_gpios_gugong[IO_MAX] = {
    {
        .dts_type = "spdt_vc1",
        .gpio_name = "gpio-spdt_vc1"
    },
    {
        .dts_type = "spdt_vc2",
        .gpio_name = "gpio-spdt_vc2"
    },
    {
        .dts_type = "bbst_en",
        .gpio_name = "gpio-bbst_en"
    },
    {
        .dts_type = "bbst_vout",
        .gpio_name = "gpio-bbst_vout"
    },
    {
        .dts_type = "clk38p4pwr",
        .gpio_name = "gpio-38p4pwr"
    },
    {
        .dts_type = "wbswt",
        .gpio_name = "gpio-swt"
    },
    {
        .dts_type = "bbst_mos_en",
        .gpio_name = "gpio-bbst_mos_en"
    },
    {
        .dts_type = "ant_sw1_vc1",
        .gpio_name = "gpio-ant_sw1_vc1"
    },
    {
        .dts_type = "ant_sw2_vc1",
        .gpio_name = "gpio-ant_sw2_vc1"
    },
    {
        .dts_type = "ant_sw2_vc2",
        .gpio_name = "gpio-ant_sw2_vc2"
    }
};

typedef enum {
    VOL_TYPE_EXIT = 0,
    VOL_TYPE_WORK,
    VOL_BUF
} vol_t;

uint32_t g_gugong_bbst_vout[VOL_BUF] = {0};

STATIC void gugong_gpio_init(struct device_node *of_node)
{
    int32_t num_gpios, index_gpio, index_dts, ret;
    const char *pin_name = NULL;

    num_gpios = of_gpio_named_count(of_node, "gugong-gpios");
    if (num_gpios <= 0) {
        ps_print_info("gugong not config gpios");
        return;
    }

    ps_print_info("gugong product gpios[%d]\n", num_gpios);

    for (index_gpio = 0, index_dts = 0; index_gpio < IO_MAX; index_gpio++) {
        if (index_dts >= num_gpios) {
            break;
        }

        ret = of_property_read_string_index(of_node, "gugong,gpio-types",
                                            index_dts, &pin_name);
        if (ret < 0) {
            ps_print_err("get gugong gpio types fail\n");
            return;
        }

        if (oal_strcmp(pin_name, g_gpios_gugong[index_gpio].dts_type) != 0) {
            ps_print_info("gugong dts [%s] not match [%s]", pin_name, g_gpios_gugong[index_gpio].dts_type);
            continue;
        }

        g_gpios_gugong[index_gpio].number = of_get_named_gpio_flags(of_node, "gugong-gpios", index_dts,
            &g_gpios_gugong[index_gpio].init_mode);
        index_dts++;

        ps_print_info("gugong gpio name %s-%d, init_mode:%u", g_gpios_gugong[index_gpio].gpio_name,
                      g_gpios_gugong[index_gpio].number, g_gpios_gugong[index_gpio].init_mode);
        ret = gpio_request_one(g_gpios_gugong[index_gpio].number,
            (g_gpios_gugong[index_gpio].init_mode == 0) ? GPIOF_OUT_INIT_LOW : GPIOF_OUT_INIT_HIGH,
            g_gpios_gugong[index_gpio].gpio_name);
        if (ret < 0) {
            ps_print_info("gugong gpio [%d], name [%s], request fail ret %d\n",
                g_gpios_gugong[index_gpio].number, g_gpios_gugong[index_gpio].gpio_name, ret);
            // 与蒲公英共gpio，申请失败后不返回
            if (index_gpio != ANT_SW1_VC1 && index_gpio != ANT_SW2_VC1 && index_gpio != ANT_SW2_VC2) {
                gugong_gpio_reinit(index_gpio);
                return;
            }
        }

        g_gpios_gugong[index_gpio].inited = OAL_TRUE;
    }
}

STATIC void gugong_voltage_init(struct device_node *of_node)
{
    if (of_property_read_u32_array(of_node, "gugong-bbst-voltage", g_gugong_bbst_vout, VOL_BUF)) {
        return;
    }
    ps_print_info("gugong voltage exit:%u,work:%u\n",
        g_gugong_bbst_vout[VOL_TYPE_EXIT], g_gugong_bbst_vout[VOL_TYPE_WORK]);
}

STATIC void gugong_spdt_gpio_config(uint32_t sub_type)
{
    if ((g_gpios_gugong[SPDT_VC1].inited == OAL_FALSE) || (g_gpios_gugong[SPDT_VC2].inited == OAL_FALSE)) {
        ps_print_err("gugong gpio[SPDT_VC1] init status is %u, gpio[SPDT_VC2] init status is %u!\n",
                     g_gpios_gugong[SPDT_VC1].inited, g_gpios_gugong[SPDT_VC2].inited);
        return;
    }

    ps_print_info("gugong spdt set type[%u], vc1 init [%d] vc2 init[%d]",
                  sub_type, g_gpios_gugong[SPDT_VC1].init_mode, g_gpios_gugong[SPDT_VC2].init_mode);

    if (sub_type == SW_OFF) {
        // gugong off
        gpio_direction_output(g_gpios_gugong[SPDT_VC1].number,
            (g_gpios_gugong[SPDT_VC1].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
        gpio_direction_output(g_gpios_gugong[SPDT_VC2].number,
            (g_gpios_gugong[SPDT_VC2].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
    } else if (sub_type == SW_ON) {
        // gugong on
        gpio_direction_output(g_gpios_gugong[SPDT_VC2].number,
            (g_gpios_gugong[SPDT_VC2].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
        gpio_direction_output(g_gpios_gugong[SPDT_VC1].number,
            (g_gpios_gugong[SPDT_VC1].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    }
}

STATIC void gugong_antsw_gpio_config(uint32_t sub_type)
{
    if ((g_gpios_gugong[ANT_SW1_VC1].inited == OAL_FALSE) || (g_gpios_gugong[ANT_SW2_VC1].inited == OAL_FALSE) ||
        (g_gpios_gugong[ANT_SW2_VC2].inited == OAL_FALSE)) {
        return;
    }

    ps_print_info("gugong_antsw_gpio_config sub_type[%u]", sub_type);
    // config GPIO 219/351/332 when gugong off or gugong on
    if (sub_type == SW_OFF) {
        gpio_direction_output(g_gpios_gugong[ANT_SW1_VC1].number, GPIO_LEVEL_HIGH);
        gpio_direction_output(g_gpios_gugong[ANT_SW2_VC1].number, GPIO_LEVEL_HIGH);
        gpio_direction_output(g_gpios_gugong[ANT_SW2_VC2].number, GPIO_LEVEL_LOW);
    } else if (sub_type == SW_ON) {
        gpio_direction_output(g_gpios_gugong[ANT_SW1_VC1].number, GPIO_LEVEL_HIGH);
        gpio_direction_output(g_gpios_gugong[ANT_SW2_VC1].number, GPIO_LEVEL_LOW);
        gpio_direction_output(g_gpios_gugong[ANT_SW2_VC2].number, GPIO_LEVEL_HIGH);
    } else {
        ps_print_warning("invalid sub_type!\n");
    }
}

STATIC void gugong_pa_gpio_off(void)
{
    // GPIO 204 CFG FOR PA MOSEN
    if (g_gpios_gugong[BBST_MOS_EN].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_MOS_EN].number,
            (g_gpios_gugong[BBST_MOS_EN].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
    }

    if (g_gpios_gugong[BBST_EN].inited == OAL_TRUE &&
        g_gpios_gugong[BBST_VOUT].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_VOUT].number,
            (g_gpios_gugong[BBST_VOUT].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
        gpio_direction_output(g_gpios_gugong[BBST_EN].number,
            (g_gpios_gugong[BBST_EN].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
    } else if (g_gpios_gugong[BBST_EN].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_EN].number,
            (g_gpios_gugong[BBST_EN].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
    } else if (g_gpios_gugong[BBST_VOUT].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_VOUT].number,
            (g_gpios_gugong[BBST_VOUT].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
    } else {
#ifdef CONFIG_HUAWEI_LOW_POWER
        buck_boost_set_vout(g_gugong_bbst_vout[VOL_TYPE_EXIT], BBST_USER_BOOST_RSMC);
        buck_boost_set_enable(GPIO_LEVEL_LOW, BBST_USER_BOOST_RSMC);
#endif
    }
}

STATIC void gugong_pa_gpio_on(void)
{
    // GPIO 204 CFG FOR PA MOSEN
    if (g_gpios_gugong[BBST_MOS_EN].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_MOS_EN].number,
            (g_gpios_gugong[BBST_MOS_EN].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    }

    if (g_gpios_gugong[BBST_EN].inited == OAL_TRUE &&
        g_gpios_gugong[BBST_VOUT].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_VOUT].number,
            (g_gpios_gugong[BBST_VOUT].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
        gpio_direction_output(g_gpios_gugong[BBST_EN].number,
            (g_gpios_gugong[BBST_EN].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    } else if (g_gpios_gugong[BBST_EN].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_EN].number,
            (g_gpios_gugong[BBST_EN].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    } else if (g_gpios_gugong[BBST_VOUT].inited == OAL_TRUE) {
        gpio_direction_output(g_gpios_gugong[BBST_VOUT].number,
            (g_gpios_gugong[BBST_VOUT].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    } else {
#ifdef CONFIG_HUAWEI_LOW_POWER
        buck_boost_set_enable(GPIO_LEVEL_HIGH, BBST_USER_BOOST_RSMC);
        buck_boost_set_vout(g_gugong_bbst_vout[VOL_TYPE_WORK], BBST_USER_BOOST_RSMC);
#endif
    }
}

STATIC void gugong_pa_gpio_config(uint32_t sub_type)
{
    ps_print_info("gugong pa set type[%u], en init[%d] vout[%d]",
                  sub_type, g_gpios_gugong[BBST_EN].inited, g_gpios_gugong[BBST_VOUT].inited);
    if (sub_type == SW_OFF) {
        // gugong PA power down
        gugong_pa_gpio_off();
    } else if (sub_type == SW_ON) {
        // gugong PA power on
        gugong_pa_gpio_on();
    }
}

STATIC void board_clk_switch_gpio_config(uint32_t sub_type)
{
    if (g_gpios_gugong[WB_SWT].inited == OAL_FALSE) {
        ps_print_warning("clk switch gpio[WB_SWT] init status is %u!\n", g_gpios_gugong[WB_SWT].inited);
        return;
    }
    if (sub_type == SW_OFF) {
        gpio_direction_output(g_gpios_gugong[WB_SWT].number,
            (g_gpios_gugong[WB_SWT].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
        ps_print_info("board clk switch power off\n");
    } else if (sub_type == SW_ON) {
        gpio_direction_output(g_gpios_gugong[WB_SWT].number,
            (g_gpios_gugong[WB_SWT].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
        ps_print_info("board clk switch power on\n");
    }
}

STATIC void wifi_power_en_config(uint32_t sub_type)
{
    if (sub_type == SW_ON) {
        conn_set_gpio_level(W_POWER, GPIO_LEVEL_HIGH);
        mdelay(20); /* delay 20ms */
        ps_print_info("set w_en to high\n");
    } else if (sub_type == SW_OFF) {
        if ((wlan_pm_is_poweron() == OAL_TRUE)) {
            ps_print_info("wifi is open, do not set w_en to low!\n");
            return;
        }
        conn_set_gpio_level(W_POWER, GPIO_LEVEL_LOW);
        mdelay(10); /* delay 10ms */
        ps_print_info("set w_en to low\n");
    }
}

STATIC int32_t gugong_rsmc_config(struct conn_gpio_userdata *data)
{
    dcxo_dl_para_stru *pst_dl_para = NULL;
    pst_dl_para = (dcxo_dl_para_stru *)get_dcxo_data_buf_addr();
    if (pst_dl_para == NULL) {
        ps_print_err("dcxo buffer alloc error\n");
        return -OAL_FAIL;
    }

    gugong_antsw_gpio_config(data->sub_type); // ant GPIO config
    if (data->sub_type == SW_OFF) {
        g_clk_mod.as_dword = board_clk_mode();
        ps_print_info("gugong rsmc off, reconfig default, clock mode is 0x%x!\n", g_clk_mod.as_dword);
        g_gugong_status_info.gugong_cmd_config = OAL_FALSE;
        pst_dl_para->reserve2[0] = OAL_FALSE;

        wifi_power_en_config(OAL_FALSE);  // wifi关闭状态，退出故宫时确保w_en是拉低状态
    } else if (data->sub_type == SW_ON) {
        if ((wlan_is_shutdown() == OAL_TRUE) && (bfgx_is_shutdown() == OAL_TRUE)) {
            if (data->data_len != sizeof(g_clk_mod)) {
                ps_print_err("gugong clock parament is not complete!\n");
                return -OAL_FAIL;
            } else {
                g_gugong_status_info.gugong_cmd_config = OAL_TRUE;
                g_clk_mod.bits.main_type = data->data[0];     // 0：GNSS clock main type
                g_clk_mod.bits.main_clk_freq = data->data[1]; // 1：GNSS clock main clock freq
                g_clk_mod.bits.sub_type = data->data[2];      // 2：GNSS clock subtype
                g_clk_mod.bits.sub_clk_freq = data->data[3];  // 3：GNSS clock sub-clock freq
                ps_print_info("gugong rsmc on, gugong user clock mode 0x%x!\n", g_clk_mod.as_dword);
                declare_dft_trace_key_info("gugong mode start", OAL_DFT_TRACE_SUCC);
                pst_dl_para->reserve2[0] = OAL_TRUE;
            }
        } else {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_OPEN_CMD_FAIL);
            ps_print_err("wifi status[%u], bt status[%u] , don't allow enter gugong!\n",
                         (wlan_is_shutdown() == OAL_TRUE) ? OAL_FALSE : OAL_TRUE,
                         (bfgx_is_shutdown() == OAL_TRUE) ? OAL_FALSE : OAL_TRUE);
            pst_dl_para->reserve2[0] = OAL_FALSE;
            return -OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

STATIC void clk_38p4_gpio_config(uint32_t sub_type)
{
    if (g_gpios_gugong[CLK38P4_PWR].inited == OAL_FALSE) {
        ps_print_warning("clk 38p4 gpio[%s] is not init!\n", g_gpios_gugong[CLK38P4_PWR].gpio_name);
        return;
    }
    if (sub_type == SW_OFF) {
        // gugong 38P4 power off
        gpio_direction_output(g_gpios_gugong[CLK38P4_PWR].number,
            (g_gpios_gugong[CLK38P4_PWR].init_mode == 0) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
        ps_print_info("clock 38P4 power off\n");
    } else if (sub_type == SW_ON) {
        // gugong 38P4 power on
        gpio_direction_output(g_gpios_gugong[CLK38P4_PWR].number,
            (g_gpios_gugong[CLK38P4_PWR].init_mode == 0) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
        ps_print_info("clock 38P4 power on\n");
    }
}

STATIC void get_gugong_status_to_user(struct conn_gpio_userdata *data, uint8_t OAL_USER *userdata)
{
    data->sub_type = g_gugong_status_info.gugong_status_done;
    ps_print_info("get gugong status %d\n", g_gugong_status_info.gugong_status_done);
    if (copy_to_user(userdata, data, sizeof(struct conn_gpio_userdata))) {
        ps_print_err("copy_to_user is err!\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_QUERY_STATUS_FAIL);
    }

    ps_core_show_bfgx_status();

    if (wlan_pm_is_poweron() == OAL_TRUE) {
        ps_print_info("wifi is open!");
    } else {
        ps_print_info("wifi is close!");
    }
}

STATIC void gugong_factory_config(uint32_t sub_type)
{
    dcxo_dl_para_stru *pst_dl_para = NULL;
    pst_dl_para = (dcxo_dl_para_stru *)get_dcxo_data_buf_addr();
    if (pst_dl_para == NULL) {
        ps_print_err("dcxo buffer alloc error\n");
        return;
    }
    if (sub_type == SW_ON) {
        g_gugong_status_info.gugong_factory_status = OAL_TRUE;
        pst_dl_para->reserve2[1] = OAL_TRUE;
        ps_print_info("it is factory status\n");
    } else {
        g_gugong_status_info.gugong_factory_status = OAL_FALSE;
        pst_dl_para->reserve2[1] = OAL_FALSE;
        ps_print_info("it is not factory status\n");
    }
}

STATIC int32_t gugong_gpio_set(struct conn_gpio_userdata *data, uint8_t OAL_USER *userdata)
{
    struct pm_top* pm_top_data = pm_get_top();
    ps_print_info("gugong set gugong type main[%u] sub[%u]\n",
                  data->main_type, data->sub_type);

    if (data->main_type == TYPE_SPDT) {
        gugong_spdt_gpio_config(data->sub_type);
    } else if (data->main_type == TYPE_PA) {
        gugong_pa_gpio_config(data->sub_type);
    } else if (data->main_type == TYPE_RSMC) {
        mutex_lock(&(pm_top_data->host_mutex));
        if (gugong_rsmc_config(data) != OAL_SUCC) {
            mutex_unlock(&(pm_top_data->host_mutex));
            return -OAL_FAIL;
        }
        mutex_unlock(&(pm_top_data->host_mutex));
    } else if (data->main_type == TYPE_CLK) {
        clk_38p4_gpio_config(data->sub_type);
    } else if (data->main_type == TYPE_QUERY) {
        get_gugong_status_to_user(data, userdata);
    } else if (data->main_type == TYPE_FAC) {
        gugong_factory_config(data->sub_type);
    } else if (data->main_type == TYPE_W_POWER) {
        mutex_lock(&(pm_top_data->host_mutex));
        wifi_power_en_config(data->sub_type);
        mutex_unlock(&(pm_top_data->host_mutex));
    }
    return OAL_SUCC;
}

STATIC void board_clk_config_switch(void)
{
    dcxo_dl_para_stru *pst_dl_para = NULL;
    pst_dl_para = (dcxo_dl_para_stru *)get_dcxo_data_buf_addr();
    if (pst_dl_para == NULL) {
        ps_print_info("input pst_dl_para is NULL\n");
        return;
    }

    pst_dl_para->dcxo_tcxo_mode = g_clk_mod.as_dword;
    ps_print_info("switch set clock mode 0x%x!\n", g_clk_mod.as_dword);
    if (g_clk_mod.bits.main_clk_freq == CLK_38P4) {
        g_st_board_info.gpio_tcxo_detect_level = GPIO_LEVEL_HIGH;
    } else if (g_clk_mod.bits.main_clk_freq == CLK_76P8) {
        g_st_board_info.gpio_tcxo_detect_level = GPIO_LEVEL_LOW;
    }
}

STATIC void board_gugong_gpio_config_reset(void)
{
    // gugong spdt_gpio off
    gugong_spdt_gpio_config(OAL_FALSE);

    // gugong PA power down
    gugong_pa_gpio_config(OAL_FALSE);

    // 切换w/b switch开关到gnss状态
    board_clk_switch_gpio_config(OAL_FALSE);

    clk_38p4_gpio_config(OAL_FALSE);
    ps_print_info("gugong gpio config reset\n");
}

void board_clk_config_reset(void)
{
    // 释放所有故宫资源
    dcxo_dl_para_stru *pst_dl_para = NULL;
    pst_dl_para = (dcxo_dl_para_stru *)get_dcxo_data_buf_addr();
    if (pst_dl_para == NULL) {
        ps_print_err("dcxo buffer alloc error\n");
        return;
    }
    gugong_status_info_init();
    board_gugong_gpio_config_reset();

    // 切回默认时钟配置
    board_clk_config_switch();
    pst_dl_para->reserve2[0] = OAL_FALSE;
    pst_dl_para->reserve2[1] = OAL_FALSE;

    wifi_power_en_config(OAL_FALSE);

    if ((g_gugong_status_info.gugong_cmd_config == OAL_TRUE) ||
        (g_gugong_status_info.gugong_status_done == OAL_TRUE)) {
        declare_dft_trace_key_info("gugong exception", OAL_DFT_TRACE_SUCC);
    }
    ps_print_info("board_clk_config_reset\n");
}

/*
 * 函 数 名  : gugong_clk_power_on
 * 功能描述  : 进入gugong的上电处理和退出gugong的上电处理
 */
void gugong_clk_power_on(uint32_t subsys)
{
    if (g_gugong_status_info.gugong_cmd_config == OAL_TRUE) {
        if (subsys != G_SYS) {
            ps_print_err("gugong must gnss first power on\n");
            board_clk_config_reset();
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_OPEN_FAIL);
            return;
        }

        if (g_gugong_status_info.gugong_on_last_entered == OAL_TRUE) {
            ps_print_err("gugong enter more than one times\n");
            board_clk_config_reset();
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_OPEN_FAIL);
            return;
        }

        g_gugong_status_info.gugong_on_last_entered = OAL_TRUE;
        g_gugong_status_info.gugong_status_done = OAL_TRUE;
        ps_print_info("enter gugong success\n");
    } else {
        g_gugong_status_info.gugong_on_last_entered = OAL_FALSE;
    }
}

/*
 * 函 数 名  : gugong_clk_power_off
 * 功能描述  : 进入gugong前的下电处理和退出gugong的下电处理
 */
void gugong_clk_power_off(void)
{
    if (g_gugong_status_info.gugong_cmd_config == OAL_TRUE) {
        if (g_gugong_status_info.gugong_off_last_entered == OAL_TRUE) {
            ps_print_err("gugong before last power off, not revice gnss gugong exit\n");
            board_clk_config_reset();
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_NO_CLOSE_CMD);
            return;
        } else {
            // gnss 配置下一次上电为故宫上电状态
            g_gugong_status_info.gugong_off_last_entered = OAL_TRUE;
            ps_print_info("gnss next enter gugong status, config gugong clk\n");

            // 切换到故宫状态的时钟模式
            board_clk_config_switch();

            // 切换w/b switch开关到故宫状态
            board_clk_switch_gpio_config(OAL_TRUE);
        }
    } else {
        g_gugong_status_info.gugong_off_last_entered = OAL_FALSE;

        // 切换到gnss状态的时钟模式
        board_clk_config_switch();

        // 切换w/b switch开关到gnss状态
        board_clk_switch_gpio_config(OAL_FALSE);

        // 退出gugong，关闭38P4时钟
        clk_38p4_gpio_config(OAL_FALSE);

        g_gugong_status_info.gugong_status_done = OAL_FALSE;
    }
}

void gugong_power_off_with_chip_on(void)
{
    if ((g_gugong_status_info.gugong_cmd_config == OAL_TRUE) &&
        (g_gugong_status_info.gugong_status_done == OAL_FALSE)) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_OPEN_FAIL);
        ps_print_err("before enter gugong status, other subsys was power on! need to reset to gnss\n");
        board_clk_config_reset();
    } else if ((g_gugong_status_info.gugong_status_done == OAL_TRUE) &&
               (g_gugong_status_info.gugong_factory_status == OAL_FALSE)) {
        ps_print_err("before exit gugong status, other subsys was power on! need to DFR exit gugong\n");
        declare_dft_trace_key_info("gugong power exception", OAL_DFT_TRACE_EXCEP);
        plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, GUGONG_POWER_FAIL);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_ALL_SYSTEM_RST_SUCC);
    }
}

uint32_t is_gugong_status_mp16c(void)
{
    ps_print_info("gugong status is %d", g_gugong_status_info.gugong_status_done);
    return g_gugong_status_info.gugong_status_done;
}

void gugong_check_subsys_power_on(uint32_t subsys)
{
    if ((g_gugong_status_info.gugong_factory_status == OAL_FALSE) &&
        (g_gugong_status_info.gugong_status_done == OAL_TRUE)) {
        if (subsys == W_SYS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_WIFI_POWER_ON);
            ps_print_err("WIFI should not be opened in the gugong state!\n");
        } else if (subsys == B_SYS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GUGONG, CHR_PLAT_DRV_ERROR_GUGONG_BT_POWER_ON);
            ps_print_err("BT should not be opened in the gugong state!\n");
        }
    }
}

STATIC void gugong_dts_config_init(struct platform_device *pdev)
{
    struct device_node *of_node = pdev->dev.of_node;
    if (of_node == NULL) {
        ps_print_info("gugong no support devicetree\n");
        return;
    }

    gugong_status_info_init();
    gugong_gpio_init(of_node);
    gugong_voltage_init(of_node);
}

STATIC void gugong_gpio_reinit(int32_t gpio_num)
{
    int32_t index_gpio;

    if (gpio_num <= 0) {
        return;
    }

    for (index_gpio = 0; index_gpio < gpio_num; index_gpio++) {
        if (g_gpios_gugong[index_gpio].inited == OAL_TRUE) {
            gpio_free(g_gpios_gugong[index_gpio].number);
            g_gpios_gugong[index_gpio].inited = OAL_FALSE;
        }
    }
}

STATIC void gugong_dts_config_reinit(void)
{
    gugong_gpio_reinit(IO_MAX);
}

STATIC struct conn_gpio_node g_gpio_spec[] = {
    {
        .pin = {
            .name = PROC_NAME_GPIO_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = CHIP_POWR,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_MPXX_POWEN_ON
                }
            },
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = W_POWER,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_WLAN_POWEN_ON_ENABLE
                }
            },
        }
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = BFGX_POWER,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_BFGX_POWEN_ON_ENABLE
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_G_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = G_POWER,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_G_POWEN_ON_ENABLE
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_CLK,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_CLK,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_MPXX_GPIO_SSI_CLK
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_DATA,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_DATA,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_MPXX_GPIO_SSI_DATA
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_WLAN,
            .is_key_gpio = OAL_TRUE,
            .type = HOST_WKUP_W,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_WIFI,
                    .label_name = DTS_PROP_GPIO_HOST_WAKEUP_WLAN
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SLE_WAKEUP_HOST,
            .is_key_gpio = OAL_TRUE,
            .type = G_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_SLE,
                    .label_name = DTS_PROP_GPIO_SLE_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "gle_wake_host",
            .flags = IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
            .init_set_enable = OAL_FALSE
        },
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_WAKEUP_HOST,
            .is_key_gpio = OAL_TRUE,
            .type = BFGX_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_BFGX,
                    .label_name = DTS_PROP_MPXX_GPIO_BFGX_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "bfgx_wake_host",
            .flags = IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
            .init_set_enable = OAL_FALSE
        },
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_WAKEUP_HOST,
            .is_key_gpio = OAL_TRUE,
            .type = W_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_WIFI,
                    .label_name = DTS_PROP_MPXX_GPIO_WLAN_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "wlan_wake_host",
            .flags = IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
            .init_set_enable = OAL_FALSE
        },
    }
};

STATIC struct conn_gpio_ops g_ops = {
    .user_set = gugong_gpio_set
};

STATIC struct conn_gpio_chip g_drv_gpio_chip = {
    .node = g_gpio_spec,
    .node_num = oal_array_size(g_gpio_spec),
    .ops = &g_ops
};

int32_t mp16c_gpio_init(struct platform_device *pdev)
{
    int32_t ret;

    gugong_dts_config_init(pdev);
    ret = conn_gpio_init(&g_drv_gpio_chip);
    if (ret < 0) {
        ps_print_err("mp16c gpio init fail\n");
        return ret;
    }

    return OAL_SUCC;
}

void mp16c_gpio_remove(struct platform_device *pdev)
{
    conn_gpio_remove();
    gugong_dts_config_reinit();
}
