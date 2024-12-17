/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: This module is used to start vcom between ril and kernel
 * Author: lizhenzhou1@huawei.com
 * Create: 2022-11-26
 */

#include "vcom.h"

#include <securec.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <log/hw_log.h>

#include "dev/bbic_dev.h"
#include "vcom_nodes.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG HW_SATE_VCOM
HWLOG_REGIST();

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define THIS_MODU mod_stavcom
#define BASE_MONITOR_START 100

struct vcom_map_s {
	char* name;
	struct cdev cdev;
	dev_t devt;
	struct class* class;
	struct file_operations* ops;
	wait_queue_head_t wait;
	unsigned int iccid;
	unsigned int flag;
	unsigned int format;
	int (*idle)(struct vcom_map_s *map);
};

static struct file_operations vcom_dev_fops;
struct vcom_map_s* g_vcom_map;

static struct vcom_map_s* get_map(struct inode *inode)
{
	if (inode->i_cdev) {
		return container_of(inode->i_cdev, struct vcom_map_s, cdev);
	}
	return (struct vcom_map_s *)PDE_DATA(inode);
}

static int32_t update_firmware(unsigned int cmd, int temp_data)
{
	int32_t ret = -1;

	hwlog_info("%s ,temp_data:%d\n", __func__, temp_data);
	switch (cmd) {
	case BBIC_START_CMD:
		if (get_bbic_pwr_status() == true)
			return 0;
		ret = pwr_gpio_ctrl(true, false);
		break;
	case BBIC_STOP_CMD:
		if (get_bbic_pwr_status() == false)
			return 0;
		ret = pwr_gpio_ctrl(false, false);
		break;
	case BBIC_SET_BOOT_MODE_CMD:
		ret = set_boot_mode(temp_data);
		break;
	default:
		hwlog_info("%s invalid cmd:%d\n", __func__, cmd);
		break;
	}
	return ret;
}

static int32_t bbic_fac_cmd_proc(unsigned int cmd, int temp_data)
{
	int32_t ret = -1;

	hwlog_info("%s ,temp_data:%d\n", __func__, temp_data);
	switch (cmd) {
	case BBIC_START_CMD_FAC:
		if (get_bbic_pwr_status() == true)
			return 0;
		ret = pwr_gpio_ctrl(true, true);
		break;
	case BBIC_STOP_CMD_FAC:
		if (get_bbic_pwr_status() == false)
			return 0;
		ret = pwr_gpio_ctrl(false, true);
		break;
	case BBIC_SET_BOOT_MODE_CMD_FAC:
		ret = set_boot_mode(temp_data);
		break;
	default:
		hwlog_info("%s invalid cmd:%d\n", __func__, cmd);
		break;
	}
	return ret;
}

static int32_t bbic_rel_cmd_proc(unsigned int cmd, int temp_data)
{
	int32_t ret = -1;

	hwlog_info("%s ,temp_data:%d\n", __func__, temp_data);
	switch (cmd) {
	case BBIC_START_CMD_RELEASE:
		if (get_bbic_pwr_status() == true)
			return 0;
		ret = pwr_gpio_ctrl(true, true);
		break;
	case BBIC_STOP_CMD_RELEASE:
		if (get_bbic_pwr_status() == false)
			return 0;
		ret = pwr_gpio_ctrl(false, true);
		break;
	case BBIC_SET_BOOT_MODE_CMD_RELEASE:
		ret = set_boot_mode(temp_data);
		break;
	default:
		hwlog_info("%s invalid cmd:%d\n", __func__, cmd);
		break;
	}
	return ret;
}

static int32_t bbic_sim_switch_cmd_proc(unsigned int cmd, int sim_state, int sim_idx)
{
	int32_t ret = -1;

	if ((sim_idx != SIM0_IDX) && (sim_idx != SIM1_IDX)) {
		hwlog_err("%s invalid sim idx:%d\n", __func__, sim_idx);
		return -1;
	}
	if (sim_state == BBIC_SET_SIM_SOC_MODMEM) {
		ret = hw_set_sim_switch_bbic_to_modem(sim_idx);
	} else if (sim_state == BBIC_SET_SIM_BBIC) {
		ret = hw_set_sim_switch_modem_to_bbic(sim_idx);
	} else {
		hwlog_err("%s invalid sim state:%d\n", __func__, sim_state);
		return -1;
	}
	hwlog_info("%s sim switch ret:%d\n", __func__, ret);
	return ret;
}

