/******************************************************************
 * Copyright    Copyright (c) 2020- Hisilicon Technologies CO., Ltd.
 * File name    ipp_core.C
 * Description:
 *
 * Date         2020-04-16 17:36:21
 ******************************************************************/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/of_irq.h>
#include <linux/iommu.h>
#include <linux/pm_wakeup.h>
#include <linux/ion.h>
#include <linux/ion/mm_ion.h>
#include <linux/genalloc.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/version.h>
#include <platform_include/isp/linux/hipp_common.h>
#include <linux/delay.h>
#include <linux/types.h>
#include "ipp.h"
#include "memory.h"
#include "ipp_core.h"
#include "ipp_pcie.h"
#include "segment_ipp_adapter.h"


#define DTS_NAME_HISI_IPP "hisilicon,ipp"

struct ipp_adapter_t {
	uint32_t cmd;
	int (*handler)(unsigned long args);
};

struct hispcpe_s *hispcpe_dev;

#define hipp_min(a, b) (((a) < (b)) ? (a) : (b))

static struct hispcpe_s *get_cpedev_bymisc(struct miscdevice *mdev)
{
	struct hispcpe_s *dev = NULL;

	if (mdev == NULL) {
		pr_err("[%s] Failed : mdev.%pK\n", __func__, mdev);
		return NULL;
	}

	dev = container_of(mdev, struct hispcpe_s, miscdev);
	return dev;
}

void __iomem *hipp_get_regaddr(unsigned int type)
{
	struct hispcpe_s *dev = hispcpe_dev;

	if (type >= hipp_min(MAX_HISP_CPE_REG, dev->reg_num)) {
		pr_err("[%s] unsupported type.0x%x\n", __func__, type);
		return NULL;
	}

	return dev->reg[type] ? dev->reg[type] : NULL;
}

int get_hipp_smmu_info(int *sid, int *ssid)
{
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.NULL\n", __func__);
		return -EINVAL;
	}

	if ((sid == NULL) || (ssid == NULL)) {
		pr_err("[%s] Failed : smmu info.NULL\n", __func__);
		return -EINVAL;
	}

	*sid = dev->sid;
	*ssid = dev->ssid;

	logd("sid=%d, ssid=%d", dev->sid, dev->ssid);
	return 0;
}

int hispcpe_reg_set(unsigned int mode, unsigned int offset, unsigned int value)
{
	struct hispcpe_s *dev = hispcpe_dev;
	void __iomem *reg_base = NULL;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.NULL\n", __func__);
		return -1;
	}

	if (mode > MAX_HISP_CPE_REG - 1) {
		pr_err("[%s] Failed : mode.%d\n", __func__, mode);
		return -1;
	}

	pr_debug("%s: mode.%d, value.0x%x\n", __func__, mode, value);
	reg_base = dev->reg[mode];
	if (reg_base == NULL) {
		pr_err("[%s] Failed : reg.NULL, mode.%d\n", __func__, mode);
		return -1;
	}

	if (offset >= resource_size(dev->r[mode])) {
		pr_err("[%s] Failed : invalied offset.0x%x\n", __func__, offset);
		return -1;
	}

	writel(value, reg_base + offset);
	return ISP_IPP_OK;
}

unsigned int hispcpe_reg_get(unsigned int mode, unsigned int offset)
{
	struct hispcpe_s *dev = hispcpe_dev;
	unsigned int value;
	void __iomem *reg_base = NULL;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.NULL\n", __func__);
		return 1;
	}

	if (mode > MAX_HISP_CPE_REG - 1) {
		pr_err("[%s] Failed : mode.%d\n", __func__, mode);
		return ISP_IPP_OK;
	}

	reg_base = dev->reg[mode];
	if (reg_base == NULL) {
		pr_err("[%s] Failed : reg.NULL, mode.%d\n", __func__, mode);
		return 1;
	}

	if (offset >= resource_size(dev->r[mode])) {
		pr_err("[%s] Failed : invalied offset.0x%x\n", __func__, offset);
		return -1;
	}

	value = readl(reg_base + offset);
	return value;
}

