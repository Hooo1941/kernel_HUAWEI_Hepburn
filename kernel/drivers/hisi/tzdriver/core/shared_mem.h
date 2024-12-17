/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Api for shared mem
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

#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <linux/types.h>
#include <linux/of.h>

#ifdef CONFIG_512K_LOG_PAGES_MEM
#define PAGES_LOG_MEM_LEN   (512 * SZ_1K) /* mem size: 512 k */
#else
#define PAGES_LOG_MEM_LEN   (256 * SZ_1K) /* mem size: 256 k */
#endif

#ifndef CONFIG_SHARED_MEM_RESERVED
typedef struct page mailbox_page_t;
#else
typedef uintptr_t mailbox_page_t;
#endif

uint64_t get_reserved_cmd_vaddr_of(phys_addr_t cmd_phys, uint64_t cmd_size);
int load_tz_shared_mem(struct device_node *np);

mailbox_page_t *mailbox_alloc_pages(int order);
void mailbox_free_pages(mailbox_page_t *pages, int order);
uintptr_t mailbox_page_address(mailbox_page_t *page);
mailbox_page_t *mailbox_virt_to_page(uint64_t ptr);
uint64_t get_operation_vaddr(void);
void free_operation(uint64_t op_vaddr);

uint64_t get_log_mem_vaddr(void);
uint64_t get_log_mem_paddr(uint64_t log_vaddr);
uint64_t get_log_mem_size(void);
void free_log_mem(uint64_t log_vaddr);

uint64_t get_cmd_mem_vaddr(void);
uint64_t get_cmd_mem_paddr(uint64_t cmd_vaddr);
void free_cmd_mem(uint64_t cmd_vaddr);

uint64_t get_spi_mem_vaddr(void);
uint64_t get_spi_mem_paddr(uintptr_t spi_vaddr);
void free_spi_mem(uint64_t spi_vaddr);
#endif
