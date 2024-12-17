/*
 * driver, hisp_rproc.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>
#include <linux/amba/bus.h>
#include <linux/dma-mapping.h>
#include <linux/remoteproc.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>
#include <linux/random.h>
#include <linux/iommu.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/miscdevice.h>
#include <platform_include/isp/linux/hisp_remoteproc.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <securec.h>
#include "hisp_internel.h"
#include "isp_ddr_map.h"

#define ISP_MEM_SIZE              0x10000
#ifdef HISP_FUZZ
#define PART_ISPFW_SIZE           0x02000000
#else
#define PART_ISPFW_SIZE           0x00E00000
#endif
#define ISP_BIN_RSC_OFFSET        0x3000
#define HISP_BW_SHRMEM_OFFSET     0xF000

struct hisp_nsec {
	unsigned int vqda;
	dma_addr_t isp_bw_pa;
	u64 pgd_base;
	void *isp_bw_va;
	struct device *device;
	struct platform_device *isp_pdev;
#ifdef DEBUG_HISP
	struct isp_pcie_cfg *isp_pcie_cfg;
#endif
	struct mutex pwrlock;
};

static struct hisp_nsec hisp_nsec_dev;

#ifdef DEBUG_HISP
#include "hisp_pcie.h"

static struct isp_pcie_cfg nsec_isp_pcie_cfg;

struct isp_pcie_cfg *hisp_get_pcie_cfg(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	if (dev->isp_pcie_cfg == NULL) {
		pr_err("[%s] Failed: isp pcie cfg not set\n", __func__);
		return NULL;
	}

	return dev->isp_pcie_cfg;
}

int hisp_check_pcie_stat(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	if (dev->isp_pcie_cfg->isp_pci_inuse_flag != ISP_PCIE_MODE)
		return ISP_NORMAL_MODE;

	if (dev->isp_pcie_cfg->isp_pci_ready != ISP_PCIE_READY) {
		pr_err("[%s] isp in pcie mode, but pcie not ready\n", __func__);
		return ISP_PCIE_NREADY;
	}
	pr_info("[%s] isp pcie inuse, status ready\n", __func__);
	return ISP_PCIE_MODE;
}

static int hisp_pcie_getdts(struct device_node *np)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	int ret;

	ret = of_property_read_u32(np, "isp-usepcie-flag", &dev->isp_pcie_cfg->isp_pci_inuse_flag);
	if (ret < 0)
		pr_err("[%s] Failed: isp_pci_inuse_flag.%d\n", __func__, ret);

	pr_info("enable_pcie_flag.0x%x of_property_read_u32.%d\n",
			dev->isp_pcie_cfg->isp_pci_inuse_flag, ret);

	return 0;
}
#endif

u64 hisp_nsec_get_remap_pa(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	if (dev->isp_bw_pa)
		return dev->isp_bw_pa;

	return 0;
}

void* hisp_nsec_get_remap_va(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	return dev->isp_bw_va;
}

u64 hisp_get_ispcpu_shrmem_nsecpa(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	if (dev->isp_bw_pa == 0)
		return 0;

	return (dev->isp_bw_pa + HISP_BW_SHRMEM_OFFSET);
}

void *hisp_get_ispcpu_shrmem_nsecva(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	if (dev->isp_bw_va == NULL)
		return NULL;

	return (dev->isp_bw_va + HISP_BW_SHRMEM_OFFSET);
}

void hisp_nsec_boot_dump(struct rproc *rproc, unsigned int size)
{
#ifdef DEBUG_HISP
	void *va = NULL;
	unsigned int i = 0;

	if (hisp_sec_boot_mode())
		return;

	va = hisp_rproc_get_imgva(rproc);
	pr_info("[%s] addr.0x%pK, size.0x%x\n", __func__, va, size);
	for (i = 0; i < size; i += 4)
		pr_info("0x%08x 0x%08x 0x%08x 0x%08x\n",
			*((unsigned int *)va + i + 0), *((unsigned int *)va + i + 1),
			*((unsigned int *)va + i + 2), *((unsigned int *)va + i + 3));
#endif
}

int hisp_nsec_jpeg_powerup(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	int ret = 0;

	pr_info("[%s] +\n", __func__);
	mutex_lock(&dev->pwrlock);
	ret = hisp_pwr_subsys_powerup();
	if (ret != 0) {
		pr_err("[%s] Failed : ispup.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}

	ret = hisp_ispcpu_qos_cfg();
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_ispcpu_qos_cfg.%d\n",
				__func__, ret);
		goto isp_down;
	}
#ifndef CONFIG_HISP_FFA_SUPPORT
	hisp_smc_set_nonsec();
#endif
	ret = hisp_pwr_core_nsec_init(dev->pgd_base);
	if (ret != 0) {
		pr_err("[%s] hisp_pwr_core_nsec_init.%d\n", __func__, ret);
		goto isp_down;
	}
	mutex_unlock(&dev->pwrlock);
	pr_info("[%s] -\n", __func__);

	return 0;

isp_down:
	if ((hisp_pwr_subsys_powerdn()) != 0)
		pr_err("[%s] Failed : ispdn\n", __func__);

	mutex_unlock(&dev->pwrlock);
	pr_info("[%s] -\n", __func__);

	return ret;
}

/* lint -save -e631 -e613 */
int hisp_nsec_jpeg_powerdn(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	int ret;

	pr_info("%s: +\n", __func__);
	mutex_lock(&dev->pwrlock);
	ret = hisp_pwr_core_nsec_exit();
	if (ret != 0)
		pr_err("%s: hisp_pwr_core_nsec_exit failed, ret.%d\n",
			__func__, ret);

	ret = hisp_pwr_subsys_powerdn();
	if (ret != 0)
		pr_err("%s: hisp_pwr_subsys_powerdn failed, ret.%d\n",
			__func__, ret);

	mutex_unlock(&dev->pwrlock);
	pr_info("%s: -\n", __func__);

	return 0;
}
/* lint -restore */