static int hispcpe_is_bypass_by_soc_spec(void)
{
	const char *soc_spec = NULL;
	int ret;

	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (np == NULL) {
		pr_err("[%s] of_find_compatible_node fail\n", __func__);
		return 0;
	}

	ret = of_property_read_string(np, "soc_spec_set", &soc_spec);
	if (ret < 0) {
		pr_err("[%s] of_property_read_string fail\n", __func__);
		return 0;
	}

	ret = strncmp(soc_spec, "pc", sizeof("pc"));
	if (ret == 0) {
		pr_err("[%s] soc spec is pc, bypass ipp test\n", __func__);
		return 1;
	}

	ret = strncmp(soc_spec, "lite_pc", sizeof("lite_pc"));
	if (ret == 0) {
		pr_err("[%s] soc spec is lite_pc, bypass ipp test\n", __func__);
		return 1;
	}

	pr_info("[%s] soc spec is not bypass\n", __func__);
	return 0;
}

void lock_ipp_wakelock(void)
{
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL)
		return;

	mutex_lock(&dev->ipp_wakelock_mutex);
	if (!dev->ipp_wakelock->active) {
		__pm_stay_awake(dev->ipp_wakelock);
		pr_info("ipp power up wake lock.\n");
	}
	mutex_unlock(&dev->ipp_wakelock_mutex);
}

void relax_ipp_wakelock(void)
{
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL)
		return;

	mutex_lock(&dev->ipp_wakelock_mutex);
	if (dev->ipp_wakelock->active) {
		__pm_relax(dev->ipp_wakelock);
		pr_info("ipp power up wake unlock.\n");
	}
	mutex_unlock(&dev->ipp_wakelock_mutex);
}

static int hipp_cmd_pwrup(unsigned long args)
{
	struct hispcpe_s *dev = hispcpe_dev;
	int ret = 0;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	logd("+");
	if (dev->refs_power_up > 0) {
		dev->refs_power_up++;
		pr_info("[%s]: refs_power_up.%d\n", __func__, dev->refs_power_up);
		mutex_unlock(&dev->dev_lock);
		return 0;
	}

	ret = hipp_powerup();
	if (ret < 0) {
		pr_err("[%s] Failed : hipp_powerup.%d\n", __func__, ret);
		mutex_unlock(&dev->dev_lock);
		return ret;
	}
	dev->refs_power_up++;
	pr_info("[%s]: refs_power_up.%d\n", __func__, dev->refs_power_up);
	logd("-");
	mutex_unlock(&dev->dev_lock);

	lock_ipp_wakelock();
	return 0;
}

static int hipp_cmd_pwrdn(unsigned long args)
{
	int ret = 0;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	logd("+");
	if (dev->refs_power_up <= 0) {
		pr_err("[%s] : ippcore no pw\n", __func__);
		mutex_unlock(&dev->dev_lock);
		return 0;
	}

	if (dev->refs_power_up == 1) {
		ret = hispcpe_work_check();
		if (ret < 0){
			pr_err("[%s] Failed : hispcpe working\n", __func__);
			mutex_unlock(&dev->dev_lock);
			return ret;
		}
	}

	dev->refs_power_up--;
	if (dev->refs_power_up > 0) {
		pr_info("[%s]: refs_power_up.%d\n", __func__, dev->refs_power_up);
		mutex_unlock(&dev->dev_lock);
		return 0;
	}

	ret = hipp_powerdn();
	if (ret < 0)
		pr_err("[%s] Failed : hipp_powerdn.%d\n", __func__, ret);
	logd("-");
	mutex_unlock(&dev->dev_lock);

	relax_ipp_wakelock();

	return ret;
}

