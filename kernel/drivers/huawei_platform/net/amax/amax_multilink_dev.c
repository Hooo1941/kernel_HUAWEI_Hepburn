/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: This module is to support wifi multilink ioctl.
 * Create: 2023-03-31
 */

#include "amax_multilink_dev.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "amax_multilink_tool.h"
#include "amax_multilink_main.h"


#define AMAX_DEV_NAME        "amax_dev"
#define AMAX_CLASS_NAME      "amax_class"
#define AMAX_DEV_CNT         1

#define AMAX_ERR_COMMON (-1)
#define AMAX_SUCC 0

#define THROUGHPUT_IDX       0
#define LATENCY_IDX          1
#define PACKETNUMBER_IDX     2

#define AMAX_TINY_BUFF_SIZE 64
#define DEVICE_PIPE_BUFSIZE 1400
#define MAX_MULTILINK_PARAM 3
#define AMAX_MODULE_INIT    3
#define AMAX_MODULE_EXIT    4

typedef enum {
	CMD_SET_MULTILINK = 301,
	CMD_SET_MULTILINK_PARAM = 302,
	CMD_DEVICE_PIPE_DATA_FRMAE = 308,
	CMD_MULTILINK_MAX,
} amax_iotcl_cmd;

typedef enum {
	AMAX_DEV_NOT_OPENED = 0,
	AMAX_DEV_OPENED,
	AMAX_DEV_READ,
	AMAX_DEV_WRITE,
	AMAX_DEV_IOTCRL,
} amax_dev_ops_stat;

typedef struct {
	char param_buf[AMAX_TINY_BUFF_SIZE];
	unsigned int param_size;
} amax_ioctl_para;

typedef struct {
	char param_buf[DEVICE_PIPE_BUFSIZE];
	unsigned int param_size;
} dps_ioctl_para;

static amax_device g_amax_dev;
static uint32_t g_dev_status = AMAX_DEV_NOT_OPENED;

static amax_stat_ctl_hook_func g_state_ctl_hook = NULL;
static amax_thold_set_hook_func g_thold_set_hook = NULL;

static amax_module_init_hook_func g_mod_init_hook = NULL;
static amax_module_exit_hook_func g_mod_exit_hook = NULL;

void amax_ioctl_reg(amax_stat_ctl_hook_func ctl_func, amax_thold_set_hook_func set_func)
{
	amax_print(PRINT_INFO, "register amax ioctl\n");
	g_state_ctl_hook = ctl_func;
	g_thold_set_hook = set_func;
}
EXPORT_SYMBOL(amax_ioctl_reg);

void amax_ioctl_unreg(void)
{
	amax_print(PRINT_INFO, "unregister amax ioctl\n");
	g_state_ctl_hook = NULL;
	g_thold_set_hook = NULL;
}
EXPORT_SYMBOL(amax_ioctl_unreg);

static dps_msg_hook_func g_dps_msg_hook = NULL;

void dps_ioctl_reg(dps_msg_hook_func msg_func)
{
	amax_print(PRINT_INFO, "[DPS] register dps ioctl\n");
	g_dps_msg_hook = msg_func;
}
EXPORT_SYMBOL(dps_ioctl_reg);

void dps_ioctl_unreg(void)
{
	amax_print(PRINT_INFO, "[DPS] unregister dps ioctl\n");
	g_dps_msg_hook = NULL;
}
EXPORT_SYMBOL(dps_ioctl_unreg);

void amax_mod_reg(amax_module_init_hook_func init_func, amax_module_exit_hook_func exit_func)
{
	amax_print(PRINT_INFO, "register amax mod\n");
	g_mod_init_hook = init_func;
	g_mod_exit_hook = exit_func;
}
EXPORT_SYMBOL(amax_mod_reg);

void amax_mod_unreg(void)
{
	amax_print(PRINT_INFO, "unregister amax mod\n");
	g_mod_init_hook = NULL;
	g_mod_exit_hook = NULL;
}
EXPORT_SYMBOL(amax_mod_unreg);

