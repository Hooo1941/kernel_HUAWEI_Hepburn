/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Declaration Driver Entry Function
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#define MPXX_LOG_MODULE_NAME     "[BOARD_DTS]"
#define MPXX_LOG_MODULE_NAME_VAR board_dts_loglevel
#include <linux/clk.h>
#include <linux/platform_device.h>

#include "board_dts.h"
#include "chr_user.h"
#include "plat_common_clk.h"
#include "plat_debug.h"
#include "plat_pm.h"
#include "plat_uart.h"
#include "plat_gpio.h"

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#include "bfgx_data_parse.h"
#include "plat_firmware_uart.h"
#endif

STATIC int32_t mpxx_board_wifi_tas_gpio_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    int32_t physical_gpio = 0;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_MPXX_WIFI);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_MPXX_WIFI);
        return BOARD_SUCC;
    }

    ret = of_property_read_bool(np, DTS_PROP_WIFI_TAS_EN);
    if (ret) {
        ps_print_info("mpxx wifi tas enable\n");
        g_st_board_info.wifi_tas_enable = WIFI_TAS_ENABLE;
    } else {
        ps_print_info("mpxx wifi tas not enable\n");
        g_st_board_info.wifi_tas_enable = WIFI_TAS_DISABLE;
        return BOARD_SUCC;
    }

    /* wifi tas control gpio request */
    ret = get_board_gpio(DTS_NODE_MPXX_WIFI, DTS_PROP_GPIO_WIFI_TAS, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_err("get dts prop %s failed\n", DTS_PROP_GPIO_WIFI_TAS);
        return BOARD_SUCC;
    }

    g_st_board_info.rf_wifi_tas = physical_gpio;
    ps_print_info("mpxx wifi tas gpio is %d\n", g_st_board_info.rf_wifi_tas);

    ret = of_property_read_u32(np, DTS_PROP_MPXX_WIFI_TAS_STATE, &g_st_board_info.wifi_tas_state);
    if (ret) {
        ps_print_err("read prop [%s] fail, ret=%d\n", DTS_PROP_MPXX_WIFI_TAS_STATE, ret);
    }
    if (g_st_board_info.wifi_tas_state) {
        ret = gpio_request_one(physical_gpio, GPIOF_OUT_INIT_HIGH, PROC_NAME_GPIO_WIFI_TAS);
    } else {
        ret = gpio_request_one(physical_gpio, GPIOF_OUT_INIT_LOW, PROC_NAME_GPIO_WIFI_TAS);
    }
    if (ret) {
        ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_WIFI_TAS);
        return BOARD_FAIL;
    }
#endif

    return BOARD_SUCC;
}

STATIC void mpxx_free_board_wifi_tas_gpio(void)
{
    oal_gpio_free(g_st_board_info.rf_wifi_tas);
}

/*
 * 函 数 名  : mpxx_board_flowctrl_gpio_init
 * 功能描述  : 注册gpio流控中断
 */
STATIC int32_t mpxx_board_flowctrl_gpio_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    int32_t physical_gpio;

    ret = get_board_gpio(DTS_NODE_MPXX_WIFI, DTS_PROP_GPIO_WLAN_FLOWCTRL, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_info("NO dts prop %s\n", DTS_PROP_GPIO_WLAN_FLOWCTRL);
        return BOARD_SUCC;
    }

    ret = board_gpio_request(physical_gpio, PROC_NAME_GPIO_WLAN_FLOWCTRL, GPIO_DIRECTION_INPUT);
    if (ret != BOARD_SUCC) {
        ps_print_err("gpio requeset %s\n", PROC_NAME_GPIO_WLAN_FLOWCTRL);
        return BOARD_FAIL;
    }

    g_st_board_info.flowctrl_gpio = physical_gpio;
    ps_print_info("mpxx flow ctrl gpio is %d\n", g_st_board_info.flowctrl_gpio);
#endif
    return BOARD_SUCC;
}

