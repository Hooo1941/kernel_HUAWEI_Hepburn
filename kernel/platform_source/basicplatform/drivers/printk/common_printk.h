/*
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _COMMON_PRINTK_H_
#define _COMMON_PRINTK_H_

#include <linux/spinlock.h>
#include <linux/version.h>
#include <mntn_public_interface.h>

u64 dfx_getcurtime(void);

#define TIME_LOG_SIZE 80
#define TS_BUF_SIZE 80

#define FPGA_CONFIGED 1
#define FPGA_SERIAL_CLOCK 19200000

#define FPGA_SCBBPD_RX_STAT1 0x1008
#define FPGA_SCBBPD_RX_STAT2 0x100c

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#ifdef DEVKMSG_LIMIT_CONTROL
#define KMSG_TIME_INTERNEL	5
#define KMSG_RATELIMIT_BURST	100
#endif
#define PREFIX_MAX		80
#endif

#ifdef CONFIG_DFX_TIME
int decode_log_buff(char *dst, size_t size, char *src);
void parse_logringbuff_memory(char *buf, long *logringbuffer_lens, int num);
void free_logringbuff_memory(void);
void get_logringbuffer_addr_size(struct log_ring_buffer_dump_info *p);
int logbuf_print_all(char *buf, int size);
#else
static inline int decode_log_buff(char *dst, size_t size, char *src) { return 0; }
static inline void parse_logringbuff_memory(char *buf, long *logringbuffer_lens, int num) { return; }
static inline void free_logringbuff_memory(void) { return; }
static inline void get_logringbuffer_addr_size(struct log_ring_buffer_dump_info *p) { return; }
static inline int logbuf_print_all(char *buf, int size) { return 0; }
#endif
#endif
