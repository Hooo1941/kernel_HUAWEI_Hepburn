#include <linux/bootdevice/bootdevice.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <platform_include/basicplatform/linux/partition/partition_ap_kernel.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <platform_include/basicplatform/linux/ufs/ufs_config_efuse.h>
#include <securec.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_SERVICE
#include <linux/ufs/ufs_virtualiz_service.h>
#endif
#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_AGENT
#include <linux/ufs/ufs_virtualiz_agent.h>
#endif

#define MAX_NAME_LEN 32
#define MAX_REV_LEN 8
#define MANUFACTURE_ID_LENGTH 16

#if defined(CONFIG_SCSI_UFS_VIRTUALIZ_SERVICE) || defined(CONFIG_SCSI_UFS_VIRTUALIZ_AGENT)
#define MAX_NAME_LENTH 16
#define MAX_REV_LENTH 8

#pragma pack(1)
struct bootdevice_virtualiz {
	sector_t size;
	u32 cid[4];
	unsigned int manfid;
	u8 pre_eol_info;
	u8 life_time_est_typ_a;
	u8 life_time_est_typ_b;
	char name[MAX_NAME_LENTH + 1];
	char product_name[MAX_NAME_LENTH + 1];
	char rev[MAX_REV_LENTH + 1];
};
#pragma pack()
#endif

struct __bootdevice {
	enum bootdevice_type type;
	const struct device *dev;
	sector_t size;
	u32 cid[4];
	char product_name[MAX_NAME_LEN + 1];
	u8 pre_eol_info;
	u8 life_time_est_typ_a;
	u8 life_time_est_typ_b;
	unsigned int manfid;
	u8 ptn_index;
	/* UFS and EMMC all */
	struct rpmb_config_info rpmb_config;

	volatile s32 rpmb_done;
	int (*get_rev)(const struct device *, char *);
#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_AGENT
	char name[MAX_NAME_LENTH + 1];
	char rev[MAX_REV_LENTH + 1];
#endif
};

static struct semaphore flash_find_index_sem;

static struct __bootdevice bootdevice;

void set_rpmb_total_blks(u64 total_blks)
{
	bootdevice.rpmb_config.rpmb_total_blks = total_blks;
}

void set_rpmb_blk_size(u8 blk_size)
{
	bootdevice.rpmb_config.rpmb_blk_size = blk_size;
}

void set_rpmb_read_frame_support(u64 read_frame_support)
{
	bootdevice.rpmb_config.rpmb_read_frame_support = read_frame_support;
}

void set_rpmb_write_frame_support(u64 write_frame_support)
{
	bootdevice.rpmb_config.rpmb_write_frame_support = write_frame_support;
}

void set_rpmb_read_align(u8 read_align)
{
	bootdevice.rpmb_config.rpmb_read_align = read_align;
}

void set_rpmb_write_align(u8 write_align)
{
	bootdevice.rpmb_config.rpmb_write_align = write_align;
}

void set_rpmb_region_enable(u8 region_enable)
{
	bootdevice.rpmb_config.rpmb_region_enable = region_enable;
}

void set_rpmb_unit_size(u64 unit_size)
{
	bootdevice.rpmb_config.rpmb_unit_size = unit_size;
}

void set_rpmb_region_size(int region_num, u8 region_size)
{
	if (region_num >= 0 && region_num < MAX_RPMB_REGION_NUM)
		bootdevice.rpmb_config.rpmb_region_size[region_num] =
			region_size;
}

void set_rpmb_config_ready_status(void)
{
	bootdevice.rpmb_done = RPMB_DONE;
}

u64 get_rpmb_total_blks(void)
{
	return bootdevice.rpmb_config.rpmb_total_blks;
}

u8 get_rpmb_blk_size(void)
{
	return bootdevice.rpmb_config.rpmb_blk_size;
}

u64 get_rpmb_read_frame_support(void)
{
	return bootdevice.rpmb_config.rpmb_read_frame_support;
}

