/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: coul_core.c
 *
 * smartstar coulometer functions
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <securec.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_event.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "coul_dts.h"
#include "coul_fault_evt_ops.h"
#include "coul_interface.h"
#include "coul_nv.h"
#include "coul_temp.h"
#include "coul_ocv_ops.h"
#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>

static int calculate_real_fcc_uah(struct smartstar_coul_device *di,
	int *ret_fcc_uah);
static int coul_get_rm(struct smartstar_coul_device *di);

static ATOMIC_NOTIFIER_HEAD(g_coul_fault_notifier_list);
static ATOMIC_NOTIFIER_HEAD(g_coul_fault_notifier_list_h);

static int register_coul_fault_notifier(struct notifier_block *nb, int batt)
{
	if (batt == BATT_0)
		return atomic_notifier_chain_register(&g_coul_fault_notifier_list, nb);
	else
		return atomic_notifier_chain_register(&g_coul_fault_notifier_list_h, nb);
}

int coul_core_notify_fault(int batt, unsigned long val, void *v)
{
	if (batt == BATT_0)
		return atomic_notifier_call_chain(&g_coul_fault_notifier_list, val, v);
	else
		return atomic_notifier_call_chain(&g_coul_fault_notifier_list_h, val, v);
}

static int unregister_coul_fault_notifier(struct notifier_block *nb, int batt)
{
	if (batt == BATT_0)
		return atomic_notifier_chain_unregister(&g_coul_fault_notifier_list, nb);
	else
		return atomic_notifier_chain_unregister(&g_coul_fault_notifier_list_h, nb);
}

static void basp_fcc_learn_evt_handler(struct smartstar_coul_device *di,
	enum basp_fcc_learn_event evt)
{
	switch (evt) {
	case EVT_START:
		di->g_basp_fcc_ls = LS_INIT;
		break;
	case EVT_PER_CHECK:
		if ((di->g_basp_fcc_ls == LS_INIT) || (di->g_basp_fcc_ls == LS_GOOD)) {
			if ((di->batt_temp > BASP_FCC_LERAN_TEMP_MIN) &&
				(di->batt_temp < BASP_FCC_LERAN_TEMP_MAX))
				di->g_basp_fcc_ls = LS_GOOD;
			else
				di->g_basp_fcc_ls = LS_BAD;
		}
		break;
	case EVT_DONE:
		di->g_basp_fcc_ls = LS_UNKNOWN;
		break;
	default:
		break;
	}

	if (di->g_basp_fcc_ls != di->prev_state) {
		coul_core_info(BASP_TAG"prev_state:%d, new_state:%d, batt_temp:%d\n",
			di->prev_state, di->g_basp_fcc_ls, di->batt_temp);
		di->prev_state = di->g_basp_fcc_ls;
	}
}

static int calc_uuc(struct smartstar_coul_device *di, int zero_voltage,
	int ratio, int i_ma)
{
	int prev_delta_uv = 0;
	int prev_rbatt_mohm = 0;
	int i, unusable_uv, uuc_rbatt_uv, ocv_uv, rbatt_mohm;

	for (i = 0; i <= SOC_FULL; i++) {
		ocv_uv = coul_interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
			di->batt_temp / TENTH, i * TENTH) * PERMILLAGE;
		rbatt_mohm = coul_get_rbatt(di, i, di->batt_temp);

		coul_core_debug("%s, batt %d, rbatt_mohm %d, ratio %d, i_ma %d, zero_voltage %d\n",
			__func__, di->batt_index, rbatt_mohm, ratio, i_ma, zero_voltage);

		rbatt_mohm = rbatt_mohm * ratio / PERCENT;
		unusable_uv = (rbatt_mohm * i_ma) + (zero_voltage * PERMILLAGE);

		if (ocv_uv - unusable_uv > 0) {
			coul_core_debug("%s, batt %d, ocv_uv %d, unusable_uv %d\n",
				__func__, di->batt_index, ocv_uv, unusable_uv);
			break;
		}

		prev_delta_uv = ocv_uv - unusable_uv;
		prev_rbatt_mohm = rbatt_mohm;
	}
	uuc_rbatt_uv = coul_linear_interpolate(rbatt_mohm, ocv_uv - unusable_uv,
		prev_rbatt_mohm, prev_delta_uv, 0);

	unusable_uv = (uuc_rbatt_uv * i_ma) + (zero_voltage * PERMILLAGE);
	coul_core_debug("%s, batt %d, unusable_uv %d\n",
		__func__, di->batt_index, unusable_uv);
	return coul_calculate_pc(di, unusable_uv, di->batt_temp,
		di->batt_chargecycles / PERCENT);
}

static int calc_rm_chg_uah(struct smartstar_coul_device *di, int fcc_uah)
{
	int pc, remaining_charge_uah;

	pc = coul_calculate_pc(di, di->batt_ocv, di->batt_ocv_temp,
		di->batt_chargecycles / PERCENT);
	remaining_charge_uah = (fcc_uah / PERMILLAGE) * pc;

	return remaining_charge_uah;
}

/* Return: unuse uah without adjust */
static int calculate_termination_uuc_pc(
	struct smartstar_coul_device *di, int cur_ma)
{
	int zero_voltage, pc_unusable;
	int i_ma = cur_ma;
	int ratio = RARIO_MIN;

#if RBATT_ADJ
	if (di->rbatt_ratio) {
		ratio = di->rbatt_ratio;
		i_ma = di->last_fifo_iavg_ma;
	}
#endif
	zero_voltage = coul_get_zero_cap_vol(di);
	pc_unusable = calc_uuc(di, zero_voltage, ratio, i_ma);

	coul_core_debug("%s, batt %d, zero_voltage %d, pc_unusable %d\n", __func__,
		di->batt_index, zero_voltage, pc_unusable);
	return pc_unusable;
}

/* adjust unuse uah, uuc changes no more than 2% */
static int adjust_uuc(struct smartstar_coul_device *di, int fcc_uah,
	int new_pc_unusable, int new_uuc)
{
	int uuc_pc_max_diff = UUC_PC_MAX_DIFF;
	int uuc_pc_step_add = UUC_PC_STEP_ADD;
	int uuc_pc_step_sub = UUC_PC_STEP_SUB;

	coul_core_debug("%s, batt %d\n", __func__, di->batt_index);

	if ((di->prev_pc_unusable == -EINVAL) ||
		(abs(di->prev_pc_unusable - new_pc_unusable) <= uuc_pc_max_diff)) {
		di->prev_pc_unusable = new_pc_unusable;
		return new_uuc;
	}

	/* The UUC change in each cycle is reduced by half in low-temperature scenarios for soc smooth */
	if ((di->batt_soc > NORMAL_SOC_LIMIT) &&
		(di->batt_temp / TENTH < ZERO_V_ADJ_END_T))
		uuc_pc_step_add = uuc_pc_step_add / HALF;

	/* the uuc is trying to change more than 2% restrict it */
	if (new_pc_unusable > di->prev_pc_unusable)
		di->prev_pc_unusable += uuc_pc_step_add;
	else
		di->prev_pc_unusable -= uuc_pc_step_sub;

	coul_core_debug("%s, batt %d, new_pc_unusable %d, prev_pc_unusable %d, "
	"uuc_pc_step_add %d, uuc_pc_step_sub %d\n", __func__, di->batt_index,
	new_pc_unusable, di->prev_pc_unusable, uuc_pc_step_add, uuc_pc_step_sub);

	new_uuc = (fcc_uah / PERMILLAGE) * di->prev_pc_unusable;

	return new_uuc;
}

/* Return: unuse uah without adjust and changes less than 2% */
static int calculate_unusable_charge_uah(
	struct smartstar_coul_device *di, int fcc_uah, int iavg_ua)
{
	int i;
	int uuc_uah_iavg;
	int iavg_ma = iavg_ua / PERMILLAGE;
	int pc_unusable;

	/* use a nominal avg current to keep a reasonable UUC while charging */
	iavg_ma = (iavg_ma < 0) ? CHARGING_IAVG_MA : iavg_ma;
	di->iavg1.iavg_samples[di->iavg1.iavg_index] = iavg_ma;
	di->iavg1.iavg_index = (di->iavg1.iavg_index + 1) % IAVG_SAMPLES;
	di->iavg1.iavg_num_samples++;
	if (di->iavg1.iavg_num_samples >= IAVG_SAMPLES)
		di->iavg1.iavg_num_samples = IAVG_SAMPLES;

	/* now that this sample is added calcualte the average */
	iavg_ma = 0;
	if (di->iavg1.iavg_num_samples != 0) {
		for (i = 0; i < di->iavg1.iavg_num_samples; i++)
			iavg_ma += di->iavg1.iavg_samples[i];
		iavg_ma = DIV_ROUND_CLOSEST(iavg_ma, di->iavg1.iavg_num_samples);
	}

	pc_unusable = calculate_termination_uuc_pc(di, iavg_ma);
	uuc_uah_iavg = pc_unusable * (fcc_uah / PERMILLAGE);
	coul_core_info("RBATT_ADJ: batt %d, UUC= %d uAh, pc= %d.%d\n",
		di->batt_index, uuc_uah_iavg, pc_unusable / TENTH, pc_unusable % TENTH);

	di->rbatt_ratio = 0;
	/* restrict the uuc such that it can increase only by one percent */
	uuc_uah_iavg = adjust_uuc(di, fcc_uah, pc_unusable, uuc_uah_iavg);
	/* while discharging */
	if (iavg_ua >= 0)
		uuc_uah_iavg += fcc_uah / PERCENT;
	di->batt_uuc = uuc_uah_iavg;
	coul_core_debug("%s, batt %d, batt_uuc %d\n", __func__,
		di->batt_index, di->batt_uuc);

	return uuc_uah_iavg;
}

/* set the new battery charge cycles and notify all who care about */
static void set_charge_cycles(struct smartstar_coul_device *di,
	 const unsigned int cycles)
{
	di->batt_chargecycles = cycles;

	/* The cycles of high-side battery and low-side battery must be similar. */
	if(di->batt_index == BATT_0)
		call_coul_blocking_notifiers(BATT_EEPROM_CYC, &di->batt_chargecycles);
	coul_core_debug("%s, batt %d, batt_chargecycles %d\n", __func__,
		di->batt_index, di->batt_chargecycles);
}

