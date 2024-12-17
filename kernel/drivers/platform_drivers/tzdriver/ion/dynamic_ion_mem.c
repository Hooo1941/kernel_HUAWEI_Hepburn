/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: dynamic Ion memory allocation and free.
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
#include "dynamic_ion_mem.h"
#include <stdarg.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/cma.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#if ((defined CONFIG_ION_MM) || (defined CONFIG_ION_MM_SECSG))
#include <linux/ion/mm_ion.h>
#endif
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "smc_smp.h"
#include "gp_ops.h"
#include "teek_client_constants.h"
#include "mailbox_mempool.h"
#include "dynamic_ion_uuid.h"

static DEFINE_MUTEX(dynamic_mem_lock);
struct dynamic_mem_list {
	struct list_head list;
};

static const struct dynamic_mem_config g_dyn_mem_config[] = {
	{ TEE_SECIDENTIFICATION1, SEC_EID },
	{ TEE_SECIDENTIFICATION3, SEC_EID },
	{ TEE_SERVICE_AI, SEC_AI_ION },
	{ TEE_SERVICE_AI_TINY, SEC_AI_ION },
	{ TEE_SERVICE_VCODEC, SEC_DRM_TEE },
};

static struct dynamic_mem_list g_dynamic_mem_list;
static const uint32_t g_dyn_mem_config_num = ARRAY_SIZE(g_dyn_mem_config);

static int release_ion_srv(const struct tc_uuid *uuid)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };

	smc_cmd.err_origin = TEEC_ORIGIN_COMMS;
	smc_cmd.cmd_type = CMD_TYPE_GLOBAL;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_RELEASE_ION_SRV;
	if (memcpy_s(&smc_cmd.uuid, sizeof(smc_cmd.uuid), uuid, sizeof(*uuid))) {
		tloge("copy uuid failed\n");
		return -ENOMEM;
	}

	if (tc_ns_smc(&smc_cmd)) {
		tloge("send release ion srv cmd failed\n");
		return -EPERM;
	}
	return 0;
}

static int get_ion_sglist(struct dynamic_mem_item *mem_item)
{
	struct sglist *tmp_sglist = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	uint32_t sglist_size;
	uint32_t i = 0;
	struct sg_table *ion_sg_table = mem_item->memory.dyn_sg_table;

	if (!ion_sg_table)
		return -EINVAL;

	if (ion_sg_table->nents <= 0 || ion_sg_table->nents > MAX_ION_NENTS)
		return -EINVAL;

	for_each_sg(ion_sg_table->sgl, sg, ion_sg_table->nents, i) {
		if (!sg) {
			tloge("an error sg when get ion sglist\n");
			return -EINVAL;
		}
	}

	sglist_size = sizeof(struct ion_page_info) * ion_sg_table->nents +
		sizeof(*tmp_sglist);
	tmp_sglist = (struct sglist *)mailbox_alloc(sglist_size, MB_FLAG_ZERO);
	if (!tmp_sglist) {
		tloge("mailbox alloc failed\n");
		return -ENOMEM;
	}
	tmp_sglist->sglist_size = (uint64_t)sglist_size;
	tmp_sglist->ion_size = (uint64_t)mem_item->size;
	tmp_sglist->info_length = (uint64_t)ion_sg_table->nents;
	for_each_sg(ion_sg_table->sgl, sg, ion_sg_table->nents, i) {
		page = sg_page(sg);
		tmp_sglist->page_info[i].phys_addr = page_to_phys(page);
		tmp_sglist->page_info[i].npages = sg->length / PAGE_SIZE;
	}
	mem_item->memory.ion_phys_addr = mailbox_virt_to_phys((uintptr_t)(void *)tmp_sglist);
	mem_item->memory.len = sglist_size;
	return 0;
}

