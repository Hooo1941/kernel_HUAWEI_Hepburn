/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "hvgr_ffa.h"

#include "platform_include/see/ffa/ffa_plat_drv.h"
#include "platform_include/see/ffa/ffa_msg_id.h"

#define FID_BL31_LP_GPU_SH_RUNTIME 0xc3004507

static unsigned long hvgr_ffa_ops_list[HVGR_FFA_OPS_MAX] = {
	FID_BL31_GPU_SMMU_TBU_BYPASS,
	FID_BL31_GPU_SMMU_TBU_LINK,
	FID_BL31_GPU_SMMU_TBU_UNLINK,
	FID_BL31_GPU_SEC_CONFIG,
	FID_BL31_GPU_CRG_RESET,
	FID_BL31_GPU_SET_MODE,
	FID_BL31_GPU_SMMU_SET_SID,
	FID_BL31_GPU_SET_QOS,
	FID_BL31_LP_GPU_SH_RUNTIME,
	FID_BL31_GPU_ENABLE_ECC,
	FID_BL31_GPU_GET_ECC_STATUS,
};

uint64_t hvgr_ffa_call(struct hvgr_device *gdev, enum hvgr_ffa_ops ops,
	unsigned long value1, unsigned long value2)
{
	struct ffa_send_direct_data args = {0};
	int ret;

	if (ops >= HVGR_FFA_OPS_MAX) {
		dev_err(gdev->dev, "ffa input is invalid.");
		return (uint64_t)-EINVAL;
	}

	args.data0 = hvgr_ffa_ops_list[ops];
	args.data1 = value1;
	args.data2 = value2;
	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0) {
		dev_err(gdev->dev, "send ffa msg failed ret=%d.", ret);
		return (uint64_t)-EINVAL;
	}

	return args.data0;
}

uint64_t hvgr_ffa_call_with_shmem(struct hvgr_device *gdev, enum hvgr_ffa_ops ops,
	unsigned char *buf, unsigned long size)
{
	struct ffa_send_direct_data args = {0};
	int ret;

	if (ops >= HVGR_FFA_OPS_MAX || buf == NULL || size == 0u) {
		dev_err(gdev->dev, "ffa shmem input is invalid.");
		return (uint64_t)-EINVAL;
	}

	if (size > FFA_LOCAL_BUFFER_MAX) {
		dev_err(gdev->dev, "ffa shmem size %lu out of range.", size);
		return (uint64_t)-EINVAL;
	}

	args.data0 = hvgr_ffa_ops_list[ops];
	args.data4 = size;
	ret = ffa_platdrv_send_msg_with_shmem(&args, buf, SHMEM_IN);
	if (ret != 0) {
		dev_err(gdev->dev, "send ffa shmem msg failed ret=%d.", ret);
		return (uint64_t)-EINVAL;
	}

	return args.data0;
}

