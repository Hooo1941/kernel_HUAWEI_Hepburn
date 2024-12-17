/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: Contexthub boot.
 * Create: 2019-06-10
 */
#include <platform_include/smart/linux/iomcu_config.h>
#include <platform_include/smart/linux/iomcu_log.h>
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <platform_include/smart/linux/iomcu_status.h>
#ifdef CONFIG_BSP_SYSCOUNTER
#include <linux/platform_drivers/bsp_syscounter.h>
#endif
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include "securec.h"
#include <linux/err.h>
#ifndef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <iomcu_ddr_map.h>
#else
#include <internal_security_interface.h>
#endif

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
unsigned int g_iomcu_config_start = 0;
unsigned int g_iomcu_config_size = 0;
unsigned int g_iomcu_config_default = 0;
#else
unsigned int g_iomcu_config_start = DDR_CONFIG_ADDR_AP;
unsigned int g_iomcu_config_size = DDR_CONFIG_SIZE;
unsigned int g_iomcu_config_default = (DDR_CONFIG_SIZE * 3 / 4);
#endif

// align to 4 bytes.
#define iomcu_config_align(x)  ((((x) + 3) >> 2) << 2)

static struct iomcu_config *g_iomcu_config = NULL;
static struct iomcu_config g_iomcu_abnormal_config = {0};
struct mutex g_iomcu_config_mutex;

int iomcu_config_register(int id, int len, int attr)
{
	if (id >= IOMCU_CONFIG_USR_MAX) {
		pr_err("%s: invalid id %x.\n", __func__, id);
		return -EFAULT;
	}

	mutex_lock(&g_iomcu_config_mutex);
	if (g_iomcu_config->node[id].offset != 0) {
		pr_err("%s: already registered. %x.\n", __func__, id);
		mutex_unlock(&g_iomcu_config_mutex);
		return -EFAULT;
	}

	if (g_iomcu_config->used + (unsigned int)len >= g_iomcu_config_default) {
		pr_err("%s: not enough memory. id %x, used %x, len %x.\n", __func__, id, g_iomcu_config->used, len);
		mutex_unlock(&g_iomcu_config_mutex);
		return -EFAULT;
	}

	g_iomcu_config->node[id].offset = (int)g_iomcu_config->used;
	g_iomcu_config->node[id].len = len;
	g_iomcu_config->used += iomcu_config_align((unsigned int)len);

	pr_info("%s: register success id %x, len %x. %x.\n", __func__, id, len);

	mutex_unlock(&g_iomcu_config_mutex);

	return 0;
}

void *iomcu_config_get(int id, int len)
{
	if (g_iomcu_config_default >= g_iomcu_config_size) {
		pr_err("%s:g_iomcu_config_default =%u\n", g_iomcu_config_default);
		return (void *)(&g_iomcu_abnormal_config);
	}

	if (g_iomcu_config == NULL) {
		pr_err("%s: init failed id 0x%x\n", __func__, id);
		return (void *)(&g_iomcu_abnormal_config);
	}

	if (id >= IOMCU_CONFIG_USR_MAX) {
		pr_err("%s: invalid id 0x%x\n", __func__, id);
		return (void *)(&g_iomcu_abnormal_config);
	}

	if (g_iomcu_config->node[id].offset == 0) {
		pr_err("%s: unregistered 0x%x\n", __func__, id);
		return (void *)(&g_iomcu_abnormal_config);
	}

	if (g_iomcu_config->node[id].offset + len >= (int)g_iomcu_config_default) {
		pr_err("%s: invalid offset id 0x%x, offset 0x%x, len 0x%x\n",
			__func__, id, g_iomcu_config->node[id].offset, len);
		return (void *)(&g_iomcu_abnormal_config);
	}

	if (g_iomcu_config->node[id].len != len) {
		pr_err("%s: len mismatch. id 0x%x, len 0x%x-0x%x.\n",
			__func__, id, g_iomcu_config->node[id].len, len);
		return (void *)(&g_iomcu_abnormal_config);
	}

	return (void *)((char *)g_iomcu_config + g_iomcu_config->node[id].offset);
}

