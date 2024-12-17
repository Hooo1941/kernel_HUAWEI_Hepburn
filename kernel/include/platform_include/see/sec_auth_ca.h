/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: sec auth ca header
 * Create: 2022/11/25
 */
#ifndef __SEC_AUTH_CA_H__
#define __SEC_AUTH_CA_H__

#include <linux/types.h>
#include "cert_item.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PARTITION_NAME_LEN_MAX                    36u
#define IMAGE_NAME_LEN_MAX                        36u
#define SEC_AUTH_CERT_SIZE                        0x2000    /* 8K */

#define SEC_AUTH_NAME                             "sec_auth"

/* sec_auth TA */
/* 2f4a54a1-2b3d-4ec7-914e-c4f770905f7a */
#define TEE_SERVICE_SECAUTH \
{ \
	0x2F4A54A1, \
	0x2B3D, \
	0x4EC7, \
	{ \
		0x91, 0x4E, 0xC4, 0xF7, 0x70, 0x90, 0x5F, 0x7A \
	} \
}

/* sec_auth TA cmd_id */
enum SEC_AUTH_CMD_ID {
	SEC_AUTH_CMD_ID_INVALID = 0x0,
	SEC_AUTH_CMD_ID_INIT,
	SEC_AUTH_CMD_ID_READ_CERT,
	SEC_AUTH_CMD_ID_READ_DATA,
	SEC_AUTH_CMD_ID_VERIFY,
	SEC_AUTH_CMD_ID_FINAL,
	SEC_AUTH_CMD_ID_READ_CERT_PARSE_DATA_SIZE,
	SEC_AUTH_CMD_ID_MAX
};

struct image_id_t {
	char partition_name[PARTITION_NAME_LEN_MAX];
	char image_name[IMAGE_NAME_LEN_MAX];
};

#define CERT_ITEMS_FETCH_MAX                                    8u

struct authed_data {
	struct cert_item items[CERT_ITEMS_FETCH_MAX];
};

#ifdef __cplusplus
}
#endif
#endif
