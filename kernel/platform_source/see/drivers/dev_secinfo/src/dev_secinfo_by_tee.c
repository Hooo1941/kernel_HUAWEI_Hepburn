/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: dev_secinfo do things in tee source
 * Create: 2023/3/31
 */
#include <dev_secinfo_by_tee.h>
#include <dev_secinfo_errno.h>
#include <dev_secinfo_plat.h>
#include <platform_include/see/dev_secinfo.h>
#include <platform_include/see/dev_secinfo_auth_ca.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <securec.h>

static u32 dev_secinfo_teek_init_ctx(TEEC_Context *ctx)
{
	TEEC_Result ret;

	ret = TEEK_InitializeContext(NULL, ctx);
	if (ret != TEEC_SUCCESS) {
		dev_secinfo_err("error,InitCtx ret=0x%x\n", ret);
		return DEV_SECINFO_ERR_TEEK_INIT_CTX;
	}

	return DEV_SECINFO_OK;
}

static u32 dev_secinfo_teek_open_session(TEEC_Context *ctx, TEEC_Session *session)
{
	TEEC_Result ret;
	TEEC_UUID uuid = TEE_SERVICE_DEV_SECINFO_AUTH;
	TEEC_Operation operation = {0};
	char package_name[] = DEV_SECINFO_AUTH_NAME;
	u32 root_id = 0;

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT,
						TEEC_MEMREF_TEMP_INPUT);
	/* 2 is TEEC_RegisteredMemoryReference type */
	operation.params[2].tmpref.buffer = (void *)(&root_id);
	operation.params[2].tmpref.size = (u32)sizeof(root_id);
	/* 3 is TEEC_Value type */
	operation.params[3].tmpref.buffer = (void *)(package_name);
	operation.params[3].tmpref.size = (u32)strlen(package_name) + 1;
	ret = TEEK_OpenSession(ctx, session, &uuid, TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
	if (ret != TEEC_SUCCESS) {
		dev_secinfo_err("error,OpenSession ret=0x%x\n", ret);
		return DEV_SECINFO_ERR_TEEK_OPEN_SESSION;
	}

	return DEV_SECINFO_OK;
}

static u32 dev_secinfo_teek_close_session(TEEC_Session *session)
{
	TEEK_CloseSession(session);

	return DEV_SECINFO_OK;
}

static u32 dev_secinfo_teek_final_ctx(TEEC_Context *ctx)
{
	TEEK_FinalizeContext(ctx);

	return DEV_SECINFO_OK;
}

u32 set_uapp_enable_state_by_tee(u32 state)
{
	u32 ret;
	TEEC_Session session;
	TEEC_Context ctx;
	TEEC_Operation operation = { 0 };
	u32 origin = 0;
	u32 cmd = DEV_SECINFO_AUTH_CMD_ID_SET_UAPP_ENABLE_STATE;

	ret = dev_secinfo_teek_init_ctx(&ctx);
	if (ret != DEV_SECINFO_OK) {
		dev_secinfo_err("error, init_ctx ret=0x%x\n", ret);
		return ret;
	}

	ret = dev_secinfo_teek_open_session(&ctx, &session);
	if (ret != DEV_SECINFO_OK) {
		dev_secinfo_err("error, open_session ret=0x%x\n", ret);
		goto final_ctx;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.params[0].value.a = state;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	ret = TEEK_InvokeCommand(&session, cmd, &operation, &origin);
	if (ret != 0) {
		dev_secinfo_err("error,invoke cmd 0x%x ret=0x%x\n", cmd, ret);
		goto close_session;
	}

close_session:
	(void)dev_secinfo_teek_close_session(&session);

final_ctx:
	(void)dev_secinfo_teek_final_ctx(&ctx);

	return ret;
}