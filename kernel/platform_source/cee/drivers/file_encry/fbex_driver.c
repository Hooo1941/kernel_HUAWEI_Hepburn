/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: fbex ca communicate with TA
 * Create : 2020/01/02
 */

#include "fbex_driver.h"
#ifndef CONFIG_FBE_MSPC_ABSENT
#ifdef CONFIG_FBE3_1
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <platform_include/basicplatform/linux/partition/partition_ap_kernel.h>
#include <vendor_rpmb.h>
#include <platform_include/basicplatform/linux/ufs/hufs_hcd.h>
#else
#include <crypto_core.h>
#endif
#endif
#include <teek_client_api.h>
#include <teek_client_constants.h>
#include <linux/types.h>
#include <linux/random.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>

#define FBEX_UID       2012
#define METADATA       5
#define FBEX_BUSY      0x5A
#define FBEX_MSP_DONE  0xA5
#define FBEX_USER_NUM  4

/* uuid to TA: 54ff868f-0d8d-4495-9d95-8e24b2a08274 */
#define UUID_TEEOS_UFS_INLINE_CRYPTO                                    \
	{                                                               \
		0x54ff868f, 0x0d8d, 0x4495,                             \
		{                                                       \
			0x9d, 0x95, 0x8e, 0x24, 0xb2, 0xa0, 0x82, 0x74  \
		}                                                       \
	}

struct sece_ops {
	u32 ret;
	u32 cmd;
	u32 index;
	u8 *pubkey;
	u32 key_len;
	u8 *metadata;
	u32 iv_len;
};

struct user_info {
	u32 user_id;
	u8 status;
};

static DEFINE_MUTEX(g_fbex_sece_mutex);
static DEFINE_MUTEX(g_fbex_ta_mutex);
static struct completion g_fbex_sece_comp;
static struct sece_ops g_sece_ops;
static struct work_struct g_fbex_sece_work;
static struct workqueue_struct *g_fbex_sece_wq;
#ifndef CONFIG_FBE_MSPC_ABSENT
/* for pre loading ++ */
static struct user_info g_user_info[FBEX_USER_NUM];
static struct delayed_work g_fbex_preload_work;
static struct workqueue_struct *g_fbex_preload_wq;
static u32 g_msp_timer_cnt;
/* for pre loading -- */
#endif

/* for mspc status pre check ++ */
static bool g_msp_status = false;
static u32 g_msp_status_check; /* changed in two work, using g_fbex_msp_mutex */
#ifndef CONFIG_FBE_MSPC_ABSENT
static struct delayed_work g_fbex_mspc_work;
static struct workqueue_struct *g_fbex_mspc_wq;
/* for mspc status pre check -- */
#endif

static bool g_session_create = true;
static TEEC_Context g_context;
static TEEC_Session g_session;
static u32 g_origin;

#if !defined(CONFIG_FBE_MSPC_ABSENT) && defined(CONFIG_FBE3_1)
#define MSPC_WAIT_RPMP_TIMES          20
#define MSPC_WAIT_RPMB_DELAY          500 /* 500ms */
#define MSPC_WAIT_PARTITION_READY_CNT 6
#define MSPC_WAIT_PARTITION_DELAY     500 /* 500ms */
#define MSPC_FS_PARTITION_NAME       "hisee_fs"
#define MSPC_PTN_PATH_BUF_SIZE 128

#define MSPC_MODULE_UNREADY 0xC8723B6D
#define MSPC_MODULE_READY   0x378DC492

#define polling_with_timeout(condition, cycles, step) \
do { \
	do { \
		if (condition) \
			break; \
		msleep((uint32_t)step); \
		cycles--; \
	} while (cycles > 0); \
} while (0)

static int32_t g_mspc_status = MSPC_MODULE_UNREADY;

static int32_t mspc_wait_rpmb_ready(void)
{
	uint32_t cycles = MSPC_WAIT_RPMP_TIMES;

	polling_with_timeout((get_rpmb_init_status() == RPMB_DRIVER_IS_READY), cycles, MSPC_WAIT_RPMB_DELAY);
	return (cycles == 0) ? -1 : 0;
}

