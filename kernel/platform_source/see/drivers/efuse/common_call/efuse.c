/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: efuse source file to support interfaces
 * Create: 2022/01/18
 */

#include <platform_include/see/efuse_driver.h>
#include "efuse_internal.h"
#include "../efuse_plat.h"            /* efuse_err */
#include <linux/mutex.h>           /* mutex_lock */

static DEFINE_MUTEX(g_efuse_cnt_lock);

/*
 * Query the total BitCnt of an efuse item by item_vid.
 *
 * @param  item_vid: the virtual id of efuse item to query.
 * @param  value: the pointer to store the returned bit_cnt.
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_query_item_bitcnt(uint32_t *value, uint32_t item_vid)
{
	uint32_t ret;
	struct efuse_desc desc = { 0 };

	if (value == NULL) {
		efuse_err("error, input null pointer\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	/* get the real bit_cnt of efuse item */
	desc.item_vid = item_vid;
	ret = efuse_get_item_info(&desc);
	if (ret != EFUSE_OK) {
		efuse_err("error, get item info ret=0x%08X\n", ret);
		return ret;
	}

	*value = desc.bit_cnt;
	return EFUSE_OK;
}

/*
 * Common efuse read function operating customized bits-interval within the efuse
 *     item mapped by `item_vid`.
 * The `buf_size` will be changed as the real size that used to store the read data.
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
uint32_t efuse_read_value_t(struct efuse_desc *desc)
{
	uint32_t ret;

	mutex_lock(&g_efuse_cnt_lock);

	ret = efuse_read_process(desc);

	mutex_unlock(&g_efuse_cnt_lock);
	return ret;
}

/*
 * Read all bit values of the entire efuse item mapped by item_vid.
 * The `buf_size` will be changed as the real size that used to store the read data.
 *
 * @param  item                  pointer to struct efuse_item.
 *  struct efuse_item {
 *      uint32_t *buf;           // the target data will be stored here
 *      uint32_t buf_size;       // the size of buf in uint32_t
 *      uint32_t item_vid;       // the virtual id of efuse item to operate
 *  };
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_read_item(struct efuse_item *item)
{
	uint32_t ret;
	uint32_t bit_cnt = 0;
	struct efuse_desc desc = { 0 };

	if (item == NULL) {
		efuse_err("error, input null pointer\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	ret = efuse_query_item_bitcnt(&bit_cnt, item->item_vid);
	if (ret != EFUSE_OK) {
		efuse_err("error, query bitcnt ret=0x%08X\n", ret);
		return ret;
	}

	desc.buf = item->buf;
	desc.buf_size = item->buf_size;
	desc.start_bit = 0;
	desc.bit_cnt = bit_cnt;
	desc.item_vid = item->item_vid;

	ret = efuse_read_value_t(&desc);
	if (ret != EFUSE_OK) {
		efuse_err("error, efuse_read_value ret=0x%08X\n", ret);
		return ret;
	}

	item->buf_size = desc.buf_size;
	return EFUSE_OK;
}