static int amax_open(struct inode *inode, struct file *fp)
{
	if (g_dev_status != AMAX_DEV_NOT_OPENED) {
		amax_print(PRINT_ERROR, "open fail, dev has been opened, stat=%d\n", g_dev_status);
		return AMAX_ERR_COMMON;
	}
	g_dev_status = AMAX_DEV_OPENED;
	return AMAX_SUCC;
}

static int amax_release(struct inode *inode, struct file *fp)
{
	if (g_dev_status == AMAX_DEV_NOT_OPENED) {
		amax_print(PRINT_ERROR, "close fail, dev has been closed\n");
		return AMAX_ERR_COMMON;
	}
	g_dev_status = AMAX_DEV_NOT_OPENED;
	return AMAX_SUCC;
}

static int amax_set_multilink(char *param_buf, unsigned int param_size)
{
	int para;
	if (param_size > AMAX_TINY_BUFF_SIZE / sizeof(int)) {
		amax_print(PRINT_ERROR, "amax param size:%d\n", param_size);
		return AMAX_ERR_COMMON;
	}

	para = (int)param_buf[0];
	amax_print(PRINT_INFO, "amax para=%d\n", para);
	if (para < AMAX_MODULE_INIT) {
		if (g_state_ctl_hook != NULL)
			return g_state_ctl_hook(para);
		return AMAX_ERR_COMMON;
	} else if (para == AMAX_MODULE_INIT) {
#ifdef CONFIG_HW_WIFI_INSMOD_AMAX
		if (g_mod_init_hook != NULL)
			g_mod_init_hook();
#else
		amax_module_init();
#endif
	} else if (para == AMAX_MODULE_EXIT) {
#ifdef CONFIG_HW_WIFI_INSMOD_AMAX
		if (g_mod_exit_hook != NULL)
			g_mod_exit_hook();
#else
		amax_module_exit();
#endif
	}
	return AMAX_SUCC;
}

static int amax_set_multilink_para(char *param_buf, unsigned int param_size)
{
	int ret = AMAX_ERR_COMMON;
	if (param_size > AMAX_TINY_BUFF_SIZE / sizeof(int) || param_size < MAX_MULTILINK_PARAM) {
		amax_print(PRINT_ERROR, "amax param size:%d\n", param_size);
		return ret;
	}

	if (g_state_ctl_hook != NULL)
		ret = g_thold_set_hook((int)param_buf[THROUGHPUT_IDX], (int)param_buf[LATENCY_IDX], (int)param_buf[PACKETNUMBER_IDX]);

	return ret;
}

static int amax_ioctl_handle(unsigned int cmd, void __user *argp)
{
	amax_ioctl_para cmd_info;
	int ret = AMAX_ERR_COMMON;

	if (copy_from_user(&cmd_info, argp, sizeof(cmd_info))) {
		amax_print(PRINT_ERROR, "amax copy_from_user error\n");
		return ret;
	}
	if (cmd == CMD_SET_MULTILINK) {
		ret = amax_set_multilink(cmd_info.param_buf, cmd_info.param_size);
	} else if (cmd == CMD_SET_MULTILINK_PARAM) {
		ret = amax_set_multilink_para(cmd_info.param_buf, cmd_info.param_size);
	}
	return ret;
}

static int dps_ioctl_handle(unsigned int cmd, void __user *argp)
{
	dps_ioctl_para cmd_info;
	int ret = AMAX_SUCC;

	if (copy_from_user(&cmd_info, argp, sizeof(cmd_info))) {
		amax_print(PRINT_ERROR, "amax copy_from_user error\n");
		return ret;
	}

	printk("[DPS] amax_ioctl_handle CMD_DEVICE_PIPE_DATA_FRMAE");

	if (g_dps_msg_hook != NULL)
		g_dps_msg_hook((uint8_t *)cmd_info.param_buf, cmd_info.param_size);
	return ret;
}

