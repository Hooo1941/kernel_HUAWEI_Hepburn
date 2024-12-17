/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: dev_secinfo header
 * Create: 2023/3/31
 */
#ifndef __DEV_SECINFO_H__
#define __DEV_SECINFO_H__
#include <linux/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DEV_SECINFO_OK                           0x0
#define DEV_SECINFO_ERR_FUNC_UNSUPPORT           0x8ff7ffff

#ifdef CONFIG_SECURITY_INFO
u32 dev_secinfo_set_uapp_enable_state(u32 state);
u32 dev_secinfo_socid(u8 *buf, u32 len);

#else /* CONFIG_SECURITY_INFO */
static inline u32 dev_secinfo_set_uapp_enable_state(u32 state)
{
	(void)state;
	return DEV_SECINFO_ERR_FUNC_UNSUPPORT;
}

static inline u32 dev_secinfo_socid(u8 *buf, u32 len)
{
	(void)buf;
	(void)len;
	return DEV_SECINFO_ERR_FUNC_UNSUPPORT;
}
#endif /* CONFIG_SECURITY_INFO */

#ifdef __cplusplus
}
#endif

#endif /* __DEV_SECINFO_H__ */
