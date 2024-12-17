/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 * Description: hhee hibernate header file
 * Create : 2023/11/06
 */

#ifndef __HHEE_HIBERNATE_H__
#define __HHEE_HIBERNATE_H__

#include <linux/types.h>

#ifdef CONFIG_HHEE_HIBERNATE
int hhee_hibernate_freeze(void);
int hhee_hibernate_restore(void);
#else
static inline int hhee_hibernate_freeze(void) { return 0; }
static inline int hhee_hibernate_restore(void) { return 0; }
#endif

#endif /* __HHEE_HIBERNATE_H__ */
