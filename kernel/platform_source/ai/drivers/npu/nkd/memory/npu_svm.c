/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description:
 */
#include "npu_svm.h"

#include <linux/err.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/pid.h>
#include <linux/iommu/mm_svm.h>
#include <linux/sched/task.h>
#include <securec.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
#include <linux/sched/mm.h>
#endif

#include "mm_smmu.h"
#include "npu_platform.h"
#include "npu_log.h"
#ifdef CONFIG_FFRT_UNIFORM_ID
#include "uniform_id.h"
#endif

struct t_svm_pid_info {
	// the key of the list is svm pid
	pid_t svm_pid;
	pid_t manager_pid;
	uint16_t ssid;
	struct mm_svm *npu_svm;
	struct mm_struct *mm;
};

#define DRVDEV_APP_MAX_NUM 30
struct t_svm_manager {
	struct mutex lock;
	uint32_t item_num;
	struct t_svm_pid_info svm_pid_info[DRVDEV_APP_MAX_NUM];
};

int npu_svm_manager_init(u8 devid)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;

	if (devid >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return -1;
	}

	svm_manager = kmalloc(sizeof(struct t_svm_manager), GFP_KERNEL);
	if (svm_manager == NULL) {
		npu_drv_err("no sys mem to alloc svm manager\n");
		return -ENOMEM;
	}
	(void)memset_s(svm_manager, sizeof(struct t_svm_manager), 0x0,
		sizeof(struct t_svm_manager));
	cur_dev_ctx->svm_manager = (void *)svm_manager;
	mutex_init(&svm_manager->lock);
	return 0;
}

static int npu_get_item_bypid(struct t_svm_manager *svm_manager,
	pid_t svm_pid)
{
	// if find the item, return its index,
	// otherwise return the first empty item.
	int i;
	int j = -1;

	for (i = 0; i < DRVDEV_APP_MAX_NUM; i++) {
		if (svm_manager->svm_pid_info[i].svm_pid == svm_pid)
			break;
		if (svm_manager->svm_pid_info[i].manager_pid == 0 && j < 0)
			j = i;
	}
	return i < DRVDEV_APP_MAX_NUM ? i : j;
}

static int npu_check_item_bypid(struct t_svm_manager *svm_manager,
	pid_t svm_pid)
{
	int i;

	for (i = 0; i < DRVDEV_APP_MAX_NUM; i++) {
		if (svm_manager->svm_pid_info[i].svm_pid == svm_pid)
			return i;
	}
	return -1;
}

static int npu_get_item_manager_pid(struct t_svm_manager *svm_manager)
{
	int i;

	for (i = 0; i < DRVDEV_APP_MAX_NUM; i++) {
		if (svm_manager->svm_pid_info[i].manager_pid != 0 &&
			(svm_manager->svm_pid_info[i].manager_pid == svm_manager->svm_pid_info[i].svm_pid))
			return i;
	}
	return -1;
}

#ifdef CONFIG_LDK_SVM
int npu_svm_bind(struct npu_dev_ctx *cur_dev_ctx, pid_t manager_pid, pid_t svm_pid)
{
	int ret = 0;
	int item = 0;
	struct t_svm_manager *svm_manager = NULL;
	struct npu_platform_info *plat_info = NULL;

	cond_return_error(cur_dev_ctx == NULL, -1, "illegal cur_dev_ctx\n");

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");

	if (manager_pid <= 0 || svm_pid <= 0) {
		npu_drv_err("illegal pid\n");
		return -1;
	}

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "illegal svm_manager\n");

	mutex_lock(&svm_manager->lock);

	item = npu_check_item_bypid(svm_manager, svm_pid);
	cond_goto_error(item == -1, free_lock, ret, -1, "get item failed\n");

	if (svm_manager->svm_pid_info[item].npu_svm != NULL) {
		npu_drv_info("pid:%d had svm bind task\n", svm_pid);
		ret = 0;
		goto free_lock;
	}

	svm_manager->svm_pid_info[item].npu_svm =
		(void *)mm_svm_bind_task(plat_info->pdev, svm_pid);

	if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
		npu_drv_err("mm svm bind task failed\n");
		ret = -EBUSY;
	}

