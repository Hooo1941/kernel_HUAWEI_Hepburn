/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: dev_secinfo_auth CA header
 * Create: 2023/3/31
 */
#ifndef __DEV_SECINFO_AUTH_CA_H__
#define __DEV_SECINFO_AUTH_CA_H__
#include <linux/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DEV_SECINFO_AUTH_SEND_MAX_SIZE           0x100000  /* 1M */

#define DEV_SECINFO_AUTH_NAME                    "dev_secinfo_auth"

#define TEE_SERVICE_DEV_SECINFO_AUTH \
{ \
	0xb2733d6c, \
	0x3de5, \
	0x4bae,\
	{ \
		0x8b, 0xd9, 0x55, 0xa2, 0xc5, 0x28, 0xb6, 0xaa \
	} \
}

enum dev_secinfo_auth_cmd_id {
	DEV_SECINFO_AUTH_CMD_ID_VERIFY_BF = 0x0,
	DEV_SECINFO_AUTH_CMD_ID_GET_TRAIT_HASH,
	DEV_SECINFO_AUTH_CMD_ID_GET_PACKAGE_VERSION,
	DEV_SECINFO_AUTH_CMD_ID_GET_AUTH_CTX,
	DEV_SECINFO_AUTH_CMD_ID_SET_UAPP_ENABLE_STATE,
	DEV_SECINFO_AUTH_CMD_ID_GET_SOCID,
	DEV_SECINFO_AUTH_CMD_ID_GET_SECCFG,
	DEV_SECINFO_AUTH_CMD_ID_GET_SE_TYPE,
	DEV_SECINFO_AUTH_CMD_ID_GET_DCU_STATUS,
	DEV_SECINFO_AUTH_CMD_ID_GET_EMPOWER_FLAG,
	DEV_SECINFO_AUTH_CMD_ID_EFUSE_TC,
	DEV_SECINFO_AUTH_CMD_ID_MSPE_TEST,
	DEV_SECINFO_AUTH_CMD_ID_MAX
};

#ifdef __cplusplus
}
#endif

#endif /* __DEV_SECINFO_AUTH_CA_H__ */
