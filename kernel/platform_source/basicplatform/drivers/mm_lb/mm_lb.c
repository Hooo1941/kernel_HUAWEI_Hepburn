/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <asm/memory.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#ifdef CONFIG_MM_AMA
#include <platform_include/basicplatform/linux/mem_ama.h>
#endif

#include "mm_lb_driver.h"
#include "mm_lb_policy.h"
#include "mm_lb_priv.h"
#include <linux/mm_lb/mm_lb.h>

#define LB_PRIV_GID_WIDE          4
#define LB_PRIV_GID_MASK          (BIT(LB_PRIV_GID_WIDE) - 1)

#define FLOW_MAX_ID       4
#define FLOWID_TO_SLC(x)  (x - 1U)

unsigned int is_lb_available(void)
{
	return __is_lb_available();
}

unsigned int lb_get_available_size(void)
{
	if (!lbdev)
		return 0;

	if (!__is_lb_available())
		return 0;

	return lbdev->gdplc.max_size;
}

/*
 * this API is called by HIFI, Tiny, GPU
 * size is not used
 */
int lb_request_quota(unsigned int pid)
{
	int ret = 0;
	unsigned long flags = 0;
	struct lb_plc_info *policy = NULL;

	lb_print(LB_INFO, "into %s\n", __func__);

	if (!__is_lb_available())
		return 0;

	if (!policy_id_valid(pid, GID_NON_SEC | GID_SEC | GID_BYPASS_STATE)) {
		lb_print(LB_ERROR, "0x%x: pid invalid\n", pid);
		ret = -ENOMEM;
		goto err;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);
	policy = &lbdev->gdplc.ptbl[pid];
	if (likely(policy->stat & GID_SEC))
		ret = sec_gid_enable(policy);
	else
		ret = gid_enable(policy);

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

err:
	lb_print(LB_INFO, "out %s\n", __func__);
	return ret;
}

int lb_release_quota(unsigned int pid)
{
	int ret = 0;
	unsigned long flags = 0;
	struct lb_plc_info *policy = NULL;

	lb_print(LB_INFO, "into%s\n", __func__);

	if (!__is_lb_available())
		return 0;

	if (!policy_id_valid(pid, GID_NON_SEC | GID_SEC | GID_BYPASS_STATE)) {
		lb_print(LB_ERROR, "%u: pid invalid\n", pid);
		ret = -EINVAL;
		goto err;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);
	policy = &lbdev->gdplc.ptbl[pid];
	if (likely(policy->stat & GID_SEC))
		ret = sec_gid_disable(policy);
	else
		ret = gid_bypass(policy);

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

err:
	return ret;
}

int lb_up_policy_prio(unsigned int pid)
{
	if (!is_support_bypass())
		return 0;

	/*
	 * ISPNN might run concurrently with isppipe.
	 * To ensure the isppipe get the cacheline (in buffer mode) prior than ISPNN,
	 * we should change the ISPNN's prio to a level which is slightly lower than the highest prio.
	 */
	if (pid == PID_ISPNN)
		return lb_set_master_policy(pid, SZ_8M, LB_BUF_HIGH_PRIO + LB_PRIO_HALF_LVL);

	if (pid == PID_NPU) {
		unsigned long flags = 0;
		unsigned int quota_value = get_max_quota();
		struct lb_plc_info *policy = &lbdev->gdplc.ptbl[pid];

		spin_lock_irqsave(&policy->lock, flags);
		if (policy->enabled == STATUS_ENABLE)
			set_quota(policy, quota_value);
		spin_unlock_irqrestore(&policy->lock, flags);
	}

	return 0;
}

int lb_down_policy_prio(unsigned int pid)
{
	if (!is_support_bypass())
		return 0;

	if (pid == PID_ISPNN)
		return lb_set_master_policy(pid, SZ_8M, LB_CACHE_LOW_PRIO);

	if (pid == PID_NPU) {
		unsigned long flags = 0;
		struct lb_plc_info *policy = &lbdev->gdplc.ptbl[pid];

		spin_lock_irqsave(&policy->lock, flags);
		if (policy->enabled == STATUS_ENABLE)
			set_quota(policy, 0);
		spin_unlock_irqrestore(&policy->lock, flags);
	}

	return 0;
}