static int32_t mspc_wait_partition_ready(void)
{
	int8_t fullpath[MSPC_PTN_PATH_BUF_SIZE] = {0};
	uint32_t cycles;
	int32_t ret;

	ret = flash_find_ptn_s(MSPC_FS_PARTITION_NAME, fullpath, sizeof(fullpath));
	if (ret != 0) {
		pr_err("%s:find failed:%d\n", __func__, ret);
		return -1;
	}

	cycles = MSPC_WAIT_PARTITION_READY_CNT;
	polling_with_timeout((ksys_access(fullpath, 0) == 0), cycles, MSPC_WAIT_PARTITION_DELAY);
	return (cycles == 0) ? -1 : 0;
}

static int32_t mspc_get_init_status(void)
{
	pr_err("%s, g_mspc_status=0x%x\n", __func__, g_mspc_status);
	if (g_mspc_status == MSPC_MODULE_READY)
		return g_mspc_status;

	if (mspc_wait_rpmb_ready() != 0) {
		pr_err("%s:wait rpmb ready timeout!\n", __func__);
		return MSPC_MODULE_UNREADY;
	}

	if (mspc_wait_partition_ready() != 0) {
		pr_err("%s:wait partition ready failed!\n", __func__);
		return MSPC_MODULE_UNREADY;
	}

	g_mspc_status = MSPC_MODULE_READY;
	return MSPC_MODULE_READY;
}
#endif

static u32 fbe_ca_invoke_command(u32 cmd, TEEC_Operation *op)
{
	TEEC_Result result;
	TEEC_UUID uuid_id = UUID_TEEOS_UFS_INLINE_CRYPTO;
	TEEC_Operation operation = { 0 };
	u32 root_id = FBEX_UID;
	const char *package_name = "ufs_key_restore";

	mutex_lock(&g_fbex_ta_mutex);
	/*
	 * during the ufs reset, ca access the secos and triggers rpmb access,
	 * which may cause TEEK_OpenSession deadlock, to solve this problem, the
	 * session do not release in normal cases.
	 */
	if (g_session_create) {
		result = TEEK_InitializeContext(NULL, &g_context);
		if (result != TEEC_SUCCESS) {
			pr_err("%s, TEEC init failed.\n", __func__);
			goto exit;
		}
		/* pass TA's FULLPATH to TEE, then OpenSession */
		/* MUST use TEEC_LOGIN_IDENTIFY method */
		operation.started = 1;
		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_MEMREF_TEMP_INPUT);
		operation.params[2].tmpref.buffer = (void *)(&root_id);
		operation.params[2].tmpref.size = sizeof(root_id);
		operation.params[3].tmpref.buffer = (void *)package_name;
		operation.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);

		result = TEEK_OpenSession(&g_context, &g_session, &uuid_id,
					TEEC_LOGIN_IDENTIFY, NULL, &operation, &g_origin);
		if (result != TEEC_SUCCESS) {
			pr_err("%s: open session failed, 0x%x\n", __func__, result);
			TEEK_FinalizeContext(&g_context);
			goto exit;
		}
		g_session_create = false;
	}
	result = TEEK_InvokeCommand(&g_session, cmd, op, &g_origin);
	if (result != TEEC_SUCCESS)
		pr_err("%s: invoke failed, ret 0x%x origin 0x%x\n", __func__,
			result, g_origin);

exit:
	mutex_unlock(&g_fbex_ta_mutex);
	return (u32)result;
}

void file_encry_record_error(u32 cmd, u32 user, u32 file, u32 error)
{
	int ret;

	ret = bbox_diaginfo_record(FBE_DIAG_FAIL_ID, NULL, "FBE: cmd 0x%x fail, "
				   "user 0x%x, file 0x%x, error ID: 0x%x\n",
				   cmd, user, file, error);
	pr_err("%s, record finish ret %d\n", __func__, ret);
}

