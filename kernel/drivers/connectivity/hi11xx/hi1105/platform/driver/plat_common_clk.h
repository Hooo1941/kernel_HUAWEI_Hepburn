/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_common_clk.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLATFORM_COMMON_CLK_H
#define PLATFORM_COMMON_CLK_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "plat_type.h"

#define DCXO_DEVICE_MAX_BUF_LEN 64
enum coeff_index_enum {
    COEFF_A0 = 0,
    COEFF_A1 = 1,
    COEFF_A2 = 2,
    COEFF_A3 = 3,
    COEFF_T0 = 4,
    COEFF_NUM
};

/* 与上层GNSS约定的结构 */
typedef enum {
    CLK_MODE_TCXO   = 0,
    CLK_MODE_DCXO   = 1,
    CLK_MODE_ABSENT = 2,
    CLK_MODE_XO     = 3,
    CLK_MODE_BUFF
} clk_mode_enum;

/* 与上层GNSS约定的结构 */
typedef enum {
    CLK_NONE = 0,
    CLK_26M  = 1,
    CLK_38P4 = 2,
    CLK_48M  = 3,
    CLK_76P8 = 4,
    CLK_96   = 5,
    CLK_156  = 6,
    CLK_BUFF
} clk_freq_enum;

typedef struct {
    uint16_t cali_end;
    uint16_t auto_cali_ave0;
    uint16_t auto_cali_ave1;
    uint16_t auto_cali_ave2;
    uint16_t auto_cali_ave3;
    uint16_t wifi_ana_en;
    uint16_t xoadc_state;
    uint16_t reg_addr_low;
    uint16_t reg_addr_high;
    uint16_t pmu_type;
} dcxo_pmu_addr_stru;

typedef union {
    struct {
        uint32_t main_type     : 4; /* 0:3 */
        uint32_t main_clk_freq : 4; /* 4:7 */
        uint32_t sub_type      : 4; /* 8:11 */
        uint32_t sub_clk_freq  : 4; /* 12:15 */
    } bits;
    uint32_t as_dword;
} plat_clk_mode_str;

typedef struct {
    int32_t coeff_para[COEFF_NUM]; // 存放a0, a1, a2, a3, t0
    int32_t reserve2[2];     // 未使用
    uint32_t dcxo_tcxo_mode;
    dcxo_pmu_addr_stru pmu_addr;
    uint32_t check_data;
} dcxo_dl_para_stru;

void *get_dcxo_data_buf_addr(void);
void board_clk_init(void);
void board_clk_exit(void);
clk_freq_enum board_main_clk_freq(void);
clk_mode_enum board_clk_mode(void);
void update_dcxo_coeff(int32_t *coeff, uint32_t coeff_cnt);
int32_t read_dcxo_cali_data(void);
#endif /* end of plat_common_clk.h */
