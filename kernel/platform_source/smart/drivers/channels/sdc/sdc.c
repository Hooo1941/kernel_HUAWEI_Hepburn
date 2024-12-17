#include <linux/delay.h>
#include <linux/device.h>
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
#include <linux/ioctl.h>
#include <asm/page.h>

#include <platform_include/smart/linux/base/ap/protocol.h>
#include <platform_include/smart/linux/base/ap/sdc_sensor_type.h>
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <huawei_platform/inputhub/sensorhub.h>
#include <platform_include/smart/linux/sdc.h>
#include <securec.h>
#include <platform_include/smart/linux/iomcu_status.h>
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <internal_security_interface.h>
#else
#include <iomcu_ddr_map.h>
#endif
static uint8_t sdc_sensor_type_to_tag[SDC_SENSOR_TYPE_MAX];


#define SDCIO                             0xB5
#define SDC_IOCTL_APP_ENABLE              _IOW(SDCIO, 0x01, short)
#define SDC_IOCTL_APP_DISABLE             _IOW(SDCIO, 0x02, short)
#define SDC_IOCTL_APP_GET_OFFSET          _IOR(SDCIO, 0x01, short)

#define BOOT_MODE 0
#define RECOVERY_MODE 1
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
static struct sdc_mem_info g_sdc_mem_info = {0};
#else
#define DDR_SHMEM_SDC_SIZE  RESERVED_SENSORHUB_SDC_SIZE
#define DDR_SHMEM_SDC_BASE  RESERVED_SENSORHUB_SDC_BASE
static struct sdc_mem_info g_sdc_mem_info = { DDR_SHMEM_SDC_BASE, DDR_SHMEM_SDC_SIZE };
#endif

struct sdc_ioctl_para {
	int32_t sdc_type;
	uint32_t mem_size;
	uint64_t offset;
	uint64_t mem_addr;
};

struct sdc_config {
	unsigned int sub_cmd;
	short enable;
	short recovery_mode; /* 0: boot mode, 1: recovery mode */
};

struct sdc_sensor_info_map {
	int tag;
	unsigned int size;
	unsigned long address;
	int enabled;
};

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
struct sdc_sensor_info_map g_sdc_sensor_map[SDC_SENSOR_TYPE_MAX] = {
	{ TAG_ACCEL, 0, 0, 0 },
	{ TAG_MAG, 0, 0, 0 },
	{ TAG_ORIENTATION, 0, 0, 0 },
	{ TAG_GYRO, 0, 0, 0 },
	{ TAG_LINEAR_ACCEL, 0, 0, 0 },
	{ TAG_ROTATION_VECTORS, 0, 0, 0 },
};
#else
struct sdc_sensor_info_map g_sdc_sensor_map[SDC_SENSOR_TYPE_MAX] = {
	{ TAG_ACCEL, DDR_SHMEM_SDC_ACC_SIZE, DDR_SHMEM_SDC_ACC_ADDR_AP, 0 },
	{ TAG_MAG, DDR_SHMEM_SDC_MAG_SIZE, DDR_SHMEM_SDC_MAG_ADDR_AP, 0 },
	{ TAG_ORIENTATION, DDR_SHMEM_SDC_ORIENTATION_SIZE, DDR_SHMEM_SDC_ORIENTATION_ADDR_AP, 0 },
	{ TAG_GYRO, DDR_SHMEM_SDC_GYRO_SIZE, DDR_SHMEM_SDC_GYRO_ADDR_AP, 0 },
	{ TAG_LINEAR_ACCEL, DDR_SHMEM_SDC_LINEAR_ACC_SIZE, DDR_SHMEM_SDC_LINEAR_ACC_ADDR_AP, 0 },
	{ TAG_ROTATION_VECTORS, DDR_SHMEM_SDC_ROTATION_VECTOR_SIZE, DDR_SHMEM_SDC_ROTATION_VECTOR_ADDR_AP, 0 },
};
#endif