#ifndef CONFIG_FBE_MSPC_ABSENT
/* we don't care the pre-load result */
static void hisi_fbex_preload_iv(u32 user)
{
	int ret;
	int i;

	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.params[0].value.a = user;

	operation.started = 1;
	ret = (int)fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_PRELOADING, &operation);
	if (ret != 0) {
		pr_err("%s, preloading 0x%x key fail, ret 0x%x\n", __func__, user, ret);
		for (i = 0; i < FBEX_USER_NUM; i++) {
			g_user_info[i].user_id = 0;
			g_user_info[i].status = 0;
		}
		hisi_fbex_set_preload(false);
		cancel_delayed_work(&g_fbex_preload_work);
	}
}

static void hisi_fbex_preload_fn(struct work_struct *work)
{
	u32 index;

	for (index = 0; index < FBEX_USER_NUM; index++) {
		if (g_user_info[index].status != FBEX_BUSY)
			continue;
		if (!hisi_fbex_is_preload()) {
			g_user_info[index].status = 0;
			continue;
		}
		hisi_fbex_preloading_msp(true);

		/* Do not include "preloading" into spin lock */
		pr_err("%s, preloading start\n", __func__);
		hisi_fbex_preload_iv(g_user_info[index].user_id);
		pr_err("%s, preloading done\n", __func__);

		hisi_fbex_preloading_msp(false);
	}
	if (hisi_fbex_is_preload() && g_msp_timer_cnt < MSP_TIMER_CNT) {
		pr_info("%s, queue work for mspc power\n", __func__);
		g_msp_timer_cnt += 1;
		queue_delayed_work(g_fbex_preload_wq, &g_fbex_preload_work,
				   msecs_to_jiffies(FBEX_DELAY_POWER));
	}
	(void)work;
}

static void fbex_try_preloading(u32 user, u32 file)
{
	int status;
	u32 index;

	file &= FBEX_FILE_MASK;
	/* if CE key is loading, we do not need to keep MSP power work */
	if (file == FILE_CE)
		hisi_fbex_set_preload(false);

	/* we only init the work when DE key loading */
	if (file != FILE_DE)
		return;

	g_msp_status_check = FBEX_MSP_DONE;

	status = mspc_get_init_status();
	if (status != MSPC_MODULE_READY || !g_msp_status) {
		pr_err("%s, msp is not ready 0x%x\n", __func__, status);
		return;
	}
	/* MSP is available, init the pre loader count */
	hisi_fbex_set_preload(true);
	g_msp_timer_cnt = 0;
	/* we are supported 4 users in queue */
	for (index = 0; index < FBEX_USER_NUM; index++) {
		if (g_user_info[index].status == FBEX_BUSY)
			continue;
		break;
	}
	if (index >= FBEX_USER_NUM) {
		pr_err("%s, too busy to add user 0x%x\n", __func__, user);
		return;
	}
	g_user_info[index].user_id = user;
	g_user_info[index].status = FBEX_BUSY;
	queue_delayed_work(g_fbex_preload_wq, &g_fbex_preload_work,
			   msecs_to_jiffies(FBEX_DELAY_MSP));
}
#endif

u32 file_encry_undefined(u32 user _unused, u32 file _unused, u8 *iv _unused,
			 u32 iv_len _unused)
{
	pr_err("%s, into\n", __func__);
	return FBE3_ERROR_CMD_UNDEFINED;
}

u32 file_encry_unsupported(u32 user _unused, u32 file _unused, u8 *iv _unused,
			   u32 iv_len _unused)
{
	pr_err("%s, into\n", __func__);
	return FBE3_ERROR_CMD_UNSUPPORT;
}

