/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_parse_changid.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_PARSE_CHANGID
#define PLAT_PARSE_CHANGID
#include "platform_oneimage_define.h"
#include "plat_type.h"

int32_t plat_read_changid(void);
void plat_free_changid_buffer(void);
void plat_changid_print(void);
#endif // __PLAT_PARSE_CHANGID__
