/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: factory test
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "board_mp13.h"
#include "plat_firmware.h"
#include "plat_pm.h"

#define GPIO_BASE_ADDR                0X50004000
#define CHECK_DEVICE_RDY_ADDR         0X50000000
#define GPIO_BASE_ADDR_MP16           0x40004000
#define GPIO_BASE_ADDR_MP16C          0x40005000
#define CHECK_DEVICE_RDY_ADDR_MP16    0X40000000
#define WLAN_HOST2DEV_GPIO ((unsigned int)(1 << 1))
#define WLAN_DEV2HOST_GPIO ((unsigned int)(1 << 0))
#define WLAN_HOST2DEV_GPIO_MP16 ((unsigned int)(1 << 13))
#define WLAN_DEV2HOST_GPIO_MP16 ((unsigned int)(1 << 12))

extern int32_t g_ft_fail_powerdown_bypass;

OAL_STATIC void mp13_dump_gpio_regs(void)
{
    uint16_t value;
    int32_t ret;
    value = 0;
    ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, &value);
    if (ret) {
        return;
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "gpio reg 0x%x = 0x%x", GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, value);

    value = 0;
    ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, &value);
    if (ret) {
        return;
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "gpio reg 0x%x = 0x%x", GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, value);
}

int32_t mp13_check_device_ready(void)
{
    uint16_t value;
    int32_t ret;

    value = 0;
    if (get_mpxx_subchip_type() == BOARD_VERSION_MP16 || get_mpxx_subchip_type() == BOARD_VERSION_MP16C) {
        ret = read_device_reg16(CHECK_DEVICE_RDY_ADDR_MP16, &value);
    } else {
        ret = read_device_reg16(CHECK_DEVICE_RDY_ADDR, &value);
    }
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "read 0x%x reg failed, ret=%d", CHECK_DEVICE_RDY_ADDR, ret);
        return -OAL_EFAIL;
    }

    /* 读到0x101表示成功 */
    if (value != 0x101) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "device sysctrl reg error, value=0x%x", value);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t mp13_ssi_set_gpio_direction(uint32_t address, uint16_t gpio_bit, uint32_t direction)
{
    uint16_t value;
    int32_t ret;

    value = 0;
    ret = read_device_reg16(address, &value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "read 0x%x reg failed, ret=%d", address, ret);
        return -OAL_EFAIL;
    }

    if (direction == GPIO_DIRECTION_OUTPUT) {
        value |= gpio_bit;
    } else {
        value &= (uint16_t)(~gpio_bit);
    }

    ret = write_device_reg16(address, value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d", address, value, ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t mp13_ssi_set_gpio_value(int32_t gpio, int32_t gpio_lvl, uint32_t address, uint16_t value)
{
    int32_t ret;

    ret = write_device_reg16(address, value);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d", address, value, ret);
        return -OAL_EFAIL;
    }

    oal_msleep(1);

    if (oal_gpio_get_value(gpio) != gpio_lvl) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pull gpio to %d failed!", gpio_lvl);
        mp13_dump_gpio_regs();
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t mp13_ssi_check_gpio_value(int32_t gpio, int32_t gpio_lvl, uint32_t address, uint16_t check_bit)
{
    uint16_t read;
    int32_t ret;

    gpio_direction_output(gpio, gpio_lvl);
    oal_msleep(1);

    read = 0;
    ret = read_device_reg16(address, &read);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "read 0x%x reg failed, ret=%d", address, ret);
        return -OAL_EFAIL;
    }

    oal_print_mpxx_log(MPXX_LOG_DBG, "read 0x%x reg=0x%x", address, read);

    read &= check_bit;

    if (((gpio_lvl == GPIO_HIGHLEVEL) && (read == 0)) || ((gpio_lvl == GPIO_LOWLEVEL) && (read != 0))) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "pull gpio to %d failed!", gpio_lvl);
        mp13_dump_gpio_regs();
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t mp13_check_wlan_wakeup_host(void)
{
    int32_t i;
    int32_t ret;
    int32_t gpio_num;
    uint32_t ssi_address;
    const uint32_t ul_test_times = 2;
    uint32_t gpio_base_addr;
    uint32_t wlan_dev2host_gpio;
    int32_t board_ver;

    gpio_num = conn_get_gpio_number_by_type(W_WKUP_HOST);
    board_ver = get_mpxx_subchip_type();
    if (gpio_num == 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan_wakeup_host gpio is zero!");
        return -OAL_EIO;
    }
    if (board_ver >= BOARD_VERSION_MP16 && board_ver <= BOARD_VERSION_GF61) {
        gpio_base_addr = (board_ver == BOARD_VERSION_MP16 ? GPIO_BASE_ADDR_MP16 : GPIO_BASE_ADDR_MP16C);
        wlan_dev2host_gpio = WLAN_DEV2HOST_GPIO_MP16;
    } else {
        gpio_base_addr = GPIO_BASE_ADDR;
        wlan_dev2host_gpio = WLAN_DEV2HOST_GPIO;
    }
    ssi_address = gpio_base_addr + GPIO_INOUT_CONFIG_REGADDR;
    ret = mp13_ssi_set_gpio_direction(ssi_address, wlan_dev2host_gpio, GPIO_DIRECTION_OUTPUT);
    if (ret < 0) {
        return ret;
    }

    ssi_address = gpio_base_addr + GPIO_LEVEL_CONFIG_REGADDR;
    for (i = 0; i < ul_test_times; i++) {
        ret = mp13_ssi_set_gpio_value(gpio_num, GPIO_HIGHLEVEL, ssi_address, wlan_dev2host_gpio);
        if (ret < 0) {
            return ret;
        }

        ret = mp13_ssi_set_gpio_value(gpio_num, GPIO_LOWLEVEL, ssi_address, 0);
        if (ret < 0) {
            return ret;
        }

        oal_print_mpxx_log(MPXX_LOG_INFO, "check d2h wakeup io %d times ok", i + 1);
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "check d2h wakeup io done");
    return OAL_SUCC;
}