/* update charge/discharge times */
static void update_chargecycles(struct smartstar_coul_device *di)
{
	int batt_soc_real, charging_begin_soc;

	batt_soc_real = di->batt_soc_real / TENTH;
	charging_begin_soc = di->charging_begin_soc / TENTH;
	if (batt_soc_real - charging_begin_soc > 0) {
		set_charge_cycles(di, di->batt_chargecycles +
			batt_soc_real - charging_begin_soc);
		coul_core_info("batt %d, new chargecycle=%u, added=%d\n", di->batt_index,
			di->batt_chargecycles, batt_soc_real - charging_begin_soc);
	} else {
		coul_core_info("batt %d, chargecycle not updated, soc_begin=%d, soc_current=%d, batt_soc=%d\n",
			di->batt_index, charging_begin_soc, batt_soc_real, di->batt_soc);
	}
	if (coul_battery_para_changed(di) < 0)
		coul_core_err("batt %d, battery charge para fail\n", di->batt_index);
	di->charging_begin_soc = SOC_FULL_TENTH;
}

/* Determine if high precision qmax can update, Remark: Depending on the FCC's update condition */
static void is_high_precision_qmax_ready_to_refresh(
	struct smartstar_coul_device *di)
{
	di->qmax_start_pc = coul_interpolate_pc(di->batt_data->pc_temp_ocv_lut0,
		di->batt_ocv_temp, di->batt_ocv / PERMILLAGE) / TENTH;
	di->qmax_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index) / UA_PER_MA;

	/* start pc is lower than 20% */
	if (di->qmax_start_pc < MIN_BEGIN_PERCENT_FOR_QMAX)
		di->qmax_refresh_flag = 1;
	coul_core_info("[%s] batt %d, start_ocv = %d, start_pc = %d, cc = %d\n",
		__func__, di->batt_index, di->batt_ocv, di->qmax_start_pc, di->qmax_cc);
}

/* For Repair Network NFF tool detection only Dependencies on OCV Updates. */
static int get_high_pre_qmax(struct smartstar_coul_device *di)
{
	int design_fcc_mah;
	int tmp_qmax;
	int delta_cv_pc;

	coul_core_debug("[%s] batt %d, qmax_refresh_flag %d\n",
		__func__, di->batt_index, di->qmax_refresh_flag);
	if (di->qmax_refresh_flag) {
		di->qmax_end_pc = coul_interpolate_pc(di->batt_data->pc_temp_ocv_lut0,
			di->batt_ocv_temp, di->batt_ocv / PERMILLAGE) / TENTH;
		design_fcc_mah = coul_interpolate_fcc(di, di->batt_ocv_temp);
		/* get the percent of power after the CV is lowered. */
		delta_cv_pc = coul_basp_full_pc_by_voltage(di) / TENTH;
		coul_core_debug("%s, batt %d, delta_cv_pc %d\n",
			__func__, di->batt_index, delta_cv_pc);
		if (!delta_cv_pc)
			delta_cv_pc = PERCENT;
		/* calculate qmax */
		if (di->qmax_end_pc - di->qmax_start_pc != 0) {
			tmp_qmax = (-di->qmax_cc) * PERCENT * di->qmax_end_pc /
				(di->qmax_end_pc - di->qmax_start_pc) /
				delta_cv_pc;
		} else {
			coul_core_err("[%s] batt %d, qmax_end_pc = %d, start_pc = %d, delta_cv_pc = %d\n",
				__func__, di->batt_index, di->qmax_end_pc,
				di->qmax_start_pc, delta_cv_pc);
			return -1;
		}

		/* limit qmax max */
		if (tmp_qmax >
			design_fcc_mah * FCC_UPPER_LIMIT_PERCENT / PERCENT) {
			coul_core_info("[%s] batt %d, qmax = %d, over design\n",
				__func__, di->batt_index, tmp_qmax);
			tmp_qmax = design_fcc_mah *
				FCC_UPPER_LIMIT_PERCENT / PERCENT;
		}
		/* clear qmax refresh flag, prevent continuous calculation */
		di->qmax_refresh_flag = 0;
		di->high_pre_qmax = tmp_qmax;

		coul_core_info("[%s] batt %d, qmax =%d, start_pc =%d, end_pc =%d, delta_cv_pc =%d\n",
			__func__, di->batt_index, di->high_pre_qmax, di->qmax_start_pc,
			di->qmax_end_pc, delta_cv_pc);
		return 0;
	}

	coul_core_info("[%s] batt %d, not update\n", __func__, di->batt_index);
	return -1;
}

/* calculate ocv by 10 history data when AP exist from deep sleep */
void coul_get_ocv_by_vol(struct smartstar_coul_device *di)
{
	int voltage_uv, rm;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_err("%s, NULL\n", __func__);
		return;
	}

	voltage_uv = coul_get_ocv_vol_from_fifo(di);
	if (voltage_uv == 0) {
		coul_core_err("%s, voltage_uv is 0\n", __func__);
		return;
	}

	if (coul_is_in_capacity_dense_area(di, voltage_uv)) {
		coul_core_info("batt %d, do not update OCV %d\n", di->batt_index, voltage_uv);
		return;
	}
	coul_core_info("batt %d, awake from deep sleep, old OCV = %d\n", di->batt_index, di->batt_ocv);
	di->batt_ocv = voltage_uv;
	di->batt_ocv_temp = di->batt_temp;
	di->coul_dev_ops->save_ocv_temp(di->batt_index, (short)di->batt_ocv_temp);
	di->batt_ocv_valid_to_refresh_fcc = 1;
	coul_record_ocv_cali_info(di);
	coul_core_clear_cc_register(di);
	coul_core_clear_coul_time(di);

	di->coul_dev_ops->save_ocv(di->batt_index, voltage_uv, IS_UPDATE_FCC);
	coul_core_info("batt %d, awake from deep sleep, new OCV = %d,fcc_flag=%d\n",
		di->batt_index, di->batt_ocv, di->batt_ocv_valid_to_refresh_fcc);
	di->dbg_ocv_cng_0++;
	if (di->charging_state == CHARGING_STATE_CHARGE_DONE) {
		rm = coul_get_rm(di);
		if (rm < di->batt_limit_fcc) {
			di->batt_limit_fcc = rm * PERCENT / LIMIT_FCC_CAL_RATIO;
			coul_core_info("[%s]batt %d, rm %d, batt_limit_fcc %duah\n",
				__func__,di->batt_index, rm, di->batt_limit_fcc);
			di->is_nv_need_save = 1;
			di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
		}
		/* update qmax */
		if (!get_high_pre_qmax(di)) {
			di->is_nv_need_save = 1;
			di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
		}
	}
	coul_batt_notify(di, BATT_OCV_UPDATE);
}

/* get delta_rc if ratio < 0 */
static int get_delta_rc(struct smartstar_coul_device *di, int ocv,
	int rbatt_tbl, int pc_new, int avg_c)
{
	int pc_new_100;
	int delta_pc, delta_rc_uah;
	int delta_ocv_100 = 0;
	int delta_pc_100 = 0;
	int delta_rc_final = 0;
	int delta_rc_uah_100 = 0;

	delta_pc = pc_new - di->batt_soc_real;
	delta_rc_uah = di->batt_fcc / PERMILLAGE * delta_pc;
	if (di->rbatt_ratio <= 0) {
		coul_core_debug("%s, batt %d, ratio %d\n",
			__func__, di->batt_index, di->rbatt_ratio);
		delta_ocv_100 = -rbatt_tbl * avg_c / PERMILLAGE;
		pc_new_100 = coul_interpolate_pc(di->batt_data->pc_temp_ocv_lut,
			di->batt_temp, ocv - delta_ocv_100);
		delta_pc_100 = pc_new_100 - di->batt_soc_real;
		delta_rc_uah_100 = di->batt_fcc / PERMILLAGE * delta_pc_100;
		delta_rc_final = delta_rc_uah - delta_rc_uah_100;
	}
	coul_core_info("RBATT_ADJ: batt %d, delta_pc=%d.%d delta_rc_uah=%d "
	       "delta_ocv_100=%d delta_pc_100=%d.%d delta_rc_uah_100=%d "
	       "delta_rc_final=%d\n", di->batt_index, delta_pc / TENTH,
	       (int)abs(delta_pc % TENTH), delta_rc_uah, delta_ocv_100,
	       delta_pc_100 / TENTH, (int)abs(delta_pc_100 % TENTH),
	       delta_rc_uah_100, delta_rc_final);

	return delta_rc_final;
}

static void calc_ratio(struct smartstar_coul_device *di,
	int rbatt_tbl, int rbatt_calc)
{
	int ratio;

	if (rbatt_tbl == 0) {
		coul_core_err("%s batt %d, Divisor rbatt_tbl equal zero\n",
			__func__, di->batt_index);
		ratio = 0;
	} else {
		ratio = rbatt_calc * PERCENT / rbatt_tbl;
		coul_core_info("batt %d, rbatt_calc %d, rbatt_tbl %d, old ratio =%d\n",
			di->batt_index, rbatt_calc, rbatt_tbl, ratio);
	}

	di->rbatt_ratio = ratio;
	coul_core_info("RBATT_ADJ: batt %d, ratio = %d\n", di->batt_index, ratio);
}

static int cal_pc_new(struct smartstar_coul_device *di, int rbatt_tbl,
	int rbatt_calc, int ocv, int avg_c)
{
	int pc_new, delta_ocv, ocv_new;

	delta_ocv = (rbatt_calc - rbatt_tbl) * avg_c / PERMILLAGE;
	ocv_new = ocv - delta_ocv;
	pc_new = coul_interpolate_pc(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp, ocv_new);
	coul_core_info("RBATT_ADJ: batt %d delta_ocv=%d\n", di->batt_index, delta_ocv);

	return pc_new;
}

