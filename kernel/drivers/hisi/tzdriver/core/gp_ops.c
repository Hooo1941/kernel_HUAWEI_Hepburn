/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: alloc global operation and pass params to TEE.
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
#include "gp_ops.h"
#include <linux/uaccess.h>
#include <linux/uidgid.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/cred.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <asm/memory.h>
#include <securec.h>
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "smc_smp.h"
#include "mem.h"
#include "mailbox_mempool.h"
#include "tc_client_driver.h"
#include "internal_functions.h"
#include "reserved_mempool.h"
#include "tlogger.h"
#include "dynamic_ion_mem.h"

#define MAX_SHARED_SIZE 0x100000      /* 1 MiB */

static void free_operation(const struct tc_call_params *call_params,
	struct tc_op_params *op_params);

/* dir: 0-inclue input, 1-include output, 2-both */
#define INPUT  0
#define OUTPUT 1
#define INOUT  2

static inline bool is_input_type(int dir)
{
	if (dir == INPUT || dir == INOUT)
		return true;

	return false;
}

static inline bool is_output_type(int dir)
{
	if (dir == OUTPUT || dir == INOUT)
		return true;

	return false;
}

static inline bool teec_value_type(unsigned int type, int dir)
{
	return ((is_input_type(dir) && type == TEEC_VALUE_INPUT) ||
		(is_output_type(dir) && type == TEEC_VALUE_OUTPUT) ||
		type == TEEC_VALUE_INOUT) ? true : false;
}

static inline bool teec_tmpmem_type(unsigned int type, int dir)
{
	return ((is_input_type(dir) && type == TEEC_MEMREF_TEMP_INPUT) ||
		(is_output_type(dir) && type == TEEC_MEMREF_TEMP_OUTPUT) ||
		type == TEEC_MEMREF_TEMP_INOUT) ? true : false;
}

static inline bool teec_memref_type(unsigned int type, int dir)
{
	return ((is_input_type(dir) && type == TEEC_MEMREF_PARTIAL_INPUT) ||
		(is_output_type(dir) && type == TEEC_MEMREF_PARTIAL_OUTPUT) ||
		type == TEEC_MEMREF_PARTIAL_INOUT) ? true : false;
}

static int check_user_param(const struct tc_ns_client_context *client_context,
	unsigned int index)
{
	if (!client_context) {
		tloge("client_context is null\n");
		return -EINVAL;
	}

	if (index >= PARAM_NUM) {
		tloge("index is invalid, index:%x\n", index);
		return -EINVAL;
	}
	return 0;
}

bool is_tmp_mem(uint32_t param_type)
{
	if (param_type == TEEC_MEMREF_TEMP_INPUT ||
		param_type == TEEC_MEMREF_TEMP_OUTPUT ||
		param_type == TEEC_MEMREF_TEMP_INOUT)
		return true;

	return false;
}

bool is_ref_mem(uint32_t param_type)
{
	if (param_type == TEEC_MEMREF_PARTIAL_INPUT ||
		param_type == TEEC_MEMREF_PARTIAL_OUTPUT ||
		param_type == TEEC_MEMREF_PARTIAL_INOUT)
		return true;

	return false;
}

bool is_val_param(uint32_t param_type)
{
	if (param_type == TEEC_VALUE_INPUT ||
		param_type == TEEC_VALUE_OUTPUT ||
		param_type == TEEC_VALUE_INOUT ||
		param_type == TEEC_ION_INPUT ||
		param_type == TEEC_ION_SGLIST_INPUT)
		return true;

	return false;
}

static bool is_mem_param(uint32_t param_type)
{
	if (is_tmp_mem(param_type) || is_ref_mem(param_type))
		return true;

	return false;
}

/* Check the size and buffer addresses  have valid userspace addresses */
static bool is_usr_refmem_valid(const union tc_ns_client_param *client_param)
{
	uint32_t size = 0;
	uint64_t size_addr = client_param->memref.size_addr |
		((uint64_t)client_param->memref.size_h_addr << ADDR_TRANS_NUM);
	uint64_t buffer_addr = client_param->memref.buffer |
		((uint64_t)client_param->memref.buffer_h_addr << ADDR_TRANS_NUM);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (access_ok(VERIFY_READ, (void *)(uintptr_t)size_addr, sizeof(uint32_t)) == 0)
#else
	if (access_ok((void *)(uintptr_t)size_addr, sizeof(uint32_t)) == 0)
#endif
		return false;

	get_user(size, (uint32_t *)(uintptr_t)size_addr);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (access_ok(VERIFY_READ, (void *)(uintptr_t)buffer_addr, size) == 0)
#else
	if (access_ok((void *)(uintptr_t)buffer_addr, size) == 0)
#endif
		return false;

	return true;
}

static bool is_usr_valmem_valid(const union tc_ns_client_param *client_param)
{
	uint64_t a_addr = client_param->value.a_addr |
		((uint64_t)client_param->value.a_h_addr << ADDR_TRANS_NUM);
	uint64_t b_addr = client_param->value.b_addr |
		((uint64_t)client_param->value.b_h_addr << ADDR_TRANS_NUM);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (access_ok(VERIFY_READ, (void *)(uintptr_t)a_addr, sizeof(uint32_t)) == 0)
#else
	if (access_ok((void *)(uintptr_t)a_addr, sizeof(uint32_t)) == 0)
#endif
		return false;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (access_ok(VERIFY_READ, (void *)(uintptr_t)b_addr, sizeof(uint32_t)) == 0)
#else
	if (access_ok((void *)(uintptr_t)b_addr, sizeof(uint32_t)) == 0)
#endif
		return false;

	return true;
}