STATIC void mpxx_free_board_flowctrl_gpio(void)
{
    oal_gpio_free(g_st_board_info.flowctrl_gpio);
}


STATIC int32_t mpxx_get_board_pmu_clk32k(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    const char *clk_name = NULL;
    struct clk *clk = NULL;
    struct device *dev = NULL;

    ret = get_board_custmize(DTS_NODE_HISI_MPXX, DTS_PROP_CLK_32K, &clk_name);
    if (ret != BOARD_SUCC) {
        ps_print_info("NO dts prop %s\n", DTS_PROP_CLK_32K);
        return BOARD_SUCC;
    }
    g_st_board_info.clk_32k_name = clk_name;

    ps_print_info("mpxx 32k clk name is %s\n", g_st_board_info.clk_32k_name);

    dev = &pdev->dev;
    clk = devm_clk_get(dev, clk_name);
    if (clk == NULL) {
        ps_print_err("Get 32k clk %s failed!!!\n", clk_name);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_DTS, CHR_PLAT_DRV_ERROR_32K_CLK_DTS);
        return BOARD_FAIL;
    }
    g_st_board_info.clk_32k = clk;
    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}

STATIC int32_t mpxx_get_board_uart_port(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;
    const char *uart_port = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_MPXX_BFGX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_MPXX_BFGX);
        return BOARD_SUCC;
    }

    /* 使用uart4，需要在dts里新增DTS_PROP_MPXX_UART_PCLK项，指明uart4不依赖sensorhub */
    ret = of_property_read_bool(np, DTS_PROP_MPXX_UART_PCLK);
    if (ret) {
        ps_print_info("uart pclk normal\n");
        g_st_board_info.uart_pclk = UART_PCLK_NORMAL;
    } else {
        ps_print_info("uart pclk from sensorhub\n");
        g_st_board_info.uart_pclk = UART_PCLK_FROM_SENSORHUB;
    }

    ret = get_board_custmize(DTS_NODE_MPXX_BFGX, DTS_PROP_MPXX_UART_POART, &uart_port);
    if (ret != BOARD_SUCC) {
        ps_print_info("no %s\n", DTS_PROP_MPXX_UART_POART);
    }
    g_st_board_info.uart_port[BUART] = uart_port;

    if ((get_mpxx_subchip_type() == BOARD_VERSION_MP16)) {
        ret = get_board_custmize(DTS_NODE_MPXX_ME, DTS_PROP_MPXX_GUART_POART, &uart_port);
        if (ret != BOARD_SUCC) {
            ps_print_info("no %s\n", DTS_PROP_MPXX_GUART_POART);
        }
        g_st_board_info.uart_port[GUART] = uart_port;
    } else if ((get_mpxx_subchip_type() == BOARD_VERSION_MP16C) ||
                get_mpxx_subchip_type() == BOARD_VERSION_GF61) {
        ret = get_board_custmize(DTS_NODE_MPXX_SLE, DTS_PROP_MPXX_SLEUART_POART, &uart_port);
        if (ret != BOARD_SUCC) {
            ps_print_info("no %s\n", DTS_PROP_MPXX_SLEUART_POART);
        }
        g_st_board_info.uart_port[GUART] = uart_port;
    }

    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}


#ifdef HAVE_HISI_IR
STATIC int32_t mpxx_board_ir_ctrl_gpio_init(void)
{
    int32_t ret;
    int32_t physical_gpio = 0;

    ret = get_board_gpio(DTS_NODE_MPXX_BFGX, DTS_PROP_GPIO_BFGX_IR_CTRL, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_info("dts prop %s not exist\n", DTS_PROP_GPIO_BFGX_IR_CTRL);
        g_st_board_info.bfgx_ir_ctrl_gpio = 0;
    } else {
        g_st_board_info.bfgx_ir_ctrl_gpio = physical_gpio;

        ret = gpio_request_one(physical_gpio, GPIOF_OUT_INIT_LOW, PROC_NAME_GPIO_BFGX_IR_CTRL);
        if (ret) {
            ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_BFGX_IR_CTRL);
        }
    }

    return BOARD_SUCC;
}

