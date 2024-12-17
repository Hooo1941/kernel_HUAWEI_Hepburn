/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 前端初始化处理.
 * Create: 2023-02-03
 */
#include "fe_pub_def.h"
#include "fe_extern_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 前端外部接口的回调列表
static void *g_fe_ext_if_cb[FE_EXT_IF_CB_END] = {0};
// 前端外部接口回调的注册
void fe_ext_if_cb_register(fe_ext_if_cb_id func_id, void *func)
{
    if (func_id >= FE_EXT_IF_CB_END) {
        return;
    }
    g_fe_ext_if_cb[func_id] = func;
}

// 根据枚举获取前端外部接口回调
void *fe_get_out_if_cb(fe_ext_if_cb_id func_id)
{
    if (func_id >= FE_EXT_IF_CB_END) {
        return 0;
    }
    return g_fe_ext_if_cb[func_id];
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
