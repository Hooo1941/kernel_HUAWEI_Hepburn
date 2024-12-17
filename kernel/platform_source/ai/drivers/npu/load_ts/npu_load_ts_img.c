/*
 * npu_load_ts_img.c
 *
 * about load and verify npu tscpu firmware img to reserved addr
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#include "npu_load_ts_img.h"

#include <platform_include/see/sec_auth_ca.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <platform_include/basicplatform/linux/partition/partition_ap_kernel.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <platform_include/basicplatform/linux/partition/partition_macro.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <linux/version.h>
#include <securec.h>
#include <linux/completion.h>
#include <linux/time.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include "npu_log.h"
#include "npu_adapter.h"

#define SEC_AUTH_SEND_MAX_SIZE 0x100000  /* 1M */
#define SEC_AUTH_NAME          "sec_auth"
#define NPU_LOAD_TS_QUEUE_NAME "npu_load_ts_queue"
#define NPU_LOAD_S_TO_US_MULT  1000000
#define NPU_LOAD_NS_TO_US_DIV  1000
#define NPU_LOAD_WAITING_TIMEOUT_IN_MS 500

/* Kernel can use the fp->f_inode->i_size method to read the size of the image, but this method is
   invalid for the partition file. The size of the image read by the partition file is always 0,
   so it can only be set according to the empirical value, and a certain amount is reserved.
   The size of the image signed by the TSCPU Firmware is less than 150K, add a part of the margin,
   hardcode it to 256K. If the size of the image signed by the TSCPU Firmware exceeds 256K in the future,
   this MACRO needs to be modified. */
#define SEC_TSFW_MAX_SIZE 0x40000   /* 256K */

enum npu_tsfw_load_status {
	NPU_TSFW_UNLOAD,
	NPU_TSFW_LOADING,
	NPU_TSFW_LOADED_SUCC,
	NPU_TSFW_LOADED_FAIL,
};

struct npu_load_ts_work_ctx {
	int32_t result;
	struct completion completion;
	struct work_struct work;
	struct workqueue_struct *queue;
};

struct npu_load_ts_work_ctx g_load_npu_work_ctx;

static atomic_t g_load_status = ATOMIC_INIT(NPU_TSFW_UNLOAD);

static inline struct npu_load_ts_work_ctx * load_npu_get_ctx(void)
{
	return &g_load_npu_work_ctx;
}

static int32_t load_npu_teek_init(TEEC_Session *session, TEEC_Context *context)
{
	TEEC_Result result;
	TEEC_UUID svc_uuid = TEE_SERVICE_SECAUTH;
	TEEC_Operation operation = {0};
	char package_name[] = SEC_AUTH_NAME;
	uint32_t root_id = 0;

	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		npu_drv_err("[load_npu]TEEK_InitializeContext fail res=0x%x\n", result);
		return -EACCES;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);

	/* 2 is TEEC_RegisteredMemoryReference type */
	operation.params[2].tmpref.buffer = (void *)(&root_id);
	operation.params[2].tmpref.size = (uint32_t)sizeof(root_id);

	/* 3 is TEEC_Value type */
	operation.params[3].tmpref.buffer = (void *)(package_name);
	operation.params[3].tmpref.size = (uint32_t)(strlen(package_name) + 1);
	result = TEEK_OpenSession(context, session, &svc_uuid, TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		npu_drv_err("[load_npu]TEEK_OpenSession failed result=0x%x!\n", result);
		TEEK_FinalizeContext(context);
		return -EACCES;
	}

	npu_drv_debug("[load_npu]load_npu_teek_init succ\n");
	return 0;
}

static void load_npu_teek_deinit(TEEC_Session *session, TEEC_Context *context)
{
	TEEK_CloseSession(session);
	TEEK_FinalizeContext(context);
}

static int32_t load_npu_node_init(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	uint32_t origin = 0;

	npu_drv_debug("[load_npu]SEC_AUTH_CMD_ID_INIT begin\n");

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_INIT, operation, &origin);
	if (result != TEEC_SUCCESS) {
		npu_drv_err("[load_npu]init ret=0x%x\n", result);
		return -ENOTTY;
	}

	npu_drv_debug("[load_npu]load_npu_node_init succ\n");
	return 0;
}

static int32_t load_npu_node_read_cert(TEEC_Session *session, TEEC_Operation *operation, uintptr_t addr, uint32_t size)
{
	TEEC_Result result;
	uint32_t origin = 0;

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
	operation->params[1].tmpref.buffer = (void *)addr;
	operation->params[1].tmpref.size = size;
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_READ_CERT, operation, &origin);
	if (result != TEEC_SUCCESS) {
		npu_drv_err("[load_npu]read_cert ret=0x%x\n", result);
		return -ENOTTY;
	}

	return 0;
}

