/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: This module is to support wifi multilink ioctl.
 * Create: 2023-03-31
 */

#include "wifi_clock_dev.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <pmic_interface.h>
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>

#define WIFI_CLOCK_DEV_NAME        "wifi_clock_dev"
#define WIFI_CLOCK_CLASS_NAME      "wifi_clock_class"

#define CLOCK_DEV_CNT           1
#define CLOCK_ERR_COMMON       (-1)
#define CLOCK_SUCC              0
#define WIFI_CLOCK_SQUARE_WAVE  0
#define WIFI_CLOCK_SIN_WAVE     1
#define WIFI_CLOCK_DISABLE      0
#define WIFI_CLOCK_ENABLE       1
#define TINY_BUFF_SIZE          64

typedef enum {
	CMD_GET_SQUARE_WAVE = 306,
	CMD_SET_SQUARE_WAVE = 307,
	CMD_MAX,
} clock_iotcl_cmd;

typedef enum {
	CLOCK_DEV_NOT_OPENED = 0,
	CLOCK_DEV_OPENED,
	CLOCK_DEV_READ,
	CLOCK_DEV_WRITE,
	CLOCK_DEV_IOTCRL,
} clock_dev_ops_stat;

typedef struct {
	char param_buf[TINY_BUFF_SIZE];
	unsigned int param_size;
} wifi_clock_iotcl_para;

static wifi_clock_device g_wifi_clock_dev;
static uint32_t g_dev_status = CLOCK_DEV_NOT_OPENED;
static int g_enable = 0;

static int wifi_clock_open(struct inode *inode, struct file *fp)
{
	if (g_dev_status != CLOCK_DEV_NOT_OPENED) {
		hwlog_err("open fail, dev has been opened, stat=%d\n", g_dev_status);
		return CLOCK_ERR_COMMON;
	}
	g_dev_status = CLOCK_DEV_OPENED;
	return CLOCK_SUCC;
}

static int wifi_clock_release(struct inode *inode, struct file *fp)
{
	if (g_dev_status == CLOCK_DEV_NOT_OPENED) {
		hwlog_err("close fail, dev has been closed\n");
		return CLOCK_ERR_COMMON;
	}
	g_dev_status = CLOCK_DEV_NOT_OPENED;
	return CLOCK_SUCC;
}

static int wifi_clock_read_wave_state(void)
{
	int ret = 0;
	if (g_enable == 0)
		return ret;

	ret = pmic_read_reg(PMIC_NP_CLK_WIFI_CTRL_ADDR(0));
	hwlog_info("%s: read from wave is: %d", __func__, ret);
	return ret;
}

static void wifi_enable_square(void)
{
	if (g_enable == 0 || wifi_clock_read_wave_state() == WIFI_CLOCK_SQUARE_WAVE)
		return;

	hwlog_info("enable the square wave\n");
	pmic_write_reg(PMIC_NP_CLK_WIFI_CTRL_ADDR(0), WIFI_CLOCK_SQUARE_WAVE);
}

static int wifi_clock_set_wave_state(int enable)
{
	if (g_enable == 0)
		return 0;

	hwlog_info("enable %d write to state", enable);
	if (enable == WIFI_CLOCK_ENABLE)
		wifi_enable_square();

	return 0;
}

static int wifi_clock_ioctl_handle(unsigned int cmd, void __user *argp)
{
	wifi_clock_iotcl_para cmd_info;
	int ret = CLOCK_ERR_COMMON;
	int enable = 0;

	if (copy_from_user(&cmd_info, argp, sizeof(cmd_info))) {
		hwlog_err("copy_from_user error\n");
		return ret;
	}
	hwlog_info("cmd ioctl: %u", cmd);
	if (cmd == CMD_GET_SQUARE_WAVE) {
		ret = wifi_clock_read_wave_state() ? WIFI_CLOCK_DISABLE : WIFI_CLOCK_ENABLE;
	} else if (cmd == CMD_SET_SQUARE_WAVE) {
		enable = (int)cmd_info.param_buf[0];
		ret = wifi_clock_set_wave_state(enable);
	}

	return ret;
}

/* 下发控制指令 */
static long wifi_clock_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int ret = CLOCK_ERR_COMMON;
	void __user *argp = (void __user *)arg;
	if (g_dev_status != CLOCK_DEV_OPENED || g_enable == 0) {
		hwlog_err("dev not open or being operated, stat=%d\n", g_dev_status);
		return CLOCK_ERR_COMMON;
	}

	if (cmd < CMD_GET_SQUARE_WAVE || cmd >= CMD_MAX) {
		hwlog_err("unknown ioctl: %u", cmd);
		return CLOCK_ERR_COMMON;
	}
	g_dev_status = CLOCK_DEV_IOTCRL;
	ret = wifi_clock_ioctl_handle(cmd, argp);
	g_dev_status = CLOCK_DEV_OPENED;
	return ret;
}

