/* comb_key_event driver
*
* Copyright (c) 2021 Huawei Technologies Co., Ltd.
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

#ifndef _EXT_HALL_EVENT_H
#define _EXT_HALL_EVENT_H
#include <linux/notifier.h>

#define DEFALT_EXT_HALL_FOLD_STATUS  1

enum {
	EXT_HALL_BIT_CASE = 0,
	EXT_HALL_BIT_HINGLE = 1,
};

#ifdef CONFIG_HUAWEI_HALL_INPUTHUB
int ext_hall_register_notifier(struct notifier_block *nb);
int ext_hall_unregister_notifier(struct notifier_block *nb);
int ext_hall_call_notifiers(unsigned long val,void *v);
void ext_hall_status_distinguish(unsigned long pressed);
int ext_hall_nb_init(void);
#else
int get_ext_hall_status(unsigned int ext_hall_type)
{
	return 1;
}

int ext_hall_register_notifier(struct notifier_block *nb)
{
	return 0;
}

int ext_hall_unregister_notifier(struct notifier_block *nb)
{
	return 0;
}

int ext_hall_call_notifiers(unsigned long val,void *v)
{
	return 0;
}

void ext_hall_status_distinguish(unsigned long pressed)
{
	return;
}

int ext_hall_nb_init(void)
{
	return 0;
}

#endif

#endif