unsigned int lb_get_master_policy(unsigned int pid, enum policy_type type)
{
	unsigned int ret = 0;
	unsigned long flags = 0;
	struct lb_plc_info *policy = NULL;

	if (!is_support_bypass())
		return 0;

	if (!policy_id_valid(pid, GID_NON_SEC | GID_BYPASS_STATE)) {
		lb_print(LB_ERROR, "invalid pid:%u\n", pid);
		return 0;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);

	policy = &lbdev->gdplc.ptbl[pid];
	switch (type) {
	case MASTER_GID:
		ret = policy->gid;
		break;
	case MASTER_QUOTA:
		ret = policy->quota;
		break;
	case MASTER_PLC:
		ret = policy->plc;
		break;
	default:
		ret = 0;
		break;
	}

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

	lb_print(LB_ERROR, "%s: pid:0x%x, type:0x%x, ret:0x%x\n",
		__func__, pid, type, ret);

	return ret;
}

/*
 * assert at page to virt
 * assert at kmap
 * it is ok
 */
void __lb_assert_page(struct page *pg)
{
	if (!lbdev || !pg)
		return;

	if (PageLB(pg))
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_LB, 0ULL, 0ULL);
}

/* assert at page to virt */
void __lb_assert_phys(phys_addr_t phys)
{
	struct page *p = NULL;

	if (!lbdev)
		return;

	/* on owner platform valid check is not ok */
	if (!lb_pfn_valid_within(phys))
		return;

	p = phys_to_page(phys);
	if (p && PageLB(p)) {
		lb_print(LB_ERROR, "page flag 0x%lx\n", p->flags);
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_LB, 0ULL, 0ULL);
	}
}

/*
 * assert at set pte
 * assert for page's gid is diff pte gid
 */
void __lb_assert_pte(pte_t pte)
{
}

void *lb_page_to_virt(struct page *page)
{
	unsigned long virt;

	if (!__is_lb_available() || !page)
		return NULL;

	WARN(!PageLB(page), "fail pagelb");
	WARN(!PagePrivate(page), "fail pageprivate");
	virt = page->private & (~LB_PRIV_GID_MASK);

	return (void *)virt;
}
EXPORT_SYMBOL(lb_page_to_virt);

u64 lb_pid_to_gidphys(u32 pid)
{
	if (!__is_lb_available())
		return 0;

	if (!policy_id_valid(pid, GID_NON_SEC)) {
		lb_print(LB_ERROR, "0x%x: pid invalid\n", pid);
		return 0;
	}

	return PTE_LB(lbdev->gdplc.ptbl[pid].gid);
}

unsigned int lb_page_to_gid(struct page *page)
{
	unsigned int gid_idx;

	if (!__is_lb_available() || !page || !PageLB(page))
		return 0;

	WARN(!PagePrivate(page), "fail pageprivate");
	gid_idx = page->private & LB_PRIV_GID_MASK;

	return gid_idx;
}

/* when call it set pte for cpu/iommu/gpu */
unsigned long lb_pte_attr(phys_addr_t phy_addr)
{
	unsigned long gid_idx;
	struct page *pg = NULL;

	if (!__is_lb_available())
		return 0;

	/*
	 * pfn illegal judgment affects efficiency
	 * the current only gpu use lb_pte_attr
	 * gpu input a valid addree
	 */
	pg = phys_to_page(phy_addr);
	if (!PageLB(pg))
		return 0;

	gid_idx = lb_page_to_gid(pg);

	return pgprot_val(PAGE_LB_CACHEABLE(gid_idx));
}

/*
 * in order to avoid GPU mapping framebuf with dss'gid,
 * only is called by GPU r18,
 * only is a circumvention plan.
 */