int hisp_nsec_device_enable(struct rproc *rproc)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	unsigned int canary = 0;
	int ret;

	mutex_lock(&dev->pwrlock);
	ret = hisp_pwr_subsys_powerup();
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_pwr_subsys_powerup.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}

	ret = hisp_ispcpu_qos_cfg();
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_ispcpu_qos_cfg.%d\n", __func__, ret);
		goto isp_down;
	}
#ifndef CONFIG_HISP_FFA_SUPPORT
	hisp_smc_set_nonsec();
#endif
#ifdef ISP_LINX_CPU_SUPPORT
	dev->pgd_base = kernel_domain_get_ttbr(dev->device);
#endif
	ret = hisp_pwr_core_nsec_init(dev->pgd_base);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_pwr_core_nsec_init.%d\n",
			__func__, ret);
		goto isp_down;
	}

	get_random_bytes(&canary, sizeof(canary));
	ret = hisp_wait_rpmsg_completion(rproc);
	if (ret != 0)
		goto boot_exit;

	ret = hisp_pwr_cpu_nsec_dst(dev->isp_bw_pa, canary);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_pwr_cpu_nsec_dst.%d\n",
			__func__, ret);
		goto boot_exit;
	}
	mutex_unlock(&dev->pwrlock);
#ifdef CONFIG_HISP_DPM
	hisp_dpm_probe();
#endif

	return 0;

boot_exit:
	if (hisp_pwr_core_nsec_exit())
		pr_err("[%s] Failed : hisp_pwr_core_nsec_exit\n", __func__);
isp_down:
	if (hisp_pwr_subsys_powerdn())
		pr_err("[%s] Failed : hisp_pwr_subsys_powerdn\n", __func__);

	mutex_unlock(&dev->pwrlock);

	return ret;
}

