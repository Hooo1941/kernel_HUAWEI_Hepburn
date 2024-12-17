/*
 * brd_import.h
 *
 * routines imported from drivers/block/brd.c
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef BRD_IMPORT_H
#define BRD_IMPORT_H

#ifdef CONFIG_HUAWEI_DYNAMIC_BRD
#ifdef CONFIG_BOOT_DETECTOR_HISI
struct list_head* get_brd_devices(void);
struct mutex* get_brd_devices_mutex(void);
int get_rd_nr(void);
void dynamic_brd_del_one(struct brd_device *brd);
#else
int brd_create(unsigned int size);
int brd_delete(int nr);
#endif
#endif

#endif
