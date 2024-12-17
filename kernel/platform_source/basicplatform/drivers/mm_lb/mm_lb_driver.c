/*
 *
 * lb drv
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <asm/tlbflush.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/memblock.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/smp.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/spinlock.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/page-flags.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/uaccess.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <dsm/dsm_pub.h>

#include "mm_lb_priv.h"
#include "mm_lb_driver.h"
#include "mm_lb_cache.h"

#define DFX_CMD_TYPE_MASK BIT(12)
#define DFX_CMD_MID_SHIFT 4
#define DFX_CMD_MID_MASK  (0xff << DFX_CMD_MID_SHIFT)

#define LB_LARGIEST_COUNT         0x7000

#ifdef CONFIG_MM_LB_V300
#define INT_MASK                  0xfdffffff
#define ROUTE_INT_MASK            0xfffff
#else
#define INT_MASK                  0xffffffff
#define ROUTE_INT_MASK            0xffff
#endif
#define CMO_CFG_INT_MASK          0x1f000

#define DFX_ECCERR_SINGLE_MASK     BIT(0)
#define DFX_ECCERR_MULTI_MASK      BIT(1)
#define DSM_LB_ECC_SINGLE_BIT      925205200
#define DSM_LB_ECC_MUTIL_BIT       925205201

#define SINGLE_BIT_EVENT_MAX   10
#define MUTIL_BIT_EVENT_MAX    10

#define CMO_OPERATE_MASK  0xf
#define CMO_BITMAP_MASK   0xffff
#define CMO_64B_PA_SHIFT  6
#define CMO_128B_PA_SHIFT 7
#define CMO_4K_PA_SHIFT   12
#define DFX_CMD_PA_SHIFT  2

#define FLOW_START_MASK   0x4000319
#define FLOW_END_MASK     0x318
#define FLOW_GID_SHIFT    10
#define FLOW_GID_MASK     (0xffff << FLOW_GID_SHIFT)
#define FLOW_ENABLE_MASK  BIT(26)
#ifdef CONFIG_MM_LB_V300
#define     GLB_FLOW_STAT           0x8108
#define     GLB_FLOW_STAT_IN        0x8204
#define     GLB_FLOW_STAT_OUT       0x8208
#endif

#define SC_POLL_TIMEOUT_US	1000000

typedef union {
	u64 value;
	union {
		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 way_bitmap : 16;
			u64 res : 40;
		} by_way;

		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 gid_bitmap : 16;
			u64 res : 40;
		} by_gid;

		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 res0 : 2;
			u64 pa : 31;
			u64 res1 : 18;
			u64 cache_mode : 1;
			u64 gid : 4;
		} by_64pa;

		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 res0 : 3;
			u64 pa : 30;
			u64 res1 : 18;
			u64 cache_mode : 1;
			u64 gid : 4;
		} by_128pa;
#ifdef CONFIG_MM_LB_V500
		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 spa : 25;
			u64 res : 1;
			u64 epa : 25;
			u64 cache_mode : 1;
			u64 gid : 4;
		} by_4xkpa;
#else
		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 spa : 24;
			u64 epa : 24;
			u64 res : 3;
			u64 cache_mode : 1;
			u64 gid : 4;
		} by_4xkpa;
#endif

		struct {
			u64 opt : 4;
			u64 cmdt : 4;
			u64 rslt : 4;
			u64 id : 16;
			u64 seq : 10;
		} sync;
	} param;
} lb_cmd;

struct lb_device *lbdev;
static int single_bit;
static int mutil_bit;
u32 cmo_dummy_pa;

void lb_cmo_sync_cmd_by_event(void)
{
	u32 status;
	int cpu_id;
	ktime_t timeout;
	unsigned long flags;
	lb_cmd snyc;

	preempt_disable();
	local_irq_save(flags);

	cpu_id = 0;
	snyc.value = 0;
	snyc.param.sync.opt = 1;
	snyc.param.sync.cmdt = 1;
	snyc.param.sync.rslt = cpu_id;
	snyc.param.sync.seq = 1;
	writeq(snyc.value, (lbdev->base + CMO_CMD));

	/* ensure cmo cmd complete */
	mb();

	timeout = ktime_add_us(ktime_get(), SC_POLL_TIMEOUT_US);

	while (!(readl(lbdev->base + CMO_ADDR(CMO_STAT, cpu_id))
		& FINISH_MASK)) {
		if (ktime_compare(ktime_get(), timeout) > 0) {
			lb_print(LB_ERROR, "time out\n");
			break;
		}
		wfe();
	}

	writel(CMO_EVENT_INT_CLR_FLAG,
		(lbdev->base + CMO_ADDR(CMO_CLEAR, cpu_id)));

	status = readl(lbdev->base + CMO_ADDR(CMO_STAT, cpu_id));

	WARN_ON(status & FINISH_MASK);

	local_irq_restore(flags);
	preempt_enable();
}

