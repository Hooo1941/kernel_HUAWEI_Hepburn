/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-. All rights reserved.
 *
 * usb smmu
 *
 * add usb smmu
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "usb_smmu.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/iommu/mm_svm.h>
#include <linux/of_address.h>
#include <linux/arm-smccc.h>
#include <linux/bitops.h>
#include <platform_include/basicplatform/linux/hsdt_bus_api.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>
#ifdef CONFIG_HIUSB_SUPPORT_FFA
#include <platform_include/see/ffa/ffa_plat_drv.h>
#include <platform_include/see/ffa/ffa_msg_id.h>
#else
#include <platform_include/see/bl31_smc.h>
#endif

atomic_t tcu_ctrl_cnt = ATOMIC_INIT(0);
atomic_t tbu0_ctrl_cnt = ATOMIC_INIT(0);
atomic_t tbu1_ctrl_cnt = ATOMIC_INIT(0);
atomic_t tbu2_ctrl_cnt = ATOMIC_INIT(0);

static DEFINE_MUTEX(smmu_mutex);
static DEFINE_SPINLOCK(g_tbu_lock);

static void put_smmu_resoure(struct usb_smmu *smmu)
{
	if (!smmu)
		return;

	of_remove_chip_usb_reg_cfg(smmu->usb_smmu_hsdt1_unreset);
	of_remove_chip_usb_reg_cfg(smmu->usb_smmu_tbu_unreset);
	of_remove_chip_usb_reg_cfg(smmu->usb_smmu_hsdt1_reset);
	of_remove_chip_usb_reg_cfg(smmu->usb_smmu_tbu_reset);

	if (smmu->tbu_base)
		iounmap(smmu->tbu_base);
	if (smmu->tbu_sid_base)
		iounmap(smmu->tbu_sid_base);
}

static atomic_t *get_smmu_tbu_ctrl_cnt(struct usb_smmu *smmu)
{
	atomic_t *ctrl_cnt = NULL;

	switch (smmu->tbu_num) {
	case 0:
		ctrl_cnt = &tbu0_ctrl_cnt;
		break;
	case 1:
		ctrl_cnt = &tbu1_ctrl_cnt;
		break;
	case 2:
		ctrl_cnt = &tbu2_ctrl_cnt;
		break;
	default:
		break;
	}
	return ctrl_cnt;
}

static void ecc_detect_handler(void *data)
{
	struct usb_smmu *smmu = (struct usb_smmu *)data;
	unsigned long flags;

	if (!smmu || smmu->tbu_num > MAX_TBU_INDEX) {
		smmu_err("smmu is null or tbu num too large\n");
		return;
	}

	smmu_err("ecc detect_handler in tbu%u\n", smmu->tbu_num);
	/* Clear ecc intr */
	spin_lock_irqsave(&g_tbu_lock, flags);
	if (atomic_read(get_smmu_tbu_ctrl_cnt(smmu)) != 0)
		writel(SMMU_TBU_IRPT_CLR_NS_ECC_1BIT_MSK, smmu->tbu_base + SMMU_TBU_IRPT_CLR_NS_OFFSET);
	spin_unlock_irqrestore(&g_tbu_lock, flags);
	/* reserved for dmd */
#ifdef CONFIG_HSDT_BUS_DOMAIN
	bbox_diaginfo_record(ECC_USB, NULL, "tbu %u ecc detect-bit error", smmu->tbu_num);
#endif
}

