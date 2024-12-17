/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: Base driver for Seplat, including initialization.
 * Create: 2023/09/21
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <teek_client_constants.h>
#include <securec.h>
#include <linux/kthread.h>
#include <linux/wait.h>

#include "seplat_mntn.h"

#ifndef unused
#define unused(x) (void)(x)
#endif

/* uuid to SE TA: 78a354fa-7843-487b-a69e-0da325a828aa */
#define UUID_TEE_SERVICE_SEPLAT                        \
{                                                      \
	0x78a354fa, 0x7843, 0x487b,                        \
	{                                                  \
		0xa6, 0x9e, 0x0d, 0xa3, 0x25, 0xa8, 0x28, 0xaa \
	}                                                  \
}

#define ROOTID 0
#define SEPLAT_CHIP_TYPE    "ca11"
#define TEE_SERVICE_NAME    "seplat"
#define SEPLAT_DEVICE_NAME  "seplat"
/* Make sure it is the same as the definition in tee. */
#define SEPLAT_TEE_CMD_ID_SHUTDOWN  11U
#define SEPLAT_WAIT_TIMEOUT_MS 10

static struct task_struct *g_notifier_thread;
static DECLARE_WAIT_QUEUE_HEAD(g_seplat_shutdown_wait);
static bool g_enter_off_mode_finish = false;

static int seplat_teec_send_cmd(TEEC_Session *session, u32 cmd)
{
	TEEC_Result result;
	TEEC_Operation op = {0};
	u32 origin = 0;

	if (!session) {
		pr_err("%s: param error!\n", __func__);
		return -EINVAL;
	}

	op.started = 1;
	op.cancel_flag = 0;
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
					TEEC_NONE, TEEC_NONE);

	result = TEEK_InvokeCommand(session, cmd, &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s, send cmd fail, res=0x%x, origin=0x%x\n", __func__, result, origin);
		return -EFAULT;
	}

	return 0;
}

static int seplat_teec_open(TEEC_Context *context, TEEC_Session *session)
{
	int ret;
	u32 root_id = ROOTID;
	u32 origin = 0;
	const char *package_name = TEE_SERVICE_NAME;
	TEEC_UUID svc_id = UUID_TEE_SERVICE_SEPLAT;
	TEEC_Operation op;
	TEEC_Result result;

	ret = memset_s(&op, sizeof(TEEC_Operation), 0, sizeof(TEEC_Operation));
	if (ret) {
		pr_err("memset_s failed\n");
		return ret;
	}

	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		pr_err("InitializeContext failed, result=0x%x\n", result);
		goto cleanup_1;
	}

	op.started = 1;
	op.cancel_flag = 0;
	op.params[2].tmpref.buffer = (void *)&root_id;
	op.params[2].tmpref.size = sizeof(root_id);
	op.params[3].tmpref.buffer = (void *)package_name;
	op.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
					TEEC_NONE,
					TEEC_MEMREF_TEMP_INPUT,
					TEEC_MEMREF_TEMP_INPUT);

	result = TEEK_OpenSession(context, session, &svc_id,
				TEEC_LOGIN_IDENTIFY, NULL, &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s, OpenSession fail, rt=0x%x, origin=0x%x\n", __func__, result, origin);
		goto cleanup_2;
	}

	return 0;
cleanup_2:
	TEEK_FinalizeContext(context);
cleanup_1:
	return -EINVAL;
}

static void seplat_teec_close(TEEC_Context *context, TEEC_Session *session)
{
	TEEK_CloseSession(session);
	TEEK_FinalizeContext(context);
}

static int seplat_enter_off_mode(void)
{
	int ret;
	TEEC_Context ctx;
	TEEC_Session session;

	ret = seplat_teec_open(&ctx, &session);
	if (ret) {
		pr_err("%s, open session failed!\n", __func__);
		return ret;
	}

	ret = seplat_teec_send_cmd(&session, SEPLAT_TEE_CMD_ID_SHUTDOWN);
	if (ret)
		pr_err("%s, send cmd failed!\n", __func__);

	seplat_teec_close(&ctx, &session);

	pr_debug("%s, exit\n", __func__);

	return ret;
}