STATIC int32_t mpxx_board_ir_ctrl_pmic_init(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret = BOARD_FAIL;
    struct device_node *np = NULL;
    int32_t irled_voltage = 0;
    if (pdev == NULL) {
        ps_print_err("board pmu pdev is NULL!\n");
        return ret;
    }

    ret = get_board_dts_node(&np, DTS_NODE_MPXX_BFGX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_MPXX_BFGX);
        return ret;
    }

    g_st_board_info.bfgn_ir_ctrl_ldo = regulator_get(&pdev->dev, DTS_PROP_MPXX_IRLED_LDO_POWER);

    if (IS_ERR(g_st_board_info.bfgn_ir_ctrl_ldo)) {
        ps_print_err("board_ir_ctrl_pmic_init get ird ldo failed\n");
        return ret;
    }

    ret = of_property_read_u32(np, DTS_PROP_MPXX_IRLED_VOLTAGE, &irled_voltage);
    if (ret == BOARD_SUCC) {
        ps_print_info("set irled voltage %d mv\n", irled_voltage / VOLATAGE_V_TO_MV); /* V to mV */
        ret = regulator_set_voltage(g_st_board_info.bfgn_ir_ctrl_ldo, (int)irled_voltage, (int)irled_voltage);
        if (ret) {
            ps_print_err("board_ir_ctrl_pmic_init set voltage ldo failed\n");
            return ret;
        }
    } else {
        ps_print_err("get irled voltage failed ,use default\n");
    }

    ret = regulator_set_mode(g_st_board_info.bfgn_ir_ctrl_ldo, REGULATOR_MODE_NORMAL);
    if (ret) {
        ps_print_err("board_ir_ctrl_pmic_init set ldo mode failed\n");
        return ret;
    }
    ps_print_info("board_ir_ctrl_pmic_init success\n");
    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}
#endif

STATIC int32_t mpxx_board_ir_ctrl_init(struct platform_device *pdev)
{
#if (defined HAVE_HISI_IR) && (defined _PRE_CONFIG_USE_DTS)
    int ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_MPXX_BFGX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_MPXX_BFGX);
        return BOARD_SUCC;
    }
    g_st_board_info.have_ir = of_property_read_bool(np, "have_ir");
    if (!g_st_board_info.have_ir) {
        ps_print_info("board has no Ir");
    } else {
        ret = of_property_read_u32(np, DTS_PROP_MPXX_IR_LDO_TYPE, &g_st_board_info.irled_power_type);
        ps_print_info("read property ret is %d, irled_power_type is %d\n", ret, g_st_board_info.irled_power_type);
        if (ret != BOARD_SUCC) {
            ps_print_err("get dts prop %s failed\n", DTS_PROP_MPXX_IR_LDO_TYPE);
            goto err_get_ir_ctrl_gpio;
        }

        if (g_st_board_info.irled_power_type == IR_GPIO_CTRL) {
            ret = mpxx_board_ir_ctrl_gpio_init();
            if (ret != BOARD_SUCC) {
                ps_print_err("ir_ctrl_gpio init failed\n");
                goto err_get_ir_ctrl_gpio;
            }
        } else if (g_st_board_info.irled_power_type == IR_LDO_CTRL) {
            ret = mpxx_board_ir_ctrl_pmic_init(pdev);
            if (ret != BOARD_SUCC) {
                ps_print_err("ir_ctrl_pmic init failed\n");
                goto err_get_ir_ctrl_gpio;
            }
        } else {
            ps_print_err("get ir_ldo_type failed!err num is %d\n", g_st_board_info.irled_power_type);
            goto err_get_ir_ctrl_gpio;
        }
    }
    return BOARD_SUCC;

err_get_ir_ctrl_gpio:
    return BOARD_FAIL;