u64 get_rpmb_write_frame_support(void)
{
	return bootdevice.rpmb_config.rpmb_write_frame_support;
}

u8 get_rpmb_read_align(void)
{
	return bootdevice.rpmb_config.rpmb_read_align;
}

u8 get_rpmb_write_align(void)
{
	return bootdevice.rpmb_config.rpmb_write_align;
}

u8 get_rpmb_region_enable(void)
{
	return bootdevice.rpmb_config.rpmb_region_enable;
}

u64 get_rpmb_unit_size(void)
{
	return bootdevice.rpmb_config.rpmb_unit_size;
}

u8 get_rpmb_region_size(int region_num)
{
	if (region_num >= 0 && region_num < MAX_RPMB_REGION_NUM)
		return bootdevice.rpmb_config.rpmb_region_size[region_num];
	else
		return 0;
}

int get_rpmb_config_ready_status(void)
{
	return bootdevice.rpmb_done;
}

struct rpmb_config_info get_rpmb_config(void)
{
	return bootdevice.rpmb_config;
}

void set_bootdevice_type(enum bootdevice_type type)
{
	bootdevice.type = type;
}

enum bootdevice_type get_bootdevice_type(void)
{
	return bootdevice.type;
}

void set_bootdevice_name(struct device *dev)
{
	bootdevice.dev = dev;
}

void set_bootdevice_rev_handler(int (*get_rev_func)(const struct device *,
						    char *))
{
	bootdevice.get_rev = get_rev_func;
}

static int rev_proc_show(struct seq_file *m, void *v)
{
#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_AGENT
	seq_printf(m, "%s\n", bootdevice.rev);
	return 0;
#else
	char rev[MAX_REV_LEN + 1] = { 0 };
	int ret = -EINVAL;

	if (bootdevice.get_rev) {
		ret = bootdevice.get_rev(bootdevice.dev, rev);
		seq_printf(m, "%s\n", rev);
	}
	return ret;
#endif
}