static int build_cmo_cmd(sync_type synct, ops_type ops,
		cmo_type by_x, u32 bitmap, u64 pa, size_t sz, u64 *value)
{
	int ret = 0;
	lb_cmd cmo;

	cmo.value = 0;

	switch (by_x) {
	case CMO_BY_WAY:
		cmo.param.by_way.opt = ops & CMO_OPERATE_MASK;
		cmo.param.by_way.cmdt = by_x;
		cmo.param.by_way.way_bitmap = bitmap & CMO_BITMAP_MASK;
		break;

	case CMO_BY_GID:
		cmo.param.by_gid.opt = ops & CMO_OPERATE_MASK;
		cmo.param.by_gid.cmdt = by_x;
		cmo.param.by_gid.gid_bitmap = bitmap & CMO_BITMAP_MASK;
		break;

	case CMO_BY_64PA:
		cmo.param.by_64pa.opt = ops & CMO_OPERATE_MASK;
		cmo.param.by_64pa.cmdt = by_x;
		cmo.param.by_64pa.pa = pa >> CMO_64B_PA_SHIFT;
		cmo.param.by_64pa.gid = bitmap;
		if (!IS_ALIGNED(pa, SZ_64)) {
			lb_print(LB_ERROR, "pa is not align 64\n");
			ret = -EINVAL;
		}
		break;

	case CMO_BY_128PA:
		cmo.param.by_128pa.opt = ops & CMO_OPERATE_MASK;
		cmo.param.by_128pa.cmdt = by_x;
		cmo.param.by_128pa.pa = pa >> CMO_128B_PA_SHIFT;
		cmo.param.by_128pa.gid = bitmap;
		if (!IS_ALIGNED(pa, SZ_128)) {
			lb_print(LB_ERROR, "pa is not align 64\n");
			ret = -EINVAL;
		}
		break;
	case CMO_BY_4XKPA:
		cmo.param.by_4xkpa.opt = ops & CMO_OPERATE_MASK;
		cmo.param.by_4xkpa.cmdt = by_x;
		cmo.param.by_4xkpa.spa = (pa >> CMO_4K_PA_SHIFT);
		cmo.param.by_4xkpa.epa = ((pa + sz) >> CMO_4K_PA_SHIFT) - 1;
		cmo.param.by_4xkpa.gid = bitmap;
		if (!IS_ALIGNED(pa, PAGE_SIZE)
			|| !IS_ALIGNED(pa + sz, PAGE_SIZE)
			|| pa >= pa + sz) {
			WARN_ON(1);

			ret = -EINVAL;
		}
		break;

	default:
		lb_print(LB_ERROR, "invalid type %d\n", by_x);
		ret = -EINVAL;
		break;
	}

	*value = cmo.value;

	return ret;
}