#ifdef CONFIG_FBE3_1
u32 fbe_ca_msp_available(u32 *flag)
{
	u32 ret;
	TEEC_Operation operation = { 0 };

	if (!flag) {
		pr_err("%s, input params null error\n", __func__);
		return FBE3_ERROR_NULL_POINTER;
	}
	/* Check the MSP status from TA */
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_MSPC_CHECK, &operation);
	if (ret != 0) {
		pr_err("%s, msp status check fail 0x%x\n", __func__, ret);
		return FBE3_ERROR_MSPC_CHK_FROM_TA;
	}
	if (operation.params[0].value.a == MSP_ONLINE) {
		pr_err("%s, mspc is available\n", __func__);
		*flag = FBE_CA_MSP_AVAILABLE;
	} else {
		pr_err("%s, mspc is unavailable\n", __func__);
		*flag = FBE_CA_MSP_UNAVAILABLE;
	}
	return (u32)0;
}
#endif

/*
 * Function: file_encry_add_iv
 * Parameters:
 *    user: input, user id
 *    file: input, file type(DE/CE/ECE/SECE)
 *    iv: input/output, iv buffer
 *    iv_len: input, iv length
 * Description:
 *    This is called when user create. iv can be create one at a time.
 *    Function should be called 4 times for one user(DE/CE/ECE/SECE)
 */
u32 file_encry_add_iv(u32 user, u32 file, u8 *iv, u32 iv_len)
{
	u32 ret;
	u32 flag;
	u8 slot;

	TEEC_Operation operation = { 0 };

#if !defined(CONFIG_FBE_MSPC_ABSENT) && defined(CONFIG_FBE3_1)
	ret = (u32)ufshcd_get_sk_state();
	if( ret != 0) {
		pr_err("%s, ufs set sk timeout, ret = 0x%x\n", __func__, ret);
		return ret;
	}
#endif
	if (!iv || iv_len != KEY_LEN) {
		pr_err("%s, input iv buffer is error 0x%x\n", __func__, iv_len);
		return FBE3_ERROR_IV_BUFFER;
	}
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
						TEEC_VALUE_INPUT,
						TEEC_NONE,
						TEEC_NONE);
	operation.params[0].tmpref.buffer = (void *)iv;
	operation.params[0].tmpref.size = iv_len;
	operation.params[1].value.a = user;
	operation.params[1].value.b = file;

	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_VOLD_ADD_IV,
				    &operation);
	if (ret == 0) {
		slot = iv[iv_len - 1];

#ifndef CONFIG_FBE_MSPC_ABSENT
#ifdef CONFIG_FBE3_1
		flag = FBE_CA_MSP_UNAVAILABLE;
		ret = fbe_ca_msp_available(&flag);
		if( ret!= 0) {
			pr_err("%s, msp available is error, ret = 0x%x\n", __func__, ret);
			return ret;
		}
		if (flag == FBE_CA_MSP_AVAILABLE)
			fbex_try_preloading(user, file);
#else
		fbex_try_preloading(user, file);
#endif
#endif
		fbex_init_slot(user, file, slot);
	}
	(void)flag;
	return ret;
}

/*
 * Function: file_encry_delete_iv
 * Parameters:
 *    user: input, user id
 *    file: input, file type(DE/CE/ECE/SECE)
 *    iv: input, iv buffer(used for verify)
 *    iv_len: input, iv length
 * Description:
 *    This is called when user delete. iv can be delete one at a time.
 *    Function should be called 4 times for one user(DE/CE/ECE/SECE)
 */
u32 file_encry_delete_iv(u32 user, u32 file, u8 *iv, u32 iv_len)
{
	u32 ret;
	TEEC_Operation operation = { 0 };

	if (!iv || iv_len != KEY_LEN) {
		pr_err("%s, input iv buffer is error 0x%x\n", __func__, iv_len);
		return FBE3_ERROR_IV_BUFFER;
	}
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
						TEEC_VALUE_INPUT,
						TEEC_NONE, TEEC_NONE);
	operation.params[0].tmpref.buffer = (void *)iv;
	operation.params[0].tmpref.size = iv_len;
	operation.params[1].value.a = user;
	operation.params[1].value.b = file;

	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_VOLD_DELETE_IV,
				    &operation);
	fbex_deinit_slot(user, file);
	return ret;
}