static int hipp_cmd_get_mem_size(unsigned long args)
{
	int ret = 0;
	unsigned long remaining_bytes = 0;
	struct ipp_feature_mem_size_t *ipp_cfg = NULL;
	void __user *p_msg = (void __user *)(uintptr_t)args;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	if (p_msg == NULL) {
		pr_err("[%s] Failed : p_msg ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	logd("+");

	ipp_cfg = (struct ipp_feature_mem_size_t *)vmalloc(sizeof(struct ipp_feature_mem_size_t));
	if (ipp_cfg == NULL) {
		pr_err("[%s] Failed to vmalloc memory for ipp_cfg\n", __func__);
		mutex_unlock(&dev->dev_lock);
		return ISP_IPP_ERR;
	}

	if (memset_s(ipp_cfg, sizeof(struct ipp_feature_mem_size_t), 0, sizeof(struct ipp_feature_mem_size_t))) {
		pr_err("[%s] Failed to memset_s memory for ipp_cfg\n", __func__);
		ret = ISP_IPP_ERR;
		goto out;
	}

	if ((sizeof(struct ipp_feature_mem_size_t) + (uintptr_t)p_msg) < (uintptr_t)p_msg) {
		pr_err("[%s] Failed arg size is too large\n", __func__);
		ret = ISP_IPP_ERR;
		goto out;
	}

	remaining_bytes = copy_from_user(ipp_cfg, p_msg, sizeof(struct ipp_feature_mem_size_t));
	if (remaining_bytes != 0) {
		pr_err("[%s] copy_from_user failed, remaining_bytes = %ld\n", __func__, remaining_bytes);
		ret = ISP_IPP_ERR;
		goto out;
	}

	if(ipp_cfg->work_module >= WORK_MODULE_MAX) {
		pr_err("[%s] work_module = %d invalid\n", __func__, ipp_cfg->work_module);
		ret = ISP_IPP_ERR;
		goto out;
	}
	ipp_cfg->mem_size = calculate_mem_size_info(ipp_cfg->work_module);
	logd("The work_module.%d need 0x%x Bytes memory", ipp_cfg->work_module, ipp_cfg->mem_size);

	remaining_bytes = copy_to_user(p_msg, ipp_cfg, sizeof(struct ipp_feature_mem_size_t));
	if (remaining_bytes != 0) {
		pr_err("[%s] copy_to_user failed, remaining_bytes = %ld\n", __func__, remaining_bytes);
		ret = ISP_IPP_ERR;
		goto out;
	}
	logd("-");

out:
	vfree(ipp_cfg);
	ipp_cfg = NULL;
	mutex_unlock(&dev->dev_lock);
	return ret;
}

static int hipp_cmd_do_req(unsigned long args)
{
	int ret;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	logi("+");
	if (dev->refs_power_up<= 0) {
		pr_err("[%s] Failed : ipp powerdown.%d\n", __func__, dev->refs_power_up);
		mutex_unlock(&dev->dev_lock);
		return -EINVAL;
	}

	mutex_unlock(&dev->dev_lock);
	ret = hipp_adapter_do_request(args);
	if (ret < 0)
		loge("Failed : hipp_adapter_do_request .%d", ret);

	return ret;
}

static int hipp_cmd_map_buf(unsigned long args)
{
	int ret;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	ret = hispcpe_map_kernel(args);
	if (ret != 0) {
		pr_err("[%s] Failed : hispcpe_map_kernel.%d\n",	__func__, ret);
		mutex_unlock(&dev->dev_lock);
		return -EFAULT;
	}
	mutex_unlock(&dev->dev_lock);

	return 0;
}

static int hipp_cmd_unmap_buf(unsigned long args)
{
	int ret;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	ret = hispcpe_unmap_kernel(args);
	if (ret < 0) {
		pr_err("[%s] Failed: hispcpe_umap_kernel.%d\n", __func__, ret);
		mutex_unlock(&dev->dev_lock);
		return -ENOMEM;
	}
	mutex_unlock(&dev->dev_lock);

	return ret;
}

static int hipp_cmd_map_algo_buf(unsigned long args)
{
	int ret;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	ret = hispcpe_map_kernel_algo(args);
	if (ret != 0) {
		pr_err("[%s] Failed : hispcpe_map_kernel.%d\n",	__func__, ret);
		mutex_unlock(&dev->dev_lock);
		return -EFAULT;
	}
	mutex_unlock(&dev->dev_lock);
	return 0;
}

static int hipp_cmd_unmap_algo_buf(unsigned long args)
{
	int ret;
	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dev->dev_lock);
	ret = hispcpe_unmap_kernel_algo(args);
	if (ret < 0) {
		pr_err("[%s] Failed: hispcpe_unmap_kernel_algo.%d\n", __func__, ret);
		mutex_unlock(&dev->dev_lock);
		return -ENOMEM;
	}
	mutex_unlock(&dev->dev_lock);

	return ret;
}

#ifdef CONFIG_IPP_DEBUG
static int hipp_cmd_syzkaller_test(unsigned long args)
{
	int ret;
	int ret1;

	struct hispcpe_s *dev = hispcpe_dev;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev ptr is null\n", __func__);
		return -EINVAL;
	}

	ret = hipp_cmd_pwrup(0);
	if (ret < 0) {
		loge("Failed : hipp_cmd_pwrup .%d", ret);
		return ret;
	}

	mutex_lock(&dev->dev_lock);
	logi("+");
	if (dev->refs_power_up<= 0) {
		pr_err("[%s] Failed : ipp powerdown.%d\n", __func__, dev->refs_power_up);
		mutex_unlock(&dev->dev_lock);
		return -EINVAL;
	}

	hipp_resource_init();
	ret = hipp_adapter_do_request(args);
	if (ret < 0)
		loge("Failed : hipp_adapter_do_request .%d", ret);
	hipp_resource_deinit();
	mutex_unlock(&dev->dev_lock);

	ret1 = hipp_cmd_pwrdn(0);
	if (ret1 < 0)
		loge("Failed : hipp_cmd_pwrdn .%d", ret1);

	if (ret < 0 || ret1 < 0)
		return ISP_IPP_ERR;
	else
		return ISP_IPP_OK;
}
#endif

