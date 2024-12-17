/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: efuse header file used for driver
 * Create: 2022/01/19
 */

#ifndef __EFUSE_INTERNAL_H__
#define __EFUSE_INTERNAL_H__

#include <platform_include/see/efuse_driver.h>
#include <linux/types.h>                       /* uint32_t */

/*
 * Get efuse item information by item_vid.
 *
 * @param  desc                 pointer to struct efuse_desc.
 *  struct efuse_desc {
 *      uint32_t *buf;          // the target data will be stored here
 *      uint32_t buf_size;      // the size of buf in uint32_t
 *      uint32_t start_bit;     // the start offset in **current item**
 *      uint32_t bit_cnt;       // the size of efuse bit to be operated
 *      uint32_t item_vid;      // the virtual id of efuse item to operate
 *  };
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_get_item_info(struct efuse_desc *desc);

/*
 * The inner read process.
 * 1. Check parameters.
 * 2. Split the interval and deal with each of them.
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_read_process(struct efuse_desc *desc);

#endif /* __EFUSE_INTERNAL_H__ */
