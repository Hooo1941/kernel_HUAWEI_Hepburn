/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT types. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-06-01, Create file. \n
 */
#ifndef TIOT_TYPES_H
#define TIOT_TYPES_H

#ifdef __KERNEL__
/* 内核类型可能与编译器stdint.h冲突. */
#include <linux/kernel.h>
#include <linux/version.h>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_types TIoT types.
 * @ingroup  tiot_common_interface
 * @{
 */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* container */
#undef tiot_offsetof
#ifdef __compiler_offsetof
#define tiot_offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE, MEMBER)
#else
#define tiot_offsetof(TYPE, MEMBER) ((int)(unsigned long)&((TYPE *)0)->MEMBER)
#endif

#ifdef _MSC_VER
#define tiot_container_of(ptr, type, member) ((type *)((char *)(const void *){ptr} - tiot_offsetof(type, member)))
#else
#define tiot_container_of(ptr, type, member) ({                 \
    const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)((char *)__mptr - tiot_offsetof(type, member)); })
#endif

#define tiot_unused(var) \
    do {                 \
        (void)(var);       \
    } while (0)

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif