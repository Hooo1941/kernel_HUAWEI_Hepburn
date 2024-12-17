// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charger_lvc.c
 *
 * direct charger with lvc (load switch) driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_debug.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_work.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_sysfs.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG direct_charge_lvc
HWLOG_REGIST();

static struct direct_charge_device *g_lvc_di;

static int lvc_set_disable_func(unsigned int val)
{
	int work_mode = direct_charge_get_working_mode();

	if (!g_lvc_di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	g_lvc_di->force_disable = val;
	hwlog_info("working_mode = %d, work_mode = %d\n", g_lvc_di->working_mode, work_mode);
	if (val && (work_mode != UNDEFINED_MODE) &&
		(g_lvc_di->working_mode == work_mode))
		wired_connect_send_icon_uevent(ICON_TYPE_NORMAL);
	hwlog_info("set force_disable = %u\n", val);
	return 0;
}

static int lvc_get_disable_func(unsigned int *val)
{
	if (!g_lvc_di || !val) {
		hwlog_err("di or val is null\n");
		return -ENODEV;
	}

	*val = g_lvc_di->force_disable;
	return 0;
}

static int lvc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -EPERM;

	ret = lvc_set_disable_flags((val ?
		DC_CLEAR_DISABLE_FLAGS : DC_SET_DISABLE_FLAGS),
		DC_DISABLE_SYS_NODE);
	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);
	return ret;
}

static int lvc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	*val = di->sysfs_enable_charger;
	return 0;
}

static int lvc_set_iin_limit(unsigned int val)
{
	return dc_sysfs_set_iin_thermal_array(LVC_MODE, DC_SINGLE_CHANNEL, (int)val);
}

static int lvc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EPERM;
	}

	dc_sysfs_get_iin_thermal_limit(LVC_MODE, di->cur_mode, val);

	return 0;
}

static int lvc_set_iin_thermal(unsigned int index, unsigned int iin_thermal_value)
{
	if (index != DC_SINGLE_CHANNEL) {
		hwlog_err("error index: %u, out of boundary\n", index);
		return -EPERM;
	}
	return lvc_set_iin_limit(iin_thermal_value);
}

static int lvc_set_ichg_ratio(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%u\n", val);
	return 0;
}

static int lvc_set_vterm_dec(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	di->vterm_dec = val;
	hwlog_info("set vterm_dec=%u\n", val);
	return 0;
}

static int lvc_get_rt_test_prot_type(void)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	return di->prot_type;
}

static int lvc_get_hota_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->hota_iin_limit;
	return 0;
}

static int lvc_get_startup_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->startup_iin_limit;
	return 0;
}

static int lvc_get_ibus(int *ibus)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || di->ls_ibus < 0 || !ibus) {
		hwlog_err("di or ibus is null, or ibus value is err\n");
		return -EPERM;
	}

	*ibus = di->ls_ibus;
	return 0;
}

static int lvc_get_vbus(int *vbus)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || di->ls_vbus < 0 || !vbus)
		return -EPERM;

	*vbus = di->ls_vbus;
	return 0;
}

static struct power_if_ops lvc_if_ops = {
	.set_enable_charger = lvc_set_enable_charger,
	.get_enable_charger = lvc_get_enable_charger,
	.set_iin_limit = lvc_set_iin_limit,
	.get_iin_limit = lvc_get_iin_limit,
	.set_iin_thermal = lvc_set_iin_thermal,
	.set_iin_thermal_all = lvc_set_iin_limit,
	.set_ichg_ratio = lvc_set_ichg_ratio,
	.set_vterm_dec = lvc_set_vterm_dec,
	.get_rt_test_prot_type = lvc_get_rt_test_prot_type,
	.get_hota_iin_limit = lvc_get_hota_iin_limit,
	.get_startup_iin_limit = lvc_get_startup_iin_limit,
	.get_ibus = lvc_get_ibus,
	.get_vbus = lvc_get_vbus,
	.set_disable_func = lvc_set_disable_func,
	.get_disable_func = lvc_get_disable_func,
	.type_name = "lvc",
};

