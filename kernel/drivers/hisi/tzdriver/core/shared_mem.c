/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "shared_mem.h"
#include <securec.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <asm/io.h>
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "smc_smp.h"
#include "internal_functions.h"
#include "mailbox_mempool.h"
#include "ko_adapt.h"

uint64_t get_reserved_cmd_vaddr_of(phys_addr_t cmd_phys, uint64_t cmd_size)
{
	if(cmd_phys == 0 || cmd_size == 0) {
		tloge("cmd phy or cmd size is error\n");
		return 0;
	}
	uint64_t cmd_vaddr = (uint64_t)(uintptr_t)ioremap_cache(cmd_phys, cmd_size);
	if (cmd_vaddr == 0) {
		tloge("io remap for reserved cmd buffer failed\n");
		return 0;
	}
	(void)memset_s((void *)(uintptr_t)cmd_vaddr, cmd_size, 0, cmd_size);
	return cmd_vaddr;
}

#ifdef CONFIG_SHARED_MEM_RESERVED

#define CMD_MEM_MIN_SIZE 0x1000
#define SPI_MEM_MIN_SIZE 0x1000
#define OPERATION_MEM_MIN_SIZE 0x1000
uint64_t g_cmd_mem_paddr;
uint64_t g_cmd_mem_size;
uint64_t g_mailbox_paddr;
uint64_t g_mailbox_size;
uint64_t g_log_mem_paddr;
uint64_t g_log_mem_size;
uint64_t g_spi_mem_paddr;
uint64_t g_spi_mem_size;
static mailbox_page_t *g_mailbox_page;
static uintptr_t g_shmem_start_virt;
static uintptr_t g_page_offset;