int hisp_nsec_device_disable(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	int ret;

#ifdef CONFIG_HISP_DPM
	hisp_dpm_release();
#endif
	mutex_lock(&dev->pwrlock);

	ret = hisp_pwr_cpu_nsec_rst();
	if (ret != 0)
		pr_err("[%s] hisp_pwr_cpu_nsec_rst failed, ret.%d\n",
			__func__, ret);

	ret = hisp_pwr_core_nsec_exit();
	if (ret != 0)
		pr_err("[%s] hisp_pwr_core_nsec_exit failed, ret.%d\n",
			__func__, ret);

	ret = hisp_pwr_subsys_powerdn();
	if (ret != 0)
		pr_err("[%s] hisp_pwr_subsys_powerdn failed, ret.%d\n",
			__func__, ret);
	mutex_unlock(&dev->pwrlock);

	return 0;
}
EXPORT_SYMBOL(hisp_nsec_device_disable);

u64 hisp_nsec_get_pgd(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	return dev->pgd_base;
}

int hisp_nsec_set_pgd(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	struct hisp_shared_para *param = NULL;

	hisp_lock_sharedbuf();
	param = hisp_share_get_para();
	if (param == NULL) {
		pr_err("[%s] Failed : param.%pK\n", __func__, param);
		hisp_unlock_sharedbuf();
		return -EINVAL;
	}
	param->dynamic_pgtable_base = dev->pgd_base;
	hisp_unlock_sharedbuf();

	return 0;
}

static int hisp_remap_rsc(struct hisp_nsec *dev)
{
	dev->isp_bw_va = dma_alloc_coherent(dev->device, ISP_MEM_SIZE,
					&dev->isp_bw_pa, GFP_KERNEL);
	if (dev->isp_bw_va == NULL) {
		pr_err("[%s] isp_bw_va failed\n", __func__);
		return -ENOMEM;
	}
	pr_info("[%s] isp_bw_va.%pK\n", __func__, dev->isp_bw_va);

	return 0;
}

static void hisp_unmap_rsc(struct hisp_nsec *dev)
{
	if (dev->isp_bw_va != NULL)
		dma_free_coherent(dev->device, ISP_MEM_SIZE,
				dev->isp_bw_va, dev->isp_bw_pa);

	dev->isp_bw_va = NULL;
}

static int hisp_nsec_getdts(struct platform_device *pdev,
		struct hisp_nsec *dev)
{
	struct device *device = &pdev->dev;
	struct device_node *np = device->of_node;
	int ret;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);

	ret = of_property_read_u32(np, "isp-ipc-addr", &dev->vqda);
	if (ret < 0) {
		pr_err("[%s] Failed: vqda.0x%x of_property_read_u32.%d\n",
			__func__, dev->vqda, ret);
		return ret;
	}

	pr_info("[%s] vqda.0x%x\n", __func__, dev->vqda);

#ifdef DEBUG_HISP
	ret = hisp_pcie_getdts(np);
	if (ret < 0) {
		pr_err("Failed : hisp_pcie_getdts.%d\n", ret);
		return ret;
	}
#endif

	pr_info("[%s] -\n", __func__);

	return 0;
}

#ifdef DEBUG_HISP
static int hisp_nsec_pcie_probe(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	int ret;

	if (dev->isp_pcie_cfg->isp_pci_inuse_flag == 0)
		return ISP_NORMAL_MODE;

	dev->isp_pcie_cfg->isp_pci_ready = ISP_PCIE_NREADY;

	ret = hisp_pcie_register();
	if (ret != 0) {
		pr_err("[%s] Failed : hispcpe_pcie_register\n", __func__);
		goto clean_isp_pcie;
	}

	ret = hisp_get_pcie_outbound(dev->isp_pcie_cfg);
	if (ret != ISP_PCIE_READY) {
		pr_err("[%s] Failed : hisp_get_pcie_outbound\n", __func__);
		goto clean_isp_pcie;
	}

	dev->isp_pcie_cfg->isp_pci_reg = ioremap(dev->isp_pcie_cfg->isp_pci_addr, dev->isp_pcie_cfg->isp_pci_size);

	pr_info("[%s] pci reg VA: 0x%pK\n", __func__, (uintptr_t)dev->isp_pcie_cfg->isp_pci_reg);

	return ISP_PCIE_MODE;

clean_isp_pcie:
	hisp_pcie_unregister();
	return ret;
}

