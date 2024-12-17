/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * mfregression.h
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM mfregression

#if !defined(_TRACE_MFREGRESSION_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MFREGRESSION_H

#include <platform_include/cee/linux/phase.h>
#include <linux/tracepoint.h>
#include <securec.h>

TRACE_EVENT(mfregression_predict_buffer,

	TP_PROTO(struct task_struct *pleft, struct task_struct *pright,
		 u64 *buffer, enum smt_mode mode, bool match),

	TP_ARGS(pleft, pright, buffer, mode, match),

	TP_STRUCT__entry(
		__array(	char,	left_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	left_pid			)
		__array(	char,	right_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	right_pid			)
		__field(	enum smt_mode,		mode	)
		__field(	bool,	match			)

		__field(	u64,	buffer0			)
		__field(	u64,	buffer1			)
		__field(	u64,	buffer2			)
		__field(	u64,	buffer3			)
		__field(	u64,	buffer4			)
		__field(	u64,	buffer5			)
		__field(	u64,	buffer6			)
		__field(	u64,	buffer7			)
		__field(	u64,	buffer8			)
		__field(	u64,	buffer9			)
		__field(	u64,	buffer10		)
		__field(	u64,	buffer11		)
		__field(	u64,	buffer12		)
		__field(	u64,	buffer13		)
		__field(	u64,	buffer14		)
		__field(	u64,	buffer15		)
		__field(	u64,	buffer16		)
		__field(	u64,	buffer17		)
		__field(	u64,	buffer18		)
		__field(	u64,	buffer19		)
		__field(	u64,	buffer20		)
	),

	TP_fast_assign(
		memcpy_s(__entry->left_comm, TASK_COMM_LEN, pleft->comm, TASK_COMM_LEN);
		__entry->left_pid	= pleft->pid;
		memcpy_s(__entry->right_comm, TASK_COMM_LEN, pright->comm, TASK_COMM_LEN);
		__entry->right_pid	= pright->pid;
		__entry->mode		= mode;
		__entry->match		= match;
		__entry->buffer0	= buffer[0];
		__entry->buffer1	= buffer[1];
		__entry->buffer2	= buffer[2];
		__entry->buffer3	= buffer[3];
		__entry->buffer4	= buffer[4];
		__entry->buffer5	= buffer[5];
		__entry->buffer6	= buffer[6];
		__entry->buffer7	= buffer[7];
		__entry->buffer8	= buffer[8];
		__entry->buffer9	= buffer[9];
		__entry->buffer10	= buffer[10];
		__entry->buffer11	= buffer[11];
		__entry->buffer12	= buffer[12];
		__entry->buffer13	= buffer[13];
		__entry->buffer14	= buffer[14];
		__entry->buffer15	= buffer[15];
		__entry->buffer16	= buffer[16];
		__entry->buffer17	= buffer[17];
		__entry->buffer18	= buffer[18];
		__entry->buffer19	= buffer[19];
		__entry->buffer20	= buffer[20];
	),

	TP_printk("match=%d mode=%d "
		  "left_comm=%s left_pid=%d "
		  "right_comm=%s right_pid=%d "
		  "buffer[0]=%llu "
		  "buffer[1-5]=%llu/%llu/%llu/%llu/%llu\n "
		  "buffer[6-10]=%llu/%llu/%llu/%llu/%llu\n "
		  "buffer[11-15]=%llu/%llu/%llu/%llu/%llu\n "
		  "buffer[16-20]=%llu/%llu/%llu/%llu/%llu\n ",
		  __entry->match, __entry->mode,
		  __entry->left_comm, __entry->left_pid,
		  __entry->right_comm, __entry->right_pid,
		  __entry->buffer0,
		  __entry->buffer1, __entry->buffer2, __entry->buffer3, __entry->buffer4, __entry->buffer5,
		  __entry->buffer6, __entry->buffer7, __entry->buffer8, __entry->buffer9, __entry->buffer10,
		  __entry->buffer11, __entry->buffer12, __entry->buffer13, __entry->buffer14, __entry->buffer15,
		  __entry->buffer16, __entry->buffer17, __entry->buffer18, __entry->buffer19, __entry->buffer20)
);

TRACE_EVENT(mfregression_predict_pmu,

	TP_PROTO(struct task_struct *pleft, struct task_struct *pright,
		 struct phase_event_pcount *cleft, struct phase_event_pcount *cright,
		 enum smt_mode mode, bool match),

	TP_ARGS(pleft, pright, cleft, cright, mode, match),

	TP_STRUCT__entry(
		__array(	char,	left_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	left_pid			)
		__array(	char,	right_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	right_pid			)
		__field(	enum smt_mode,	mode		)
		__field(	bool,	match			)

		__field(	u64,	cleft0			)
		__field(	u64,	cleft1			)
		__field(	u64,	cleft2			)
		__field(	u64,	cleft3			)
		__field(	u64,	cleft4			)
		__field(	u64,	cleft5			)
		__field(	u64,	winleft			)

		__field(	u64,	cright0			)
		__field(	u64,	cright1			)
		__field(	u64,	cright2			)
		__field(	u64,	cright3			)
		__field(	u64,	cright4			)
		__field(	u64,	cright5			)
		__field(	u64,	winright		)
	),

	TP_fast_assign(
		memcpy_s(__entry->left_comm, TASK_COMM_LEN, pleft->comm, TASK_COMM_LEN);
		__entry->left_pid	= pleft->pid;
		memcpy_s(__entry->right_comm, TASK_COMM_LEN, pright->comm, TASK_COMM_LEN);
		__entry->right_pid	= pright->pid;
		__entry->mode		= mode;
		__entry->match		= match;
		__entry->cleft0		= cleft->data[0];
		__entry->cleft1		= cleft->data[1];
		__entry->cleft2		= cleft->data[2];
		__entry->cleft3		= cleft->data[3];
		__entry->cleft4		= cleft->data[4];
		__entry->cleft5		= cleft->data[5];
		__entry->winleft	= phase_active_window_size(&pleft->phase_info->window, mode);
		__entry->cright0	= cright->data[0];
		__entry->cright1	= cright->data[1];
		__entry->cright2	= cright->data[2];
		__entry->cright3	= cright->data[3];
		__entry->cright4	= cright->data[4];
		__entry->cright5	= cright->data[5];
		__entry->winright	= phase_active_window_size(&pright->phase_info->window, mode);
	),

	TP_printk("match=%d mode=%d "
		  "left_comm=%s left_pid=%d left_pmu=%llu/%llu/%llu/%llu/%llu/%llu left_win=%llu ==> "
		  "right_comm=%s right_pid=%d right_pmu=%llu/%llu/%llu/%llu/%llu/%llu right_win=%llu",
		  __entry->match, __entry->mode,
		  __entry->left_comm, __entry->left_pid,
		  __entry->cleft0, __entry->cleft1, __entry->cleft2,
		  __entry->cleft3, __entry->cleft4, __entry->cleft5,
		  __entry->winleft,
		  __entry->right_comm, __entry->right_pid,
		  __entry->cright0, __entry->cright1, __entry->cright2,
		  __entry->cright3, __entry->cright4, __entry->cright5,
		  __entry->winright)
);

#endif

/* This part must be outside protection */
#include <trace/define_trace.h>