static void ecc_multpl_handler(void *data)
{
	struct usb_smmu *smmu = (struct usb_smmu *)data;
	unsigned long flags;

	if (!smmu || smmu->tbu_num > MAX_TBU_INDEX) {
		smmu_err("smmu is null or tbu num too large\n");
		return;
	}

	smmu_err("ecc multpl_handler in tbu%u\n", smmu->tbu_num);
	/* Clear ecc intr */
	spin_lock_irqsave(&g_tbu_lock, flags);
	if (atomic_read(get_smmu_tbu_ctrl_cnt(smmu)) != 0)
		writel(SMMU_TBU_IRPT_CLR_NS_ECC_MULTI_BITS_MSK, smmu->tbu_base + SMMU_TBU_IRPT_CLR_NS_OFFSET);
	spin_unlock_irqrestore(&g_tbu_lock, flags);

	/* reserved for dmd */
#ifdef CONFIG_HSDT_BUS_DOMAIN
	bbox_diaginfo_record(ECC_USB, NULL, "tbu %u ecc multpl-bit error", smmu->tbu_num);
#endif
}

static void ecc_tbu_handler_register(struct usb_smmu *smmu)
{
	int ret;
	static int tbu_num[MAX_TBU_NUM] = {0};

	mutex_lock(&smmu_mutex);
	if (tbu_num[smmu->tbu_num] > 0) {
	    mutex_unlock(&smmu_mutex);
	    return;
	}

	smmu_info("usb ecc tbu%u irq handler register\n", smmu->tbu_num);
	ret = hsdt_bus_ecc_trigger_register(HSDT_BUS_1, DETECT_ECC_IRQ, BIT(smmu->tbu_num),
			ecc_detect_handler, (void *)smmu);
	if (ret < 0)
		smmu_err("tbu%u register one ecc intr fail\n", smmu->tbu_num);

	ret = hsdt_bus_ecc_trigger_register(HSDT_BUS_1, MULTPL_ECC_IRQ, BIT(smmu->tbu_num),
			 ecc_multpl_handler, (void *)smmu);
	if (ret < 0)
		smmu_err("tbu%u register multi ecc intr fail\n", smmu->tbu_num);

	tbu_num[smmu->tbu_num]++;
	mutex_unlock(&smmu_mutex);
}

int get_smmu_info(struct chip_usb_misc_ctrl *misc_ctrl)
{
	struct device *dev = misc_ctrl->dev;
	struct device_node *np = dev->of_node;
	struct usb_smmu *smmu = NULL;
	int ret;
	bool flag = false;

	misc_ctrl_info("start\n");
	if (!np) {
		misc_ctrl_err("np is null\n");
		return 0;
	}

	flag = of_property_read_bool(np, "usb_smmu");
	if (!flag) {
		misc_ctrl_info("not support smmu\n");
		return 0;
	}

	smmu = kzalloc(sizeof(struct usb_smmu), GFP_KERNEL);
	if (!smmu)
		return -ENOMEM;
	smmu->support_smmu = flag;
	smmu->bypass_smmu = !of_property_read_bool(np, "enable_smmu");

	ret = of_property_read_u32(np, "tbu_num", &smmu->tbu_num);
	if (ret < 0 || smmu->tbu_num > MAX_TBU_INDEX) {
		misc_ctrl_err("cannot get tbu_num\n");
		kfree(smmu);
		return ret;
	}

	ret = of_property_read_u32(np, "smmu_id", &smmu->smmu_id);
	if (ret < 0) {
		misc_ctrl_err("cannot get smmu id\n");
		kfree(smmu);
		return ret;
	}

	ret = of_property_read_u32(np, "sid", &smmu->sid);
	if (ret < 0) {
		misc_ctrl_err("cannot get smmmu  sid\n");
		kfree(smmu);
		return ret;
	}

	ret = of_property_read_u32(np, "smmu_secbit_offset", &smmu->sec_bit_offset);
	if (ret < 0) {
		misc_ctrl_err("cannot get smmu_secbit_offset info\n");
		kfree(smmu);
		return ret;
	}

	ret = of_property_read_u32_array(np, "mmussid_offset", (u32 *)smmu->mmusid_off, 2);
	if (ret < 0) {
		misc_ctrl_err("cannot get smmu offset\n");
		kfree(smmu);
		return ret;
	}

	if (!smmu->bypass_smmu) {
		ret = of_property_read_u32(np, "portid", &smmu->port_id);
		if (ret < 0) {
			smmu_err("cannot get port_id info\n");
			kfree(smmu);
			return ret;
		}
	}

	smmu->tbu_base = of_iomap(np, 1);
	smmu->tbu_sid_base = of_iomap(np, 2);
	smmu->usb_smmu_hsdt1_unreset = of_get_chip_usb_reg_cfg(np, "usb_smmu_hsdt1_unreset");
	smmu->usb_smmu_tbu_unreset = of_get_chip_usb_reg_cfg(np, "usb_smmu_tbu_unreset");
	smmu->usb_smmu_hsdt1_reset = of_get_chip_usb_reg_cfg(np, "usb_smmu_hsdt1_reset");
	smmu->usb_smmu_tbu_reset = of_get_chip_usb_reg_cfg(np, "usb_smmu_tbu_reset");

	if (!smmu->tbu_base || !smmu->tbu_sid_base 
		|| !smmu->usb_smmu_hsdt1_unreset || !smmu->usb_smmu_tbu_unreset
		|| !smmu->usb_smmu_hsdt1_reset || !smmu->usb_smmu_tbu_reset) {
		misc_ctrl_err("put_smmu dt resource\n");
		put_smmu_resoure(smmu);
		kfree(smmu);
		return -EINVAL;
	}

	if (!smmu->bypass_smmu)
		ecc_tbu_handler_register(smmu);

	misc_ctrl->smmu = smmu;
	misc_ctrl_info("end\n");
	return 0;
}

