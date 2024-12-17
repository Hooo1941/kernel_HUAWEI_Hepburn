/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * phase_feature_mfregression.c
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

#include <platform_include/cee/linux/phase.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/build_bug.h>
#include <../kernel/sched/sched.h>
#include "phase_feature.h"

#define CREATE_TRACE_POINTS
#include <platform_include/trace/events/mfregression.h>

#define MODEL_FORWARD_PATH "/mnt/MFREGRESSION_FORWARD.cfg"
#define MODEL_REVERSE_PATH "/mnt/MFREGRESSION_REVERSE.cfg"
#define NR_MAX_PMU_EVENTS	5

/*
 * struct mfregression layout:
 *       ---------------------------------------------
 *       |   int type                                |
 *       ---------------------------------------------
 *       |   int model_scaler                        |
 *       ---------------------------------------------
 *       |   int buffer_scaler                       |
 *       ---------------------------------------------
 *       |   int dim_para                            |
 *       ---------------------------------------------
 *       |   int ratio                               |
 *       ---------------------------------------------
 *       |   int ratio_scaler                        |
 *       ---------------------------------------------
 *       |   int ratio_enable                        |
 *       ---------------------------------------------
 *       |   int ipc_gain                            |
 *       ---------------------------------------------
 *       |   int *parameters                         |
 *       ---------------------------------------------
 *       |   u64 *buffer                             |
 *       ---------------------------------------------
 *       |   u64 *last_pred_ipc                      |
 *       ---------------------------------------------
 *       |   int *pid_record                         |
 *       ---------------------------------------------
 */

struct mfregression {
	int type;
	int model_scaler;
	int buffer_scaler;
	int dim_para;
	int ratio;
	int ratio_scaler;
	int ratio_enable;
	int ipc_gain;
	int *parameters;
	u64 *buffer;
	u64 *last_pred_ipc;
	int *pid_record;
};


#define PHASE_EVENT_LIST	\
	EVENT(CYCLES),		\
	EVENT(INST_RETIRED),	\
	EVENT(BR_MIS_PRED),	\
	EVENT(ROB_FLUSH),	\
	EVENT(DSP_STALL),	\
	EVENT(L1D_CACHE)

enum {
#define EVENT(e) e##_INDEX
	PHASE_EVENT_LIST
#undef EVENT
};

static atomic_t mfregression_created = ATOMIC_INIT(0);
static struct mfregression *pmfregression[NR_PREDICT_MODE];

static int mfregression_pevents[PHASE_PEVENT_NUM] = {
#define EVENT(e) e
	PHASE_EVENT_LIST,
#undef EVENT
	PHASE_EVENT_FINAL_TERMINATOR,
};

static int build_parameters(struct mfregression *mfregression, char *buf, s64 *pos, loff_t size)
{
	/* get parameters */
	mfregression->parameters = (int *)((char *)mfregression + sizeof(struct mfregression));
	if (parse_one_line(mfregression->parameters, buf + *pos, mfregression->dim_para) != 0)
		return -EINVAL;

	consume_line(buf, pos, size);
	return 0;
}

static void print_mfregression(struct mfregression *mfregression)
{
	int i;
	printk("The model of MFREGRESSION:\n");
	printk("type %d, model_scaler %d, buffer_scaler %d, dim_para %d\n ratio %d, ratio_scaler %d, ratio_enable %d, ipc_gain %d",
		mfregression->type, mfregression->model_scaler, mfregression->buffer_scaler,
		mfregression->dim_para, mfregression->ratio, mfregression->ratio_scaler,
		mfregression->ratio_enable, mfregression->ipc_gain);
	printk("mfregression parameters:\n");
	for (i = 0; i < mfregression->dim_para; i++)
		printk("%8d", (mfregression->parameters)[i]);
	printk("\n");
}

