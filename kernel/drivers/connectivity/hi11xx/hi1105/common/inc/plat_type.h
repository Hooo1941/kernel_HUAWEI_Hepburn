/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Declare the variable definition type
 * Author: @CompanyNameTag
 */

#ifndef PLAT_TYPE_H
#define PLAT_TYPE_H

#include "platform_oneimage_define.h"
#include <linux/types.h>

/* 宏定义 */
#if (defined(_PRE_PC_LINT) || defined(WIN32))
/*
 * Note: lint -e530 says don't complain about uninitialized variables for
 * this varible.  Error 527 has to do with unreachable code.
 * -restore restores checking to the -save state
 */
#define unref_param(P) (P) = (P)
#else
#define unref_param(P)
#endif

#ifdef _PRE_LINUX_TEST
#define os_str_len(s) strlen(s)
#define STATIC
#else
#define STATIC static
#endif

enum return_type {
    SUCC = 0,
    EFAIL,
};


#endif /* PLAT_TYPE_H */