free_lock:
	mutex_unlock(&svm_manager->lock);
	return ret;
}
#else
int npu_svm_bind(struct npu_dev_ctx *cur_dev_ctx, pid_t manager_pid, pid_t svm_pid, uint16_t *uid)
{
	int ret = 0;
	int item = 0;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;
	struct t_svm_manager *svm_manager = NULL;
	struct npu_platform_info *plat_info = NULL;
	cond_return_error(cur_dev_ctx == NULL, -1, "illegal cur_dev_ctx\n");
	
	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");

	if (manager_pid <= 0 || svm_pid <= 0 || uid == NULL) {
		npu_drv_err("illegal pid\n");
		return -1;
	}

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "illegal svm_manager\n");

	mutex_lock(&svm_manager->lock);

	item = npu_check_item_bypid(svm_manager, svm_pid);
	cond_goto_error(item == -1, free_lock, ret, -1, "get item failed\n");

	if (svm_manager->svm_pid_info[item].npu_svm != NULL) {
		npu_drv_info("pid:%d had svm bind task\n", svm_pid);
		ret = 0;
		goto free_lock;
	}

	pid_struct = find_get_pid(svm_pid);
	cond_goto_error(pid_struct == NULL, free_lock, ret, -1, "get pid failed\n");

	task = get_pid_task(pid_struct, PIDTYPE_PID);
	cond_goto_error(task == NULL, free_pid, ret, -1, "get pid task failed\n");

	svm_manager->svm_pid_info[item].mm = get_task_mm(task);
	cond_goto_error(svm_manager->svm_pid_info[item].mm == NULL, free_task, ret, -1, "get task mm failed\n");

	svm_manager->svm_pid_info[item].npu_svm =
		(void*)mm_svm_bind_task(plat_info->pdev, task, svm_manager->svm_pid_info[item].ssid);
	if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
		npu_drv_err("mm svm bind task failed\n");
		mmput(svm_manager->svm_pid_info[item].mm);
		svm_manager->svm_pid_info[item].mm = NULL;
		ret = -EBUSY;
	}

#ifdef CONFIG_NPU_MMAP
	/* server proc cannot hold mm, otherwise the following situations may occur:
	   mm has hold npu0 file's refs because user calls npu_map.
	   when server proc exits abnormally, npu_manager is released, and npu0 may have unfinished tasks,
	   then server proc's svm will be released in npu0_release lately.
	   because the svm'mm has hold npu0 file's refs, so npu0 will never be released. */
	if (svm_manager->svm_pid_info[item].svm_pid == svm_manager->svm_pid_info[item].manager_pid) {
		mmput(svm_manager->svm_pid_info[item].mm);
		svm_manager->svm_pid_info[item].mm = NULL;
	}
#endif

free_task:
	put_task_struct(task);
free_pid:
	put_pid(pid_struct);
free_lock:
	mutex_unlock(&svm_manager->lock);
	if (ret == 0)
		*uid = svm_manager->svm_pid_info[item].ssid;
	return ret;
}
#endif

int npu_svm_unbind(u8 devid, pid_t svm_pid)
{
	int item = 0;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "illegal cur_dev_ctx\n");

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "illegal svm_manager\n");

	mutex_lock(&svm_manager->lock);

	item = npu_check_item_bypid(svm_manager, svm_pid);
	if (item == -1) {
		mutex_unlock(&svm_manager->lock);
		npu_drv_warn("not find item, svm_pid=%d\n", svm_pid);
		return 0;
	}

	if (svm_manager->svm_pid_info[item].npu_svm != NULL) {
		mm_svm_unbind_task(svm_manager->svm_pid_info[item].npu_svm);
		svm_manager->svm_pid_info[item].npu_svm = NULL;
		npu_drv_warn("unbind item, mnt_pid=%d svm_pid=%d",
			svm_manager->svm_pid_info[item].manager_pid, svm_pid);
	}

