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

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gfp.h>
#include <linux/sort.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <asm/cacheflush.h>
#include <linux/dma-direction.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/genalloc.h>
#include <dsm/dsm_pub.h>

#include "mm_lb_priv.h"
#include "mm_lb_driver.h"
#include "mm_lb_cache.h"
#include "mm_lb_policy.h"
#include <linux/mm_lb/mm_lb.h>

#define MAX_NSEC_GID_NUM          8

#define GID_TINY                  14
#define GID_HIFI                  15
struct lb_area {
	struct list_head next;
	struct vm_struct *area;
};

struct vm_info {
	struct list_head list;
	struct mutex mutex;
	struct gen_pool *pool;
};

static int get_policy_pdata(struct platform_device *pdev, struct lb_device *lbd)
{
	int i = 0;
	u32 prot;
	int ret;
	const char *plc_name = NULL;

	struct device_node *np = NULL;
	struct lb_plc_info *p_data = NULL;
	const struct device_node *dt_node = pdev->dev.of_node;

	lb_print(LB_INFO, "into %s\n", __func__);

	ret = of_property_read_u32(dt_node, "cmo-dummy-pa", &prot);
	if (ret >= 0)
		cmo_dummy_pa = prot;

	ret = of_property_read_u32(dt_node, "support-bypass", &prot);
	if (ret >= 0)
		lbd->gdplc.is_support_bypass = prot;

	ret = of_property_read_u32(dt_node, "max-quota", &prot);
	if (ret >= 0)
		lbd->gdplc.max_quota = prot;

	ret = of_property_read_u32(dt_node, "max-size", &prot);
	if (ret >= 0)
		lbd->gdplc.max_size = prot;

	/* if policy id is add, so plc-num also need be add */
	ret = of_property_read_u32(dt_node, "plc-num", &prot);
	if (ret < 0) {
		lb_print(LB_ERROR, "can not get plc number\n");
		return -EINVAL;
	}
	lbd->gdplc.nr = prot;

	ret = of_property_read_u32(dt_node, "slc-idx", &prot);
	if (ret < 0) {
		lb_print(LB_ERROR, "failed to get slice idx\n");
		return -EINVAL;
	}
	lbd->slc_idx = prot;

	ret = of_property_read_u32(dt_node, "way-en", &prot);
	if (ret < 0) {
		lb_print(LB_ERROR, "failed to get enable way\n");
		return -EINVAL;
	}
	lbd->way_en = prot;

	p_data = devm_kzalloc(&pdev->dev, sizeof(*p_data) * lbd->gdplc.nr,
				  GFP_KERNEL);
	if (!p_data) {
		lb_print(LB_ERROR, "[%s] failed to kzalloc\n", __func__);
		return -ENOMEM;
	}
	lbd->gdplc.ptbl = p_data;

	for_each_child_of_node(dt_node, np) {
		ret = of_property_read_string(np, "lb-name", &plc_name);
		if (ret < 0)
			continue;
		lbd->gdplc.ptbl[i].name = plc_name;

		ret = of_property_read_u32(np, "lb-pid", &prot);
		if (ret < 0)
			continue;

		if (i != prot)
			rdr_syserr_process_for_ap(MODID_AP_S_PANIC_LB,
					0ULL, 0ULL);

		lbd->gdplc.ptbl[i].pid = prot;

		ret = of_property_read_u32(np, "lb-prio", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].prio = prot;
		lbd->gdplc.ptbl[i].r_prio = prot;

		ret = of_property_read_u32(np, "lb-gid", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].gid = prot;

		ret = of_property_read_u32(np, "lb-stat", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].stat = prot;

		ret = of_property_read_u32(np, "lb-quota", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].quota = prot;
		lbd->gdplc.ptbl[i].r_quota = prot;

		ret = of_property_read_u32(np, "lb-alloc", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].alloc = prot;

		ret = of_property_read_u32(np, "lb-plc", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].plc = prot;

		ret = of_property_read_u32(np, "lb-flow", &prot);
		if (ret < 0)
			continue;

		lbd->gdplc.ptbl[i].flowid = prot;

		i++;
	}

	if (i != lbd->gdplc.nr)
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_LB, 0ULL, 0ULL);

	lb_print(LB_INFO, "out(%s)\n", __func__);

	return ret;
}

