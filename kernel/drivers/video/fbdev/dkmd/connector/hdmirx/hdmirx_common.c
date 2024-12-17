/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include <hdmirx_common.h>

void hdmirx_ctrl_irq_clear(char __iomem *addr)
{
	uint32_t clear_irq;

	clear_irq = inp32(addr);
	hdmirx_set_reg(addr, clear_irq, 32, 0);
}

void hdmirx_set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask, temp;

	mask = GENMASK(bs + bw - 1, bs);
	temp = readl(addr);
	temp &= ~mask;

	/**
	 * @brief if you want to check config value, please uncomment following code
	 * dpu_pr_debug("addr:%#x value:%#x ", addr, temp | ((val << bs) & mask));
	 */
	writel(temp | ((val << bs) & mask), addr);
}

void hdmirx_reg_read_block(const char __iomem *addr, uint32_t offset, uint32_t *dst, uint32_t num)
{
	uint32_t cur_offset = offset;
	uint32_t cnt = 0;

	if ((dst == NULL) || (addr == NULL)) {
		return;
	}

	while (cnt < num) {
		*dst = inp32(addr + cur_offset + sizeof(uint32_t) * cnt);
		dst++;
		cnt++;
	}
}

static uint32_t get_right_high(uint32_t mask)
{
	uint32_t index;

	for (index = 0; index < 32; index++) { /* 32: check front 32 data */
		if ((mask & 0x01) == 0x01) {
			break;
		} else {
			mask >>= 1;
		}
	}
	return index;
}

uint32_t hdmirx_reg_read(char __iomem *addr, uint32_t offset, uint32_t mask)
{
	uint32_t temp;
	uint32_t index;

	if ((mask == 0) || (addr == NULL)) {
		return 0;
	}


	temp = inp32(addr + offset);
	index = get_right_high(mask);

	return ((temp & mask) >> index);
}

void hdmirx_reg_write(char __iomem *addr, uint32_t offset, uint32_t mask, uint32_t value)
{
	uint32_t temp;
	uint32_t index;

	if ((mask == 0) || (addr == NULL))
		return;

	temp = inp32(addr + offset);
	index = get_right_high(mask);
	value <<= index;
	value &= mask;
	temp &= ~mask;
	temp |= value;
	writel(temp, addr + offset);
}