#else
    return BOARD_SUCC;
#endif
}

void mpxx_free_board_ir_gpio(void)
{
#if (defined HAVE_HISI_IR)
    oal_gpio_free(g_st_board_info.bfgx_ir_ctrl_gpio);
#endif
}

#define EMU_INI_BUF_SIZE 50
static void mpxx_emu_init(void)
{
    char buff[EMU_INI_BUF_SIZE];
    g_st_board_info.is_emu = OAL_FALSE;

    memset_s(buff, sizeof(buff), 0, sizeof(buff));
    if (get_cust_conf_string(INI_MODU_PLAT, "emu_enable", buff, sizeof(buff) - 1) == INI_SUCC) {
        if (!oal_strncmp("yes", buff, OAL_STRLEN("yes"))) {
            g_st_board_info.is_emu = OAL_TRUE;
            ps_print_info("emu enable\n");
        }
    }
}

STATIC int32_t mpxx_check_evb_or_fpga(void)
{
#ifdef _PRE_HI375X_NO_DTS
        ps_print_info("MPXX ASIC VERSION\n");
        g_st_board_info.is_asic = VERSION_ASIC;
#else
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_HISI_MPXX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_MPXX);
        return BOARD_FAIL;
    }

    ret = of_property_read_bool(np, DTS_PROP_MPXX_VERSION);
    if (ret) {
        ps_print_info("MPXX ASIC VERSION\n");
        g_st_board_info.is_asic = VERSION_ASIC;
    } else {
        ps_print_info("MPXX FPGA VERSION\n");
        g_st_board_info.is_asic = VERSION_FPGA;
    }
#endif
#endif
    return BOARD_SUCC;
}
#ifdef _PRE_CONFIG_USE_DTS
static void mpxx_check_gt_subsystem_support(void)
{
    int32_t ret;
    struct device_node *np = NULL;
    char *dts_status_string = NULL;
    g_st_board_info.is_gt_disable = 1;
    ret = get_board_dts_node(&np, DTS_NODE_HISI_GT);
    if (ret != BOARD_SUCC) {
        ps_print_info("DTS read node %s fail, not support gt\n", DTS_NODE_HISI_GT);
        return;
    }
    ret = of_property_read_string(np, "status", (const char **)&dts_status_string);
    if (ret < 0 || dts_status_string == NULL) {
        ps_print_info("get gt dts node status fail\n");
        return;
    }
    if (!oal_strcmp("ok", dts_status_string)) {
        g_st_board_info.is_gt_disable = 0;
    }
}
#endif

STATIC int32_t mpxx_check_mpxx_subsystem_support(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_HISI_MPXX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_MPXX);
        return BOARD_FAIL;
    }

    ret = of_property_read_bool(np, DTS_PROP_MPXX_WIFI_DISABLE);
    if (ret) {
        g_st_board_info.is_wifi_disable = 1;
    } else {
        g_st_board_info.is_wifi_disable = 0;
    }

    ret = of_property_read_bool(np, DTS_PROP_MPXX_BFGX_DISABLE);
    if (ret) {
        g_st_board_info.is_bfgx_disable = 1;
    } else {
        g_st_board_info.is_bfgx_disable = 0;
    }
    mpxx_check_gt_subsystem_support();
#endif
    mpxx_emu_init();

    if (hi110x_is_emu() == OAL_TRUE) {
        // non-bfgx in emu
        g_st_board_info.is_bfgx_disable = 1;
    }

    ps_print_info("wifi %s, bfgx %s, gt %s\n",
                  (g_st_board_info.is_wifi_disable == 0) ? "enabled" : "disabled",
                  (g_st_board_info.is_bfgx_disable == 0) ? "enabled" : "disabled",
                  (g_st_board_info.is_gt_disable == 0) ? "enabled" : "disabled");

    return BOARD_SUCC;
}