int lb_ops_cache(sync_type synct, ops_type ops, cmo_type by_x,
	u32 bitmap, u64 pa, size_t sz)
{
	int ret;
	lb_cmd cmo;

	if ((ops != INV && ops != CLEAN && ops != CI) || by_x > CMO_BY_4XKPA) {
		lb_print(LB_ERROR, "%d %d invalid para\n", ops, by_x);
		return -EINVAL;
	}

	cmo.value = 0;

	ret = build_cmo_cmd(synct, ops, by_x, bitmap, pa, sz, &cmo.value);
	if (ret)
		return ret;

	writeq(cmo.value, (lbdev->base + CMO_CMD));

	return ret;
}

void lb_invalid_cache(u32 gid, u64 pa, size_t sz)
{
	unsigned long flags = 0;
#if defined(CONFIG_MM_LB_V300) && defined(CONFIG_HISI_LB_SCB_ENABLE)
	size_t num, offset;
	unsigned int scb = 0;
	u64 addr, end_addr;
#endif

	if (!lbdev) {
		lb_print(LB_ERROR, "lbdev is null\n");
		return;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);

	if (!lbdev->power_state)
		goto unlock;

#if defined(CONFIG_MM_LB_V300) && defined(CONFIG_HISI_LB_SCB_ENABLE)
	/* pa of the dmi port is scrambled, but pa of the cmo is not scrambled.
	   As a result, pa of the dmi port is inconsistent with pa of the cmo.
	   the cache flushing of cmo based on the pa region is abnormal.
	   Therefore, software is required to scramble pa of the cmo. */
	end_addr = pa + sz;
	while (pa < end_addr) {
		addr = pa;
		num = sz / SZ_32K;
		if ((addr % SZ_32K == 0) && (num > 0)) {
			offset = num * SZ_32K;
		} else {
			offset = SZ_4K;
			scb = ((addr >> SCB_BIT14) ^ (addr >> SCB_BIT23)) & BIT(0);
			addr = (addr & (~BIT(SCB_BIT14))) | (scb << SCB_BIT14);
			scb = ((addr >> SCB_BIT13) ^ (addr >> SCB_BIT22)) & BIT(0);
			addr = (addr & (~BIT(SCB_BIT13))) | (scb << SCB_BIT13);
			scb = ((addr >> SCB_BIT12) ^ (addr >> SCB_BIT21)) & BIT(0);
			addr = (addr & (~BIT(SCB_BIT12))) | (scb << SCB_BIT12);
		}
		if (lb_ops_cache(EVENT, INV, CMO_BY_4XKPA, gid, addr, offset))
			goto unlock;
		pa += offset;
		sz -= offset;
	}
#else
	if (lb_ops_cache(EVENT, INV, CMO_BY_4XKPA, gid, pa, sz))
		goto unlock;
#endif

	if (cmo_dummy_pa && lb_ops_cache(EVENT, CLEAN, CMO_BY_4XKPA, gid,
		cmo_dummy_pa, PAGE_SIZE))
		goto unlock;

	/* ensure cmo ops complete */
	mb();

	lb_cmo_sync_cmd_by_event();
unlock:
	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);
}

void set_gid_config(unsigned int gid, unsigned int plc,
	unsigned int way, unsigned int quota)
{
	writel(plc, (lbdev->base + GID_ADDR(GID_ALLC_PLC, gid)));
	writel(way, (lbdev->base + GID_ADDR(GID_WAY_ALLC, gid)));
	writel(quota, (lbdev->base + GID_ADDR(GID_QUOTA, gid)));
#ifdef CONFIG_MM_LB_V500
	/*
	 * Allc_plc's gid_en bit do not exist after v500.
	 * We have to use bypass register to control gid enable status instead.
	 * Note that the cfgcmo has been excuted during lb_pages_detach->reset_page,
	 * the bypass status is updated without doing cfgcmo here.
	 * It's the master's duty to ensure that the memory is not access by UAF.
	 */
	writel(plc & PLC_ENABLE_BIT ? 0 : 1, (lbdev->base + GID_ADDR(SC_GID_BYPASS, gid)));
#endif
}

