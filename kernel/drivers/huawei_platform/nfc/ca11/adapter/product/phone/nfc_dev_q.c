

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>

#include <linux/regulator/consumer.h>

#include <linux/io.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/spinlock_types.h>
#include <linux/kthread.h>
#include <linux/pwm.h>

#include "oal_os.h"
#include "board_nfc.h"
#include "nfc_dev_q.h"
#include "oal_plat_op.h"
#include "priv_nfc_driver.h"
#include "nfc_dev_attr_q.h"
#ifdef PLATFORM_DEBUG_ENABLE
#include "nfc_dev_test_q.h"
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#ifdef NFC_PLATFORM_Q
#include <hwmanufac/dev_detect/dev_detect.h>
#else
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#endif

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#include <linux/errno.h>
#endif

#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *nfc_dclient = NULL;
#endif

#define CAXX_MAGIC         0xE9
/*
 * caxx power control via ioctl
 * CAXX_SET_PWR(0): power off
 * CAXX_SET_PWR(1): power on
 * CAXX_SET_PWR(>1): power on with firmware download enabled
 */
#define CAXX_SET_PWR _IOW(CAXX_MAGIC, 0x01, unsigned int)
#define ESE_SET_PWR _IOW(CAXX_MAGIC, 0x02, unsigned int)
#define ESE_GET_PWR _IOR(CAXX_MAGIC, 0x03, unsigned int)
#define GET_CHIP_TYPE _IOW(CAXX_MAGIC, 0x04, unsigned int)

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_nfc = {
	.name = "dsm_nfc",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024, // 1k buffer
};
#endif

#ifdef CONFIG_CA11_NFC
int board_get_chip_type(void);
#endif

/* FUNCTION: caxx_dev_read
 * DESCRIPTION: read i2c data
 * Parameters
 * struct file *filp:device structure
 * char __user *buf:return to user buffer
 * size_t count:read data count
 * loff_t *offset:offset
 * RETURN VALUE
 * ssize_t: result
 */
static ssize_t caxx_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	struct caxx_dev *plat_dev = NULL;
	char tmp[MAX_RECV_DATA_LEN] = { 0 };

	unused(offset);

	plat_dev = filp->private_data;

	if (plat_dev == NULL) {
		PS_PRINT_ERR("plat_dev is err.\n");
		return -EFAULT;
	}

	if (filp->f_flags & O_NONBLOCK) {
		PS_PRINT_WARNING("read will block anyway\n");
	}

	ret = nfc_i2c_read(tmp, count, get_timeout());
	if ((ret < 0) || (ret > count)) {
		PS_PRINT_WARNING("i2c read data len: %d, expect:%d\n", ret, count);
		return (size_t)ret;
	}

	if (copy_to_user(buf, tmp, ret)) {
		PS_PRINT_ERR("failed to copy to user space\n");
		return -EFAULT;
	}
	return (size_t)ret;
}

/* FUNCTION: caxx_dev_write
 * DESCRIPTION: write i2c data
 * Parameters
 * struct file *filp:device structure
 * char __user *buf:user buffer to write
 * size_t count:write data count
 * loff_t *offset:offset
 * RETURN VALUE
 * ssize_t: result
 */
static ssize_t caxx_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	struct caxx_dev *plat_dev = NULL;
	char tmp[MAX_SEND_DATA_LEN] = { 0 };

	unused(offset);

	plat_dev = filp->private_data;

	if (plat_dev == NULL) {
		PS_PRINT_ERR("plat_dev is err.\n");
		return -EFAULT;
	}

	if (count > MAX_SEND_DATA_LEN) {
		PS_PRINT_ERR("write len:%d, change to %d\n", count, MAX_SEND_DATA_LEN);
		count = MAX_SEND_DATA_LEN;
	}

	if (copy_from_user(tmp, buf, count)) {
		PS_PRINT_ERR("copy_from_user fail\n");
		return -EFAULT;
	}

	ret = nfc_i2c_write(tmp, count);
	if (ret != count) {
		return -EIO;
	}
	return ret;
}

/* FUNCTION: caxx_dev_open
 * DESCRIPTION: caxx_dev_open, used by user space to enable caxx
 * Parameters
 * struct inode *inode:device inode
 * struct file *filp:device file
 * RETURN VALUE
 * int: result
 */
static int caxx_dev_open(struct inode *inode, struct file *filp)
{
	struct caxx_dev *caxx_dev = NULL;

	caxx_dev = container_of(filp->private_data,
						struct caxx_dev,
						caxx_device);
	filp->private_data = caxx_dev;
	PS_PRINT_INFO("imajor:%d, iminor:%d\n", imajor(inode), iminor(inode));
	PS_PRINT_INFO("caxx nfc open\n");
	return 0;
}

static int caxx_dev_close(struct inode *inode, struct file *filp)
{
	unused(inode);
	unused(filp);
	PS_PRINT_INFO("caxx nfc close\n");
	return 0;
}

/* FUNCTION: caxx_dev_ioctl
 * DESCRIPTION: caxx_dev_ioctl, used by user space
 * Parameters
 * struct file *filp:device file
 * unsigned int cmd:command
 * unsigned long arg:parameters
 * RETURN VALUE
 * long: result
 */