STATIC int32_t mpxx_check_pmu_clk_share(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;
    int32_t pmu_clk_request = PMU_CLK_REQ_DISABLE;

    ret = get_board_dts_node(&np, DTS_NODE_HISI_MPXX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_MPXX);
        return BOARD_FAIL;
    }

    ret = of_property_read_u32(np, DTS_PROP_MPXX_PMU_CLK, &pmu_clk_request);
    if (ret != BOARD_SUCC) {
        ps_print_info("get dts prop %s failed, mpxx pmu clk request disable\n", DTS_PROP_MPXX_PMU_CLK);
        g_st_board_info.pmu_clk_share_enable = PMU_CLK_REQ_DISABLE;
    } else {
        ps_print_info("mpxx PMU clk request is %s\n", (pmu_clk_request ? "enable" : "disable"));
        g_st_board_info.pmu_clk_share_enable = pmu_clk_request;
    }

    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}

STATIC int32_t mpxx_board_get_power_pinctrl(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    int32_t physical_gpio = 0;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_HISI_MPXX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_MPXX);
        return BOARD_FAIL;
    }

    ret = of_property_read_bool(np, DTS_PROP_MPXX_POWER_PREPARE);
    if (ret) {
        ps_print_info("need prepare before board power on\n");
        g_st_board_info.need_power_prepare = NEED_POWER_PREPARE;
    } else {
        ps_print_info("no need prepare before board power on\n");
        g_st_board_info.need_power_prepare = NO_NEED_POWER_PREPARE;
        return BOARD_SUCC;
    }

    ret = get_board_gpio(DTS_NODE_HISI_MPXX, DTS_PROP_MPXX_GPIO_TCXO_FREQ_DETECT, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_err("get dts prop %s failed\n", DTS_PROP_MPXX_GPIO_TCXO_FREQ_DETECT);
        return BOARD_FAIL;
    }

    g_st_board_info.tcxo_freq_detect = physical_gpio;

    if (board_main_clk_freq() == CLK_38P4) {
        g_st_board_info.gpio_tcxo_detect_level = 1;
    } else if (board_main_clk_freq() == CLK_76P8) {
        g_st_board_info.gpio_tcxo_detect_level = 0;
    } else {
        ps_print_info("get tcxo value from dts\n");
        ret = of_property_read_u32(np, DTS_PROP_GPIO_TCXO_LEVEL, &physical_gpio);
        if (ret != BOARD_SUCC) {
            ps_print_err("get dts prop %s failed\n", DTS_PROP_GPIO_TCXO_LEVEL);
            return BOARD_FAIL;
        }
        g_st_board_info.gpio_tcxo_detect_level = physical_gpio;
    }
    // 0:tcxo 76.8M  1:tcxo 38.4M
    ps_print_info("have get dts_prop and prop_val: %s=%d\n", DTS_PROP_GPIO_TCXO_LEVEL, physical_gpio);
#endif

    return BOARD_SUCC;
}

int32_t mpxx_get_ini_file_name_from_dts(const char *dts_prop, char *prop_value, uint32_t size)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;
    int32_t len;
    char out_str[HISI_CUST_NVRAM_LEN] = { 0 };

    np = of_find_compatible_node(NULL, NULL, COST_MPXX_COMP_NODE);
    if (np == NULL) {
        ini_error("dts node %s not found", COST_MPXX_COMP_NODE);
        return INI_FAILED;
    }

    len = of_property_count_u8_elems(np, dts_prop);
    if (len < 0) {
        ini_error("can't get len of dts prop(%s)", dts_prop);
        return INI_FAILED;
    }

    len = ini_min(len, (int32_t)sizeof(out_str));
    ini_debug("read len of dts prop %s is:%d", dts_prop, len);
    ret = of_property_read_u8_array(np, dts_prop, out_str, len);
    if (ret < 0) {
        ini_error("read dts prop (%s) fail", dts_prop);
        return INI_FAILED;
    }

    len = ini_min(len, (int32_t)size);
    if (memcpy_s(prop_value, (size_t)size, out_str, (size_t)len) != EOK) {
        ini_error("memcpy_s error, destlen=%d, srclen=%d\n ", size, len);
        return INI_FAILED;
    }
    ini_debug("dts prop %s value is:%s", dts_prop, prop_value);