bool tc_user_param_valid(struct tc_ns_client_context *client_context,
	unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	unsigned int param_type;

	if (check_user_param(client_context, index) != 0)
		return false;

	client_param = &(client_context->params[index]);
	param_type = teec_param_type_get(client_context->param_types, index);
	tlogd("param %u type is %x\n", index, param_type);
	if (param_type == TEEC_NONE) {
		tlogd("param type is TEEC_NONE\n");
		return true;
	}

	if (is_mem_param(param_type)) {
		if (!is_usr_refmem_valid(client_param))
			return false;
	} else if (is_val_param(param_type)) {
		if (!is_usr_valmem_valid(client_param))
			return false;
	} else {
		tloge("param types is not supported\n");
		return false;
	}

	return true;
}

/*
 * These function handle read from client. Because client here can be
 * kernel client or user space client, we must use the proper function
 */
int read_from_client(void *dest, size_t dest_size,
	const void __user *src, size_t size, uint8_t kernel_api)
{
	int ret;

	if (!dest || !src) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}

	if (size > dest_size) {
		tloge("size is larger than dest_size or size is 0\n");
		return -EINVAL;
	}
	if (size == 0)
		return 0;

	if (kernel_api != 0) {
		ret = memcpy_s(dest, dest_size, src, size);
		if (ret != EOK) {
			tloge("memcpy fail. line=%d, s_ret=%d\n",
				__LINE__, ret);
			return ret;
		}
		return ret;
	}
	/* buffer is in user space(CA call TEE API) */
	if (copy_from_user(dest, src, size) != 0) {
		tloge("copy from user failed\n");
		return -EFAULT;
	}

	return 0;
}

int write_to_client(void __user *dest, size_t dest_size,
	const void *src, size_t size, uint8_t kernel_api)
{
	int ret;

	if (!dest || !src) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}

	if (size > dest_size) {
		tloge("size is larger than dest_size\n");
		return -EINVAL;
	}

	if (size == 0)
		return 0;

	if (kernel_api != 0) {
		ret = memcpy_s(dest, dest_size, src, size);
		if (ret != EOK) {
			tloge("write to client fail. line=%d, ret=%d\n",
			      __LINE__, ret);
			return ret;
		}
		return ret;
	}

	/* buffer is in user space(CA call TEE API) */
	if (copy_to_user(dest, src, size) != 0) {
		tloge("copy to user failed\n");
		return -EFAULT;
	}
	return 0;
}

static bool is_input_tempmem(unsigned int param_type)
{
	if (param_type == TEEC_MEMREF_TEMP_INPUT ||
		param_type == TEEC_MEMREF_TEMP_INOUT)
		return true;

	return false;
}

static int update_input_data(const union tc_ns_client_param *client_param,
	uint32_t buffer_size, void *temp_buf,
	unsigned int param_type, uint8_t kernel_params)
{
	uint64_t buffer_addr;
	if (!is_input_tempmem(param_type))
		return 0;

	buffer_addr = client_param->memref.buffer |
		((uint64_t)client_param->memref.buffer_h_addr << ADDR_TRANS_NUM);
	if (read_from_client(temp_buf, buffer_size,
		(void *)(uintptr_t)buffer_addr,
		buffer_size, kernel_params) != 0) {
		tloge("copy memref buffer failed\n");
		return -EFAULT;
	}
	return 0;
}

/*
 * temp buffers we need to allocate/deallocate
 * for every operation
 */
static int alloc_for_tmp_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	void *temp_buf = NULL;
	uint32_t buffer_size = 0;
	uint64_t size_addr;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	/* For compatibility sake we assume buffer size to be 32bits */
	client_param = &(call_params->context->params[index]);
	size_addr = client_param->memref.size_addr |
		((uint64_t)client_param->memref.size_h_addr << ADDR_TRANS_NUM);
	if (read_from_client(&buffer_size, sizeof(buffer_size),
		(uint32_t __user *)(uintptr_t)size_addr,
		sizeof(uint32_t), kernel_params) != 0) {
		tloge("copy memref.size_addr failed\n");
		return -EFAULT;
	}

	if (buffer_size > MAX_SHARED_SIZE) {
		tloge("buffer size %u from user is too large\n", buffer_size);
		return -EFAULT;
	}

	op_params->mb_pack->operation.params[index].memref.size = buffer_size;
	/* TEEC_MEMREF_TEMP_INPUT equal to TEE_PARAM_TYPE_MEMREF_INPUT */
	op_params->trans_paramtype[index] = param_type;

	if (buffer_size == 0) {
		op_params->local_tmpbuf[index].temp_buffer = NULL;
		op_params->local_tmpbuf[index].size = 0;
		op_params->mb_pack->operation.params[index].memref.buffer = 0;
		op_params->mb_pack->operation.buffer_h_addr[index] = 0;
		return 0;
	}

	temp_buf = mailbox_alloc(buffer_size, MB_FLAG_ZERO);
	if (!temp_buf) {
		tloge("temp buf malloc failed, i = %u\n", index);
		return -ENOMEM;
	}
	op_params->local_tmpbuf[index].temp_buffer = temp_buf;
	op_params->local_tmpbuf[index].size = buffer_size;

	if (update_input_data(client_param, buffer_size, temp_buf,
		param_type, kernel_params) != 0)
		return -EFAULT;

	op_params->mb_pack->operation.params[index].memref.buffer =
		mailbox_virt_to_phys((uintptr_t)temp_buf);
	op_params->mb_pack->operation.buffer_h_addr[index] =
		(unsigned int)(mailbox_virt_to_phys((uintptr_t)temp_buf) >> ADDR_TRANS_NUM);

	return 0;
}

