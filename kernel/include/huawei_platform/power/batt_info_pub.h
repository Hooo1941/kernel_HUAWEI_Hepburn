/*
 * batt_info_pub.h
 *
 * battery information public interface head file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BATT_INFO_PUB_H_
#define _BATT_INFO_PUB_H_

#define BATTERY_TYPE_BUFF_SIZE  6
#define BATTERY_TYPE_SIZE       2

#define BATTERY_SN_CMP_SAME     0
#define BATTERY_SN_CMP_DIFF     1
#define BATTERY_SN_CMP_ERROR    (-1)

#ifdef CONFIG_HUAWEI_BATTERY_INFORMATION
int get_battery_type(unsigned char *name, unsigned int name_size);
int get_batt_changed_on_boot(void);
int check_battery_sn_changed(void);
int get_battery_identifier(char *buf, int buf_size, int identifier_index, int identifier_len);
int get_battery_backup_id(char *buf, int buf_size);
#else
static inline int get_battery_type(unsigned char *name, unsigned int name_size)
{
	return -1;
}

static inline int get_batt_changed_on_boot(void)
{
	return BATTERY_SN_CMP_ERROR;
}

static inline int check_battery_sn_changed(void)
{
	return BATTERY_SN_CMP_ERROR;
}

static inline int get_battery_identifier(char *buf, int buf_size, int identifier_index, int identifier_len)
{
	return -1;
}

static inline int get_battery_backup_id(char *buf, int buf_size)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_BATTERY_INFORMATION */

#endif /* _BATT_INFO_PUB_H_ */
