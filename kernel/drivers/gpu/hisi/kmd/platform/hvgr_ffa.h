/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef HVGR_FFA_H
#define HVGR_FFA_H

#include <linux/types.h>
#include "hvgr_defs.h"

enum hvgr_ffa_ops {
	HVGR_FFA_SMMU_TBU_BYPASS,
	HVGR_FFA_SMMU_TBU_LINK,
	HVGR_FFA_SMMU_TBU_UNLINK,
	HVGR_FFA_SEC_CONFIG,
	HVGR_FFA_CRG_RESET,
	HVGR_FFA_SET_MODE,
	HVGR_FFA_SMMU_SET_SID,
	HVGR_FFA_SET_QOS,
	HVGR_FFA_SH_RUNTIME,
	HVGR_FFA_ENABLE_ECC,
	HVGR_FFA_GET_ECC_STATUS,
	HVGR_FFA_OPS_MAX
};

uint64_t hvgr_ffa_call(struct hvgr_device *gdev, enum hvgr_ffa_ops ops,
	unsigned long value1, unsigned long value2);

uint64_t hvgr_ffa_call_with_shmem(struct hvgr_device *gdev, enum hvgr_ffa_ops ops,
	unsigned char *buf, unsigned long size);

#endif