/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HOST HAL MAIN
 * 作    者 :
 * 创建日期 : 2020年06月14日
 */

#include "host_hal_main.h"
#include "oneimage.h"
#if (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
#include "board.h"
#endif

int hal_main_init(void)
{
    hal_host_board_init();
    return OAL_SUCC;
}

void hal_main_exit(void)
{
    /* mpxx暂未实现驱动ko卸载功能 */
}