unsigned int clear_gid_quota(unsigned int gid)
{
	unsigned int r_quota;

	r_quota = readl((lbdev->base + GID_ADDR(GID_QUOTA, gid)));
	writel(0, (lbdev->base + GID_ADDR(GID_QUOTA, gid)));

	return r_quota;
}

void set_gid_quota(unsigned int gid, unsigned int quota)
{
	writel(quota, (lbdev->base + GID_ADDR(GID_QUOTA, gid)));
}

void set_quota(struct lb_plc_info *policy, unsigned int quota_value)
{
	gid quota_set;
	gid allc_set;
	gid allc_efuse;

	DEFINE_INIT_ALLOC(allc_set, lbdev->way_en, lbdev->way_en);
	DEFINE_INIT_ALLOC(allc_efuse, lbdev->way_efuse, lbdev->way_efuse);
	DEFINE_INIT_QUOTA(quota_set, quota_value & QUOTA_L_MASK,
		quota_value & QUOTA_L_MASK);
	if (likely(policy->stat & GID_NON_SEC)) {
		writel(allc_set.value & allc_efuse.value, (lbdev->base +
			GID_ADDR(GID_WAY_ALLC, policy->gid)));
		writel(quota_set.value, (lbdev->base +
			GID_ADDR(GID_QUOTA, policy->gid)));
	} else {
		sec_gid_set(SEC_GID_CONFIG, policy->gid, quota_set.value);
	}
}
#ifndef CONFIG_MM_LB_V500
void clr_bypass_status(struct lb_plc_info *policy)
{
	u32 gid_byp;

	gid_byp = readl(lbdev->base + GLB_ROUTE_PLC_S);
	writel(gid_byp & (~BIT(policy->gid)), (lbdev->base + GLB_ROUTE_PLC_S));
	writel(0, (lbdev->base + GID_ADDR(GID_ALLC_PLC, policy->gid)));
}
#endif

#ifdef CONFIG_MM_LB_V300
unsigned int get_flow(unsigned int id, unsigned int dir)
{
	if (dir)
		return readl(lbdev->base + SLC_ADDR2000(GLB_FLOW_STAT_IN, id));
	else
		return readl(lbdev->base + SLC_ADDR2000(GLB_FLOW_STAT_OUT, id));
}

void start_flow(unsigned int gid, unsigned int id)
{
	unsigned int reg;

	reg = readl(lbdev->base + SLC_ADDR2000(GLB_FLOW_STAT, id));
	if (reg & FLOW_ENABLE_MASK)
		return;

	reg = FLOW_START_MASK | BIT(FLOW_GID_SHIFT + gid);
	writel(reg, (lbdev->base + SLC_ADDR2000(GLB_FLOW_STAT, id)));
}

void end_flow(unsigned int id)
{
	writel(FLOW_END_MASK, (lbdev->base + SLC_ADDR2000(GLB_FLOW_STAT, id)));
}
#endif

static void parse_dfx_err_info(uint32_t slice_idx, uint32_t err_inf1,
	uint32_t err_inf2)
{
	unsigned int master_id;
	unsigned int id;

	lb_print(LB_ERROR, "SLC_DFX_ERR_1 slice%u:0x%x\n", slice_idx, err_inf1);
	lb_print(LB_ERROR, "SLC_DFX_ERR_2 slice%u:0x%x\n", slice_idx, err_inf2);
	lb_print(LB_ERROR, "dfx_fail_cmd_pa:0x%x\n", err_inf1 << DFX_CMD_PA_SHIFT);

	if (err_inf2 & DFX_CMD_TYPE_MASK)
		lb_print(LB_ERROR, "dfx_fail_cmd_type is write\n");
	else
		lb_print(LB_ERROR, "dfx_fail_cmd_type is read\n");

	master_id = (err_inf2 & DFX_CMD_MID_MASK) >> DFX_CMD_MID_SHIFT;
	lb_print(LB_ERROR, "Master id is 0x%x\n", master_id);

	id = err_inf2 & 0xf;
	lb_print(LB_ERROR, "dfx_fail_cmd_gid:%u\n", id);
}

