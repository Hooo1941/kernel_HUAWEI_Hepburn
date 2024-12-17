/*
 * memcheck_ioctl.c
 *
 * implement the ioctl for user space to get memory usage information,
 * and also provider control command
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#endif
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
#include <linux/signal_types.h>
#include <linux/pagewalk.h>
#endif
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
#include <linux/mmap_lock.h>
#endif
#include "memcheck_ioctl.h"
#include "memcheck_stack.h"
#include "memcheck_detail.h"

#define NATIVE_LIB_MALLOC	"libc_malloc"
#define NATIVE_LIB_MALLOC_LEN	11
#define NATIVE_SCUDO	"scudo:"
#define NATIVE_SCUDO_LEN	6
#define NATIVE_GWP_ASAN	"GWP_ASan"
#define NATIVE_GWP_ASAN_LEN	8
#ifdef CONFIG_DFX_OHOS
#define NATIVE_HEAP	"native_heap:"
#define NATIVE_HEAP_LEN 12
#else
#define JAVA_TAG	"dalvik-"
#define JAVA_TAG_LEN	7
#define JAVA_TAG2	"maple_alloc_ros"
#define JAVA_TAG2_LEN	15
#endif

struct memsize_stats {
	u64 pss;
	u64 swap;
	u64 java_pss;
	u64 native_pss;
};

enum heap_type {
	HEAP_OTHER,
	HEAP_JAVA,
	HEAP_NATIVE,
};

static const char *const java_tag[] = {
#ifdef CONFIG_DFX_OHOS
	"ArkTS",
#else
	"dalvik-alloc space",
	"dalvik-main space",
	"dalvik-large object space",
	"dalvik-free list large object space",
	"dalvik-non moving space",
	"dalvik-zygote space",
#endif
};

static bool is_java_heap(const char *tag)
{
	int i;
	char *tmp = NULL;

#ifndef CONFIG_DFX_OHOS
	if (strncmp(tag, JAVA_TAG2, JAVA_TAG2_LEN) == 0)
		return true;
#endif

	for (i = 0; i < ARRAY_SIZE(java_tag); i++) {
		tmp = strstr(tag, java_tag[i]);
		if (tmp == tag)
			return true;
	}

	return false;
}

static bool is_native_heap(const char *tag)
{
#ifdef CONFIG_DFX_OHOS
	if (strncmp(tag, NATIVE_HEAP, NATIVE_HEAP_LEN) == 0)
		return true;
#endif
	if (strncmp(tag, NATIVE_LIB_MALLOC, NATIVE_LIB_MALLOC_LEN) == 0 ||
	    strncmp(tag, NATIVE_SCUDO, NATIVE_SCUDO_LEN) == 0 ||
	    strncmp(tag, NATIVE_GWP_ASAN, NATIVE_GWP_ASAN_LEN) == 0)
		return true;
	return false;
}

static enum heap_type memcheck_get_heap_type(const char *name)
{
	enum heap_type type = HEAP_OTHER;

	if (!name)
		return type;

	if (is_native_heap(name))
		type = HEAP_NATIVE;
	else if (is_java_heap(name))
		type = HEAP_JAVA;
	return type;
}

static struct page **alloc_page_pointers(size_t num)
{
	struct page **page = NULL;
	size_t page_len = sizeof(**page) * num;

	page = kzalloc(page_len, GFP_KERNEL);
	if (!page)
		return ERR_PTR(-ENOMEM);

	return page;
}

static size_t do_strncpy_from_remote_string(char *dst, long page_offset,
					    struct page **page, long num_pin,
					    long count)
{
	long i;
	size_t sz;
	size_t strsz;
	size_t copy_sum = 0;
	long page_left = min((long)PAGE_SIZE - page_offset, count);
	const char *p = NULL;
	const char *kaddr = NULL;

	count = min(count, num_pin * (long)PAGE_SIZE - page_offset);

	for (i = 0; i < num_pin; i++) {
		kaddr = (const char *)kmap(page[i]);
		if (!kaddr)
			break;

		if (i == 0) {
			p = kaddr + page_offset;
			sz = page_left;
		} else {
			p = kaddr;
			sz = min((long)PAGE_SIZE, count - page_left -
				 (i - 1) * (long)PAGE_SIZE);
		}

		strsz = strnlen(p, sz);
		memcpy(dst, p, strsz);

		kunmap(page[i]);

		dst += strsz;
		copy_sum += strsz;

		if (strsz != sz)
			break;
	}

	for (i = 0; i < num_pin; i++)
		put_page(page[i]);

	return copy_sum;
}

#ifndef CONFIG_DFX_OHOS
static long strncpy_from_remote_user(char *dst, struct mm_struct *remote_mm,
				     const char __user *src, long count)
{
	long num_pin;
	size_t copy_sum;
	struct page **page = NULL;

	uintptr_t src_page_start = (uintptr_t)src & PAGE_MASK;
	uintptr_t src_page_offset = (uintptr_t)(src - src_page_start);
	size_t num_pages = DIV_ROUND_UP(src_page_offset + count,
					(long)PAGE_SIZE);

	page = alloc_page_pointers(num_pages);
	if (IS_ERR_OR_NULL(page))
		return PTR_ERR(page);

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	num_pin = get_user_pages_remote(remote_mm, src_page_start,
					num_pages, 0, page, NULL, NULL);
#elif (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	num_pin = get_user_pages_remote(current, remote_mm,
					src_page_start, num_pages, 0,
					page, NULL, NULL);
#else
	num_pin = get_user_pages_remote(current, remote_mm,
					src_page_start, num_pages, 0,
					page, NULL);
#endif
	if (num_pin < 1) {
		kfree(page);
		return 0;
	}

	copy_sum = do_strncpy_from_remote_string(dst, src_page_offset, page,
						 num_pin, count);
	kfree(page);

	return copy_sum;
}

enum heap_type memcheck_anon_vma_name(struct vm_area_struct *vma)
{
	const char __user *name_user = vma_get_anon_name(vma);
	unsigned long max_len = min((unsigned long)NAME_MAX + 1,
				    (unsigned long)PAGE_SIZE);
	char *out_name = NULL;
	enum heap_type type = HEAP_OTHER;
	long retcpy;

	out_name = kzalloc(max_len, GFP_KERNEL);
	if (!out_name)
		return type;

	retcpy = strncpy_from_remote_user(out_name, vma->vm_mm,
				  name_user, max_len);
	if (retcpy <= 0)
		goto free_name;

	type = memcheck_get_heap_type(out_name);

free_name:
	kfree(out_name);

	return type;
}
#endif

enum heap_type memcheck_get_type(struct vm_area_struct *vma)
{
	char *name = NULL;
	struct mm_struct *mm = vma->vm_mm;
	enum heap_type type = HEAP_OTHER;

	/* file map is never heap in Android Q */
	if (vma->vm_file)
		return type;

	/* get rid of stack */
	if ((vma->vm_start <= vma->vm_mm->start_stack) &&
	    (vma->vm_end >= vma->vm_mm->start_stack))
		return type;

	if ((vma->vm_ops) && (vma->vm_ops->name)) {
		name = (char *)vma->vm_ops->name(vma);
		if (name)
			goto got_name;
	}

	name = (char *)arch_vma_name(vma);
	if (name)
		goto got_name;

	/* get rid of vdso */
	if (!mm)
		return type;

	/* main thread native heap */
	if ((vma->vm_start <= mm->brk) && (vma->vm_end >= mm->start_brk))
		return HEAP_NATIVE;

