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
	return (uint64_t)__get_free_page(GFP_KERNEL | __GFP_ZERO);
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

	free_page(cmd_vaddr);
}

uint64_t get_spi_mem_vaddr(void)
{
	return (uint64_t)__get_free_page(GFP_KERNEL | __GFP_ZERO);
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

	free_page(spi_vaddr);
}
