/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: hisp config base header file.
 * Create: 2016-04-01
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, MA 02111-1307 USA
 */

#ifndef __HW_KERNEL_CAM_HISP_CFG_BASE_H__
#define __HW_KERNEL_CAM_HISP_CFG_BASE_H__

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <platform_include/camera/native/camera.h>
#include <platform_include/isp/linux/hisp_mempool.h>
#include <linux/compat.h>
#include <linux/iommu.h>

enum {
	HISP_NAME_SIZE = 32,
	HISP_V4L2_EVENT_TYPE = V4L2_EVENT_PRIVATE_START + 0x00090000,

	HISP_HIGH_PRIO_EVENT = 0x1500,
	HISP_MIDDLE_PRIO_EVENT = 0x2000,
	HISP_LOW_PRIO_EVENT = 0x3000,
};

typedef enum _tag_hisp_event_kind {
	HISP_RPMSG_CB,
} hisp_event_kind_t;

#define HISP_MESSAGE_ID_CLIENT_BIT 29
typedef struct _tag_hisp_event {
	hisp_event_kind_t kind; // possible with client_id
} hisp_event_t;

struct hisp_map_info_t {
	int fd;
	unsigned int size;
	unsigned int fw_mem_size;
	unsigned int ispfw_mem_size;
	unsigned int fw_mem_prot;
	unsigned int ispfw_mem_prot;
};

struct hisp_isp_mem_info_t {
#ifdef ISP_LINX_CPU_SUPPORT
	unsigned long long cpu_addr;
#else
	unsigned int cpu_addr;
#endif
	unsigned int core_addr;
	unsigned int size;
};

struct hisp_mem_info_t {
#ifdef ISP_LINX_CPU_SUPPORT
	unsigned long long cpu_addr;
#else
	unsigned int cpu_addr;
#endif
	unsigned int cpu_size;
	struct hisp_isp_mem_info_t fw_mem;
	struct hisp_isp_mem_info_t ispfw_mem;
};

typedef struct addr_params {
	uint32_t moduleAddr;
#ifdef ISP_LINX_CPU_SUPPORT
	uint64_t iova;
#else
	uint32_t iova;
#endif
	uint32_t sharedFd;
	uint32_t type;
	uint32_t prot;
	uint32_t size;
	void *vaddr;
	size_t offset_in_pool;
	size_t pool_align_size;
	uint32_t security_isp_mode;
	uint32_t isApCached;
	uint32_t ispCoreIova;
	uint32_t usage;
	struct hisp_map_info_t map_info;
	struct hisp_mem_info_t mem_info;
	uint16_t mem_master;
} addr_param_t;

typedef struct imu_addr_params {
#ifdef ISP_LINX_CPU_SUPPORT
	uint64_t iova;
#else
	uint32_t iova;
#endif
	uint32_t size;
	uint32_t usage;
} imu_addr_param_t;

#define HISP_CFG_DATA_LEN 256

struct hisp_cfg_data {
	int cfgtype;
	int mode;
	int isSecure;
	int secMemType;

	union {
		addr_param_t param;
		struct { /* for buffer operation */
			int share_fd;
			uint32_t buf_size;
		};
		uint32_t cfgdata[HISP_CFG_DATA_LEN];
		struct {
			imu_addr_param_t imu_param;
		};
	};
};

enum hisp_config_type {
	HISP_CONFIG_POWER_ON = 0,
	HISP_CONFIG_POWER_OFF,
	HISP_CONFIG_GET_MAP_ADDR,
	HISP_CONFIG_UNMAP_ADDR,
	HISP_CONFIG_INIT_MEMORY_POOL,
	HISP_CONFIG_DEINIT_MEMORY_POOL,
	HISP_CONFIG_ALLOC_MEM,
	HISP_CONFIG_FREE_MEM,
	HISP_CONFIG_ISP_TURBO,
	HISP_CONFIG_ISP_NORMAL,
	HISP_CONFIG_ISP_LOWPOWER,
	HISP_CONFIG_ISP_ULTRALOW,
	HISP_CONFIG_R8_TURBO,
	HISP_CONFIG_R8_NORMAL,
	HISP_CONFIG_R8_LOWPOWER,
	HISP_CONFIG_R8_ULTRALOW,
	HISP_CONFIG_PROC_TIMEOUT,
	HISP_CONFIG_GET_SEC_ISPFW_SIZE,
	HISP_CONFIG_SET_SEC_ISPFW_BUFFER,
	HISP_CONFIG_RELEASE_SEC_ISPFW_BUFFER,
	HISP_CONFIG_SET_MDC_BUFFER,
	HISP_CONFIG_RELEASE_MDC_BUFFER,
	HISP_CONFIG_PHY_CSI_CONNECT,
	HISP_CONFIG_LOCK_VOLTAGE,
	HISP_CONFIG_ALLOC_ISP_CPU_MEM,
	HISP_CONFIG_FREE_ISP_CPU_MEM,
	HISP_CONFIG_SECBOOT_PREPARE,
	HISP_CONFIG_SECBOOT_UNPREPARE,
	HISP_CONFIG_INIT_IMU_BUFFER,
	HISP_CONFIG_DEINIT_IMU_BUFFER,
	HISP_CONFIG_SET_CSI_MODE,
	HISP_CONFIG_SELECT_AO_I2C,
	HISP_CONFIG_ISP_FE,
	HISP_CONFIG_ISP_BE,
	HISP_CONFIG_ISP_PE,
	HISP_CONFIG_MAX_INDEX
};

/* if struct changes, notify platform_include/isp/linux/hisp_remoteproc.h */
struct generic_phy_info {
	__u32 is_master_sensor;
	__u32 phy_id;
	__u32 phy_mode;
	__u32 phy_freq_mode;
	__u32 phy_freq;
	__u32 phy_work_mode;
};

struct phy_csi_connect_info {
	__u32 csi_index;
	struct generic_phy_info phy_info;
};

/* isp_indirect_msg - make hisp_msg_t opaque to kernel
 * @isp_msg: pointer to user space hisp_msg_t variable
 * @msg_size: valid size pointed to by @isp_msg
 */
struct isp_indirect_msg {
	void *isp_msg;
	__u32 msg_size;
};

typedef struct _tag_hisp_info {
	char name[HISP_NAME_SIZE];
} hisp_info_t;

typedef enum _timestamp_mode {
	TIMESTAMP_USE_REALTIME = 0,
	TIMESTAMP_USE_BOOTTIME
} timestamp_mode;

typedef struct _hisp_system_time_t {
	timestamp_mode mode;
	unsigned int s_system_couter_rate;
	unsigned long long s_system_counter;
	unsigned long long s_system_us;
} hisp_system_time_t;

#define HISP_IOCTL_POWER_ON _IO('A', BASE_VIDIOC_PRIVATE + 0x01)
#define HISP_IOCTL_POWER_OFF _IO('A', BASE_VIDIOC_PRIVATE + 0x02)
#define HISP_IOCTL_GET_INFO _IOR('A', BASE_VIDIOC_PRIVATE + 0x05, hisp_info_t)
#define HISP_IOCTL_CFG_ISP _IOWR('A', BASE_VIDIOC_PRIVATE + 0x06, \
	struct hisp_cfg_data)
#define HISP_IOCTL_GET_SYSTEM_TIME _IOWR('A', BASE_VIDIOC_PRIVATE + 0x07, \
	hisp_system_time_t)

#endif /* __HW_KERNEL_CAM_HISP_CFG_BASE_H__ */