static int check_buffer_for_ref(uint32_t *buffer_size,
	const union tc_ns_client_param *client_param, uint8_t kernel_params)
{
	uint64_t size_addr = client_param->memref.size_addr |
		((uint64_t)client_param->memref.size_h_addr << ADDR_TRANS_NUM);
	if (read_from_client(buffer_size, sizeof(*buffer_size),
		(uint32_t __user *)(uintptr_t)size_addr,
		sizeof(uint32_t), kernel_params) != 0) {
		tloge("copy memref.size_addr failed\n");
		return -EFAULT;
	}
	if (*buffer_size == 0) {
		tloge("buffer_size from user is 0\n");
		return -ENOMEM;
	}
	return 0;
}

static bool is_refmem_offset_valid(const struct tc_ns_shared_mem *shared_mem,
	const union tc_ns_client_param *client_param, uint32_t buffer_size)
{
	/*
	 * arbitrary CA can control offset by ioctl, so in here
	 * offset must be checked, and avoid integer overflow.
	 */
	if (((shared_mem->len - client_param->memref.offset) >= buffer_size) &&
		(shared_mem->len > client_param->memref.offset))
		return true;
	tloge("Unexpected size %u vs %u", shared_mem->len, buffer_size);
	return false;
}

static bool is_phyaddr_valid(const struct tc_ns_operation *operation, int index)
{
	/*
	 * for 8G physical memory device, there is a chance that
	 * operation->params[i].memref.buffer could be all 0,
	 * buffer_h_addr cannot be 0 in the same time.
	 */
	if ((operation->params[index].memref.buffer == 0) &&
		(operation->buffer_h_addr[index]) == 0) {
		tloge("can not find shared buffer, exit\n");
		return false;
	}

	return true;
}

static int set_operation_buffer(const struct tc_ns_shared_mem *shared_mem, void *buffer_addr,
	uint32_t buffer_size, unsigned int index, struct tc_op_params *op_params)
{
	if (shared_mem->mem_type == RESERVED_TYPE) {
		/* no copy to mailbox */
		op_params->mb_pack->operation.mb_buffer[index] = buffer_addr;
		op_params->mb_pack->operation.params[index].memref.buffer =
			res_mem_virt_to_phys((uintptr_t)buffer_addr);
		op_params->mb_pack->operation.buffer_h_addr[index] =
			res_mem_virt_to_phys((uintptr_t)buffer_addr) >> ADDR_TRANS_NUM;
	} else {
		void *tmp_buffer_addr = mailbox_copy_alloc(buffer_addr, buffer_size);
		if (tmp_buffer_addr == NULL)
			return -ENOMEM;

		op_params->mb_pack->operation.mb_buffer[index] = tmp_buffer_addr;
		op_params->mb_pack->operation.params[index].memref.buffer =
			(unsigned int)mailbox_virt_to_phys((uintptr_t)tmp_buffer_addr);
		op_params->mb_pack->operation.buffer_h_addr[index] =
			(unsigned int)((uint64_t)mailbox_virt_to_phys((uintptr_t)tmp_buffer_addr) >> ADDR_TRANS_NUM);
	}
	return 0;
}

/*
 * MEMREF_PARTIAL buffers are already allocated so we just
 * need to search for the shared_mem ref;
 * For interface compatibility we assume buffer size to be 32bits
 */
static int alloc_for_ref_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	struct tc_ns_shared_mem *shared_mem = NULL;
	uint32_t buffer_size = 0;
	void *buffer_addr = NULL;
	int ret = 0;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	if (check_buffer_for_ref(&buffer_size, client_param, kernel_params) != 0)
		return -EINVAL;

	op_params->mb_pack->operation.params[index].memref.buffer = 0;

	mutex_lock(&call_params->dev->shared_mem_lock);
	list_for_each_entry(shared_mem,
		&call_params->dev->shared_mem_list, head) {
		buffer_addr = (void *)(uintptr_t)(client_param->memref.buffer |
			((uint64_t)client_param->memref.buffer_h_addr << ADDR_TRANS_NUM));
		if (shared_mem->user_addr != buffer_addr)
			continue;
		if (!is_refmem_offset_valid(shared_mem, client_param,
			buffer_size)) {
			break;
		}
		buffer_addr = (void *)(uintptr_t)(
			(uintptr_t)shared_mem->kernel_addr +
			client_param->memref.offset);

		ret = set_operation_buffer(shared_mem, buffer_addr, buffer_size, index, op_params);
		if (ret != 0) {
			tloge("set operation buffer failed\n");
			break;
		}
		op_params->mb_pack->operation.sharemem[index] = shared_mem;
		get_sharemem_struct(shared_mem);
		break;
	}
	mutex_unlock(&call_params->dev->shared_mem_lock);
	if (ret != 0)
		return ret;

	if (!is_phyaddr_valid(&op_params->mb_pack->operation, index))
		return -EINVAL;

	op_params->mb_pack->operation.params[index].memref.size = buffer_size;
	/* Change TEEC_MEMREF_PARTIAL_XXXXX  to TEE_PARAM_TYPE_MEMREF_XXXXX */
	op_params->trans_paramtype[index] = param_type -
		(TEEC_MEMREF_PARTIAL_INPUT - TEE_PARAM_TYPE_MEMREF_INPUT);

	if (shared_mem->mem_type == RESERVED_TYPE)
		op_params->trans_paramtype[index] +=
			(TEE_PARAM_TYPE_RESMEM_INPUT - TEE_PARAM_TYPE_MEMREF_INPUT);
	return ret;
}

