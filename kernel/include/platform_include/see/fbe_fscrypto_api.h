/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: fbex for fs crypto headers
 * Author: Huawei OS Kernel Lab
 * Create : Thu Dec 29 11:29:30 2022
 */

#ifndef FBE_FS_CRYPTO_API_H
#define FBE_FS_CRYPTO_API_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define IV_LEN       64  /* 64 * 8 = 512bits */
#define PUBKEY_LEN   0x41 /* pubkey len, 0x41 for p256 */
#define METADATA_LEN 16 /* 16 * 8 = 128bits */

struct fbex_fscrypto_param {
	u32 cmd;
	u32 idx;
	u32 pubkey_len;
	u8 pubkey[PUBKEY_LEN];
	u32 metadata_len;
	u8 metadata[METADATA_LEN];
};

struct fbex_set_status_param {
	uint32_t user;
	bool lock;
};

struct fbex_lock_screen_param {
	uint32_t user;
	uint32_t file;
};

struct fbex_unlock_screen_param {
	uint32_t user;
	uint32_t file;
	uint32_t iv_len;
	uint8_t iv_data[IV_LEN];
};

#define FBEX_IOC_MAGIC 'e'
#define HISI_FBEX_DIRECT_LOCK_SCREEN     _IOW(FBEX_IOC_MAGIC, 1, struct fbex_lock_screen_param)
#define HISI_FBEX_DIRECT_UNLOCK_SCREEN   _IOWR(FBEX_IOC_MAGIC, 2, struct fbex_unlock_screen_param)
#define HISI_FBEX_SLOT_CLEAN             _IOW(FBEX_IOC_MAGIC, 3, uint32_t)
#define HISI_FBEX_SET_STATUS             _IOW(FBEX_IOC_MAGIC, 4, struct fbex_set_status_param)
#define HISI_FBEX_KEY_RESTORE            _IOW(FBEX_IOC_MAGIC, 5, uint32_t)
#define HISI_FBEX_GET_METADATA_SECE      _IOWR(FBEX_IOC_MAGIC, 6, struct fbex_fscrypto_param)
#define HISI_FBEX_GET_METADATA           _IOR(FBEX_IOC_MAGIC, 7, u8[METADATA_LEN])
#define HISI_FBEX_ENABLE_KDF             _IOW(FBEX_IOC_MAGIC, 9, uint32_t)
#ifdef CONFIG_FBE3_1
#define HISI_FBEX_GEN_SK                 _IOW(FBEX_IOC_MAGIC, 8, uint32_t)
#define HISI_FBEX_MSPC_CHECK             _IOR(FBEX_IOC_MAGIC, 10, uint32_t)
#endif

#endif /* FBE_FS_CRYPTO_API_H */
