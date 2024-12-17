/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Declaration Driver Entry Function
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "board.h"
#include "chr_user.h"
#include "plat_debug.h"

#ifdef _PRE_HI_DRV_GPIO
#define WIFI_PRODUCT_VER_OSCA_550 0
#define WIFI_PRODUCT_VER_OSCA_550A 1

#define SSI_CLK_HISI_GPIO 7*8
#define SSI_DATA_HISI_GPIO (6 * 8 + 7)

STATIC void hitv_ssi_gpio_init(void)
{
    hitv_gpio_request(SSI_CLK_HISI_GPIO, 0);
    hitv_gpio_direction_output(SSI_CLK_HISI_GPIO, GPIO_HIGHLEVEL);
    hitv_gpio_request(SSI_DATA_HISI_GPIO, 0);
    hitv_gpio_direction_output(SSI_DATA_HISI_GPIO, GPIO_HIGHLEVEL);
}
#endif

#ifdef _PRE_SUSPORT_OEMINFO
int32_t mp13_get_wifi_calibrate_in_oeminfo_state(const char *node, const char *prop, int32_t *prop_val)
{
    int32_t ret;
    struct device_node *np = NULL;

    if (node == NULL || prop == NULL || prop_val == NULL) {
        ps_print_suc("func has NULL input param!\n");
        return BOARD_FAIL;
    }

    ret = get_board_dts_node(&np, node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = of_property_read_u32(np, prop, prop_val);
    if (ret) {
        ps_print_suc("can't get dts prop val: prop=%s\n", prop);
        return ret;
    }

    return BOARD_SUCC;
}

int32_t is_hitv_miniproduct(void)
{
    int32_t wifi_calibrate_in_oeminfo = 0;
    int32_t ret;
    ret = mp13_get_wifi_calibrate_in_oeminfo_state(DTS_NODE_MPXX_WIFI, DTS_WIFI_CALIBRATE_IN_OEMINFO,
                                                   &wifi_calibrate_in_oeminfo);
    if (ret != BOARD_SUCC) {
        return ret;
    }

    if (wifi_calibrate_in_oeminfo == 1) {
        ps_print_info("hitv is mini product.\n");
        return BOARD_SUCC;
    }
    return BOARD_FAIL;
}
#endif