#ifdef CONFIG_NOCOPY_SHAREDMEM
static int fill_shared_mem_info(void *start_vaddr, uint32_t pages_no,
	uint32_t offset, uint32_t buffer_size, void *buff)
{
	struct pagelist_info *page_info = NULL;
	struct page **pages = NULL;
	uint64_t *phys_addr = NULL;
	uint32_t page_num;
	uint32_t i;

	if (pages_no == 0)
		return -EFAULT;

	pages = (struct page **)vmalloc(pages_no * sizeof(uint64_t));
	if (pages == NULL)
		return -EFAULT;

	down_read(&mm_sem_lock(current->mm));
	page_num = get_user_pages((uintptr_t)start_vaddr, pages_no, FOLL_WRITE, pages, NULL);
	up_read(&mm_sem_lock(current->mm));
	if (page_num != pages_no) {
		tloge("get page phy addr failed\n");
		if (page_num > 0)
			release_pages(pages, page_num);
		vfree(pages);
		return -EFAULT;
	}

	page_info = buff;
	page_info->page_num = pages_no;
	page_info->page_size = PAGE_SIZE;
	page_info->sharedmem_offset = offset;
	page_info->sharedmem_size = buffer_size;

	phys_addr = (uint64_t *)buff + (sizeof(*page_info) / sizeof(uint64_t));
	for (i = 0; i < pages_no; i++) {
		struct page *page = pages[i];
		if (page == NULL) {
			release_pages(pages, page_num);
			vfree(pages);
			return -EFAULT;
		}
		phys_addr[i] = (uintptr_t)page_to_phys(page);
	}

	vfree(pages);
	return 0;
}

static int check_buffer_for_sharedmem(uint32_t *buffer_size,
	const union tc_ns_client_param *client_param, uint8_t kernel_params)
{
	uint64_t size_addr = client_param->memref.size_addr |
		((uint64_t)client_param->memref.size_h_addr << ADDR_TRANS_NUM);
	uint64_t buffer_addr = client_param->memref.buffer |
		((uint64_t)client_param->memref.buffer_h_addr << ADDR_TRANS_NUM);
	if (read_from_client(buffer_size, sizeof(*buffer_size),
		(uint32_t __user *)(uintptr_t)size_addr,
		sizeof(uint32_t), kernel_params)) {
		tloge("copy size_addr failed\n");
		return -EFAULT;
	}

	if (*buffer_size == 0 || *buffer_size > SZ_256M) {
		tloge("invalid buffer size\n");
		return -ENOMEM;
	}

	if ((client_param->memref.offset >= SZ_256M) ||
		(UINT64_MAX - buffer_addr <= client_param->memref.offset)) {
		tloge("invalid buff or offset\n");
		return -EFAULT;
	}
	return 0;
}

static int transfer_shared_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	void *buff = NULL;
	void *start_vaddr = NULL;
	union tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	uint32_t pages_no;
	uint32_t offset;
	uint32_t buff_len;
	uint64_t buffer_addr;

	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	if (check_buffer_for_sharedmem(&buffer_size, client_param, kernel_params))
		return -EINVAL;

	buffer_addr = client_param->memref.buffer |
		((uint64_t)client_param->memref.buffer_h_addr << ADDR_TRANS_NUM);
	buff = (void *)(uint64_t)(buffer_addr + client_param->memref.offset);
	start_vaddr = (void *)(((uint64_t)buff) & PAGE_MASK);
	offset = ((uint32_t)(uintptr_t)buff) & (~PAGE_MASK);
	pages_no = PAGE_ALIGN(offset + buffer_size) / PAGE_SIZE;

	buff_len = sizeof(struct pagelist_info) + (sizeof(uint64_t) * pages_no);
	buff = mailbox_alloc(buff_len, MB_FLAG_ZERO);
	if (buff == NULL)
		return -EFAULT;

	if (fill_shared_mem_info(start_vaddr, pages_no, offset, buffer_size, buff)) {
		mailbox_free(buff);
		return -EFAULT;
	}

	op_params->local_tmpbuf[index].temp_buffer = buff;
	op_params->local_tmpbuf[index].size = buff_len;

	op_params->mb_pack->operation.params[index].memref.buffer = mailbox_virt_to_phys((uintptr_t)buff);
	op_params->mb_pack->operation.buffer_h_addr[index] = (uint64_t)mailbox_virt_to_phys((uintptr_t)buff) >> ADDR_TRANS_NUM;
	op_params->mb_pack->operation.params[index].memref.size = buff_len;
	op_params->trans_paramtype[index] = param_type;
	return 0;
}
#else
static int transfer_shared_mem(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	(void)call_params;
	(void)op_params;
	(void)kernel_params;
	(void)param_type;
	(void)index;
	tloge("invalid shared mem type\n");
	return -1;
}
#endif

