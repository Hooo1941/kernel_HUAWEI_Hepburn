/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 */

#ifndef _LITE_HCK_CODE_DECRYPT_H
#define _LITE_HCK_CODE_DECRYPT_H

#include <linux/fs.h>
#include <platform_include/basicplatform/linux/lite_vendor_hooks.h>

#ifndef CONFIG_HCK
#undef CALL_HCK_LITE_HOOK
#define CALL_HCK_LITE_HOOK(name, args...)
#undef REGISTER_HCK_LITE_HOOK
#define REGISTER_HCK_LITE_HOOK(name, probe)
#undef REGISTER_HCK_LITE_DATA_HOOK
#define REGISTER_HCK_LITE_DATA_HOOK(name, probe, data)
#else
DECLARE_HCK_LITE_HOOK(get_decrypt_file_lhck,
	TP_PROTO(struct file **file, unsigned long off, unsigned long size),
	TP_ARGS(file, off, size));

#endif /* CONFIG_HCK */

#endif /* _LITE_HCK_CODE_DECRYPT_H */