int32_t get_board_dts_node(struct device_node **np, const char *node_prop)
{
	if (np == NULL || node_prop == NULL) {
		hwlog_err("func has NULL input param!!!, np=%p, node_prop=%p\n", np, node_prop);
		return CLOCK_ERR_COMMON;
	}

	*np = of_find_compatible_node(NULL, NULL, node_prop);
	if (*np == NULL) {
		hwlog_info("No compatible node %s found.\n", node_prop);
		return CLOCK_ERR_COMMON;
	}

	return CLOCK_SUCC;
}

static int32_t board_get_wifi_clock_enable(void)
{
	int32_t ret;
	struct device_node *np = NULL;

	ret = get_board_dts_node(&np, DTS_NODE_HISI_MPXX);
	if (ret != CLOCK_SUCC) {
		hwlog_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_MPXX);
		return CLOCK_ERR_COMMON;
	}
	ret = of_property_read_bool(np, DTS_PROP_ENABLE_SQUARE_WAVE);
	if (ret) {
		hwlog_info("enable square wave\n");
		g_enable = 1;
	} else {
		hwlog_info("no need enable square wave\n");
		g_enable = 0;
	}

	return CLOCK_SUCC;
}

static const struct file_operations wifi_clock_fops = {
	.owner = THIS_MODULE,
	.open = wifi_clock_open,
	.release = wifi_clock_release,
	.unlocked_ioctl = wifi_clock_ioctl,
};

static int __init wifi_clock_dev_init(void)
{
	int ret;
	hwlog_info("wifi_clock_dev_init init enter\n");
	ret = board_get_wifi_clock_enable();
	if (g_enable == 0) {
		hwlog_info("wifi_clock_dev_init init not enable\n");
		return CLOCK_SUCC;
	}

	/* 注册设备号 */
	if (g_wifi_clock_dev.major) {
		g_wifi_clock_dev.devid = MKDEV(g_wifi_clock_dev.major, 0);
		ret = register_chrdev_region(g_wifi_clock_dev.devid, CLOCK_DEV_CNT, WIFI_CLOCK_DEV_NAME);
	} else {
		ret = alloc_chrdev_region(&g_wifi_clock_dev.devid, 0, CLOCK_DEV_CNT, WIFI_CLOCK_DEV_NAME);
	}
	if (ret) {
		hwlog_err("alloc_chrdev_region err(%d)!\n", g_wifi_clock_dev.devid);
		goto cdev_err;
	}

	/* 注册字符设备 */
	g_wifi_clock_dev.cdev.owner = THIS_MODULE;
	cdev_init(&g_wifi_clock_dev.cdev, &wifi_clock_fops);
	ret = cdev_add(&g_wifi_clock_dev.cdev, g_wifi_clock_dev.devid, CLOCK_DEV_CNT);
	if (ret) {
		hwlog_err("add cdev fail!\n");
		goto cdev_add_err;
	}

	/* 创建设备节点 */
	g_wifi_clock_dev.class = class_create(THIS_MODULE, WIFI_CLOCK_DEV_NAME);
	if (IS_ERR(g_wifi_clock_dev.class)) {
		hwlog_err("creat class fail(%ld)!\n", PTR_ERR(g_wifi_clock_dev.class));
		goto class_create_err;
	}
	g_wifi_clock_dev.dev = device_create(g_wifi_clock_dev.class, NULL, g_wifi_clock_dev.devid, NULL, WIFI_CLOCK_DEV_NAME);
	if (IS_ERR(g_wifi_clock_dev.dev)) {
		hwlog_err("creat dev fail(%ld)!\n", PTR_ERR(g_wifi_clock_dev.dev));
		goto dev_create_err;
	}
	return CLOCK_SUCC;
dev_create_err:
	class_destroy(g_wifi_clock_dev.class);
class_create_err:
	cdev_del(&g_wifi_clock_dev.cdev);
cdev_add_err:
	unregister_chrdev_region(g_wifi_clock_dev.devid, CLOCK_DEV_CNT);
cdev_err:
	return CLOCK_ERR_COMMON;
}

static void __exit wifi_clock_dev_exit(void)
{
	hwlog_info("wifi_clock_dev init exit\n");
	device_destroy(g_wifi_clock_dev.class, g_wifi_clock_dev.devid);
	class_destroy(g_wifi_clock_dev.class);
	cdev_del(&g_wifi_clock_dev.cdev);
	unregister_chrdev_region(g_wifi_clock_dev.devid, CLOCK_DEV_CNT);
	return;
}

module_init(wifi_clock_dev_init);
module_exit(wifi_clock_dev_exit);
MODULE_DESCRIPTION("HW Wifi Clock");