static int32_t load_npu_node_read_data(TEEC_Session *session, TEEC_Operation *operation, uintptr_t addr, uint32_t size)
{
	TEEC_Result result;
	uint32_t origin = 0;
	uint32_t send_size = 0;
	uint32_t cur_size = 0;

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
	while(send_size < size) {
		/* only can send 1M data per time in CA-TA, if size>1M, send to CA in (size/1M) times */
		cur_size = size - send_size;
		if (cur_size > SEC_AUTH_SEND_MAX_SIZE)
			cur_size = SEC_AUTH_SEND_MAX_SIZE;

		operation->params[1].tmpref.buffer = (void *)(addr + (uintptr_t)send_size);
		operation->params[1].tmpref.size = cur_size;
		operation->params[2].value.a = send_size;
		operation->params[2].value.b = size;
		result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_READ_DATA, operation, &origin);
		if (result != TEEC_SUCCESS) {
			npu_drv_err("[load_npu]read_data send_size=%u cur_size=%u ret=0x%x\n", send_size, cur_size, result);
			return (int32_t)result;
		}
		send_size += cur_size;
	}

	return 0;
}

static int32_t load_npu_node_verify(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	uint32_t origin = 0;
	struct authed_data out = {
		.items = {
			{ .item_id = CERT_ITEM_IMAGE_LOAD_SIZE },
			{ .item_id = CERT_ITEM_IMAGE_HASH },
		},
	};

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEE_PARAM_TYPE_MEMREF_INOUT, TEEC_NONE, TEEC_NONE);
	operation->params[1].tmpref.buffer = (void *)&out;
	operation->params[1].tmpref.size = (uint32_t)sizeof(out);
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_VERIFY, operation, &origin);
	if (result != TEEC_SUCCESS) {
		npu_drv_err("[load_npu]verify ret=0x%x\n", result);
		return -ENOTTY;
	}

	return 0;
}

static int32_t load_npu_node_final(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	uint32_t origin = 0;

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE,  TEEC_NONE);
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_FINAL, operation, &origin);
	if (result != TEEC_SUCCESS) {
		npu_drv_err("[load_npu]final ret=0x%x\n", result);
		return -ENOTTY;
	}

	return 0;
}

static int32_t load_npu_read_bin(struct file *fp, uint32_t offset, uint32_t length, char *buffer)
{
	ssize_t ret;
	loff_t read_offset = offset;

	ret = kernel_read(fp, buffer, length, &read_offset);
	if (ret != (ssize_t)length) {
		npu_drv_err("[load_npu]read ops fail ret=0x%x len=0x%x\n", ret, length);
		return -EIO;
	}

	return 0;
}

static struct file * load_npu_open_partition(const char *partition_name)
{
	struct file *fp = NULL;
	int32_t ret = 0;
	mm_segment_t oldfs;

	char *partition_path = (char *)kzalloc(PARTITION_NAME_LEN_MAX, GFP_KERNEL);
	if (partition_path == NULL) {
		npu_drv_err("[load_npu]kzalloc for npu_partition fail\n");
		return NULL;
	}

	/* find partition info in ptable */
	ret = flash_find_ptn_s(partition_name, partition_path, PARTITION_NAME_LEN_MAX);
	if (ret != 0) {
		npu_drv_err("[load_npu]partition of npu not in ptable, ret=0x%x\n", ret);
		kfree(partition_path);
		return NULL;
	}

	pr_debug("[load_npu]partition name=%s\n", partition_path);

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	/* open ts partition filp in read-only mode */
	fp = filp_open(partition_path, O_RDONLY, 0400);
	if (IS_ERR(fp)) {
		npu_drv_err("[load_npu]open ts partition filp in read-only mode failed 0x%llx, path=%s\n", fp, partition_path);
		fp = NULL;
	}

	kfree(partition_path);
	set_fs(oldfs);

	return fp;
}

static int32_t load_npu_malloc(uintptr_t *cert_addr, uintptr_t *data_addr)
{
	*cert_addr = (uintptr_t)vmalloc(SEC_AUTH_CERT_SIZE);
	if (*cert_addr == (uintptr_t)NULL) {
		npu_drv_err("[load_npu]vmalloc(SEC_AUTH_CERT_SIZE) fail, no memory\n");
		return -ENOMEM;
	}

	*data_addr = (uintptr_t)vmalloc(SEC_TSFW_MAX_SIZE - SEC_AUTH_CERT_SIZE);
	if (*data_addr == (uintptr_t)NULL) {
		npu_drv_err("[load_npu]vmalloc(data_size) fail, no memory\n");
		vfree((void *)*cert_addr);
		*cert_addr = (uintptr_t)NULL;
		return -ENOMEM;
	}

	return 0;
}

