/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: Base driver for Crypto Core, including initialization.
 * Create: 2019/11/11
 */

#include "crypto_core_internal.h"
#include <platform_include/see/efuse_driver.h>
#include <vendor_rpmb.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <platform_include/basicplatform/linux/partition/partition_ap_kernel.h>
#include <linux/rpmb/rpmb.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <securec.h>
#include "crypto_core_at.h"
#include "crypto_core_errno.h"
#include "crypto_core_factory.h"
#include "crypto_core_flash.h"
#include "crypto_core_fs.h"
#include "crypto_core.h"
#include "crypto_core_power.h"
#include "crypto_core_smc.h"

#define MSPC_WAIT_RPMP_TIMES                20
#define MSPC_WAIT_RPMB_DELAY                500  /* 500ms */

#define MSPC_WAIT_PARTITION_READY_CNT       5
#define MSPC_WAIT_PARTITION_DELAY           500 /* 500ms */

#define MSPC_ERRCODE_MAXLEN                 12

/* Begin of msp core file conctrol define. */
#ifdef CONFIG_DFX_DEBUG_FS
static DEVICE_ATTR(mspc_power, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
		   mspc_powerctrl_show, mspc_powerctrl_store);
static DEVICE_ATTR(mspc_check_ready, (S_IRUSR | S_IRGRP),
		   mspc_check_ready_show, NULL);
#endif /* CONFIG_DFX_DEBUG_FS */

static DEVICE_ATTR(mspc_at_ctrl, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
		   mspc_at_ctrl_show, mspc_at_ctrl_store);

/* End of msp core file conctrol define. */

struct mspc_module_data g_mspc_data;

static int32_t g_mspc_status = MSPC_MODULE_UNREADY;

int32_t mspc_filectrl_preprocess(const int8_t *buf, size_t count)
{
	if (unlikely(!buf || count > MSPC_CMD_MAX_LEN)) {
		pr_err("%s:Invalid param:%lu\n", __func__, count);
		return	MSPC_INVALID_PARAM_ERROR;
	}

	if (unlikely(!g_mspc_data.rpmb_is_ready)) {
		pr_err("%s:rpmb is not ready now!\n", __func__);
		return MSPC_RPMB_MODULE_INIT_ERROR;
	}
	return MSPC_OK;
}

int32_t mspc_get_init_status(void)
{
	return g_mspc_status;
}

void mspc_lock(void)
{
	mutex_lock(&g_mspc_data.mspc_mutex);
}

void mspc_unlock(void)
{
	mutex_unlock(&g_mspc_data.mspc_mutex);
}

void mspc_record_errno(int32_t error)
{
	atomic_set(&g_mspc_data.errno, error);
}

int32_t mspc_get_recorded_errno(void)
{
	return atomic_read(&g_mspc_data.errno);
}

struct device *mspc_get_device(void)
{
	return g_mspc_data.device;
}

uint32_t crypto_core_get_sm_efuse_pos(void)
{
	return g_mspc_data.sm_efuse_pos;
}

static int32_t mspc_wait_rpmb_ready(void)
{
	uint32_t retry = MSPC_WAIT_RPMP_TIMES;
	const uint32_t timeout = MSPC_WAIT_RPMB_DELAY;

	do {
		if (get_rpmb_init_status() == RPMB_DRIVER_IS_READY) {
			g_mspc_data.rpmb_is_ready = true;
			break;
		}
		msleep(timeout);
		retry--;
	} while (retry > 0);

	if (retry == 0)
		return MSPC_ERROR;

	return MSPC_OK;
}

static int32_t mspc_wait_partition_ready(void)
{
	int8_t fullpath[MSPC_PTN_PATH_BUF_SIZE] = {0};
	uint32_t i, retry;
	uint32_t timeout = MSPC_WAIT_PARTITION_DELAY;
	const char *ptn_name[] = {
		MSPC_ENCOS_PARTITION_NAME, MSPC_FS_PARTITION_NAME
	};
	int32_t ret;

	for (i = 0; i < ARRAY_SIZE(ptn_name); i++) {
		retry = MSPC_WAIT_PARTITION_READY_CNT;
		ret = flash_find_ptn_s(ptn_name[i], fullpath, sizeof(fullpath));
		if (ret != 0) {
			pr_err("%s:find %u failed:%d\n", __func__, i, ret);
			return MSPC_FS_FIND_PTN_ERROR;
		}

		do {
			if(crypto_core_sys_access(fullpath, 0) == 0)
				break;
			msleep(timeout);
			retry--;
		} while (retry > 0);

		if (retry == 0)
			return MSPC_ACCESS_FILE_ERROR;
	}
	return MSPC_OK;
}

static int32_t mspc_preprocess_thread(void *data)
{
	int32_t ret;

	ret = mspc_wait_rpmb_ready();
	if (ret != MSPC_OK) {
		pr_err("%s:wait rpmb ready timeout!\n", __func__);
		goto exit;
	}

	/* Flash cos is saved in ufs normal partition. */
	ret = mspc_wait_partition_ready();
	if (ret != MSPC_OK) {
		pr_err("%s:wait partition ready failed!\n", __func__);
		goto exit;
	}

	/* After loading native image, mspc is ready to use. */
	g_mspc_status = MSPC_MODULE_READY;

exit:
	pr_err("%s:run ret 0x%x\n", __func__, ret);
	mspc_record_errno(ret);
	return ret;
}