static int build_mfregression_from_file(struct mfregression *mfregression, const char *model_file_path)
{
	int ret;
	char *buf = NULL;
	loff_t pos = 0;
	loff_t size = 0;

	buf = load_from_file(model_file_path, &size);
	if (buf == NULL)
		return -ENOMEM;

	/* get model mode */
	get_matches(buf + pos, "%d %d %d %d %d %d %d %d\n", &mfregression->type, &mfregression->model_scaler,
		    &mfregression->buffer_scaler, &mfregression->dim_para, &mfregression->ratio, &mfregression->ratio_scaler,
		    &mfregression->ratio_enable, &mfregression->ipc_gain);
	consume_line(buf, &pos, size);

	ret = build_parameters(mfregression, buf, &pos, size);
	if (ret != 0) {
		pr_err("build mfregression->parameters fail\n");
		goto out_free_buf;
	}
	mfregression->buffer = (u64 *)((char *)mfregression->parameters + mfregression->dim_para * sizeof(int));
	mfregression->last_pred_ipc = (u64 *)((char *)mfregression->buffer + mfregression->dim_para * sizeof(u64));
	mfregression->pid_record = (int *)((char *)mfregression->last_pred_ipc + sizeof(u64));

	*(mfregression->last_pred_ipc) = 0;

	print_mfregression(mfregression);

out_free_buf:
	kfree(buf);
	return ret;
}

static size_t get_mfregression_size_from_cfg(void)
{
	char *buf = NULL;
	loff_t pos = 0;
	loff_t size = 0;
	size_t mfregression_size;
	struct mfregression tmp;

	buf = load_from_file(MODEL_FORWARD_PATH, &size);
	if (buf == NULL)
		return 0;

	/* get model mode */
	get_matches(buf + pos, "%d %d %d %d %d %d %d %d\n", &tmp.type, &tmp.model_scaler,
		    &tmp.buffer_scaler, &tmp.dim_para, &tmp.ratio, &tmp.ratio_scaler,
		    &tmp.ratio_enable, &tmp.ipc_gain);
	consume_line(buf, &pos, size);

	/* kzalloc mfregression */
	mfregression_size = sizeof(struct mfregression) + tmp.dim_para * sizeof(int) +
				tmp.dim_para * sizeof(u64) + sizeof(u64) + 2 * sizeof(int);
	kfree(buf);
	return mfregression_size;
}

static inline int build_mfregression(struct mfregression *mfregression, enum phase_predict_mode predict_mode)
{
	int ret;
	const char *config_file = (predict_mode == FORWARD_MODE) ? MODEL_FORWARD_PATH : MODEL_REVERSE_PATH;

	/* load mfregression from file and build mfregression */
	ret = build_mfregression_from_file(mfregression, config_file);
	if (ret != 0) {
		pr_err("build MFREGRESSION fail\n");
		return ret;
	}

	pr_info("create MFREGRESSION mode [%d] success\n", predict_mode);
	return ret;
}

static int phase_build_mfregression(void)
{
	int index, i, ret;
	size_t mfregression_size;
	struct mfregression *mfregression = NULL;

	BUILD_BUG_ON((int)FORWARD_MODE != (int)ST_MODE);
	BUILD_BUG_ON((int)REVERSE_MODE != (int)SMT_MODE);

	if (atomic_read(&mfregression_created) != 0)
		return -EEXIST;

	/* get kzalloc mfregression size */
	mfregression_size = get_mfregression_size_from_cfg();
	if (mfregression_size == 0) {
		pr_err("get MFREGRESSION size 0 error\n");
		return -EINVAL;
	}

	mfregression = kzalloc(mfregression_size * NR_PREDICT_MODE, GFP_KERNEL);
	if (mfregression == NULL) {
		pr_err("kzalloc %zu error\n", mfregression_size * NR_PREDICT_MODE);
		return -ENOMEM;
	}

	for (index = 0; index < NR_PREDICT_MODE; index++) {
		pmfregression[index] = (struct mfregression *)((char *)mfregression + index * mfregression_size);
		ret = build_mfregression(pmfregression[index], index);
		if (ret != 0)
			goto out_del_mfregression;
	}

	phase_tk_mfregression.private_data = (void *)mfregression;
	atomic_inc(&mfregression_created);

	return 0;

out_del_mfregression:
	for (i = 0; i < index; i++)
		pmfregression[index] = NULL;

	kfree(mfregression);

	return ret;
}

static void mfregression_input_data_trans(u64 lpmudata, u64 rpmudata, u64 lpmu, u64 rpmu, u64 *buffer, int *pos)
{
	u64 ltemp = PHASE_SCALE(lpmudata) / lpmu;
	u64 rtemp = PHASE_SCALE(rpmudata) / rpmu;
	buffer[(*pos)++] = ltemp;
	buffer[(*pos)++] = rtemp;
	buffer[(*pos)++] = rtemp * ltemp;
	buffer[(*pos)++] = ltemp * ltemp;
	buffer[(*pos)++] = rtemp * rtemp;
}