static long caxx_dev_ioctl(struct file *filp,
							unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unused(filp);

	PS_PRINT_INFO("caxx_dev_ioctl cmd 0x%x\n", cmd);
	if (cmd == 0)
		return -EINVAL;

	if (cmd == GET_CHIP_TYPE) {
#ifdef CONFIG_CA11_NFC
		ret = board_get_chip_type();
#else
		ret = NFCTYPE_CA01;
#endif
		return ret;
	}

	if (arg == 0) {
		/* power off */
		PS_PRINT_INFO("nfc ioctl poweroff\n");
		board_pwn_ctrl(NFC_DISABLE);
	} else if (arg == 1) {
		/* power on */
		PS_PRINT_INFO("nfc ioctl poweron\n");
		ret = nfc_chip_power_on();
	} else {
		PS_PRINT_ERR("bad arg %lu\n", arg);
		return -EINVAL;
	}

	return ret;
}

static const struct file_operations caxx_dev_fops = {
	.owner	= THIS_MODULE,
	.llseek	= no_llseek,
	.read	= caxx_dev_read,
	.write	= caxx_dev_write,
	.open	= caxx_dev_open,
	.release = caxx_dev_close,
	.unlocked_ioctl	= caxx_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = caxx_dev_ioctl,
#endif
};

static inline struct caxx_dev *caxx_device_create(struct i2c_client *client)
{
	int ret = 0;
	struct caxx_dev *caxx_dev = kzalloc(sizeof(*caxx_dev), GFP_KERNEL);
	if (caxx_dev == NULL) {
		dev_err(&client->dev, "failed to allocate memory for module data\n");
		return NULL;
	}
	caxx_dev->caxx_device.minor = MISC_DYNAMIC_MINOR;
	caxx_dev->caxx_device.name = "nfc_dg";
	caxx_dev->caxx_device.fops = &caxx_dev_fops;

	/* sim_select = 1,UICC select */
	caxx_dev->sim_switch = CARD1_SELECT;
	caxx_dev->sim_status = CARD_UNKNOWN;
	caxx_dev->enable_status = ENABLE_START;

	ret = misc_register(&caxx_dev->caxx_device);
	if (ret != 0) {
		dev_err(&client->dev, "%s: misc_register err %d\n", __func__, ret);
		kfree(caxx_dev);
		return NULL;
	}
	return caxx_dev;
}

/* FUNCTION: caxx_probe
 * DESCRIPTION: caxx_probe
 * Parameters
 * struct i2c_client *client:i2c client data
 * const struct i2c_device_id *id:i2c device id
 * RETURN VALUE
 * int: result
 */
static int caxx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct caxx_dev *caxx_dev = NULL;
	unused(id);

	PS_PRINT_INFO("caxx_probe begin!\n");
#ifdef CONFIG_HUAWEI_DSM
	if (!nfc_dclient)
		nfc_dclient = dsm_register_client(&dsm_nfc);
#endif

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		PS_PRINT_ERR("need I2C_FUNC_I2C\n");
		return  -ENODEV;
	}

	/* create interface node */
	if (create_attribute_node(client) < 0) {
		PS_PRINT_ERR("Failed to create_attribute_node\n");
		return -ENODEV;
	}

#ifdef PLATFORM_DEBUG_ENABLE
	if (create_debug_node(client) < 0) {
		PS_PRINT_ERR("Failed to create_debug_node\n");
		goto err_platform_data;
	}
#endif

	caxx_dev = caxx_device_create(client);
	if (caxx_dev == NULL) {
		goto err_platform_debug;
	}

	ret = board_init((void *)client);
	if (ret != BOARD_SUCC) {
		goto err_board_init;
	}

	caxx_attr_init(client);
	i2c_set_clientdata(client, caxx_dev);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_NFC);
#endif
	PS_PRINT_INFO("caxx_probe sucess!\n");
	return 0;

err_board_init:
	misc_deregister(&caxx_dev->caxx_device);
	kfree(caxx_dev);
err_platform_debug:
#ifdef PLATFORM_DEBUG_ENABLE
	remove_debug_sysfs(&client->dev);
err_platform_data:
#endif
	remove_attribute_sysfs(&client->dev);
	dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	return 0;
}

/* FUNCTION: caxx_remove
 * DESCRIPTION: caxx_remove
 * Parameters
 * struct i2c_client *client:i2c client data
 * RETURN VALUE
 * int: result
 */
static int caxx_remove(struct i2c_client *client)
{
	struct caxx_dev *caxx_dev = NULL;

	PS_PRINT_INFO("caxx_remove begin!\n");
	dev_info(&client->dev, "%s ++\n", __func__);

	caxx_dev = i2c_get_clientdata(client);
	misc_deregister(&caxx_dev->caxx_device);
	board_deinit();
	remove_attribute_sysfs(&client->dev);
#ifdef PLATFORM_DEBUG_ENABLE
	remove_debug_sysfs(&client->dev);
#endif
	caxx_attr_deinit();
	nfc_driver_deinit();
	kfree(caxx_dev);
	return 0;
}

#ifdef CONFIG_CA11_NFC
static struct of_device_id caxx_match_table[] = {
	{ .compatible = "hisilicon,ca11", },
	{ },
};
#else
static struct of_device_id caxx_match_table[] = {
	{ .compatible = "hisilicon,fanli", },
	{ },
};
#endif

static struct i2c_driver caxx_driver = {
	.probe		= caxx_probe,
	.remove		= caxx_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "hisi_nfc",
		.of_match_table	= caxx_match_table,
	},
};

static int __init caxx_dev_init(void)
{
	PS_PRINT_INFO("caxx dev init\n");
	return i2c_add_driver(&caxx_driver);
}
module_init(caxx_dev_init);

static void __exit caxx_dev_exit(void)
{
	PS_PRINT_INFO("caxx dev exit\n");
	i2c_del_driver(&caxx_driver);
}
module_exit(caxx_dev_exit);

MODULE_AUTHOR("HUAWEI");
MODULE_DESCRIPTION("NFC caxx driver");
MODULE_LICENSE("GPL");