/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: define adapter for NVT
 */

#ifndef ADAPTER_NVT_H
#define ADAPTER_NVT_H

#include <hwbootfail/core/adapter.h>
#ifdef __cplusplus
extern "C" {
#endif

#define BFI_DEV_PATH     "/dev/block/by-name/rrecord"
#define PHYS_MEM_PATH    "/dev/block/by-name/dfx"
#define PATH_MAX_LEN     256
#define BF_SIZE_1K       1024

struct partition_info {
	int is_write;                 /* 读写操作 */
	int offset;                   /* 数据偏移量 */
	int length;                   /* 数据长度 */
	char data[BF_SIZE_1K];        /* 读写分区数据 */
	char path[PATH_MAX_LEN];      /* 分区路径 */
};

int nvt_adapter_init(struct adapter *padp);
int bootfail_rw_thread(void *data);

#ifdef __cplusplus
}
#endif
#endif