static uint64_t usb_smmu_sec_enable(struct usb_smmu *smmu, u32 sec_enable)
{
#ifdef CONFIG_HIUSB_SUPPORT_FFA
	int ret;
	struct ffa_send_direct_data args = {0};
	args.data0 = FID_USB_SMMU_SEC;
	args.data1 = sec_enable;
	args.data2 = smmu->sec_bit_offset;
	args.data3 = smmu->port_id;

	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0)
		pr_err("%s:smmu:ffa send fail:%d\n", __func__, ret);
	return 0;
#else
	struct arm_smccc_res res;

	/* x1:0 sec disable  x1:1 sec enable x2:smmu_tbu_num */
	arm_smccc_smc((u64)FID_USB_SMMU_SEC, sec_enable, smmu->sec_bit_offset,
		0, 0, 0, 0, 0, &res);
	return res.a0;
#endif
}

static void usb_config_smmu_sid(struct usb_smmu *smmu)
{
	/* set smmu unsec */
	usb_smmu_sec_enable(smmu, SEC_DISABLE);
	writel((smmu->sid) << 1, smmu->tbu_sid_base + smmu->mmusid_off[0]);
	writel((smmu->sid) << 1, smmu->tbu_sid_base + smmu->mmusid_off[1]);
}

static noinline uint64_t usb_tbu_bypass(struct usb_smmu *smmu, u32 bypass)
{
#ifdef CONFIG_HIUSB_SUPPORT_FFA
	int ret;
	struct ffa_send_direct_data args = {0};
	args.data0 = FID_USB_SMMU_CONFIG;
	args.data1 = bypass;
	args.data2 = smmu->tbu_num;
	args.data3 = 0;

	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0)
		pr_err("%s:smmu:ffa send fail:%d\n", __func__, ret);
	return 0;
#else
	struct arm_smccc_res res;

	/* x1:0 clear_bypass  x1:1 set_bypass x2:smmu_tbu_num */
	arm_smccc_smc((u64)FID_USB_SMMU_CONFIG, bypass, smmu->tbu_num,
		0, 0, 0, 0, 0, &res);
	return res.a0;
#endif
}