static int send_dyn_ion_cmd(struct dynamic_mem_item *mem_item, unsigned int cmd_id, int32_t *ret_origin)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret;
	struct mb_cmd_pack *mb_pack = NULL;

	if (!mem_item) {
		tloge("mem_item is null\n");
		return -EINVAL;
	}

	ret = get_ion_sglist(mem_item);
	if (ret != 0)
		return ret;

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		mailbox_free(phys_to_virt(mem_item->memory.ion_phys_addr));
		tloge("alloc cmd pack failed\n");
		return -ENOMEM;
	}
	smc_cmd.cmd_type = CMD_TYPE_GLOBAL;
	smc_cmd.cmd_id = cmd_id;
	smc_cmd.err_origin = TEEC_ORIGIN_COMMS;
	mb_pack->operation.paramtypes = teec_param_types(
		TEE_PARAM_TYPE_ION_SGLIST_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_NONE);

	mb_pack->operation.params[0].memref.size = (uint32_t)mem_item->memory.len;
	mb_pack->operation.params[0].memref.buffer =
		(uint32_t)(mem_item->memory.ion_phys_addr & 0xFFFFFFFF);
	mb_pack->operation.buffer_h_addr[0] =
		(uint64_t)(mem_item->memory.ion_phys_addr) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[1].value.a = (uint32_t)mem_item->size;
	mb_pack->operation.params[2].value.a = mem_item->configid;
	smc_cmd.operation_phys =
		(unsigned int)mailbox_virt_to_phys((uintptr_t)&mb_pack->operation);
	smc_cmd.operation_h_phys =
		(uint64_t)mailbox_virt_to_phys((uintptr_t)&mb_pack->operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(&smc_cmd)) {
		if (ret_origin)
			*ret_origin = smc_cmd.err_origin;
		ret = -EPERM;
		tlogd("send loadapp ion failed\n");
	}
	mailbox_free(phys_to_virt(mem_item->memory.ion_phys_addr));
	mailbox_free(mb_pack);
	return ret;
}

static struct dynamic_mem_item *find_memitem_by_configid_locked(
	uint32_t configid)
{
	struct dynamic_mem_item *item = NULL;

	list_for_each_entry(item, &g_dynamic_mem_list.list, head) {
		if (item->configid == configid)
			return item;
	}

	return NULL;
}

static struct dynamic_mem_item *find_memitem_by_uuid_locked(
	const struct tc_uuid *uuid)
{
	struct dynamic_mem_item *item = NULL;

	list_for_each_entry(item, &g_dynamic_mem_list.list, head) {
		if (!memcmp(&item->uuid, uuid, sizeof(*uuid)))
			return item;
	}

	return NULL;
}

#define BLOCK_64KB_SIZE   (64 * 1024) /* 64KB */
#define BLOCK_64KB_MASK   0xFFFFFFFFFFFF0000
/* size should be aligned with 64KB */
#define BLOCK_64KB_SIZE_MASK (BLOCK_64KB_SIZE - 1)
static int proc_alloc_dyn_mem(struct dynamic_mem_item *mem_item)
{
	struct sg_table *ion_sg_table = NULL;

	if (mem_item->size + BLOCK_64KB_SIZE_MASK < mem_item->size) {
		tloge("ion size is error, size = %x\n", mem_item->size);
		return -EINVAL;
	}
	mem_item->memory.len = (mem_item->size + BLOCK_64KB_SIZE_MASK) &
		BLOCK_64KB_MASK;

	ion_sg_table = mm_secmem_alloc(mem_item->ddr_sec_region,
		mem_item->memory.len);
	if (!ion_sg_table) {
		tloge("failed to get ion page, configid=%d\n",
			mem_item->configid);
		return -ENOMEM;
	}
	mem_item->memory.dyn_sg_table = ion_sg_table;
	return 0;
}

static void proc_free_dyn_mem(struct dynamic_mem_item *mem_item)
{
	if (!mem_item->memory.dyn_sg_table) {
		tloge("ion_phys_addr is NULL\n");
		return;
	}
	mm_secmem_free(mem_item->ddr_sec_region,
		mem_item->memory.dyn_sg_table);
	mem_item->memory.dyn_sg_table = NULL;
	return;
}

int init_dynamic_mem(void)
{
	INIT_LIST_HEAD(&(g_dynamic_mem_list.list));
	return 0;
}

static int32_t find_ddr_sec_region_by_uuid(const struct tc_uuid *uuid,
	uint32_t *ddr_sec_region)
{
	uint32_t i;

	for (i = 0; i < g_dyn_mem_config_num; i++) {
		if (!memcmp(&(g_dyn_mem_config[i].uuid), uuid,
			sizeof(*uuid))) {
			*ddr_sec_region = g_dyn_mem_config[i].ddr_sec_region;
			return 0;
		}
	}
	return -EINVAL;
}

