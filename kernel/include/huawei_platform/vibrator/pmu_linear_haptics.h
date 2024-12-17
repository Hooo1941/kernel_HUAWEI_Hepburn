/*
 * pmu_linear_haptics.h
 *
 * pmu_linear_haptics driver
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
#ifndef _PMU_LINEAR_HAPTICS_H_
#define _PMU_LINEAR_HAPTICS_H_

#include <linux/types.h>

#ifdef CONFIG_PMU_LINEAR_HAPTIC_EVADE
bool pmu_linear_haptic_check_splice_haptic(unsigned int state);
void pmu_linear_haptic_play_splice_haptic_effect(unsigned int state);
#else
static inline bool pmu_linear_haptic_check_splice_haptic(unsigned int state)
{
	return false;
}
static inline void pmu_linear_haptic_play_splice_haptic_effect(unsigned int state)
{
}
#endif

#endif
