/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *  Description: cert item header
 *  Create: 2022/12/17
 */

#ifndef __CERT_ITEM_H__
#define __CERT_ITEM_H__

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CERT_ITEM_VALUE_LEN_MAX                 32u

enum cert_item_id {
	CERT_ITEM_USER_ID = 3,
	CERT_ITEM_HBK_TYPE,
	CERT_ITEM_CERT_VERSION,
	CERT_ITEM_OEM_ID,
	CERT_ITEM_PRODUCT_ID,
	CERT_ITEM_PARTITION_NAME = 8,
	CERT_ITEM_RELEASE_FLAG,
	CERT_ITEM_NV_ID,
	CERT_ITEM_TRUST_NV_VAL,
	CERT_ITEM_NOTRUST_NV_VAL,
	CERT_ITEM_TRUST2_NV_VAL,
	CERT_ITEM_IMAGE_NAME,
	CERT_ITEM_PLAT_INFO = 17,
	CERT_ITEM_HASH_ALG,
	CERT_ITEM_CIPHER_ALG,
	CERT_ITEM_NUM_NONCE,
	CERT_ITEM_CERT_LCS,
	CERT_ITEM_DEBUG_MASK,
	CERT_ITEM_RMA_FLAG,
	CERT_ITEM_SOC_ID = 25,
	CERT_ITEM_CERT_GENE,
	CERT_ITEM_NV,
	CERT_ITEM_SEVD,
	CERT_ITEM_TRUST_PUBK = 51,
	CERT_ITEM_NOTRUST_PUBK,
	CERT_ITEM_SECDBG_SEVD_PUBK,
	CERT_ITEM_AUTH_PUBK,
	CERT_ITEM_FLASH_PUBK,
	CERT_ITEM_IMAGE_NUM = 64,
	CERT_ITEM_IMAGE_HASH,
	CERT_ITEM_IMAGE_LOAD_ADDR,
	CERT_ITEM_IMAGE_LOAD_SIZE = 67,
	CERT_ITEM_IMAGE_STORE_ADDR,
	CERT_ITEM_IMAGE_STORE_SIZE,
	CERT_ITEM_IMAGE_HASH1 = 70,
	CERT_ITEM_TYPE_MAX,
};

struct cert_item {
	uint32_t item_id;
	union {
		struct {
			uint32_t value;
		} integer;
		struct {
			uint8_t buf[CERT_ITEM_VALUE_LEN_MAX];  // store the item value
			uint32_t len;                            // len of item value
		} octets;
	};
};

#ifdef __cplusplus
}
#endif

#endif /* __CERT_ITEM_H__ */