unsigned long lb_pte_attr_gpu(phys_addr_t phy_addr)
{
	unsigned long gid_idx;
	struct page *pg = NULL;

	if (!__is_lb_available())
		return 0;

	/*
	 * pfn illegal judgment affects efficiency
	 * the current only gpu use lb_pte_attr
	 * gpu input a valid addree
	 */
	pg = phys_to_page(phy_addr);
	if (!PageLB(pg))
		return 0;

	gid_idx = lb_page_to_gid(pg);
	if (gid_idx == lbdev->gdplc.ptbl[PID_IDISPLAY].gid) {
		WARN_ON_ONCE(1);
		return 0;
	}

	return pgprot_val(PAGE_LB_CACHEABLE(gid_idx));
}

/* when call it @ dma XXX function */
int lb_prot_build(struct page *pages, pgprot_t *pageprot)
{
	unsigned long gid_idx;

	if (!__is_lb_available() || !pages || !pageprot || !PageLB(pages))
		return 0;

	gid_idx = lb_page_to_gid(pages);
	*pageprot = pgprot_lb(*pageprot, gid_idx);

	return 0;
}

/*
 * when call it @ ion
 * @ map to cpu user/kernel space
 */
int lb_pid_prot_build(unsigned int pid, pgprot_t *pageprot)
{
	struct lb_plc_info *policy = NULL;

	if (!__is_lb_available())
		return 0;

	if (!pageprot)
		return -EINVAL;

	if (!policy_id_valid(pid, GID_NON_SEC)) {
		lb_print(LB_ERROR, "%s:policy_id invalid\n", __func__);
		return -EINVAL;
	}

	policy = &lbdev->gdplc.ptbl[pid];
	*pageprot = pgprot_lb(*pageprot, policy->gid);

	return 0;
}

int lb_pages_attach(unsigned int pid, struct page *pg, size_t count)
{
	int ret;
	unsigned long flags = 0;
	gid quota_set;
	gid allc_set;
	unsigned int quota = 0;
	struct lb_plc_info *policy = NULL;

	lb_print(LB_INFO, "into %s\n", __func__);

	if (!__is_lb_available())
		return 0;

	if (!pg) {
		lb_print(LB_ERROR, "%s:pages is  invalid\n", __func__);
		goto err;
	}

	if (!policy_id_valid(pid, GID_PAGES_REMAP)) {
		lb_print(LB_ERROR, "%s:policy_id invalid\n", __func__);
		goto err;
	}

	/* protect gid map and quota */
	spin_lock_irqsave(&lbdev->gdplc.lock, flags);
	policy = &lbdev->gdplc.ptbl[pid];
	if (!policy->page_count) {
		policy->gid = alloc_gid(policy);
		quota = alloc_quota(policy,
			count << (PAGE_SHIFT - LB_LINE_SHIFT));

		DEFINE_INIT_QUOTA(quota_set, (quota & QUOTA_L_MASK),
			(quota & QUOTA_H_MASK) >> QUOTA_L_SHIFT);
		DEFINE_INIT_ALLOC(allc_set, lbdev->way_en, lbdev->way_en);

		if (set_gid(policy, allc_set.value, quota_set.value,
			policy->plc))
			goto err_set_gid;
	}
	policy->page_count += count;
	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

	/* set pages */
	ret = set_page(policy, pg, count);
	if (ret) {
		lb_print(LB_ERROR, "%s: set page failed\n", __func__);
		goto err_set_page;
	}

	lb_print(LB_INFO, "out %s\n", __func__);

	return 0;

err_set_page:
	spin_lock_irqsave(&lbdev->gdplc.lock, flags);

	if (WARN_ON(policy->page_count < count))
		policy->page_count = 0;
	else
		policy->page_count -= count;

	if (!policy->page_count) {
		reset_gid(policy);
		free_quota(policy, quota);
		free_gid(policy);
	}

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

err:
	return -EINVAL;

err_set_gid:
	free_quota(policy, quota);
	free_gid(policy);
	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);
	return -EINVAL;
}

