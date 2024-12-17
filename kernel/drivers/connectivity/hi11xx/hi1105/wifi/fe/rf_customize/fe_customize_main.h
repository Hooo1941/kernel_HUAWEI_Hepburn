/*
 * 版权所有 (c) 华为技术有限公司 2015-2018
 * 功能说明   : wifi定制化头文件声明
 * 作者       : wifi
 * 创建日期   : 2015年10月08日
 */
/* 宏定义 */
#ifndef FE_CUSTOMIZE_MAIN_H
#define FE_CUSTOMIZE_MAIN_H
#include "customize_spec.h"
#include "fe_pub_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */
extern uint8_t *g_fe_cust_info;
uint8_t *fe_get_customize_mem_addr(void);
uint32_t fe_get_customize_mem_size(void);
void fe_set_customize_mem_addr(uint8_t *addr);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif // fe_customiz_main.h
