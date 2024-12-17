/* Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * FileName: spi_nor_agent.h
 * Description: add spi nor agent in kernel.
 * Author: hisi
 * Create: 2023-11-23
 */

#ifndef __SPI_NOR_H__
#define __SPI_NOR_H__

#include <linux/types.h>

#define SPI_NOR_WRITE 0
#define SPI_NOR_READ 1

struct opt_nor_flash_user {
	uint32_t operation;
	char part_name[36];
	uint32_t offset;
	uint32_t size;
	void *data;
};
#define SPI_NOR_IOCTL_NUM 27
#define NORFLASHACCESSDATA _IOWR('N', SPI_NOR_IOCTL_NUM, struct opt_nor_flash_user)

#ifdef CONFIG_SPI_NOR_AGENT
int spi_nor_read(const char *ptn_name, u64 offset, void *data, u64 size);
int spi_nor_write(const char *ptn_name, u64 offset, void *data, u64 size);
#else
static inline int spi_nor_read(const char *ptn_name, u64 offset, void *data, u64 size)
{ return 0; }
static inline int spi_nor_write(const char *ptn_name, u64 offset, void *data, u64 size)
{ return 0; }
#endif

#endif