static bool calc_feature_per_inst(struct phase_info *linfo, struct phase_info *rinfo, enum smt_mode smt, u64 *buffer)
{
	int pos = 0;
	u64 *ldata = phase_get_counter(linfo, smt)->data;
	u64 *rdata = phase_get_counter(rinfo, smt)->data;

	if (ldata[INST_RETIRED_INDEX] < 10000 || rdata[INST_RETIRED_INDEX] < 10000)
		return false;

	buffer[pos++] = 1;
	/* calc task feature */
	mfregression_input_data_trans(ldata[CYCLES_INDEX], rdata[CYCLES_INDEX],
				      ldata[INST_RETIRED_INDEX], rdata[INST_RETIRED_INDEX], buffer, &pos);
	mfregression_input_data_trans(ldata[BR_MIS_PRED_INDEX] + ldata[ROB_FLUSH_INDEX],
				      rdata[BR_MIS_PRED_INDEX] + rdata[ROB_FLUSH_INDEX],
				      ldata[INST_RETIRED_INDEX], rdata[INST_RETIRED_INDEX], buffer, &pos);
	mfregression_input_data_trans(ldata[DSP_STALL_INDEX], rdata[DSP_STALL_INDEX],
				      ldata[INST_RETIRED_INDEX], rdata[INST_RETIRED_INDEX],
				      buffer, &pos);
	mfregression_input_data_trans(ldata[L1D_CACHE_INDEX], rdata[L1D_CACHE_INDEX],
				      ldata[INST_RETIRED_INDEX], rdata[INST_RETIRED_INDEX],
				      buffer, &pos);

	return true;
}

static bool calc_feature_per_cycle(struct phase_info *linfo, struct phase_info *rinfo, enum smt_mode smt, u64 *buffer)
{
	int pos = 0;
	u64 *ldata = phase_get_counter(linfo, smt)->data;
	u64 *rdata = phase_get_counter(rinfo, smt)->data;

	if (ldata[INST_RETIRED_INDEX] < 10000 || rdata[INST_RETIRED_INDEX] < 10000)
		return false;

	buffer[pos++] = 1;
	/* calc task feature */
	mfregression_input_data_trans(ldata[INST_RETIRED_INDEX], rdata[INST_RETIRED_INDEX],
				      ldata[CYCLES_INDEX], rdata[CYCLES_INDEX], buffer, &pos);
	mfregression_input_data_trans(ldata[BR_MIS_PRED_INDEX] + ldata[ROB_FLUSH_INDEX],
				      rdata[BR_MIS_PRED_INDEX] + rdata[ROB_FLUSH_INDEX],
				      ldata[CYCLES_INDEX], rdata[CYCLES_INDEX], buffer, &pos);
	mfregression_input_data_trans(ldata[DSP_STALL_INDEX], rdata[DSP_STALL_INDEX],
				      ldata[CYCLES_INDEX], rdata[CYCLES_INDEX], buffer, &pos);
	mfregression_input_data_trans(ldata[L1D_CACHE_INDEX], rdata[L1D_CACHE_INDEX],
				      ldata[CYCLES_INDEX], rdata[CYCLES_INDEX], buffer, &pos);

	return true;
}

static bool phase_calc_feature(struct phase_info *linfo, struct phase_info *rinfo, enum smt_mode mode, u64 *buffer)
{
	if (!!sysctl_phase_calc_feature_mode)
		return calc_feature_per_inst(linfo, rinfo, mode, buffer);
	else
		return calc_feature_per_cycle(linfo, rinfo, mode, buffer);
}

static void update_ratio(struct task_struct *pleft, struct task_struct *pright, u64 ipc_measure, enum smt_mode smt)
{
	int pid1 = (pmfregression[1 - smt]->pid_record)[0];
	int pid2 = (pmfregression[1 - smt]->pid_record)[1];
	int coeff1 = 70, coeff2 = 30;

	if ((pleft->pid == pid1 && pright->pid == pid2) || (pleft->pid == pid2 && pright->pid == pid1)) {
		if (*(pmfregression[1 - smt]->last_pred_ipc) != 0) {
			int ratio_tmp = ipc_measure * pmfregression[1 - smt]->ratio_scaler / *(pmfregression[1 - smt]->last_pred_ipc);
			pmfregression[1 - smt]->ratio = (ratio_tmp * coeff1 + pmfregression[1 - smt]->ratio * coeff2) / (coeff1 + coeff2);
		}
	}
}

