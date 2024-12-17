/*
 * QIC V100 init functions source file.
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/arm-smccc.h>
#include <linux/compiler.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include "../dfx_sec_qic.h"
#include "dfx_sec_qic_err_probe.h"
#include "../dfx_sec_qic_dump.h"
#include "dfx_sec_qic_init.h"
#include "securec.h"
#include "platform_include/see/bl31_smc.h"
#include "../common/sec_qic_share_info.h"
#ifdef CONFIG_PLATDRV_SUPPORT_FFA
#include <platform_include/see/ffa/ffa_plat_drv.h>
#endif

static int dfx_sec_qic_get_master_info(const struct device_node *qic_node, struct platform_device *pdev,
				   struct qic_master_info **master_info, u32 *master_num, const char *name)
{
	const struct device_node *np = NULL;
	const struct property *prop = NULL;
	const __be32 *val = NULL;
	u32 i;
	size_t cnt;

	np = of_find_compatible_node((struct device_node *)qic_node, NULL, name);
	if (unlikely(!np)) {
		pr_err("[%s] cannot get %s node\n", __func__, name);
		return -ENODEV;
	}

	if (of_property_read_u32(np, "target_num", master_num) != 0) {
		dev_err(&pdev->dev, "[%s] get %s target_num from DTS error.\n", __func__, name);
		return -ENODEV;
	}
	*master_info = devm_kzalloc(&pdev->dev,
		*master_num * sizeof(*(*master_info)), GFP_KERNEL);
	if (unlikely(!(*master_info))) {
		dev_err(&pdev->dev, "[%s] alloc mid info fail\n", __func__);
		return -ENOMEM;
	}

	prop = of_find_property(np, "target_list", NULL);
	if (!prop || !(prop->value)) {
		pr_err("get target_list fail\n");
		return -ENODEV;
	}

	val = prop->value;
	for (i = 0; i < *master_num; i++) {
		(*master_info)[i].masterid_value = be32_to_cpup(val++);
		(*master_info)[i].masterid_name = (const char *)val;
		cnt = strlen((const char *)val);
		val = (__be32 *)((char *)val + cnt + 1);
	}
	return 0;
}

static int dfx_sec_qic_get_mid_and_acpucore_info(const struct device_node *qic_node, struct platform_device *pdev)
{
	int ret;
	struct dfx_sec_qic_device *qic_dev = platform_get_drvdata(pdev);

	if (unlikely(!qic_dev)) {
		dev_err(&pdev->dev, "cannot get qic dev\n");
		return -ENODEV;
	}

	ret = dfx_sec_qic_get_master_info(qic_node, pdev, &(qic_dev->mid_info),
				      &(qic_dev->mid_info_num), "dfx,sec_qic_mid_info");
	if (ret) {
		dev_err(&pdev->dev, "cannot get master info\n");
		return ret;
	}

	return dfx_sec_qic_get_master_info(qic_node, pdev, &(qic_dev->acpu_core_info),
				       &(qic_dev->acpu_core_info_num), "dfx,sec_qic_acpu_core_info");
}

static int dfx_sec_qic_get_detail_bus_info(const struct device_node *businfo_node, struct dfx_sec_qic_device *qic_dev)
{
	const char **name = NULL;
	u32 *bus_key = NULL;
	u32 *mid_offset = NULL;
	u32 i;
	int ret = 0;

	name = kmalloc(qic_dev->bus_info_num * sizeof(*name), GFP_KERNEL);
	if (unlikely(ZERO_OR_NULL_PTR((u64)(uintptr_t)name))) {
		pr_err("[%s] alloc bus name fail\n", __func__);
		return -ENOMEM;
	}
	ret = of_property_read_string_array(businfo_node, "bus_name", name, qic_dev->bus_info_num);
	if (ret < 0) {
		pr_err("[%s] get bus name from DTS error,ret:%d\n", __func__, ret);
		goto out;
	}

	bus_key = kmalloc(qic_dev->bus_info_num * sizeof(*bus_key), GFP_KERNEL);
	if (unlikely(ZERO_OR_NULL_PTR((u64)(uintptr_t)bus_key))) {
		pr_err("[%s] alloc bus key fail\n", __func__);
		ret = -ENOMEM;
		goto out;
	}
	ret = of_property_read_u32_array(businfo_node, "bus_key", bus_key, qic_dev->bus_info_num);
	if (ret) {
		pr_err("[%s] get bus key from DTS error,ret:%d\n", __func__, ret);
		goto out;
	}

	mid_offset = kmalloc(qic_dev->bus_info_num * sizeof(*mid_offset), GFP_KERNEL);
	if (unlikely(ZERO_OR_NULL_PTR((u64)(uintptr_t)mid_offset))) {
		pr_err("[%s] alloc mid_offset fail\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	ret = of_property_read_u32_array(businfo_node, "mid_offset", mid_offset, qic_dev->bus_info_num);
	if (ret) {
		pr_err("[%s] get mid_offset from DTS error, use default value\n", __func__);
		goto out;
	}

	for (i = 0; i < qic_dev->bus_info_num; i++) {
		qic_dev->bus_info[i].bus_key = bus_key[i];
		qic_dev->bus_info[i].bus_name = name[i];
		if (mid_offset)
			qic_dev->bus_info[i].mid_offset = mid_offset[i];
		else
			qic_dev->bus_info[i].mid_offset = QIC_DEFAULT_MID_OFFSET;
	}
out:
	if (name)
		kfree(name);
	if (bus_key)
		kfree(bus_key);
	if (mid_offset)
		kfree(mid_offset);
	return ret;
}

static int dfx_sec_qic_get_bus_info(const struct device_node *qic_node, struct platform_device *pdev)
{
	const struct device_node *np = NULL;
	struct dfx_sec_qic_device *qic_dev = platform_get_drvdata(pdev);

	if (unlikely(!qic_dev)) {
		dev_err(&pdev->dev, "cannot get qic dev\n");
		return -ENODEV;
	}

	np = of_find_compatible_node((struct device_node *)qic_node, NULL, "dfx,sec_qic_bus_info");
	if (unlikely(!np)) {
		pr_err("[%s] cannot get sec_qic_bus_info node\n", __func__);
		return -ENODEV;
	}

	if (of_property_read_u32(np, "bus_num", &(qic_dev->bus_info_num)) != 0) {
		dev_err(&pdev->dev, "[%s] get bus_num from DTS error.\n", __func__);
		return -ENODEV;
	}
	qic_dev->bus_info = devm_kzalloc(&pdev->dev,
		qic_dev->bus_info_num * sizeof(*(qic_dev->bus_info)), GFP_KERNEL);
	if (unlikely(!(qic_dev->bus_info))) {
		dev_err(&pdev->dev, "[%s] alloc bus info fail\n", __func__);
		return -ENOMEM;
	}

	return dfx_sec_qic_get_detail_bus_info(np, qic_dev);
}

static int dfx_sec_qic_get_stop_cpu_buses(const struct device_node *qic_node, struct platform_device *pdev)
{
	struct dfx_sec_qic_device *qic_dev = platform_get_drvdata(pdev);

	if (unlikely(!qic_dev)) {
		dev_err(&pdev->dev, "cannot get qic dev\n");
		return -ENODEV;
	}

	if (of_property_read_u32(qic_node, "stop_cpu_bus_num", &(qic_dev->need_stop_cpu_buses_num)) != 0) {
		dev_err(&pdev->dev, "[%s] get stop_cpu_bus_num from DTS error.\n", __func__);
		return -ENODEV;
	}

	qic_dev->need_stop_cpu_buses = devm_kzalloc(&pdev->dev,
		qic_dev->need_stop_cpu_buses_num * sizeof(*(qic_dev->need_stop_cpu_buses)), GFP_KERNEL);
	if (unlikely(!(qic_dev->need_stop_cpu_buses))) {
		dev_err(&pdev->dev, "[%s] alloc buses fail\n", __func__);
		return -ENOMEM;
	}

	if (of_property_read_u32_array(qic_node, "stop_cpu_bus",
		qic_dev->need_stop_cpu_buses, qic_dev->need_stop_cpu_buses_num) != 0) {
		dev_err(&pdev->dev, "[%s] get stop_cpu_bus from DTS error\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static int dfx_sec_qic_ib_timeout_enable(const struct device_node *qic_node, struct platform_device *pdev)
{
	struct dfx_sec_qic_device *qic_dev = platform_get_drvdata(pdev);
	int ret;
	u32 i;
	u32 j;
	u32 value;

	if (unlikely(!qic_dev)) {
		dev_err(&pdev->dev, "cannot get qic dev\n");
		return -ENODEV;
	}

	ret = dfx_sec_qic_get_component_reg(qic_node, pdev, &(qic_dev->timeout_en_reg),
		&(qic_dev->timeout_en_reg_num), "dfx,sec_qic_timeout_en_reg");
	if (ret) {
		dev_err(&pdev->dev, "get timeout enable regs fail\n");
		return -ENODEV;
	}

	for (i = 0; i < qic_dev->timeout_en_reg_num; i++) {
		for (j = 0; j < qic_dev->timeout_en_reg[i].len; j++) {
			value = (qic_dev->timeout_en_reg[i].reg_mask[j] >> QIC_TIMEOUT_EN_OFFSET) +
				qic_dev->timeout_en_reg[i].reg_mask[j];
			writel_relaxed(value, qic_dev->timeout_en_reg[i].reg_base +
				       qic_dev->timeout_en_reg[i].offsets[j]);
		}
	}

	return 0;
}

int dfx_sec_qic_smc_call(u64 function_id, u64 bus_key)
{
#ifdef CONFIG_PLATDRV_SUPPORT_FFA
	int        ret;
	struct ffa_send_direct_data args = {0};
	args.data0 = MNTN_QIC_FID_VALUE;
	args.data1 = function_id;
	args.data2 = 0;
	args.data3 = bus_key;

	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0) {
		pr_err("%s:qic:ffa send fail:%d\n", __func__, ret);
		return ret;
	}
#else
	struct arm_smccc_res res;

	arm_smccc_smc((u64)MNTN_QIC_FID_VALUE, function_id, 0, bus_key, 0, 0, 0, 0, &res);
	if (res.a0)
		return -1;
#endif
	return 0;
}

static void dfx_sec_qic_unmap_reg(struct qic_src_reg *qic_reg, u32 reg_num)
{
	u32 i;

	for (i = 0; i < reg_num; i++) {
		if (qic_reg[i].reg_base) {
			iounmap(qic_reg[i].reg_base);
			qic_reg[i].reg_base = NULL;
		}
	}
}

void dfx_sec_qic_free_source(struct dfx_sec_qic_device *qic_dev)
{
	if (!qic_dev) {
		pr_err("no need to free\n");
		return;
	}

	dfx_sec_qic_unmap_reg(qic_dev->irq_reg, qic_dev->irq_reg_num);
	dfx_sec_qic_unmap_reg(qic_dev->timeout_en_reg, qic_dev->timeout_en_reg_num);
	if (qic_dev->qic_share_base) {
		iounmap(qic_dev->qic_share_base);
		qic_dev->qic_share_base = NULL;
	}
	if (qic_dev->gic_ctrl_base) {
		iounmap(qic_dev->gic_ctrl_base);
		qic_dev->gic_ctrl_base = NULL;
	}
}

int dfx_sec_qic_common_init(const struct device_node *np, struct platform_device *pdev)
{
	int ret;
	struct dfx_sec_qic_device *qic_dev = NULL;
	struct device_node *np_addr = NULL;
	u32 bl31_share_mem_base;

	ret = dfx_sec_qic_smc_call(QIC_INIT_FIQ_SUBTYPE, 0);
	if (ret)
		goto qic_init_err;

	ret = dfx_sec_qic_dump_init();
	if (ret)
		goto qic_init_err;

	qic_dev = platform_get_drvdata(pdev);
	if (qic_dev == NULL)
		goto qic_init_err;

	np_addr = of_find_compatible_node(NULL, NULL, "hisilicon,bl31_share_address");
	if (!np_addr) {
		pr_err("%s: no compatible bl31 mntn node found\n", __func__);
		goto qic_init_err;
	}

	ret = of_property_read_u32(np_addr, "base_addr", &bl31_share_mem_base);
	if (ret) {
		pr_err("%s , get bl31 share base addr err\n", __func__);
		goto qic_init_err;
	}

	qic_dev->qic_share_base = ioremap_wc(bl31_share_mem_base + QIC_SHARE_MEM_OFFSET, QIC_SHARE_MEM_SIZE);
	if (!qic_dev->qic_share_base)
		goto qic_init_err;

	np_addr = of_find_compatible_node(NULL, NULL, "arm,lp_gic");
	if (!np_addr) {
		pr_err("%s: no compatible gicv600 node found\n", __func__);
		goto qic_init_err;
	}

	qic_dev->gic_ctrl_base = of_iomap(np_addr, 0);
	if (qic_dev->gic_ctrl_base == NULL)
		goto qic_init_err;

	ret = dfx_sec_qic_ib_timeout_enable(np, pdev);
	if (ret)
		goto qic_init_err;

	ret = dfx_sec_qic_get_stop_cpu_buses(np, pdev);
	if (ret)
		goto qic_init_err;

	ret = dfx_sec_qic_get_bus_info(np, pdev);
	if (ret)
		goto qic_init_err;

	ret = dfx_sec_qic_get_mid_and_acpucore_info(np, pdev);
	if (ret)
		goto qic_init_err;

	return 0;
qic_init_err:
	dev_err(&pdev->dev, "qic common init fail!");
	return ret;
}