unsigned int __is_lb_available(void)
{
	return (unsigned int)(lbdev && lbdev->is_available);
}

static int change_size_from_efuse(struct lb_device *lbd)
{
	int i = 0, j = 0;
	unsigned int way = lbd->way_en;
	unsigned int way_efuse = lbd->way_efuse;

	while (way) {
		if ((way & BIT(0)) == (way_efuse & BIT(0)))
			j++;

		way >>= 1;
		way_efuse >>= 1;
		i++;
	}

	if (unlikely(i == 0))
		return -EINVAL;

	lbd->gdplc.max_quota = lbd->gdplc.max_quota / i * j;
	lbd->gdplc.max_size = lbd->gdplc.max_size / i * j;

	lb_print(LB_ERROR, "[%s] way: 0x%x, efuse: 0x%x, quota: 0x%x, size: 0x%x\n",
		__func__, lbd->way_en, lbd->way_efuse,
		lbd->gdplc.max_quota, lbd->gdplc.max_size);

	return 0;
}

unsigned int is_support_bypass(void)
{
	if (!lbdev) {
		lb_print(LB_ERROR, "lbdev is null\n");
		return 0;
	} else {
		return lbdev->gdplc.is_support_bypass;
	}
}

unsigned int alloc_quota(struct lb_plc_info *policy, unsigned int quota)
{
	return policy->quota;
}

void free_quota(struct lb_plc_info *policy, unsigned int quota)
{
	/*
	 * Current quota static alloc
	 * In the future, need dynamic free quota
	 */
}

unsigned int get_max_quota(void)
{
	if (!lbdev) {
		lb_print(LB_ERROR, "lbdev is null\n");
		return 0;
	} else {
		return lbdev->gdplc.max_quota;
	}
}

bool policy_id_valid(unsigned int policy_id, unsigned int mask)
{
	if (!lbdev) {
		lb_print(LB_ERROR, "lbdev is null\n");
		return false;
	}

	if ((!policy_id && !(mask & GID_BYPASS_STATE)) ||
			policy_id >= (unsigned int)lbdev->gdplc.nr) {
		lb_print(LB_ERROR, "policy_id%u is invalid\n", policy_id);
		return false;
	}

	if (!(lbdev->gdplc.ptbl[policy_id].stat & mask)) {
		lb_print(LB_ERROR, "policy_id%u mask 0x%x 0x%x is invalid\n",
			policy_id, mask, lbdev->gdplc.ptbl[policy_id].stat);
		return false;
	}

	return true;
}

unsigned int alloc_gid(struct lb_plc_info *policy)
{
	unsigned int i;

	if (policy->stat & GID_STATIC) {
		WARN_ON(!policy->gid);
		return policy->gid;
	}

	for (i = 1; i < MAX_NSEC_GID_NUM; i++) {
		if (!(lbdev->gdplc.gidmap & BIT(i))) {
			policy->gid = i;
			lbdev->gdplc.gidmap |= BIT(i);
			return policy->gid;
		}
	}

	return 0;
}

void free_gid(struct lb_plc_info *policy)
{
	if (policy->stat & GID_STATIC)
		return;

	lbdev->gdplc.gidmap &= ~BIT(policy->gid);
	policy->gid = 0;
}

static int update_status(struct lb_plc_info *policy, unsigned int quota_value)
{
	int ret = 0;
	gid quota_set;

	if (!(policy->stat & GID_BYPASS_STATE))
		return ret;

	DEFINE_INIT_QUOTA(quota_set, quota_value & QUOTA_L_MASK,
		quota_value & QUOTA_L_MASK);

	if (quota_value && policy->enabled != STATUS_ENABLE) {
		ret |= gid_bypass_control(GID_ENABLE, policy->gid,
			quota_set.value, policy->plc);
		policy->enabled = STATUS_ENABLE;
	}

	if (!quota_value && policy->enabled != STATUS_BYPASS) {
		ret |= gid_bypass_control(GID_BYPASS, policy->gid, 0, 0);
		policy->enabled = STATUS_BYPASS;
	}

	if (ret)
		lb_print(LB_ERROR, "%s is failed 0x%x 0x%x\n", __func__,
			policy->gid, quota_value);

	return ret;
}

