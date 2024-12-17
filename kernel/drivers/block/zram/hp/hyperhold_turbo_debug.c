/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: hyperhold implement
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sched/task.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/memcontrol.h>
#include <linux/hp/memcg_policy.h>
#include <linux/hp/hyperhold_inf.h>

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"
#include "hyperhold_area.h"

#define TURBO_RECLAIM_IN_NUM 1024

static bool g_test_sync_flag = false;
static int g_test_set_exts_num = TURBO_RECLAIM_IN_NUM;
static int g_test_exts_num = 0;
static struct hyperhold_turbo_ext g_test_exts[TURBO_RECLAIM_IN_NUM] = { 0 };

void hyperhold_turbo_set_num(int num)
{
	g_test_set_exts_num = num;
	hh_print(HHLOG_ERR, "g_test_set_exts_num %d\n", g_test_set_exts_num);
}

void hyperhold_turbo_set_sync_flag(bool flag)
{
	g_test_sync_flag = flag;
	hh_print(HHLOG_ERR, "g_test_sync_flag %d\n", g_test_sync_flag);
}

static void hyperhold_turbo_end_test(int err, struct hyperhold_turbo_ext *ext)
{
	u8 *buffer = NULL;

	if (!ext) {
		hh_print(HHLOG_ERR, "ext is null\n");
		return;
	}

	hh_print(HHLOG_ERR, "err %d ext_id %d private %d page %llx\n",
		err, ext->ext_id, ext->private, ext->page);

	if (ext->page) {
		buffer = page_to_virt(ext->page);
		hh_print(HHLOG_ERR, "page %x %x %x %x\n",
			buffer[0], buffer[1], buffer[2], buffer[3]);
		__free_page(ext->page);
	}
}

void hyperhold_turbo_free_exts_test(void)
{
	int loop;

	if (g_test_exts_num > TURBO_RECLAIM_IN_NUM)
		g_test_exts_num = TURBO_RECLAIM_IN_NUM;

	for (loop = 0; loop < g_test_exts_num; ++loop) {
		hyperhold_free_nopkg_extent(g_test_exts[loop].ext_id);
		g_test_exts[loop].ext_id = -EINVAL;
	}
	g_test_exts_num = 0;
}

static int hyperhold_turbo_alloc_write_ext_test(u8 *value,
	struct hyperhold_turbo_ext *ext, int index)
{
	u8 *buffer = NULL;

	ext->ext_id = hyperhold_alloc_nopkg_extent();
	if (ext->ext_id < 0)
		return -ENOSPC;

	ext->page = alloc_page(GFP_KERNEL);
	if (!ext->page) {
		hyperhold_free_nopkg_extent(ext->ext_id);
		ext->ext_id = -EINVAL;
		return -ENOMEM;
	}
	ext->private = index;

	buffer = page_to_virt(ext->page);
	buffer[0] = (*value)++;
	buffer[1] = (*value)++;
	buffer[2] = (*value)++;
	buffer[3] = (*value)++;

	hh_print(HHLOG_ERR, "page %llx: %x %x %x %x\n",
		ext->page, buffer[0], buffer[1], buffer[2], buffer[3]);
	return 0;
}

static int hyperhold_turbo_alloc_read_ext_test(struct hyperhold_turbo_ext *ext,
	int index)
{
	u8 *buffer = NULL;

	ext->page = alloc_page(GFP_KERNEL);
	if (!ext->page) {
		hyperhold_free_nopkg_extent(ext->ext_id);
		ext->ext_id = -EINVAL;
		return -ENOMEM;
	}
	ext->private = index;

	buffer = page_to_virt(ext->page);
	hh_print(HHLOG_ERR, "page %llx: %x %x %x %x\n",
		ext->page, buffer[0], buffer[1], buffer[2], buffer[3]);
	return 0;
}

void hyperhold_turbo_reclaim_in_test(void)
{
	int num;
	int loop;
	struct hyperhold_turbo_para para;
	static u8 value = 0;

	para.done = hyperhold_turbo_end_test;
	para.sync_flag = g_test_sync_flag;
	para.exts = g_test_exts;

	if (g_test_set_exts_num > TURBO_RECLAIM_IN_NUM)
		g_test_set_exts_num = TURBO_RECLAIM_IN_NUM;

	for (loop = 0; loop < g_test_set_exts_num; ++loop)
		if (hyperhold_turbo_alloc_write_ext_test(&value, &para.exts[loop], loop))
			break;

	para.page_num = loop;
	g_test_exts_num = loop;

	num = hyperhold_turbo_reclaim_in(&para);
	hh_print(HHLOG_ERR, "page_num %d num %d sync_flag %d\n",
		para.page_num, num, g_test_sync_flag);
}

void hyperhold_turbo_batch_out_test(void)
{
	int num;
	int loop;
	struct hyperhold_turbo_para para;

	para.done = hyperhold_turbo_end_test;
	para.sync_flag = g_test_sync_flag;
	para.exts = g_test_exts;

	for (loop = 0; loop < g_test_exts_num; ++loop)
		if (hyperhold_turbo_alloc_read_ext_test(&para.exts[loop], loop))
			break;

	para.page_num = loop;

	num = hyperhold_turbo_batch_out(&para);
	hh_print(HHLOG_ERR, "page_num %d num %d sync_flag %d\n",
		para.page_num, num, g_test_sync_flag);
}

void hyperhold_turbo_fault_out_test(void)
{
	int num;
	struct hyperhold_turbo_para para;
	static u8 value = 0;

	para.done = hyperhold_turbo_end_test;
	para.sync_flag = g_test_sync_flag;
	para.exts = g_test_exts;

	hyperhold_turbo_alloc_read_ext_test(&para.exts[0], 0);
	para.page_num = 1;

	num = hyperhold_turbo_fault_out(&para);
	hh_print(HHLOG_ERR, "page_num %d num %d sync_flag %d\n",
		para.page_num, num, g_test_sync_flag);
}