static int transfer_client_value(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	union tc_ns_client_param *client_param = NULL;
	uint64_t a_addr, b_addr;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	a_addr = client_param->value.a_addr |
		((uint64_t)client_param->value.a_h_addr << ADDR_TRANS_NUM);
	b_addr = client_param->value.b_addr |
		((uint64_t)client_param->value.b_h_addr << ADDR_TRANS_NUM);

	if (read_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)a_addr,
		sizeof(operation->params[index].value.a),
		kernel_params) != 0) {
		tloge("copy valuea failed\n");
		return -EFAULT;
	}
	if (read_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)b_addr,
		sizeof(operation->params[index].value.b),
		kernel_params) != 0) {
		tloge("copy valueb failed\n");
		return -EFAULT;
	}

	/* TEEC_VALUE_INPUT equal to TEE_PARAM_TYPE_VALUE_INPUT */
	op_params->trans_paramtype[index] = param_type;
	return 0;
}

static int alloc_operation(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	int ret = 0;
	uint32_t index;
	uint8_t kernel_params;
	uint32_t param_type;

	kernel_params = call_params->dev->kernel_api;
	for (index = 0; index < TEE_PARAM_NUM; index++) {
		/*
		 * Normally kernel_params = kernel_api
		 * But when TC_CALL_LOGIN, params 2/3 will
		 * be filled by kernel. so under this circumstance,
		 * params 2/3 has to be set to kernel mode; and
		 * param 0/1 will keep the same with kernel_api.
		 */
		if ((call_params->flags & TC_CALL_LOGIN) && (index >= 2))
			kernel_params = TEE_REQ_FROM_KERNEL_MODE;
		param_type = teec_param_type_get(
			call_params->context->param_types, index);

		tlogd("param %u type is %x\n", index, param_type);
		if (teec_tmpmem_type(param_type, INOUT))
			ret = alloc_for_tmp_mem(call_params, op_params,
				kernel_params, param_type, index);
		else if (teec_memref_type(param_type, INOUT))
			ret = alloc_for_ref_mem(call_params, op_params,
				kernel_params, param_type, index);
		else if (teec_value_type(param_type, INOUT))
			ret = transfer_client_value(call_params, op_params,
				kernel_params, param_type, index);
		else if (param_type == TEEC_ION_INPUT)
			ret = alloc_for_ion(call_params, op_params,
				kernel_params, param_type, index);
		else if (param_type == TEEC_ION_SGLIST_INPUT)
			ret = alloc_for_ion_sglist(call_params, op_params,
				kernel_params, param_type, index);
		else if (param_type == TEEC_MEMREF_SHARED_INOUT)
			ret = transfer_shared_mem(call_params, op_params,
				kernel_params, param_type, index);
		else
			tlogd("param type = TEEC_NONE\n");

		if (ret != 0)
			break;
	}
	if (ret != 0) {
		free_operation(call_params, op_params);
		return ret;
	}
	op_params->mb_pack->operation.paramtypes =
		teec_param_types(op_params->trans_paramtype[0],
		op_params->trans_paramtype[1],
		op_params->trans_paramtype[2],
		op_params->trans_paramtype[3]);
	op_params->op_inited = true;

	return ret;
}

static int update_tmp_mem(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params, unsigned int index, bool is_complete)
{
	union tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	uint64_t size_addr, buffer_addr;

	if (index >= TEE_PARAM_NUM) {
		tloge("tmp buf size or index is invalid\n");
		return -EFAULT;
	}

	buffer_size = operation->params[index].memref.size;
	client_param = &(call_params->context->params[index]);
	size_addr = client_param->memref.size_addr |
		((uint64_t)client_param->memref.size_h_addr << ADDR_TRANS_NUM);
	buffer_addr = client_param->memref.buffer |
		((uint64_t)client_param->memref.buffer_h_addr << ADDR_TRANS_NUM);
	/* Size is updated all the time */
	if (write_to_client((void *)(uintptr_t)size_addr,
		sizeof(buffer_size),
		&buffer_size, sizeof(buffer_size),
		call_params->dev->kernel_api) != 0) {
		tloge("copy tempbuf size failed\n");
		return -EFAULT;
	}
	if (buffer_size > op_params->local_tmpbuf[index].size) {
		/* incomplete case, when the buffer size is invalid see next param */
		if (!is_complete)
			return 0;
		/*
		 * complete case, operation is allocated from mailbox
		 *  and share with gtask, so it's possible to be changed
		 */
		tloge("memref.size has been changed larger than the initial\n");
		return -EFAULT;
	}
	if (buffer_size == 0)
		return 0;
	/* Only update the buffer when the buffer size is valid in complete case */
	if (write_to_client((void *)(uintptr_t)buffer_addr,
		operation->params[index].memref.size,
		op_params->local_tmpbuf[index].temp_buffer,
		operation->params[index].memref.size,
		call_params->dev->kernel_api) != 0) {
		tloge("copy tempbuf failed\n");
		return -ENOMEM;
	}
	return 0;
}