static int usb_tbu_config(struct usb_smmu *smmu, u32 enable)
{
	u32 reg_val;
	u32 tok_trans_gnt;
	u32 time = TBU_CONFIG_TIMEOUT;

	/* Clear bypass flag */
	if (enable)
		usb_tbu_bypass(smmu, DISABLE);

	/* enable req */
	reg_val = readl(smmu->tbu_base + TBU_CR);
	misc_ctrl_info("TBU_CR default value:0x%x\n", reg_val);
	if (enable)
		reg_val |= TBU_EN_REQ_BIT;
	else
		reg_val &= ~TBU_EN_REQ_BIT;

	writel(reg_val, smmu->tbu_base + TBU_CR);

	reg_val = readl(smmu->tbu_base + TBU_CRACK);
	while (!(reg_val & TBU_EN_ACK_BIT)) {
		udelay(1);
		if (time == 0) {
			misc_ctrl_err("TBU req not acknowledged tbu_crack:0x%x\n", reg_val);
			return -1;
		}
		time--;
		reg_val = readl(smmu->tbu_base + TBU_CRACK);
	}

	reg_val = readl(smmu->tbu_base + TBU_CRACK);
	if (enable) {
		if (!(reg_val & TBU_CONNCT_STATUS_BIT)) {
			misc_ctrl_err("TBU connecting failed tbu_crack:0x%x\n", reg_val);
			return -1;
		}

		tok_trans_gnt = (reg_val & TBU_TOK_TRANS_MSK) >> TBU_TOK_TRANS;
		if (tok_trans_gnt < TBU_DEFALUT_TOK_TRANS)
			misc_ctrl_err("tok_trans_gnt is less than setting tbu_crack:0x%x\n", reg_val);
	} else {
		if (reg_val & TBU_CONNCT_STATUS_BIT) {
			misc_ctrl_err("TBU is disconnect from TCU fail tbu_crack:0x%x\n", reg_val);
			return -1;
		}
	}

	return 0;
}

static void unreset_usb_smmu_hsdt1(struct usb_smmu *smmu)
{
	int ret = 0;

	if (smmu->usb_smmu_hsdt1_unreset) {
		misc_ctrl_info("usb_smmu_hsdt1_unreset\n");
		ret = chip_usb_reg_write(smmu->usb_smmu_hsdt1_unreset);
		if (ret)
			misc_ctrl_err("usb_smmu_hsdt1_unreset failed\n");
	}
}

static void unreset_usb_smmu_tbu(struct usb_smmu *smmu)
{
	int ret = 0;

	if (smmu->usb_smmu_tbu_unreset) {
		misc_ctrl_info("usb_smmu_tbu_unreset\n");
		ret = chip_usb_reg_write(smmu->usb_smmu_tbu_unreset);
		if (ret)
			misc_ctrl_err("usb_smmu_tbu_unreset failed\n");
	}
}

static void reset_usb_smmu_hsdt1(struct usb_smmu *smmu)
{
	int ret = 0;

	if (smmu->usb_smmu_hsdt1_reset) {
		misc_ctrl_info("usb_smmu_hsdt1_reset\n");
		ret = chip_usb_reg_write(smmu->usb_smmu_hsdt1_reset);
		if (ret)
			misc_ctrl_err("usb_smmu_hsdt1_reset failed\n");
	}
}

static void reset_usb_smmu_tbu(struct usb_smmu *smmu)
{
	int ret = 0;

	if (smmu->usb_smmu_tbu_reset) {
		misc_ctrl_info("usb_smmu_tbu_reset\n");
		ret = chip_usb_reg_write(smmu->usb_smmu_tbu_reset);
		if (ret)
			misc_ctrl_err("usb_smmu_tbu_reset failed\n");
	}
}