#endif
    return INI_SUCC;
}

STATIC int32_t mpxx_gpio_dts_init(struct platform_device *pdev)
{
    int32_t ret;

    ret = mpxx_board_wifi_tas_gpio_init();
    if (ret != BOARD_SUCC) {
        ps_print_err("get wifi tas prop failed\n");
        goto oal_board_wifi_tas_gpio_fail;
    }

    ret = mpxx_board_flowctrl_gpio_init();
    if (ret != BOARD_SUCC) {
        ps_print_err("get wifi tas prop failed\n");
        goto oal_board_flowctrl_gpio_fail;
    }

    return BOARD_SUCC;

oal_board_flowctrl_gpio_fail:
    mpxx_free_board_wifi_tas_gpio();
oal_board_wifi_tas_gpio_fail:
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_BOARD_GPIO_INIT);

    return BOARD_FAIL;
}

STATIC void mpxx_gpio_dts_free(struct platform_device *pdev)
{
    mpxx_free_board_ir_gpio();
    mpxx_free_board_flowctrl_gpio();
    mpxx_free_board_wifi_tas_gpio();
}

STATIC void mpxx_coldboot_partion_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    const char *coldboot_partion = NULL;

    ret = get_board_custmize(DTS_NODE_MPXX_BFGX, DTS_PROP_MPXX_COLDBOOT_PARTION, &coldboot_partion);
    if (ret == BOARD_SUCC) {
        g_st_board_info.coldboot_partion = coldboot_partion;
        ps_print_info("coldboot_partion: %s\n",  coldboot_partion);
    }
#endif
}


STATIC int32_t mpxx_sys_attr_dts_init(struct platform_device *pdev)
{
    int32_t ret;
    ret = mpxx_check_mpxx_subsystem_support();
    if (ret != BOARD_SUCC) {
        ps_print_err("mpxx_check_mpxx_subsystem_support failed\n");
        goto sys_cfg_fail;
    }

    ret = mpxx_check_evb_or_fpga();
    if (ret != BOARD_SUCC) {
        ps_print_err("mpxx_check_evb_or_fpga failed\n");
        goto sys_cfg_fail;
    }

    ret = mpxx_check_pmu_clk_share();
    if (ret != BOARD_SUCC) {
        ps_print_err("mpxx_check_pmu_clk_share failed\n");
        goto sys_cfg_fail;
    }

    ret = mpxx_get_board_pmu_clk32k(pdev);
    if (ret != BOARD_SUCC) {
        ps_print_err("mpxx_check_pmu_clk_share failed\n");
        goto sys_cfg_fail;
    }

    ret = mpxx_get_board_uart_port();
    if (ret != BOARD_SUCC) {
        ps_print_err("get uart port failed\n");
        goto sys_cfg_fail;
    }

    ret = mpxx_board_get_power_pinctrl();
    if (ret != BOARD_SUCC) {
        ps_print_err("mpxx_board_get_power_pinctrl failed\n");
        goto sys_cfg_fail;
    }

    ret = mpxx_board_ir_ctrl_init(pdev);
    if (ret != BOARD_SUCC) {
        ps_print_err("get ir dts prop failed\n");
        goto sys_cfg_fail;
    }

    mpxx_coldboot_partion_init();

    return BOARD_SUCC;

sys_cfg_fail:
    return  BOARD_FAIL;
}

void board_callback_init_dts(void)
{
    g_st_board_info.bd_ops.board_gpio_init = mpxx_gpio_dts_init;
    g_st_board_info.bd_ops.board_gpio_free = mpxx_gpio_dts_free;
    g_st_board_info.bd_ops.board_sys_attr_init = mpxx_sys_attr_dts_init;
}
