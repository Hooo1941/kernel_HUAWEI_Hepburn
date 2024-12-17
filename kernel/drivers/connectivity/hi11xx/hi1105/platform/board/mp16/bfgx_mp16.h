/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: bfgx_mp16.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_MP16_H
#define BFGX_MP16_H

/* 其他头文件包含 */
#include "plat_type.h"
#include "hw_bfg_ps.h"

struct bfgx_prj_desc *mp16_bfgx_prj_desc_init(void);
void mp16_prj_desc_remove(void);
#endif // _BFGX_MP16_H_