static int32_t bbic_sim_vcc_cmd_proc(unsigned int cmd, int vcc_idx)
{
	int32_t ret = hw_sim_vcc_switch_proc(vcc_idx);
	return ret;
}

static int32_t bbic_sleep_gpio_switch_proc(int status)
{
	int32_t ret = hw_bbic_sleep_switch(status);
	return ret;
}

static int32_t bbic_sleep_gpio_status_query_proc(void)
{
	int32_t ret = hw_bbic_sleep_gpio_status_query();
	return ret;
}

static int32_t bbic_fac_ioctl_proc(unsigned int cmd, int temp_data)
{
	int32_t ret = -1;

	switch (cmd) {
	case BBIC_START_CMD_FAC:
	case BBIC_STOP_CMD_FAC:
		ret = bbic_fac_cmd_proc(cmd, 0);
		break;
	case BBIC_SET_BOOT_MODE_CMD_FAC:
		ret = bbic_fac_cmd_proc(cmd, temp_data);
		break;
	case BBIC_SET_SIM0_SWITCH_CMD_FAC:
		ret = bbic_sim_switch_cmd_proc(cmd, temp_data, SIM0_IDX);
		break;
	case BBIC_SET_SIM1_SWITCH_CMD_FAC:
		ret = bbic_sim_switch_cmd_proc(cmd, temp_data, SIM1_IDX);
		break;
	case BBIC_SET_SIM_VCC_CMD_FAC:
		ret = bbic_sim_vcc_cmd_proc(cmd, temp_data);
		break;
	case BBIC_SET_SLEEP_GPIO_SWITCH_CMD_FAC:
		ret = bbic_sleep_gpio_switch_proc(temp_data);
		break;
	case BBIC_SET_SLEEP_GPIO_QUERY_CMD_FAC:
		ret = bbic_sleep_gpio_status_query_proc();
		if ((ret != BBIC_SLEEP_GPIO_ALL_LOW_STATUS) && (ret != BBIC_SLEEP_GPIO_ALL_HIGH_STATUS))
			return ret;
		break;
	default:
		hwlog_info("%s invalid cmd\n", __func__);
		break;
	}
	return ret;
}

static int32_t bbic_release_version_ioctl_proc(unsigned int cmd, int temp_data)
{
	int32_t ret = -1;

	switch (cmd) {
	case BBIC_START_CMD_RELEASE:
	case BBIC_STOP_CMD_RELEASE:
		ret = bbic_rel_cmd_proc(cmd, 0);
		break;
	case BBIC_SET_BOOT_MODE_CMD_RELEASE:
		ret = bbic_rel_cmd_proc(cmd, temp_data);
		break;
	case BBIC_SET_SIM0_SWITCH_CMD_RELEASE:
		ret = bbic_sim_switch_cmd_proc(cmd, temp_data, SIM0_IDX);
		break;
	case BBIC_SET_SIM1_SWITCH_CMD_RELEASE:
		ret = bbic_sim_switch_cmd_proc(cmd, temp_data, SIM1_IDX);
		break;
	case BBIC_SET_SIM_VCC_CMD_RELEASE:
		ret = bbic_sim_vcc_cmd_proc(cmd, temp_data);
		break;
	default:
		hwlog_info("%s invalid cmd\n", __func__);
		break;
	}
	return ret;
}

static long vcom_ioctl(struct file *filp, unsigned int cmd, unsigned long data)
{
	int32_t ret = -1;

	hwlog_info("%s enter, cmd:%d, cmd type:%c\n", __func__, cmd, _IOC_TYPE(cmd));
	switch (_IOC_TYPE(cmd)) {
	case BBIC_IOCTL_MAGIC:
		if ((cmd == BBIC_START_CMD) || (cmd == BBIC_STOP_CMD)) {
			ret = update_firmware(cmd, 0);
		} else if (cmd == BBIC_SET_BOOT_MODE_CMD) {
			ret = update_firmware(cmd, data);
		} else {
			hwlog_info("%s invalid cmd\n", __func__);
		}
		break;
	case BBIC_IOCTL_MAGIC_FAC:
		ret = bbic_fac_ioctl_proc(cmd, data);
		break;
	case BBIC_IOCTL_MAGIC_RELEASE:
		ret = bbic_release_version_ioctl_proc(cmd, data);
		break;
	default:
		hwlog_info("%s invalid cmd type\n", __func__);
		break;
	}
	return ret;
}