static void smmu_tbu_ecc_enable(void *tbu_base)
{
	u32 reg;

	reg = readl_relaxed(tbu_base + SMMU_TBU_MEM_CTRL_S_DR_OFFSET);
	reg |= SMMU_TBU_MEM_CTRL_S_ECC_ENABLE;
	writel_relaxed(reg, tbu_base + SMMU_TBU_MEM_CTRL_S_DR_OFFSET);

	reg = readl_relaxed(tbu_base + SMMU_TBU_MEM_CTRL_S_SR_OFFSET);
	reg |= SMMU_TBU_MEM_CTRL_S_ECC_ENABLE;
	writel_relaxed(reg, tbu_base + SMMU_TBU_MEM_CTRL_S_SR_OFFSET);

	reg = readl_relaxed(tbu_base + SMMU_TBU_MEM_CTRL_T_DR_OFFSET);
	reg |= SMMU_TBU_MEM_CTRL_T_ECC_ENABLE;
	writel_relaxed(reg, tbu_base + SMMU_TBU_MEM_CTRL_T_DR_OFFSET);

	reg = readl_relaxed(tbu_base + SMMU_TBU_MEM_CTRL_T_SR_OFFSET);
	reg |= SMMU_TBU_MEM_CTRL_T_ECC_ENABLE;
	writel_relaxed(reg, tbu_base + SMMU_TBU_MEM_CTRL_T_SR_OFFSET);

	reg = readl_relaxed(tbu_base + SMMU_TBU_MEM_CTRL_B_DR_OFFSET);
	reg |= SMMU_TBU_MEM_CTRL_B_ECC_ENABLE;
	writel_relaxed(reg, tbu_base + SMMU_TBU_MEM_CTRL_B_DR_OFFSET);

	reg = readl_relaxed(tbu_base + SMMU_TBU_MEM_CTRL_B_SR_OFFSET);
	reg |= SMMU_TBU_MEM_CTRL_B_ECC_ENABLE;
	writel_relaxed(reg, tbu_base + SMMU_TBU_MEM_CTRL_B_SR_OFFSET);

	/* enable interrupts */
	reg = readl_relaxed(tbu_base + SMMU_TBU_IRPT_MASK_NS_OFFSET);
	reg &= SMMU_TBU_IRPT_MASK_NS_ECC_MSK;
	writel_relaxed(reg, tbu_base + SMMU_TBU_IRPT_MASK_NS_OFFSET);
}

static void smmu_mem_tra_cfg(void *tbu_base)
{
	unsigned int tmp, orig;

	orig = readl(tbu_base + SMMU_TBU_MEM_CTRL_TRA_OFFSET);
	tmp = orig & ~SMMU_TBU_MEM_CTRL_TRA_MSK;
	tmp |= SMMU_TBU_MEM_CTRL_TRA_VALUE & SMMU_TBU_MEM_CTRL_TRA_MSK;

	writel(tmp, tbu_base + SMMU_TBU_MEM_CTRL_TRA_OFFSET);
}

static int usb_enable_smmu(struct chip_usb_misc_ctrl *misc_ctrl)
{
	struct usb_smmu *smmu = misc_ctrl->smmu;
	struct device *dev = misc_ctrl->dev;
	int ret;

	mutex_lock(&smmu_mutex);
	/* Step 1. smmu_nonsec_tcuctl  tcu_power_on */
	if (atomic_add_return(1, &tcu_ctrl_cnt) == 1) {
		unreset_usb_smmu_hsdt1(smmu);
		ret = mm_smmu_nonsec_tcuctl(smmu->smmu_id);
		if (ret) {
			misc_ctrl_err("mm_smmu_nonsec_tcuctl fail %d\n", ret);
			mutex_unlock(&smmu_mutex);
			return -1;
		}

		misc_ctrl_info("mm_smmu_poweron\n");
		ret = mm_smmu_poweron(dev);
		if (ret) {
			misc_ctrl_err("mm_smmu_poweron fail %d\n", ret);
			mutex_unlock(&smmu_mutex);
			return -1;
		}
	}

	/* Step 2. smmu tbu config */
	usb_config_smmu_sid(smmu);
	if (atomic_add_return(1, get_smmu_tbu_ctrl_cnt(smmu)) == 1) {
		unreset_usb_smmu_tbu(smmu);
		ret = usb_tbu_config(smmu, ENABLE);
		if (ret && (atomic_sub_return(1, &tcu_ctrl_cnt) == 0)) {
			misc_ctrl_err("mm_smmu_poweroff");
			(void)mm_smmu_poweroff(dev);
			misc_ctrl_err("tbu config fail");
			mutex_unlock(&smmu_mutex);
			return -1;
		}
		smmu_tbu_ecc_enable(smmu->tbu_base);
		smmu_mem_tra_cfg(smmu->tbu_base);
		misc_ctrl_info("tbu init done");
	}
	mutex_unlock(&smmu_mutex);

	misc_ctrl_info("tcu_ctrl_cnt:%d tbu%d_ctrl_cnt:%d\n", atomic_read(&tcu_ctrl_cnt), smmu->tbu_num,
		 atomic_read(get_smmu_tbu_ctrl_cnt(smmu)));
	return 0;
}