static struct dynamic_mem_item *alloc_dyn_mem_item(uint32_t configid,
	uint32_t cafd, const struct tc_uuid *uuid, uint32_t size)
{
	uint32_t ddr_sec_region;
	struct dynamic_mem_item *mem_item = NULL;
	int32_t result;

	result = find_ddr_sec_region_by_uuid(uuid, &ddr_sec_region);
	if (result != 0) {
		tloge("find ddr sec region failed\n");
		return NULL;
	}

	mem_item = kzalloc(sizeof(*mem_item), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)mem_item)) {
		tloge("alloc mem item failed\n");
		return NULL;
	}

	mem_item->ddr_sec_region = ddr_sec_region;
	mem_item->configid = configid;
	mem_item->size = size;
	mem_item->cafd = cafd;
	result = memcpy_s(&mem_item->uuid, sizeof(mem_item->uuid), uuid,
		sizeof(*uuid));
	if (result != EOK) {
		tloge("memcpy uuid failed\n");
		kfree(mem_item);
		return NULL;
	}
	return mem_item;
}

static int trans_configid2memid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size, int32_t *ret_origin)
{
	int result;

	if (!uuid)
		return -EINVAL;

	/* if config id is memid map,and can reuse */
	mutex_lock(&dynamic_mem_lock);
	do {
		struct dynamic_mem_item *mem_item =
			find_memitem_by_configid_locked(configid);
		if (mem_item) {
			result = -EINVAL;
			break;
		}

		mem_item = alloc_dyn_mem_item(configid, cafd, uuid, size);
		if (!mem_item) {
			tloge("alloc dyn mem item failed\n");
			result = -ENOMEM;
			break;
		}

		result = proc_alloc_dyn_mem(mem_item);
		if (result != 0) {
			tloge("alloc dyn mem failed ,ret = %d\n", result);
			kfree(mem_item);
			break;
		}
		/* register to tee */
		result = send_dyn_ion_cmd(mem_item, GLOBAL_CMD_ID_ADD_DYNAMIC_ION, ret_origin);
		if (result != 0) {
			tloge("register to tee failed ,result =%d\n", result);
			proc_free_dyn_mem(mem_item);
			kfree(mem_item);
			break;
		}
		list_add_tail(&mem_item->head, &g_dynamic_mem_list.list);
		tloge("log import:alloc ion configid=%d\n",
			mem_item->configid);
	} while (0);

	mutex_unlock(&dynamic_mem_lock);
	return result;
}

static void release_configid_mem_locked(uint32_t configid)
{
	int result;

	/* if config id is memid map,and can reuse */
	do {
		struct dynamic_mem_item *mem_item =
			find_memitem_by_configid_locked(configid);
		if (!mem_item) {
			tloge("fail to find memitem by configid\n");
			break;
		}

		result = send_dyn_ion_cmd(mem_item, GLOBAL_CMD_ID_DEL_DYNAMIC_ION, NULL);
		if (result != 0) {
			tloge("unregister_from_tee configid=%d,result=%d\n",
				mem_item->configid, result);
			break;
		}
		proc_free_dyn_mem(mem_item);
		list_del(&mem_item->head);
		kfree(mem_item);
		tloge("log import: free ion\n");
	} while (0);

	return;
}

int load_app_use_configid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size, int32_t *ret_origin)
{
	int result;

	if (!uuid)
		return -EINVAL;

	result = trans_configid2memid(configid, cafd, uuid, size, ret_origin);
	if (result != 0) {
		tloge("trans_configid2memid failed ret=%d\n", result);
		if (release_ion_srv(uuid) != 0)
			tloge("release ion srv failed\n");
	}
	return result;
}

void kill_ion_by_uuid(const struct tc_uuid *uuid)
{
	if (!uuid) {
		tloge("uuid is null\n");
		return;
	}
	mutex_lock(&dynamic_mem_lock);
	do {
		struct dynamic_mem_item *mem_item =
			find_memitem_by_uuid_locked(uuid);
		if (!mem_item)
			break;
		tlogd("kill ION by UUID\n");
		release_configid_mem_locked(mem_item->configid);
	} while (0);
	mutex_unlock(&dynamic_mem_lock);
}

void kill_ion_by_cafd(unsigned int cafd)
{
	struct dynamic_mem_item *item = NULL;
	struct dynamic_mem_item *temp = NULL;

	tlogd("kill_ion_by_cafd:\n");
	mutex_lock(&dynamic_mem_lock);
	list_for_each_entry_safe(item, temp, &g_dynamic_mem_list.list, head) {
		if (item->cafd == cafd)
			release_configid_mem_locked(item->configid);
	}
	mutex_unlock(&dynamic_mem_lock);
}

