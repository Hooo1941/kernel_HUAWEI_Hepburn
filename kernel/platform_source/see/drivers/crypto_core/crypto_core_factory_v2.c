/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Drivers for Crypto Core chip test.
 * Create: 2019/11/17
 */

#include "crypto_core_factory.h"
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <general_see_mntn.h>
#include <vendor_rpmb.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <securec.h>
#include "crypto_core_internal.h"
#include "crypto_core_errno.h"
#include "crypto_core_flash.h"
#include "crypto_core_fs.h"
#include "crypto_core_power.h"
#include "crypto_core_smc.h"
#include "crypto_core_upgrade.h"

#define MSPC_WAIT_COS_READY_TIME         10000 /* 10s */
#define MSPC_WAIT_COS_DOWN_TIME          10000 /* 10s */

#define MSPC_POLL_WAIT_TIME               100    /* 100ms */
#define MSPC_ENTER_FACTORY_MODE_WAIT_TIME 20000  /* 20s */
#define MSPC_SMC_ACK_WAIT_TIME            10000  /* 10s */

static struct mutex g_mspc_otp_mutex;

/* flag to indicate running status of flash otp1 */
enum otp1_status {
	NO_NEED = 0,
	PREPARED,
	RUNING,
	FINISH,
};

/* flag to indicate running status of flash otp1 */
static uint32_t g_mspc_flash_otp1_status;

/* check otp1 write work is running */
/* flash_otp_task may not being created by set/get efuse _securitydebug_value */
bool mspc_chiptest_otp1_is_running(void)
{
	pr_info("mspc otp1 work status %x\n", g_mspc_flash_otp1_status);
	if (g_mspc_flash_otp1_status == RUNING)
		return true;

	if (g_mspc_flash_otp1_status == PREPARED &&
	    mspc_flash_is_task_started())
		return true;

	return false;
}

void mspc_chiptest_set_otp1_status(enum otp1_status status)
{
	g_mspc_flash_otp1_status = status;
	pr_err("mspc set otp1 state:%x\n", g_mspc_flash_otp1_status);
}

static int32_t mspc_wait_smc(uint32_t cmd_type)
{
	int32_t ret;
	int32_t wait_time = MSPC_SMC_ACK_WAIT_TIME;

	ret = mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
				   (uint64_t)cmd_type, NULL, 0);
	if (ret != MSPC_OK) {
		pr_err("%s send smc %x failed\n", __func__, cmd_type);
		return ret;
	}

	while (wait_time > 0) {
		ret = mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
					   (uint64_t)MSPC_SMC_GET_ACK,
					   NULL, 0);
		if (ret != 0)
			return ret;
		msleep(MSPC_POLL_WAIT_TIME);
		wait_time -= MSPC_POLL_WAIT_TIME;
	}
	pr_err("%s err %x\n", __func__, ret);

	return MSPC_SMC_CMD_TIMEOUT_ERROR;
}

static int32_t mspc_manufacture_check_lcs(void)
{
	return mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
				    (uint64_t)MSPC_SMC_GET_LCS_SM, NULL, 0);
}

int32_t mspc_enter_factory_mode(void)
{
	int32_t wait_time = MSPC_ENTER_FACTORY_MODE_WAIT_TIME;
	int32_t vote_atf, vote_tee;
	int32_t ret;
	uint32_t msg[MSPC_SMC_PARAM_SIZE];

	ret = mspc_force_power_off();
	if (ret != MSPC_OK) {
		pr_err("%s power off mspc failed\n", __func__);
		return ret;
	}

	msg[MSPC_SMC_PARAM_0] = 0;
	msg[MSPC_SMC_PARAM_1] = (uint32_t)MSPC_NATIVE_COS_ID;
	while (wait_time > 0) {
		ret = mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
					   (uint64_t)MSPC_SMC_ENTER_FACTORY_MODE,
					   (const char *)msg, sizeof(msg));
		if (ret == MSPC_OK)
			return ret;
		msleep(MSPC_POLL_WAIT_TIME);
		wait_time -= MSPC_POLL_WAIT_TIME;
	}
	pr_err("%s err %d\n", __func__, ret);

	vote_atf = mspc_send_smc_no_ack((uint64_t)HISEE_MNTN_ID,
					(uint64_t)HISEE_SMC_GET_ATF_VOTE, NULL, 0);
	vote_tee = mspc_send_smc_no_ack((uint64_t)HISEE_MNTN_ID,
					(uint64_t)HISEE_SMC_GET_TEE_VOTE, NULL, 0);
	pr_err("vote vote atf[0x%x] vote TEE[0x%x]\n", vote_atf, vote_tee);

	return MSPC_ENTER_FACTROY_MODE_ERROR;
}

int32_t mspc_exit_factory_mode(void)
{
	int32_t ret;
	uint32_t msg[MSPC_SMC_PARAM_SIZE] = {0};

	msg[MSPC_SMC_PARAM_0] = 0;
	msg[MSPC_SMC_PARAM_1] = (uint32_t)MSPC_NATIVE_COS_ID;
	ret = mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
				   (uint64_t)MSPC_SMC_EXIT_FACTORY_MODE,
				   (const char *)msg, sizeof(msg));
	if (ret != MSPC_OK) {
		pr_err("%s err %d\n", __func__, ret);
		return MSPC_EXIT_FACTROY_MODE_ERROR;
	}

	return MSPC_OK;
}