#ifndef CONFIG_LDK_SVM
	if (svm_manager->svm_pid_info[item].mm != NULL) {
		mmput(svm_manager->svm_pid_info[item].mm);
		svm_manager->svm_pid_info[item].mm = NULL;
		npu_drv_warn("put mm, mnt_pid=%d svm_pid=%d",
			svm_manager->svm_pid_info[item].manager_pid, svm_pid);
	}
#endif

	mutex_unlock(&svm_manager->lock);
	return 0;
}

// for current and app process
// 0. item should exist 1.[bind svm] 2.get ssid
#ifdef CONFIG_LDK_SVM
int npu_get_ssid_bypid(u8 devid, pid_t manager_pid, pid_t svm_pid,
	uint16_t *ssid, uint64_t *ttbr, uint64_t *tcr)
{
	int ret = 0;
	int item = 0;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;
	struct npu_platform_info *plat_info = NULL;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");

	if (devid >= NPU_DEV_NUM || manager_pid <= 0 || svm_pid <= 0 ||
		ssid == NULL || ttbr == NULL || tcr == NULL) {
		npu_drv_err("illegal input para\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "illegal cur_dev_ctx\n");

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "illegal svm_manager\n");

	mutex_lock(&svm_manager->lock);

	item = npu_check_item_bypid(svm_manager, svm_pid);
	cond_goto_error(item == -1, free_resource, ret, -1, "get item failed\n");

	if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
		svm_manager->svm_pid_info[item].npu_svm =
			(void*)mm_svm_bind_task(plat_info->pdev, svm_pid);
		if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
			npu_drv_err("mm svm bind task failed\n");
			ret = -EBUSY;
			goto free_resource;
		}
	}

	ret = mm_svm_get_ssid(svm_manager->svm_pid_info[item].npu_svm, ssid, ttbr, tcr);
	if (ret != 0)
		npu_drv_err("fail to get ssid, ret = %d\n", ret);

free_resource:
	mutex_unlock(&svm_manager->lock);
	return ret;
}
#else
int npu_get_ssid_bypid(u8 devid, pid_t manager_pid, pid_t svm_pid,
	uint16_t *ssid, uint64_t *ttbr, uint64_t *tcr)
{
	int ret = 0;
	int item = 0;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;
	struct npu_platform_info *plat_info = NULL;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");

	if (devid >= NPU_DEV_NUM || manager_pid <= 0 || svm_pid <= 0 ||
		ssid == NULL || ttbr == NULL || tcr == NULL) {
		npu_drv_err("illegal input para\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "illegal cur_dev_ctx\n");

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "illegal svm_manager\n");

	mutex_lock(&svm_manager->lock);

	item = npu_check_item_bypid(svm_manager, svm_pid);
	cond_goto_error(item == -1, free_resource, ret, -1, "get item failed\n");

	if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
		pid_struct = find_get_pid(svm_pid);
		cond_goto_error(pid_struct == NULL, free_resource, ret, -1, "get pid failed\n");

		task = get_pid_task(pid_struct, PIDTYPE_PID);
		cond_goto_error(task == NULL, free_resource, ret, -1, "get pid task failed\n");

		svm_manager->svm_pid_info[item].mm = get_task_mm(task);
		cond_goto_error(svm_manager->svm_pid_info[item].mm == NULL, free_resource, ret, -1, "get pid mm failed\n");

		svm_manager->svm_pid_info[item].npu_svm =
			(void*)mm_svm_bind_task(plat_info->pdev, task, svm_manager->svm_pid_info[item].ssid);
		if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
			npu_drv_err("mm svm bind task failed\n");
			mmput(svm_manager->svm_pid_info[item].mm);
			svm_manager->svm_pid_info[item].mm = NULL;
			ret = -EBUSY;
			goto free_resource;
		}

#ifdef CONFIG_NPU_MMAP
	/* server proc cannot hold mm, otherwise the following situations may occur:
	   mm has hold npu0 file's refs because user calls npu_map.
	   when server proc exits abnormally, npu_manager is released, and npu0 may have unfinished tasks,
	   then server proc's svm will be released in npu0_release lately.
	   because the svm'mm has hold npu0 file's refs, so npu0 will never be released. */
		if (svm_manager->svm_pid_info[item].svm_pid == svm_manager->svm_pid_info[item].manager_pid) {
			mmput(svm_manager->svm_pid_info[item].mm);
			svm_manager->svm_pid_info[item].mm = NULL;
		}
#endif
	}

	ret = mm_svm_get_ssid(svm_manager->svm_pid_info[item].npu_svm, ssid, ttbr, tcr);
	if (ret != 0)
		npu_drv_err("fail to get ssid, ret = %d\n", ret);