int lb_pages_detach(unsigned int pid, struct page *pages, size_t count)
{
	struct lb_plc_info *policy = NULL;
	unsigned long flags = 0;

	lb_print(LB_INFO, "into%s\n", __func__);

	if (!__is_lb_available())
		return 0;

	if (!pages) {
		lb_print(LB_ERROR, "%s:pages is  invalid\n", __func__);
		return -EINVAL;
	}

	if (!policy_id_valid(pid, GID_PAGES_REMAP)) {
		lb_print(LB_ERROR, "%s:policy_id invalid\n", __func__);
		return -EINVAL;
	}

	policy = &lbdev->gdplc.ptbl[pid];
	reset_page(policy, pages, count);

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);

	if (WARN_ON(policy->page_count < count))
		policy->page_count = 0;
	else
		policy->page_count -= count;

	if (!policy->page_count) {
		reset_gid(policy);
		free_quota(policy, 0);
		free_gid(policy);
	}

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

	lb_print(LB_INFO, "out%s\n", __func__);

	return 0;
}

int lb_sg_attach(unsigned int pid, struct scatterlist *sgl, unsigned int nents)
{
	int i, j;
	struct scatterlist *sg = NULL;

	lb_print(LB_INFO, "into %s pid %u\n", __func__, pid);

	/* for gpu pid is normal call path */
	if (!__is_lb_available() || !pid)
		return 0;

	if (!sgl)
		return -EINVAL;

	for_each_sg(sgl, sg, (int)nents, i) {
		if (!sg || lb_pages_attach(pid, phys_to_page(sg_phys(sg)),
			sg->length >> PAGE_SHIFT))
			goto err;
	}

	lb_print(LB_INFO, "out %s\n", __func__);

	return 0;

err:
	for_each_sg(sgl, sg, i, j) {
		if (!sg || lb_pages_detach(pid, phys_to_page(sg_phys(sg)),
			sg->length >> PAGE_SHIFT))
			lb_print(LB_ERROR, "%s page detach failed\n", __func__);
	}

	return -EINVAL;
}

int lb_sg_detach(unsigned int pid, struct scatterlist *sgl, unsigned int nents)
{
	int i;
	struct scatterlist *sg = NULL;

	lb_print(LB_INFO, "into %s\n", __func__);

	/* for gpu pid is normal call path */
	if (!__is_lb_available() || !pid)
		return 0;

	if (!sgl && sg_nents(sgl) != nents)
		return -EINVAL;

	for_each_sg(sgl, sg, (int)nents, i)
		if (lb_pages_detach(pid, phys_to_page(sg_phys(sg)),
			sg->length >> PAGE_SHIFT))
			lb_print(LB_ERROR, "%s page detach failed\n", __func__);

	lb_print(LB_INFO, "out %s\n", __func__);

	return 0;
}

struct page *lb_alloc_pages(unsigned int pid, gfp_t gfp_mask,
		unsigned int order)
{
	struct page *lb_page = NULL;

	/*
	 * alloc pages
	 * pid = 0 for GPU
	 */
#ifdef CONFIG_MM_AMA
	lb_page = ama_alloc_meida_pages(gfp_mask, order);
#else
	lb_page = alloc_pages(gfp_mask, order);
#endif
	if (!lb_page) {
		lb_print(LB_ERROR, "%u:alloc pages failed\n", pid);
		goto err;
	}

	if (!__is_lb_available() || !pid)
		return lb_page;

	if (lb_pages_attach(pid, lb_page, 1ULL << order)) {
		lb_print(LB_ERROR, "%u:lb pages attach failed\n", pid);
		goto err;
	}

	return lb_page;

err:
	if (lb_page)
		__free_pages(lb_page, order);
	return NULL;
}

int lb_free_pages(unsigned int pid, struct page *pages, unsigned int order)
{
	if (!__is_lb_available() || !pid)
		goto succ;

	if (lb_pages_detach(pid, pages, 1UL << order)) {
		lb_print(LB_ERROR, "%s:lb pages detach failed\n", __func__);
		goto err_nofree;
	}

succ:
	if (pages)
		__free_pages(pages, order);
	return 0;

err_nofree:
	return -EINVAL;
}

int lb_gid_enable(unsigned int pid)
{
	if (pid && !is_support_bypass())
		return 0;

	return lb_request_quota(pid);
}