static int set_quota_and_update(struct lb_plc_info *policy,
		unsigned int quota)
{
	int ret;
	unsigned long flags = 0;

	spin_lock_irqsave(&policy->lock, flags);
	set_quota(policy, quota);
	ret = update_status(policy, quota);
	spin_unlock_irqrestore(&policy->lock, flags);

	return ret;
}

static int update_quota(void)
{
	int i;
	int ret = 0;
	int quota_set = 0;
	int last_prio = 0;
	unsigned int quota;
	unsigned int remaining_quota = get_max_quota();
	struct lb_plc_info *policy = NULL;

	for (i = 0; i < lbdev->gdplc.job_cnt; i++) {
		policy = &lbdev->gdplc.ptbl[lbdev->gdplc.job_queue[i][0]];
		quota = (policy->quota & QUOTA_H_MASK) >> QUOTA_L_SHIFT;

		/* need set quota = 0 when job be deleted */
		if (lbdev->gdplc.job_queue[i][1] == LB_CACHE_LOW_PRIO) {
			ret |= set_quota_and_update(policy, 0);
			continue;
		}

		if (last_prio == policy->prio && quota_set) {
			ret |= set_quota_and_update(policy, quota);
			remaining_quota = remaining_quota > quota ?
				remaining_quota - quota : 0;
			continue;
		}

		if (remaining_quota < quota)
			quota_set = remaining_quota;
		else
			quota_set = quota;

		remaining_quota -= quota_set;
		ret |= set_quota_and_update(policy, quota_set);
		last_prio = policy->prio;
	}

	return ret;
}

static int cmp(const void *a, const void *b)
{
	const unsigned int *ap = (unsigned int *)a;
	const unsigned int *bp = (unsigned int *)b;

	return ap[1] < bp[1] ? -1 : ap[1] > bp[1] ? 1 : 0;
}

static int update_job(void)
{
	sort(lbdev->gdplc.job_queue, lbdev->gdplc.job_cnt,
		sizeof(lbdev->gdplc.job_queue[0]), cmp, NULL);

	return update_quota();
}

static int add_job(unsigned int pid, unsigned int prio)
{
	int i;

	if (pid && !is_support_bypass())
		return 0;

	if (lbdev->gdplc.job_cnt >= PID_MAX) {
		lb_print(LB_ERROR, "%s invalid add job 0x%x 0x%x\n",
			__func__, pid, prio);
		return -EPERM;
	}

	for (i = 0; i < lbdev->gdplc.job_cnt; i++) {
		if (lbdev->gdplc.job_queue[i][0] == pid)
			return 0;
	}

	lbdev->gdplc.job_queue[lbdev->gdplc.job_cnt][0] = pid;
	lbdev->gdplc.job_queue[lbdev->gdplc.job_cnt][1] = prio;

	lbdev->gdplc.job_cnt++;

	return update_job();
}

static int del_job(unsigned int pid)
{
	int i, index;
	int ret;

	if (pid && !is_support_bypass())
		return 0;

	index = lbdev->gdplc.job_cnt;
	for (i = 0; i < lbdev->gdplc.job_cnt; i++)
		if (lbdev->gdplc.job_queue[i][0] == pid)
			index = i;

	if (index == lbdev->gdplc.job_cnt)
		return 0;

	lbdev->gdplc.job_queue[index][0] = pid;
	lbdev->gdplc.job_queue[index][1] = LB_CACHE_LOW_PRIO;

	ret = update_job();

	lbdev->gdplc.job_cnt--;

	return ret;
}

int lb_set_master_policy(unsigned int pid, unsigned int quota,
		unsigned int prio)
{
	int i;
	unsigned long flags = 0;
	struct lb_plc_info *policy = NULL;

	if (!is_support_bypass())
		return 0;

	if (prio < LB_BUF_HIGH_PRIO || prio > LB_CACHE_LOW_PRIO)
		return -EINVAL;

	if (!policy_id_valid(pid, GID_NON_SEC | GID_BYPASS_STATE)) {
		lb_print(LB_ERROR, "0x%x: pid invalid\n", pid);
		return -EINVAL;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);
	policy = &lbdev->gdplc.ptbl[pid];
	policy->prio = prio;

	for (i = 0; i < lbdev->gdplc.job_cnt; i++) {
		if (lbdev->gdplc.job_queue[i][0] == pid) {
			lbdev->gdplc.job_queue[i][1] = prio;
			update_job();
			break;
		}
	}

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

	return 0;
}