#if defined(CONFIG_IPP_DEBUG) && defined(IPP_KERNEL_USE_PCIE_VERIFY)
static int hispcpe_ioremap_reg(struct hispcpe_s *dev)
{
	return 0;
}
static int hispcpe_register_irq(struct hispcpe_s *dev)
{
	return 0;
}
static void hispcpe_unregister_irq(struct hispcpe_s *dev)
{
	return;
}

void hispcpe_pcie_ioremap_reg(struct hispcpe_s *dev)
{
	unsigned int min;
	unsigned int i;

	if (dev->pci_flag == 0)
		return;

	min = hipp_min(MAX_HISP_CPE_REG, dev->reg_num);
	pr_err("[%s] min regs: %d\n", __func__, min);

	for (i = 0; i < min; i++) {
		dev->reg[i] = hispcpe_pcie_reg_addr(dev->r[i]->start);
		pr_info("[%s] reg[%d] addr.0x%llx\n", __func__, i, dev->reg[i]);
	}
}

#else
static int hispcpe_ioremap_reg(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	unsigned int i;
	unsigned int min;

	device = &dev->pdev->dev;

	min = hipp_min(MAX_HISP_CPE_REG, dev->reg_num);
	for (i = 0; i < min; i++) {
		dev->reg[i] = devm_ioremap_resource(device, dev->r[i]);

		if (dev->reg[i] == NULL) {
			pr_err("[%s] Failed : %d.devm_ioremap_resource.%pK\n",
				__func__, i, dev->reg[i]);
			return -ENOMEM;
		}
	}

	return ISP_IPP_OK;
}

static int hispcpe_register_irq(struct hispcpe_s *dev)
{
	int ret = 0;
	unsigned int min;
	unsigned int i;

	min = hipp_min(MAX_HISP_CPE_IRQ, dev->irq_num);
	for (i = 0; i < min; i++) {
		pr_info("[%s] : Hipp.%d, IRQ.%d\n", __func__, i, dev->irq[i]);

		if (i == CPE_IRQ_0) {
			ret = hipp_adapter_register_irq(dev->irq[i]);
			if (ret != 0) {
				pr_err("[%s] Failed : %d.request_irq.%d\n", __func__, i, ret);
				return ret;
			}
		}
	}

	return ISP_IPP_OK;
}