int load_image_for_ion(const struct load_img_params *params, int32_t *ret_origin)
{
	int ret = 0;

	if (!params)
		return -EFAULT;
	/* check need to add ionmem  */
	uint32_t configid = params->mb_pack->operation.params[1].value.a;
	uint32_t ion_size = params->mb_pack->operation.params[1].value.b;
	int32_t check_result = (configid != 0 && ion_size != 0);

	tloge("check load result=%d, cfgid =%d, ion_size =%d, uuid=%x\n",
		check_result, configid, ion_size,
		params->uuid_return->time_low);
	if (check_result) {
		ret = load_app_use_configid(configid,
			params->dev_file->dev_file_id,
			params->uuid_return, ion_size, ret_origin);
		if (ret != 0) {
			tloge("load app use configid failed ret =%d\n", ret);
			return -EFAULT;
		}
	}
	return ret;
}

bool is_ion_param(uint32_t param_type)
{
	if (param_type == TEEC_ION_INPUT ||
		param_type == TEEC_ION_SGLIST_INPUT)
		return true;

	return false;
}

static void fill_sg_list(struct sg_table *ion_table,
	uint32_t ion_list_num, struct sglist *tmp_sglist)
{
	uint32_t i;
	struct page *page = NULL;
	struct scatterlist *sg = NULL;

	for_each_sg(ion_table->sgl, sg, ion_list_num, i) {
		page = sg_page(sg);
		tmp_sglist->page_info[i].phys_addr =
			page_to_phys(page);
		tmp_sglist->page_info[i].npages =
			sg->length / PAGE_SIZE;
	}
}

static int check_sg_list(const struct sg_table *ion_table, uint32_t ion_list_num)
{
	struct scatterlist *sg = NULL;
	uint32_t i;

	for_each_sg(ion_table->sgl, sg, ion_list_num, i) {
		if (!sg) {
			tloge("an error sg when get ion sglist \n");
			return -EFAULT;
		}
	}

	return 0;
}

static int get_ion_sg_list_from_fd(uint32_t ion_shared_fd,
	uint32_t ion_alloc_size, phys_addr_t *sglist_table,
	size_t *ion_sglist_size)
{
	struct sg_table *ion_table = NULL;
	struct sglist *tmp_sglist = NULL;
	uint64_t ion_id = 0;
	enum SEC_SVC ion_type = 0;
	uint32_t ion_list_num = 0;
	uint32_t sglist_size;
	if (secmem_get_buffer(ion_shared_fd, &ion_table, &ion_id, &ion_type)) {
		tloge("get ion table failed. \n");
		return -EFAULT;
	}

	if (ion_type != SEC_DRM_TEE) {
		if (ion_table->nents <= 0 || ion_table->nents > MAX_ION_NENTS)
			return -EFAULT;
		ion_list_num = (uint32_t)(ion_table->nents & INT_MAX);
		if (check_sg_list(ion_table, ion_list_num) != 0)
			return -EFAULT;
	}
	/* ion_list_num is less than 1024, so sglist_size won't flow */
	sglist_size = sizeof(struct ion_page_info) * ion_list_num +
		sizeof(*tmp_sglist);
	tmp_sglist = (struct sglist *)mailbox_alloc(sglist_size, MB_FLAG_ZERO);
	if (!tmp_sglist) {
		tloge("sglist mem alloc failed\n");
		return -ENOMEM;
	}
	tmp_sglist->sglist_size = (uint64_t)sglist_size;
	tmp_sglist->ion_size = (uint64_t)ion_alloc_size;
	tmp_sglist->info_length = (uint64_t)ion_list_num;
	if (ion_type != SEC_DRM_TEE)
		fill_sg_list(ion_table, ion_list_num, tmp_sglist);
	else
		tmp_sglist->ion_id = ion_id;

	*sglist_table = mailbox_virt_to_phys((uintptr_t)tmp_sglist);
	*ion_sglist_size = sglist_size;
	return 0;
}

