/*
* haptic_core.h
*
* code for vibrator
*
* Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#ifndef _HAPTIC_CORE_H_
#define _HAPTIC_CORE_H_

#include "haptic_types.h"
#include "vib_log.h"

int hwvibrator_init(struct haptic *haptic);
int hwvibrator_deinit(struct haptic *haptic);

int haptic_play_effect_type(struct haptic *haptic, int type);
int ram_update(struct haptic *haptic);
int get_ram_num(struct haptic *haptic);

#endif // _HAPTIC_CORE_H_
