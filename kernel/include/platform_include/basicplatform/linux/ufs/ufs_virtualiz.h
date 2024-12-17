/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: ufs virtualization
 * Create: 2023-02-01
 */

#ifndef __UFS_VIRTUALIZ_H__
#define __UFS_VIRTUALIZ_H__

#include <linux/types.h>

typedef enum {
	SHMEC_MODULE_UFS_CRYPTO = 0,
	SHMEC_MODULE_UFS_BOOTDEVICE,
	SHMEC_MODULE_UFS_BOOT_PARTITION,
	SHMEC_MODULE_MAX_NUM
} shmec_module;

#define MAX_SHMEC_NAME_SIZE 32
#define MAX_SHMEC_BUF_SIZE 128

#define SHMEC_UFS_CRYPTO "shmec_ufs_crypto"
#define SHMEC_UFS_BOOTDEVICE "shmec_ufs_bootdeivce"
#define SHMEC_UFS_BOOT_PARTITION "shmec_ufs_boot_partition"

#endif