static int update_for_ref_mem(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	unsigned int orig_size = 0;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	uint64_t size_addr;

	if (index >= TEE_PARAM_NUM) {
		tloge("index is invalid\n");
		return -EFAULT;
	}

	/* update size */
	buffer_size = operation->params[index].memref.size;
	client_param = &(call_params->context->params[index]);
	size_addr = client_param->memref.size_addr |
		((uint64_t)client_param->memref.size_h_addr << ADDR_TRANS_NUM);

	if (read_from_client(&orig_size,
		sizeof(orig_size),
		(uint32_t __user *)(uintptr_t)size_addr,
		sizeof(orig_size), call_params->dev->kernel_api) != 0) {
		tloge("copy orig memref.size_addr failed\n");
		return -EFAULT;
	}

	if (write_to_client((void *)(uintptr_t)size_addr,
		sizeof(buffer_size),
		&buffer_size, sizeof(buffer_size),
		call_params->dev->kernel_api) != 0) {
		tloge("copy buf size failed\n");
		return -EFAULT;
	}

	/* reserved memory no need to copy */
	if (operation->sharemem[index]->mem_type == RESERVED_TYPE)
		return 0;
	/* copy from mb_buffer to sharemem */
	if (operation->mb_buffer[index] && orig_size >= buffer_size) {
		void *buffer_addr =
			(void *)(uintptr_t)((uintptr_t)
			operation->sharemem[index]->kernel_addr +
			client_param->memref.offset);
		if (memcpy_s(buffer_addr,
			operation->sharemem[index]->len -
			client_param->memref.offset,
			operation->mb_buffer[index], buffer_size) != 0) {
			tloge("copy to sharemem failed\n");
			return -EFAULT;
		}
	}
	return 0;
}

static int update_for_value(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	uint64_t a_addr, b_addr;

	if (index >= TEE_PARAM_NUM) {
		tloge("index is invalid\n");
		return -EFAULT;
	}
	client_param = &(call_params->context->params[index]);
	a_addr = client_param->value.a_addr |
		((uint64_t)client_param->value.a_h_addr << ADDR_TRANS_NUM);
	b_addr = client_param->value.b_addr |
		((uint64_t)client_param->value.b_h_addr << ADDR_TRANS_NUM);

	if (write_to_client((void *)(uintptr_t)a_addr,
		sizeof(operation->params[index].value.a),
		&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		call_params->dev->kernel_api) != 0) {
		tloge("inc copy value.a_addr failed\n");
		return -EFAULT;
	}
	if (write_to_client((void *)(uintptr_t)b_addr,
		sizeof(operation->params[index].value.b),
		&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		call_params->dev->kernel_api) != 0) {
		tloge("inc copy value.b_addr failed\n");
		return -EFAULT;
	}
	return 0;
}

static int update_client_operation(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params, bool is_complete)
{
	int ret = 0;
	uint32_t param_type;
	uint32_t index;

	if (!op_params->op_inited)
		return 0;

	/* if paramTypes is NULL, no need to update */
	if (call_params->context->param_types == 0)
		return 0;

	for (index = 0; index < TEE_PARAM_NUM; index++) {
		param_type = teec_param_type_get(
			call_params->context->param_types, index);
		if (teec_tmpmem_type(param_type, OUTPUT))
			ret = update_tmp_mem(call_params, op_params,
				index, is_complete);
		else if (teec_memref_type(param_type, OUTPUT))
			ret = update_for_ref_mem(call_params,
				op_params, index);
		else if (is_complete && teec_value_type(param_type, OUTPUT))
			ret = update_for_value(call_params, op_params, index);
		else
			tlogd("param_type:%u don't need to update\n", param_type);
		if (ret != 0)
			break;
	}
	return ret;
}

#ifdef CONFIG_NOCOPY_SHAREDMEM
static void release_page(void *buf)
{
	uint32_t i;
	uint64_t *phys_addr = NULL;
	struct pagelist_info *page_info = NULL;
	struct page *page = NULL;

	page_info = buf;
	phys_addr = (uint64_t *)buf + (sizeof(*page_info) / sizeof(uint64_t));
	for (i = 0; i < page_info->page_num; i++) {
		page = (struct page *)(uintptr_t)phys_to_page(phys_addr[i]);
		if (page == NULL)
			continue;
		set_bit(PG_dirty, &page->flags);
		put_page(page);
	}
}
#endif