/*
 * Function: file_encry_logout_iv
 * Parameters:
 *    user: input, user id
 *    file: input, file type(DE/CE/ECE/SECE)
 *    iv: input/output, iv buffer
 *    iv_len: input, iv length
 * Description:
 *    This is called when user logout. iv can be logout one at a time.
 *    Function should be called 3 times for one user(CE/ECE/SECE)
 */
u32 file_encry_logout_iv(u32 user, u32 file, u8 *iv, u32 iv_len)
{
	u32 ret;
	TEEC_Operation operation = { 0 };

	if (!iv || iv_len != KEY_LEN) {
		pr_err("%s, input iv buffer is error 0x%x\n", __func__, iv_len);
		return FBE3_ERROR_IV_BUFFER;
	}
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
						TEEC_VALUE_INPUT,
						TEEC_NONE, TEEC_NONE);
	operation.params[0].tmpref.buffer = (void *)iv;
	operation.params[0].tmpref.size = iv_len;
	operation.params[1].value.a = user;
	operation.params[1].value.b = file;

	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_USER_LOGOUT,
				    &operation);
	fbex_deinit_slot(user, file);
	return ret;
}

u32 file_encry_status_report(u32 user _unused, u32 file _unused,
			     u8 *iv _unused, u32 iv_len _unused)
{
	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);

	operation.started = 1;
	return fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_STATUS_REPORT,
				     &operation);
}

/* lock/unlock screen request to TA interface */
u32 fbex_ca_lock_screen(u32 user, u32 file)
{
	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.params[0].value.a = user;
	operation.params[0].value.b = file;

	operation.started = 1;
	return fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_LOCK_SCREEN,
				     &operation);
}

u32 fbex_ca_unlock_screen(u32 user, u32 file, u8 *iv, u32 iv_len)
{
	TEEC_Operation operation = { 0 };

	if (!iv || iv_len != KEY_LEN) {
		pr_err("%s, input iv buffer is error 0x%x\n", __func__, iv_len);
		return FBE3_ERROR_IV_BUFFER;
	}
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
						TEEC_VALUE_INPUT,
						TEEC_NONE, TEEC_NONE);
	operation.params[0].tmpref.buffer = (void *)iv;
	operation.params[0].tmpref.size = iv_len;
	operation.params[1].value.a = user;
	operation.params[1].value.b = file;

	operation.started = 1;
	return fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_UNLOCK_SCREEN,
				     &operation);
}

/* Below functions are for kernel drivers */
u32 get_metadata(u8 *buf, u32 len)
{
	if (!buf || len != METADATA_LEN)
		return FBE3_ERROR_BUFFER_NULL;
	if ((u32)get_random_bytes_arch(buf, (int)len) != len) {
		pr_err("%s, invoke get_random_bytes_arch fail, need generate software random.\n", __func__);
		get_random_bytes(buf, (int)len);
	}
	return 0;
}

u32 fbex_restore_key(void)
{
	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	return fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_KEY_RESTORE,
				&operation);
}

u32 fbex_enable_kdf(void)
{
	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	return fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_ENABLE_KDF,
				&operation);
}

#ifdef CONFIG_FBE3_1
static u32 generate_session_key(uint32_t type)
{
	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	operation.params[0].value.a = type;
	return fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_GEN_SK,
				     &operation);
}
u32 fbex_gen_sk(uint32_t type)
{
	u32 ret = FBE3_ERROR_GEN_SK;

	pr_err("%s, in\n", __func__);
	if (mspc_wait_partition_ready() == 0) {
		ret = generate_session_key(type);
	}
	else {
		pr_err("%s, ret=%x, retry\n", __func__, ret);
		if (mspc_wait_partition_ready() == 0)
			ret = generate_session_key(type);
		else
			pr_err("%s, failed, ret=%x\n", __func__, ret);
	}
	return ret;
}
#endif

