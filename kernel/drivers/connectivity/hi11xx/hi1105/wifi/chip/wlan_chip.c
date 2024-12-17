/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 芯片差异文件
 * 作    者 :
 * 创建日期 : 2020年6月19日
 */

#include "oneimage.h"
#include "wlan_chip.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif
#include "oam_ext_if.h"
#include "host_hal_main.h"
#include "host_hal_ops.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WLAN_CHIP_C

OAL_STATIC const struct hal_common_ops_stru g_hal_dummy_ops = { NULL, };
const struct hal_common_ops_stru *g_hal_common_ops = &g_hal_dummy_ops;
uint32_t wlan_hal_ops_init(void)
{
    int32_t chip = get_mpxx_subchip_type();
    /* 根据不同芯片类型，挂接不同回调函数 */
    if (chip == BOARD_VERSION_MP13) {
        g_hal_common_ops = &g_hal_dummy_ops;
    } else if (chip == BOARD_VERSION_MP15) {
        g_hal_common_ops = &g_hal_dummy_ops;
    } else if (chip == BOARD_VERSION_MP16) {
        g_hal_common_ops = &g_hal_common_ops_mp16;
    } else if (chip == BOARD_VERSION_MP16C) {
        g_hal_common_ops = &g_hal_common_ops_mp16c;
    } else if (chip == BOARD_VERSION_GF61) {
        g_hal_common_ops = &g_hal_common_ops_mp16c; // keep gf61 & mp16c the same
    } else {
        g_hal_common_ops = &g_hal_dummy_ops;
        oam_error_log1(0, OAM_SF_ANY, "wlan_hal_ops_init: unsupport chitype!%d\n", get_mpxx_subchip_type());
        return -OAL_EFAIL;
    }
    oal_io_print("wlan_hal_ops_init::chiptype %d.\r\n", get_mpxx_subchip_type());
    return OAL_SUCC;
}

const struct wlan_chip_ops g_wlan_chip_dummy_ops = { NULL, };
const struct wlan_chip_ops *g_wlan_chip_ops = &g_wlan_chip_dummy_ops;
uint32_t wlan_chip_ops_init(void)
{
    /* 根据不同芯片类型，挂接不同回调函数 */
    int32_t chip_type = get_mpxx_subchip_type();
    if (chip_type == BOARD_VERSION_MP13) {
        g_wlan_chip_ops = &g_wlan_chip_ops_mp13;
    } else if (chip_type == BOARD_VERSION_MP15) {
        g_wlan_chip_ops = &g_wlan_chip_ops_mp15;
    } else if (chip_type == BOARD_VERSION_MP16) {
        g_wlan_chip_ops = &g_wlan_chip_ops_mp16;
    } else if (chip_type == BOARD_VERSION_MP16C) {
        g_wlan_chip_ops = &g_wlan_chip_ops_mp16c;
    } else if (chip_type == BOARD_VERSION_GF61) {
        g_wlan_chip_ops = &g_wlan_chip_ops_gf61;
    } else {
        g_wlan_chip_ops = &g_wlan_chip_dummy_ops;
        oam_error_log1(0, OAM_SF_ANY, "wlan_chip_ops_init: wifi unsupport chitype!%d\n",
            get_mpxx_subchip_type());
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

#ifndef _PRE_LINUX_TEST
// 区分不同芯片host侧规格全局变量控制开关
const oal_wlan_cfg_stru *g_wlan_spec_cfg;
#else
oal_wlan_cfg_stru *g_wlan_spec_cfg;
#endif

int32_t wlan_chip_feature_init(void)
{
    int32_t chip = get_mpxx_subchip_type();
    if (chip == BOARD_VERSION_MP13) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_mp13;
    } else if (chip == BOARD_VERSION_MP15) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_mp15;
    } else if (chip == BOARD_VERSION_MP16) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_mp16;
    } else if (chip == BOARD_VERSION_MP16C) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_mp16c;
    } else if (chip == BOARD_VERSION_GF61) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_gf61;
    } else {
        oal_io_print("wifi unsupport chitype!\n");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