void lb_ecc_info_notify(u32 bit_err)
{
	struct dsm_client *ai_client = lbdev->dsm.ai_client;

	if (!ai_client) {
		lb_print(LB_ERROR, "ai_client is null\n");
		return;
	}

	if ((bit_err == MULTI_BIT_ERR) &&
		(mutil_bit < MUTIL_BIT_EVENT_MAX)) {
#ifdef CONFIG_MM_LB_ENG_DEBUG
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_LB, 0ULL, 0ULL);
		mutil_bit++;
#else
		if (!dsm_client_ocuppy(ai_client)) {
			dsm_client_record(ai_client, "lb ecc mutil bit\n");
			dsm_client_notify(ai_client, DSM_LB_ECC_MUTIL_BIT);
			mutil_bit++;
		}
#endif
	} else if ((bit_err == SINGLE_BIT_ERR) &&
		(single_bit < SINGLE_BIT_EVENT_MAX)) {
		if (!dsm_client_ocuppy(ai_client)) {
			dsm_client_record(ai_client, "lb ecc 1bit\n");
			dsm_client_notify(ai_client, DSM_LB_ECC_SINGLE_BIT);
			single_bit++;
		}
	}
}

#ifndef CONFIG_MM_LB_V500
void gid_suspend_enable(unsigned int gid)
{
	writel(BIT(GID_EN_SHIFT), (lbdev->base + GID_ADDR(GID_ALLC_PLC, gid)));
}

void lb_dfx_irq_disable(struct lb_device *lbd)
{
	int i;
	/* disable int */
#ifndef CONFIG_MM_LB_V100
	writel(0, (lbd->base + SC_RT_INT_EN));
#endif
	writel(0, (lbd->base + CMO_CFG_INT_EN));
	for (i = 0; i < lbd->slc_idx; i++)
		writel(0, (lbd->base + SLC_ADDR2000(SLC02_INT_EN, i)));
}

void lb_dfx_irq_enable(struct lb_device *lbd)
{
	int i;

	/* clear the int */
	for (i = 0; i < lbd->slc_idx; i++)
		writel(INT_MASK,
			(lbd->base + SLC_ADDR2000(SLC02_INT_CLEAR, i)));

#ifndef CONFIG_MM_LB_V100
	writel(ROUTE_INT_MASK, (lbd->base + SC_RT_INT_CLR));
#endif
	writel(CMO_CFG_INT_MASK, (lbd->base + CMO_CFG_INT_CLR));

	/* enable int */
	for (i = 0; i < lbd->slc_idx; i++)
		writel(INT_MASK, (lbd->base + SLC_ADDR2000(SLC02_INT_EN, i)));

#ifndef CONFIG_MM_LB_V100
	writel(ROUTE_INT_MASK, (lbd->base + SC_RT_INT_EN));
#endif
	writel(CMO_CFG_INT_MASK, (lbd->base + CMO_CFG_INT_EN));
}

void lb_dfx_print(struct lb_device *lbd)
{
	int i;
	/* read the int status */
	lb_print(LB_ERROR, "glb interrupt 0x%x\n",
		readl(lbd->base + CFG_INT_STATUS));

	lb_print(LB_ERROR, "CMO interrupt 0x%x\n",
		readl(lbd->base + CMO_CFG_INT_INI));
	for (i = 0; i < lbd->slc_idx; i++)
		lb_print(LB_ERROR, "slc interrupt 0x%x",
			readl(lbd->base + SLC_ADDR2000(SLC02_INT_INI, i)));
}