static int cal_delta_rc_final(struct smartstar_coul_device *di,
	int rbatt_tbl, int fcc_uah, struct vcdata *vc, int *soc_new)
{
	int ocv, rbatt_calc, pc_new;
	int delta_rc_final, rc_new_uah;

	ocv = coul_interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp / TENTH, di->batt_soc_real);

	rbatt_calc = (ocv - vc->avg_v) * PERMILLAGE / vc->avg_c;

	coul_update_ocv_cali_rbatt(di, vc->avg_c, rbatt_calc);

	calc_ratio(di, rbatt_tbl, rbatt_calc);

	pc_new = cal_pc_new(di, rbatt_tbl, rbatt_calc, ocv, vc->avg_c);

	delta_rc_final = get_delta_rc(di, ocv, rbatt_tbl, pc_new, vc->avg_c);

	rc_new_uah = di->batt_fcc / PERMILLAGE * pc_new;
	if (fcc_uah == 0) {
		coul_core_err("%s batt %d, Divisor fcc_uah equal zero\n", __func__, di->batt_index);
		*soc_new = 0;
	} else {
		*soc_new = rc_new_uah * PERCENT / fcc_uah;
	}
	*soc_new = coul_bound_soc(*soc_new);
	coul_core_info("RBATT_ADJ: batt %d, soc_new=%d rbat_calc=%d rbat_tbl=%d last_ocv=%d "
		"ocv_temp=%d soc=%d.%d, ocv=%d delta_rc_final=%d\n",
		di->batt_index, *soc_new, rbatt_calc, rbatt_tbl,
		di->batt_ocv, di->batt_ocv_temp, di->batt_soc_real / TENTH,
		di->batt_soc_real % TENTH, ocv, delta_rc_final);

	return delta_rc_final;
}

static int calculate_delta_rc(
	struct smartstar_coul_device *di, int rbatt_tbl, int fcc_uah)
{
	int vbat_uv = 0;
	int ibat_ua = 0;
	int delta_rc_final = 0;
	int soc_new = -EINVAL;
	struct vcdata vc = {0};

	coul_core_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);

	di->coul_dev_ops->get_fifo_avg_data(di->batt_index, &vc);
	/* Fifo is null */
	if (vc.avg_v < NORMAL_VOL_MIN)
		goto out;

	if (vc.avg_c < FIFO_AVG_CUR)
		goto out;

	if (di->coul_dev_ops->get_delta_rc_ignore_flag(di->batt_index)) {
		coul_core_info("batt %d, first ignore delta_rc\n", di->batt_index);
		goto out;
	}

	vc.avg_v += (di->r_pcb / PERMILLAGE) * (vc.avg_c) / PERMILLAGE;
	di->last_fifo_iavg_ma = vc.avg_c;
	delta_rc_final = cal_delta_rc_final(di, rbatt_tbl, fcc_uah, &vc, &soc_new);

out:
	coul_core_info("RBATT_ADJ: batt %d, c=%d u=%d cmin=%d cmax=%d cavg=%d vavg=%d, soc_new=%d\n",
		di->batt_index, ibat_ua, vbat_uv, vc.min_c, vc.max_c, vc.avg_c, vc.avg_v, soc_new);
	di->batt_soc_est = soc_new;

	return delta_rc_final;
}

static int coul_div_round_closest(int a, int b)
{
	if (b == 0) {
		coul_core_err("%s, a = %d, b = %d, error\n", __func__, a, b);
		return 0;
	}

	return DIV_ROUND_CLOSEST(a, b);
}

static int adjust_soc(struct smartstar_coul_device *di, int soc)
{
	int vbat_uv = 0;
	int ibat_ua = 0;
	int delta_soc = 0;
	int zero_vol;
	int n = 0;
	int soc_new = soc;
	int soc_est_avg = 0;

	coul_core_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);

	if ((ibat_ua < -CHARGING_CURRENT_OFFSET) || (di->batt_soc_est < 0))
		goto out;

	di->soc_ests[di->est_i++] = di->batt_soc_est;
	di->est_i = di->est_i % SOC_EST_NUM;

	soc_est_avg = coul_div_round_closest((di->soc_ests[0] + di->soc_ests[1] + di->soc_ests[2]),
		SOC_EST_NUM);
	delta_soc = soc - soc_est_avg;
	if ((soc_est_avg > SOC_TWO) || (soc <= soc_est_avg))
		goto out;

	zero_vol = coul_get_zero_cap_vol(di);
	if ((vbat_uv / UA_PER_MA > zero_vol + ZERO_V_COPM_MV) &&
		(di->batt_temp / TENTH < ZERO_V_ADJ_START_T)) {
		coul_core_debug("%s, batt %d, vbat_uv %d, zero vol %d, temp %d\n",
			__func__, vbat_uv, zero_vol, di->batt_temp);
		goto out;
	}

	n = SOC_THREE - soc_est_avg;
	soc_new = soc - delta_soc * n / SOC_THREE;

	if ((coul_get_temperature_stably(di, USER_COUL) >
		TEMP_OCV_ALLOW_CLEAR * TENTH) &&
		(delta_soc > ABNORMAL_DELTA_SOC)) {
		coul_core_info("batt %d, delta_soc = %d, mark save ocv is invalid\n",
			di->batt_index, delta_soc);
		di->coul_dev_ops->clear_ocv(di->batt_index);
		di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
		di->coul_dev_ops->save_ocv_level(di->batt_index, di->last_ocv_level);
		di->batt_ocv_valid_to_refresh_fcc = 0;
	}
out:
	coul_core_info("batt %d, soc_est_avg=%d delta_soc=%d n=%d\n",
		di->batt_index, soc_est_avg, delta_soc, n);
	soc_new = coul_bound_soc(soc_new);
	return soc_new;
}

static int limit_soc_during_running(struct smartstar_coul_device *di,
	int current_ua, int input_soc, int last_soc)
{
	int output_soc;

	coul_core_debug("%s batt %d +\n", __func__, di->batt_index);

	/* soc can not increase during discharging */
	if (current_ua >= CHARGING_CURRENT_OFFSET) {
		if (last_soc - input_soc >= 1) {
			output_soc = last_soc - 1;
		} else {
			output_soc = last_soc;
		}
		coul_core_info("%s, [dischg] batt %d, current_ua %d, output_soc %d, last_soc %d\n",
			__func__, di->batt_index, current_ua, output_soc, last_soc);
	} else {
		if (input_soc - last_soc >= 1)
			output_soc = last_soc + 1;
		else
			output_soc = last_soc;
		coul_core_info("%s, [chg] batt %d, current_ua %d, output_soc %d, last_soc %d\n",
			__func__, di->batt_index, current_ua, output_soc, last_soc);
	}
	return output_soc;
}

static int limit_soc_after_resume(struct smartstar_coul_device *di,
	int current_ua, int input_soc, int last_soc)
{
	int output_soc = input_soc;

	if ((current_ua >= CHARGING_CURRENT_OFFSET) ||
		(di->charging_state == CHARGING_STATE_CHARGE_STOP)) {
		if (last_soc < input_soc)
			output_soc = last_soc;
	} else {
		if (last_soc > input_soc)
			output_soc = last_soc;
	}
	coul_core_info("%s, batt %d, current_ua: %d, last_soc: %d, input_soc: %d, output_soc %d\n",
		__func__, di->batt_index, current_ua, last_soc, input_soc, output_soc);
	return output_soc;
}

static int limit_soc(struct smartstar_coul_device *di, int input_soc)
{
	int last_soc;
	int current_ua = 0;
	int voltage_uv = 0;
	int output_soc = input_soc;

	last_soc = di->batt_soc;
	coul_core_get_battery_voltage_and_current(di, &current_ua, &voltage_uv);
	/* change <=1% */
	if (di->soc_limit_flag == STATUS_RUNNING)
		output_soc = limit_soc_during_running(di, current_ua, input_soc, last_soc);
	/* exist from sleep */
	else if (di->soc_limit_flag == STATUS_WAKEUP)
		output_soc = limit_soc_after_resume(di, current_ua, input_soc, last_soc);

	/* charge_done, then soc 100% */
	if (di->charging_state == CHARGING_STATE_CHARGE_DONE) {
		coul_core_info("batt %d, pre_chargedone output_soc = %d\n",
			di->batt_index, output_soc);
		output_soc = SOC_FULL;
	}
	if ((di->charging_state == CHARGING_STATE_CHARGE_START) &&
		(voltage_uv / PERMILLAGE > BATTERY_SWITCH_ON_VOLTAGE) &&
		(output_soc == 0) &&
		((current_ua < -CHARGING_CURRENT_OFFSET) ||
		(di->power_on_cnt < POWER_ON_CNT_MAX)))
		output_soc = SOC_ONE;

	di->power_on_cnt++;
	return output_soc;
}

/* limit delta_rc 1% change */
static int adjust_delta_rc(
	struct smartstar_coul_device *di, int delta_rc, int fcc_uah)
{
	int max_changeable_delta_rc = fcc_uah * MAX_DELTA_RC_PC / PERCENT;

	if (abs(di->batt_pre_delta_rc - delta_rc) <= max_changeable_delta_rc) {
		di->batt_pre_delta_rc = delta_rc;
		return delta_rc;
	}
	coul_core_info("%s, batt %d, delta_rc change exceed 1 percents, pre = %d, current = %d\n",
			__func__, di->batt_index, di->batt_pre_delta_rc, delta_rc);
	if (di->batt_pre_delta_rc > delta_rc)
		di->batt_pre_delta_rc -= max_changeable_delta_rc;
	else
		di->batt_pre_delta_rc += max_changeable_delta_rc;
	return di->batt_pre_delta_rc;
}

static void calculate_soc_params(struct smartstar_coul_device *di,
	struct soc_param_data *soc_params)
{
	int soc_rbatt, delt_rc;
	int delta_cc, delta_time, iavg_ua;
	u64 time_now;
	int uuc_update_flag = 0;

	time_now = get_jiffies_64();
	if ((di->soc_limit_flag == STATUS_START) ||
		(time_now - di->uuc_update_time > UUC_UPDATE_INTERVAL)) {
		uuc_update_flag = 1;
		di->uuc_update_time = time_now;
	}

	/* calc fcc by cc and soc change */
	soc_params->fcc_uah = coul_calculate_fcc_uah(di);
	di->batt_fcc = soc_params->fcc_uah;

	/* calculate remainging charge */
	soc_params->remaining_charge_uah = calc_rm_chg_uah(di, soc_params->fcc_uah);
	di->batt_rm = soc_params->remaining_charge_uah;

	/* calculate cc micro_volt_hour */
	di->cc_end_value = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	soc_params->cc_uah = di->cc_end_value;
	di->base_info.cc_uash = di->cc_end_value;

