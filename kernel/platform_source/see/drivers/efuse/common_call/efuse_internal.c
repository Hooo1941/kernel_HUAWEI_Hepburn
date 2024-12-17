/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: efuse source file to provide drivers
 * Create: 2022/01/18
 */

#include <platform_include/see/efuse_driver.h>
#include <securec.h>               /* memset_s */
#include "efuse_internal.h"
#include "../efuse_utils.h"           /* efuse_min */
#include "../efuse_plat.h"            /* efuse_err */
#include "../efuse_call.h"             /* efuse_call_args */

#ifdef CONFIG_EFUSE_SUPPORT_FFA
#include <platform_include/see/ffa/ffa_msg_id.h> /* ffa_id */
#else
#include <platform_include/see/bl31_smc.h> /* smc_id */
#endif

/*
 * check params
 *
 * @param desc        pointer of struct efuse_desc
 *
 * @return
 * EFUSE_OK           params ok
 * other              invalid params
 */
static uint32_t efuse_check_param(struct efuse_desc *desc)
{
	uint32_t tmp_len;
	uint32_t buf_size;
	uint32_t start_bit;
	uint32_t bit_cnt;

	if (desc == NULL || desc->buf == NULL) {
		efuse_err("error, null pointer\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	buf_size = desc->buf_size;
	start_bit = desc->start_bit;
	bit_cnt = desc->bit_cnt;
	if (start_bit >= EFUSE_MAX_SIZE || bit_cnt > EFUSE_MAX_SIZE ||
	    bit_cnt == 0 || (start_bit > EFUSE_MAX_SIZE - bit_cnt)) {
		efuse_err("error, invalid params, start_bit = %u, bit_cnt = %u\n", start_bit, bit_cnt);
		return EFUSE_INVALID_PARAM_ERR;
	}

	tmp_len = efuse_div_round_up(bit_cnt, EFUSE_BITS_PER_GROUP);
	if (buf_size < tmp_len) {
		efuse_err("error, buf_size=%u, min need_size=%u\n", buf_size, tmp_len);
		return EFUSE_INVALID_PARAM_ERR;
	}

	return EFUSE_OK;
}

/*
 * combine the start_bit and bit_cnt params into one uint32_t because
 *     the params number of smc may be not enough.
 *
 * @param  start_bit  efuse start bit
 * @param  bit_cnt    efuse bit cnt
 *
 * @return
 * combine result
 */
static uint32_t efuse_combine_bit_param(uint32_t start_bit, uint32_t bit_cnt)
{
	uint32_t result;

	result = (start_bit << EFUSE_BITCNT_ALLOC_BITS) | bit_cnt;
	return result;
}

/*
 * bottom layer function of read efuse value
 * only support not more than 32 bit efuse for each operation now
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
 * EFUSE_OK           operation succeed
 * other              error code
 */
#ifdef CONFIG_EFUSE_SUPPORT_FFA
static uint32_t read_single_interval(struct efuse_desc *desc)
{
	uint32_t ret;
	struct efuse_call_args args = { 0 };

	ret = efuse_check_param(desc);
	if (ret != EFUSE_OK || desc->bit_cnt > EFUSE_BITS_PER_GROUP) {
		efuse_err("error, invalid params\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	args.arg0 = (uintptr_t)EFUSE_TZSP_KERNEL_READ_EFUSE;
	args.arg1 = (uintptr_t)desc->item_vid;
	args.arg2 = (uintptr_t)efuse_combine_bit_param(desc->start_bit, desc->bit_cnt);
	ret = efuse_ffa(&args);
	if (ret != EFUSE_OK) {
		efuse_err("error, ffa failed, ffa_id=0x%08X, ret=0x%08X, item_vid=0x%08X\n", \
			  EFUSE_TZSP_KERNEL_READ_EFUSE, ret, desc->item_vid);
		return EFUSE_FFA_PROC_ERR;
	}

	*(desc->buf) = args.arg1;
	return EFUSE_OK;
}
#else
static uint32_t read_single_interval(struct efuse_desc *desc)
{
	uint32_t ret;
	struct efuse_call_args args = { 0 };

	ret = efuse_check_param(desc);
	if (ret != EFUSE_OK || desc->bit_cnt > EFUSE_BITS_PER_GROUP) {
		efuse_err("error, invalid params\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	args.arg0 = (uintptr_t)EFUSE_BL31_KERNEL_READ_EFUSE;
	args.arg1 = (uintptr_t)desc->item_vid;
	args.arg2 = (uintptr_t)efuse_combine_bit_param(desc->start_bit, desc->bit_cnt);
	ret = efuse_smc(&args);
	if (ret != EFUSE_OK) {
		efuse_err("error, smc failed, smc_id=0x%08X, ret=0x%08X, item_vid=0x%08X\n", \
			  args.arg0, ret, desc->item_vid);
		return EFUSE_SMC_PROC_ERR;
	}

	*(desc->buf) = args.arg1;
	return EFUSE_OK;
}
#endif

/*
 * get efuse item information by item_vid.
 *
 * @param  desc       pointer to struct efuse_desc
 *
 * @return
 * EFUSE_OK           operation succeed
 * other              error code
 */
#ifdef CONFIG_EFUSE_SUPPORT_FFA
uint32_t efuse_get_item_info(struct efuse_desc *desc)
{
	uint32_t ret;
	struct efuse_call_args args = { 0 };

	args.arg0 = (uintptr_t)EFUSE_TZSP_KERNEL_GET_ITEM_INFO;
	args.arg1 = (uintptr_t)desc->item_vid;
	ret = efuse_ffa(&args);
	if (ret != EFUSE_OK) {
		efuse_err("error, ffa failed, ffa_id=0x%08X, ret=0x%08X, item_vid=0x%08X\n", \
			  EFUSE_TZSP_KERNEL_GET_ITEM_INFO, ret, desc->item_vid);
		return EFUSE_FFA_PROC_ERR;
	}

	desc->bit_cnt = args.arg1;
	return EFUSE_OK;
}
#else
uint32_t efuse_get_item_info(struct efuse_desc *desc)
{
	uint32_t ret;
	struct efuse_call_args args = { 0 };

	args.arg0 = (uintptr_t)EFUSE_BL31_KERNEL_GET_ITEM_INFO;
	args.arg1 = (uintptr_t)desc->item_vid;
	ret = efuse_smc(&args);
	if (ret != EFUSE_OK) {
		efuse_err("error, smc failed, smc_id=0x%08X, ret=0x%08X, item_vid=0x%08X\n", \
			  args.arg0, ret, desc->item_vid);
		return EFUSE_SMC_PROC_ERR;
	}

	desc->bit_cnt = args.arg1;
	return EFUSE_OK;
}
#endif

/*
 * Inner layer function of read efuse value.
 *
 * In kernel, the caller doesn't need to know specific efuse layout, especially
 *     the real start bit and bit cnt. We only provide the item_vid to support the
 *     operation of an valid interval within this efuse item.
 *
 * However, there is no share memory between kernel and bl31, we cannot operate the
 *     whole efuse interval when the interval includes more than 1 groups due to the length
 *     limition of smc parameters.
 *
 * Hence, we need to divide the item into several parts so that each interval could
 *     be operated with a uint32_t. Therefore
 * 1.we split the original interval by a given limitation length
 * 2.use the start_bit and bit_cnt parameters to mark different parts
 *
 * The start_bit here is a concept of offset in the corresponding efuse item and will
 *     be mapped to the real start_bit in atf. See below:
 *
 *   kernel:            +<--------+ bit_cnt +------->+
 *                      |                            |
 *                      |       start_bit=32     start_bit=0
 *                      |            |               |
 *                      +<-cur_cnt-->+<---cur_cnt--->+               +
 *                      |            |               |
 *                      v            v               v
 *   +---------------+-------+-------+---------------+
 *   |               |               |               |
 *   +---------------+---------------+---------------+
 *   95             64              32               0
 *                           +
 *                           |
 *                           v
 *
 *   atf:         +<--------+ bit_cnt +------->+
 *                |                            |
 *                |        start_bit       start_bit
 *                |            |               |
 *                +<-cur_cnt-->+<---cur_cnt--->+                    +
 *                |            |               |
 *                v            v               v
 *   +------------+--+---------+-----+---------+-----+
 *   |               |               |               |
 *   +---------------+---------------+---------------+
 *   95             64              32               0
 *
 *   @param desc      pointer to struct efuse_desc
 *
 *   @return
 *   EFUSE_OK         operation succeed
 *   other            error code
 */
static uint32_t split_deal_read_intervals(struct efuse_desc *desc)
{
	uint32_t ret;
	uint32_t efuse_val = 0;
	uint32_t start_bit = desc->start_bit;
	uint32_t bit_cnt = desc->bit_cnt;
	uint32_t cur_cnt;
	uint32_t grp_cnt = 0;
	struct efuse_desc worker = { 0 };

	worker.buf = &efuse_val;
	worker.buf_size = (uint32_t)(sizeof(efuse_val) / sizeof(uint32_t));
	worker.item_vid = desc->item_vid;

	// iteratively read each cur_cnt
	while (bit_cnt > 0) {
		cur_cnt = efuse_min(bit_cnt, (uint32_t)EFUSE_BITS_PER_GROUP);
		worker.start_bit = start_bit;
		worker.bit_cnt = cur_cnt;
		ret = read_single_interval(&worker);
		if (ret != EFUSE_OK) {
			(void)memset_s(desc->buf, desc->buf_size * sizeof(uint32_t), 0, \
				       desc->buf_size * sizeof(uint32_t));
			efuse_err("error,read efuse failed, start_bit=%u, cur_cnt=%u\n", start_bit, cur_cnt);
			return ret;
		}

		desc->buf[grp_cnt++] = efuse_val;
		start_bit += cur_cnt;
		bit_cnt -= cur_cnt;
	}

	desc->buf_size = grp_cnt;
	return EFUSE_OK;
}

/*
 * The inner read process.
 * 1. Check parameters.
 * 2. Split the interval and deal with each of them.
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_read_process(struct efuse_desc *desc)
{
	uint32_t ret;

	if (desc == NULL) {
		efuse_err("error, input null pointer\n");
		return EFUSE_INVALID_PARAM_ERR;
	}

	ret = efuse_check_param(desc);
	if (ret != EFUSE_OK) {
		efuse_err("error, invalid params\n");
		return ret;
	}

	ret = split_deal_read_intervals(desc);
	if (ret != EFUSE_OK) {
		efuse_err("error, split read ret=0x%08X\n", ret);
		return ret;
	}

	return EFUSE_OK;
}
