/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file describe GPU interface/callback
 * Author: Huawei Technologies Co., Ltd.
 * Create: 2019-2-24
 */
#ifndef GPU_HOOK_H
#define GPU_HOOK_H

#include <linux/types.h>

#if defined(CONFIG_QCOM_KGSL) && defined(CONFIG_GPU_FREQ_STAT)
#include <linux/msm_kgsl.h>
#endif

/* GPU AI frequency schedule interface */
struct kbase_fence_info {
	pid_t game_pid;
	u64 signaled_seqno;
};

#ifdef CONFIG_GPU_FREQ_STAT
struct gpu_freq_stats {
#ifdef CONFIG_QCOM_KGSL
	struct kgsl_gpu_freq_stats kgsl_freq_stats;
#endif
};
#endif

#ifdef CONFIG_GPU_AI_FENCE_INFO
int perf_ctrl_get_gpu_fence(void __user *uarg);
int perf_ctrl_get_gpu_buffer_size(void __user *uarg);
#ifdef CONFIG_GPU_FREQ_STAT
int perf_ctrl_get_gpu_freq_stats(void __user *uarg);
#else
static inline int perf_ctrl_get_gpu_freq_stats(void __user *uarg)
{
	return -EINVAL;
}
#endif
#else
static inline int perf_ctrl_get_gpu_fence(void __user *uarg)
{
	return -EINVAL;
}
static inline int perf_ctrl_get_gpu_buffer_size(void __user *uarg)
{
	return -EINVAL;
}
static inline int perf_ctrl_get_gpu_freq_stats(void __user *uarg)
{
	return -EINVAL;
}
#endif

#if defined(CONFIG_GPU_AI_FENCE_INFO) && defined(CONFIG_MALI_LAST_BUFFER)
int perf_ctrl_enable_gpu_lb(void __user *uarg);
#else
static inline int perf_ctrl_enable_gpu_lb(void __user *uarg)
{
	return -EINVAL;
}
#endif

#endif /* GPU_HOOK_H */