	di->batt_ruc = soc_params->remaining_charge_uah - soc_params->cc_uah;
	di->get_cc_end_time = di->coul_dev_ops->get_coul_time(di->batt_index);
	if (di->batt_fcc == 0)
		di->batt_soc_real = 0;
	else
		di->batt_soc_real = DIV_ROUND_CLOSEST(di->batt_ruc, di->batt_fcc / PERMILLAGE);
	coul_core_info("%s batt %d, SOC real = %d\n", __func__, di->batt_index, di->batt_soc_real);

	soc_rbatt = di->batt_soc_real / TENTH;
	soc_rbatt = (soc_rbatt > 0) ? soc_rbatt : 0;
	soc_params->rbatt = coul_get_rbatt(di, soc_rbatt, di->batt_temp);

#if RBATT_ADJ
	if (uuc_update_flag && di->soc_limit_flag != STATUS_WAKEUP) {
		delt_rc = calculate_delta_rc(di, soc_params->rbatt, soc_params->fcc_uah);
		di->batt_delta_rc = adjust_delta_rc(di, delt_rc, di->batt_fcc);
	}
	soc_params->delta_rc_uah = di->batt_delta_rc;
#endif

	if (di->first_in) {
		iavg_ua = di->coul_dev_ops->get_battery_current_ua(di->batt_index);
		di->first_in = 0;
	} else {
		delta_cc = di->cc_end_value - di->last_cc;
		delta_time = (int)(di->get_cc_end_time - di->last_time);

		if (delta_time > 0)
			iavg_ua = div_s64((s64)delta_cc * SEC_PER_HOUR, delta_time);
		else
			iavg_ua = di->coul_dev_ops->get_battery_current_ua(di->batt_index);

		coul_core_info("batt %d, delta_time=%d, i_ua=%d\n", di->batt_index, delta_time, iavg_ua);
	}
	di->last_cc = di->cc_end_value;
	di->last_time = di->get_cc_end_time;

	coul_calculate_iavg_ma(di, iavg_ua);

	if (uuc_update_flag)
		soc_params->unusable_charge_uah =
			calculate_unusable_charge_uah(di, soc_params->fcc_uah, iavg_ua);
	else
		soc_params->unusable_charge_uah = di->batt_uuc;

	coul_core_info("batt %d, FCC=%duAh, UUC=%duAh, RC=%duAh, CC=%duAh, delta_RC=%duAh, Rbatt=%dmOhm\n",
		di->batt_index, soc_params->fcc_uah, soc_params->unusable_charge_uah,
		soc_params->remaining_charge_uah, soc_params->cc_uah,
		soc_params->delta_rc_uah, soc_params->rbatt);
}

static int current_full_adjust_limit_fcc(struct smartstar_coul_device *di)
{
	if (!di->batt_report_full_fcc_cal) {
		if (!di->batt_report_full_fcc_real)
			di->batt_report_full_fcc_cal = di->batt_fcc * di->soc_at_term / SOC_FULL;
		else
			di->batt_report_full_fcc_cal = min(di->batt_report_full_fcc_real,
				di->batt_fcc * di->soc_at_term / SOC_FULL);
	}

	if (!di->batt_limit_fcc) {
		di->batt_limit_fcc = di->batt_report_full_fcc_cal;
		coul_core_info("[%s]batt %d, pre is 0, batt_limit_fcc = %duah\n",
			__func__, di->batt_index, di->batt_limit_fcc);
	}

	if (di->batt_limit_fcc_begin &&
		(di->charging_state == CHARGING_STATE_CHARGE_START) &&
		(di->charging_begin_soc / TENTH <= CUR_FULL_CHG_BEGIN_SOC_UPPER) &&
			(di->batt_soc_real / TENTH <= CURRENT_FULL_TERM_SOC))
		di->batt_limit_fcc = di->batt_limit_fcc_begin +
			(di->batt_soc_real - di->charging_begin_soc) *
			(di->batt_report_full_fcc_cal - di->batt_limit_fcc_begin) /
			(CURRENT_FULL_TERM_SOC * TENTH - di->charging_begin_soc);

	coul_core_info("[%s] batt %d, limit_fcc %d, full_fcc_cal %d, "
		"limit_fcc_begin %d, soc_real %d, begin_soc %d\n", __func__,
		di->batt_index, di->batt_limit_fcc, di->batt_report_full_fcc_cal,
		di->batt_limit_fcc_begin, di->batt_soc_real, di->charging_begin_soc);

	return di->batt_limit_fcc;
}

static bool adjust_fcc_uah(struct smartstar_coul_device *di,
	struct soc_param_data *soc_params)
{
	bool soc_at_term_flag = true;

	if (di->enable_current_full) {
		soc_params->fcc_uah = current_full_adjust_limit_fcc(di);
		soc_at_term_flag = false;
		coul_core_debug("%s, batt %d, enable_current_full %d, fcc_uah %d\n",
			__func__, di->batt_index, di->enable_current_full, soc_params->fcc_uah);
	} else {
		if (di->batt_limit_fcc &&
			(di->batt_limit_fcc < soc_params->fcc_uah * di->soc_at_term / SOC_FULL)) {
			soc_at_term_flag = false;
			coul_core_info("batt %d, FCC = %duAh term flag= %d\n",
				di->batt_index, soc_params->fcc_uah, soc_at_term_flag);
		} else {
			coul_core_info("batt %d, limit FCC = %duAh, FCC %duAh, term flag= %d\n",
				di->batt_index, di->batt_limit_fcc, soc_params->fcc_uah, soc_at_term_flag);
			return soc_at_term_flag;
		}

		if (di->batt_limit_fcc &&
			(di->batt_limit_fcc < soc_params->fcc_uah)) {
			soc_params->fcc_uah = di->batt_limit_fcc;
			coul_core_info("%s, batt %d, use limit_FCC %duAh\n",
				__func__, di->batt_index, soc_params->fcc_uah);
		}
	}
	/* 100 is to fit the percentage */
	if (soc_params->fcc_uah < INVALID_FCC_UAH)
		soc_params->fcc_uah = di->batt_fcc * di->soc_at_term / SOC_FULL;

	coul_core_debug("%s, batt %d, soc_at_term_flag %d\n",
		__func__, di->batt_index, soc_at_term_flag);
	return soc_at_term_flag;
}

static int calculate_state_of_charge(struct smartstar_coul_device *di)
{
	int rm_usable_chg_uah, soc, soc_no_uuc, soc_before_adjust;
	bool soc_at_term_flag = true;
	struct soc_param_data soc_params = {0};

	coul_core_info("coul_board_type: batt %d board_type = %u, batt_exist = %d\n",
		       di->batt_index, di->is_board_type, di->batt_exist);
	if (!di->batt_exist)
		return 0;

	if (di->batt_index == 0)
		check_chg_done_max_avg_cur_flag(di);

	calculate_soc_params(di, &soc_params);

	di->rbatt = soc_params.rbatt;

	soc_at_term_flag = adjust_fcc_uah(di, &soc_params);
	soc = coul_div_round_closest(
		soc_params.remaining_charge_uah - soc_params.cc_uah,
		soc_params.fcc_uah / PERCENT);

	soc_no_uuc = soc;

	rm_usable_chg_uah = soc_params.remaining_charge_uah -
		soc_params.cc_uah - soc_params.unusable_charge_uah +
		soc_params.delta_rc_uah;
	if (soc_params.fcc_uah - soc_params.unusable_charge_uah <= 0) {
		soc = 0;
	} else {
		if ((di->soc_at_term == SOC_FULL) || !soc_at_term_flag)
			soc = coul_div_round_closest((rm_usable_chg_uah),
				((soc_params.fcc_uah - soc_params.unusable_charge_uah) / PERCENT));
		else
			soc = coul_div_round_closest((rm_usable_chg_uah), ((soc_params.fcc_uah -
				soc_params.unusable_charge_uah) *
				(di->soc_at_term) / SOC_FULL / PERCENT));
	}
	soc = min(soc, SOC_FULL);
	soc_before_adjust = soc;
	soc = adjust_soc(di, soc);
	di->soc_unlimited = soc;
	/* not exiting from ECO Mode capacity can not change more than 1% */
	soc = limit_soc(di, soc);
	coul_judge_eco_leak_uah(di, soc);
	coul_core_info("batt %d, SOC without UUC = %d, SOC before adjust = %d,"
		"SOC before limit = %d, SOC after limit = %d\n",
		di->batt_index, soc_no_uuc, soc_before_adjust, di->soc_unlimited, soc);

	/* default is no battery in sft and udp, so here soc is fixed 20 to prevent low power reset */
	if (di->is_board_type != BAT_BOARD_ASIC) {
		soc = max(soc, DEFAULT_SOC);
		coul_core_info("batt %d, SFT and udp board: adjust Battery Capacity to %d Percents\n",
			di->batt_index, soc);
	}
	di->batt_soc = soc;

	return soc;
}

/* get remain capacity. */
static int coul_get_rm(struct smartstar_coul_device *di)
{
	struct soc_param_data soc_params = {0};
	int rm;

	calculate_soc_params(di, &soc_params);
	rm = soc_params.remaining_charge_uah - soc_params.cc_uah;

	coul_core_info("%s, rm_chg %d, cc %d, rm %duah\n",
		__func__, soc_params.remaining_charge_uah, soc_params.cc_uah, rm);
	return rm;
}