free_resource:
	if (task != NULL)
		put_task_struct(task);
	if (pid_struct != NULL)
		put_pid(pid_struct);
	mutex_unlock(&svm_manager->lock);
	return ret;
}
#endif
// app process:called by CreateProcess(Runtime)
// AicpServer:called by open manager device
// 1. item  should not exist
// 2. insert a item, and set flag
int npu_insert_item_bypid(u8 devid, pid_t manager_pid, pid_t svm_pid)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;
	int i;
	int ret = 0;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;

	if (manager_pid <= 0 || svm_pid <= 0) {
		npu_drv_err("illegal pid: manager pid = %d, svm pid = %d\n", manager_pid, svm_pid);
		return -1;
	}

	cond_return_error(devid >= NPU_DEV_NUM, -1, "illegal npu dev id\n");

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx is null\n");

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "illegal svm_manager\n");

	mutex_lock(&svm_manager->lock);

	i = npu_get_item_bypid(svm_manager, svm_pid);
	if (i < 0) {
		// table is full
		npu_drv_err("svm manager table is full\n");
		ret = -1;
		goto Complete;
	}
	if (svm_manager->svm_pid_info[i].manager_pid == 0) {
#ifndef CONFIG_LDK_SVM
		// create a new item
		pid_struct = find_get_pid(svm_pid);
		if (pid_struct == NULL) {
			npu_drv_err("find get pid failed\n");
			ret = -1;
			goto Complete;
		}
		task = get_pid_task(pid_struct, PIDTYPE_PID);
		if (task == NULL) {
			npu_drv_err("get pid task failed\n");
			ret = -1;
			goto Complete;
		}
		ret = mm_svm_flag_set(task, 1);
#else
		ret = mm_svm_flag_set(svm_pid, 1);
#endif
		if (ret != 0) {
			npu_drv_err("mm svm flag set failed\n");
			goto Complete;
		}
		svm_manager->svm_pid_info[i].manager_pid = manager_pid;
		svm_manager->svm_pid_info[i].svm_pid = svm_pid;
#ifdef CONFIG_FFRT_UNIFORM_ID
		svm_manager->svm_pid_info[i].ssid = ffrt_get_uniform_id(svm_pid);
#else
		svm_manager->svm_pid_info[i].ssid = UINT16_MAX;
#endif
		svm_manager->item_num++;

		npu_drv_warn("insert a new item, manager_pid=%d, svm_pid=%d, item_num=%u",
			manager_pid, svm_pid, svm_manager->item_num);
	} else {
		npu_drv_err("the pid is existed, exe_p_pid=%d, manager_pid=%d, svm_pid=%d\n",
			svm_manager->svm_pid_info[i].manager_pid, manager_pid, svm_pid);
		ret = -1;
		goto Complete;
	}