int lvc_get_di(struct direct_charge_device **di)
{
	if (!g_lvc_di || !di) {
		hwlog_err("g_lvc_di or di is null\n");
		return -EPERM;
	}

	*di = g_lvc_di;

	return 0;
}

int lvc_set_disable_flags(int val, int type)
{
	int i;
	unsigned int disable;
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (type < DC_DISABLE_BEGIN || type >= DC_DISABLE_END) {
		hwlog_err("invalid disable_type=%d\n", type);
		return -EPERM;
	}

	disable = 0;
	di->sysfs_disable_charger[type] = val;
	for (i = 0; i < DC_DISABLE_END; i++)
		disable |= di->sysfs_disable_charger[i];
	di->sysfs_enable_charger = !disable;

	hwlog_info("set_disable_flag val=%d, type=%d, disable=%u\n",
		val, type, disable);
	return 0;
}

static void lvc_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;
	int bat_capacity;

	if (!work)
		return;

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di)
		return;

	if (di->fault_data)
		snprintf(reg_info, sizeof(reg_info),
			"lvc charge_fault=%d, addr=0x%x, event1=0x%x, event2=0x%x\n",
			di->charge_fault, di->fault_data->addr,
			di->fault_data->event1, di->fault_data->event2);
	else
		snprintf(reg_info, sizeof(reg_info),
			"lvc charge_fault=%d, addr=0x0, event1=0x0, event2=0x0\n",
			di->charge_fault);
	dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_HAPPEN_LVC_FAULT, reg_info);

	switch (di->charge_fault) {
	case POWER_NE_DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");
		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			POWER_DSM_DIRECT_CHARGE_VBUS_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_REVERSE_OCP:
		bat_capacity = power_platform_get_battery_ui_capacity();
		if (bat_capacity < BATTERY_CAPACITY_HIGH_TH)
			di->reverse_ocp_cnt++;

		hwlog_err("reverse ocp happened, capacity=%d, ocp_cnt=%d\n",
			bat_capacity, di->reverse_ocp_cnt);

		if (di->reverse_ocp_cnt >= REVERSE_OCP_CNT) {
			di->reverse_ocp_cnt = REVERSE_OCP_CNT;
			snprintf(buf, sizeof(buf), "reverse ocp happened\n");
			strncat(buf, reg_info, strlen(reg_info));
			power_dsm_report_dmd(POWER_DSM_BATTERY,
				POWER_DSM_DIRECT_CHARGE_REVERSE_OCP, buf);
		}
		break;
	case POWER_NE_DC_FAULT_OTP:
		hwlog_err("otp happened\n");
		di->otp_cnt++;
		if (di->otp_cnt >= OTP_CNT) {
			di->otp_cnt = OTP_CNT;
			snprintf(buf, sizeof(buf), "otp happened\n");
			strncat(buf, reg_info, strlen(reg_info));
			power_dsm_report_dmd(POWER_DSM_BATTERY,
				POWER_DSM_DIRECT_CHARGE_OTP, buf);
		}
		break;
	case POWER_NE_DC_FAULT_INPUT_OCP:
		hwlog_err("input ocp happened\n");
		snprintf(buf, sizeof(buf), "input ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			POWER_DSM_DIRECT_CHARGE_INPUT_OCP, buf);
		break;
	case POWER_NE_DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");
		snprintf(buf, sizeof(buf), "vdrop ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case POWER_NE_DC_FAULT_INA231:
		hwlog_err("ina231 interrupt happened\n");
		dcm_enable_ic(LVC_MODE, di->cur_mode, DC_IC_DISABLE);
		break;
	case POWER_NE_DC_FAULT_CC_SHORT:
		hwlog_err("typec cc vbus short happened\n");
		break;
	case POWER_NE_DC_FAULT_AC_HARD_RESET:
		hwlog_err("adapter hard reset\n");
		break;
	default:
		hwlog_err("unknown fault: %u happened\n", di->charge_fault);
		break;
	}
}

static void lvc_init_parameters(struct direct_charge_device *di)
{
	di->sysfs_enable_charger = 1;
	di->dc_stage = DC_STAGE_DEFAULT;
	dc_adpt_set_mode_info(LVC_MODE, ADAP_MAX_ISET, di->iin_thermal_default);
	di->dc_succ_flag = DC_ERROR;
	di->scp_stop_charging_complete_flag = 1;
	di->dc_err_report_flag = FALSE;
	di->sc_conv_ocp_count = 0;
	di->cur_mode = CHARGE_IC_MAIN;
	di->tbat_id = BAT_TEMP_MIXED;
	di->local_mode = LVC_MODE;
	di->multi_ic_check_info.force_single_path_flag = false;
}

static int lvc_probe(struct platform_device *pdev)
{
	int ret;
	struct direct_charge_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	dc_cable_init(di->dev, LVC_MODE);
	dc_ctrl_init(di->dev, LVC_MODE);
	dc_adpt_init(di->dev, LVC_MODE);
	ret = dc_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;
	lvc_init_parameters(di);
	direct_charge_set_local_mode(OR_SET, LVC_MODE);
	dc_comm_set_mode_ratio(LVC_MODE, di->dc_volt_ratio);
	g_lvc_di = di;
	direct_charge_set_di(di);
	dc_sysfs_init(di->dev, LVC_MODE);

	di->charging_wq = create_singlethread_workqueue("lvc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("lvc_wtd_wq");
	di->charging_lock = power_wakeup_source_register(di->dev, "lvc_wakelock");
	dc_init_work(LVC_MODE);
	INIT_WORK(&di->fault_work, lvc_fault_work);
	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = power_event_anc_register(POWER_ANT_LVC_FAULT, &di->fault_nb);
	ret += power_event_anc_register(POWER_ANT_DC_FAULT, &di->fault_nb);
	if (ret < 0)
		goto fail_create_link;

	power_if_ops_register(&lvc_if_ops);
	dc_dbg_register(di, LVC_MODE);

	platform_set_drvdata(pdev, di);

	return 0;

fail_create_link:
	dc_sysfs_remove(di->dev, LVC_MODE);

	power_wakeup_source_unregister(di->charging_lock);
fail_free_mem:
	dc_cable_remove(di->dev, LVC_MODE);
	dc_ctrl_remove(di->dev, LVC_MODE);
	dc_adpt_remove(di->dev, LVC_MODE);
	devm_kfree(&pdev->dev, di);
	g_lvc_di = NULL;

	return ret;
}

static int lvc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_anc_unregister(POWER_ANT_LVC_FAULT, &di->fault_nb);
	power_event_anc_unregister(POWER_ANT_DC_FAULT, &di->fault_nb);
	dc_sysfs_remove(di->dev, LVC_MODE);
	dc_ctrl_remove(di->dev, LVC_MODE);
	dc_cable_remove(di->dev, LVC_MODE);
	dc_adpt_remove(di->dev, LVC_MODE);

	power_wakeup_source_unregister(di->charging_lock);
	devm_kfree(&pdev->dev, di);
	g_lvc_di = NULL;

	return 0;
}

static const struct of_device_id lvc_match_table[] = {
	{
		.compatible = "direct_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver lvc_driver = {
	.probe = lvc_probe,
	.remove = lvc_remove,
	.driver = {
		.name = "direct_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lvc_match_table),
	},
};

static int __init lvc_init(void)
{
	return platform_driver_register(&lvc_driver);
}

static void __exit lvc_exit(void)
{
	platform_driver_unregister(&lvc_driver);
}

late_initcall(lvc_init);
module_exit(lvc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charger with loadswitch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