static void basp_record_fcc(struct smartstar_coul_device *di)
{
	int sum = 0;
	int index, i;
	struct ss_coul_nv_info *pinfo = NULL;
#ifdef CONFIG_HUAWEI_DSM
	char buff[DSM_BUFF_SIZE_MAX] = {0};
	int ret;
#endif

	pinfo = &di->nv_info;
	index = pinfo->latest_record_index % MAX_RECORDS_CNT;
	pinfo->real_fcc_record[index] = di->fcc_real_mah;
	pinfo->latest_record_index = index + 1;
	coul_core_info(BASP_TAG"[%s]batt %d, learn times = %d, index = %d\n",
		__func__, di->batt_index, pinfo->latest_record_index, index);
	for (i = 0; i < MAX_RECORDS_CNT; i++)
		sum = sum + pinfo->real_fcc_record[i];
	pinfo->fcc_check_sum_ext = sum;

#ifdef CONFIG_HUAWEI_DSM
	ret = sprintf_s(buff, (size_t)DSM_BUFF_SIZE_MAX,
	"batt: %d, fcc_real_mah:%d, batt_brand:%s, batt_fcc:%d, charging_begin_soc:%d, "
		 "batt_chargecycles:%u, batt_ocv:%d, basp_level:%u\n", di->batt_index,
		 di->fcc_real_mah, di->batt_data->batt_brand, di->batt_fcc / PERMILLAGE,
		 di->charging_begin_soc, di->batt_chargecycles / PERCENT,
		 di->batt_ocv, di->basp_level);
	if (ret < 0) {
		coul_core_err("sprintf_s failed, ret=%d\n", ret);
		return;
	}
	coul_core_dsm_report_ocv_cali_info(di, ERROR_SAFE_PLOICY_LEARN, buff);
#endif
}

static bool check_ocv_valid(struct smartstar_coul_device *di)
{
	coul_core_debug("%s, batt %d, batt_ocv %d\n", __func__, di->batt_index, di->batt_ocv);
	if (((di->batt_ocv > OCV_3200_UV) && (di->batt_ocv < OCV_3670_UV)) ||
		((di->batt_ocv > OCV_3690_UV) && (di->batt_ocv < OCV_3730_UV)) ||
		((di->batt_ocv > OCV_3800_UV) && (di->batt_ocv < OCV_3900_UV)))
		return true;
	return false;
}

static void basp_refresh_fcc(struct smartstar_coul_device *di)
{
	int fcc_uah, new_fcc_uah, delta_fcc_uah, max_delta_fcc_uah;

	if ((di->g_basp_fcc_ls == LS_GOOD) &&
		((di->batt_temp > BASP_FCC_LERAN_TEMP_MIN) &&
		(di->batt_temp < BASP_FCC_LERAN_TEMP_MAX)) &&
		(di->charging_begin_soc / TENTH < MIN_BEGIN_PERCENT_FOR_SAFE) &&
		di->batt_ocv_valid_to_refresh_fcc && check_ocv_valid(di)) {
		new_fcc_uah = calculate_real_fcc_uah(di, &fcc_uah);
		max_delta_fcc_uah = coul_interpolate_fcc(di, di->batt_temp) *
			DELTA_SAFE_FCC_PERCENT * TENTH;
		delta_fcc_uah = abs(new_fcc_uah - fcc_uah);
		if (delta_fcc_uah > max_delta_fcc_uah) {
			/* new_fcc_uah is outside the scope limit it */
			if (new_fcc_uah > fcc_uah)
				new_fcc_uah = (fcc_uah + max_delta_fcc_uah);
			else
				new_fcc_uah = (fcc_uah - max_delta_fcc_uah);
			coul_core_info(BASP_TAG" batt %d, delta_fcc=%d > %d percent of fcc = %d restring it to %d\n",
				       di->batt_index, delta_fcc_uah, DELTA_SAFE_FCC_PERCENT,
				       fcc_uah, new_fcc_uah);
		}
		di->fcc_real_mah = new_fcc_uah / PERMILLAGE;
		coul_core_info(BASP_TAG"batt %d, refresh_fcc, start soc=%d, new fcc=%d\n",
			       di->batt_index, di->charging_begin_soc, di->fcc_real_mah);
		/* record fcc */
		basp_record_fcc(di);
	} else {
		coul_core_err(BASP_TAG"[%s], batt %d, basp_fcc_ls:%d, batt_temp:%d, charging_begin_soc:%d, "
			"ocv_valid:%d, batt_ocv:%d\n", __func__, di->batt_index,
			 di->g_basp_fcc_ls, di->batt_temp,
			di->charging_begin_soc,
			di->batt_ocv_valid_to_refresh_fcc, di->batt_ocv);
	}
	basp_fcc_learn_evt_handler(di, EVT_DONE);
}

static void coul_check_drained_battery_flag(struct smartstar_coul_device *di)
{
	int ret;
	int ibat_ua = 0;
	int vbat_uv = 0;
	int drained_battery_flag = false;
	char buff[DSM_BUFF_SIZE_MAX] = {0};
	static bool g_batt_removed_flag = true;

	/* Waiting for DMD module initialization */
	if(jiffies - di->start_up_time < DELAY_REPORT_DMD_TIME )
		return;

	if (!strstr(saved_command_line, "androidboot.mode=normal") ||
		!strstr(saved_command_line, "androidboot.swtype=normal")) {
		coul_core_err("%s not a normal version, do nothing\n", __func__);
		return;
	}
	if (di->coul_dev_ops->get_drained_battery_flag &&
		di->coul_dev_ops->get_drained_battery_flag(di->batt_index))
		drained_battery_flag = true;

	coul_core_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);
	ret = snprintf_s(buff, sizeof(buff), sizeof(buff) - 1,
		"batt %d, last_cycles:%d,cur_cylces:%u,vol=%d,cur=%d\n", di->batt_index,
		di->g_last_charge_cycles, di->batt_chargecycles, vbat_uv, ibat_ua);
	if (ret < 0) {
		coul_core_err("%s: snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}
	coul_core_info("batt %d, removed_flag=%d, drained_flag=%d,vol=%d,cur=%d\n",
		di->batt_index, di->batt_removed_flag, drained_battery_flag, vbat_uv, ibat_ua);

#ifdef CONFIG_HUAWEI_DSM
	if (drained_battery_flag) {
		coul_core_dsm_report_ocv_cali_info(di, DSM_BATTERY_DRAINED_NO, buff);
	} else if (di->batt_removed_flag && g_batt_removed_flag) {
		coul_core_dsm_report_ocv_cali_info(di, DSM_BATTERY_CHANGED_NO, buff);
		g_batt_removed_flag = false;
	}
#endif

	if (di->coul_dev_ops->clear_drained_battery_flag)
		di->coul_dev_ops->clear_drained_battery_flag(di->batt_index);
}

static void judge_update_ocv_after_charge_done(
	struct smartstar_coul_device *di, int sleep_cc, int sleep_time)
{
	int sleep_current;

	if (sleep_time <= 0) {
		coul_core_info("batt %d, sleep time < 0\n", di->batt_index);
	} else {
		/* uah/s = (mah/1000)/(s/3600) */
		sleep_current = (sleep_cc * SEC_PER_HOUR_DIV_200) /
			(sleep_time * PERMILLAGE_DIV_200);

		if (sleep_current < 0)
			sleep_current = -sleep_current;
		coul_core_info("sleep_current = %d\n", sleep_current);

		if (sleep_current < CHG_DONE_SLEEP_CUR_UPPER) {
			di->last_ocv_level = OCV_LEVEL_0;
			di->coul_dev_ops->save_ocv_level(di->batt_index, di->last_ocv_level);
			coul_get_ocv_by_vol(di);
		}
	}
}

static void cal_soc(struct smartstar_coul_device *di)
{
	int ocv_time_inc, sleep_cc, sleep_time, time_now;

	di_lock();
	/* calc soc */
	di->batt_soc = calculate_state_of_charge(di);

	/* Calibration every 5 minutes */
	coul_calibration_adc(di, 0);

	if (di->charging_state == CHARGING_STATE_CHARGE_DONE) {
		if (di->soc_data.charged_cnt == 0) {
			di->soc_data.last_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
			di->soc_data.last_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
			di->soc_data.charging_done_ocv_enter_time =
				(int)dfx_getcurtime() / NSEC_PER_SEC;
			/* charge done need sleep by CEC, limit SR OCV update time */
			di->charging_stop_time =(int)di->coul_dev_ops->get_coul_time(di->batt_index);
		}

		ocv_time_inc = (int)(dfx_getcurtime() / NSEC_PER_SEC) -
			di->soc_data.charging_done_ocv_enter_time;
		di->soc_data.charged_cnt++;

		if (ocv_time_inc >= CHARGED_OCV_UPDATE_INTERVAL_S) {
			di->soc_data.charging_done_ocv_enter_time =
				(int)(dfx_getcurtime() / NSEC_PER_SEC);
			sleep_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
			sleep_cc = sleep_cc - di->soc_data.last_cc;
			time_now = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
			sleep_time = time_now - di->soc_data.last_time;
			coul_core_info("batt %d, sleep_cc=%d, sleep_time=%d\n",
				di->batt_index, sleep_cc, sleep_time);

			judge_update_ocv_after_charge_done(di, sleep_cc,
				sleep_time);

			di->soc_data.last_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
			di->soc_data.last_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
		}
		/* acr check condition and notify */
		coul_start_soh_check(di);
	} else {
		di->soc_data.charged_cnt = 0;
	}
	di_unlock();
}

void calculate_soc_work(void *data)
{
	int ret;
	short offset_cur_modify_val;
	struct blocking_notifier_head *notifier_list = NULL;
	struct smartstar_coul_device *di = data;

	coul_core_info("%s +\n", __func__);

	if(di == NULL) {
		coul_core_info("%s di is null\n", __func__);
		return;
	}

	if (di->is_nv_need_save) {
		ret = coul_save_nv_info(di);
		if (!ret)
			di->is_nv_need_save = 0;
	}

	coul_check_drained_battery_flag(di);

	coul_core_set_low_vol_int(di, LOW_INT_STATE_RUNNING);
	basp_fcc_learn_evt_handler(di, EVT_PER_CHECK);

	offset_cur_modify_val = di->coul_dev_ops->get_offset_current_mod();
	coul_core_info("batt %d, offset_cur_modify_val:0x%x\n", di->batt_index, offset_cur_modify_val);
	if (offset_cur_modify_val != 0)
		coul_core_err("batt %d, curexception, offset_cur_modify_val:0x%x\n",
			di->batt_index, offset_cur_modify_val);
	offset_cur_modify_val = di->coul_dev_ops->get_offset_vol_mod();
	if (offset_cur_modify_val != 0) {
		di->coul_dev_ops->set_offset_vol_mod();
		coul_core_err("batt %d, curexception, offset_vol_modify_val:0x%x\n",
		di->batt_index, offset_cur_modify_val);
	}

	cal_soc(di);
	/* work faster when capacity <= 3% */
	if (di->batt_soc <= BATTERY_CC_LOW_LEV) {
		coul_core_info("batt %d, SMARTSTAR SHUTDOWN SOC LEVEL\n", di->batt_index);
		get_notifier_list(&notifier_list);
		blocking_notifier_call_chain(notifier_list,
			BATTERY_LOW_SHUTDOWN, NULL);
	}

	coul_core_info("%s -\n", __func__);
}

static void read_temperature_work(struct work_struct *work)
{
	struct smartstar_coul_device *di =
		container_of(work, struct smartstar_coul_device,
			read_temperature_delayed_work.work);

	coul_update_battery_temperature(di, TEMPERATURE_UPDATE_STATUS);
	queue_delayed_work(system_power_efficient_wq, &di->read_temperature_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));
}