static void hispcpe_unregister_irq(struct hispcpe_s *dev)
{
	int ret = 0;
	unsigned int min;
	unsigned int i;

	min = hipp_min(MAX_HISP_CPE_IRQ, dev->irq_num);
	for (i = 0; i < min; i++) {
		pr_info("[%s] : Hipp.%d, IRQ.%d\n", __func__, i, dev->irq[i]);

		if (i == CPE_IRQ_0) {
			ret = hipp_adapter_unregister_irq(dev->irq[i]);
			if (ret != 0)
				pr_err("[%s] Failed : %d.request_irq.%d\n", __func__, i, ret);
		}
	}

	return;
}
#endif

static const struct ipp_adapter_t g_ipp_adapt[] = {
	{HISP_IPP_PWRUP,         hipp_cmd_pwrup     },
	{HISP_IPP_GET_MEM_SIZE,  hipp_cmd_get_mem_size},
	{HISP_IPP_MAP_BUF,       hipp_cmd_map_buf   },
	{HISP_IPP_CMD_REQUEST,   hipp_cmd_do_req    },
	{HISP_IPP_UNMAP_BUF,     hipp_cmd_unmap_buf },
	{HISP_IPP_PWRDN,         hipp_cmd_pwrdn     },
	{HISP_IPP_MAP_ALGO_BUF,    hipp_cmd_map_algo_buf   },
	{HISP_IPP_UNMAP_ALGO_BUF,  hipp_cmd_unmap_algo_buf },
#ifdef CONFIG_IPP_DEBUG
	{HISP_IPP_SYZKALLER_TEST,  hipp_cmd_syzkaller_test },
#endif
};

static long hispcpe_ioctl(struct file *filp,
	unsigned int cmd, unsigned long args)
{
	int ret = 0;
	size_t nums;
	unsigned int index;
	struct hispcpe_s *dev = NULL;
	int (*filter)(unsigned long args) = NULL;

	dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data);
	if ((dev == NULL) || (dev != hispcpe_dev)) {
		pr_err("[%s] Failed : dev.invalid\n", __func__);
		return -EINVAL;
	}

	if (dev->initialized == 0) {
		pr_err("[%s] Failed : IPP Device Not Exist.0\n", __func__);
		return -ENXIO;
	}

	nums = ARRAY_SIZE(g_ipp_adapt);
	for (index = 0; index < nums; index++) {
		if (g_ipp_adapt[index].cmd == cmd) {
			filter = g_ipp_adapt[index].handler;
			break;
		}
	}

	if (filter == NULL) {
		pr_err("[%s] Failed : not supported.0x%x\n", __func__, cmd);
		return -EINVAL;
	}

	ret = filter(args);

	return ret;
}

static int hispcpe_open(struct inode *inode, struct file *filp)
{
	struct hispcpe_s *dev = NULL;

	pr_info("[%s] +\n", __func__);

	dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data);
	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (!dev->initialized) {
		pr_err("[%s] Failed : CPE Device Not Exist.%d\n",
			__func__, dev->initialized);
		return -ENXIO;
	}

	mutex_lock(&dev->open_lock);
	if (dev->open_refs != 0) {
		pr_err("[%s] Failed: Opened, open_refs.0x%x\n",
			__func__, dev->open_refs);
		mutex_unlock(&dev->open_lock);
		return -EBUSY;
	}

	dev->open_refs = 1;
	mutex_unlock(&dev->open_lock);

	pr_info("[%s] -\n", __func__);

	return ISP_IPP_OK;
}

