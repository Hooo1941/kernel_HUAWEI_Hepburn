/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: image load driver
 * Create: 2022/11/28
 */

#include "dsp_load_ca.h"

#include <linux/version.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <platform_include/see/sec_auth_ca.h>
#include <platform_include/basicplatform/linux/partition/partition_macro.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <platform_include/basicplatform/linux/partition/partition_ap_kernel.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <linux/version.h>
#include <securec.h>
#include "audio_log.h"
#include "dsp_om.h"

#define DEVICE_PATH  "/dev/block/bootdevice/by-name/"

#define load_dsp_min(a, b)                       ((a) < (b) ? (a) : (b))

#define SEC_AUTH_NAME                            "sec_auth"

static struct image_id_t dsp_image_id = {
	.partition_name = PART_FW_HIFI,
	.image_name = "hifi",
};

static int dsp_read_bin(unsigned int offset, unsigned int length, char *buffer)
{
	int ret = LOAD_DSP_ERROR;
	char *pathname = NULL;
	u64 pathlen;
	struct file *fp = NULL;
	loff_t read_offset = offset;

	if (!buffer) {
		loge("buffer is null\n");
		return ret;
	}

	/* get resource, add 1 for last '\0' */
	pathlen = strlen(DEVICE_PATH) + 1 + strnlen(dsp_image_id.partition_name, (u64)PARTITION_NAME_LEN_MAX);
	pathname = kzalloc(pathlen, GFP_KERNEL);
	if (!pathname) {
		loge("pathname kzalloc failed\n");
		return ret;
	}

	ret = flash_find_ptn_s((const char *)dsp_image_id.partition_name, pathname, pathlen);
	if (ret != 0) {
		loge("partion_name(%s) not in ptable, ret=0x%x\n",
			      dsp_image_id.partition_name, ret);
		goto free_pname;
	}
	/* Only read file */
	fp = filp_open(pathname, O_RDONLY, 0400);
	if (IS_ERR(fp)) {
		logw("filp_open(%s) failed, errorno:%ld\n", pathname, PTR_ERR(fp));
		ret = LOAD_DSP_ERROR;
		goto free_pname;
	}

	ret = kernel_read(fp, buffer, length, &read_offset);
	if (ret != (int)length) {
		loge("read ops fail ret=0x%x len=0x%x\n", ret, length);
		goto close_file;
	}
	ret = LOAD_DSP_OK;

close_file:
	filp_close(fp, NULL);

free_pname:
	kfree(pathname);

	return ret;
}

int dsp_read_image(void)
{
	unsigned int value;

	return dsp_read_bin(0, sizeof(value), (char *)&value);
}

/*
 * Function name: teek_init.
 * Discription:Init the TEEC and get the context
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ context: context.
 * Return value:
 *      @ TEEC_SUCCESS-->success, others-->failed.
 * Note: this function is actually a static function so the params are not
 *       checked. The reason for not adding 'static' is only used for
 *       mdpp certification and can guarantee the params isn't null.
 */
static int load_dsp_teek_init(TEEC_Session *session, TEEC_Context *context)
{
	TEEC_Result result;
	TEEC_UUID svc_uuid = TEE_SERVICE_SECAUTH;
	TEEC_Operation operation = { 0 };
	char package_name[] = SEC_AUTH_NAME;
	unsigned int root_id = 0;

	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		loge("TEEK_InitializeContext fail res=0x%x\n", result);
		return (int)result;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	/* 2 is TEEC_RegisteredMemoryReference type */
	operation.params[2].tmpref.buffer = (void *)(&root_id);
	operation.params[2].tmpref.size = sizeof(root_id);
	/* 3 is TEEC_Value type */
	operation.params[3].tmpref.buffer = (void *)(package_name);
	operation.params[3].tmpref.size = strlen(package_name) + 1;
	result = TEEK_OpenSession(context, session, &svc_uuid,
				  TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		loge("TEEK_OpenSession failed result=0x%x!\n", result);
		TEEK_FinalizeContext(context);
		return (int)result;
	}

	return LOAD_DSP_OK;
}

static int load_dsp_node_init(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	unsigned int origin = 0;

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_INIT, operation, &origin);
	if (result != TEEC_SUCCESS) {
		loge("init ret=0x%x\n", result);
		return (int)result;
	}

	return LOAD_DSP_OK;
}

static int load_dsp_node_read_cert(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	int ret = LOAD_DSP_ERROR;
	unsigned int origin = 0;
	uintptr_t cert_addr = 0;

	cert_addr = (uintptr_t)vmalloc(SEC_AUTH_CERT_SIZE);
	if (cert_addr == 0) {
		loge("cert addr malloc failed\n");
		return ret;
	}
	ret = dsp_read_bin(0, SEC_AUTH_CERT_SIZE, (char *)cert_addr);
	if (ret != LOAD_DSP_OK) {
		loge("prepare dsp cert ret=%d\n", ret);
		goto err;
	}

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
	operation->params[1].tmpref.buffer = (void *)cert_addr;
	operation->params[1].tmpref.size = SEC_AUTH_CERT_SIZE;
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_READ_CERT, operation, &origin);
	if (result != TEEC_SUCCESS) {
		loge("read_cert ret=0x%x\n", result);
		ret = (int)result;
		goto err;
	}

