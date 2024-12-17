/*
 * npu_platform_resource.h
 *
 * about npu platform resource
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __NPU_PLATFORM_RESOURCE_H
#define __NPU_PLATFORM_RESOURCE_H

#define NPU_MAX_PROCESS_SQ_NUM  40

#define NPU_PLAT_AICORE_MAX     1

#define NPU_AIC_SINK_SHORT_STREAM_SUM   352
#define NPU_AIC_SINK_LONG_STREAM_SUM    32
#define NPU_AIV_SINK_SHORT_STREAM_SUM   128
#define NPU_AIV_SINK_LONG_STREAM_SUM    0
#define NPU_AIC_NON_SINK_STREAM_SUM     256
#define NPU_AIV_NON_SINK_STREAM_SUM     128

#define NPU_MAX_NON_SINK_STREAM_ID      ((NPU_AIC_NON_SINK_STREAM_SUM) + (NPU_AIV_NON_SINK_STREAM_SUM))
#define NPU_MAX_SINK_LONG_STREAM_ID     ((NPU_AIC_SINK_LONG_STREAM_SUM) + (NPU_AIV_SINK_LONG_STREAM_SUM))
#define NPU_MAX_SINK_SHORT_STREAM_ID    ((NPU_AIC_SINK_SHORT_STREAM_SUM) + (NPU_AIV_SINK_SHORT_STREAM_SUM))
#define NPU_MAX_STREAM_ID               ((NPU_MAX_NON_SINK_STREAM_ID) + \
	(NPU_MAX_SINK_LONG_STREAM_ID) + (NPU_MAX_SINK_SHORT_STREAM_ID))

#define NPU_AIC_HWTS_SQ_NUM           384
#define NPU_AIV_HWTS_SQ_NUM           128

#define NPU_MAX_HWTS_SQ_NUM           ((NPU_AIC_HWTS_SQ_NUM) + (NPU_AIV_HWTS_SQ_NUM))
#define NPU_MAX_HWTS_CQ_NUM           30
#define NPU_MAX_HWTS_SQ_DEPTH         1024
#define NPU_MAX_LONG_HWTS_SQ_DEPTH    8192
#define NPU_MAX_HWTS_CQ_DEPTH         1024
#define NPU_MAX_LONG_HWTS_SQ_NUM      32

#define NPU_AIC_MODEL_SUM               192
#define NPU_AIV_MODEL_SUM               64
#define NPU_MAX_MODEL_ID                ((NPU_AIC_MODEL_SUM) + (NPU_AIV_MODEL_SUM))
#define NPU_MAX_PROCESS_MODEL_NUM       ((NPU_AIC_MODEL_SUM) + (NPU_AIV_MODEL_SUM))

#define NPU_PLAT_SYSCACHE_SIZE  0x400000 /* 4M for v110 */

#define NPU_NS_PROF_SIZE        0x1000
#define PROF_HWTS_LOG_SIZE            0x200000
#define PROF_HWTS_PROFILING_SIZE           0x200000
#endif