int lb_reset_master_policy(unsigned int pid)
{
	int i;
	unsigned long flags = 0;
	struct lb_plc_info *policy = NULL;

	if (!is_support_bypass())
		return 0;

	if (!policy_id_valid(pid, GID_NON_SEC | GID_BYPASS_STATE)) {
		lb_print(LB_ERROR, "0x%x: pid invalid\n", pid);
		return -EINVAL;
	}

	spin_lock_irqsave(&lbdev->gdplc.lock, flags);
	policy = &lbdev->gdplc.ptbl[pid];
	policy->prio = policy->r_prio;
	policy->quota = policy->r_quota;

	for (i = 0; i < lbdev->gdplc.job_cnt; i++) {
		if (lbdev->gdplc.job_queue[i][0] == pid) {
			lbdev->gdplc.job_queue[i][1] = policy->prio;
			update_job();
			break;
		}
	}

	spin_unlock_irqrestore(&lbdev->gdplc.lock, flags);

	return 0;
}

int gid_enable(struct lb_plc_info *policy)
{
	int ret;

	if (policy->pid && !is_support_bypass())
		return 0;

	if (policy->pid && !policy->page_count) {
		lb_print(LB_ERROR, "%s gid:0x%x page count invalid\n",
			__func__, policy->pid);
		return -EPERM;
	}

	/* lb need powerup when lb is powered down */
	if (!lbdev->power_state && power_control(POWERUP)) {
		lb_print(LB_ERROR, "%s powerup fail\n", __func__);
		return -EINVAL;
	}

	ret = add_job(policy->pid, policy->prio);
	if (ret) {
		lb_print(LB_ERROR, "%s 0x%x, %d fail\n",
			__func__, policy->pid, ret);

		return ret;
	}

	lbdev->power_state |= BIT(policy->gid);

	return ret;
}

int gid_bypass(struct lb_plc_info *policy)
{
	int ret;

	if (policy->pid && !is_support_bypass())
		return 0;

	ret = del_job(policy->pid);
	if (ret) {
		lb_print(LB_ERROR, "%s pid: 0x%x, ret: %d fail\n",
			__func__, policy->pid, ret);
		return -EINVAL;
	}

	/* lb need powerdown when only gid is enable */
	if (lbdev->power_state && !(lbdev->power_state & ~BIT(policy->gid)))
		ret = power_control(POWERDOWN);

	lbdev->power_state &= ~BIT(policy->gid);

	return ret;
}

int sec_gid_enable(struct lb_plc_info *policy)
{
	int ret = 0;

	/* lb need powerup when lb is powered down */
	if (!lbdev->power_state)
		if (power_control(POWERUP))
			return -EINVAL;
	lbdev->power_state |= BIT(policy->gid);

	if (policy->stat & GID_SUSPEND) {
		gid_suspend_enable(policy->gid);
		return 0;
	} else {
		sec_gid_set(SEC_GID_REQUEST, policy->gid, 0);
	}

	/* ensure set gid cfg complete */
	mb();
	ret = add_job(policy->pid, policy->prio);
	return ret;
}

int sec_gid_disable(struct lb_plc_info *policy)
{
	int ret = 0;

	if (!(policy->stat & GID_SUSPEND)) {
		sec_gid_set(SEC_GID_RELEASE, policy->gid, 0);
		ret = del_job(policy->pid);
	}
	if (policy->pid == PID_HIFI)
		set_gid_config(policy->gid, BIT(GID_EN_SHIFT), 0, 0);
	else
		set_gid_config(policy->gid, 0, 0, 0);

	policy->enabled = STATUS_DISABLE;
	lbdev->power_state &= ~BIT(policy->gid);

	/* lb need powerdown when all gid is disable */
	if (!lbdev->power_state)
		ret |= power_control(POWERDOWN);

	return ret;
}

int set_gid(struct lb_plc_info *policy, unsigned int way_alloc,
		unsigned int quota_set, unsigned int plc_set)
{
	int ret = 0;
	gid allc_efuse;

	/* lb need powerup when lb is powered down */
	if (!lbdev->power_state)
		if (power_control(POWERUP))
			return -EINVAL;

	DEFINE_INIT_ALLOC(allc_efuse, lbdev->way_efuse, lbdev->way_efuse);
	set_gid_config(policy->gid, plc_set, way_alloc & allc_efuse.value, quota_set);

	/* ensure set gid cfg complete */
	mb();
	policy->enabled = STATUS_ENABLE;
	lbdev->power_state |= BIT(policy->gid);

	ret = add_job(policy->pid, policy->prio);
	if ((policy->stat & GID_BYPASS_STATE) && policy->pid != PID_NPU)
		gid_bypass(policy);
	return ret;
}

