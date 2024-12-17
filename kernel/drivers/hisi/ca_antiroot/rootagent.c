/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2022. All rights reserved.
 * Description: the rootagent.c for kernel space root scan.
 * Create: 2016-5-19
 */

#include "rootagent.h"
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/random.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "rootagent_api.h"
#include "rootagent_common.h"
#ifdef CONFIG_TEE_ANTIROOT_CLIENT_ENG_DEBUG
#include "rootagent_check.h"
#endif

#define REPEATNUM 3
#define HEX_NUM 16
#define ROOTAGENT_SLEEP_TIME 50 /* ms */
#define ROOTAGENT_VERSION "rootagent-v0.05"
#define TIMER_PERIOC (jiffies + msecs_to_jiffies(SCAN_CYCLE))
#define TIMER_FIRST (jiffies + msecs_to_jiffies(SCAN_CYCLE))
#define DEFAULT_PROC "/init"

static void do_work(struct work_struct *unused);
static void module_init_do_work(struct work_struct *unused);
static DECLARE_WORK(teeagent_work, do_work);
static DECLARE_WORK(module_init_work, module_init_do_work);
static struct workqueue_struct *g_teeagent_workqueue;
static struct timer_list g_teeagent_timer;
static struct ragent_config g_ragent_config;
static struct rscan_result_dynamic *g_scan;
static DEFINE_MUTEX(root_ca_lock);

#define PM_SUSPEND 0x1 /* 1 for suspend */
#define PM_RESUME 0x0 /* 0 for resume successful */
int g_antiroot_sr_flag = PM_RESUME;

static inline struct wakeup_source *rootagent_wakeup_register(const char *name)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	return wakeup_source_register(NULL, name);
#else
	return wakeup_source_register(name);
#endif
}