static void free_operation(const struct tc_call_params *call_params, struct tc_op_params *op_params)
{
	uint32_t param_type;
	uint32_t index;
	void *temp_buf = NULL;
	struct tc_ns_temp_buf *local_tmpbuf = op_params->local_tmpbuf;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;

	for (index = 0; index < TEE_PARAM_NUM; index++) {
		param_type = teec_param_type_get(call_params->context->param_types, index);
		if (is_tmp_mem(param_type)) {
			/* free temp buffer */
			temp_buf = local_tmpbuf[index].temp_buffer;
			tlogd("free temp buf, i = %u\n", index);
#if (!defined(CONFIG_LIBLINUX)) && (!defined(CONFIG_SHARED_MEM_RESERVED))
			/* if temp_buf from iomap instead of page_alloc, virt_addr_valid will return false */
			if (!virt_addr_valid((unsigned long)(uintptr_t)temp_buf))
				continue;
#endif
			if (!ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		} else if (is_ref_mem(param_type)) {
			struct tc_ns_shared_mem *shm = operation->sharemem[index];
			if (shm != NULL && shm->mem_type == RESERVED_TYPE) {
				put_sharemem_struct(operation->sharemem[index]);
				continue;
			}
			put_sharemem_struct(operation->sharemem[index]);
			if (operation->mb_buffer[index])
				mailbox_free(operation->mb_buffer[index]);
		} else if (param_type == TEEC_ION_SGLIST_INPUT) {
			temp_buf = local_tmpbuf[index].temp_buffer;
			tlogd("free ion sglist buf, i = %u\n", index);
#if (!defined(CONFIG_LIBLINUX)) && (!defined(CONFIG_SHARED_MEM_RESERVED))
			/* if temp_buf from iomap instead of page_alloc, virt_addr_valid will return false */
			if (!virt_addr_valid((uint64_t)(uintptr_t)temp_buf))
				continue;
#endif
			if (!ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		} else if (param_type == TEEC_MEMREF_SHARED_INOUT) {
#ifdef CONFIG_NOCOPY_SHAREDMEM
			temp_buf = local_tmpbuf[index].temp_buffer;
			if (temp_buf != NULL) {
				release_page(temp_buf);
				mailbox_free(temp_buf);
			}
#endif
		}
	}
}

static bool is_clicall_params_vaild(const struct tc_call_params *call_params)
{
	if (!call_params) {
		tloge("call param is null");
		return false;
	}

	if (!call_params->dev) {
		tloge("dev file is null");
		return false;
	}

	if (!call_params->context) {
		tloge("client context is null");
		return false;
	}

	return true;
}

static int alloc_for_client_call(struct tc_op_params *op_params)
{
	op_params->smc_cmd = kzalloc(sizeof(*(op_params->smc_cmd)),
		GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(op_params->smc_cmd))) {
		tloge("smc cmd malloc failed\n");
		return -ENOMEM;
	}

	op_params->mb_pack = mailbox_alloc_cmd_pack();
	if (!op_params->mb_pack) {
		kfree(op_params->smc_cmd);
		op_params->smc_cmd = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int init_smc_cmd(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	struct tc_ns_smc_cmd *smc_cmd = op_params->smc_cmd;
	struct tc_ns_client_context *context = call_params->context;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	bool global = call_params->flags & TC_CALL_GLOBAL;

	smc_cmd->cmd_type = global ? CMD_TYPE_GLOBAL : CMD_TYPE_TA;
	if (memcpy_s(smc_cmd->uuid, sizeof(smc_cmd->uuid),
		context->uuid, UUID_LEN) != 0) {
		tloge("memcpy uuid error\n");
		return -EFAULT;
	}
	smc_cmd->cmd_id = context->cmd_id;
	smc_cmd->dev_file_id = call_params->dev->dev_file_id;
	smc_cmd->context_id = context->session_id;
	smc_cmd->err_origin = context->returns.origin;
	smc_cmd->started = context->started;
	smc_cmd->ca_pid = current->pid;
	smc_cmd->pid = current->tgid;

	tlogv("current uid is %u\n", smc_cmd->uid);
	if (context->param_types != 0) {
		smc_cmd->operation_phys =
			mailbox_virt_to_phys((uintptr_t)operation);
		smc_cmd->operation_h_phys =
			(uint64_t)mailbox_virt_to_phys((uintptr_t)operation) >> ADDR_TRANS_NUM;
	} else {
		smc_cmd->operation_phys = 0;
		smc_cmd->operation_h_phys = 0;
	}
	smc_cmd->login_method = context->login.method;

	/* if smc from kernel CA, set login_method to TEEK_LOGIN_IDENTIFY */
	if (call_params->dev->kernel_api == TEE_REQ_FROM_KERNEL_MODE)
		smc_cmd->login_method = TEEK_LOGIN_IDENTIFY;

	return 0;
}

static bool need_check_login(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params)
{
	if (call_params->dev->pub_key_len == sizeof(uint32_t) &&
		op_params->smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		current->mm && ((call_params->flags & TC_CALL_GLOBAL) != 0))
		return true;

	return false;
}

static int check_login_for_encrypt(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	struct tc_ns_session *sess = call_params->sess;
	struct tc_ns_smc_cmd *smc_cmd = op_params->smc_cmd;
	struct mb_cmd_pack *mb_pack = op_params->mb_pack;

	if (need_check_login(call_params, op_params) && sess) {
		if (memcpy_s(mb_pack->login_data, sizeof(mb_pack->login_data),
			sess->auth_hash_buf,
			sizeof(sess->auth_hash_buf)) != 0) {
			tloge("copy login data failed\n");
			return -EFAULT;
		}
		smc_cmd->login_data_phy = mailbox_virt_to_phys((uintptr_t)mb_pack->login_data);
		smc_cmd->login_data_h_addr =
			(uint64_t)mailbox_virt_to_phys((uintptr_t)mb_pack->login_data) >> ADDR_TRANS_NUM;
		smc_cmd->login_data_len = MAX_SHA_256_SZ * (NUM_OF_SO + 1);
	} else {
		smc_cmd->login_data_phy = 0;
		smc_cmd->login_data_h_addr = 0;
		smc_cmd->login_data_len = 0;
	}
	return 0;
}

static uint32_t get_uid_for_cmd(void)
{
	kuid_t kuid;

	kuid = current_uid();
	return kuid.val;
}

static void reset_session_id(const struct tc_call_params *call_params,
	const struct tc_op_params *op_params, int tee_ret)
{
	bool need_reset = false;

	call_params->context->session_id = op_params->smc_cmd->context_id;
	/*
	 * if tee_ret error except TEEC_PENDING,
	 * but context_id is seted,need to reset to 0
	 */
	need_reset = ((call_params->flags & TC_CALL_GLOBAL) &&
		call_params->context->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		tee_ret && tee_ret != (int)TEEC_PENDING);
	if (need_reset)
		call_params->context->session_id = 0;
	return;
}

static void pend_ca_thread(struct tc_ns_session *session,
	const struct tc_ns_smc_cmd *smc_cmd)
{
	struct tc_wait_data *wq = NULL;

	if (session)
		wq = &session->wait_data;

	if (wq) {
		tlogv("before wait event\n");
		/*
		 * use wait_event instead of wait_event_interruptible so
		 * that ap suspend will not wake up the TEE wait call
		 */
		wait_event(wq->send_cmd_wq, wq->send_wait_flag != 0);
		wq->send_wait_flag = 0;
	}
	tlogv("operation start is :%d\n", smc_cmd->started);
	return;
}


static void release_tc_call_resource(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int tee_ret)
{
	/* kfree(NULL) is safe and this check is probably not required */
	call_params->context->returns.code = tee_ret;
	call_params->context->returns.origin = op_params->smc_cmd->err_origin;

	/*
	 * 1. when CA invoke command and crash, Gtask release service node
	 * then del ion won't be triggered, so here tzdriver need to kill ion;
	 * 2. when ta crash, tzdriver also need to kill ion;
	 */
	if (tee_ret == (int)TEE_ERROR_TAGET_DEAD || tee_ret == (int)TEEC_ERROR_GENERIC)
		kill_ion_by_uuid((struct tc_uuid *)op_params->smc_cmd->uuid);

	if (op_params->op_inited)
		free_operation(call_params, op_params);

	kfree(op_params->smc_cmd);
	mailbox_free(op_params->mb_pack);
}

static int config_smc_cmd_context(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	int ret;

	ret = init_smc_cmd(call_params, op_params);
	if (ret != 0)
		return ret;

	ret = check_login_for_encrypt(call_params, op_params);

	return ret;
}

static int handle_ta_pending(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int *tee_ret)
{
	if (*tee_ret != (int)TEEC_PENDING)
		return 0;

	while (*tee_ret == (int)TEEC_PENDING) {
		pend_ca_thread(call_params->sess, op_params->smc_cmd);
		*tee_ret = tc_ns_smc_with_no_nr(op_params->smc_cmd);
	}

	return 0;
}

static int post_proc_smc_return(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int tee_ret)
{
	int ret;

	if (tee_ret != 0) {
		tloge("smc call ret 0x%x, cmd ret val 0x%x, origin %u\n", tee_ret,
			op_params->smc_cmd->ret_val, op_params->smc_cmd->err_origin);
		/* same as libteec_vendor, err from TEE, set ret positive */
		ret = EFAULT;
		if (tee_ret == (int)TEEC_CLIENT_INTR)
			ret = -ERESTARTSYS;

		if (tee_ret == (int)TEEC_ERROR_SHORT_BUFFER)
			(void)update_client_operation(call_params, op_params, false);
	} else {
		tz_log_write();
		ret = update_client_operation(call_params, op_params, true);
	}

	return ret;
}

int tc_client_call(const struct tc_call_params *call_params)
{
	int ret;
	int tee_ret = 0;
	struct tc_op_params op_params = { NULL, NULL, {{0}}, {0}, false };

	if (!is_clicall_params_vaild(call_params))
		return -EINVAL;

	if (alloc_for_client_call(&op_params) != 0)
		return -ENOMEM;

	op_params.smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
	op_params.smc_cmd->uid = get_uid_for_cmd();
	if (call_params->context->param_types != 0) {
		ret = alloc_operation(call_params, &op_params);
		if (ret != 0)
			goto free_src;
	}

	ret = config_smc_cmd_context(call_params, &op_params);
	if (ret != 0)
		goto free_src;

	tee_ret = tc_ns_smc(op_params.smc_cmd);

	reset_session_id(call_params, &op_params, tee_ret);

	ret = handle_ta_pending(call_params, &op_params, &tee_ret);
	if (ret != 0)
		goto free_src;

	ret = post_proc_smc_return(call_params, &op_params, tee_ret);

free_src:
	if (ret < 0) /* if ret > 0, means err from TEE */
		op_params.smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
	release_tc_call_resource(call_params, &op_params, tee_ret);
	return ret;
}