/* 下发控制指令 */
static long amax_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int ret = AMAX_ERR_COMMON;
	void __user *argp = (void __user *)arg;
	amax_print(PRINT_INFO, "cmd ioctl: %u", cmd);
	if (g_dev_status != AMAX_DEV_OPENED) {
		amax_print(PRINT_ERROR, "dev not open or being operated, stat=%d\n", g_dev_status);
		return AMAX_ERR_COMMON;
	}

	if (cmd < CMD_SET_MULTILINK || cmd >= CMD_MULTILINK_MAX) {
		amax_print(PRINT_ERROR, "unknown ioctl: %u", cmd);
		return AMAX_ERR_COMMON;
	}
	g_dev_status = AMAX_DEV_IOTCRL;
	if (cmd == CMD_DEVICE_PIPE_DATA_FRMAE) {
		ret = dps_ioctl_handle(cmd, argp);
	} else {
		ret = amax_ioctl_handle(cmd, argp);
	}
	g_dev_status = AMAX_DEV_OPENED;
	return ret;
}

static const struct file_operations amax_fops = {
	.owner = THIS_MODULE,
	.open = amax_open,
	.release = amax_release,
	.unlocked_ioctl = amax_ioctl,
};

static int __init amax_dev_init(void)
{
	int ret;
	amax_print(PRINT_INFO, "amax_dev init enter\n");
	/* 注册设备号 */
	if (g_amax_dev.major) {
		g_amax_dev.devid = MKDEV(g_amax_dev.major, 0);
		ret = register_chrdev_region(g_amax_dev.devid, AMAX_DEV_CNT, AMAX_DEV_NAME);
	} else {
		ret = alloc_chrdev_region(&g_amax_dev.devid, 0, AMAX_DEV_CNT, AMAX_DEV_NAME);
	}
	if (ret) {
		amax_print(PRINT_ERROR, "alloc_chrdev_region err(%d)!\n", g_amax_dev.devid);
		goto cdev_err;
	}

	/* 注册字符设备 */
	g_amax_dev.cdev.owner = THIS_MODULE;
	cdev_init(&g_amax_dev.cdev, &amax_fops);
	ret = cdev_add(&g_amax_dev.cdev, g_amax_dev.devid, AMAX_DEV_CNT);
	if (ret) {
		amax_print(PRINT_ERROR, "add cdev fail!\n");
		goto cdev_add_err;
	}

	/* 创建设备节点 */
	g_amax_dev.class = class_create(THIS_MODULE, AMAX_CLASS_NAME);
	if (IS_ERR(g_amax_dev.class)) {
		amax_print(PRINT_ERROR, "creat class fail(%ld)!\n", PTR_ERR(g_amax_dev.class));
		goto class_create_err;
	}
	g_amax_dev.dev = device_create(g_amax_dev.class, NULL, g_amax_dev.devid, NULL, AMAX_DEV_NAME);
	if (IS_ERR(g_amax_dev.dev)) {
		amax_print(PRINT_ERROR, "creat dev fail(%ld)!\n", PTR_ERR(g_amax_dev.dev));
		goto dev_create_err;
	}
	return AMAX_SUCC;
dev_create_err:
	class_destroy(g_amax_dev.class);
class_create_err:
	cdev_del(&g_amax_dev.cdev);
cdev_add_err:
	unregister_chrdev_region(g_amax_dev.devid, AMAX_DEV_CNT);
cdev_err:
	return AMAX_ERR_COMMON;
}

static void __exit amax_dev_exit(void)
{
	amax_print(PRINT_INFO, "amax_dev init exit\n");
	device_destroy(g_amax_dev.class, g_amax_dev.devid);
	class_destroy(g_amax_dev.class);
	cdev_del(&g_amax_dev.cdev);
	unregister_chrdev_region(g_amax_dev.devid, AMAX_DEV_CNT);
	return;
}

module_init(amax_dev_init);
module_exit(amax_dev_exit);
MODULE_LICENSE("GPL");
