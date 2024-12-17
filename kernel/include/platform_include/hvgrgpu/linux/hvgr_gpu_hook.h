/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This file describe GPU interface/callback
 * Create: 2022-10-24
 */
#ifndef HVGR_GPU_HOOK_H
#define HVGR_GPU_HOOK_H

#include <linux/types.h>

/* HVGR GPU AI frequency schedule interface */
struct kmd_fence_info {
	pid_t game_pid;
	u64 signaled_seqno;
};

#ifdef CONFIG_HISI_GPU_AI_FENCE_INFO
int perf_ctrl_get_hvgr_gpu_fence(void __user *uarg);
int perf_ctrl_get_hvgr_gpu_buffer_size(void __user *uarg);
#else
static inline int perf_ctrl_get_hvgr_gpu_fence(void __user *uarg)
{
	return -EINVAL;
}
static inline int perf_ctrl_get_hvgr_gpu_buffer_size(void __user *uarg)
{
	return -EINVAL;
}
#endif

#if defined(CONFIG_HISI_GPU_AI_FENCE_INFO) && defined(CONFIG_HVGR_SYSTEM_CACHE)
int perf_ctrl_hvgr_enable_gpu_lb(void __user *uarg);
#else
static inline int perf_ctrl_hvgr_enable_gpu_lb(void __user *uarg __maybe_unused)
{
	return -EINVAL;
}
#endif

#endif /* HVGR_GPU_HOOK_H */
