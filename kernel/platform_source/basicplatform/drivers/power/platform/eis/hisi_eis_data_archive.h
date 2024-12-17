/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_data_archive.h
 *
 * eis core io interface header
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

#ifndef _BATT_EIS_CORE_UFS_H_
#define _BATT_EIS_CORE_UFS_H_

#include <linux/device.h>
#include <linux/kernel.h>
#include "hisi_eis_core_freq.h"

#define EIS_BUF_PATH_LEN        128
#define EIS_FILE_LIMIT          0660

/* offset from 110K */
#define EIS_FREQ_OFFSET_BASE    0x1B800
#define EIS_FREQ_SIZE           0x190F0
#define FREQ_INDEX_OFFSET_BASE  0x348FF
/* offset 110k for batt_1 and batt_2 */
#define EIS_ONE_BATT_SIZE       0x1B800
#define FREQ_INDEX_MAGIC        0x7FF7
#define EIS_FREQ_ROLLING        160
#define EIS_FREQ_ROW            16
#define EIS_FREQ_LINE_WIDTH     180
/* (7 * sizeof (int) + 2 * sizeof(long long)+ 8) * 16 + 1 */
#define EIS_FREQ_PARAM_SIZE     4200

#define eis_freq_offset(x)      ((EIS_FREQ_OFFSET_BASE) + (x) * (EIS_ONE_BATT_SIZE))
#define freq_index_offset(x)    ((FREQ_INDEX_OFFSET_BASE) + (x) * (EIS_ONE_BATT_SIZE))

void hisi_eis_freq_write_info_to_flash(
	struct eis_freq_info *freq_inf, unsigned int batt);
void hisi_eis_freq_info_printk(struct eis_freq_info *freq_info, unsigned int batt);
void hisi_eis_freq_read_single_info_from_flash(
	struct eis_freq_info *freq_inf, int index, unsigned int batt);
void hisi_eis_freq_read_all_info_from_flash(
	struct eis_freq_info (*freq_inf)[LEN_T_FREQ], unsigned int batt);
int eis_get_g_full_freq_rolling(unsigned int batt);
int eis_get_g_freq_rolling_now(unsigned int batt);
void hisi_eis_freq_save_index_to_flash(unsigned int batt);
void hisi_eis_freq_read_index_from_flash(unsigned int batt);
int hisi_eis_read_flash_data(void *buf, size_t buf_size, u32 flash_offset);

#ifdef CONFIG_DFX_DEBUG_FS
int show_numed_idx_info_and_cycle_to_shell(int idx, unsigned int batt);
void hisi_eis_freq_clear_flash_data(unsigned int batt);
void set_latest_cycle_inc_to_flash_from_shell(int inc, unsigned int batt);
void test_freq_task_schedule(int batt);
int test_eis_set_temp(int temp);
int get_test_bat_temp(void);
#endif /* CONFIG_DFX_DEBUG_FS */

#endif /* _BATT_EIS_CORE_UFS_H_ */