/*
 * be called when charge begin, update batt_ocv_valid_to_refresh_fcc flag by ocv update long,
 * because cc err that affects fcc accuracy is larger in long interval of ocv update.
 * ocv time limit(T):  current [200ma,]: NA current [50ma, 100ma]: T<4H
 * current [100ma, 200ma]: T<8H current [,50ma]: NA;
 */
static void fcc_update_limit_by_ocv(struct smartstar_coul_device *di)
{
	int iavg_ma = 0;
	int ocv_update_time, delta_cc_uah;

	if (di->coul_dev_ops == NULL) {
		coul_core_err("%s, di is null\n", __func__);
		return;
	}
	if (!di->batt_ocv_valid_to_refresh_fcc)
		return;
	ocv_update_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	delta_cc_uah = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	/* ocv update begin */
	if (ocv_update_time > 0)
		iavg_ma = (div_s64((s64)delta_cc_uah * SEC_PER_HOUR,
			ocv_update_time)) / UA_PER_MA;

	if (iavg_ma > IAVG_MA_200) {
		coul_core_info("%s:batt %d, current [200ma,]=%dma\n",
			__func__, di->batt_index, iavg_ma);
	} else if (iavg_ma > IAVG_MA_100) {
		if (ocv_update_time > (FOUR_HOUR * SEC_PER_HOUR))
			di->batt_ocv_valid_to_refresh_fcc = 0;
		coul_core_info("%s: batt %d, current [100ma,200]= %d,t=%d\n",
				__func__, di->batt_index, iavg_ma, ocv_update_time);
	} else if (iavg_ma > IAVG_MA_50) {
		if (ocv_update_time > (EIGHT_HOUR * SEC_PER_HOUR))
			di->batt_ocv_valid_to_refresh_fcc = 0;
		coul_core_info("%s: batt %d, current [50ma,100]= %d,t=%d\n",
			__func__, di->batt_index, iavg_ma, ocv_update_time);
	} else {
		coul_core_info("%s: batt %d, current[,50ma]=%dma,NA\n",
			__func__, di->batt_index, iavg_ma);
	}
	coul_core_info("[%s]: batt %d, fcc_flag = %d\n",
		__func__, di->batt_index, di->batt_ocv_valid_to_refresh_fcc);
}

/* be called when charge begin, update charge status, calc soc, begin cc, can't be called in atomic context */
void coul_core_charging_begin(struct smartstar_coul_device *di)
{
	coul_core_info("%s +\n", __func__);
	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return;
	}
	coul_core_info("batt %d, pre charging state is %d\n", di->batt_index, di->charging_state);

	if (di->charging_state == CHARGING_STATE_CHARGE_START)
		return;

	di->charging_state = CHARGING_STATE_CHARGE_START;

	fcc_update_limit_by_ocv(di);

	/* calculate soc again */
	di->batt_soc = calculate_state_of_charge(di);

	/* record soc of charging begin */
	di->charging_begin_soc = di->batt_soc_real;
	di->batt_limit_fcc_begin = di->batt_limit_fcc;
	di->batt_report_full_fcc_cal =
		min(di->batt_fcc * di->soc_at_term / SOC_FULL,
			di->batt_report_full_fcc_real);
	basp_fcc_learn_evt_handler(di, EVT_START);

	/* record cc value */
	di->charging_begin_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	di->charging_begin_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);

	coul_core_info("%s -\n", __func__);
	coul_core_info("batt %d, batt_soc=%d, charging_begin_soc=%d,"
		"charging_begin_cc=%d,batt_limit_fcc_begin =%d, charging_begin_time %d\n",
		di->batt_index, di->batt_soc, di->charging_begin_soc,
		di->charging_begin_cc, di->batt_limit_fcc_begin, di->charging_begin_time);
}

/*
 * be called when charge stop, update charge status, update chargecycles
 * calc soc, cc, rm and set low vol reg. can't be called in atomic context
 */
void coul_core_charging_stop(struct smartstar_coul_device *di)
{
	int rm, cc, fcc_101;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_err("%s NULL point\n", __func__);
		return;
	}
	fcc_101 = di->batt_fcc * LIMIT_FCC_CAL_RATIO / SOC_FULL;
	if (di->charging_state == CHARGING_STATE_CHARGE_UNKNOW)
		return;

	di->coul_dev_ops->irq_enable(di->batt_index);
	di->batt_soc = calculate_state_of_charge(di);

	coul_core_info("%s batt %d, chg state %d, \n",
		__func__, di->batt_index, di->charging_state);

	if (di->charging_state == CHARGING_STATE_CHARGE_START) {
		update_chargecycles(di);
		di->is_nv_need_save = 1;
	}

	di->batt_ocv_valid_to_refresh_fcc = 0;
	cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);

	rm = di->batt_rm - cc;
	/* adjust rm */
	if (rm > fcc_101) {
		cc = cc + (rm - fcc_101);
		di->coul_dev_ops->save_cc_uah(di->batt_index, cc);
		di->batt_limit_fcc = 0;
		di->is_nv_need_save = 1;
		di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
		coul_core_info("%s batt %d, batt_limit_fcc %duah\n",
		__func__, di->batt_index, di->batt_limit_fcc);
	} else if ((di->batt_soc == SOC_FULL) &&
		(di->batt_soc_real > SOC_TO_CAL_LIMIT_FCC)) {
		di->batt_limit_fcc = rm * SOC_FULL / LIMIT_FCC_CAL_RATIO;
		di->is_nv_need_save = 1;
		di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
		coul_core_info("%s batt %d,batt_soc is full, batt_limit_fcc %duah\n",
			__func__, di->batt_index, di->batt_limit_fcc);
	}

	di->charging_state = CHARGING_STATE_CHARGE_STOP;
	di->charging_stop_soc = di->batt_soc_real;
	di->charging_stop_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	coul_core_set_low_vol_int(di, LOW_INT_STATE_RUNNING);
}

/* calc fcc by cc_change /soc_change. */
static int calculate_real_fcc_uah(struct smartstar_coul_device *di,
	int *ret_fcc_uah)
{
	struct soc_param_data soc_params = {0};
	int real_fcc_uah, terminate_soc_real;

	terminate_soc_real = di->batt_soc_real;
	calculate_soc_params(di, &soc_params);
	*ret_fcc_uah = soc_params.fcc_uah;
	if (terminate_soc_real == di->charging_begin_soc)
		real_fcc_uah = *ret_fcc_uah;
	else
		real_fcc_uah = (-(soc_params.cc_uah - di->charging_begin_cc)) /
			(terminate_soc_real - di->charging_begin_soc) *
			terminate_soc_real;

	coul_core_info("batt %d, real_fcc=%d, RC=%d CC=%d fcc=%d charging_begin_soc=%d.%d\n",
		di->batt_index, real_fcc_uah, soc_params.remaining_charge_uah,
		soc_params.cc_uah, soc_params.fcc_uah, di->charging_begin_soc / TENTH,
		di->charging_begin_soc);

	return real_fcc_uah;
}

/* fcc self_study, establish a temp_fcc lookup table */
static void readjust_fcc_table(struct smartstar_coul_device *di)
{
	struct single_row_lut *temp = NULL;
	struct single_row_lut *now = NULL;
	int i, fcc, ratio;

	if (!di->batt_data->fcc_temp_lut) {
		coul_core_err("%s, batt %d, The static fcc lut table is NULL\n",
			__func__, di->batt_index);
		return;
	}
	if (di->adjusted_fcc_temp_lut == NULL) {
		temp = &di->adjusted_fcc_temp_lut_tbl1;
		now = di->batt_data->fcc_temp_lut;
		coul_core_debug("%s, batt %d, lut is null, use adjusted_fcc_temp_lut_tbl1\n",
			__func__, di->batt_index);
	} else if (di->adjusted_fcc_temp_lut == &di->adjusted_fcc_temp_lut_tbl1) {
		temp = &di->adjusted_fcc_temp_lut_tbl2;
		now = di->batt_data->fcc_temp_lut;
		coul_core_debug("%s, batt %d, use adjusted_fcc_temp_lut_tbl2\n",
			__func__, di->batt_index);
	} else {
		temp = &di->adjusted_fcc_temp_lut_tbl1;
		now = di->batt_data->fcc_temp_lut;
		coul_core_debug("%s, batt %d, use adjusted_fcc_temp_lut_tbl1\n",
			__func__, di->batt_index);
	}

	fcc = coul_interpolate_fcc(di, di->batt_temp);
	temp->cols = now->cols;
	for (i = 0; i < now->cols; i++) {
		temp->x[i] = now->x[i];
		ratio = (int)div_u64(((u64)(now->y[i]) * PERMILLAGE), fcc);
		temp->y[i] = ratio * di->fcc_real_mah;
		temp->y[i] /= PERMILLAGE;
		coul_core_info("%s, batt %d, temp=%d, staticfcc=%d, adjfcc=%d, ratio=%d\n",
			__func__, di->batt_index, temp->x[i], now->y[i], temp->y[i], ratio);
	}
	di->adjusted_fcc_temp_lut = temp;
}

static bool is_fcc_ready_to_refresh(struct smartstar_coul_device *di,
	int charging_iavg_ma)
{
	bool ret = FALSE;

	if ((di->charging_begin_soc / TENTH < MIN_BEGIN_PERCENT_FOR_LEARNING) &&
		((di->batt_temp > FCC_UPDATE_TEMP_MIN) &&
		(di->batt_temp < FCC_UPDATE_TEMP_MAX)) &&
		(di->batt_ocv_temp > FCC_UPDATE_TEMP_MIN) &&
		(charging_iavg_ma < -FCC_UPDATE_CHARGING_CURR_MIN_MA) &&
		check_ocv_valid(di))
		ret = TRUE;

	coul_core_info("%s, batt %d, ret %d\n", __func__, di->batt_index, ret);
	return ret;
}