static void hisp_nsec_pcie_remove(void)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	if (dev->isp_pcie_cfg->isp_pci_inuse_flag == 0)
		return;

	iounmap(dev->isp_pcie_cfg->isp_pci_reg);
	(void)hisp_pcie_unregister();
	dev->isp_pcie_cfg->isp_pci_ready = ISP_PCIE_NREADY;
}
#endif

int hisp_nsec_probe(struct platform_device *pdev)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;
	int ret;

	pr_alert("[%s] +\n", __func__);
	dev->device = &pdev->dev;
	dev->isp_pdev = pdev;

#ifdef DEBUG_HISP
	dev->isp_pcie_cfg = &nsec_isp_pcie_cfg;
#endif

	ret = hisp_remap_rsc(dev);
	if (ret != 0) {
		pr_err("[%s] failed, isp_remap_src.%d\n", __func__, ret);
		return ret;
	}

	ret = hisp_nsec_getdts(pdev, dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_nsec_getdts.%d.\n",
				__func__, ret);
		goto hisp_nsec_getdts_fail;
	}

	dev->pgd_base = kernel_domain_get_ttbr(dev->device);
#ifdef DEBUG_HISP
	ret = hisp_nsec_pcie_probe();
	if (ret == ISP_PCIE_NREADY)
		goto hisp_nsec_getdts_fail;
#endif

	mutex_init(&dev->pwrlock);
	pr_alert("[%s] -\n", __func__);

	return 0;

loadbin_fail:
#ifdef DEBUG_HISP
	hisp_nsec_pcie_remove();
#endif

hisp_nsec_getdts_fail:
	hisp_unmap_rsc(dev);

	return ret;
}

int hisp_nsec_remove(struct platform_device *pdev)
{
	struct hisp_nsec *dev = &hisp_nsec_dev;

	(void)pdev;

	mutex_destroy(&dev->pwrlock);

#ifdef DEBUG_HISP
	hisp_nsec_pcie_remove();
#endif
	hisp_unmap_rsc(dev);
	return 0;
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HiStar nsec device driver");

#ifdef DEBUG_HISP
#define SECU_MAX_SIZE 64
#define TMP_MAX_SIZE 0x1000

static int hisp_boot_mode_show_part(char *tmp, ssize_t *size)
{
	int ret;

	ret = snprintf_s(tmp, SECU_MAX_SIZE, SECU_MAX_SIZE - 1,
			"\nISP Types(Current ISP Type:%u):\n",
			hisp_get_boot_mode());
	if (ret < 0) {
		pr_err("[%s] Failed : Current ISP Type\n", __func__);
		return ret;
	}
	*size += ret;

	ret = snprintf_s(tmp + *size, SECU_MAX_SIZE, SECU_MAX_SIZE - 1, "%s\n",
		"0. SEC_CASE(if current platform support, e.g. V120)");
	if (ret < 0) {
		pr_err("[%s] Failed : SEC_CASE\n", __func__);
		return ret;
	}
	*size += ret;

	ret = snprintf_s(tmp + *size, SECU_MAX_SIZE, SECU_MAX_SIZE - 1, "%s\n",
		"1. NONSEC_CASE(if current platform support, e.g. V120)");
	if (ret < 0) {
		pr_err("[%s] Failed : SEC_CASE\n", __func__);
		return ret;
	}
	*size += ret;

	return 0;
}

ssize_t hisp_boot_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t size = 0;
	char *tmp = NULL;
	int ret = 0;

	(void)dev;
	(void)attr;

	pr_info("[%s] +\n", __func__);

	tmp = kzalloc(TMP_MAX_SIZE, GFP_KERNEL);
	if (tmp == NULL)
		return 0;

	ret = hisp_boot_mode_show_part(tmp, &size);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_boot_mode_show_part.%d\n", __func__, ret);
		goto show_err;
	}

	ret = snprintf_s(tmp + size, SECU_MAX_SIZE, SECU_MAX_SIZE - 1, "%s\n",
				"3. INVAL_CASE");
	if (ret < 0) {
		pr_err("[%s] Failed : INVAL_CASE\n", __func__);
		goto show_err;
	}
	size += ret;

	ret = snprintf_s(tmp + size, SECU_MAX_SIZE, SECU_MAX_SIZE - 1, "\n%s\n",
				"ISP Type Set:");
	if (ret < 0) {
		pr_err("[%s] Failed : ISP Type Set\n", __func__);
		goto show_err;
	}
	size += ret;

	ret = snprintf_s(tmp + size, SECU_MAX_SIZE, SECU_MAX_SIZE - 1, "%s\n",
				"e.g. echo SEC_CASE > sec_nsec_isp:");
	if (ret < 0) {
		pr_err("[%s] Failed : sec_nsec_isp\n", __func__);
		goto show_err;
	}
	size += ret;