#ifdef CONFIG_DFX_OHOS
	name = (char *)vma_get_anon_name(vma);
#else
	if (vma_get_anon_name(vma))
		return memcheck_anon_vma_name(vma);
#endif

got_name:
	return memcheck_get_heap_type(name);
}

static void memcheck_accum_mss(struct vm_area_struct *vma,
			       struct memsize_stats *mss_total,
			       u64 pss, u64 swap)
{
	enum heap_type type;
	u64 pss_all = pss + swap;

	mss_total->pss += pss_all;
	mss_total->swap += swap;
	type = memcheck_get_type(vma);
	if (type == HEAP_JAVA)
		mss_total->java_pss += pss_all;
	else if (type == HEAP_NATIVE)
		mss_total->native_pss += pss_all;
}

static inline int memcheck_is_contended(struct mm_struct *mm)
{
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	return rwsem_is_contended(&mm->mmap_lock);
#else
	return rwsem_is_contended(&mm->mmap_sem);
#endif
}

static inline int memcheck_read_lock(struct mm_struct *mm)
{
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	return mmap_read_lock_killable(mm);
#else
	return down_read_killable(&mm->mmap_sem);
#endif
}

static inline void memcheck_read_unlock(struct mm_struct *mm)
{
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	up_read(&mm->mmap_lock);
#else
	up_read(&mm->mmap_sem);
#endif
}

