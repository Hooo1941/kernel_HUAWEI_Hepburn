/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 前端对外接口的声明.
 * Create: 2023-02-03
 */

#ifndef FE_EXTERN_IF_H
#define FE_EXTERN_IF_H
#if (defined(_PRE_FE_IS_DEVICE)) // 只在device调用的接口
#include "fe_depend_os_if.h"
#endif
// 上电早期的fe初始化
void fe_init_when_power_on(void);
// 校准前的fe初始化
void fe_init_before_cali(void);

typedef enum {
    FE_EXT_IF_CB_START = 0,

    // device host都有调用的接口
    FE_EXT_IF_CB_RF_FSM_GET_STATE,         /* fe_rf_fsm_get_state */
    FE_CUSTOM_GET_CUSTOMIZE_SIZE,
    FE_CUSTOM_GET_MEM_ADDR,
    FE_CUSTOM_SET_MEM_ADDR,

#if (!defined(_PRE_FE_IS_HD_SEPARATE) || defined(_PRE_IS_DEVICE)) // 只在device调用的接口
    FE_EXT_IF_CB_RF_SET_CHANNEL,           /* fe_rf_set_rf_channel */
    RF_GET_BASE_POWER,                     /* fe_rf_get_base_power */
    FE_CUSTOM_MAIN_INIT,
    FE_CUSTOM_FLUSH_POW_REGION,
    FE_GET_BW_CAP_BY_MODE,
    FE_CHECK_FEM_TX_ABNORMAL,
#endif

#if (!defined(_PRE_FE_IS_HD_SEPARATE) || defined(_PRE_IS_HOST)) // 只在host调用的接口
    FE_CUSTOM_HOST_READ_INI_PARSE,
    FE_CUSTOM_UPDATE_POW_REGION,
    FE_CUSTOM_CONFIG_UPDATE_POW,
    FE_GET_CUSTOM_MEM_AND_SIZE,
    FE_CUSTOM_GET_SECTION_INFO,
#endif

    FE_EXT_IF_CB_END,
} fe_ext_if_cb_id;

// 根据枚举获取前端外部接口回调
void *fe_get_out_if_cb(fe_ext_if_cb_id func_id);

#endif