static int get_fcc_after_cv_adjust(struct smartstar_coul_device *di, int fcc_mah)
{
	int delta_cv_pc;
	int real_fcc_mah = fcc_mah;

	coul_core_debug("%s, batt %d +\n", __func__, di->batt_index);

	if (di->nondc_volt_dec > BASP_FATAL_VDEC_TH) {
		delta_cv_pc = coul_basp_full_pc_by_voltage(di) / TENTH;
		if (!delta_cv_pc)
			delta_cv_pc = PERCENT;
		real_fcc_mah = fcc_mah * PERCENT / delta_cv_pc;
		coul_core_info("batt %d, basp fatal dec adjust fcc %d, delta_cv_pc %d\n",
			di->batt_index, fcc_mah, delta_cv_pc);
	}
	return real_fcc_mah;
}

static int calc_charging_iavg_ma(struct smartstar_coul_device *di)
{
	int delta_cc_uah, charging_time, charging_iavg_ma;

	charging_time =
		(int)di->coul_dev_ops->get_coul_time(di->batt_index) - di->charging_begin_time;
	delta_cc_uah =
		di->coul_dev_ops->calculate_cc_uah(di->batt_index) - di->charging_begin_cc;
	if (charging_time > 0)
		charging_iavg_ma =
			(div_s64((s64)delta_cc_uah * SEC_PER_HOUR,
				charging_time)) / UA_PER_MA;
	else
		charging_iavg_ma = 0;
	coul_core_info("[%s]: batt %d, charging_time = %ds,delta_cc_uah = %duah,charging_iavg_ma = %dma\n",
		__func__, di->batt_index, charging_time, delta_cc_uah, charging_iavg_ma);

	return charging_iavg_ma;
}

static void limit_delta_fcc_uah(struct smartstar_coul_device *di,
	int new_fcc_uah, int delta_fcc_uah, int design_fcc_mah, int fcc_uah)
{
	int max_delta_fcc_uah;
#ifdef CONFIG_HUAWEI_DSM
	char buff[DSM_BUFF_SIZE_MAX] = {0};
#endif

	coul_core_debug("%s, batt %d +\n", __func__, di->batt_index);

	if (delta_fcc_uah < 0) {
		max_delta_fcc_uah = design_fcc_mah * DELTA_MAX_DECR_FCC_PERCENT * TENTH;
		delta_fcc_uah = -delta_fcc_uah;
	} else {
		max_delta_fcc_uah = design_fcc_mah * DELTA_MAX_INCR_FCC_PERCENT * TENTH;
	}
	if (delta_fcc_uah > max_delta_fcc_uah) {
		/* new_fcc_uah is outside the scope limit it */
		if (new_fcc_uah > fcc_uah)
			new_fcc_uah = (fcc_uah + max_delta_fcc_uah);
		else
			new_fcc_uah = (fcc_uah - max_delta_fcc_uah);
		coul_core_info("batt %d, delta_fcc=%d > %d percent of fcc=%d restring it to %d\n",
			di->batt_index, delta_fcc_uah, max_delta_fcc_uah, fcc_uah, new_fcc_uah);
#ifdef CONFIG_HUAWEI_DSM
		/*
		 * dmd report: current information --
		 * old_fcc,new_fcc, delta_fcc, charging_begin_soc,
		 * charging_begin_cc, charing_end_cc, temp, basplevel
		 */
		sprintf_s(buff, (size_t)DSM_BUFF_SIZE_MAX, " [refresh fcc warning] batt %d, old_fcc:%dmAh, "
			"new_fcc:%dmAh, delta_fcc:%dmAh, charging_beging_soc:%d, "
			"charging_begin_cc:%dmAh, charging_end_cc:%dmAh, temp:%d, basplevel:%u",
			di->batt_index, di->fcc_real_mah, new_fcc_uah / UA_PER_MA,
			delta_fcc_uah / UA_PER_MA, di->charging_begin_soc,
			di->charging_begin_cc / UA_PER_MA,
			di->coul_dev_ops->calculate_cc_uah(di->batt_index) / UA_PER_MA,
			di->batt_temp, di->basp_level);
		coul_core_dsm_report_ocv_cali_info(di, ERROR_REFRESH_FCC_OUTSIDE, buff);
#endif
	}
	di->fcc_real_mah = new_fcc_uah / PERMILLAGE;
}

/*
 * fcc self_study, check learning condition when charge done and
 * call readjust_fcc_table to establish a temp_fcc lookup table
 */
static void refresh_fcc(struct smartstar_coul_device *di)
{
	int charging_iavg_ma;
	int fcc_uah = 0;
	int new_fcc_uah, delta_fcc_uah, design_fcc_mah;

	coul_core_debug("%s, batt %d +\n", __func__, di->batt_index);

	charging_iavg_ma = calc_charging_iavg_ma(di);
	if (di->batt_ocv_valid_to_refresh_fcc &&
		(is_fcc_ready_to_refresh(di, charging_iavg_ma))) {
		new_fcc_uah = calculate_real_fcc_uah(di, &fcc_uah);
		design_fcc_mah = coul_interpolate_fcc(di, di->batt_temp);
		delta_fcc_uah = new_fcc_uah - fcc_uah;
		limit_delta_fcc_uah(di, new_fcc_uah, delta_fcc_uah, design_fcc_mah, fcc_uah);
		di->fcc_real_mah = get_fcc_after_cv_adjust(di, di->fcc_real_mah);
		/* limit max fcc, consider boardd 1.05 * fcc gain */
		if (di->fcc_real_mah >
			design_fcc_mah * FCC_UPPER_LIMIT_PERCENT / SOC_FULL)
			di->fcc_real_mah = design_fcc_mah *
				FCC_UPPER_LIMIT_PERCENT / SOC_FULL;
		coul_core_info("refresh_fcc, batt %d, start soc=%d, new fcc=%d\n",
			di->batt_index, di->charging_begin_soc, di->fcc_real_mah);
		/* update the temp_fcc lookup table */
		readjust_fcc_table(di);

		/* high precision qmax refresh check */
		is_high_precision_qmax_ready_to_refresh(di);
	}
	coul_core_debug("%s, batt %d -\n", __func__, di->batt_index);
}

/*
 * be called when charge finish, update charge status, chargecycles  calc soc(100%), OCV.
 * can't be called in atomic context, refresh_fcc if can be
 */
void coul_core_charging_done(struct smartstar_coul_device *di)
{
	int rm, ocv_update_hour;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;
	if (di->charging_state != CHARGING_STATE_CHARGE_START) {
		coul_core_info("charging_done, batt %d, pre charge state is %d\n",
			di->batt_index, di->charging_state);
		return;
	}
	/* limt fcc refresh by ocv update time */
	ocv_update_hour = (int)(di->coul_dev_ops->get_coul_time(di->batt_index) / SEC_PER_HOUR);
	if (ocv_update_hour >= FCC_UPDATE_MAX_OCV_INTERVAL)
		di->batt_ocv_valid_to_refresh_fcc = 0;
	coul_core_info("batt %d, done fcc_flag = %d,ocv_time = %d hour\n",
		di->batt_index, di->batt_ocv_valid_to_refresh_fcc, ocv_update_hour);
	/* enable coul irq */
	di->coul_dev_ops->irq_enable(di->batt_index);
	basp_refresh_fcc(di);
	refresh_fcc(di);
	rm = coul_get_rm(di);
	di->batt_limit_fcc = rm * SOC_FULL / LIMIT_FCC_CAL_RATIO;
	coul_core_info("coul_core_charging_done, batt %d, adjust soc from %d to 100, limit fcc %d\n",
		di->batt_index, di->batt_soc, di->batt_limit_fcc);

	di->batt_soc = SOC_FULL;

	coul_get_ocv_by_fcc(di);
	di->batt_fcc = coul_calculate_fcc_uah(di);
	coul_core_info("coul_core_charging_done, batt %d, di->batt_fcc %duah\n",
		di->batt_index, di->batt_fcc);
	update_chargecycles(di);
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);

	/* NV save in shutdown charging */
	if (coul_core_get_pd_charge_flag() == 1) {
		if (di->is_nv_need_save) {
			if (!coul_save_nv_info(di))
				di->is_nv_need_save = 0;
		}
	}
	di->charging_state = CHARGING_STATE_CHARGE_DONE;

	if (di->nondc_volt_dec > BASP_FATAL_VDEC_TH) {
		/* wait for ibat 1.5s */
		msleep(SLEEP_1500_MS);
		coul_force_ocv_update(di);
	}

	coul_core_info("new charging cycles = %u%%\n", di->batt_chargecycles);
}


/*
 * respond the fault events from coul driver
 * Parameters: fault_nb:fault notifier_block, event:fault event name, data:unused
 * return value: NOTIFY_OK-success or others
 */
static int coul_fault_notifier_call(struct notifier_block *fault_nb,
	unsigned long event, void *data)
{
	struct smartstar_coul_device *di =
		container_of(fault_nb, struct smartstar_coul_device, fault_nb);

	coul_core_debug("%s batt %d, event %d +\n", __func__, di->batt_index, event);

	di->coul_fault = (enum coul_fault_type)event;
	queue_work(system_power_efficient_wq, &di->fault_work);

	return NOTIFY_OK;
}