static void hisi_fbex_kca_fn(struct work_struct *work)
{
	TEEC_Operation operation = { 0 };
	u32 type = TEEC_MEMREF_TEMP_OUTPUT;

	if (g_sece_ops.cmd == SEC_FILE_ENCRY_CMD_ID_GEN_METADATA)
		type = TEEC_MEMREF_TEMP_INPUT;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, type,
						TEEC_MEMREF_TEMP_OUTPUT,
						TEEC_NONE);
	operation.params[0].value.a = g_sece_ops.index;
	operation.params[1].tmpref.buffer = (void *)g_sece_ops.pubkey;
	operation.params[1].tmpref.size = g_sece_ops.key_len;
	operation.params[2].tmpref.buffer = (void *)g_sece_ops.metadata;
	operation.params[2].tmpref.size = g_sece_ops.iv_len;
	operation.started = 1;
	g_sece_ops.ret = fbe_ca_invoke_command(g_sece_ops.cmd, &operation);
	(void)work;
	complete(&g_fbex_sece_comp);
}

/*
 * New SECE file:
 * cmd: SEC_FILE_ENCRY_CMD_ID_NEW_SECE
 *     index: slot id, file device privkey index
 *     pubkey: output, file pub key
 *     key_len: input, pubkey len
 *     metadata: output, new metadata
 *     iv_len: medata len
 * Open SECE file:
 * cmd: SEC_FILE_ENCRY_CMD_ID_GEN_METADATA
 *     index: slot id, file device privkey index
 *     pubkey: input, file pubkey
 *     key_len: pubkey len
 *     metadata: output, open metadata
 *     iv_len: metadata len
 * Note: Do not need to check the input parameters
 */
u32 get_metadata_sece(struct fbex_sece_param *param)
{
	if (!param) {
		pr_err("%s, param pointer is null\n", __func__);
		return FBE3_ERROR_NULL_POINTER;
	}
	if (param->idx >= FBEX_MAX_UFS_SLOT) {
		pr_err("%s, invalid index 0x5%x\n", __func__, param->idx);
		return FBE3_ERROR_SLOT_ID;
	}
	if (!param->pubkey || param->key_len != PUBKEY_LEN ||
		!param->metadata || param->iv_len != METADATA_LEN) {
		pr_err("%s, input buffer is invalid\n", __func__);
		return FBE3_ERROR_BUFFER_INVALID;
	}
	mutex_lock(&g_fbex_sece_mutex);
	g_sece_ops.ret = 0;
	g_sece_ops.cmd = param->cmd;
	g_sece_ops.index = param->idx;
	g_sece_ops.pubkey = param->pubkey;
	g_sece_ops.key_len = param->key_len;
	g_sece_ops.metadata = param->metadata;
	g_sece_ops.iv_len = param->iv_len;

	queue_work(g_fbex_sece_wq, &g_fbex_sece_work);
	wait_for_completion(&g_fbex_sece_comp);
	if (g_sece_ops.ret)
		file_encry_record_error(0, param->idx, METADATA, g_sece_ops.ret);
	mutex_unlock(&g_fbex_sece_mutex);
	return g_sece_ops.ret;
}

#ifndef CONFIG_FBE_MSPC_ABSENT
static void hisi_fbex_msp_status(struct work_struct *work)
{
	u32 ret;
	u32 status;
	TEEC_Operation operation = { 0 };

	/* we need to stop the check work if g_msp_status_check == done */
	if (g_msp_status_check == FBEX_MSP_DONE)
		return;

	status = (u32)mspc_get_init_status();
	/* if MSPC driver is not ready, queue the work again */
	if (status != MSPC_MODULE_READY) {
		pr_err("%s, msp init status 0x%x\n", __func__, status);
		queue_delayed_work(g_fbex_mspc_wq, &g_fbex_mspc_work,
				   msecs_to_jiffies(FBEX_DELAY_MSP));
		return;
	}
	/* MSPC driver is ready, check the MSP status from TA */
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_MSPC_CHECK, &operation);
	if (ret != 0) {
		pr_err("%s, msp status check fail 0x%x\n", __func__, ret);
		g_msp_status = false;
		goto finish;
	}
	g_msp_status = (operation.params[0].value.a == MSP_ONLINE) ? true : false;
