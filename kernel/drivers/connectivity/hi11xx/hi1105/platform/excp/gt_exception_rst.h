/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gt_exception_rst.c header file
 * Author: @CompanyNameTag
 */

#ifndef GT_EXCEPTION_RST_H
#define GT_EXCEPTION_RST_H

#include "plat_type.h"

int32_t gt_device_reset(void);
void gt_memdump_finish(void);
int32_t gt_notice_hal_memdump(void);
int32_t gt_memdump_enquenue(uint8_t *buf_ptr, uint16_t count);

#endif