int alloc_for_ion_sglist(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	struct tc_ns_operation *operation = NULL;
	size_t ion_sglist_size = 0;
	phys_addr_t ion_sglist_addr = 0x0;
	union tc_ns_client_param *client_param = NULL;
	unsigned int ion_shared_fd = 0;
	unsigned int ion_alloc_size;
	uint64_t a_addr, b_addr;

	/* this never happens */
	if (index >= TEE_PARAM_NUM || !call_params || !op_params)
		return -EINVAL;

	operation = &op_params->mb_pack->operation;
	client_param = &(call_params->context->params[index]);
	a_addr = client_param->value.a_addr |
		((uint64_t)client_param->value.a_h_addr << ADDR_TRANS_NUM);
	b_addr = client_param->value.b_addr |
		((uint64_t)client_param->value.b_h_addr << ADDR_TRANS_NUM);

	if (read_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)a_addr,
		sizeof(operation->params[index].value.a), kernel_params)) {
		tloge("valuea copy failed\n");
		return -EFAULT;
	}
	if (read_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)b_addr,
		sizeof(operation->params[index].value.b), kernel_params)) {
		tloge("valueb copy failed\n");
		return -EFAULT;
	}
	ion_shared_fd = operation->params[index].value.a;
	ion_alloc_size = operation->params[index].value.b;

	if (get_ion_sg_list_from_fd(ion_shared_fd, ion_alloc_size,
		&ion_sglist_addr, &ion_sglist_size)) {
		tloge("get ion sglist failed, fd=%u\n", ion_shared_fd);
		return -EFAULT;
	}
	op_params->local_tmpbuf[index].temp_buffer =
		phys_to_virt(ion_sglist_addr);
	op_params->local_tmpbuf[index].size = ion_sglist_size;

	operation->params[index].memref.buffer = (unsigned int)ion_sglist_addr;
	operation->buffer_h_addr[index] =
		(uint64_t)ion_sglist_addr >> ADDR_TRANS_NUM;
	operation->params[index].memref.size = (unsigned int)ion_sglist_size;
	op_params->trans_paramtype[index] = param_type;

	return 0;
}

static int transfer_ion_params(struct tc_ns_operation *operation,
	union tc_ns_client_param *client_param, uint8_t kernel_params,
	unsigned int index)
{
	uint64_t a_addr = client_param->value.a_addr |
		((uint64_t)client_param->value.a_h_addr << ADDR_TRANS_NUM);
	uint64_t b_addr = client_param->value.b_addr |
		((uint64_t)client_param->value.b_h_addr << ADDR_TRANS_NUM);

	if (read_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)a_addr,
		sizeof(operation->params[index].value.a), kernel_params)) {
		tloge("value.a_addr copy failed\n");
		return -EFAULT;
	}

	if (read_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)b_addr,
		sizeof(operation->params[index].value.b), kernel_params)) {
		tloge("value.b_addr copy failed\n");
		return -EFAULT;
	}

	return 0;
}

int alloc_for_ion(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	struct tc_ns_operation *operation = NULL;
	size_t drm_ion_size = 0;
	phys_addr_t drm_ion_phys = 0x0;
	struct dma_buf *drm_dma_buf = NULL;
	union tc_ns_client_param *client_param = NULL;
	unsigned int ion_shared_fd = 0;
	int ret = 0;

	/* this never happens */
	if (index >= TEE_PARAM_NUM || !call_params || !op_params)
		return -EINVAL;

	operation = &op_params->mb_pack->operation;
	client_param = &(call_params->context->params[index]);
	if (transfer_ion_params(operation, client_param, kernel_params, index))
		return -EFAULT;

	ion_shared_fd = operation->params[index].value.a;
	drm_dma_buf = dma_buf_get(ion_shared_fd);
	if (IS_ERR_OR_NULL(drm_dma_buf)) {
		tloge("drm dma buf is err, ret=%d fd=%u\n", ret, ion_shared_fd);
		return -EFAULT;
	}
	ret = ion_secmem_get_phys(drm_dma_buf, &drm_ion_phys, &drm_ion_size);
	if (ret != 0) {
		tloge("in %s err:ret=%d fd=%u\n", __func__, ret, ion_shared_fd);
		dma_buf_put(drm_dma_buf);
		return -EFAULT;
	}

	if (drm_ion_size > operation->params[index].value.b)
		drm_ion_size = operation->params[index].value.b;
	operation->params[index].value.a = (unsigned int)drm_ion_phys;
	operation->params[index].value.b = (unsigned int)drm_ion_size;
	op_params->trans_paramtype[index] = param_type;
	dma_buf_put(drm_dma_buf);

	return ret;
}
