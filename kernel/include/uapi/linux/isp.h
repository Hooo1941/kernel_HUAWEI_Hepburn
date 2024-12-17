/*
 * include/uapi/linux/isp.h
 *
 * Copyright (c) 2013-2014 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ISP_USER_H
#define ISP_USER_H

#include <asm/ioctl.h>
#include <platform_include/isp/linux/hisp_mempool.h>

#define MAX_INPUT_STREAM_NUM (2)
#define MAX_STREAM_NUM       (8)

struct map_info_s {
	int fd;
	unsigned int size;
#ifdef ISP_LINX_CPU_SUPPORT
	unsigned int prot;
#else
	unsigned int fw_mem_size;
	unsigned int ispfw_mem_size;
	unsigned int fw_mem_prot;
	unsigned int ispfw_mem_prot;
#endif
};

typedef struct addrparams {
	unsigned int moduleaddr;
	unsigned int iova;
	unsigned int sharefd;
	unsigned int type;
	unsigned int prot;
	unsigned int size;
	unsigned int pool_num;
	unsigned int pool_addr;
	unsigned int pool_size;
	void *vaddr;
	size_t offset_in_pool;
	size_t pool_align_size;
	unsigned int security_isp_mode;
	unsigned int is_ap_cached;
	unsigned int isp_core_iova;
	unsigned int usage;
	struct map_info_s map_info;
	struct hisp_mem_info_s mem_info;
} addr_param_t;

typedef struct map_hisp_cfg_data {
	int cfgtype;
	int mode;
	addr_param_t param;
} map_hisp_cfg_data_t;

typedef struct memory_block_s {
	int shared_fd;
	int size;
	unsigned long prot;
	unsigned int da;
	int usage;
	void *viraddr;
} memory_block_s;

#define SENSOR_NAME_SIZE (64)
#define ISP_MSG_UNION_SIZE (424)

typedef struct rpmsg_ioctl_arg {
	int index;
	char name[SENSOR_NAME_SIZE];
} rpmsg_ioctl_arg_t;

#define HWSENSOR_IOCTL_POWER_UP     _IOWR('i', 0x01, rpmsg_ioctl_arg_t)
#define HWSENSOR_IOCTL_POWER_DOWN   _IOWR('i', 0x02, rpmsg_ioctl_arg_t)
#define HISP_GET_MAPBUFFER          _IOWR('i', 0x10, map_hisp_cfg_data_t)
#define HISP_UNMAP_BUFFER           _IOWR('i', 0x11, map_hisp_cfg_data_t)
#define HISP_POOL_STEUP             _IOWR('i', 0x12, map_hisp_cfg_data_t)
#define HISP_POOL_DESTROY           _IOWR('i', 0x13, map_hisp_cfg_data_t)
#define HISP_POOL_ALLOC             _IOWR('i', 0x14, map_hisp_cfg_data_t)
#define HISP_POOL_FREE              _IOWR('i', 0x15, map_hisp_cfg_data_t)
#define HISP_MAP_IOMMU              _IOWR('i', 0x16, memory_block_s)
#define HISP_UNMAP_IOMMU            _IOWR('i', 0x17, memory_block_s)
#define HISP_ALLOC_CPU_MEM          _IOWR('i', 0x18, memory_block_s)
#define HISP_FREE_CPU_MEM           _IOWR('i', 0x19, memory_block_s)
#define HISP_SECISP_TA_MAP          _IOWR('i', 0x21, map_hisp_cfg_data_t)
#define HISP_SECISP_TA_UNMAP        _IOWR('i', 0x22, map_hisp_cfg_data_t)

struct msg_base;

typedef void (*msg_looper_handler)(struct msg_base *, void *);

typedef struct msg_base {
	struct list_head link;
	msg_looper_handler handler;
	void *user;
} msg_base_t;

typedef struct _isp_msg_t {
	unsigned int message_size;
	unsigned int api_name;
	unsigned int message_id;
	unsigned int message_hash;
	union {
		char reserve[ISP_MSG_UNION_SIZE];
	} u;
	msg_base_t base;
	short token;
	struct rpmsg_ept *ept;
} isp_msg_t;

#endif /* MESSAGE_H_INCLUDED */
