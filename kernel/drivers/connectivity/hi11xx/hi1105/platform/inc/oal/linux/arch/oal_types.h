/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: oal_types.h header file, definie basic data type
 * Author: @CompanyNameTag
 */

#ifndef OAL_LINUX_TYPES_H
#define OAL_LINUX_TYPES_H
#include <linux/types.h>

/* 宏定义 */
#ifdef INLINE_TO_FORCEINLINE
#define OAL_INLINE __forceinline
#else
#ifdef _PRE_FUZZ_TEST
#define OAL_INLINE static inline
#else
#define OAL_INLINE inline
#endif
#endif

#ifdef _PRE_FUZZ_TEST
#define OAL_STATIC
#else
#define OAL_STATIC static
#endif

#if defined(_PRE_PC_LINT) /* 仅适用于PC_LINT工程时 */
#define OAL_VOLATILE
#else
#define OAL_VOLATILE volatile
#endif

#define OAL_USER  __user

#define OAL_NOTIFY_BAD NOTIFY_BAD  /* 0x8002 kernel NOTIFY_BAD will break notify chain loop */

#endif /* end of oal_types.h */