static void hispcpe_exception(void)
{
	struct hispcpe_s *dev = hispcpe_dev;
	int ret;

	pr_alert("[%s] : enter\n", __func__);
	if (dev == NULL) {
		pr_err("[%s] Failed : Device Not Exist\n", __func__);
		return;
	}

	mutex_lock(&dev->dev_lock);
	if (dev->refs_power_up > 0) {
		ret = hipp_powerdn();
		if (ret < 0)
			pr_err("[%s] Failed : hipp_powerdn.%d\n", __func__, ret);
		dev->refs_power_up = 0;
	}

	ret = hispcpe_work_check();
	if (ret == 0)
		hipp_adapter_exception();

	mutex_unlock(&dev->dev_lock);

	relax_ipp_wakelock();
}

static int hispcpe_release(struct inode *inode, struct file *filp)
{
	struct hispcpe_s *dev = NULL;

	pr_info("[%s] enter\n", __func__);

	dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data);
	if (dev == NULL) {
		pr_err("[%s] Failed : dev.NULL\n", __func__);
		return -EINVAL;
	}

	if (dev->initialized == 0) {
		pr_err("[%s] Failed : CPE Device Not Exist\n", __func__);
		return -ENXIO;
	}

	mutex_lock(&dev->open_lock);
	if (dev->open_refs <= 0) {
		pr_err("[%s] Failed: Closed, open_refs.0\n", __func__);
		mutex_unlock(&dev->open_lock);
		return -EBUSY;
	}
	dev->open_refs = 0;
	mutex_unlock(&dev->open_lock);

	hispcpe_exception();

	return ISP_IPP_OK;
}

static const struct file_operations hispcpe_fops = {
	.owner          = THIS_MODULE,
	.open           = hispcpe_open,
	.release        = hispcpe_release,
	.unlocked_ioctl = hispcpe_ioctl,
};

static struct miscdevice hispcpe_miscdev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = KBUILD_MODNAME,
	.fops   = &hispcpe_fops,
};

static int hispcpe_resource_init(struct hispcpe_s *dev)
{
	int ret;

	ret = hispcpe_register_irq(dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hispcpe_register_irq.%d\n",
			__func__, ret);
		goto error_exit;
	}

	ret = hispcpe_ioremap_reg(dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hispcpe_ioremap_reg.%d\n",
			__func__, ret);
		goto error_exit;
	}

	return ISP_IPP_OK;
error_exit:
	hispcpe_unregister_irq(dev);
	return ISP_IPP_ERR;
}

int hispcpe_get_irq(unsigned int index)
{
	struct device_node *np = NULL;
	char *name = DTS_NAME_HISI_IPP;
	int irq;

	np = of_find_compatible_node(NULL, NULL, name);
	if (np == NULL) {
		pr_err("[%s] Failed : %s.of_find_compatible_node.%pK\n",
			__func__, name, np);
		return -ENXIO;
	}

	irq = irq_of_parse_and_map(np, index);
	if (irq == 0) {
		pr_err("[%s] Failed : irq_map.%d\n", __func__, irq);
		return -ENXIO;
	}

	pr_info("%s: comp.%s, cpe irq.%d.\n", __func__, name, irq);
	return irq;
}

static int hispcpe_getdts_irq(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	unsigned int i;
	int irq;
	int ret;
	unsigned int min;

	if ((dev == NULL) || (dev->pdev == NULL)) {
		pr_err("[%s] Failed : dev or pdev.NULL\n", __func__);
		return -ENXIO;
	}

	device = &dev->pdev->dev;
	if (device == NULL) {
		pr_err("[%s] Failed : device.NULL\n", __func__);
		return -ENXIO;
	}

	ret = of_property_read_u32(device->of_node, "irq-num",
		(unsigned int *)(&dev->irq_num));
	if (ret < 0) {
		pr_err("[%s] Failed: irq-num.%d\n", __func__, ret);
		return -EINVAL;
	}

	pr_info("[%s] Hisp irq_num.%d\n", __func__, dev->irq_num);

	min = hipp_min(MAX_HISP_CPE_IRQ, dev->irq_num);
	for (i = 0; i < min; i++) {
		irq = hispcpe_get_irq(i);
		if (irq <= 0) {
			pr_err("[%s] Failed : platform_get_irq.%d\n",
				__func__, irq);
			return -EINVAL;
		}

		dev->irq[i] = irq;
		pr_info("[%s] Hisp CPE %d.IRQ.%d\n", __func__, i, dev->irq[i]);
	}

	pr_info("%s: -\n", __func__);
	return ISP_IPP_OK;
}

