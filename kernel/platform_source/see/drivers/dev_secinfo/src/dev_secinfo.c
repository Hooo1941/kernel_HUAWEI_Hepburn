/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: dev_secinfo source
 * Create: 2023/3/31
 */
#include <platform_include/see/dev_secinfo.h>
#include <dev_secinfo_by_tee.h>
#include <dev_secinfo_errno.h>
#include <dev_secinfo_plat.h>

u32 dev_secinfo_set_uapp_enable_state(u32 state)
{
	return set_uapp_enable_state_by_tee(state);
}