Complete:
	if (task != NULL)
		put_task_struct(task);
	if (pid_struct != NULL)
		put_pid(pid_struct);

	mutex_unlock(&svm_manager->lock);
	return ret;
}

// for app process
// 1. item should exist
// 2. unbind + unsetflag + delete item
int npu_release_item_bypid(u8 devid, pid_t manager_pid, pid_t svm_pid)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;
	int ret = 0;
	int i;

	if (manager_pid <= 0 || svm_pid <= 0) {
		npu_drv_err("illegal manager_pid=%d, svm_pid=%d\n",
			manager_pid, svm_pid);
		return -1;
	}

	if (devid >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return -1;
	}
	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;

	mutex_lock(&svm_manager->lock);
	i = npu_check_item_bypid(svm_manager, svm_pid);
	if (i < 0 || svm_manager->svm_pid_info[i].manager_pid != manager_pid) {
		npu_drv_err("pid is invalid, the manager_pid=%d, svm_pid=%d \n",
			manager_pid, svm_pid);
		goto Complete;
	}
	// unbind the svm
	if (svm_manager->svm_pid_info[i].npu_svm) {
		mm_svm_unbind_task(svm_manager->svm_pid_info[i].npu_svm);
		svm_manager->svm_pid_info[i].npu_svm = NULL;
#ifndef CONFIG_LDK_SVM
		if (svm_manager->svm_pid_info[i].mm != NULL)
			mmput(svm_manager->svm_pid_info[i].mm);
		svm_manager->svm_pid_info[i].mm = NULL;
#endif
	}

	// delete the item
	svm_manager->svm_pid_info[i].manager_pid = 0;
	svm_manager->svm_pid_info[i].svm_pid = 0;
#ifdef CONFIG_FFRT_UNIFORM_ID
	ffrt_put_uniform_id(svm_manager->svm_pid_info[i].ssid);
#endif
	svm_manager->svm_pid_info[i].ssid = 0;
	svm_manager->item_num--;
	npu_drv_warn("delete a new item, manager_pid=%d, svm_pid=%d, item_num=%u",
		manager_pid, svm_pid, svm_manager->item_num);

	// unsetflag, ai client exit first, then call api rtProcessDestory, so pid is invalid
#ifndef CONFIG_LDK_SVM
	pid_struct = find_get_pid(svm_pid);
	if (pid_struct == NULL) {
		npu_drv_warn("pid_struct is null, pid = %d", svm_pid);
		goto Complete;
	}
	task = get_pid_task(pid_struct, PIDTYPE_PID);
	if (task == NULL) {
		npu_drv_warn("pid task is null, pid = %d", svm_pid);
		goto Pid_out;
	}
	mm_svm_flag_set(task, 0);
	put_task_struct(task);

Pid_out:
	put_pid(pid_struct);
#else
	mm_svm_flag_set(svm_pid, 0);
#endif

Complete:
	mutex_unlock(&svm_manager->lock);
	return ret;
}

