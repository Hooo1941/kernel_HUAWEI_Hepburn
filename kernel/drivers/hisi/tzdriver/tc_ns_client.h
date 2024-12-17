/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: data structure declaration for nonsecure world
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
#ifndef TC_NS_CLIENT_H
#define TC_NS_CLIENT_H

#include <linux/types.h>
#include <linux/version.h>

#define UUID_LEN                16
#define PARAM_NUM               4
#define ADDR_TRANS_NUM          32

#define teec_param_types(param0_type, param1_type, param2_type, param3_type) \
	((param3_type) << 12 | (param2_type) << 8 | \
	(param1_type) << 4 | (param0_type))

#define teec_param_type_get(param_types, index) \
	(((param_types) >> ((index) << 2)) & 0x0F)

#ifndef ZERO_SIZE_PTR
#define ZERO_SIZE_PTR ((void *)16)
#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= (unsigned long)ZERO_SIZE_PTR)
#endif

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
#define mm_sem_lock(mm) (mm)->mmap_lock
#else
#define mm_sem_lock(mm) (mm)->mmap_sem
#endif

struct tc_ns_client_login {
	__u32 method;
	__u32 mdata;
};

union tc_ns_client_param {
	struct {
		__u32 buffer;
		__u32 buffer_h_addr;
		__u32 offset;
		__u32 h_offset;
		__u32 size_addr;
		__u32 size_h_addr;
	} memref;
	struct {
		__u32 a_addr;
		__u32 a_h_addr;
		__u32 b_addr;
		__u32 b_h_addr;
	} value;
};

struct tc_ns_client_return {
	int code;
	__u32 origin;
};

struct tc_ns_client_context {
	unsigned char uuid[UUID_LEN];
	__u32 session_id;
	__u32 cmd_id;
	struct tc_ns_client_return returns;
	struct tc_ns_client_login login;
	union tc_ns_client_param params[PARAM_NUM];
	__u32 param_types;
	__u8 started;
	__u32 calling_pid;
	unsigned int file_size;
	union {
		char *file_buffer;
		struct {
			uint32_t file_addr;
			uint32_t file_h_addr;
		} memref;
	};
};

struct tc_ns_client_time {
	uint32_t seconds;
	uint32_t millis;
};

enum secfile_type_t {
	LOAD_TA = 0,
	LOAD_SERVICE,
	LOAD_LIB,
	LOAD_DYNAMIC_DRV,
	LOAD_PATCH,
	LOAD_TYPE_MAX,
};

struct sec_file_info {
	enum secfile_type_t secfile_type;
	uint32_t file_size;
	int32_t sec_load_err;
};

struct load_secfile_ioctl_struct {
	struct sec_file_info sec_file_info;
	unsigned char uuid[UUID_LEN];
	union {
		char *file_buffer;
		struct {
			uint32_t file_addr;
			uint32_t file_h_addr;
		} memref;
	};
}__attribute__((packed));

struct agent_ioctl_args {
	uint32_t id;
	uint32_t buffer_size;
	union {
		void *buffer;
		unsigned long long addr;
	};
};

struct tc_ns_client_crl {
	union {
		uint8_t *buffer;
		struct {
			uint32_t buffer_addr;
			uint32_t buffer_h_addr;
		} memref;
	};
	uint32_t size;
};

#ifdef CONFIG_LOG_POOL_ENABLE
struct tc_ns_log_pool {
	uint64_t addr;
	uint64_t size;
};
#endif

#define MAX_SHA_256_SZ 32

#define TC_NS_CLIENT_IOCTL_SES_OPEN_REQ \
	 _IOW(TC_NS_CLIENT_IOC_MAGIC, 1, struct tc_ns_client_context)
#define TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 2, struct tc_ns_client_context)
#define TC_NS_CLIENT_IOCTL_SEND_CMD_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 3, struct tc_ns_client_context)
#define TC_NS_CLIENT_IOCTL_SHRD_MEM_RELEASE \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 4, unsigned int)
#define TC_NS_CLIENT_IOCTL_WAIT_EVENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 5, unsigned int)
#define TC_NS_CLIENT_IOCTL_SEND_EVENT_RESPONSE \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 6, unsigned int)
#define TC_NS_CLIENT_IOCTL_REGISTER_AGENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 7, struct agent_ioctl_args)
#define TC_NS_CLIENT_IOCTL_UNREGISTER_AGENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 8, unsigned int)
#define TC_NS_CLIENT_IOCTL_LOAD_APP_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 9, struct load_secfile_ioctl_struct)
#define TC_NS_CLIENT_IOCTL_NEED_LOAD_APP \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 10, struct tc_ns_client_context)
#define TC_NS_CLIENT_IOCTL_ALLOC_EXCEPTING_MEM \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 12, unsigned int)
#define TC_NS_CLIENT_IOCTL_CANCEL_CMD_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 13, struct tc_ns_client_context)
#define TC_NS_CLIENT_IOCTL_LOGIN \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 14, int)
#define TC_NS_CLIENT_IOCTL_TUI_EVENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 16, int)
#define TC_NS_CLIENT_IOCTL_SYC_SYS_TIME \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 17, struct tc_ns_client_time)
#define TC_NS_CLIENT_IOCTL_SET_NATIVECA_IDENTITY \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 18, int)
#define TC_NS_CLIENT_IOCTL_LOAD_TTF_FILE_AND_NOTCH_HEIGHT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 19, unsigned int)
#define TC_NS_CLIENT_IOCTL_LATEINIT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 20, unsigned int)
#define TC_NS_CLIENT_IOCTL_GET_TEE_VERSION \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 21, unsigned int)
#define TC_NS_CLIENT_IOCTL_UPDATE_TA_CRL\
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 22, struct tc_ns_client_crl)
#ifdef CONFIG_LOG_POOL_ENABLE
#define TC_NS_CLIENT_IOCTL_GET_LOG_POOL \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 23, struct tc_ns_log_pool)
#endif
#endif