int load_tz_shared_mem(struct device_node *np)
{
	int rc;

	rc = of_property_read_u64(np, "tz_shmem_cmd_addr", &g_cmd_mem_paddr);
	if (rc != 0) {
		tloge("read tz_shmem_cmd_addr failed\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_cmd_size", &g_cmd_mem_size);
	if (rc != 0 || g_cmd_mem_size < CMD_MEM_MIN_SIZE) {
		tloge("read tz_shmem_cmd_size failed or size too short\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_mailbox_addr", &g_mailbox_paddr);
	if (rc != 0) {
		tloge("read tz_shmem_mailbox_addr failed\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_mailbox_size", &g_mailbox_size);
	if (rc != 0 || g_mailbox_size < MAILBOX_POOL_SIZE + OPERATION_MEM_MIN_SIZE) {
		tloge("read tz_shmem_mailbox_size failed or size too short\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_spi_addr", &g_spi_mem_paddr);
	if (rc != 0) {
		tloge("read tz_shmem_spi_addr failed\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_spi_size", &g_spi_mem_size);
	if (rc != 0 || g_spi_mem_size < SPI_MEM_MIN_SIZE) {
		tloge("read tz_shmem_spi_size failed or size too short\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_log_addr", &g_log_mem_paddr);
	if (rc != 0) {
		tloge("read tz_shmem_log_addr failed\n");
		return -ENODEV;
	}

	rc = of_property_read_u64(np, "tz_shmem_log_size", &g_log_mem_size);
	if (rc != 0 || g_log_mem_size < PAGES_LOG_MEM_LEN) {
		tloge("read tz_shmem_log_size failed or size too short\n");
		return -ENODEV;
	}

	return 0;
}

mailbox_page_t *mailbox_alloc_pages(int order)
{
	uint32_t i;
	uint32_t page_num = 1 << (unsigned int)order;
	uint32_t page_size = page_num * sizeof(mailbox_page_t);

	g_page_offset = MAILBOX_POOL_SIZE / page_num;
	g_mailbox_page = kmalloc(page_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)g_mailbox_page)) {
		tloge("Failed to allocate mailbox page\n");
		return NULL;
	}

	g_shmem_start_virt = (uintptr_t)ioremap_cache(g_mailbox_paddr, g_mailbox_size);
	if (g_shmem_start_virt == 0) {
		tloge("io remap for mailbox page failed\n");
		kfree(g_mailbox_page);
		g_mailbox_page = NULL;
		return NULL;
	}
	(void)memset_s((void *)g_shmem_start_virt, g_mailbox_size, 0, g_mailbox_size);
	g_mailbox_page[0] = (mailbox_page_t)g_shmem_start_virt;
	for (i = 1; i < page_num; i++)
		g_mailbox_page[i] = g_mailbox_page[i - 1] + g_page_offset;

	return g_mailbox_page;
}

void mailbox_free_pages(mailbox_page_t *pages, int order)
{
	if (!pages || pages != g_mailbox_page)
		return;

	(void)order;
	kfree(pages);
	g_mailbox_page = NULL;
}

uintptr_t mailbox_page_address(mailbox_page_t *page)
{
	if (!page)
		return 0;

	return *page;
}

uintptr_t mailbox_virt_to_phys(uintptr_t addr)
{
	if (addr < g_shmem_start_virt || addr > g_shmem_start_virt + g_mailbox_size)
		return 0;

	return g_mailbox_paddr + (addr - g_shmem_start_virt);
}

mailbox_page_t *mailbox_virt_to_page(uint64_t ptr)
{
	if (ptr < g_shmem_start_virt || ptr > g_shmem_start_virt + g_mailbox_size)
		return 0;

	return &g_mailbox_page[(ptr - g_shmem_start_virt) / g_page_offset];
}

uint64_t get_operation_vaddr(void)
{
	return g_shmem_start_virt + MAILBOX_POOL_SIZE;
}

void free_operation(uint64_t op_vaddr)
{
	(void)op_vaddr;
}

/*
 * This function only for wireless platform, CONFIG_LOG_POOL
 * marco controls the log retention of soft reset feature.
 * Enable CONFIG_LOG_POOL macro, this function won't memset
 * log pool memory, and the old log before reset can be retention.
 */
uint64_t get_log_mem_vaddr(void)
{
	uint64_t log_vaddr = (uint64_t)(uintptr_t)ioremap_cache(g_log_mem_paddr, g_log_mem_size);
	if (log_vaddr == 0) {
		tloge("ioremap for log buffer failed\n");
		return 0;
	}
#ifndef CONFIG_LOG_POOL
	(void)memset_s((void *)(uintptr_t)log_vaddr, g_log_mem_size, 0, g_log_mem_size);
#endif

	return log_vaddr;
}

uint64_t get_log_mem_paddr(uint64_t log_vaddr)
{
	(void)log_vaddr;
	return g_log_mem_paddr;
}

uint64_t get_log_mem_size(void)
{
	return g_log_mem_size;
}

void free_log_mem(uint64_t log_vaddr)
{
	iounmap((void __iomem*)(uintptr_t)log_vaddr);
}

uint64_t get_cmd_mem_vaddr(void)
{
	return get_reserved_cmd_vaddr_of(g_cmd_mem_paddr, g_cmd_mem_size);
}

uint64_t get_cmd_mem_paddr(uint64_t cmd_vaddr)
{
	(void)cmd_vaddr;
	return g_cmd_mem_paddr;
}

void free_cmd_mem(uint64_t cmd_vaddr)
{
	iounmap((void __iomem*)(uintptr_t)cmd_vaddr);
}

uint64_t get_spi_mem_vaddr(void)
{
	uint64_t spi_vaddr = (uint64_t)(uintptr_t)ioremap_cache(g_spi_mem_paddr, g_spi_mem_size);
	if (spi_vaddr == 0) {
		tloge("io remap for spi buffer failed\n");
		return 0;
	}
	(void)memset_s((void *)(uintptr_t)spi_vaddr, g_spi_mem_size, 0, g_spi_mem_size);
	return spi_vaddr;
}

uint64_t get_spi_mem_paddr(uintptr_t spi_vaddr)
{
	(void)spi_vaddr;
	return g_spi_mem_paddr;
}

void free_spi_mem(uint64_t spi_vaddr)
{
	iounmap((void __iomem*)(uintptr_t)spi_vaddr);
}

#else

int load_tz_shared_mem(struct device_node *np)
{
	(void)np;
	return 0;
}

mailbox_page_t *mailbox_alloc_pages(int order)
{
	return koadpt_alloc_pages(GFP_KERNEL, order);
}

void mailbox_free_pages(mailbox_page_t *pages, int order)
{
	if (!pages)
		return;

	__free_pages(pages, order);
}

uintptr_t mailbox_page_address(mailbox_page_t *page)
{
	if (!page)
		return 0;

	return page_address(page);
}

uintptr_t mailbox_virt_to_phys(uintptr_t addr)
{
	if (!addr)
		return 0;

	return virt_to_phys(addr);
}

mailbox_page_t *mailbox_virt_to_page(uint64_t ptr)
{
	if (!ptr)
		return NULL;

	return virt_to_page(ptr);
}

uint64_t get_operation_vaddr(void)
{
	return kzalloc(sizeof(struct tc_ns_operation), GFP_KERNEL);
}

void free_operation(uint64_t op_vaddr)
{
	if (!op_vaddr)
		return;

	kfree(op_vaddr);
}

uint64_t get_log_mem_vaddr(void)
{
	return __get_free_pages(GFP_KERNEL | __GFP_ZERO, get_order(PAGES_LOG_MEM_LEN));
}

uint64_t get_log_mem_paddr(uint64_t log_vaddr)
{
	if (!log_vaddr)
		return 0;

	return virt_to_phys((void *)(uintptr_t)log_vaddr);
}

uint64_t get_log_mem_size(void)
{
	return 0;
}

void free_log_mem(uint64_t log_vaddr)
{
	if (!log_vaddr)
		return;

	free_pages(log_vaddr, get_order(PAGES_LOG_MEM_LEN));
}

#define PAGES_BIG_SESSION_CMD_LEN 6
uint64_t get_cmd_mem_vaddr(void)
{
#ifdef CONFIG_BIG_SESSION
	/* we should map at least 64 pages for 1000 sessions, 2^6 > 40 */
	return (uint64_t)__get_free_pages(GFP_KERNEL | __GFP_ZERO, PAGES_BIG_SESSION_CMD_LEN);
#else
	return (uint64_t)__get_free_page(GFP_KERNEL | __GFP_ZERO);
#endif
}

uint64_t get_cmd_mem_paddr(uint64_t cmd_vaddr)
{
	if (!cmd_vaddr)
		return 0;

	return virt_to_phys((void *)(uintptr_t)cmd_vaddr);
}

void free_cmd_mem(uint64_t cmd_vaddr)
{
	if (!cmd_vaddr)
		return;

#ifdef CONFIG_BIG_SESSION
	free_pages(cmd_vaddr, PAGES_BIG_SESSION_CMD_LEN);
#else
	free_page(cmd_vaddr);
#endif
}

uint64_t get_spi_mem_vaddr(void)
{
#ifdef CONFIG_BIG_SESSION
	/* we should map at least 3 pages for 100 sessions, 2^2 > 3 */
	return (uint64_t)__get_free_pages(GFP_KERNEL | __GFP_ZERO, CONFIG_NOTIFY_PAGE_ORDER);
#else
	return (uint64_t)__get_free_page(GFP_KERNEL | __GFP_ZERO);
#endif
}

uint64_t get_spi_mem_paddr(uintptr_t spi_vaddr)
{
	if (spi_vaddr == 0)
		return 0;

	return virt_to_phys((void *)spi_vaddr);
}

void free_spi_mem(uint64_t spi_vaddr)
{
	if (!spi_vaddr)
		return;

#ifdef CONFIG_BIG_SESSION
	free_pages(spi_vaddr, CONFIG_NOTIFY_PAGE_ORDER);
#else
	free_page(spi_vaddr);
#endif
}
#endif