void write_timestamp_base_to_sharemem(void)
{
	u64 syscnt = 0;
	u64 kernel_ns;
	struct timespec64 ts;
	struct time_config *time = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	ktime_get_boottime_ts64(&ts);
#else
	get_monotonic_boottime64(&ts);
#endif
#ifdef CONFIG_BSP_SYSCOUNTER
	syscnt = bsp_get_syscount();
#endif
	kernel_ns = (u64)(ts.tv_sec * NSEC_PER_SEC) + (u64)ts.tv_nsec;

	time = iomcu_config_get(IOMCU_CONFIG_USR_TIME, sizeof(struct time_config));
	time->timestamp_base.syscnt = syscnt;
	time->timestamp_base.kernel_ns = kernel_ns;

	return;
}

static int iomcu_config_init(void)
{
	int ret;
	struct log_config *log = NULL;
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	ret = get_dts_info();
	if (ret) {
		pr_err("%s:failed to get ddr dts info\n", __func__);
		ret = -EINVAL;
	} else {
		pr_info("%s:get ddr dts ok!\n", __func__);
	}
#endif
	// already init
	if (g_iomcu_config != NULL)
		return 0;

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	g_iomcu_config_start = dts_ddr_addr_get(DTSI_DDR_CONFIG_ADDR_AP);
	g_iomcu_config_size = dts_ddr_size_get(DTSI_DDR_CONFIG_ADDR_AP);
	g_iomcu_config_default = (g_iomcu_config_size * 3 / 4);
#endif
	g_iomcu_config = (struct iomcu_config *)ioremap_wc(g_iomcu_config_start, g_iomcu_config_size);
	if (g_iomcu_config == NULL) {
		pr_err("%s: remap failed.\n", __func__);
		return -EPIPE;
	}

	ret = memset_s((void *)g_iomcu_config, g_iomcu_config_size, 0, g_iomcu_config_size);
	if (ret != EOK) {
		pr_err("%s: memset_s err", __func__);

		goto init_failed;
	}

	g_iomcu_config->magic = DDR_CONFIG_MAGIC;
	g_iomcu_config->used = (unsigned int)sizeof(struct iomcu_config);

	mutex_init(&g_iomcu_config_mutex);

	(void)iomcu_config_register(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_LOG, sizeof(struct log_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_WAKEUP, sizeof(struct wakeup_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_GYRO, sizeof(struct gyro_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_MODEM, sizeof(struct modem_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_HIFI, sizeof(struct hifi_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_IGS, sizeof(struct igs_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_TIME, sizeof(struct time_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_BBOX, sizeof(struct bbox_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_ALS, sizeof(struct als_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_VERSION, sizeof(struct version_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_PEDO, sizeof(struct pedo_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_MOTION, sizeof(struct motion_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_COUL, sizeof(struct coul_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_NEARBY_BEACON, sizeof(struct nearby_beacon_config), 0);
	(void)iomcu_config_register(IOMCU_CONFIG_USR_ALS1, sizeof(struct als_config), 0);

	log = iomcu_config_get(IOMCU_CONFIG_USR_LOG, sizeof(struct log_config));
	log->log_buff_cb_backup.mutex = 0;
	log->log_level = INFO_LEVEL;

	write_timestamp_base_to_sharemem();

	pr_info("%s: done.\n", __func__);
	return 0;

init_failed:
	iounmap(g_iomcu_config);
	g_iomcu_config = NULL;
	return ret;
}

static void __exit iomcu_config_exit(void)
{
}

device_initcall(iomcu_config_init);
module_exit(iomcu_config_exit);

MODULE_DESCRIPTION("iomcu config");
MODULE_LICENSE("GPL");