err:
	if (cert_addr != 0) {
		memset((void*)cert_addr, 0, SEC_AUTH_CERT_SIZE);
		vfree((void*)cert_addr);
	}

	return ret;
}

static int load_dsp_node_read_data(TEEC_Session *session, TEEC_Operation *operation)
{
	int ret = LOAD_DSP_ERROR;
	TEEC_Result result;
	unsigned int origin = 0;
	unsigned int send_size, cur_size;
	uintptr_t data_addr = 0;

	data_addr = (uintptr_t)vmalloc(DSP_CA_SEND_MAX_SIZE);
	if (data_addr == 0) {
		loge("data addr malloc failed\n");
		return ret;
	}

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
	send_size = 0;
	while(send_size < DSP_IMG_MAX_SIZE) {
		cur_size = load_dsp_min((DSP_IMG_MAX_SIZE - send_size), DSP_CA_SEND_MAX_SIZE);
		ret = dsp_read_bin(SEC_AUTH_CERT_SIZE + send_size, cur_size, (char *)data_addr);
		if (ret != LOAD_DSP_OK) {
			loge("prepare dsp data ret=%d\n", ret);
			goto err;
		}
		operation->params[1].tmpref.buffer = (void *)data_addr;
		operation->params[1].tmpref.size = cur_size;
		operation->params[2].value.a = send_size;
		operation->params[2].value.b = DSP_IMG_MAX_SIZE;
		result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_READ_DATA, operation, &origin);
		if (result != TEEC_SUCCESS) {
			loge("read_data send_size=%u cur_size=%u ret=0x%x\n", send_size, cur_size, result);
			ret = (int)result;
			goto err;
		}
		send_size += cur_size;
	}

err:
	if (data_addr != 0) {
		// cppcheck-suppress *
		memset((void*)data_addr, 0, DSP_CA_SEND_MAX_SIZE);
		vfree((void*)data_addr);
	}

	return ret;
}

static int load_dsp_node_verify(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	unsigned int origin = 0;
	struct authed_data out = { 0 };

	operation->params[1].tmpref.buffer = (void *)&out;
	operation->params[1].tmpref.size = sizeof(out);
	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEE_PARAM_TYPE_MEMREF_INOUT, TEEC_NONE, TEEC_NONE);
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_VERIFY, operation, &origin);
	if (result != TEEC_SUCCESS) {
		loge("verify ret=0x%x\n", result);
		return (int)result;
	}

	return LOAD_DSP_OK;
}

static int load_dsp_node_final(TEEC_Session *session, TEEC_Operation *operation)
{
	TEEC_Result result;
	unsigned int origin = 0;

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE,  TEEC_NONE);
	result = TEEK_InvokeCommand(session, SEC_AUTH_CMD_ID_FINAL, operation, &origin);
	if (result != TEEC_SUCCESS) {
		loge("final ret=0x%x\n", result);
		return (int)result;
	}

	return LOAD_DSP_OK;
}

int dsp_load_and_verify_image(void)
{
	int ret;
	TEEC_Session session;
	TEEC_Context context;
	TEEC_Operation operation = { 0 };

	logi("load_dsp_teek_init +\n");
	ret = load_dsp_teek_init(&session, &context);
	if (ret != LOAD_DSP_OK) {
		loge("sec_auth teek init failed, ret is 0x%x\n", ret);
		return ret;
	}
	logi("load_dsp_teek_init -\n");

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.params[0].tmpref.buffer = (void *)&dsp_image_id;
	operation.params[0].tmpref.size = sizeof(dsp_image_id);

	logi("load_dsp_node_init +\n");
	ret = load_dsp_node_init(&session, &operation);
	if (ret != LOAD_DSP_OK) {
		loge("load_dsp node init failed, ret is 0x%x\n", ret);
		goto err_out;
	}
	logi("load_dsp_node_init -\n");

	logi("load_dsp_node_read_cert +\n");
	ret = load_dsp_node_read_cert(&session, &operation);
	if (ret != LOAD_DSP_OK) {
		loge("load_dsp node read cert failed, ret is 0x%x\n", ret);
		goto err_out;
	}
	logi("load_dsp_node_read_cert -\n");

	logi("load_dsp_node_read_data +\n");
	ret = load_dsp_node_read_data(&session, &operation);
	if (ret != LOAD_DSP_OK) {
		loge("load_dsp node read data failed, ret is 0x%x\n", ret);
		goto err_out;
	}
	logi("load_dsp_node_read_data -\n");

	logi("load_dsp_node_verify +\n");
	ret = load_dsp_node_verify(&session, &operation);
	if (ret != LOAD_DSP_OK) {
		loge("load_dsp verify failed, ret is 0x%x\n", ret);
		goto err_out;
	}
	logi("load_dsp_node_verify -\n");

	logi("load_dsp_node_final +\n");
	ret = load_dsp_node_final(&session, &operation);
	if (ret != LOAD_DSP_OK) {
		loge("load_dsp final failed, ret is 0x%x\n", ret);
		goto err_out;
	}
	logi("load_dsp_node_final -\n");

err_out:
	TEEK_CloseSession(&session);
	TEEK_FinalizeContext(&context);

	return ret;
}
