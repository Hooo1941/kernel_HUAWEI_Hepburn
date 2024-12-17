/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * Description:
 * smmu test for smmu driver implementations based on ARM architected
 * SMMU.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Create: 2019-12-6
 */

#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-iommu.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_iommu.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/dma-heap.h>
#include "mm_smmu.h"

static struct platform_device *test_pdev;

static int mm_vsmmu_test_probe(struct platform_device *pdev)
{
	pr_err("smmu test probe\n");

	if (!test_pdev) {
		test_pdev = pdev;
		pr_err("%s %d:test dev record\n", __func__, __LINE__);
	}
	dma_set_mask_and_coherent(&pdev->dev, ~0ULL);

	pr_err("dev %pK (%s)\n", &pdev->dev, dev_name(&pdev->dev));

	pr_err("smmu test probe succ\n");

	return 0;
}

static int mm_vsmmu_test_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mm_vsmmu_test_of_match[] = {
	{.compatible = "mm,smmu_test" },
	{},
};
MODULE_DEVICE_TABLE(of, mm_vmmu_test_of_match);

static struct platform_driver mm_vsmmu_test_driver = {
	.driver = {
			.owner = THIS_MODULE,
			.name = "smmu-test",
			.of_match_table = of_match_ptr(mm_vsmmu_test_of_match),
		},
	.probe = mm_vsmmu_test_probe,
	.remove = mm_vsmmu_test_remove,
};

static int __init mm_vsmmu_test_init(void)
{
	int ret = 0;
	pr_err("vsmmu test init\n");
	ret = platform_driver_register(&mm_vsmmu_test_driver);

	return ret;
}

static void __exit mm_vsmmu_test_exit(void)
{
	platform_driver_unregister(&mm_vsmmu_test_driver);
}

module_init(mm_vsmmu_test_init);
module_exit(mm_vsmmu_test_exit);
