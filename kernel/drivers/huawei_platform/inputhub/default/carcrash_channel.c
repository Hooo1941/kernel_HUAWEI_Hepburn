#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <huawei_platform/log/hwlog_kernel.h>

#include "contexthub_boot.h"
#include "contexthub_route.h"
#include "big_data_channel.h"

#ifdef CONFIG_HUAWEI_DSM
static ssize_t carcrash_read(struct file *file, char __user *buf, size_t count,
	loff_t *pos)
{
	int ret;
	struct detect_result crash_result = get_crash_result();

	if (!buf)
		return 0;
	ret = (int)copy_to_user(buf, &crash_result, sizeof(crash_result));
	if (ret != 0) {
		hwlog_err("%s copy to user failed\n", __func__);
		return -1;
	}
	return 0;
}

static const struct file_operations chb_fops = {
	.owner = THIS_MODULE,
	.read = carcrash_read,
};

/* miscdevice to carcrash */
static struct miscdevice carcrash_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "carcrashhub",
	.fops = &chb_fops,
};

/*
 * Description:   apply kernel buffer, register carcrashhub_miscdev
 * Return:        result of function, 0 successed, else false
 */
static int __init carcrashhub_init(void)
{
	int ret;

	if (is_sensorhub_disabled())
		return -1;
	if (!get_sensor_mcu_mode()) {
		hwlog_err("mcu boot fail,carcrash_init exit\n");
		return -1;
	}

	ret = misc_register(&carcrash_miscdev);
	if (ret != 0) {
		hwlog_err("cannot register miscdev err=%d\n", ret);
	}

	return ret;
}

/*
 * Description:   release kernel buffer, deregister carcrashhub_miscdev
 */
static void __exit carcrashhub_exit(void)
{
	misc_deregister(&carcrash_miscdev);
	hwlog_info("exit %s\n", __func__);
}
late_initcall_sync(carcrashhub_init);
module_exit(carcrashhub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("CarcrashHub driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
#endif