static void load_npu_free(uintptr_t *cert_addr, uintptr_t *data_addr)
{
	if (*cert_addr != 0) {
		vfree((void *)(*cert_addr));
		*cert_addr = 0;
	}

	if (*data_addr != 0) {
		vfree((void *)(*data_addr));
		*data_addr = 0;
	}
}

static int32_t load_npu_read(const char *partition_name, uintptr_t *cert_addr, uintptr_t *data_addr)
{
	int32_t ret = 0;
	uint32_t data_size = SEC_TSFW_MAX_SIZE - SEC_AUTH_CERT_SIZE;
	struct file *fp = NULL;

	fp = load_npu_open_partition(partition_name);
	if (fp == NULL)
		return -ENOENT;

	ret = load_npu_read_bin(fp, 0, SEC_AUTH_CERT_SIZE, (char *)(*cert_addr));
	if (ret != 0) {
		npu_drv_err("[load_npu]prepare npu cert ret=%d\n", ret);
		filp_close(fp, NULL);
		return ret;
	}

	ret = load_npu_read_bin(fp, SEC_AUTH_CERT_SIZE, data_size, (char *)(*data_addr));
	if (ret != 0) {
		npu_drv_err("[load_npu]prepare npu data ret=%d\n", ret);
		filp_close(fp, NULL);
		return ret;
	}

	npu_drv_debug("[load_npu]load_npu_read cert and data succ\n", ret);
	filp_close(fp, NULL);

	return ret;
}

static int32_t load_npu_deliver(uintptr_t cert_addr, uintptr_t data_addr,
	TEEC_Session *session, TEEC_Operation *operation)
{
	int32_t ret = 0;

	npu_drv_debug("[load_npu]load_npu_node_read_cert +\n");
	ret = load_npu_node_read_cert(session, operation, cert_addr, SEC_AUTH_CERT_SIZE);
	if (ret != 0) {
		npu_drv_err("[load_npu]load_npu node read cert failed, ret is 0x%x\n", ret);
		return ret;
	}
	npu_drv_debug("[load_npu]load_npu_node_read_cert -\n");

	npu_drv_debug("[load_npu]load_npu_node_read_data +\n");
	ret = load_npu_node_read_data(session, operation, data_addr, SEC_TSFW_MAX_SIZE - SEC_AUTH_CERT_SIZE);
	if (ret != 0) {
		npu_drv_err("[load_npu]load_npu node read data failed, ret is 0x%x\n", ret);
		return ret;
	}
	npu_drv_debug("[load_npu]load_npu_node_read_data -\n");

	return 0;
}

static int32_t load_npu_verify(TEEC_Session *session, TEEC_Operation *operation)
{
	int32_t ret = 0;

	npu_drv_debug("[load_npu]load_npu_node_verify +\n");
	ret = load_npu_node_verify(session, operation);
	if (ret != 0) {
		npu_drv_err("[load_npu]load_npu verify failed, ret is 0x%x\n", ret);
		return ret;
	}
	npu_drv_debug("[load_npu]load_npu_node_verify -\n");

	return 0;
}

static void load_npu_deinit(TEEC_Session *session, TEEC_Operation *operation)
{
	int32_t ret = 0;

	npu_drv_debug("[load_npu]load_npu_node_final +\n");
	ret = load_npu_node_final(session, operation);
	if (ret != 0) {
		npu_drv_err("[load_npu]load_npu final failed, ret is 0x%x\n", ret);
		return;
	}

	npu_drv_debug("[load_npu]load_npu_node_final -\n");
}

static int32_t load_npu_verify_work(void)
{
	int32_t ret;
	TEEC_Session session;
	TEEC_Context context;
	TEEC_Operation operation = { 0 };
	uintptr_t cert_addr = 0;
	uintptr_t data_addr = 0;
	struct image_id_t id = {
		.partition_name = "npu",
		.image_name = "npu_fw",
	};

	ret = load_npu_malloc(&cert_addr, &data_addr);
	if (ret != 0)
		return ret;

	ret = load_npu_read((const char *)&id.partition_name[0], &cert_addr, &data_addr);
	if (ret != 0)
		goto free_heap_memory;

	ret = load_npu_teek_init(&session, &context);
	if (ret != 0)
		goto free_heap_memory;

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.params[0].tmpref.buffer = (void *)&id;
	operation.params[0].tmpref.size = (uint32_t)sizeof(id);
	ret = load_npu_node_init(&session, &operation);
	if (ret != 0)
		goto teek_deinit;

	ret = load_npu_deliver(cert_addr, data_addr, &session, &operation);
	if (ret != 0)
		goto npu_node_deinit;

	ret = load_npu_verify(&session, &operation);

npu_node_deinit:
	load_npu_deinit(&session, &operation);
teek_deinit:
	load_npu_teek_deinit(&session, &context);
free_heap_memory:
	load_npu_free(&cert_addr, &data_addr);

	return ret;
}

