/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: efuse driver head file
 * Create: 2021/2/8
 */

#ifndef __EFUSE_DRIVER_H__
#define __EFUSE_DRIVER_H__
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/string.h>
#include "efuse_kernel_def.h"

/* error code */
#define EFUSE_OK                              0x0u
#define EFUSE_INVALID_PARAM_ERR               0x5D000001u
#define EFUSE_OVERFLOW_ERR                    0x5D000002u
#define EFUSE_SMC_PROC_ERR                    0x5D000004u
#define EFUSE_LOCK_OPS_ERR                    0x5D000008u
#define EFUSE_FFA_PROC_ERR                    0x5D000010u
#define EFUSE_FUNC_UNSUPPORT_ERR              0x5D00FFFFu

#define EFUSE_MAX_SIZE                        4096
#define EFUSEC_GROUP_MAX_COUNT                128
#define EFUSE_BITS_PER_GROUP                  32

#define efuse_div_round_up(nr, d)             (((nr) + (d) - 1) / (d))

// Get i bits mask
#define efuse_bit_mask(i)                     ((1u << (i)) - 1)

// Extract `j` bits from bit `i` in value `x`.
// Means that: MSB..{i+j-1..i}..0
#define efuse_get_bits(x, i, j)               (((x) >> (i)) & bit_mask(j))
#define EFUSE_ITEM_MAX_BYTES                  32

/*
 * Struct for operating the entire efuse item.
 */
struct efuse_item {
	uint32_t *buf;          // the target data will be stored here
	uint32_t buf_size;      // the size of buf in uint32_t
	uint32_t item_vid;      // the virtual id of efuse item to operate
};

/*
 * Struct for operating customized bits-interval within the efuse item.
 */
struct efuse_desc {
	uint32_t *buf;          // the target data will be stored here
	uint32_t buf_size;      // the size of buf in uint32_t
	uint32_t start_bit;     // the start offset in **current item**
	uint32_t bit_cnt;       // the size of efuse bit to be operated
	uint32_t item_vid;      // the virtual id of efuse item to operate
};

#ifdef CONFIG_VENDOR_EFUSE
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
uint32_t efuse_query_item_bitcnt (uint32_t *value, uint32_t item_vid);


/*
 * Common efuse read function operating customized bits-interval within the efuse
 *     item mapped by `item_vid`.
 * The member `buf_size` will be changed as the real size that used to store the read data.
 *
 * @param  desc                  pointer to struct efuse_desc.
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_read_value_t(struct efuse_desc *desc);


/*
 * Read all bit values of the entire efuse item mapped by item_vid.
 * The member `buf_size` will be changed as the real size that used to store the read data.
 *
 * @param  item                  pointer to struct efuse_item.
 *
 * @return
 * EFUSE_OK                      operation succeed
 * other                         error code
 */
uint32_t efuse_read_item(struct efuse_item *item);

#else
static inline uint32_t efuse_query_item_bitcnt (uint32_t *value, uint32_t item_vid)
{
	(void)value;
	(void)item_vid;
	return EFUSE_FUNC_UNSUPPORT_ERR;
}

static inline uint32_t efuse_read_value_t(struct efuse_desc *desc)
{
	(void)desc;
	return EFUSE_FUNC_UNSUPPORT_ERR;
}

static inline uint32_t efuse_read_item(struct efuse_item *item)
{
	(void)item;
	return EFUSE_FUNC_UNSUPPORT_ERR;
}
#endif

enum tag_efuse_log_level {
	LOG_LEVEL_DISABLE = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_TOTAL = LOG_LEVEL_INFO
};

enum tag_efuse_mem_attr {
	EFUSE_MEM_ATTR_NONE = -1,
	EFUSE_MEM_ATTR_HUK = 0,
	EFUSE_MEM_ATTR_SCP,
	EFUSE_MEM_ATTR_AUTHKEY,
	EFUSE_MEM_ATTR_CHIPID,
	EFUSE_MEM_ATTR_TSENSOR_CALIBRATION,
	EFUSE_MEM_ATTR_HUK_RD_DISABLE,
	EFUSE_MEM_ATTR_AUTHKEY_RD_DISABLE,
	EFUSE_MEM_ATTR_DBG_CLASS_CTRL,
	EFUSE_MEM_ATTR_DIEID,
#ifdef CONFIG_DFX_DEBUG_FS
	EFUSE_MEM_ATTR_SLTFINISHFLAG,
#endif
	EFUSE_MEM_ATTR_MODEM_AVS,
	EFUSE_MEM_ATTR_MAX
};

struct tag_efuse_attr_info {
	u32 bits_width;
};