OAL_STATIC int32_t mp13_check_host_wakeup_wlan(void)
{
    int32_t i;
    int32_t ret;
    int32_t gpio_num;
    uint32_t ssi_address;
    const uint32_t ul_test_times = 2;
    uint32_t gpio_base_addr;
    uint32_t wlan_host2dev_gpio;
    int32_t board_ver;

    gpio_num = conn_get_gpio_number_by_type(HOST_WKUP_W);
    board_ver = get_mpxx_subchip_type();
    if (gpio_num <= 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "host_wakeup_wlan gpio is zero!");
        return -OAL_EIO;
    }
    if (board_ver >= BOARD_VERSION_MP16 && board_ver <= BOARD_VERSION_GF61) {
        gpio_base_addr = (board_ver == BOARD_VERSION_MP16 ? GPIO_BASE_ADDR_MP16 : GPIO_BASE_ADDR_MP16C);
        wlan_host2dev_gpio = WLAN_HOST2DEV_GPIO_MP16;
    } else {
        gpio_base_addr = GPIO_BASE_ADDR;
        wlan_host2dev_gpio = WLAN_HOST2DEV_GPIO;
    }
    ssi_address = gpio_base_addr + GPIO_INOUT_CONFIG_REGADDR;
    ret = mp13_ssi_set_gpio_direction(ssi_address, wlan_host2dev_gpio, GPIO_DIRECTION_INPUT);
    if (ret < 0) {
        return ret;
    }

    ssi_address = gpio_base_addr + GPIO_LEVEL_GET_REGADDR;
    for (i = 0; i < ul_test_times; i++) {
        ret = mp13_ssi_check_gpio_value(gpio_num, GPIO_HIGHLEVEL, ssi_address, wlan_host2dev_gpio);
        if (ret < 0) {
            return ret;
        }

        ret = mp13_ssi_check_gpio_value(gpio_num, GPIO_LOWLEVEL, ssi_address, wlan_host2dev_gpio);
        if (ret < 0) {
            return ret;
        }
        oal_print_mpxx_log(MPXX_LOG_INFO, "check h2d wakeup io %d times ok", i + 1);
    }

    oal_print_mpxx_log(MPXX_LOG_INFO, "check h2d wakeup io done");
    return 0;
}

OAL_STATIC int32_t mp13_test_device_io(void)
{
    int32_t ret;
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if (wlan_pm_info == NULL) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan_pm_info is NULL!");
        return -FAILURE;
    }

    hcc_bus_and_wake_lock(wlan_pm_info->pst_bus);
    /* power on wifi, need't download firmware */
    ret = board_power_on(W_SYS);
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "power on wlan failed=%d", ret);
        hcc_bus_and_wake_unlock(wlan_pm_info->pst_bus);
        return ret;
    }

    hcc_bus_power_action(hcc_get_bus(HCC_EP_WIFI_DEV), HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    ret = hcc_bus_reinit(wlan_pm_info->pst_bus);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "reinit bus %d failed, ret=%d", wlan_pm_info->pst_bus->bus_type, ret);
        goto error;
    }

    wlan_pm_init_dev();

    ret = mp13_check_device_ready();
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_device_ready failed, ret=%d", ret);
        goto error;
    }

    /* check io */
    ret = mp13_check_host_wakeup_wlan();
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_host_wakeup_wlan failed, ret=%d", ret);
        goto error;
    }

    ret = mp13_check_wlan_wakeup_host();
    if (ret) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "check_wlan_wakeup_host failed, ret=%d", ret);
        goto error;
    }

    (void)board_power_off(W_SYS);
    hcc_bus_and_wake_unlock(wlan_pm_info->pst_bus);
    return OAL_SUCC;

error:
    if (!g_ft_fail_powerdown_bypass) {
        (void)board_power_off(W_SYS);
    }
    hcc_bus_and_wake_unlock(wlan_pm_info->pst_bus);
    return ret;
}

int32_t hi1103_dev_io_test(void)        // GPIO测试接口
{
    int32_t ret;
    declare_time_cost_stru(cost);
    struct pm_top* pm_top_data = pm_get_top();

    if (oal_mutex_trylock(&pm_top_data->host_mutex) == 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "get lock fail!");
        return -EINVAL;
    }

    if (!bfgx_is_shutdown()) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "bfgx is open, test abort!");
        bfgx_print_subsys_state(BUART);
        oal_mutex_unlock(&pm_top_data->host_mutex);
        return -OAL_ENODEV;
    }

    if (!wlan_is_shutdown()) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "wlan is open, test abort!");
        oal_mutex_unlock(&pm_top_data->host_mutex);
        return -OAL_ENODEV;
    }

    oal_get_time_cost_start(cost);

    ret = mp13_test_device_io();
    if (ret < 0) {
        oal_print_mpxx_log(MPXX_LOG_ERR, "dev io test fail!");
    }
    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_mpxx_log(MPXX_LOG_INFO, "mp13 device io test cost %llu us", time_cost_var_sub(cost));

    oal_mutex_unlock(&pm_top_data->host_mutex);
    return ret;
}
EXPORT_SYMBOL(hi1103_dev_io_test);