int usb_disable_smmu(struct chip_usb_misc_ctrl *misc_ctrl)
{
	struct device *dev = NULL;
	struct usb_smmu *smmu = NULL;
	int ret;
	unsigned long flags;

	if (!misc_ctrl->smmu || !misc_ctrl->smmu->support_smmu)
		return 0;

	dev = misc_ctrl->dev;

	smmu = misc_ctrl->smmu;
	if (smmu->bypass_smmu) {
		mutex_lock(&smmu_mutex);
		if (atomic_sub_return(1, &tcu_ctrl_cnt) == 0)
			reset_usb_smmu_hsdt1(smmu);

		if (atomic_sub_return(1, get_smmu_tbu_ctrl_cnt(smmu)) == 0)
			reset_usb_smmu_tbu(smmu);
		mutex_unlock(&smmu_mutex);
		return 0;
	}

	mutex_lock(&smmu_mutex);
	spin_lock_irqsave(&g_tbu_lock, flags);
	if (atomic_sub_return(1, get_smmu_tbu_ctrl_cnt(smmu)) == 0) {
		ret = usb_tbu_config(smmu, DISABLE);
		if (ret) {
			misc_ctrl_err("tbu config(disconnect) failed\n");
			spin_unlock_irqrestore(&g_tbu_lock, flags);
			mutex_unlock(&smmu_mutex);
			return -1;
		}
		reset_usb_smmu_tbu(smmu);
	}
	spin_unlock_irqrestore(&g_tbu_lock, flags);

	if (atomic_sub_return(1, &tcu_ctrl_cnt) == 0) {
		ret = mm_smmu_poweroff(dev);
		if (ret) {
			misc_ctrl_err("tcu power off fail\n");
			mutex_unlock(&smmu_mutex);
			return -1;
		}
		reset_usb_smmu_hsdt1(smmu);
	}

	mutex_unlock(&smmu_mutex);
	misc_ctrl_info("tcu_ctrl_cnt:%d tbu%d_ctrl_cnt:%d\n", atomic_read(&tcu_ctrl_cnt), smmu->tbu_num,
		 atomic_read(get_smmu_tbu_ctrl_cnt(smmu)));
	misc_ctrl_info("tbu disable done\n");
	return 0;
}

int usb_smmu_init(struct chip_usb_misc_ctrl *misc_ctrl)
{
	struct usb_smmu *smmu = NULL;

	if (!misc_ctrl->smmu || !misc_ctrl->smmu->support_smmu)
		return 0;

	smmu = misc_ctrl->smmu;
	if (smmu->bypass_smmu) {
		mutex_lock(&smmu_mutex);
		if (atomic_add_return(1, &tcu_ctrl_cnt) == 1)
			unreset_usb_smmu_hsdt1(smmu);

		if (atomic_add_return(1, get_smmu_tbu_ctrl_cnt(smmu)) == 1) {
			misc_ctrl_info("usb bypass smmu cfg\n");
			unreset_usb_smmu_tbu(smmu);
			usb_tbu_bypass(smmu, ENABLE);
		}
		mutex_unlock(&smmu_mutex);
		misc_ctrl_info("tbu bypass\n");
		return 0;
	}

	return usb_enable_smmu(misc_ctrl);
}