static int di_initial(struct smartstar_coul_device *di)
{
	int i;

	mutex_init(&di->soc_mutex);
	di->power_wake_lock = wakeup_source_register(NULL, "power wakelock");
	if (!di->power_wake_lock) {
		coul_core_err("%s register wake lock failed!\n", __func__);
		return -1;
	}
	coul_core_set_low_vol_int(di, LOW_INT_STATE_RUNNING);

	/* set di element with default data */
	di->prev_pc_unusable = -EINVAL;
	di->batt_pre_delta_rc = 0;
	di->last_iavg_ma = IMPOSSIBLE_IAVG;
	di->batt_under_voltage_flag = 0;

	/* read nv info */
	coul_get_initial_params(di);
	di->g_last_charge_cycles = (int)di->batt_chargecycles;
	di->is_nv_read = di->coul_dev_ops->get_nv_read_flag(di->batt_index);
	di->is_nv_need_save = 0;
	di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_SUCCESS);
	di->sr_resume_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	di->resume_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	di->g_sr_cur_state = SR_DEVICE_WAKEUP;
	di->batt_temp = coul_battery_get_temperature_tenth_degree(di, USER_COUL);
	coul_get_battery_id_voltage(di);

	for(i = 0; i < SOC_EST_NUM; i++)
		di->soc_ests[i] = SOC_FULL;
	di->est_i = 0;

	di->first_in = 1;
	di->wakeup_first_in = 1;
	di->g_basp_fcc_ls = LS_UNKNOWN;
	di->start_up_time = jiffies;
	di->cali_adc_disable = 0;
	di->eis_freq_ntf_cnt = 0;
	di->uuc_update_time = get_jiffies_64();
	return 0;
}

static int coul_work_register(struct smartstar_coul_device *di)
{
	int retval;

	coul_core_lock_init(di);
	/* Init soc calc work */
	INIT_WORK(&di->fault_work, coul_core_fault_work);
	INIT_DELAYED_WORK(&di->read_temperature_delayed_work,
		read_temperature_work);
	di->work_init_flag = 1;
	di->fault_nb.notifier_call = coul_fault_notifier_call;
	retval = register_coul_fault_notifier(&di->fault_nb, di->batt_index);

	return retval;
}

static void get_initial_soc(struct smartstar_coul_device *di)
{
	/* get the first soc value */
	di_lock();
	di->soc_limit_flag = STATUS_START;
	di->soc_monitor_flag = STATUS_START;
	di->batt_soc = calculate_state_of_charge(di);
	di->soc_limit_flag = STATUS_RUNNING;
	di->charging_stop_soc = di->batt_soc_real;
	di->charging_stop_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	di_unlock();
}

int coul_core_register(struct coul_device_ops *dev_ops, const struct device_node *cfg)
{
	struct smartstar_coul_device *di = NULL;
	struct coul_merge_drv_ops *coul_ops = NULL;
	int retval;

	di = (struct smartstar_coul_device *)kzalloc(sizeof(*di), GFP_KERNEL);
	if (di == NULL) {
		coul_core_err("%s failed to alloc di\n", __func__);
		return -ENOMEM;
	}

	di->coul_dev_ops = dev_ops;
	retval = coul_probe_check_coul_dev_ops(di);
	if (retval)
		goto coul_failed;

	/* get dts data */
	coul_core_get_battery_dts(di, cfg);
	di->low_vol_filter_cnt = LOW_INT_VOL_COUNT;

	retval = di_initial(di);
	if (retval)
		goto coul_failed;

	retval = coul_check_batt_data(di);
	if (retval == -EINVAL) {
		coul_core_info("%s, coul_no_battery\n", __func__);
		goto coul_no_battery;
	}
	if (retval == -1) {
		coul_core_err("%s, coul_unset_pdev\n", __func__);
		goto coul_failed;
	}

	retval = coul_work_register(di);
	if (retval < 0) {
		coul_core_err("coul_fault_register_notifier failed\n");
		goto coul_unregister_nb;
	}

	coul_core_info("%s, batt %d, battery temperature is %d.%d\n",
		__func__, di->batt_index, di->batt_temp / TENTH, di->batt_temp % TENTH);

	/* calculate init soc */
	coul_core_get_initial_ocv(di);

	/* battery moved, clear battery data, then update basp level */
	coul_clear_moved_battery_data(di, cfg);
	di->qmax = coul_core_get_qmax(di);
	coul_core_info("%s, batt %d, qmax is %dmAh\n",
		__func__, di->batt_index, di->qmax / UA_PER_MA);

	get_initial_soc(di);

	queue_delayed_work(system_power_efficient_wq,
		&di->read_temperature_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));

coul_no_battery:
	coul_ops = get_coul_interface_ops();
	di->ops = coul_ops;
	retval = coul_merge_drv_coul_ops_register(coul_ops, di, di->batt_index, di->batt_name);
	if (retval) {
		coul_core_err("failed to register coul ops\n");
		goto coul_cancel_work;
	}

	coul_calibration_adc(di, 0);

	/* on boot multi steps will update charge cycles, so notify at end of function */
	/* The cycles of high-side battery and low-side battery must be similar. */
	if (di->batt_index == BATT_0)
		call_coul_blocking_notifiers(BATT_EEPROM_CYC, &di->batt_chargecycles);
	coul_core_info("batt %d, name %s, %s succ\n", di->batt_index, di->batt_name, __func__);
	return 0;

coul_cancel_work:
	di->ops = NULL;
	if (di->work_init_flag == 1)
		cancel_delayed_work(&di->read_temperature_delayed_work);

coul_unregister_nb:
	unregister_coul_fault_notifier(&di->fault_nb, di->batt_index);
coul_failed:
	coul_core_err("%s failed\n", __func__);
	wakeup_source_unregister(di->power_wake_lock);
	di->power_wake_lock = NULL;
	kfree(di);
	di = NULL;
	return retval;
}

#ifdef CONFIG_PM
int coul_core_suspend(void *data)
{
	int current_sec, wakeup_time;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_info("%s: di is null\n", __func__);
		return 0;
	}

	current_sec = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	di_lock();
	di->suspend_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);

	di->suspend_time = (unsigned int)current_sec;
	di->sr_suspend_time = current_sec;

	coul_core_info("%s: batt %d suspend_cc %d, current_sec %d\n",
		__func__, di->batt_index, di->suspend_cc, current_sec);

	wakeup_time = current_sec - di->sr_resume_time;
	coul_record_wakeup_info(di, wakeup_time);
	di->g_sr_cur_state = SR_DEVICE_SLEEP;
	di_unlock();
	if (di->work_init_flag == 1)
		cancel_delayed_work(&di->read_temperature_delayed_work);

	coul_core_set_low_vol_int(di, LOW_INT_STATE_SLEEP);
	di->coul_dev_ops->enter_eco(di->batt_index);
	return 0;
}

static void report_battery_adjust(struct smartstar_coul_device *di, int delta_ocv, int delta_soc,
	int delta_uah, int sleep_cc)
{
	coul_core_debug("%s:batt %d delta_ocv=%d delta_soc=%d delta_uah=%d sleep_uah=%d",
			__func__, di->batt_index, delta_ocv, delta_soc, delta_uah, sleep_cc);
#ifdef CONFIG_POWER_DUBAI
	if (delta_ocv != 0)
		HWDUBAI_LOGE("DUBAI_TAG_BATTERY_ADJUST", "delta_soc=%d delta_uah=%d sleep_uah=%d",
			delta_soc, delta_uah, sleep_cc);
#endif
}

static void queue_work_after_coul_core_resume(struct smartstar_coul_device *di)
{
	if (di->batt_exist && (di->work_init_flag == 1)) {
		queue_delayed_work(system_power_efficient_wq,
			&di->read_temperature_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));

		/* start soh check if charging done */
		coul_start_soh_check(di);
	}
}

#ifdef CONFIG_HUAWEI_DSM
static void resume_soc_jump_report(struct smartstar_coul_device *di, int old_soc)
{
	char buff[DSM_BUFF_SIZE_MAX] = {0};

	/* dmd report */
	if (abs(old_soc - di->batt_soc) >= SOC_JUMP_MAX) {
		/* current information --- fcc, temp, old_soc, new_soc */
		sprintf_s(buff, (size_t)DSM_BUFF_SIZE_MAX,
			"[resume jump]batt %d, fcc:%d, temp:%d, old_soc:%d, new_soc:%d",
			 di->batt_index, di->batt_fcc, di->batt_temp, old_soc, di->batt_soc);
		coul_core_dsm_report_ocv_cali_info(di, ERROR_RESUME_SOC_JUMP, buff);
	}
}
#endif

/* called when coul wakeup from deep sleep */
int coul_core_resume(void *data)
{
	int sr_sleep_time, old_soc, sleep_cc;
	int pre_ocv, delta_soc;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_info("%s: di is null\n", __func__);
		return 0;
	}
	coul_core_info("%s:batt %d +\n", __func__, di->batt_index);
	pre_ocv = di->batt_ocv;
	di_lock();
	di->sr_resume_time = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	coul_update_battery_temperature(di, TEMPERATURE_INIT_STATUS);
	sr_sleep_time = di->sr_resume_time - di->sr_suspend_time;
	sleep_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index) - di->suspend_cc; /* sleep uah */

	coul_core_set_low_vol_int(di, LOW_INT_STATE_RUNNING);

	di->g_sr_cur_state = SR_DEVICE_WAKEUP;
	coul_record_sleep_info(di, sr_sleep_time);
	coul_update_ocv_after_resume(di);

	di->coul_dev_ops->exit_eco(di->batt_index);
	di->soc_limit_flag = STATUS_WAKEUP;
	di->soc_monitor_flag = STATUS_WAKEUP;
	old_soc = di->batt_soc;
	di->batt_soc = calculate_state_of_charge(di);

#ifdef CONFIG_HUAWEI_DSM
	resume_soc_jump_report(di, old_soc);
#endif
	di->soc_limit_flag = STATUS_RUNNING;
	di->resume_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	delta_soc = old_soc - di->batt_soc;
	report_battery_adjust(di, pre_ocv - di->batt_ocv, delta_soc,
		delta_soc * di->batt_fcc / PERCENT, sleep_cc);

	di_unlock();

	queue_work_after_coul_core_resume(di);

	coul_core_info("%s:batt %d -\n", __func__, di->batt_index);
	return 0;
}
#endif

void coul_core_shutdown(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	coul_core_err("%s +\n", __func__);
	if (di == NULL) {
		coul_core_err("[coul_shutdown]:di is NULL\n");
		return;
	}

	if (di->coul_dev_ops->set_bootocv_sample != NULL) {
		if (coul_could_cc_update_ocv(di) == TRUE)
			di->coul_dev_ops->set_bootocv_sample(di->batt_index, 1);
		else
			di->coul_dev_ops->set_bootocv_sample(di->batt_index, 0);
	}
	if (di->work_init_flag == 1)
		cancel_delayed_work(&di->read_temperature_delayed_work);

	coul_core_err("%s -\n", __func__);
}