int lb_ecc_intr_parase(u32 ecc_inf)
{
	if (ecc_inf & DFX_ECCERR_MULTI_MASK) {
		return MULTI_BIT_ERR;
	} else if (ecc_inf & DFX_ECCERR_SINGLE_MASK) {
		return SINGLE_BIT_ERR;
	} else {
		return ERR_NON;
	}
}

unsigned int ecc_info_handle(struct lb_device *lbd, int slc)
{
	unsigned int ecc_inf;
	int bit_err;

	ecc_inf = readl(lbd->base + SLC_ADDR2000(SLC_DFX_ECCERR, slc));
	lb_print(LB_ERROR, "ecc info: 0x%x\n", ecc_inf);

	/* only report ecc bit err */
	bit_err = lb_ecc_intr_parase(ecc_inf);
	if (bit_err)
		lb_ecc_info_notify(bit_err);
	return bit_err;
}

void get_err_info(int slc, unsigned int *err1, unsigned int *err2)
{
	*err1 = readl(lbdev->base + SLC_ADDR2000(SLC_DFX_ERR_INF1, slc));
	*err2 = readl(lbdev->base + SLC_ADDR2000(SLC_DFX_ERR_INF2, slc));
}

void err_info_handle(int slc)
{
	unsigned int err_inf1, err_inf2;

	get_err_info(slc, &err_inf1, &err_inf2);
	if (err_inf1 || err_inf2)
		parse_dfx_err_info(slc, err_inf1, err_inf2);
}
#endif

static irqreturn_t lb_dfx_handle(int irq, void *lbd)
{
	int i;
	unsigned int ecc_err = 0;
	bool sys_err_flag = false;
	struct lb_device *d = lbd;

	lb_print(LB_ERROR, "--> %s\n", __func__);

	lb_dfx_irq_disable(d);

	lb_dfx_print(d);
	/* read ecc info */
	if (!lbdev->dsm.ai_client)
		schedule_work(&(d->dsm.dsm_work));

	for (i = 0; i < d->slc_idx; i++) {
		ecc_err = ecc_info_handle(d, i);
		if(ecc_err != ERR_NON && ecc_err != SINGLE_BIT_ERR)
			sys_err_flag = true;
	}

	for (i = 0; i < d->slc_idx; i++)
		err_info_handle(i);

	lb_dfx_irq_enable(d);

#ifdef CONFIG_MM_LB_ENG_DEBUG
	if (sys_err_flag)
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_LB, 0ULL, 0ULL);
#else
	WARN_ON(1);
#endif

	return IRQ_HANDLED;
}

static irqreturn_t lb_cmd_handle(int irq, void *lbd)
{
	struct lb_device *d = lbd;

	lb_print(LB_INFO, "into %s\n", __func__);
	if (!d) {
		lb_print(LB_ERROR, "lbd is null\n");
		return IRQ_NONE;
	}

	lb_print(LB_INFO, "out %s\n", __func__);
	return IRQ_NONE;
}

int reset_ip(struct lb_device *lbd)
{
	int ret;

	lb_dfx_irq_disable(lbd);
	ret = devm_request_threaded_irq(lbd->dev, lbd->dfx_irq, lb_dfx_handle,
		NULL, IRQF_TRIGGER_RISING | IRQF_SHARED, "dfx-intr", lbd);
	if (ret) {
		lb_print(LB_ERROR, "failed to enable dfx irq\n");
		return -EINVAL;
	}
	ret = devm_request_threaded_irq(lbd->dev, lbd->cmd_irq, lb_cmd_handle,
		NULL, IRQF_TRIGGER_RISING | IRQF_SHARED, "cmd-intr", lbd);
	if (ret) {
		lb_print(LB_ERROR, "failed to enable cmd irq\n");
		return -EINVAL;
	}

	lb_dfx_irq_enable(lbd);
	return 0;
}