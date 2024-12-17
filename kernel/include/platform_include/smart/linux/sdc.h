/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: sdc header file
 * Create: 2022/07/25
 */
#ifndef __SDC_H__
#define __SDC_H__

#include <linux/types.h>

struct sdc_mem_info {
	u64 phy_addr;
	u32 size;
};

/**
 * @brief get base physical address of sdc buf.
 *
 * @return struct sdc_mem_info *: Always not null.If sdc is not supported, function sdc_buf_get will not be compiled.
 */
struct sdc_mem_info *sdc_buf_get(void);

/**
 * @brief Reserved interface.Currently, this interface is empty implementation.
 *
 * @param info : mem info, use the return value of sdc_buf_get as the input param.
 * @return void.
 */
void sdc_buf_put(struct sdc_mem_info *info);


#endif