static u64 predict_ipc(struct mfregression *mfregression)
{
	int i;
	s64 ipc_pred = 0;

	for (i = 0; i < mfregression->dim_para; i++)
		ipc_pred += (s64)mfregression->buffer[i] * mfregression->parameters[i];

	if (ipc_pred <= 0)
		return 0;

	return (u64)ipc_pred;
}

static bool predict_mfregression(struct mfregression *mfregression,
				 struct task_struct *pleft, struct task_struct *pright, enum smt_mode smt)
{
	bool is_match = false;
	u64 *ldata = phase_get_counter(pleft->phase_info, smt)->data;
	u64 *rdata = phase_get_counter(pright->phase_info, smt)->data;
	u64 ipc = PHASE_SCALE(ldata[INST_RETIRED_INDEX]) * mfregression->buffer_scaler / ldata[CYCLES_INDEX] +
			PHASE_SCALE(rdata[INST_RETIRED_INDEX]) * mfregression->buffer_scaler / rdata[CYCLES_INDEX];
	u64 ipc_pred = predict_ipc(mfregression);

	if (ipc_pred == 0)
		return false;

	if (mfregression->ratio_enable == 1) {
		update_ratio(pleft, pright, ipc, smt);
		ipc *= mfregression->ratio_scaler;
		ipc_pred *= mfregression->ratio;
	}
	if (smt == REVERSE_MODE) {
		if (100 * ipc < 100 * ipc_pred + mfregression->ipc_gain * ipc_pred) {
			is_match = false;
			*(pmfregression[smt]->last_pred_ipc) = ipc_pred;
			(mfregression->pid_record)[0] = pleft->pid;
			(mfregression->pid_record)[1] = pright->pid;
		} else {
			is_match = true;
		}
	} else {
		if (100 * ipc_pred < 100 * (ipc >> 1) + mfregression->ipc_gain * (ipc >> 1)) {
			is_match = false;
		} else {
			is_match = true;
			*(pmfregression[smt]->last_pred_ipc) = ipc_pred;
			(mfregression->pid_record)[0] = pleft->pid;
			(mfregression->pid_record)[1] = pright->pid;
		}
	}

	/* print the data which calc by pmu */
	trace_mfregression_predict_buffer(pleft, pright, mfregression->buffer, smt, is_match);

	/* print the raw pmu data */
	trace_mfregression_predict_pmu(pleft, pright, phase_get_counter(pleft->phase_info, smt),
					phase_get_counter(pright->phase_info, smt), smt, is_match);
	return is_match;
}

static bool phase_predict_mfregression(struct task_struct *left,
					struct task_struct *right,
					enum phase_predict_mode predict_mode)
{
	if (atomic_read(&mfregression_created) == 0)
		return false;

	if (!phase_calc_feature(left->phase_info, right->phase_info, predict_mode, pmfregression[predict_mode]->buffer))
		return true;

	return predict_mfregression(pmfregression[predict_mode], left, right, predict_mode);
}

static inline void phase_destroy_mfregression(void)
{
	int index;
	struct mfregression *mfregression = NULL;

	atomic_dec(&mfregression_created);

	mfregression = (struct mfregression *)phase_tk_mfregression.private_data;
	if (mfregression == NULL)
		return;

	for (index = 0; index < NR_PREDICT_MODE; index++)
		pmfregression[index] = NULL;

	kfree(mfregression);
	phase_tk_mfregression.private_data = NULL;
}

static inline int phase_create_mfregression(void)
{
	return phase_build_mfregression();
}

static inline void phase_release_mfregression(void)
{
	phase_destroy_mfregression();
}

struct phase_think_class phase_tk_mfregression = {
	.id = PHASE_TK_ID_MULTI_FEATURE_REGRESSION,
	.name = "mfregression",
	.pevents = mfregression_pevents,
	.fevents = NULL,
	.create = phase_create_mfregression,
	.release = phase_release_mfregression,
	.predict = phase_predict_mfregression,
	.sched_core_find = phase_sched_core_find_default,
	.next = NULL,
};