int lb_gid_bypass(unsigned int pid)
{
	if (pid && !is_support_bypass())
		return 0;

	return lb_release_quota(pid);
}

int perf_ctrl_lb_set_policy(void __user *uarg)
{
	int res;
	struct lb_policy_config lb_config;

	if (uarg == NULL)
		return -EINVAL;

	if (!__is_lb_available() || !is_support_bypass())
		return 0;

	if (copy_from_user(&lb_config, uarg, sizeof(struct lb_policy_config))) {
		pr_err("set master prio copy_to_user fail.\n");
		return -EFAULT;
	}

	res = lb_set_master_policy(lb_config.pid, lb_config.quota, lb_config.prio);

	return res;
}

int perf_ctrl_lb_reset_policy(void __user *uarg)
{
	int res;
	struct lb_policy_config lb_config;

	if (uarg == NULL)
		return -EINVAL;

	if (!__is_lb_available() || !is_support_bypass())
		return 0;

	if (copy_from_user(&lb_config, uarg, sizeof(struct lb_policy_config))) {
		pr_err("reset master prio copy_to_user fail.\n");
		return -EFAULT;
	}

	res = lb_reset_master_policy(lb_config.pid);

	return res;
}

int perf_ctrl_lb_lite_info(void __user *uarg)
{
	struct lb_policy_config lb_config = {};

	if (uarg == NULL)
		return -EINVAL;

	lb_config.available = __is_lb_available();

	if (copy_to_user(uarg, &lb_config, sizeof(struct lb_policy_config))) {
		pr_err("get lite info copy_to_user fail.\n");
		return -EFAULT;
	}

	return 0;
}

int get_flow_stat(struct lb_policy_flow *flow)
{
	unsigned int id;
	unsigned int pid;
	struct lb_plc_info *policy = NULL;

	if (!__is_lb_available())
		return 0;

	if (!flow) {
		lb_print(LB_ERROR, "policy flow is null\n");
		return -EINVAL;
	}

	pid = flow->pid;
	if (!policy_id_valid(pid, GID_NON_SEC | GID_BYPASS_STATE)) {
		lb_print(LB_ERROR, "0x%x: pid invalid\n", pid);
		return -EINVAL;
	}

	policy = &lbdev->gdplc.ptbl[pid];
	if (!policy->flowid || policy->flowid > FLOW_MAX_ID) {
		lb_print(LB_ERROR, "0x%x: pid not support flow\n", pid);
		return -EINVAL;
	}

	id = FLOWID_TO_SLC(policy->flowid);

	end_flow(id);
	flow->in = get_flow(id, 1);
	flow->out = get_flow(id, 0);
	start_flow(policy->gid, id);

	return 0;
}

unsigned int get_sc_size(void)
{
	if (!__is_lb_available() || !is_support_bypass())
		return 0;
	return lbdev->gdplc.max_size;
}

#ifdef CONFIG_MM_LB_L3_EXTENSION
void lb_ci_cache_exclusive(void)
{
	u32 r_quota;
	unsigned long flags = 0;
	struct lb_plc_info *policy = NULL;

	if (!__is_lb_available())
		return;

	if (!lbdev) {
		lb_print(LB_ERROR, "lbdev is null\n");
		return;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);

	policy = &lbdev->gdplc.ptbl[0];
	if (!(lbdev->power_state & BIT(0)) || policy->enabled != STATUS_ENABLE)
		goto unlock;

	/*
	 * save gid0 quota
	 * set gid0 quota 0
	 */
	r_quota = clear_gid_quota(0);

	if (lb_ops_cache(EVENT, CI, CMO_BY_GID, BIT(0), 0, 0))
		goto unlock;

	if (cmo_dummy_pa && lb_ops_cache(EVENT, CLEAN, CMO_BY_4XKPA, 0,
		cmo_dummy_pa, PAGE_SIZE))
		goto unlock;

	/* ensure cmo ops complete */
	mb();

	lb_cmo_sync_cmd_by_event();

	/* recover gid0 quota */
	set_gid_quota(0, r_quota);

unlock:
	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);
}
#endif
