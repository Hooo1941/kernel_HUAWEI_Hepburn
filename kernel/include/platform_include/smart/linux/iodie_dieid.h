/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: iodie_dieid.h.
 * Create: 2023-3-18
 */

#ifndef __IODIE_DIEID_H
#define __IODIE_DIEID_H

#if (defined(CONFIG_IO_DIE_STS_DEBUG) && defined(CONFIG_PLATFORM_DIEID))
int hi9030_get_dieid(char *dieid, unsigned int len);
#else
static inline int hi9030_get_dieid(char *dieid, unsigned int buf_len)
{
	return -1;
}
#endif

#endif
