/*
 * 版权所有 (c) 华为技术有限公司 2015-2018
 * 功能说明   : wifi定制化函数实现
 * 作者       : wifi
 * 创建日期   : 2020年5月20日
 */
/* 头文件包含 */
#include "fe_customize_main.h"
#if defined(_PRE_FE_IS_HOST)
#include "fe_depend_linux_os_if.h"
#endif

void fe_set_customize_mem_addr(uint8_t *addr)
{
    g_fe_cust_info = addr;
}

uint8_t *fe_get_customize_mem_addr(void)
{
    if (g_fe_cust_info == NULL) {
        return NULL;
    }
    return g_fe_cust_info;
}
uint32_t fe_get_customize_mem_size(void)
{
    return FE_CUSTOMIZATION_LEN;
}
