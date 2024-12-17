/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function declarment for rpmb
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
 #ifndef __RPMB_H__
#define __RPMB_H__

#include <linux/kconfig.h>
#include <uapi/linux/bsg.h>

#define MAX_CDB_CMD_LENGTH 16
#define UFS_IOC_MAX_RPMB_CMD 3
#define STORAGE_IOC_MAX_RPMB_CMD 3
#define MAX_IOC_RPMB_BYTES (4 * 1024)

enum rpmb_op_type {
	RPMB_OP_RD = 0,
	RPMB_OP_WR_DATA = 1,
	RPMB_OP_WR_CNT = 2
};

enum func_id {
	RPMB_FUNC_ID_RESERVED,
	RPMB_FUNC_ID_SE,
	RPMB_FUNC_ID_SECURE_OS,
	RPMB_FUNC_ID_MAX,
};

enum rpmb_version {
	RPMB_VER_INVALID = 0,
	RPMB_VER_UFS_21 = 21,
	RPMB_VER_UFS_30 = 30,
	RPMB_VER_MAX = 999
};

struct storage_blk_ioc_data {
	unsigned char *buf;
	u64 buf_bytes;
	u32 blocks;
};

struct ufs_blk_ioc_data {
	struct sg_io_v4 siv;
	unsigned char *buf;
	u64 buf_bytes;
};

struct storage_blk_ioc_rpmb_data {
	struct storage_blk_ioc_data data[STORAGE_IOC_MAX_RPMB_CMD];
};

struct ufs_blk_ioc_rpmb_data {
	struct ufs_blk_ioc_data data[UFS_IOC_MAX_RPMB_CMD];
	u8 sdb_command[UFS_IOC_MAX_RPMB_CMD][MAX_CDB_CMD_LENGTH];
};

extern struct mutex rpmb_counter_lock;

extern int vendor_rpmb_ioctl_cmd(
	enum func_id id,
	enum rpmb_op_type operation,
	struct storage_blk_ioc_rpmb_data *storage_data);

#endif /* __RPMB_H__ */