// for current and app process
// if pid == 0, do device clear
// if called by powerdown(flag=0):unbind
// if called by close device(flag=1):
// unbind + (unsetflag + delete item)(exept current->tgid)
int npu_clear_pid_ssid_table(u8 devid, pid_t pid, int flag)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;
	pid_t svm_pid;
	int i;

	if (devid >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return -1;
	}
	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;

	mutex_lock(&svm_manager->lock);
	for (i = 0; i < DRVDEV_APP_MAX_NUM; i++) {
		svm_pid = svm_manager->svm_pid_info[i].svm_pid;
		if ((svm_manager->svm_pid_info[i].manager_pid != pid) && (pid != 0))
			continue;
		// 1. unbind
		if (svm_manager->svm_pid_info[i].npu_svm != NULL) {
			mm_svm_unbind_task(svm_manager->svm_pid_info[i].npu_svm);
			npu_drv_warn("unbind item, mnt_pid=%d svm_pid=%d",
				svm_manager->svm_pid_info[i].manager_pid, svm_pid);
			svm_manager->svm_pid_info[i].npu_svm = NULL;
#ifndef CONFIG_LDK_SVM
			if (svm_manager->svm_pid_info[i].mm != NULL)
				mmput(svm_manager->svm_pid_info[i].mm);
			svm_manager->svm_pid_info[i].mm = NULL;
#endif
		}

		if (flag != 0 && svm_pid != pid) {
			svm_manager->svm_pid_info[i].manager_pid = 0;
			svm_manager->svm_pid_info[i].svm_pid = 0;
#ifdef CONFIG_FFRT_UNIFORM_ID
			ffrt_put_uniform_id(svm_manager->svm_pid_info[i].ssid);
#endif
			svm_manager->svm_pid_info[i].ssid = 0;
			svm_manager->item_num--;

			// 2. unset flag + delete item
#ifndef CONFIG_LDK_SVM
			pid_struct = find_get_pid(svm_pid);
			if (pid_struct == NULL) {
				npu_drv_warn("find get pid failed\n");
				continue;
			}
			task = get_pid_task(pid_struct, PIDTYPE_PID);
			if (task == NULL) {
				put_pid(pid_struct);
				npu_drv_warn("get pid task failed\n");
				continue;
			}
			mm_svm_flag_set(task, 0);
			put_task_struct(task);
			put_pid(pid_struct);
#else
			mm_svm_flag_set(svm_pid, 0);
#endif
		}
	}
	mutex_unlock(&svm_manager->lock);
	npu_drv_warn("clear table when %s", flag == 0 ? "powerdown" :
		"close npu device");
	return 0;
}

int npu_svm_manager_destroy(u8 devid)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct t_svm_manager *svm_manager = NULL;

	if (devid >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return -1;
	}

	npu_clear_pid_ssid_table(devid, 0, 1);

	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	mutex_destroy(&svm_manager->lock);
	kfree(svm_manager);
	cur_dev_ctx->svm_manager = NULL;
	return 0;
}
int npu_get_ttbr(u64 *ttbr, u64 *tcr, pid_t process_id)
{
#ifndef CONFIG_LDK_SVM
	unsigned long asid;
	struct mm_struct *mm = NULL;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;

	if ((ttbr == NULL) || (tcr == NULL)) {
		npu_drv_err("ttbr or tcr is null\n");
		return -EINVAL;
	}

	pid_struct = find_get_pid(process_id);
	if (pid_struct == NULL) {
		npu_drv_err("pid_struct is null\n");
		return -ESRCH;
	}

	task = get_pid_task(pid_struct, PIDTYPE_PID);
	if (task == NULL) {
		put_pid(pid_struct);
		npu_drv_err("get pid task failed\n");
		return -ESRCH;
	}

	mm = get_task_mm(task);
	if (mm == NULL) {
		put_task_struct(task);
		put_pid(pid_struct);
		npu_drv_err("get mm is null\n");
		return -ESRCH;
	}

	asid = ASID(mm);
	*ttbr = virt_to_phys(mm->pgd) | (asid << 48);
	*tcr  = read_sysreg(tcr_el1);
	npu_drv_debug("pgdaddr:0x:%pK, context:0x%pK, pa:0x%pK\n",
		mm->pgd, (u64 *)(mm->pgd), (void *)(uintptr_t)virt_to_phys(mm->pgd));

	mmput(mm);
	npu_drv_debug("asid:%lu ,ttbr:0x%pK, tcr:0x%pK\n", asid,
		(void *)(uintptr_t)*ttbr, (void *)(uintptr_t)*tcr);

	put_task_struct(task);
	put_pid(pid_struct);
	return 0;
#else
	npu_drv_warn("no need to get ttbr in LDK");
	return 0;
#endif
}