static TEEC_Result req_challenge(struct ragent_challenge *chal)
{
	TEEC_Result teec_ret;

	struct rscan_status status = {0};

	if (chal == NULL) {
		antiroot_error("invalid parameter\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	rscan_get_status(&status);

	/* can arbitary status values attack TA? */
	chal->cpuload = status.cpuload;
	chal->battery = status.battery;
	chal->charging = status.charging;
	chal->time = status.time;
	chal->timezone = status.timezone;

	antiroot_debug("%u, %u, %u, %u, %u\n",
		status.cpuload, status.battery, status.charging,
		status.time, status.timezone);

	teec_ret = request_challenge(chal);
	if (teec_ret != TEEC_SUCCESS)
		antiroot_error("request_challenge failed 0x%x\n", teec_ret);

	return teec_ret;
}

static uint32_t calculate_op_mask(uint32_t opid)
{
	uint32_t op_mask = 0;

	switch (opid) {
	case RAOPID_KCODE:
		op_mask |= D_RSOPID_KCODE;
		break;
	case RAOPID_SYSCALL:
		op_mask |= D_RSOPID_SYS_CALL;
		break;
	case RAOPID_RPROCS:
		op_mask |= D_RSOPID_RRPOCS;
		break;
	case RAOPID_SELINUX_STATUS:
		op_mask |= D_RSOPID_SE_STATUS;
		break;
	case RAOPID_SELINUX_HOOK:
		op_mask |= D_RSOPID_SE_HOOKS;
		break;
	case RAOPID_SETID:
		op_mask |= D_RSOPID_SETID;
		break;
	case RAOPID_NOOP:
		break;
	default:
		/*
		 * if TA send a OPID that CA not support,
		 * just not check or report
		 * this OPID, and continue next one.
		 */
		antiroot_warning("gen_response switch invalid OPID %u\n", opid);
		break;
	}
	return op_mask;
}

static bool is_rprocs_invalid(uint32_t raw)
{
	return (raw & D_RSOPID_RRPOCS) || (strlen(g_scan->rprocs) == 0) ||
		(strlen(g_scan->rprocs) > ROOTAGENT_RPROCS_MAX_LENGTH);
}

static int get_kcodes(uint32_t raw, struct ragent_response *resp)
{
	int s_ret;

	if (raw & D_RSOPID_KCODE)
		s_ret = memcpy_s(resp->runtime_white_list.kcodes,
				KERNEL_CODE_LENGTH_SHA,
				g_ragent_config.white_list.kcodes,
				KERNEL_CODE_LENGTH_SHA);
	else
		s_ret = memcpy_s(resp->runtime_white_list.kcodes,
				KERNEL_CODE_LENGTH_SHA,
				g_scan->kcode,
				KERNEL_CODE_LENGTH_SHA);
	if (s_ret != EOK)
		antiroot_error("memcpy_s failed\n");

	return s_ret;
}

static int get_syscalls(uint32_t raw, struct ragent_response *resp)
{
	int s_ret;

	if (raw & D_RSOPID_SYS_CALL)
		s_ret = memcpy_s(resp->runtime_white_list.syscalls,
				SYSTEM_CALL_LENGTH_SHA,
				g_ragent_config.white_list.syscalls,
				SYSTEM_CALL_LENGTH_SHA);
	else
		s_ret = memcpy_s(resp->runtime_white_list.syscalls,
				SYSTEM_CALL_LENGTH_SHA,
				g_scan->syscalls,
				SYSTEM_CALL_LENGTH_SHA);
	if (s_ret != EOK)
		antiroot_error("memcpy_s failed\n");

	return s_ret;
}

static void get_root_proc(uint32_t raw, struct ragent_rootproc *root_proc)
{
	if (is_rprocs_invalid(raw)) {
		root_proc->length = strlen(DEFAULT_PROC);
		root_proc->procid = (uint8_t *)DEFAULT_PROC;
	} else {
		root_proc->length = strlen(g_scan->rprocs);
		root_proc->procid = (uint8_t *)(g_scan->rprocs);
	}
}

static void get_se_status(uint32_t raw, struct ragent_response *resp)
{
	if (raw & D_RSOPID_SE_STATUS)
		resp->runtime_white_list.selinux = g_ragent_config.white_list.selinux;
	else
		resp->runtime_white_list.selinux = (g_scan->seenforcing != 1);
}

static int get_se_hooks(uint32_t raw, struct ragent_response *resp)
{
	int s_ret;

	if (raw & D_RSOPID_SE_HOOKS)
		s_ret = memcpy_s(resp->runtime_white_list.sehooks,
				SELINUX_HOOKS_LENGTH_SHA,
				g_ragent_config.white_list.sehooks,
				SELINUX_HOOKS_LENGTH_SHA);
	else
		s_ret = memcpy_s(resp->runtime_white_list.sehooks,
				SELINUX_HOOKS_LENGTH_SHA,
				g_scan->sehooks,
				SELINUX_HOOKS_LENGTH_SHA);
	if (s_ret != EOK)
		antiroot_error("memcpy_s failed\n");

	return s_ret;
}

static void get_setid(uint32_t raw, struct ragent_response *resp)
{
	if (raw & D_RSOPID_SETID)
		resp->runtime_white_list.setid =
			g_ragent_config.white_list.setid;
	else
		resp->runtime_white_list.setid = g_scan->setid;
}

static int handle_challengeid(uint32_t raopid, uint32_t raw,
			struct ragent_response *resp,
			struct ragent_rootproc *root_proc)
{
	int ret = EOK;

	switch (raopid) {
	case RAOPID_KCODE:
		ret = get_kcodes(raw, resp);
		break;
	case RAOPID_SYSCALL:
		ret = get_syscalls(raw, resp);
		break;
	case RAOPID_RPROCS:
		get_root_proc(raw, root_proc);
		break;
	case RAOPID_SELINUX_STATUS:
		get_se_status(raw, resp);
		break;
	case RAOPID_SELINUX_HOOK:
		ret = get_se_hooks(raw, resp);
		break;
	case RAOPID_SETID:
		get_setid(raw, resp);
		break;
	case RAOPID_NOOP:
		resp->noop = 0;
		break;
	default:
		break;
	}
	return ret;
}

static uint32_t op_mask_handle(const struct ragent_challenge *chal)
{
	int id_count;
	uint32_t opid;
	uint32_t op_mask = 0;

	for (id_count = 0; (id_count < CHALLENGE_MAX_LENGTH) &&
			(chal->challengeid[id_count]); id_count++) {
		opid = chal->challengeid[id_count];
		antiroot_debug("gen_response opid = %u\n", opid);
		if ((opid <= 0) || (opid >= RAOPID_MAXID)) {
			antiroot_warning("gen_response start invalid OPID %u\n", opid);
			/*
			 * if TA return challenge error,
			 * we only check and report
			 * the right opid.
			 */
			continue;
		}
		op_mask |= calculate_op_mask(opid);
	}
	return op_mask;
}

static int send_response(const struct ragent_challenge *chal)
{
	uint32_t op_mask;
	struct ragent_response resp;
	struct ragent_rootproc root_proc;
	TEEC_Result teec_ret;
	int id_count;
	int ret_raw = 0;
	int s_ret;

	op_mask = op_mask_handle(chal);

	s_ret = memset_s(g_scan, sizeof(struct rscan_result_dynamic),
			0x0, sizeof(struct rscan_result_dynamic));
	if (s_ret != EOK) {
		antiroot_error("memset_s failed\n");
		return s_ret;
	}

	antiroot_info("challenge is 0x%x\n", op_mask);

	if (op_mask != 0)
		ret_raw = rscan_dynamic_raw_and_upload(op_mask, g_scan);

	if (ret_raw < 0) {
		antiroot_error("rscan dynamic raw and upload failed\n");
		return ret_raw;
	}

	s_ret = memset_s(&resp, sizeof(struct ragent_response),
			0x0, sizeof(struct ragent_response));
	if (s_ret != EOK) {
		antiroot_error("memset_s failed\n");
		return s_ret;
	}
	s_ret = memset_s(&root_proc, sizeof(struct ragent_rootproc),
			0x0, sizeof(struct ragent_rootproc));
	if (s_ret != EOK) {
		antiroot_error("memset_s failed\n");
		return s_ret;
	}

	for (id_count = 0; (id_count < CHALLENGE_MAX_LENGTH) &&
			(chal->challengeid[id_count] != 0); id_count++) {
		s_ret = handle_challengeid(chal->challengeid[id_count],
					ret_raw, &resp, &root_proc);
		if (s_ret != EOK)
			return s_ret;
	}
	teec_ret = feedback_response(&resp, &root_proc);
	if (teec_ret != TEEC_SUCCESS) {
		antiroot_debug("feedback_response failed 0x%x\n", teec_ret);
		return teec_ret;
	}
	return TEEC_SUCCESS;
}

static int system_sr_notify(struct notifier_block *nb,
			unsigned long pm_event, void *unused)
{
	(void)unused;

	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
		g_antiroot_sr_flag = PM_SUSPEND;
		antiroot_debug("system is suspending\n");
		break;
	case PM_POST_SUSPEND:
		g_antiroot_sr_flag = PM_RESUME;
		antiroot_debug("system resume successful\n");
		break;
	default:
		break;
	}
	return 0;
}

static int do_req_challenge(struct ragent_challenge *challenge)
{
	/*
	 * REPEATNUM
	 * if req_challenge failed, agent will request again.
	 * max request time is REPEATNUM.
	 */
	int repeat = REPEATNUM;
	TEEC_Result ret;

	do {
		ret = req_challenge(challenge);
		antiroot_debug("req_challenge ret = %x\n", ret);
		if (ret == TEEC_SUCCESS)
			return TEEC_SUCCESS;

		if (ret == (uint32_t)TEE_ERROR_ANTIROOT_RSP_FAIL) {
			antiroot_error("req_challenge TEE_ERROR_ANTIROOT_RSP_FAIL\n");
			return TEE_ERROR_ANTIROOT_RSP_FAIL; /* only when TA readstatus is root */
		}
	} while (--repeat);

	return TEE_ERROR_ANTIROOT_RSP_FAIL;
}

static int do_send_response(struct ragent_challenge *challenge)
{
	/*
	 * REPEATNUM
	 * if send_response failed, agent will request again.
	 * max request time is REPEATNUM.
	 */
	int repeat = REPEATNUM;
	TEEC_Result ret;
	int status = get_ro_secure();

	do {
		ret = send_response(challenge);
		antiroot_debug("send_response ret = %x\n", ret);
		if (ret == TEEC_SUCCESS) {
			rodata_stp_upload();
			return TEEC_SUCCESS;
		} else if (ret == (uint32_t)TEE_ERROR_ANTIROOT_RSP_FAIL) {
			antiroot_debug(
				"send_response TEE_ERROR_ANTIROOT_RSP_FAIL, status = %d\n", status);
			if (status)
				return TEE_ERROR_ANTIROOT_RSP_FAIL;
			else
				return TEEC_SUCCESS;
		}
	} while (--repeat);

	return TEE_ERROR_ANTIROOT_RSP_FAIL;
}

static void do_work(struct work_struct *unused)
{
	struct wakeup_source *anti_wake_src = NULL;
	struct ragent_challenge challenge;
	int status;
	int s_ret;

	(void)unused;

	mutex_lock(&root_ca_lock);
	status = get_ro_secure();
	anti_wake_src = rootagent_wakeup_register("rootscan");
	if (!anti_wake_src) {
		antiroot_error("do_work wakeup source register failed");
		mutex_unlock(&root_ca_lock);
		return;
	}
	while (g_antiroot_sr_flag == PM_SUSPEND) {
		antiroot_warning("system is suspend now, try later\n");
		msleep(ROOTAGENT_SLEEP_TIME);
	}

	s_ret = memset_s(&challenge, sizeof(struct ragent_challenge),
			0x0, sizeof(struct ragent_challenge));
	if (s_ret != EOK) {
		antiroot_error("memset_s failed\n");
		goto out;
	}

	antiroot_debug("status = %d\n", status);

	s_ret = do_req_challenge(&challenge);
	if (s_ret != TEEC_SUCCESS)
		goto error;

	s_ret = do_send_response(&challenge);
	if (s_ret == TEEC_SUCCESS)
		goto out;
	else
		goto error;

error:
	/* prepare CA/TA to stop */
	wakeup_source_unregister(anti_wake_src);
	root_monitor_finalize();
	del_timer_sync(&g_teeagent_timer);
	mutex_unlock(&root_ca_lock);
	antiroot_info("antiroot is closing up\n");
	return;

out:
	wakeup_source_unregister(anti_wake_src);
	mutex_unlock(&root_ca_lock);
	antiroot_debug("do_work out!\n");
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void teeagent_timer_func(unsigned long unused)
#else
static void teeagent_timer_func(struct timer_list *unused)
#endif
{
	(void)unused;

	mod_timer(&g_teeagent_timer, TIMER_PERIOC); /* 10 mins */

	if (queue_work(g_teeagent_workqueue, &teeagent_work) == 0)
		antiroot_error("Timer work has joined the queued\n");
}

static int rscan_get_baseline(struct rscan_result_dynamic *base)
{
	int ret;

	if (base == NULL)
		return -1;

	ret = load_rproc_trustlist(base->rprocs, ROOTAGENT_RPROCS_MAX_LENGTH);
	if ((ret != 0) || (strlen(base->rprocs) == 0) ||
			(strlen(base->rprocs) > ROOTAGENT_RPROCS_MAX_LENGTH)) {
		ret = memcpy_s(base->rprocs, MAX_RPROC_SIZE, DEFAULT_PROC,
			strlen(DEFAULT_PROC));
		if (ret != EOK) {
			antiroot_error("memcpy_s failed\n");
			return ret;
		}
	}

	rscan_dynamic_raw(D_RSOPID_SETID, base);

	base->seenforcing = 0;
	return ret;
}

static int tee_init(void)
{
	TEEC_Result teec_ret;
	struct ragent_rootproc root_proc;

	g_scan = kzalloc(sizeof(struct rscan_result_dynamic), GFP_KERNEL);
	if (g_scan == NULL) {
		antiroot_error("g_scan (struct rscan_result_dynamic) malloc failed\n");
		return -ENOMEM;
	}
	if (rscan_get_baseline(g_scan) != 0) {
		antiroot_error("rscan_get_baseline failed\n");
		goto out;
	}

	mutex_lock(&root_ca_lock);
	teec_ret = root_monitor_tee_init();
	mutex_unlock(&root_ca_lock);
	if (teec_ret != TEEC_SUCCESS) {
		antiroot_error("root_monitor_tee_init failed 0x%x\n", teec_ret);
		goto out;
	}

	g_ragent_config.white_list.dstatus = get_ro_secure();
	antiroot_debug("tee_init dstatus = %u\n",
		g_ragent_config.white_list.dstatus);

	g_ragent_config.white_list.selinux = g_scan->seenforcing;
	g_ragent_config.white_list.setid = g_scan->setid;

	get_random_bytes(&g_ragent_config.cipher_key.cha_req_key,
			CHALLENGE_REQ_KEY_LENGTH);
	get_random_bytes(&g_ragent_config.cipher_key.cha_key,
			CHALLENGE_KEY_LENGTH);
	root_proc.length = strlen(g_scan->rprocs);
	antiroot_debug("proc length %u\n", root_proc.length);
	root_proc.procid = (uint8_t *)(g_scan->rprocs);

	mutex_lock(&root_ca_lock);
	teec_ret = setting_config(&g_ragent_config, &root_proc);
	mutex_unlock(&root_ca_lock);

	if (teec_ret != TEEC_SUCCESS) {
		antiroot_error("setting_config failed 0x%x\n", teec_ret);
		goto out;
	}
	return 0;

out:
	root_monitor_finalize();
	kfree(g_scan);
	g_scan = NULL;
	return -1;
}

static void module_init_do_work(struct work_struct *unused)
{
	int ret;
	static struct notifier_block system_sr_notifier = {
		.notifier_call = system_sr_notify,
	};

	(void)unused;

	if (g_teeagent_workqueue != NULL) {
		antiroot_info("antiroot-ca workqueue has been created\n");
		return;
	}

	antiroot_info("tee_rootscan_init versin: %s\n", ROOTAGENT_VERSION);

	g_teeagent_workqueue = create_singlethread_workqueue("antiroot-ca");
	if (g_teeagent_workqueue == NULL) {
		antiroot_error("fail to create queue\n");
		return;
	}

	ret = register_pm_notifier(&system_sr_notifier);
	if (ret != 0) {
		antiroot_error("fail to register system_sr_notifier\n");
		goto workqueue_free;
	}

	ret = tee_init();
	if (ret != 0) {
		antiroot_error("tee_init() failed. ret = %d\n", ret);
		unregister_pm_notifier(&system_sr_notifier);
		goto workqueue_free;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	g_teeagent_timer.function = teeagent_timer_func;
	g_teeagent_timer.data = 0;
	init_timer(&g_teeagent_timer);
#else
	timer_setup(&g_teeagent_timer, teeagent_timer_func, 0);
#endif
	mod_timer(&g_teeagent_timer, TIMER_FIRST);

#ifdef CONFIG_TEE_ANTIROOT_CLIENT_ENG_DEBUG
	handle_proc_create();
#endif

	return;
workqueue_free:
	destroy_workqueue(g_teeagent_workqueue);
}

#ifdef CONFIG_HW_ROOT_SCAN_RODATA_MEASUREMENT_API
void tee_rootscan_run(void)
{
	if (queue_work(system_wq, &module_init_work) == 0)
		antiroot_error("rootscan module work has joined the queued\n");
}

static __init int tee_rootscan_init(void)
{
	return 0;
}
#else
static __init int tee_rootscan_init(void)
{
	if (queue_work(system_wq, &module_init_work) == 0)
		antiroot_error("rootscan module work has joined the queued\n");

	return 0;
}
#endif

late_initcall(tee_rootscan_init);