int reset_gid(struct lb_plc_info *policy)
{
	int ret = 0;

	lb_print(LB_ERROR, "%s: gid = %u\n", __func__, policy->gid);

	/* doubel free same gid */
	if (!(lbdev->power_state & BIT(policy->gid))) {
		if (is_support_bypass() && (policy->stat & GID_BYPASS_STATE)) {
			clr_bypass_status(policy);
			policy->enabled = STATUS_DISABLE;
		} else {
			lb_print(LB_ERROR, "double free gid %u %u\n",
				lbdev->power_state, policy->gid);
		}

		return 0;
	}

	ret |= del_job(policy->pid);

	set_gid_config(policy->gid, 0, 0, 0);
	if (is_support_bypass() && (policy->stat & GID_BYPASS_STATE))
		clr_bypass_status(policy);

	/* ensure set gid cfg complete */
	mb();
	policy->enabled = STATUS_DISABLE;

	lbdev->power_state &= ~BIT(policy->gid);

	/* lb need powerdown when all gid is disable */
	if (!lbdev->power_state)
		ret |= power_control(POWERDOWN);

	return ret;
}

int lb_pfn_valid_within(phys_addr_t phys)
{
	/* In the future, need handle invalid pfn */
	return 1;
}

#ifndef CONFIG_LIBLINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int f(pte_t *pte, unsigned long addr, void *data)
{
	pte_t ***p = data;

	if (p) {
		*(*p) = pte;
		(*p)++;
	}
	return 0;
}

static struct vm_struct *__alloc_vm_area(size_t size, pte_t **ptes)
{
	struct vm_struct *area;

	area = get_vm_area_caller(size, VM_IOREMAP,
				__builtin_return_address(0));
	if (area == NULL)
		return NULL;

	/*
	 * This ensures that page tables are constructed for this region
	 * of kernel virtual address space and mapped into init_mm.
	 */
	if (apply_to_page_range(&init_mm, (unsigned long)area->addr,
				size, f, ptes ? &ptes : NULL)) {
		free_vm_area(area);
		return NULL;
	}

	return area;
}
#endif