static int hispcpe_getdts_reg(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	int ret;
	unsigned int i = 0;
	unsigned int min;

	if ((dev == NULL) || (dev->pdev == NULL)) {
		pr_err("[%s] Failed : dev or pdev.NULL\n", __func__);
		return -ENXIO;
	}

	device = &dev->pdev->dev;

#if defined(CONFIG_IPP_DEBUG) && defined(IPP_KERNEL_USE_PCIE_VERIFY)
	ret = of_property_read_u32(device->of_node, "ipp-pcie-flag",
		(unsigned int *)(&dev->pci_flag));
	pr_info("[%s] ipp-pcie-flag.%d\n", __func__, ret);
	if (ret < 0)
		pr_err("[%s] Failed: ipp-pcie-flag.%d\n", __func__, ret);
#endif

	ret = of_property_read_u32(device->of_node, "reg-num",
		(unsigned int *)(&dev->reg_num));
	if (ret < 0) {
		pr_err("[%s] Failed: reg-num.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("[%s] Hipp reg_num.%d\n", __func__, dev->reg_num);

	min = hipp_min(MAX_HISP_CPE_REG, dev->reg_num);
	for (i = 0; i < min; i++) {
		dev->r[i] = platform_get_resource(dev->pdev, IORESOURCE_MEM, i);
		if (dev->r[i] == NULL) {
			pr_err("[%s] Failed : platform_get_resource.%pK\n",
				__func__, dev->r[i]);
			return -ENXIO;
		}
	}

	ret = of_property_read_u32(device->of_node, "sid-num",
		(unsigned int *)(&dev->sid));
	if (ret < 0) {
		pr_err("[%s] Failed: ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	ret = of_property_read_u32(device->of_node, "ssid-num",
		(unsigned int *)(&dev->ssid));
	if (ret < 0) {
		pr_err("[%s] Failed: ret.%d\n", __func__, ret);
		return -EINVAL;
	}

	pr_info("%s: -\n", __func__);
	return ISP_IPP_OK;
}

static int hispcpe_getdts(struct hispcpe_s *dev)
{
	int ret;

	ret = hispcpe_getdts_irq(dev);
	if (ret != 0) {
		pr_err("[%s] Failed : irq.%d\n", __func__, ret);
		return ret;
	}

	ret = hispcpe_getdts_reg(dev);
	if (ret != 0) {
		pr_err("[%s] Failed : reg.%d\n", __func__, ret);
		return ret;
	}

	return ISP_IPP_OK;
}

static int hispcpe_attach_misc(struct hispcpe_s *dev,
	struct miscdevice *mdev)
{
	pr_info("%s: +\n", __func__);

	if (dev == NULL || mdev == NULL) {
		pr_err("[%s] Failed : dev.%pK, mdev.%pK\n",
			__func__, dev, mdev);
		return -EINVAL;
	}

	dev->miscdev = hispcpe_miscdev;
	pr_info("%s: -\n", __func__);
	return ISP_IPP_OK;
}

static int hipp_client_probe(struct platform_device *pdev)
{
	int ret;

	ret = hipp_adapter_probe(pdev);
	if (ret < 0) {
		pr_err("[%s] Failed : hipp_adapter_probe.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static void hipp_client_remove(void)
{
	hipp_adapter_remove();
}

static int hispcpe_probe(struct platform_device *pdev)
{
	struct hispcpe_s *dev = NULL;
	int ret;

	pr_info("[%s] +\n", __func__);

	ret = hispcpe_is_bypass_by_soc_spec();
	if (ret != 0) {
		pr_err("[%s] Failed : bypass ipp.\n", __func__);
		return ISP_IPP_OK;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret < 0) {
		pr_err("[%s] Failed : dma_set\n", __func__);
		return ret;
	}

	dev = kzalloc(sizeof(struct hispcpe_s), GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;

	dev->pdev = pdev;
	platform_set_drvdata(pdev, dev);

	ret = hipp_client_probe(pdev);
	if (ret < 0) {
		pr_err("[%s] Failed : hipp_adapter_probe.%d\n", __func__, ret);
		goto free_dev;
	}

	ret = hispcpe_getdts(dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hispcpe_getdts.%d\n", __func__, ret);
		goto clean_ipp_client;
	}

#if defined(CONFIG_IPP_DEBUG) && defined(IPP_KERNEL_USE_PCIE_VERIFY)
	ret = hispcpe_pcie_init(&pdev->dev, dev->pci_flag);
	if (ret != 0)
		pr_err("[%s] Failed : hispcpe_pcie_init\n", __func__);

	hispcpe_pcie_ioremap_reg(dev);
#endif
	ret = hispcpe_resource_init(dev);
	if (ret != 0) {
		pr_err("[%s] Failed : resource.%d\n", __func__, ret);
		goto clean_ipp_client;
	}

	dev->initialized = 0;
	hispcpe_attach_misc(dev, &hispcpe_miscdev);

	ret = misc_register((struct miscdevice *)&dev->miscdev);
	if (ret != 0) {
		pr_err("[%s] Failed : misc_register.%d\n", __func__, ret);
		goto clean_ipp_client;
	}

	dev->open_refs = 0;
	mutex_init(&dev->open_lock);
	mutex_init(&dev->dev_lock);

	dev->ipp_wakelock = wakeup_source_register(&pdev->dev, "ipp_wakelock");
	if (!dev->ipp_wakelock) {
		pr_err("ipp_wakelock register fail\n");
		goto clean_misc;
	}
	mutex_init(&dev->ipp_wakelock_mutex);
	dev->initialized = 1;
	hispcpe_dev = dev;
	pr_info("[%s] -\n", __func__);

	return ISP_IPP_OK;

clean_misc:
	misc_deregister(&dev->miscdev);

clean_ipp_client:
	hipp_client_remove();

free_dev:
	kfree(dev);

	return -ENODEV;
}

static int hispcpe_remove(struct platform_device *pdev)
{
	struct hispcpe_s *dev = NULL;
	int ret;
	ret = hispcpe_is_bypass_by_soc_spec();
	if (ret != 0) {
		pr_err("[%s] Failed : bypass ipp.\n", __func__);
		return ISP_IPP_OK;
	}

	dev = (struct hispcpe_s *)platform_get_drvdata(pdev);
	if (dev == NULL) {
		pr_err("[%s] Failed : drvdata, pdev.%pK\n", __func__, pdev);
		return -ENODEV;
	}

	hispcpe_unregister_irq(dev);

#if defined(CONFIG_IPP_DEBUG) && defined(IPP_KERNEL_USE_PCIE_VERIFY)
	hispcpe_pcie_deinit();
#endif
	misc_deregister(&dev->miscdev);
	dev->open_refs = 0;

	wakeup_source_unregister(dev->ipp_wakelock);
	mutex_destroy(&dev->ipp_wakelock_mutex);
	mutex_destroy(&dev->open_lock);
	mutex_destroy(&dev->dev_lock);

	hipp_client_remove();

	dev->initialized = 0;
	kfree(dev);
	dev = NULL;

	return ISP_IPP_OK;
}

#ifdef CONFIG_OF
static const struct of_device_id hisiipp_of_id[] = {
	{.compatible = DTS_NAME_HISI_IPP},
	{}
};
#endif

static struct platform_driver hispcpe_pdrvr = {
	.probe          = hispcpe_probe,
	.remove         = hispcpe_remove,
	.driver         = {
		.name           = "hisiipp",
		.owner          = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(hisiipp_of_id),
#endif
	},
};

module_platform_driver(hispcpe_pdrvr);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisilicon ISP CPE Driver");
MODULE_AUTHOR("ipp");

// end-of-file