static int memcheck_get_mss(pid_t pid, struct memsize_stats *mss_total)
{
	int ret = -EINVAL;
	unsigned long last_vma_end = 0;
	struct task_struct *tsk = NULL;
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma = NULL;

	rcu_read_lock();
	tsk = find_task_by_vpid(pid);
	if (tsk)
		get_task_struct(tsk);
	rcu_read_unlock();
	if (!tsk)
		return ret;
	mm = get_task_mm(tsk);
	if (!mm)
		goto err_put_task;

	memset(mss_total, 0, sizeof(*mss_total));

	ret =  memcheck_read_lock(mm);
	if (ret)
		goto err_put_mm;

	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		u64 pss;
		u64 swap = 0;

		pss = smaps_get_pss(vma, &swap);
		memcheck_accum_mss(vma, mss_total, pss, swap);
		last_vma_end = vma->vm_end;
		if (memcheck_is_contended(mm)) {
			memcheck_read_unlock(mm);
			ret = memcheck_read_lock(mm);
			if (ret)
				goto err_put_mm;
			vma = find_vma(mm, last_vma_end - 1);
			if (!vma)
				break;
			if (vma->vm_start >= last_vma_end)
				continue;
			if (vma->vm_end > last_vma_end) {
				swap = 0;
				pss = smaps_get_pss(vma, &swap);
				memcheck_accum_mss(vma, mss_total, pss, swap);
			}
		}
	}
	memcheck_read_unlock(mm);
	ret = 0;

err_put_mm:
	mmput(mm);
err_put_task:
	put_task_struct(tsk);

	return ret;
}

unsigned short memcheck_get_memstat(struct memstat_pss *p)
{
	int ret;
	struct memsize_stats mss_total;
	unsigned short result = 0;

	memset(&mss_total, 0, sizeof(mss_total));

	/* read the smaps */
	ret = memcheck_get_mss(p->id, &mss_total);
	if (ret)
		return result;

	p->pss = mss_total.pss;
	p->swap = mss_total.swap;
	if (p->type & MTYPE_JAVA) {
		p->java_pss = mss_total.java_pss;
		result = result | MTYPE_JAVA;
	}
	if (p->type & MTYPE_NATIVE) {
		p->native_pss = mss_total.native_pss;
		result = result | MTYPE_NATIVE;
	}

	return result;
}

