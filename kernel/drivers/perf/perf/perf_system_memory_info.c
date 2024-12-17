
#include <linux/sysinfo.h>
#include <linux/mmzone.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/vmalloc.h>
#include <linux/dma-buf.h>
#include <platform_include/basicplatform/linux/ion/mm_ion.h>
#include <platform/trace/hooks/memcheck.h>
#include <../drivers/staging/android/ashmem.h>

#include "utils/perf_buffered_log_sender.h"
#include "perf_ioctl.h"
#include "utils/perf_utils.h"
#include "perf_system_memory_info.h"

struct perf_system_memory_info {
	int mem_total;
	int mem_free;
	int mem_available;
	int buffers;
	int cached;
	int swap_cached;
	int active;
	int inactive;
	int active_anon;
	int inactive_anon;
	int active_file;
	int inactive_file;
	int slab;
	int sreclaimable;
	int sunreclaim;
	int gpu_total_used;
	int ash_mem;
	int ion_total_used;
	int ion_total_cached;
	int vmalloc_used;
	int swap_free;
};

static void perf_sys_meminfo_get_data(
	struct perf_system_memory_info *sys_mem_info)
{
	struct sysinfo i;
	long cached;
	long available;
	unsigned long pages[NR_LRU_LISTS];
	unsigned long sreclaimable, sunreclaim;
	unsigned long gpu_total;
	int lru;

	si_meminfo(&i);
	si_swapinfo(&i);

	cached = global_node_page_state(NR_FILE_PAGES) -
			total_swapcache_pages() - i.bufferram;
	if (cached < 0)
		cached = 0;

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_node_page_state(NR_LRU_BASE + lru);

	available = si_mem_available();
	sreclaimable = global_node_page_state_pages(NR_SLAB_RECLAIMABLE_B);
	sunreclaim = global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B);

	/* data for KB */
	sys_mem_info->mem_total = (i.totalram) << (PAGE_SHIFT - 10);
	sys_mem_info->mem_free = (i.freeram) << (PAGE_SHIFT - 10);
	sys_mem_info->mem_available = (available) << (PAGE_SHIFT - 10);
	sys_mem_info->buffers = (i.bufferram) << (PAGE_SHIFT - 10);
	sys_mem_info->cached = (cached) << (PAGE_SHIFT - 10);
	sys_mem_info->swap_cached = (total_swapcache_pages()) << (PAGE_SHIFT - 10);
	sys_mem_info->active = (pages[LRU_ACTIVE_ANON] + pages[LRU_ACTIVE_FILE]) <<
		(PAGE_SHIFT - 10);
	sys_mem_info->inactive = (pages[LRU_INACTIVE_ANON] +
		pages[LRU_INACTIVE_FILE]) << (PAGE_SHIFT - 10);
	sys_mem_info->active_anon = (pages[LRU_ACTIVE_ANON]) << (PAGE_SHIFT - 10);
	sys_mem_info->inactive_anon = (pages[LRU_INACTIVE_ANON]) <<
		(PAGE_SHIFT - 10);
	sys_mem_info->active_file = (pages[LRU_ACTIVE_FILE]) << (PAGE_SHIFT - 10);
	sys_mem_info->inactive_file = (pages[LRU_INACTIVE_FILE]) <<
		(PAGE_SHIFT - 10);
	sys_mem_info->slab = (sreclaimable + sunreclaim) << (PAGE_SHIFT - 10);
	sys_mem_info->sreclaimable = (sreclaimable) << (PAGE_SHIFT - 10);
	sys_mem_info->sunreclaim = (sunreclaim) << (PAGE_SHIFT - 10);

	trace_hck_gpu_get_total_used(&gpu_total);
	sys_mem_info->gpu_total_used = (gpu_total) << (PAGE_SHIFT - 10);

#ifdef CONFIG_DFX_MEMCHECK
	sys_mem_info->ash_mem = (ashmem_get_total_size() / PAGE_SIZE) <<
		(PAGE_SHIFT - 10);
#endif
	sys_mem_info->ion_total_used = mm_ion_total() >> PAGE_SHIFT;
	sys_mem_info->ion_total_cached = global_zone_page_state(NR_IONCACHE_PAGES);
	sys_mem_info->vmalloc_used = (vmalloc_nr_pages()) << (PAGE_SHIFT - 10);
	sys_mem_info->swap_free = (i.freeswap) << (PAGE_SHIFT - 10);
}

long perf_ioctl_system_memory_info(unsigned int cmd, void __user *argp)
{
	int ret = -EINVAL;
	unsigned int entry_size;
	struct perf_buffered_log_entry *entry = NULL;

	entry_size = sizeof(struct perf_buffered_log_entry) +
		sizeof(struct perf_system_memory_info);

	entry = perf_create_log_entry(entry_size, IOC_SYSTEM_MEM_INFO, 1);
	if (entry == NULL) {
		pr_err("PerfD %s: failed to create log entry\n", __func__);
		goto error;
	}

	perf_sys_meminfo_get_data(
		(struct perf_system_memory_info *)(&entry->data));

	ret = send_perf_buffered_log_entry(entry);
	if (ret < 0) {
		pr_err("PerfD %s: failed to send log entry ret %d\n", __func__, ret);
		goto error;
	}

error:
	free_perf_buffered_log_entry(entry);
	return ret;
}