static int32_t mspc_remove(struct platform_device *pdev)
{
	if (g_mspc_data.device) {
#ifdef CONFIG_DFX_DEBUG_FS
		device_remove_file(g_mspc_data.device, &dev_attr_mspc_power);
		device_remove_file(g_mspc_data.device,
				   &dev_attr_mspc_check_ready);
#endif
		device_remove_file(g_mspc_data.device,
				   &dev_attr_mspc_at_ctrl);
		of_reserved_mem_device_release(g_mspc_data.device);
	}
	return 0;
}

static int32_t mspc_create_files(struct device *pdevice)
{
	int32_t ret = MSPC_OK;

	if (device_create_file(pdevice, &dev_attr_mspc_at_ctrl) != 0) {
		ret = MSPC_AT_CTRL_NODE_CREATE_ERROR;
		pr_err("%s:create mspc_at_ctrl file failed!\n", __func__);
		goto err_exit;
	}

#ifdef CONFIG_DFX_DEBUG_FS
	if (device_create_file(pdevice, &dev_attr_mspc_power) != 0) {
		ret = MSPC_POWER_NODE_CREATE_ERROR;
		pr_err("%s:create mspc_power file failed!\n", __func__);
		goto err_device_remove_file1;
	}

	if (device_create_file(pdevice, &dev_attr_mspc_check_ready) != 0) {
		ret = MSPC_POWER_NODE_CREATE_ERROR;
		pr_err("%s:create mspc_check_ready file failed!\n", __func__);
		goto err_device_remove_file2;
	}

	return ret;
err_device_remove_file2:
	device_remove_file(pdevice, &dev_attr_mspc_power);
err_device_remove_file1:
	device_remove_file(pdevice, &dev_attr_mspc_at_ctrl);
#endif
err_exit:
	return ret;
}

static void mspc_remove_files(struct device *pdevice)
{
	device_remove_file(pdevice, &dev_attr_mspc_at_ctrl);
#ifdef CONFIG_DFX_DEBUG_FS
	device_remove_file(pdevice, &dev_attr_mspc_check_ready);
	device_remove_file(pdevice, &dev_attr_mspc_power);
#endif
}

#define CRYPTO_CORE_PREPROCESS_TASK_ENABLE true

struct crypto_core_thread {
	int32_t (*threadfn)(void *data);
	const char *name;
	bool enabled;
};

static int32_t mspc_create_thread(void)
{
	const struct crypto_core_thread thread[] = {
		{ mspc_preprocess_thread, "mspc_preprocess_task", CRYPTO_CORE_PREPROCESS_TASK_ENABLE },
	};
	struct task_struct *task = NULL;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(thread); i++) {
		if (thread[i].enabled) {
			task = kthread_run(thread[i].threadfn, NULL, "%s", thread[i].name);
			if (!task) {
				pr_err("%s:create %s failed!\n", __func__, thread[i].name);
				return MSPC_THREAD_CREATE_ERROR;
			}
		}
	}
	return MSPC_OK;
}

static int32_t __init mspc_probe(struct platform_device *pdev)
{
	struct device *pdevice = &(pdev->dev);
	int32_t ret;

	(void)memset_s((void *)&g_mspc_data,
		       sizeof(g_mspc_data), 0, sizeof(g_mspc_data));

	g_mspc_data.device = pdevice;

	/* Initialize mutex, semaphore etc.. */
	mutex_init(&(g_mspc_data.mspc_mutex));
	mspc_init_smc();
	mspc_init_power();
	mspc_factory_init();

	/* Aassign a reserved CMA memory region to mspc. */
	ret = of_reserved_mem_device_init(pdevice);
	if (ret != 0) {
		pr_err("%s:Register mspc shared cma failed!\n", __func__);
		mspc_record_errno(MSPC_CMA_DEVICE_INIT_ERROR);
		return ret;
	}
	pr_err("mspc shared cma registere successful!\n");

	/* Create mspc files. */
	ret = mspc_create_files(pdevice);
	if (ret != MSPC_OK) {
		pr_err("%s:create files failed!\n", __func__);
		goto err_mem_release;
	}

	/* Create mspc task. */
	ret = mspc_create_thread();
	if (ret != MSPC_OK) {
		pr_err("%s:create task failed!\n", __func__);
		goto err_device_remove_file;
	}

	mspc_flash_register_func(mspc_flash_debug_switchs);
	pr_err("mspc module init successful!\n");
	mspc_record_errno(ret);
	return 0;

err_device_remove_file:
	mspc_remove_files(pdevice);
err_mem_release:
	of_reserved_mem_device_release(g_mspc_data.device);

	mspc_record_errno(ret);
	return ret;
}

static const struct of_device_id mspc_of_match[] = {
	{.compatible = "hisilicon,mspc-device" },
	{ }
};
MODULE_DEVICE_TABLE(of, mspc_of_match);

static struct platform_driver mspc_driver = {
	.probe = mspc_probe,
	.remove = mspc_remove,
	.driver = {
		.name  = MSPC_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mspc_of_match),
	},
};

static int32_t __init mspc_module_init(void)
{
	int32_t ret = 0;

	ret = platform_driver_register(&mspc_driver);

	if (ret)
		pr_err("register mspc driver failed.\n");

	return ret;
}
static void __exit mspc_module_exit(void)
{
	platform_driver_unregister(&mspc_driver);
}
module_init(mspc_module_init);
module_exit(mspc_module_exit);

MODULE_DESCRIPTION("MSP CORE KERNEL DRIVER");
MODULE_LICENSE("GPL V2");