struct sensors_cmd_map {
	int sdc_sensor_type;
	int tag;
};

static const struct sensors_cmd_map sensors_cmd_map_tab[] = {
	{ SDC_SENSOR_TYPE_ACCELEROMETER, TAG_ACCEL },
	{ SDC_SENSOR_TYPE_GYROSCOPE, TAG_GYRO },
	{ SDC_SENSOR_TYPE_MAGNETIC_FIELD, TAG_MAG },
	{ SDC_SENSOR_TYPE_LINEAR_ACCELERATION, TAG_LINEAR_ACCEL },
	{ SDC_SENSOR_TYPE_ORIENTATION_FIELD, TAG_ORIENTATION },
	{ SDC_SENSOR_TYPE_ROTATION_VECTOR, TAG_ROTATION_VECTORS },
};

static void init_hash_tables(void)
{
	unsigned int i;

	for (i = 0; i < sizeof(sensors_cmd_map_tab) / sizeof(sensors_cmd_map_tab[0]); ++i)
		sdc_sensor_type_to_tag[sensors_cmd_map_tab[i].sdc_sensor_type] =
			(uint8_t)sensors_cmd_map_tab[i].tag;
}

struct sdc_mem_info* sdc_buf_get(void)
{
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	if (g_sdc_mem_info.phy_addr == 0 || g_sdc_mem_info.size == 0) {
		pr_err("%s: invalid sdc_mem info\n", __func__);
		return NULL;
	}
#endif
	return &g_sdc_mem_info;
}

void sdc_buf_put(struct sdc_mem_info *info)
{
	// currently just empty impl
	return;
}

static struct sdc_sensor_info_map *sdc_get_sensor_map(int tag)
{
	size_t num;
	size_t i;

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	static size_t once_time = 0;
	if (once_time == 0) {
		for (i = 0; i < SDC_SENSOR_TYPE_MAX; i++) {
			g_sdc_sensor_map[i].size = dts_ddr_size_get(DTSI_DDR_SHMEM_SDC_ACC_ADDR_AP + (unsigned int)i);
			g_sdc_sensor_map[i].address = dts_ddr_addr_get(DTSI_DDR_SHMEM_SDC_ACC_ADDR_AP + (unsigned int)i);
			if (g_sdc_sensor_map[i].size == 0 || g_sdc_sensor_map[i].address == 0)
				goto sensor_map_fail;
		}
		once_time = 1;
	}
#endif
	num = sizeof(g_sdc_sensor_map) / sizeof(struct sdc_sensor_info_map);
	for (i = 0; i < num; i++) {
		if (tag == g_sdc_sensor_map[i].tag)
			return &g_sdc_sensor_map[i];
	}
sensor_map_fail:
	return NULL;
}

static int send_sensor_sdc_enable_cmd(int tag, int enable, int mode)
{
	struct sdc_sensor_info_map *sdc_map = NULL;
	struct write_info winfo;
	struct sdc_config config;
	int ret;

	sdc_map = sdc_get_sensor_map(tag);
	if (sdc_map == NULL) {
		pr_err("%s: invalid sensor tag[%d]\n", __func__, tag);
		return -EINVAL;
	}

	config.sub_cmd = SUB_CMD_ENABLE_SDC_REQ;
	config.enable = (enable == 0) ? 0 : 1;
	config.recovery_mode = (short)mode;

	winfo.tag = tag;
	winfo.cmd = CMD_CMN_CONFIG_REQ;
	winfo.wr_buf = &config;
	winfo.wr_len = (unsigned short)sizeof(config);

	pr_info("send sensor[%d] sdc enable[%d] mode[%d]\n", tag, enable, mode);

	ret = write_customize_cmd(&winfo, NULL, true);
	if (ret != 0) {
		pr_err("sensor[%d] sdc enable[%d] fail, ret[%d]\n", tag, enable, ret);
		return -EIO;
	}
	sdc_map->enabled = config.enable;

	return 0;
}