static int process_pss_read(void *arg)
{
	struct memstat_pss memstat;
	unsigned short result;

	if (copy_from_user(&memstat, arg, sizeof(memstat))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (memstat.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad magic number\n");
		return -EINVAL;
	}
	if (!(memstat.type & MTYPE_PSS)) {
		memcheck_err("invalid memtype %d\n", memstat.type);
		return -EINVAL;
	}
	if (memstat.id <= 0) {
		memcheck_err("invalid pid %d\n", memstat.id);
		return -EINVAL;
	}

	result = memcheck_get_memstat(&memstat);
	if (result < 0 || (result & MTYPE_PSS) == MTYPE_NONE) {
		memcheck_err("get memstat infor failed\n");
		return -EFAULT;
	}
	memstat.type = result;

	if (copy_to_user((void *)arg, &memstat, sizeof(memstat))) {
		memcheck_err("copy_to_user failed\n");
		return -EFAULT;
	}

	return 0;
}

static int process_do_command(const void *arg)
{
	struct track_cmd cmd;

	if (copy_from_user(&cmd, arg, sizeof(cmd))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (cmd.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad magic number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL COMMAND id=%u,type=%u,timestamp=%llu,cmd=%d\n",
		      cmd.id, cmd.type, cmd.timestamp, cmd.cmd);
	if (!(cmd.type & MTYPE_PSS)) {
		memcheck_err("invalid memtype %d\n", cmd.type);
		return -EINVAL;
	}
	if (cmd.id <= 0) {
		memcheck_err("invalid pid %d\n", cmd.id);
		return -EINVAL;
	}
	if ((cmd.cmd <= MEMCMD_NONE) || (cmd.cmd >= MEMCMD_MAX)) {
		memcheck_err("invalid cmd %d\n", cmd.cmd);
		return -EINVAL;
	}
	if ((cmd.cmd == MEMCMD_ENABLE) && !cmd.timestamp) {
		memcheck_err("invalid timestamp for MEMCMD_ENABLE\n");
		return -EINVAL;
	}

	return memcheck_do_command(&cmd);
}

static int process_stack_save(const void *arg)
{
	struct stack_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad magic number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL STACK SAVE type=%u,size=%llu\n", info.type,
		      info.size);
	if ((!info.size) || (info.size > MEMCHECK_STACKINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}

	return memcheck_stack_write(arg, &info);
}

static int process_stack_read(void *arg)
{
	struct stack_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad magic number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL STACK READ type=%u,size=%llu\n", info.type,
		      info.size);
	if ((!info.size) || (info.size > MEMCHECK_STACKINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}
	if (!(info.type & MTYPE_PSS)) {
		memcheck_err("invalid memtype %d\n", info.type);
		return -EINVAL;
	}
	return memcheck_stack_read(arg, &info);
}

static int process_detail_read(void *arg)
{
	struct detail_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad magic number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL DETAIL type=%u,size=%llu\n", info.type, info.size);
	if ((!info.size) || (info.size > MEMCHECK_DETAILINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}

	if (!(info.type & MTYPE_PSS)) {
		memcheck_err("do not support type %d\n", info.type);
		return -EINVAL;
	}

	return memcheck_detail_read(arg, &info);
}

static int process_detail_write(const void *arg)
{
	struct detail_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad magic number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL NATIVE WRITE type=%u,size=%llu\n", info.type,
		      info.size);
	if ((!info.size) || (info.size > MEMCHECK_DETAILINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}

	return memcheck_detail_write(arg, &info);
}

long memcheck_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = -EINVAL;

	if ((cmd < LOGGER_MEMCHECK_MIN) || (cmd > LOGGER_MEMCHECK_MAX))
		return MEMCHECK_CMD_INVALID;
	if (!arg)
		return -EINVAL;

	switch (cmd) {
	case LOGGER_MEMCHECK_PSS_READ:
		{
			unsigned long old_ns = ktime_get();

			ret = process_pss_read((void *)arg);
			memcheck_info("read pss take %ld ns",
				      ktime_get() - old_ns);
		}
		break;

	case LOGGER_MEMCHECK_COMMAND:
		ret = process_do_command((void *)arg);
		break;

	case LOGGER_MEMCHECK_STACK_READ:
		ret = process_stack_read((void *)arg);
		break;

	case LOGGER_MEMCHECK_STACK_SAVE:
		ret = process_stack_save((void *)arg);
		break;

	case LOGGER_MEMCHECK_DETAIL_READ:
		ret = process_detail_read((void *)arg);
		break;

	case LOGGER_MEMCHECK_DETAIL_WRITE:
		ret = process_detail_write((void *)arg);
		break;

	default:
		break;
	}

	return ret;
}
EXPORT_SYMBOL(memcheck_ioctl);