static int seplat_shutdown_notifier_call(struct notifier_block *nb,
				unsigned long action, void *data)
{
	int ret;

	unsigned long timeout = msecs_to_jiffies(SEPLAT_WAIT_TIMEOUT_MS);
	unused(nb);
	unused(action);
	unused(data);

	ret = wake_up_process(g_notifier_thread);
	if (ret == 0) {
		pr_err("%s, %d, seplat shutdown thread wake up failed!\n", __func__, __LINE__);
		return NOTIFY_OK;
	}
	while (!g_enter_off_mode_finish)
		(void)wait_event_timeout(g_seplat_shutdown_wait,
					 g_enter_off_mode_finish,
					 (long)timeout);
	pr_debug("%s exit\n", __func__);

	return NOTIFY_OK;
}

static struct notifier_block seplat_notifier_block = {
	.notifier_call = seplat_shutdown_notifier_call,
};

static int seplat_get_chiptype(const struct device_node *of_node,
				const char **chip_type)
{
	int ret;

	ret = of_property_read_string(of_node, "chip_type", chip_type);
	if (ret) {
		pr_err("%s: get chip type failed!\n", __func__);
		return ret;
	}

	return 0;
}

static int seplat_check_chiptype(const char *chip_type)
{
	if (strncmp(chip_type, SEPLAT_CHIP_TYPE, strlen(SEPLAT_CHIP_TYPE)) != 0)
		return -EINVAL;

	return 0;
}

static int seplat_shutdown_notifier_thread(void *arg)
{
	int ret;

	ret = seplat_enter_off_mode();
	if (ret) {
		pr_err("seplat_enter_off_mode failed, ret=%d\n", ret);
		goto exit;
	}

	pr_debug("seplat_shutdown_notifier_thread succ\n");
exit:
	g_enter_off_mode_finish = true;
	wake_up(&g_seplat_shutdown_wait);

	do_exit(ret);
}

static int __init seplat_probe(struct platform_device *pdev)
{
	int ret;
	const char *chip_type = NULL;

	ret = seplat_mntn_init();
	if (ret)
		pr_err("seplat_mntn_init failed, ret=%d\n", ret);

	ret = seplat_get_chiptype(pdev->dev.of_node, &chip_type);
	if (ret) {
		pr_err("seplat_get_chiptype failed, ret=%d\n", ret);
		return ret;
	}

	ret = seplat_check_chiptype(chip_type);
	if (ret) {
		pr_err("chip not valid, ret=%d\n", ret);
		return ret;
	}

	g_notifier_thread = kthread_create(seplat_shutdown_notifier_thread,
					NULL,
					"seplat_shutdown_notifier_thread");
	if (IS_ERR((const void *)g_notifier_thread)) {
		pr_err("kthread_create failed, ret=%ld\n", PTR_ERR((const void *)g_notifier_thread));
		return PTR_ERR((const void *)g_notifier_thread);
	}

	ret = register_reboot_notifier(&seplat_notifier_block);
	if (ret) {
		pr_err("%s, notifier register err, ret=%d\n", __func__, ret);
		return ret;
	}

	pr_debug("%s succ\n", __func__);

	return 0;
}

static int seplat_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = unregister_reboot_notifier(&seplat_notifier_block);
	if (ret)
		pr_err("%s, notifier unregister err, ret=%d\n", __func__, ret);

	return ret;
}

static const struct of_device_id seplat_of_match[] = {
	{.compatible = "hisilicon,seplat" },
	{ }
};
MODULE_DEVICE_TABLE(of, seplat_of_match);

static struct platform_driver seplat_driver = {
	.probe =  seplat_probe,
	.remove = seplat_remove,
	.driver = {
		.name =  SEPLAT_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(seplat_of_match),
	},
};

static int __init seplat_module_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&seplat_driver);
	if (ret)
		pr_err("register seplat driver failed, ret=%d\n", ret);
	return ret;
}

static void __exit seplat_module_exit(void)
{
	platform_driver_unregister(&seplat_driver);
}

module_init(seplat_module_init);
module_exit(seplat_module_exit);

MODULE_DESCRIPTION("SEPLAT KERNEL DRIVER");
MODULE_LICENSE("GPL V2");