static int add_gid_vm(struct vm_info *vm, size_t sz)
{
	struct lb_area *vm_ar = NULL;

	vm_ar = kzalloc(sizeof(struct lb_area), GFP_KERNEL);
	if (!vm_ar) {
		lb_print(LB_ERROR, "%s kzalloc failed\n", __func__);
		goto err;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	vm_ar->area = __alloc_vm_area(sz, NULL);
#else
	vm_ar->area = alloc_vm_area(sz, NULL);
#endif
	if (!vm_ar->area) {
		lb_print(LB_ERROR, "%s alloc vm area failed\n", __func__);
		goto free_vm_ar;
	}

	if (gen_pool_add(vm->pool, (uintptr_t)vm_ar->area->addr, sz, -1)) {
		lb_print(LB_ERROR, "%s add gen pool failed\n", __func__);
		goto free_area;
	}

	list_add_tail(&vm_ar->next, &vm->list);

	return 0;

free_area:
	free_vm_area(vm_ar->area);

free_vm_ar:
	kfree(vm_ar);

err:
	return -ENOMEM;
}

static int map_page_range(unsigned long va, size_t sz,
		pgprot_t prot, struct page **pages)
{
	int err;

	err = map_kernel_range_noflush(va, sz, prot, pages);
	flush_cache_vmap(va, va + sz);

	return err > 0 ? 0 : err;
}

static inline void free_pool(struct vm_info *vm,
		unsigned long va, size_t sz)
{
	gen_pool_free(vm->pool, va, sz);
}

static unsigned long alloc_pool(struct vm_info *vm, size_t sz)
{
	unsigned long va;

	/* try alloc pool, gen_pool_alloc have lock */
	va = gen_pool_alloc(vm->pool, sz);
	if (!va) {
		/* add gid vm need mutex lock */
		mutex_lock(&vm->mutex);

		va = gen_pool_alloc(vm->pool, sz);
		if (!va && !add_gid_vm(vm, SZ_64M))
			va = gen_pool_alloc(vm->pool, sz);

		mutex_unlock(&vm->mutex);
	}

	return va;
}

static void vm_unmap(struct vm_info *vm, const void *va, size_t sz)
{
	if (!vm) {
		lb_print(LB_ERROR, "vm not alloc\n");
		return;
	}

	unmap_kernel_range((uintptr_t)va, sz);
	free_pool(vm, (uintptr_t)va, sz);
}

static void *vm_map(struct vm_info *vm, struct page **pages,
		size_t cnt, pgprot_t prot)
{
	void *va = NULL;
	size_t sz = cnt << PAGE_SHIFT;

	if (!vm) {
		lb_print(LB_ERROR, "%s vm not alloc\n", __func__);
		return NULL;
	}

	va = (void *)alloc_pool(vm, sz);
	if (!va) {
		lb_print(LB_ERROR, "alloc pool failed\n");
		return NULL;
	}

	if (map_page_range((uintptr_t)va, sz, prot, pages)) {
		free_pool(vm, (uintptr_t)va, sz);
		lb_print(LB_ERROR, "map page range failed\n");
		return NULL;
	}

	return va;
}

static void vm_uninit(struct lb_device *lbd)
{
	int i;
	struct vm_info *vm = NULL;
	struct lb_area *vm_ar = NULL;
	struct lb_area *tmp_ar = NULL;

	for (i = 0; i < lbd->gdplc.nr; i++) {
		vm = lbd->gdplc.ptbl[i].vm;
		if (!vm)
			continue;

		list_for_each_entry_safe(vm_ar, tmp_ar, &vm->list, next) {
			list_del(&vm_ar->next);

			if (vm_ar->area)
				free_vm_area(vm_ar->area);

			kfree(vm_ar);
		}

		if (vm->pool)
			gen_pool_destroy(vm->pool);

		mutex_destroy(&vm->mutex);

		kfree(vm);
		lbd->gdplc.ptbl[i].vm = NULL;
	}
}

static int vm_init(struct lb_device *lbd)
{
	int i;
	struct vm_info *vm = NULL;

	for (i = 0; i < lbd->gdplc.nr; i++) {
		if (!(lbd->gdplc.ptbl[i].stat & GID_VM_STATE))
			continue;

		vm = kzalloc(sizeof(struct vm_info), GFP_KERNEL);
		if (!vm) {
			lb_print(LB_ERROR, "kzalloc failed\n");
			goto vm_free;
		}
		lbd->gdplc.ptbl[i].vm = vm;

		mutex_init(&(vm->mutex));
		INIT_LIST_HEAD(&(vm->list));

		vm->pool = gen_pool_create(PAGE_SHIFT, -1);
		if (!vm->pool) {
			lb_print(LB_ERROR, "gen pool create failed\n");
			goto vm_free;
		}

		if (add_gid_vm(vm, SZ_64M)) {
			lb_print(LB_ERROR, "add pool failed\n");
			goto vm_free;
		}
	}

	return 0;

vm_free:
	vm_uninit(lbd);

	return -ENOMEM;
}
#endif

int set_page(struct lb_plc_info *policy,
		struct page *page_alloc, size_t count)
{
	void *vaddr = NULL;
	struct page *pg = page_alloc;
	struct page **pages = NULL;
	struct page **tmp = NULL;
	size_t i;

#ifdef CONFIG_MM_LB_L3_EXTENSION
	change_secpage_range(page_to_phys(page_alloc),
		(uintptr_t)page_address(page_alloc),
		count << PAGE_SHIFT, __pgprot(PROT_DEVICE_nGnRE));
	flush_tlb_kernel_range((uintptr_t)page_to_virt(page_alloc),
		(uintptr_t)page_to_virt(pg) + (count << PAGE_SHIFT));
#endif

	/*
	 * invalid acpu cache by normal va
	 * it is necessary by normal va
	 * cache |--va--| |--va with gid--|
	 * lb             |-------lb------|
	 * ddr   |------|
	 */
	__dma_unmap_area(page_to_virt(pg), count << PAGE_SHIFT,
			DMA_FROM_DEVICE);

	pages = kcalloc(count, sizeof(struct page *), GFP_KERNEL);
	if (!pages) {
		lb_print(LB_ERROR, "zalloc page\n");
		goto change_mapping;
	}

	tmp = pages;
	for (i = 0; i < count; i++)
		*(tmp++) = pg++;

#ifndef CONFIG_LIBLINUX
	vaddr = vm_map(policy->vm, pages, count,
			pgprot_lb(PAGE_KERNEL, policy->gid));
#else
	vaddr = vmap(pages, count,
	   VM_MAP, pgprot_lb(PAGE_KERNEL, policy->gid));
#endif
	if (!vaddr) {
		lb_print(LB_ERROR, "%s:vm map failed\n", __func__);
		goto free_page;
	}

	kfree(pages);

	/* set the page privite */
	pg = page_alloc;

	for (i = 0; i < count; i++) {
		set_page_private(pg, (uintptr_t)vaddr | policy->gid);
		SetPagePrivate(pg);
		SetPageLB(pg);
		pg++;
		vaddr += PAGE_SIZE;
	}

	return 0;

free_page:
	kfree(pages);

change_mapping:
#ifdef CONFIG_MM_LB_L3_EXTENSION
	change_secpage_range(page_to_phys(page_alloc),
		(uintptr_t)page_address(page_alloc),
		count << PAGE_SHIFT, PAGE_KERNEL);
	flush_tlb_kernel_range((uintptr_t)page_to_virt(page_alloc),
		(uintptr_t)page_to_virt(page_alloc) + (count << PAGE_SHIFT));
#endif

	return -ENOMEM;
}

void reset_page(struct lb_plc_info *policy,
		struct page *page, size_t count)
{
	int i;
	unsigned int gid;
	void *vaddr = NULL;
	struct page *pg = page;
	phys_addr_t phys = page_to_phys(pg);
	unsigned long size = count << PAGE_SHIFT;

	if (!page) {
		lb_print(LB_ERROR, "%s:reset page falied\n", __func__);
		return;
	}

	WARN_ON(!PageLB(pg));
	gid = lb_page_to_gid(pg);
	WARN(policy->gid != gid, "pgid %u ggid %u\n", policy->gid, gid);

	/*
	 * invalid acpu cache by lb va
	 * it is necessary by lb va
	 * cache |--va--| |--va with gid--|
	 * lb             |-------lb------|
	 * ddr   |------|
	 */
	vaddr = lb_page_to_virt(page);
	__dma_unmap_area(vaddr, size, DMA_FROM_DEVICE);

#ifndef CONFIG_LIBLINUX
	vm_unmap(policy->vm, vaddr, size);
#else
	vunmap(vaddr);
#endif

	for (i = 0; i < (int)count; i++) {
		set_page_private(pg, 0);
		ClearPagePrivate(pg);
		ClearPageLB(pg);
		pg++;
	}

	/* invalid system cache */
	lb_invalid_cache(gid, phys, size);

#ifdef CONFIG_MM_LB_L3_EXTENSION
	change_secpage_range(page_to_phys(page),
		(uintptr_t)page_address(page),
		count << PAGE_SHIFT, PAGE_KERNEL);
	flush_tlb_kernel_range((uintptr_t)page_to_virt(page),
		(uintptr_t)page_to_virt(page) + (count << PAGE_SHIFT));
#endif
}

static int soc_init(struct platform_device *pdev, struct lb_device *lbd)
{
	int irq;

	lb_print(LB_INFO, "into %s\n", __func__);

	/* register intr */
	irq = platform_get_irq_byname(pdev, "dfx-intr");
	if (irq < 0) {
		lb_print(LB_ERROR, "get dfx irq failed\n");
		return -EINVAL;
	}
	lbd->dfx_irq = irq;

	irq = platform_get_irq_byname(pdev, "cmd-intr");
	if (irq < 0) {
		lb_print(LB_ERROR, "get cmd-intr failed\n");
		return -EINVAL;
	}
	lbd->cmd_irq = irq;

	/* enable intr reg */
	if (reset_ip(lbd)) {
		lb_print(LB_ERROR, "reset ip failed\n");
		return -EINVAL;
	}

	lb_print(LB_INFO, "out %s\n", __func__);

	return 0;
}

static void gidmap_init(struct lb_device *lbd)
{
	int i;

	for (i = 0; i < lbd->gdplc.nr; i++) {
		if (!(lbd->gdplc.ptbl[i].stat & GID_STATIC))
			continue;

		lbd->gdplc.gidmap |= BIT(lbd->gdplc.ptbl[i].gid);
		spin_lock_init(&lbd->gdplc.ptbl[i].lock);
	}
}

static int gid_init(struct lb_device *lbd)
{
	lb_print(LB_INFO, "into %s\n", __func__);

	spin_lock_init(&lbd->gdplc.lock);

	gidmap_init(lbd);

#ifndef CONFIG_LIBLINUX
	if (vm_init(lbd)) {
		lb_print(LB_ERROR, "vm init failed\n");
		return -EINVAL;
	}
#endif

	if (change_size_from_efuse(lbd)) {
		lb_print(LB_ERROR, "change size from efuse failed\n");
		return -EINVAL;
	}

	lb_print(LB_INFO, "out %s\n", __func__);

	return 0;
}

static void dsm_lb_handle_work(struct work_struct *work)
{
	struct dsm_client *ai_client;

	ai_client = dsm_find_client("dsm_ai");
	if (!ai_client) {
		lb_print(LB_ERROR, "lb dsm is null\n");
		return;
	}
	lbdev->dsm.ai_client = ai_client;
}

static int dsm_init(struct lb_device *lbd)
{
	INIT_WORK(&(lbd->dsm.dsm_work), dsm_lb_handle_work);

	return 0;
}

static int lb_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	struct lb_device *lb_dev = NULL;
	struct device *dev = &pdev->dev;

	lb_print(LB_INFO, "--> %s\n", __func__);

	lb_dev = devm_kzalloc(dev, sizeof(*lb_dev), GFP_KERNEL);
	if (!lb_dev) {
		lb_print(LB_ERROR, "failed to allocate lb_dev\n");
		return -ENOMEM;
	}
	lb_dev->dev = dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		lb_print(LB_ERROR, "failed to get resource\n");
		return -EINVAL;
	}

	lb_dev->base = devm_ioremap_resource(dev, res);
	if (!lb_dev->base) {
		lb_print(LB_ERROR, "io remap failed\n");
		return -EINVAL;
	}

	lb_dev->is_available = !get_lb_lite();
	if (!lb_dev->is_available) {
		lb_print(LB_ERROR, "lb unusable\n");
		lbdev = lb_dev;
		return 0;
	}

	/* get the gid set from dts */
	if (get_policy_pdata(pdev, lb_dev)) {
		lb_print(LB_ERROR, "failed to get gid set from dts\n");
		return -EINVAL;
	}

	lb_dev->way_efuse = get_lb_efuse();
	if ((lb_dev->way_efuse & lb_dev->way_en) != lb_dev->way_efuse) {
		lb_print(LB_ERROR, "lb efuse init fail\n");
		return -EINVAL;
	}

	if (soc_init(pdev, lb_dev)) {
		lb_print(LB_ERROR, "lb init failed\n");
		return -EINVAL;
	}

	if (gid_init(lb_dev)) {
		lb_print(LB_ERROR, "gid init failed\n");
		return -EINVAL;
	}

	if (lb_pmu_init(pdev, lb_dev)) {
		lb_print(LB_ERROR, "pmu init failed\n");
		return -EINVAL;
	}

	if (dsm_init(lb_dev)) {
		lb_print(LB_ERROR, "dsm init failed\n");
		return -EINVAL;
	}

	/* register to loacal device */
	lbdev = lb_dev;

	/*
	 * need lb power down
	 * because lb powerup when atf init
	 */
	if (power_control(POWERDOWN)) {
		lb_print(LB_ERROR, "powerdown failed\n");
		return -EINVAL;
	}

	lb_print(LB_ERROR, "lb init success\n");

	return 0;
}

static int lb_remove(struct platform_device *pdev)
{
	lbdev = NULL;
	return 0;
}

static const struct of_device_id lb_match_table[] = {
	{.compatible = "hisilicon,hisi-lb" },
	{},
};

static struct platform_driver lb_driver = {
	.probe = lb_probe,
	.remove = lb_remove,
	.driver = {
		.name = "lb-hisi",
		.owner = THIS_MODULE,
		.of_match_table = lb_match_table,
	},
};

static int lb_init_ns(void)
{
	return platform_driver_register(&lb_driver);
}
subsys_initcall(lb_init_ns);
module_param_named(debug_level, lb_d_lvl, uint, 0644);