show_err:
	ret = memcpy_s((void *)buf, TMP_MAX_SIZE, (void *)tmp, TMP_MAX_SIZE);
	if (ret != 0)
		pr_err("[%s] Failed : memcpy_s buf from tmp fail\n", __func__);

	kfree((void *)tmp);

	pr_info("[%s] -\n", __func__);

	return size;
}

ssize_t hisp_boot_mode_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	(void)dev;
	(void)attr;
	pr_info("[%s] +\n", __func__);
	if (strncmp("SEC_CASE", buf, sizeof("SEC_CASE") - 1) == 0) {
		pr_info("%s: SEC_CASE.\n", __func__);
		hisp_set_boot_mode(SEC_CASE);
	} else if (strncmp("NONSEC_CASE", buf, sizeof("NONSEC_CASE") - 1) == 0) {
		pr_info("%s: NONSEC_CASE.\n", __func__);
		hisp_set_boot_mode(NONSEC_CASE);
	} else {
		pr_info("%s: INVAL_CASE.\n", __func__);
	}
	pr_info("[%s] -\n", __func__);

	return (ssize_t)count;
}

static int hisp_update_perf(struct rproc *rproc, void *addr)
{
	int ret = -1;
	char pathname[128] = { 0 };
	struct file *fp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	size_t size = (size_t)hisp_get_perf_size(rproc);
	size_t len;

	if (addr == NULL)
		return -1;

	ret = snprintf_s(pathname, sizeof(pathname), sizeof(pathname) - 1, "%s",
		"/data/vendor/log/isp-log/ispperf.data");
	if (ret < 0) {
		pr_err("[%s]:snprintf_s firmware failed.%d\n", __func__, ret);
		return ret;
	}

	/* get resource */
	fp = filp_open(pathname, O_CREAT | O_RDWR, 0660);
	if (IS_ERR(fp)) {
		pr_err("[%s] : filp_open(%s) failed\n", __func__, pathname);
		return -ENOENT;
	}

	ret = vfs_llseek(fp, 0, SEEK_SET);
	if (ret < 0)
		goto err;
	fs = get_fs();
	set_fs(KERNEL_DS);

	pos = fp->f_pos;
	len = (uint32_t)vfs_write(fp, addr, (size_t)size, &pos);
	if (len != size) {
		pr_err("[%s] failed: len.%d, size.%d\n", __func__, len, size);
		set_fs(fs);
		goto err;
	}
	set_fs(fs);

	filp_close(fp, NULL);
	return 0;

err:
	filp_close(fp, NULL);
	return -1;
}
void hisp_perf_dump(struct rproc *rproc)
{
	void *vaddr = hisp_get_perf_va(rproc);
	int ret;
	pr_info("%s: ISP_MEM_PERF va:0x%x\n", __func__,vaddr);
	ret = hisp_update_perf(rproc, vaddr);
	if (ret < 0)
		pr_err("[%s] failed: ret.%d\n", __func__, ret);
}
#endif