static void load_npu_ts_work(struct work_struct *work)
{
	int32_t ret = -1;
	struct npu_load_ts_work_ctx *ctx = NULL;
	const uint32_t loop_max = 3;
	uint32_t loop = 0;

	ctx = container_of(work, struct npu_load_ts_work_ctx, work);
	while (ret != 0 && loop < loop_max) {
		ret = load_npu_verify_work();
		npu_drv_debug("[load_npu]exit, ret=0x%x!\n", ret);
		loop++;
	}
	ctx->result = ret;
	complete(&ctx->completion);
}

static int32_t load_npu_ts_work_ctx_init(struct npu_load_ts_work_ctx *ctx)
{
	struct workqueue_struct *queue = NULL;

	queue = create_singlethread_workqueue(NPU_LOAD_TS_QUEUE_NAME);
	if (IS_ERR_OR_NULL(queue)) {
		npu_drv_err("[load_npu]create_workqueue error\n");
		return -EFAULT;
	}

	ctx->queue = queue;
	INIT_WORK(&ctx->work, load_npu_ts_work);
	init_completion(&ctx->completion);

	return 0;
}

static void load_npu_ts_work_ctx_deinit(struct npu_load_ts_work_ctx *ctx)
{
	destroy_workqueue(ctx->queue);
}

void load_npu_reset_status(void)
{
	atomic_set(&g_load_status, NPU_TSFW_UNLOAD);
}

int load_npu_verify_image(void)
{
	int32_t ret = 0;
	struct npu_load_ts_work_ctx *work_ctx = load_npu_get_ctx();
	struct timespec64 start_tv, end_tv;
	uint32_t timer = 0;
	const uint32_t time_wait = 1;

	/* In most scene, load_status should be SUCC, so premote this judgement here before cmpxchg */
	if (atomic_read(&g_load_status) == NPU_TSFW_LOADED_SUCC) {
		npu_drv_warn("ts is loaded");
		return 0;
	}
	do {
		ret = atomic_cmpxchg(&g_load_status, NPU_TSFW_UNLOAD, NPU_TSFW_LOADING);
		if (ret == NPU_TSFW_LOADED_SUCC) {
			return 0;
		} else if (ret == NPU_TSFW_LOADED_FAIL) {
			return -ENOTRECOVERABLE;
		} else if (ret == NPU_TSFW_UNLOAD) {
			break;
		}
		/* ret == NPU_TSFW_LOADING */
		npu_drv_debug("tsfw is loading");
		timer += time_wait;
		mdelay(time_wait);
	} while (timer < NPU_LOAD_WAITING_TIMEOUT_IN_MS);
	if (timer >= NPU_LOAD_WAITING_TIMEOUT_IN_MS) {
		npu_drv_err("[load_npu]load ts image timeout");
		return -ENOTRECOVERABLE;
	}

	npu_drv_debug("load npu start");
	ktime_get_real_ts64(&start_tv);

	ret = load_npu_ts_work_ctx_init(work_ctx);
	if (ret != 0) {
		npu_drv_err("[load_npu]error, init test work %d!\n", ret);
		return ret;
	}

	queue_work(work_ctx->queue, &work_ctx->work);
	npu_drv_info("[load_npu]load_verify_npu_image, work for completion\n");
	wait_for_completion(&work_ctx->completion);

	ret = work_ctx->result;
	npu_drv_info("[load_npu]load_verify_npu_image work ret=%d\n", ret);

	load_npu_ts_work_ctx_deinit(work_ctx);

	atomic_set(&g_load_status, (ret == 0) ? NPU_TSFW_LOADED_SUCC : NPU_TSFW_LOADED_FAIL);
	ktime_get_real_ts64(&end_tv);

	npu_drv_debug("load npu end");

	npu_drv_warn("[load_npu]load_npu ret=%d, cost time: cost=[%lld]us\n", ret,
		(end_tv.tv_sec - start_tv.tv_sec) * NPU_LOAD_S_TO_US_MULT +
		(end_tv.tv_nsec - start_tv.tv_nsec) / NPU_LOAD_NS_TO_US_DIV);

	return ret;
}

static int __init load_npu_init(void)
{
	struct npu_load_ts_work_ctx *ctx = load_npu_get_ctx();

	/* according to Huawei programming specification, 
		no need to check "memset_s" return value in this scenario */
	(void)memset_s(ctx, sizeof(*ctx), 0, sizeof(*ctx));

	return 0;
}

static void __exit load_npu_exit(void)
{
}

static int load_npu_callback(const char *val, const struct kernel_param *kp)
{
	(void)val;
	(void)kp;

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return 0;

	return load_npu_verify_image();
}

module_param_call(npu, load_npu_callback, NULL, NULL, 0660);
module_init(load_npu_init);
module_exit(load_npu_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("load_npu");
MODULE_VERSION("V1.0");