int npu_check_app_pid(struct npu_dev_ctx *cur_dev_ctx, pid_t process_id)
{
	struct t_svm_manager *svm_manager = NULL;
	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx is null\n");
	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "svm manager is null\n");
	return npu_check_item_bypid(svm_manager, process_id);
}
#ifdef CONFIG_LDK_SVM
int npu_manager_svm_bind(struct npu_dev_ctx *cur_dev_ctx)
{
	struct t_svm_manager *svm_manager = NULL;
	int item = 0;
	int ret = 0;
	struct npu_platform_info *plat_info = NULL;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");

	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx is null\n");
	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "svm manager is null\n");

	mutex_lock(&svm_manager->lock);

	item = npu_get_item_manager_pid(svm_manager);
	cond_goto_error(item == -1, free_resource, ret, -1, "get item failed\n");

	if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
		svm_manager->svm_pid_info[item].npu_svm = (void*)mm_svm_bind_task(plat_info->pdev,
			svm_manager->svm_pid_info[item].svm_pid);
		if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
			npu_drv_err("mm svm bind task failed\n");
			ret = -EBUSY;
		}
	}

free_resource:
	mutex_unlock(&svm_manager->lock);

	return ret;
}
#else
int npu_manager_svm_bind(struct npu_dev_ctx *cur_dev_ctx)
{
	struct t_svm_manager *svm_manager = NULL;
	int item = 0;
	int ret = 0;
	struct pid *pid_struct = NULL;
	struct task_struct *task = NULL;
	struct npu_platform_info *plat_info = NULL;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");

	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx is null\n");
	svm_manager = (struct t_svm_manager *)cur_dev_ctx->svm_manager;
	cond_return_error(svm_manager == NULL, -1, "svm manager is null\n");

	mutex_lock(&svm_manager->lock);

	item = npu_get_item_manager_pid(svm_manager);
	cond_goto_error(item == -1, free_resource, ret, -1, "get item failed\n");

	if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
		pid_struct = find_get_pid(svm_manager->svm_pid_info[item].svm_pid);
		cond_goto_error(pid_struct == NULL, free_resource, ret, -1, "get pid failed\n");

		task = get_pid_task(pid_struct, PIDTYPE_PID);
		cond_goto_error(task == NULL, free_resource, ret, -1, "get pid task failed\n");

		svm_manager->svm_pid_info[item].mm = get_task_mm(task);
		cond_goto_error(svm_manager->svm_pid_info[item].mm == NULL, free_resource, ret, -1, "get pid mm failed\n");
	
		svm_manager->svm_pid_info[item].npu_svm =
			(void*)mm_svm_bind_task(plat_info->pdev, task, svm_manager->svm_pid_info[item].ssid);
		if (svm_manager->svm_pid_info[item].npu_svm == NULL) {
			npu_drv_err("mm svm bind task failed\n");
			mmput(svm_manager->svm_pid_info[item].mm);
			svm_manager->svm_pid_info[item].mm = NULL;
			ret = -EBUSY;
			goto free_resource;
		}

#ifdef CONFIG_NPU_MMAP
	/* server proc cannot hold mm, otherwise the following situations may occur:
	   mm has hold npu0 file's refs because user calls npu_map.
	   when server proc exits abnormally, npu_manager is released, and npu0 may have unfinished tasks,
	   then server proc's svm will be released in npu0_release lately.
	   because the svm'mm has hold npu0 file's refs, so npu0 will never be released. */
		if (svm_manager->svm_pid_info[item].svm_pid == svm_manager->svm_pid_info[item].manager_pid) {
			mmput(svm_manager->svm_pid_info[item].mm);
			svm_manager->svm_pid_info[item].mm = NULL;
		}
#endif
	}

free_resource:
	if (task != NULL)
		put_task_struct(task);
	if (pid_struct != NULL)
		put_pid(pid_struct);

	mutex_unlock(&svm_manager->lock);

	return ret;
}
#endif