static int rev_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rev_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops rev_proc_fops = {
	.proc_open = rev_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations rev_proc_fops = {
	.open = rev_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static int type_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d", bootdevice.type);
	return 0;
}

static int type_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, type_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops type_proc_fops = {
	.proc_open = type_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations type_proc_fops = {
	.open = type_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static int name_proc_show(struct seq_file *m, void *v)
{
#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_AGENT
	seq_printf(m, "%s\n", bootdevice.name);
#else
	seq_printf(m, "%s", dev_name(bootdevice.dev));
#endif
	return 0;
}

static int name_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, name_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops name_proc_fops = {
	.proc_open = name_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations name_proc_fops = {
	.open = name_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_size(sector_t size)
{
	bootdevice.size = size;
}

static int size_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%llu\n", (unsigned long long)bootdevice.size);
	return 0;
}

static int size_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, size_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops size_proc_fops = {
	.proc_open = size_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations size_proc_fops = {
	.open = size_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_cid(u32 *cid)
{
	memcpy(bootdevice.cid, cid, sizeof(bootdevice.cid));
}

static int cid_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%08x%08x%08x%08x\n", bootdevice.cid[0],
		   bootdevice.cid[1], bootdevice.cid[2], bootdevice.cid[3]);

	return 0;
}

static int cid_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cid_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops cid_proc_fops = {
	.proc_open = cid_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations cid_proc_fops = {
	.open = cid_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_product_name(char *product_name)
{
	strlcpy(bootdevice.product_name, product_name,
		sizeof(bootdevice.product_name));
}

/*
 * len is expected to be sizeof(product_name),
 * include last byte space for '\0'
 */
void get_bootdevice_product_name(char *product_name, u32 len)
{
	strlcpy(product_name, bootdevice.product_name, len);
}

#ifdef UFS_ANONYMOUS_FACTORY_MODE
static bool get_ufs_anonymization_from_dts(void)
{
	return false;
}
#else
static bool get_ufs_anonymization_from_dts(void)
{
	int ret;
	unsigned int flag = 0;
	struct device_node *nd = NULL;

	nd = of_find_compatible_node(NULL, NULL, "huawei,memory");
	if (nd == NULL)
		return false;

	ret = of_property_read_u32(nd, "memory-anonymization", &flag);
	if (ret != 0 || flag != 1)
		return false;

	return true;
}
#endif

static char letter_and_dec_number_convert(char c)
{
	if ((c >= 'a' && c < 'z') || (c >= 'A' && c < 'Z') ||
		(c >= '0' && c < '9'))
		return c + 1;

	if (c == 'z' || c == 'Z')
		return c - ('Z' - 'A');

	if (c == '9')
		return '0';

	return c;
}

static void vendor_arry_convert(char arry[], int arry_len)
{
	int i;
	for (i = 0; i < arry_len; i++)
		arry[i] = letter_and_dec_number_convert(arry[i]);
}

static char hex_number_convert(char c)
{
	if ((c >= '0' && c < '9') || (c >= 'a' && c < 'f'))
		return c + 1;

	if (c == '9')
		return 'a';

	if (c == 'f')
		return '0';

	return c;
}

static void hex_arry_convert(char arry[], int arry_len)
{
	int i;
	for (i = 0; i < arry_len; i++)
		arry[i] = hex_number_convert(arry[i]);
}

static int product_name_proc_show(struct seq_file *m, void *v)
{
	errno_t ret;
	char product_name[MAX_NAME_LEN + 1] = {0};

	ret = memcpy_s(product_name, MAX_NAME_LEN, bootdevice.product_name,
		MAX_NAME_LEN);
	if (ret != EOK) {
		pr_err("%s memcpy_s fail.\n", __func__);
		return 0;
	}
	vendor_arry_convert(product_name, MAX_NAME_LEN);

	if (get_ufs_anonymization_from_dts())
		seq_printf(m, "%s\n", product_name);
	else
		seq_printf(m, "%s\n", bootdevice.product_name);
	return 0;
}

static int product_name_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, product_name_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops product_name_proc_fops = {
	.proc_open = product_name_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations product_name_proc_fops = {
	.open = product_name_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_pre_eol_info(u8 pre_eol_info)
{
	bootdevice.pre_eol_info = pre_eol_info;
}

static int pre_eol_info_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%02X\n", bootdevice.pre_eol_info);

	return 0;
}

static int pre_eol_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pre_eol_info_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops pre_eol_info_proc_fops = {
	.proc_open = pre_eol_info_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations pre_eol_info_proc_fops = {
	.open = pre_eol_info_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_life_time_est_typ_a(u8 life_time_est_typ_a)
{
	bootdevice.life_time_est_typ_a = life_time_est_typ_a;
}

static int life_time_est_typ_a_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%02X\n", bootdevice.life_time_est_typ_a);

	return 0;
}

static int life_time_est_typ_a_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, life_time_est_typ_a_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops life_time_est_typ_a_proc_fops = {
	.proc_open = life_time_est_typ_a_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations life_time_est_typ_a_proc_fops = {
	.open = life_time_est_typ_a_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_life_time_est_typ_b(u8 life_time_est_typ_b)
{
	bootdevice.life_time_est_typ_b = life_time_est_typ_b;
}

static int life_time_est_typ_b_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%02X\n", bootdevice.life_time_est_typ_b);

	return 0;
}

static int life_time_est_typ_b_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, life_time_est_typ_b_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops life_time_est_typ_b_proc_fops = {
	.proc_open = life_time_est_typ_b_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations life_time_est_typ_b_proc_fops = {
	.open = life_time_est_typ_b_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

void set_bootdevice_manfid(unsigned int manfid)
{
	bootdevice.manfid = manfid;
}

unsigned int get_bootdevice_manfid(void)
{
	return bootdevice.manfid;
}

static int manfid_proc_show(struct seq_file *m, void *v)
{
	int ret;
	char manfid[MANUFACTURE_ID_LENGTH] = {0};

	ret = snprintf_s(manfid, MANUFACTURE_ID_LENGTH,
		MANUFACTURE_ID_LENGTH - 1, "%06x", bootdevice.manfid);
	if (ret == -1) {
		pr_err("%s snprintf_s fail.\n", __func__);
		return 0;
	}
	hex_arry_convert(manfid, MANUFACTURE_ID_LENGTH - 1);

	if (get_ufs_anonymization_from_dts())
		seq_printf(m, "0x%s\n", manfid);
	else
		seq_printf(m, "0x%06x\n", bootdevice.manfid);

	return 0;
}

static int manfid_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, manfid_proc_show, NULL);
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops manfid_proc_fops = {
	.proc_open = manfid_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations manfid_proc_fops = {
	.open = manfid_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static int flash_find_index_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", bootdevice.ptn_index);

	return 0;
}

static int flash_find_index_proc_open(struct inode *p_inode,
				      struct file *p_file)
{
	return single_open(p_file, flash_find_index_proc_show, NULL);
}

static long flash_find_index_proc_ioctl(struct file *p_file, unsigned int cmd,
					unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct flash_find_index_user info;

	/*
	 * ensure only one process can visit flash_find_index device at the same
	 * time in API
	 */
	if (down_interruptible(&flash_find_index_sem))
		return -EBUSY;

	switch (cmd) {
	case INDEXEACCESSDATA:
		if (copy_from_user(&info, argp,
				   sizeof(struct flash_find_index_user))) {
			up(&flash_find_index_sem);
			return -1;
		}
		/*
		 * Make sure we will not get a non-terminate string
		 * from user space
		 */
		info.name[MAX_PARTITION_NAME_LENGTH - 1] = '\0';

		info.index = flash_get_ptn_index(info.name);
		if (info.index < 0) {
			up(&flash_find_index_sem);
			return -1;
		}

		bootdevice.ptn_index = (unsigned char)info.index;
		break;

	default:
		pr_err("Unknown command!\n");
		up(&flash_find_index_sem);
		return -1;
	}

	up(&flash_find_index_sem);
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops flash_find_index_proc_fops = {
	.proc_open = flash_find_index_proc_open,
	.proc_read = seq_read,
	.proc_ioctl = flash_find_index_proc_ioctl,
#ifdef CONFIG_COMPAT
	.proc_compat_ioctl = flash_find_index_proc_ioctl,
#endif
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations flash_find_index_proc_fops = {
	.open = flash_find_index_proc_open,
	.read = seq_read,
	.unlocked_ioctl = flash_find_index_proc_ioctl,
	.compat_ioctl = flash_find_index_proc_ioctl,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

#ifdef CONFIG_HUFS_BOOTROM_MODE
static int ufs_bootrom_mode_proc_show(struct seq_file *m, void *v)
{
	u32 ret = ufs_bootrom_mode_sel_efuse_set();
	seq_printf(m, "%d\n", ret);
	return 0;
}

static int ufs_bootrom_mode_proc_open(struct inode *p_inode,
				      struct file *p_file)
{
	return single_open(p_file, ufs_bootrom_mode_proc_show, NULL);
}

static const struct proc_ops ufs_bootrom_mode_proc_fops = {
	.proc_open = ufs_bootrom_mode_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif /* CONFIG_HUFS_BOOTROM_MODE */

#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_SERVICE
static int format_bootdevice_info(char *buf, unsigned int buf_size, unsigned int *format_size)
{
	struct bootdevice_virtualiz *bootdevice_vir = NULL;
	char rev_tmp[MAX_REV_LEN + 1] = { 0 };
	u32 i, lenth;
	int ret;

	if (!buf || !format_size)
		return -EINVAL;

	if (sizeof(struct bootdevice_virtualiz) > buf_size)
		return -ENOMEM;

	bootdevice_vir = (struct bootdevice_virtualiz*)buf;
	bootdevice_vir->size = bootdevice.size;
	bootdevice_vir->pre_eol_info = bootdevice.pre_eol_info;
	bootdevice_vir->life_time_est_typ_a = bootdevice.life_time_est_typ_a;
	bootdevice_vir->life_time_est_typ_b = bootdevice.life_time_est_typ_b;
	bootdevice_vir->manfid = bootdevice.manfid;
	for (i = 0; i < 4; i++)
		bootdevice_vir->cid[i] = bootdevice.cid[i];

	/* ufs hci name */
	lenth = strlen(dev_name(bootdevice.dev));
	lenth = (lenth > MAX_NAME_LENTH) ? MAX_NAME_LENTH : lenth;
	ret = strncpy_s(bootdevice_vir->name, MAX_NAME_LENTH + 1, dev_name(bootdevice.dev), lenth);
	if (ret != EOK) {
		pr_err("%s strncpy_s name fail.\n", __func__);
		return ret;
	}
	bootdevice_vir->name[MAX_NAME_LENTH] ='\0';

	/* ufs device name */
	get_bootdevice_product_name(bootdevice_vir->product_name, MAX_NAME_LENTH + 1);
	bootdevice_vir->product_name[MAX_NAME_LENTH] ='\0';

	/* ufs device rev */
	if (bootdevice.get_rev) {
		ret = bootdevice.get_rev(bootdevice.dev, rev_tmp);
		if (ret != EOK) {
			pr_err("%s get rev fail.\n", __func__);
			return ret;
		}
		ret = strncpy_s(bootdevice_vir->rev, MAX_REV_LEN + 1, rev_tmp, MAX_REV_LEN);
		if (ret != EOK) {
			pr_err("%s strncpy_s rev fail.\n", __func__);
			return ret;
		}
		bootdevice_vir->rev[MAX_REV_LEN] ='\0';
	}

	*format_size += sizeof(struct bootdevice_virtualiz);
	return 0;
}

static int format_rpmb_info(char *buf, unsigned int buf_size, unsigned int *format_size)
{
	int i;
	struct rpmb_config_info *rpmb_config = NULL;

	if (sizeof(struct rpmb_config_info) > buf_size)
		return -ENOMEM;

	rpmb_config = (struct rpmb_config_info*)buf;
	/* format send buf */
	rpmb_config->rpmb_total_blks = get_rpmb_total_blks();
	rpmb_config->rpmb_read_frame_support = get_rpmb_read_frame_support();
	rpmb_config->rpmb_write_frame_support = get_rpmb_write_frame_support();
	rpmb_config->rpmb_unit_size = get_rpmb_unit_size();
	for (i = 0; i < MAX_RPMB_REGION_NUM; i++)
		rpmb_config->rpmb_region_size[i] = get_rpmb_region_size(i);

	rpmb_config->rpmb_blk_size = get_rpmb_blk_size();
	rpmb_config->rpmb_read_align = get_rpmb_read_align();
	rpmb_config->rpmb_write_align = get_rpmb_write_align();
	rpmb_config->rpmb_region_enable = get_rpmb_region_enable();

	*format_size += sizeof(struct rpmb_config_info);
	return 0;
}

static int bootdevice_virtualiz_service_recv_process(char *buf, unsigned int buf_size)
{
	char send_buf[MAX_SHMEC_BUF_SIZE];
	unsigned int format_size = 0;
	int ret;

	ret = format_rpmb_info(send_buf, MAX_SHMEC_BUF_SIZE, &format_size);
	if (ret) {
		pr_err("%s format rpmb info fail\n", __func__);
		return ret;
	}

	ret = format_bootdevice_info(send_buf + format_size, MAX_SHMEC_BUF_SIZE - format_size,
			&format_size);
	if (ret) {
		pr_err("%s format bootdevice info fail\n", __func__);
		return ret;
	}

	return ufs_virtualiz_service_send_buf(send_buf, format_size, SHMEC_MODULE_UFS_BOOTDEVICE);
}

static struct ufs_service_ops bootdevice_service_ops = {
	{ SHMEC_UFS_BOOTDEVICE },
	bootdevice_virtualiz_service_recv_process,
};

static int __init bootdevice_virtualiz_service_init(void)
{
	int ret;

	ret = ufs_virtualiz_service_register(&bootdevice_service_ops, SHMEC_MODULE_UFS_BOOTDEVICE);
	if (ret) {
		pr_err("%s register service fail\n", __func__);
		return ret;
	}

	return 0;
}
late_initcall(bootdevice_virtualiz_service_init);
#endif

#ifdef CONFIG_SCSI_UFS_VIRTUALIZ_AGENT
static int set_bootdevice_info(char *buf, unsigned int buf_size, unsigned int *proc_size)
{
	struct bootdevice_virtualiz *bootdevice_vir = NULL;
	int ret, i;

	if (!buf || !proc_size)
		return -EINVAL;

	if (sizeof(struct bootdevice_virtualiz) > buf_size)
		return -ENOMEM;

	bootdevice_vir = (struct bootdevice_virtualiz*)buf;
	set_bootdevice_size(bootdevice_vir->size);
	set_bootdevice_pre_eol_info(bootdevice_vir->pre_eol_info);
	set_bootdevice_life_time_est_typ_a(bootdevice_vir->life_time_est_typ_a);
	set_bootdevice_life_time_est_typ_b(bootdevice_vir->life_time_est_typ_b);
	set_bootdevice_manfid(bootdevice_vir->manfid);
	for (i = 0; i < 4; i++)
		bootdevice.cid[i] = bootdevice_vir->cid[i];
	ret = memcpy_s(bootdevice.product_name, MAX_NAME_LEN + 1,
					bootdevice_vir->product_name, MAX_NAME_LENTH + 1);
	if (ret != EOK) {
		pr_err("%s memcpy_s product_name fail.\n", __func__);
		return ret;
	}

	ret = memcpy_s(bootdevice.name, MAX_NAME_LENTH + 1,
					bootdevice_vir->name, MAX_NAME_LENTH + 1);
	if (ret != EOK) {
		pr_err("%s memcpy_s name fail.\n", __func__);
		return ret;
	}

	ret = memcpy_s(bootdevice.rev, MAX_REV_LENTH + 1,
					bootdevice_vir->rev, MAX_REV_LENTH + 1);
	if (ret != EOK) {
		pr_err("%s memcpy_s rev fail.\n", __func__);
		return ret;
	}

	*proc_size += sizeof(struct bootdevice_virtualiz);
	return 0;
}

static int set_rpmb_info(char *buf, unsigned int buf_size, unsigned int *proc_size)
{
	int i;
	struct rpmb_config_info *rpmb_config = NULL;

	if (!buf || !proc_size || (buf_size < sizeof(struct rpmb_config_info))) {
		pr_err("%s invalid parameter\n", __func__);
		return -EINVAL;
	}

	rpmb_config = (struct rpmb_config_info *)buf;
	set_rpmb_total_blks(rpmb_config->rpmb_total_blks);
	set_rpmb_blk_size(rpmb_config->rpmb_blk_size);
	set_rpmb_unit_size(rpmb_config->rpmb_unit_size);
	set_rpmb_region_enable(rpmb_config->rpmb_region_enable);
	set_rpmb_read_align(rpmb_config->rpmb_read_align);
	set_rpmb_write_align(rpmb_config->rpmb_write_align);
	for (i = 0; i < MAX_RPMB_REGION_NUM; i++)
		set_rpmb_region_size(i, rpmb_config->rpmb_region_size[i]);

	set_rpmb_read_frame_support(rpmb_config->rpmb_read_frame_support);
	set_rpmb_write_frame_support(rpmb_config->rpmb_write_frame_support);
	set_rpmb_config_ready_status();

	*proc_size += sizeof(struct rpmb_config_info);
	return 0;
}

static int bootdevice_virtualiz_agent_recv_process(char *buf, unsigned int buf_size)
{
	int ret;
	unsigned int proc_size = 0;

	if (!buf) {
		pr_err("%s invalid parameter\n", __func__);
		return -EINVAL;
	}

	ret = set_rpmb_info(buf, buf_size, &proc_size);
	if (ret) {
		pr_err("%s set rpmb info fail\n", __func__);
		return ret;
	}

	ret = set_bootdevice_info(buf + proc_size, buf_size - proc_size, &proc_size);
	if (ret) {
		pr_err("%s set rpmb info fail\n", __func__);
		return ret;
	}
	return 0;
}

struct ufs_agent_ops bootdevice_agent_ops = {
	{ SHMEC_UFS_BOOTDEVICE },
	0,
	bootdevice_virtualiz_agent_recv_process,
	NULL,
	NULL,
};

static int __init bootdevice_virtualiz_agent_init(void)
{
	struct device_node *np = NULL;
	int ret;
	char buf[MAX_SHMEC_BUF_SIZE];

	np = of_find_compatible_node(NULL, NULL, "vendor,bootdevice_virtualiz");
	if (!np)
		return 0;

	bootdevice_agent_ops.irq = irq_of_parse_and_map(np, 0);
	ret = ufs_virtualiz_agent_register(&bootdevice_agent_ops, SHMEC_MODULE_UFS_BOOTDEVICE);
	if (ret)
		pr_err("%s register agent fail\n", __func__);

	/* send event to host, notify host to send info */
	ret = ufs_virtualiz_agent_send_buf(buf, MAX_SHMEC_BUF_SIZE, SHMEC_MODULE_UFS_BOOTDEVICE);
	if (ret)
		pr_err("%s send event fail\n", __func__);

	return ret;
}
late_initcall(bootdevice_virtualiz_agent_init);
#endif

static int ufs_anonymous_proc_show(struct seq_file *m, void *v)
{
	int is_anon;
	if (get_ufs_anonymization_from_dts())
		is_anon = 1;
	else
		is_anon = 0;
	seq_printf(m, "%d\n", is_anon);
	return 0;
}

static int ufs_anonymous_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_anonymous_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops ufs_anonymous_proc_fops = {
	.proc_open = ufs_anonymous_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations ufs_anonymous_proc_fops = {
	.open = ufs_anonymous_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static int __init proc_bootdevice_init(void)
{
	if (!proc_mkdir("bootdevice", NULL)) {
		pr_err("make proc dir bootdevice failed\n");
		return -EFAULT;
	}

	proc_create("bootdevice/rev", 0000, NULL, &rev_proc_fops);
	proc_create("bootdevice/type", 0000, NULL, &type_proc_fops);
	proc_create("bootdevice/name", 0000, NULL, &name_proc_fops);
	proc_create("bootdevice/size", 0000, NULL, &size_proc_fops);
	proc_create("bootdevice/cid", 0000, NULL, &cid_proc_fops);
	proc_create("bootdevice/product_name", 0000, NULL,
		    &product_name_proc_fops);
	proc_create("bootdevice/pre_eol_info", 0000, NULL,
		    &pre_eol_info_proc_fops);
	proc_create("bootdevice/life_time_est_typ_a", 0000, NULL,
		    &life_time_est_typ_a_proc_fops);
	proc_create("bootdevice/life_time_est_typ_b", 0000, NULL,
		    &life_time_est_typ_b_proc_fops);
	proc_create("bootdevice/manfid", 0000, NULL, &manfid_proc_fops);
	sema_init(&flash_find_index_sem, 1);
	proc_create("bootdevice/flash_find_index", 0660,
		    (struct proc_dir_entry *)NULL, &flash_find_index_proc_fops);
	proc_create("bootdevice/vendor_anon", 0000, NULL, &ufs_anonymous_proc_fops);

#ifdef CONFIG_HUFS_BOOTROM_MODE
#ifdef UFS_BOOTROM_FACTORY_MODE
	proc_create("bootdevice/ufs_bootrom_mode", 0444, NULL, &ufs_bootrom_mode_proc_fops);
#else
	proc_create("bootdevice/ufs_bootrom_mode", 0440, NULL, &ufs_bootrom_mode_proc_fops);
#endif
#endif

	return 0;
}
module_init(proc_bootdevice_init);