static int vcom_open(struct inode *inode, struct file *filp)
{
	struct vcom_map_s *map = NULL;

	hwlog_info("%s enter\n", __func__);

	if (inode == NULL)
		return -1;
	if (filp == NULL)
		return -1;
	map = get_map(inode);
	hwlog_info("%s done \n", __func__);
	return 0;
}

static int vcom_release(struct inode *inode, struct file *filp)
{
	struct vcom_map_s *map = NULL;
	hwlog_info("%s enter\n", __func__);
	if (filp == NULL)
		return -1;
	map = get_map(inode);
	hwlog_info("%s done\n", __func__);
	return 0;
}

static struct file_operations vcom_dev_fops = {
	.owner = THIS_MODULE,
	.open = vcom_open,
	.unlocked_ioctl = vcom_ioctl,
	.compat_ioctl = vcom_ioctl,
	.release = vcom_release,
};

static struct file_operations* vcom_ops_setup(unsigned int idx)
{
	switch (com_name_id_map[idx].flag) {
	case VCOM_SYNCCOM:
		return &vcom_dev_fops;
	default:
		hwlog_info("%s, vcom_ops_setup: idx = %x, flag = %x\n", __func__, idx, com_name_id_map[idx].flag);
		return NULL;
	}
}

int hw_stavcom_init(void)
{
	int ret;
	unsigned int i;
	dev_t devt;
	unsigned int size;
	struct device* dev = NULL;

	size = sizeof(com_name_id_map) / sizeof(*com_name_id_map);
	g_vcom_map = kvzalloc(size * sizeof(*g_vcom_map), GFP_KERNEL);
	if (g_vcom_map == NULL)
		return -1;
	ret = alloc_chrdev_region(&devt, BASE_MONITOR_START,
		sizeof(com_name_id_map) / sizeof(*com_name_id_map), "stavcom");
	if (ret) {
		goto err_stavcom_init;
	}
	for (i = 0; i < size; i++) {
		g_vcom_map[i].name = com_name_id_map[i].name;
		g_vcom_map[i].iccid = i;
		g_vcom_map[i].format = com_name_id_map[i].format;
		if (g_vcom_map[i].format != VCOM_STREAM) {
			goto err_stavcom_init;
		}
		g_vcom_map[i].ops = vcom_ops_setup(i);
		g_vcom_map[i].flag = com_name_id_map[i].flag;
		if (VCOM_TYPE_VIRTUAL != com_name_id_map[i].type) {
			goto err_stavcom_init;
		}
		g_vcom_map[i].devt = devt + i;
		cdev_init(&g_vcom_map[i].cdev, g_vcom_map[i].ops);
		ret = cdev_add(&g_vcom_map[i].cdev, g_vcom_map[i].devt, 1);
		if (ret) {
			goto err_stavcom_init;
		}
		g_vcom_map[i].class = class_create(THIS_MODULE, g_vcom_map[i].name);
		dev = device_create(g_vcom_map[i].class, NULL, g_vcom_map[i].devt, NULL, g_vcom_map[i].name);
		if (IS_ERR(dev)) {
			goto err_stavcom_init;
		}
	}
	return 0;

err_stavcom_init:
	kfree(g_vcom_map);
	g_vcom_map = NULL;
	return -1;
}

void hw_stavcom_exit(void)
{
	int i;
	unsigned int com_size = sizeof(com_name_id_map) / sizeof(*com_name_id_map);

	hwlog_info("%s enter com_size:%d\n", __func__, com_size);
	for (i = 0; i < com_size; i++) {
		cdev_del(&(g_vcom_map[i].cdev));
		if (g_vcom_map) {
			kfree(g_vcom_map);
			g_vcom_map = NULL;
		}
		unregister_chrdev_region(g_vcom_map[i].devt, 1);
	}
	hwlog_info("%s end\n", __func__);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