static int sdc_sensor_recover(int tag)
{
	struct sdc_sensor_info_map *sdc_map = NULL;
	int ret;

	sdc_map = sdc_get_sensor_map(tag);
	if (sdc_map == NULL)
		return -EINVAL;

	/* not enabled, skip */
	if (sdc_map->enabled == 0)
		return 0;

	ret = send_sensor_sdc_enable_cmd(tag, 1, RECOVERY_MODE);
	if (ret != 0) {
		pr_err("%s: enable sensor tag[%d] fail\n", __func__, tag);
		return -EIO;
	}

	pr_info("%s, sdc sensor[0x%x] recovery succ\n", __func__, tag);
	return 0;
}

static int send_sensor_cmd(unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	int tag;
	int ret = 0;
	struct sdc_ioctl_para para;
	struct sdc_sensor_info_map *sdc_map = NULL;

	if (copy_from_user(&para, argp, sizeof(struct sdc_ioctl_para)))
		return -EFAULT;

	if (!(para.sdc_type >= SDC_SENSOR_TYPE_ACCELEROMETER &&
		para.sdc_type < SDC_SENSOR_TYPE_MAX)) {
		pr_err("error sdc_type %d in %s\n", para.sdc_type, __func__);
		return -EINVAL;
	}

	tag = sdc_sensor_type_to_tag[para.sdc_type];
	switch (cmd) {
	case SDC_IOCTL_APP_ENABLE:
		ret = send_sensor_sdc_enable_cmd(tag, 1, BOOT_MODE);
		if (ret != 0) {
			pr_err("%s: sensor[%d] enable fail\n", __func__, tag);
			return -EPERM;
		}
		break;
	case SDC_IOCTL_APP_DISABLE:
		ret = send_sensor_sdc_enable_cmd(tag, 0, BOOT_MODE);
		if (ret != 0) {
			pr_err("%s: sensor[%d] disable fail\n", __func__, tag);
			return -EPERM;
		}
		break;
	case SDC_IOCTL_APP_GET_OFFSET:
		sdc_map = sdc_get_sensor_map(tag);
		if (sdc_map == NULL) {
			pr_err("%s: invalid sensor tag[%d]\n", __func__, tag);
			return -EINVAL;
		}
		para.mem_size = sdc_map->size;
		/* Prevent data from being reversed, causing overflow */
		if (sdc_map->address == 0 || g_sdc_mem_info.phy_addr == 0 ||\
		(sdc_map->address < g_sdc_mem_info.phy_addr)) {
			pr_err("%s: invalid sensor addr info \n", __func__);
			return -EINVAL;
		}
		para.offset = sdc_map->address - g_sdc_mem_info.phy_addr;
		if (copy_to_user((void __user *)(uintptr_t)arg, (void *)(uintptr_t)&para,
			sizeof(struct sdc_ioctl_para))) {
			pr_err("%s: sensor tag[%d] copy offset to user failed!\n", __func__, tag);
			return -EINVAL;
		}
		break;
	default:
		pr_err("unknown shb_hal_cmd %d in %s\n", cmd, __func__);
		return -EINVAL;
	}

	return 0;
}

static long shb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return send_sensor_cmd(cmd, arg);
}

static int get_tag_by_offset(unsigned long offset)
{
	unsigned int sensor_num = (unsigned int)ARRAY_SIZE(g_sdc_sensor_map);
	unsigned int i = 0;

	for (i = 0; i < sensor_num; i++) {
		if (g_sdc_mem_info.phy_addr + offset == g_sdc_sensor_map[i].address)
			return g_sdc_sensor_map[i].tag;
	}
	return TAG_END;
}

