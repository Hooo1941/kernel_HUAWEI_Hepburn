/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: sensorhub version
 * Create: 2021.11.16
 */

#include <securec.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/io.h>
#include <platform_include/smart/linux/iomcu_log.h>
#include <platform_include/smart/linux/iomcu_config.h>
#include <platform_include/smart/linux/iomcu_status.h>

static struct class *g_contexthub_class;

#define MAX_VERSION_LEN	30
enum sernsorhub_version_status {
	SENSORHUB_VERSION_INIT_SUCC = 0,
	SENSORHUB_VERSION_INIT_FAIL = -1
};

static ssize_t cls_attr_sensorhub_version_show(struct class *dev,
	struct class_attribute *attr, char *buf)
{
	struct version_config *ver = iomcu_config_get(IOMCU_CONFIG_USR_VERSION, sizeof(struct version_config));

	return snprintf_s(buf, MAX_VERSION_LEN, (MAX_VERSION_LEN - 1), "%s\n", ver->sensorhub_version);
}

static struct class_attribute g_class_attr_sensorhub_version =
	__ATTR(sensorhub_version, 0440, cls_attr_sensorhub_version_show, NULL);

static int __init sensorhub_version_init(void)
{
	int ret;

	if (get_contexthub_dts_status() != 0)
		return SENSORHUB_VERSION_INIT_FAIL;

	g_contexthub_class = class_create(THIS_MODULE, "contexthub");
	if (IS_ERR(g_contexthub_class)) {
		ctxhub_err("%s contexthub class creat fail\n", __func__);
		goto CLASS_ERR;
	}

	ret = class_create_file(g_contexthub_class,
				&g_class_attr_sensorhub_version);
	if (ret != 0) {
		ctxhub_err("%s creat cls file sensorhub_version fail\n", __func__);
		goto CLASS_FILE_ERR;
	}

	return SENSORHUB_VERSION_INIT_SUCC;
CLASS_FILE_ERR:
	class_destroy(g_contexthub_class);
CLASS_ERR:
	return SENSORHUB_VERSION_INIT_FAIL;
}

static void __exit sensorhub_version_exit(void)
{
	class_remove_file(g_contexthub_class, &g_class_attr_sensorhub_version);
	class_destroy(g_contexthub_class);
	g_contexthub_class = NULL;
}

late_initcall_sync(sensorhub_version_init);
module_exit(sensorhub_version_exit);