struct tag_efusec_data {
	u32 efuse_group_max;
	phys_addr_t  paddr;
	u8 *vaddr;
	s32 (*atf_fn)(u64, u64, u64, u64);
	struct tag_efuse_attr_info efuse_attrs_from_dts[EFUSE_MEM_ATTR_MAX];
	struct mutex efuse_mutex; /* mutex to limit one caller in a time */
	u32 is_init_success;
};

/* efuse r&w func id */

/* enable and disable debug mode */
#define EFUSE_FN_ENABLE_DEBUG              0xCA000048
#define EFUSE_FN_DISABLE_DEBUG             0xCA000049

/* HEALTH_LEVEL */
#define EFUSE_FN_WR_HEALTH_LEVEL           0xCA000051

/* partial pass2 */
#define EFUSE_FN_RD_PARTIAL_PASSP2         0xCA000052

/* CHIPLEVEL_TESTFLAG */
#define EFUSE_FN_RD_CHIPLEVEL_TESTFLAG     0xCA000053

/* PARTIALGOOD_FLAG */
#define EFUSE_FN_RD_PARTIALGOOD_FLAG       0xCA000054

/* the max num of efuse group for a feature */
#define EFUSE_BUFFER_MAX_NUM               10

#ifndef OK
#define OK                                 0
#endif
#ifndef ERROR
#define ERROR                              (-1)
#endif

#define EFUSE_MODULE_INIT_SUCCESS          0x12345678

#define EFUSE_TIMEOUT_SECOND               1000
#define EFUSE_KCE_LENGTH_BYTES             16
#define EFUSE_DIEID_LENGTH_BYTES           20
#define EFUSE_CHIPID_LENGTH_BYTES          8
#define EFUSE_PARTIAL_PASS_LENGTH_BYTES    3
/* calculate the Byte of auth_key */
#define EFUSE_SECDBG_LENGTH_BYTES          4
#define EFUSE_DESKEW_LENGTH_BYTES          1
/* calculate the Byte of modem_avs */
#define EFUSE_AVS_LENGTH_BYTES(bits)       DIV_ROUND_UP(bits, 8)
#define EFUSE_AVS_MAX_LENGTH_BYTES         3
#define EFUSE_NVCNT_LENGTH_BYTES           4
#define EFUSE_SOCID_LENGTH_BYTES           32
#define EFUSE_SLTFINISHFLAG_LENGTH_BYTES   4

#ifdef CONFIG_VENDOR_EFUSE
s32 get_efuse_dieid_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_chipid_value(u8 *buf, u32 size, u32 timeout);
#if defined(CONFIG_GENERAL_SEE) || defined(CONFIG_CRYPTO_CORE)
s32 get_efuse_hisee_value(u8 *buf, u32 size, u32 timeout);
s32 set_efuse_hisee_value(u8 *buf, u32 size, u32 timeout);
#endif
s32 get_efuse_kce_value(u8 *buf, u32 size, u32 timeout);
s32 set_efuse_kce_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_deskew_value(u8 *buf, u32 size, u32 timeout);
s32 efuse_read_value(u32 *buf, u32 buf_size, u32 func_id);
s32 efuse_write_value(u32 *buf, u32 buf_size, u32 func_id);
s32 get_efuse_avs_value(u8 *buf, u32 buf_size, u32 timeout);
s32 get_partial_pass_info(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_socid_value(u8 *buf, u32 size, u32 timeout);
s32 efuse_update_nvcnt(u8 *buf, u32 size);
#else
static inline s32 get_efuse_dieid_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_chipid_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

#if defined(CONFIG_GENERAL_SEE) || defined(CONFIG_CRYPTO_CORE)
static inline s32 get_efuse_hisee_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 set_efuse_hisee_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}
#endif

static inline s32 set_efuse_kce_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_kce_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_deskew_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 efuse_read_value(u32 *buf, u32 buf_size, u32 func_id)
{
	return OK;
}

static inline s32 efuse_write_value(u32 *buf, u32 buf_size, u32 func_id)
{
	return OK;
}

static inline s32 get_efuse_avs_value(u8 *buf, u32 buf_size, u32 timeout)
{
	return OK;
}
static inline s32 get_partial_pass_info(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}
static inline s32 get_efuse_socid_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}
#endif

#ifdef CONFIG_VENDOR_EFUSE
#define  EFUSE_DIEID_GROUP_START         32
#define  EFUSE_DIEID_GROUP_WIDTH         5
#define  EFUSE_CHIPID_GROUP_START        57
#define  EFUSE_CHIPID_GROUP_WIDTH        2
#define  EFUSE_KCE_GROUP_START           28
#define  EFUSE_KCE_GROUP_WIDTH           4
#endif

#endif /* __EFUSE_DRIVER_H__ */