static int sdc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct sdc_sensor_info_map *sdc_map = NULL;
	unsigned long page = 0;
	unsigned long size;
	unsigned long offset;
	int tag;
	int ret;

	if ((filp == NULL) || (vma == NULL)) {
		pr_err("%s: param NULL\n", __func__);
		return -EFAULT;
	}

	if ((vma->vm_start == 0) || (vma->vm_start >= vma->vm_end)) {
		pr_err("%s: invalid vm address\n", __func__);
		return -EINVAL;
	}
	size = vma->vm_end - vma->vm_start;

	offset = vma->vm_pgoff;
	vma->vm_pgoff = 0;

	// offset is page size num
	offset *= PAGE_SIZE;
	tag = get_tag_by_offset(offset);
	if (tag == TAG_END) {
		pr_err("%s, offset[0x%x] get tag failed!\n", __func__, offset);
		return -EINVAL;
	}

	sdc_map = sdc_get_sensor_map(tag);
	if (sdc_map == NULL) {
		pr_err("%s: not sdc sensor tag[%d]\n", __func__, tag);
		return -EINVAL;
	}

	if (size != sdc_map->size) {
		pr_err("%s: invalid sensor[%u] buffer size[0x%x]\n", __func__, tag, size);
		return -EINVAL;
	}
	page = sdc_map->address;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), size, vma->vm_page_prot);
	if (ret != 0) {
		pr_err("%s: map fail, ret[%d]\n", __func__, ret);
		return -ENOMEM;
	}
	pr_info("%s: sdc map done, offset[0x%x] size[0x%x] prot[0x%x]\n",
		__func__, offset, size, vma->vm_page_prot);

	return 0;
}

/* shb_open: do nothing now */
static int shb_open(struct inode *inode, struct file *file)
{
	pr_info("%s ok\n", __func__);
	return 0;
}

/* shb_release: do nothing now */
static int shb_release(struct inode *inode, struct file *file)
{
	pr_info("%s ok\n", __func__);
	return 0;
}

static void enable_sdc_sensors_when_recovery_iom3(void)
{
	int tag;

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag)
		(void)sdc_sensor_recover(tag);
}

static int sdc_recovery_notifier(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	pr_info("%s %lu +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		enable_sdc_sensors_when_recovery_iom3();
		break;
	default:
		pr_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	pr_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block sdc_recovery_notify = {
	.notifier_call = sdc_recovery_notifier,
	.priority = -1,
};

static const struct file_operations sdc_fops = {
	.owner          = THIS_MODULE,
	.llseek         =   no_llseek,
	.unlocked_ioctl = shb_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = shb_ioctl,
#endif
	.open           =   shb_open,
	.release        =   shb_release,
	.mmap           = sdc_mmap,
};

static struct miscdevice sdc_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "sdc",
	.fops =     &sdc_fops,
};

static int __init sdc_init(void)
{
	int ret;
	if (get_contexthub_dts_status() != 0) {
		pr_err("[%s] contexthub disabled!\n", __func__);
		return -EINVAL;
	}
/* get sensorhub  base  for sdc */
#ifdef  CONFIG_SECURITY_HEADER_FILE_REPLACE
	g_sdc_mem_info.phy_addr = dts_ddr_addr_get(DTSI_RESERVED_SENSORHUB_SDC_BASE);
	g_sdc_mem_info.size = dts_ddr_size_get(DTSI_RESERVED_SENSORHUB_SDC_BASE);
#endif
	init_hash_tables();

	ret = misc_register(&sdc_miscdev);
	if (ret != 0) {
		pr_err("cannot register miscdev err=%d\n", ret);
		return ret;
	}
	register_iom3_recovery_notifier(&sdc_recovery_notify);
	pr_info("%s ok\n", __func__);
	return 0;
}

static void __exit sdc_exit(void)
{
	misc_deregister(&sdc_miscdev);
	pr_info("exit %s\n", __func__);
}

late_initcall_sync(sdc_init);
module_exit(sdc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sdc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
