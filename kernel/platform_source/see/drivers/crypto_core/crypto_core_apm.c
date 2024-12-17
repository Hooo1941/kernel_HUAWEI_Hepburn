/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: Key ca for apm.
 * Create: 2023/11/10
 */

#include "crypto_core_internal.h"
#include <vendor_rpmb.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include "crypto_core.h"

#include <teek_client_api.h>
#include <teek_client_constants.h>
#include <teek_client_type.h>
#include <linux/types.h>
#include <linux/random.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>

#ifdef CONFIG_CRYPTO_CORE_APM_KCA_NOTIFY
static TEEC_Context g_context;
static TEEC_Session g_session;
static bool g_apm_start_running = false;
static DECLARE_WAIT_QUEUE_HEAD(g_wait);

#define APM_UID       	    0
#define APM_TA_ONE_COMMAND  1
#define APM_TA_PATH		    "/vendor/bin/3ec8f65f-830c-4ad9-98dc-604e3b7156c3.sec"
#define UUID_TEEOS_UFS_INLINE_APM                                   \
	{                                                               \
		0x3ec8f65f, 0x830c, 0x4ad9,                             \
		{                                                       \
			0x98, 0xdc, 0x60, 0x4e, 0x3b, 0x71, 0x56, 0xc3  \
		}                                                       \
	}

static void apm_kca_end(void)
{
	pr_err("%s, APM kca ended.\n", __func__);
	TEEK_CloseSession(&g_session);
	TEEK_FinalizeContext(&g_context);
}

static void apm_kca_notify(void)
{
	const char *package_name = "mspc";
	uint32_t origin = 0;
	TEEC_Result result;
	u32 root_id = APM_UID;
	TEEC_UUID uuid_id = UUID_TEEOS_UFS_INLINE_APM;
	TEEC_Operation operation = { 0 };

	result = TEEK_InitializeContext(NULL, &g_context);
	if (result != TEEC_SUCCESS) {
		pr_err("%s, APM TE init failed.\n", __func__);
		goto exit;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
						TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	operation.params[2].tmpref.buffer = (void *)(&root_id);
	operation.params[2].tmpref.size = sizeof(root_id);
	operation.params[3].tmpref.buffer = (void *)package_name;
	operation.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);
	g_context.ta_path = (uint8_t *)APM_TA_PATH;

	result = TEEK_OpenSession(&g_context, &g_session, &uuid_id, TEEC_LOGIN_IDENTIFY, NULL, &operation, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: APM TE open session failed, 0x%x, origin: %u\n", __func__, result, origin);
		goto exit;
	} else {
		pr_err("%s, APM TE open session succeeded.\n", __func__);
	}

	result = TEEK_InvokeCommand(&g_session, APM_TA_ONE_COMMAND, &operation, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: APM TE invoke command failed, 0x%x, origin: %u\n", __func__, result, origin);
	} else {
		pr_err("%s, APM TE invoke command succeeded.\n", __func__);
	}

	apm_kca_end();
	return;

exit:
	TEEK_FinalizeContext(&g_context);
	return;
}

int hisee_apm_ta_thread(void *arg)
{
	while (1) {
		wait_event_interruptible(g_wait, g_apm_start_running);
		apm_kca_notify();
		g_apm_start_running = false;
	}

	return 0;
}

void __weak mspc_apm_kca_notify(void)
{
	pr_err("%s, APM notification received from mntn.\n", __func__);
	g_apm_start_running = true;
	wake_up_interruptible(&g_wait);
}
#endif