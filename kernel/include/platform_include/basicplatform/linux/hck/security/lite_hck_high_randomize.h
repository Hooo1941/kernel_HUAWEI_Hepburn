/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 */

#ifndef _LITE_HCK_HIGH_RANDOMIZE_H
#define _LITE_HCK_HIGH_RANDOMIZE_H

#include <platform_include/basicplatform/linux/lite_vendor_hooks.h>

#ifndef CONFIG_HCK
#define CALL_HCK_LITE_HOOK(name, args...)
#define REGISTER_HCK_LITE_HOOK(name, probe)
#define REGISTER_HCK_LITE_DATA_HOOK(name, probe, data)
#else

DECLARE_HCK_LITE_HOOK(high_randomize_stack_lhck,
	TP_PROTO(unsigned long *random_variable),
	TP_ARGS(random_variable));
	
DECLARE_HCK_LITE_HOOK(high_randomize_brk_lhck,
	TP_PROTO(unsigned long brk),
	TP_ARGS(brk));
	
DECLARE_HCK_LITE_HOOK(high_randomize_mmap_lhck,
	TP_PROTO(unsigned long *rnd),
	TP_ARGS(rnd));


#endif /* CONFIG_HCK */
#endif /* _LITE_HCK_HIDEADDR_H */