finish:
	pr_info("%s, fbex msp status %s\n", __func__,
		g_msp_status ? "online" : "offline");
	/* set g_msp_status_check = done, to end the work queue */
	g_msp_status_check = FBEX_MSP_DONE;
	(void)work;
}
#endif

#ifdef CONFIG_FBE_S4
u32 fbex_add_key_s4(u32 *slot)
{
	u32 ret;
	TEEC_Operation operation = { 0 };

	if (!slot) {
		pr_err("%s, slot is null\n", __func__);
		return FBE3_ERROR_NULL_POINTER;
	}

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
											TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_ADD_S4, &operation);
	if (ret != 0) {
		pr_err("%s, add key failed, ret: 0x%x\n", __func__, ret);
		*slot = FBEX_MAX_UFS_SLOT;
		return ret;
	}

	*slot = operation.params[0].value.a;
	return ret;
}

u32 fbex_delete_key_s4(void)
{
	u32 ret;
	TEEC_Operation operation = { 0 };

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
											TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_DELETE_S4, &operation);
	if (ret != 0)
		pr_err("%s, del key failed, ret: 0x%x\n", __func__, ret);
	return ret;
}

u32 fbex_restore_key_s4(u32 *slot)
{
	u32 ret;
	TEEC_Operation operation = { 0 };

	if (!slot) {
		pr_err("%s, slot is null\n", __func__);
		return FBE3_ERROR_NULL_POINTER;
	}

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
											TEEC_NONE, TEEC_NONE);
	operation.started = 1;
	ret = fbe_ca_invoke_command(SEC_FILE_ENCRY_CMD_ID_RESTORE_S4, &operation);
	if (ret != 0) {
		pr_err("%s, restore key failed, ret: 0x%x\n", __func__, ret);
		*slot = FBEX_MAX_UFS_SLOT;
		return ret;
	}

	*slot = operation.params[0].value.a;
	return ret;
}
#endif

bool fbex_is_msp_online(void)
{
	if (g_msp_status_check != FBEX_MSP_DONE) {
		pr_err("%s, msp status is not ready\n", __func__);
		return false;
	}
	return g_msp_status;
}

int fbex_init_driver(void)
{
	u32 ret;
	u32 flag;
	init_completion(&g_fbex_sece_comp);
	g_fbex_sece_wq = create_singlethread_workqueue("fbex_sece");
	if (!g_fbex_sece_wq) {
		pr_err("Create fbex sece failed\n");
		return -1;
	}
	INIT_WORK(&g_fbex_sece_work, hisi_fbex_kca_fn);
#ifndef CONFIG_FBE_MSPC_ABSENT
#ifdef CONFIG_FBE3_1
	flag = FBE_CA_MSP_UNAVAILABLE;
	ret = fbe_ca_msp_available(&flag);
	if(ret != 0) {
		pr_err("%s, msp available is error, ret = 0x%x\n", __func__, ret);
		return (int)ret;
	}
	if (flag != FBE_CA_MSP_AVAILABLE) {
		g_msp_status = false;
		g_msp_status_check = FBEX_MSP_DONE;
		return 0;
	}
#endif
	g_fbex_preload_wq = create_singlethread_workqueue("fbex_preload");
	if (!g_fbex_preload_wq) {
		pr_err("Create fbex preload failed\n");
		return -1;
	}
	INIT_DELAYED_WORK(&g_fbex_preload_work, hisi_fbex_preload_fn);
	g_fbex_mspc_wq = create_singlethread_workqueue("fbex_mspc_status");
	if (!g_fbex_mspc_wq) {
		pr_err("Create fbex mspc status failed\n");
		return -1;
	}
	INIT_DELAYED_WORK(&g_fbex_mspc_work, hisi_fbex_msp_status);
	queue_delayed_work(g_fbex_mspc_wq, &g_fbex_mspc_work,
			msecs_to_jiffies(FBEX_DELAY_MSP));

#else
	g_msp_status = false;
	g_msp_status_check = FBEX_MSP_DONE;
#endif
	(void)flag;
	return 0;
}