/* do some check before pinstall test */
static int32_t mspc_pinstall_pre_check(void)
{
	int32_t ret;

	ret = mspc_manufacture_check_lcs();
	if (ret != MSPC_OK)
		return ret;

	/* check rpmb key, should be ready */
	if (get_rpmb_key_status() != KEY_READY) {
		pr_err("%s rpmb key not ready\n", __func__);
		return MSPC_RPMB_KEY_UNREADY_ERROR;
	}

	ret = mspc_enter_factory_mode();
	if (ret != MSPC_OK) {
		pr_err("%s enter factory mode fail\n", __func__);
		return ret;
	}

	/* switch storage media to rpmb */
	ret = mspc_wait_smc(MSPC_SMC_SWITCH_RPMB);
	if (ret != MSPC_OK) {
		pr_err("%s switch to rpmb err %x\n", __func__, ret);
		return ret;
	}

	return MSPC_OK;
}

static int32_t mspc_set_urs(void)
{
	int32_t ret;

	ret = mspc_wait_smc(MSPC_SMC_SET_URS);
	if (ret != MSPC_OK) {
		pr_err("%s err %x\n", __func__, ret);
		return ret;
	}

	pr_err("%s successful\n", __func__);

	return ret;
}

/* power on cos, run fn, then power off */
static int32_t mspc_run_cos(int32_t (*fn)(void))
{
	int32_t ret, ret1;
	uint32_t time = (uint32_t)MSPC_WAIT_COS_READY_TIME;
	const int8_t *param = MSPC_NATIVE_COS_POWER_PARAM;

	/* Power on COS, then process fn */
	ret = mspc_power_func(param, strlen(param),
			      MSPC_POWER_ON_BOOTING, MSPC_POWER_CMD_ON);
	if (ret != MSPC_OK) {
		pr_err("%s:power on cos failed! ret=%d\n", __func__, ret);
		return ret;
	}

	ret = mspc_wait_state(MSPC_STATE_NATIVE_READY, time);
	if (ret != MSPC_OK)
		pr_err("%s:wait cos ready timeout!\n", __func__);
	else
		ret = (fn) ? fn() : MSPC_OK;

	/* power off COS */
	ret1 = mspc_power_func(param, strlen(param),
			       MSPC_POWER_OFF, MSPC_POWER_CMD_OFF);
	if (ret1 != MSPC_OK)
		pr_err("%s:power off cos failed!\n", __func__);

	return (ret == MSPC_OK) ? ret1 : ret;
}

/* function to process when securedebug disabled */
static int32_t mspc_flash_debug_fn(void)
{
	int32_t ret;
	bool is_debug_disable = false;

	ret = efuse_check_secdebug_disable(&is_debug_disable);
	if (ret != 0) {
		pr_err("%s:check secdebug failed\n", __func__);
		return MSPC_CHECK_SECDEBUG_ERROR;
	}

	/* If secdebug is DebugDisable, write otp1 */
	if (is_debug_disable) {
		ret = mspc_set_urs();
		if (ret != MSPC_OK) {
			pr_err("%s:write otp1 image failed\n", __func__);
			return ret;
		}
	}

	return MSPC_OK;
}

static bool mspc_is_product_disable(void)
{
	int32_t ret;

	ret = mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
				   (uint64_t)MSPC_SMC_GET_PRODUCT_DISABLE, NULL, 0);
	pr_err("%s:ret %x\n", __func__, ret);
	if (ret == MSPC_OK)
		return true;
	return false;
}

#define MSPC_RETRY_WAIT_TIME    1000  /* 1s */
#define MSPC_MAX_RETRY_CNT      3

static int32_t mspc_normal_manufacture_func(void)
{
	int32_t ret;
	int32_t try_cnt = 0;

	mspc_reinit_flash_complete();
	ret = mspc_pinstall_pre_check();
	if (ret != MSPC_OK) {
		pr_err("%s:pre_check failed\n", __func__);
		goto exit;
	}

	do {
		try_cnt++;
		ret = mspc_run_cos(mspc_flash_debug_fn);
		if (ret != MSPC_OK) {
			pr_err("%s:run flash debug failed 0x%x\n", __func__, ret);
			msleep(MSPC_RETRY_WAIT_TIME);
			continue;
		}
		break;
	} while (try_cnt < MSPC_MAX_RETRY_CNT);

	if (ret != MSPC_OK)
		panic("mspc pinstall fail");

exit:
	if (ret == MSPC_OK) {
		mspc_chiptest_set_otp1_status(PREPARED);
		/* sync signal for flash_otp_task */
		mspc_release_flash_complete();
	}

	return ret;
}

/* function run when AT^MSPC=INSTALL,2 */
int32_t mspc_total_manufacture_func(void)
{
	int32_t ret;

	if (mspc_is_product_disable())
		ret = MSPC_OK;
	else
		ret = mspc_normal_manufacture_func();

	mspc_record_errno(ret);
	return ret;
}

/* function run when send AT^SECUREDEBUG=, to rm debug switch in mspc */
int32_t mspc_flash_debug_switchs(void)
{
	int32_t ret;

	mutex_lock(&g_mspc_otp_mutex);
	mspc_chiptest_set_otp1_status(RUNING);

	ret = mspc_run_cos(mspc_flash_debug_fn);
	if (ret != MSPC_OK)
		pr_err("%s:run flash debug ret %x\n", __func__, ret);

	mspc_chiptest_set_otp1_status(FINISH);
	mutex_unlock(&g_mspc_otp_mutex);

	mspc_record_errno(ret);
	return ret;
}

static void mspc_set_recovery_flag(void)
{
	int32_t ret;

	if (get_boot_into_recovery_flag() != 0) {
		ret = mspc_send_smc_no_ack((uint64_t)MSPC_FN_MAIN_SERVICE_CMD,
					   (uint64_t)MSPC_SMC_SET_RECOVERY, NULL, 0);
		if (ret != MSPC_OK)
			pr_err("%s failed\n", __func__);
	}
}

void mspc_factory_init(void)
{
	mutex_init(&g_mspc_otp_mutex);
	mspc_set_recovery_flag();
}
