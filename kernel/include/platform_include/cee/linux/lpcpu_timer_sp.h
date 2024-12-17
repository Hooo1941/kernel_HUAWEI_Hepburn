/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description : information about timer sp
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LPCPU_TIMER_SP_H__
#define __LPCPU_TIMER_SP_H__

#ifdef CONFIG_LPCPU_HITIMER

#undef TIMER_1_BASE
#define TIMER_1_BASE	0x00

#undef TIMER_2_BASE
#define TIMER_2_BASE	0x400

#undef TIMER_LOAD
#define TIMER_LOAD	0x04 /* ACVR rw */

#undef TIMER_VALUE
#define TIMER_VALUE	0x14 /* ACVR ro */

#undef TIMER_CTRL
#define TIMER_CTRL	0x08 /* ACVR rw */

#undef TIMER_CTRL_ONESHOT
#define TIMER_CTRL_ONESHOT	((1 << 2) | (1 << 18)) /*  CVR */

#undef TIMER_CTRL_32BIT
#define TIMER_CTRL_32BIT	((1 << 1) | (1 << 17)) /*  CVR */

#undef TIMER_CTRL_DIV1
#define TIMER_CTRL_DIV1		(0 << 2) /* ACVR */

#undef TIMER_CTRL_DIV16
#define TIMER_CTRL_DIV16	(1 << 2) /* ACVR */

#undef TIMER_CTRL_DIV256
#define TIMER_CTRL_DIV256	(2 << 2) /* ACVR */

#undef TIMER_CTRL_IE
#define TIMER_CTRL_IE		((0 << 4) | (1 << 20)) /*   VR */

#undef TIMER_CTRL_INT_MASK
#define TIMER_CTRL_INT_MASK	((1 << 4) | (1 << 20)) /* INTR MASK */

#undef TIMER_CTRL_PERIODIC
#define TIMER_CTRL_PERIODIC	((2 << 2) | (3 << 18)) /* ACVR */

#undef TIMER_CTRL_ENABLE
#define TIMER_CTRL_ENABLE	((1 << 0) | (1 << 16)) /* ACVR */

#undef TIMER_INTCLR
#define TIMER_INTCLR	0x10 /* ACVR wo */

#undef TIMER_RIS
#define TIMER_RIS	0x10 /*  CVR ro */

#undef TIMER_MIS
#define TIMER_MIS	0x14 /*  CVR ro */

#undef TIMER_BGLOAD
#define TIMER_BGLOAD	0x18 /*  CVR rw */

#define LPCPU_CLK_CTRL		0x0
#define LPCPU_CLK_CTRL_VAL	((1 << 2) | (1 << 18))

#endif /* CONFIG_LPCPU_HITIMER */
#endif /* __LPCPU_TIMER_SP_H__ */
