// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charger_sc4.c
 *
 * direct charger with sc4 (switch cap) driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/direct_charge/direct_charge_test.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_work.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_sysfs.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG direct_charge_sc4
HWLOG_REGIST();

static struct direct_charge_device *g_sc4_di;

static int sc4_set_disable_func(unsigned int val)
{
	int work_mode = direct_charge_get_working_mode();

	if (!g_sc4_di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	g_sc4_di->force_disable = val;
	hwlog_info("working_mode = %d, work_mode = %d\n", g_sc4_di->working_mode, work_mode);
	if (val && (work_mode != UNDEFINED_MODE) &&
		(g_sc4_di->working_mode == work_mode))
		wired_connect_send_icon_uevent(ICON_TYPE_NORMAL);
	hwlog_info("force_disable = %u\n", val);
	return 0;
}

static int sc4_get_disable_func(unsigned int *val)
{
	if (!g_sc4_di || !val) {
		hwlog_err("di or val is null\n");
		return -ENODEV;
	}

	*val = g_sc4_di->force_disable;
	return 0;
}

static int sc4_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_sc4_di;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -EPERM;

	ret = sc4_set_disable_flags((val ?
		DC_CLEAR_DISABLE_FLAGS : DC_SET_DISABLE_FLAGS),
		DC_DISABLE_SYS_NODE);
	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);
	return ret;
}

static int sc4_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	*val = di->sysfs_enable_charger;
	return 0;
}

static int mainsc4_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -EPERM;

	di->sysfs_mainsc_enable_charger = val;
	hwlog_info("set mainsc enable_charger=%d\n",
		di->sysfs_mainsc_enable_charger);
	return 0;
}

static int mainsc4_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	*val = di->sysfs_mainsc_enable_charger;
	return 0;
}

static int auxsc4_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -EPERM;

	di->sysfs_auxsc_enable_charger = val;
	hwlog_info("set auxsc enable_charger=%d\n",
		di->sysfs_auxsc_enable_charger);
	return 0;
}

static int auxsc4_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	*val = di->sysfs_auxsc_enable_charger;
	return 0;
}

static int sc4_set_iin_limit(unsigned int val)
{
	return dc_sysfs_set_iin_thermal(SC4_MODE, (int)val);
}

static int sc4_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EPERM;
	}

	dc_sysfs_get_iin_thermal_limit(SC4_MODE, di->cur_mode, val);

	return 0;
}

static int sc4_set_iin_thermal(unsigned int index, unsigned int iin_thermal_value)
{
	if (index >= DC_CHANNEL_TYPE_END) {
		hwlog_err("error index: %u, out of boundary\n", index);
		return -EPERM;
	}
	return dc_sysfs_set_iin_thermal_array(SC4_MODE, index, iin_thermal_value);
}

static int sc4_set_iin_thermal_all(unsigned int value)
{
	int i;

	for (i = DC_CHANNEL_TYPE_BEGIN; i < DC_CHANNEL_TYPE_END; i++) {
		if (dc_sysfs_set_iin_thermal_array(SC4_MODE, i, value))
			return -EPERM;
	}
	return 0;
}

static int sc4_set_ichg_ratio(unsigned int val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%u\n", val);
	return 0;
}

static int sc4_set_vterm_dec(unsigned int val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	di->vterm_dec = val;
	hwlog_info("set vterm_dec=%u\n", val);
	return 0;
}

static int sc4_get_hota_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	*val = di->hota_iin_limit;
	return 0;
}

static int sc4_get_startup_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	*val = di->startup_iin_limit;
	return 0;
}

static int sc4_get_ibus(int *ibus)
{
	int ret;

	if (!ibus)
		return -EPERM;

	ret = dcm_get_ic_ibus(SC4_MODE, CHARGE_MULTI_IC, ibus);
	if (ret || ibus < 0)
		return -EPERM;
	return 0;
}

static int sc4_get_vbus(int *vbus)
{
	struct direct_charge_device *di = g_sc4_di;

	if (!di || di->ls_vbus < 0 || !vbus)
		return -EPERM;

	*vbus = di->ls_vbus;
	return 0;
}

static struct power_if_ops sc4_if_ops = {
	.set_enable_charger = sc4_set_enable_charger,
	.get_enable_charger = sc4_get_enable_charger,
	.set_iin_limit = sc4_set_iin_limit,
	.get_iin_limit = sc4_get_iin_limit,
	.set_iin_thermal = sc4_set_iin_thermal,
	.set_iin_thermal_all = sc4_set_iin_thermal_all,
	.set_ichg_ratio = sc4_set_ichg_ratio,
	.set_vterm_dec = sc4_set_vterm_dec,
	.get_hota_iin_limit = sc4_get_hota_iin_limit,
	.get_startup_iin_limit = sc4_get_startup_iin_limit,
	.get_ibus = sc4_get_ibus,
	.get_vbus = sc4_get_vbus,
	.set_disable_func = sc4_set_disable_func,
	.get_disable_func = sc4_get_disable_func,
	.type_name = "4sc",
};

static struct power_if_ops mainsc4_if_ops = {
	.set_enable_charger = mainsc4_set_enable_charger,
	.get_enable_charger = mainsc4_get_enable_charger,
	.type_name = "main4sc",
};

static struct power_if_ops auxsc4_if_ops = {
	.set_enable_charger = auxsc4_set_enable_charger,
	.get_enable_charger = auxsc4_get_enable_charger,
	.type_name = "aux4sc",
};

static void sc4_power_if_ops_register(struct direct_charge_device *di)
{
	power_if_ops_register(&sc4_if_ops);
	if (di->multi_ic_mode_para.support_multi_ic) {
		power_if_ops_register(&mainsc4_if_ops);
		power_if_ops_register(&auxsc4_if_ops);
	}
}

int sc4_get_di(struct direct_charge_device **di)
{
	if (!g_sc4_di || !di)
		return -EPERM;

	*di = g_sc4_di;

	return 0;
}

int sc4_set_disable_flags(int val, int type)
{
	int i;
	unsigned int disable;
	struct direct_charge_device *di = g_sc4_di;

	if (!di)
		return -EPERM;

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

static void sc4_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;

	if (!work)
		return;

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di)
		return;

	if (di->fault_data)
		snprintf(reg_info, sizeof(reg_info),
			"sc4 charge_fault=%d, addr=0x%x, event1=0x%x, event2=0x%x\n",
			di->charge_fault, di->fault_data->addr,
			di->fault_data->event1, di->fault_data->event2);
	else
		snprintf(reg_info, sizeof(reg_info),
			"sc4 charge_fault=%d, addr=0x0, event1=0x0, event2=0x0\n",
			di->charge_fault);
	dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_HAPPEN_SC4_FAULT, reg_info);

	switch (di->charge_fault) {
	case POWER_NE_DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");
		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_VBUS_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_TSBAT_OTP:
		hwlog_err("tsbat otp happened\n");
		snprintf(buf, sizeof(buf), "tsbat otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_TSBAT_OTP, buf);
		break;
	case POWER_NE_DC_FAULT_TSBUS_OTP:
		hwlog_err("tsbus otp happened\n");
		snprintf(buf, sizeof(buf), "tsbus otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_TSBUS_OTP, buf);
		break;
	case POWER_NE_DC_FAULT_TDIE_OTP:
		hwlog_err("tdie otp happened\n");
		snprintf(buf, sizeof(buf), "tdie otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_TDIE_OTP, buf);
		break;
	case POWER_NE_DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");
		snprintf(buf, sizeof(buf), "vdrop ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case POWER_NE_DC_FAULT_AC_OVP:
		hwlog_err("AC ovp happened\n");
		snprintf(buf, sizeof(buf), "ac ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_AC_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_AC_HARD_RESET:
		hwlog_err("adapter hard reset\n");
		break;
	case POWER_NE_DC_FAULT_VBAT_OVP:
		hwlog_err("vbat ovp happened\n");
		snprintf(buf, sizeof(buf), "vbat ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_VBAT_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_IBAT_OCP:
		hwlog_err("ibat ocp happened\n");
		snprintf(buf, sizeof(buf), "ibat ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_IBAT_OCP, buf);
		break;
	case POWER_NE_DC_FAULT_IBUS_OCP:
		hwlog_err("ibus ocp happened\n");
		snprintf(buf, sizeof(buf), "ibus ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_IBUS_OCP, buf);
		break;
	case POWER_NE_DC_FAULT_CONV_OCP:
		hwlog_err("conv ocp happened\n");
		snprintf(buf, sizeof(buf), "conv ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			POWER_DSM_DIRECT_CHARGE_SC_FAULT_CONV_OCP, buf);
		dc_test_set_adapter_test_result(di->local_mode, AT_DETECT_OTHER);
		di->sc_conv_ocp_count++;
		break;
	case POWER_NE_DC_FAULT_LTC7820:
		hwlog_err("ltc7820 chip error happened\n");
		snprintf(buf, sizeof(buf), "ltc7820 chip error happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case POWER_NE_DC_FAULT_INA231:
		hwlog_err("ina231 interrupt happened\n");
		dcm_enable_ic(SC4_MODE, di->cur_mode, DC_IC_DISABLE);
		dc_close_aux_wired_channel();
		break;
	case POWER_NE_DC_FAULT_CC_SHORT:
		hwlog_err("typec cc vbus short happened\n");
		break;
	default:
		hwlog_err("unknown fault: %u happened\n", di->charge_fault);
		break;
	}
}

static void sc4_init_parameters(struct direct_charge_device *di)
{
	di->sysfs_enable_charger = 1;
	di->sysfs_mainsc_enable_charger = 1;
	di->sysfs_auxsc_enable_charger = 1;
	di->dc_stage = DC_STAGE_DEFAULT;
	dc_adpt_set_mode_info(SC4_MODE, ADAP_MAX_ISET, di->iin_thermal_default);
	di->dc_succ_flag = DC_ERROR;
	di->scp_stop_charging_complete_flag = 1;
	di->dc_err_report_flag = FALSE;
	di->sc_conv_ocp_count = 0;
	di->cur_mode = CHARGE_IC_MAIN;
	di->tbat_id = BAT_TEMP_MIXED;
	di->local_mode = SC4_MODE;
	di->cc_safe = true;
}

static void sc4_init_mulit_ic_check_info(struct direct_charge_device *di)
{
	di->multi_ic_check_info.limit_current = di->iin_thermal_default;
	di->multi_ic_check_info.ibat_th = MULTI_IC_INFO_IBAT_TH_DEFAULT;
	di->multi_ic_check_info.force_single_path_flag = false;
}

static int sc4_probe(struct platform_device *pdev)
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

	dc_cable_init(di->dev, SC4_MODE);
	dc_ctrl_init(di->dev, SC4_MODE);
	dc_adpt_init(di->dev, SC4_MODE);
	ret = dc_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	sc4_init_parameters(di);
	sc4_init_mulit_ic_check_info(di);
	direct_charge_set_local_mode(OR_SET, SC4_MODE);
	dc_comm_set_mode_ratio(SC4_MODE, di->dc_volt_ratio);
	g_sc4_di = di;
	direct_charge_set_di(di);
	dc_sysfs_init(di->dev, SC4_MODE);

	di->charging_wq = create_singlethread_workqueue("sc4_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("sc4_wtd_wq");
	di->charging_lock = power_wakeup_source_register(di->dev, "sc4_wakelock");
	dc_init_work(SC4_MODE);
	INIT_WORK(&di->fault_work, sc4_fault_work);
	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = power_event_anc_register(POWER_ANT_SC4_FAULT, &di->fault_nb);
	ret += power_event_anc_register(POWER_ANT_DC_FAULT, &di->fault_nb);
	if (ret < 0)
		goto fail_create_link;

	sc4_power_if_ops_register(di);
	dc_dbg_register(di, SC4_MODE);

	platform_set_drvdata(pdev, di);

	return 0;

fail_create_link:
	dc_sysfs_remove(di->dev, SC4_MODE);

	power_wakeup_source_unregister(di->charging_lock);
fail_free_mem:
	dc_cable_remove(di->dev, SC4_MODE);
	dc_ctrl_remove(di->dev, SC4_MODE);
	dc_adpt_remove(di->dev, SC4_MODE);
	devm_kfree(&pdev->dev, di);
	g_sc4_di = NULL;

	return ret;
}

static int sc4_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_anc_unregister(POWER_ANT_SC4_FAULT, &di->fault_nb);
	power_event_anc_unregister(POWER_ANT_DC_FAULT, &di->fault_nb);
	dc_sysfs_remove(di->dev, SC4_MODE);
	dc_ctrl_remove(di->dev, SC4_MODE);
	dc_cable_remove(di->dev, SC4_MODE);
	dc_adpt_remove(di->dev, SC4_MODE);

	power_wakeup_source_unregister(di->charging_lock);
	devm_kfree(&pdev->dev, di);
	g_sc4_di = NULL;

	return 0;
}

static const struct of_device_id sc4_match_table[] = {
	{
		.compatible = "direct_charger_sc4",
		.data = NULL,
	},
	{},
};

static struct platform_driver sc4_driver = {
	.probe = sc4_probe,
	.remove = sc4_remove,
	.driver = {
		.name = "direct_charger_sc4",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sc4_match_table),
	},
};

static int __init sc4_init(void)
{
	return platform_driver_register(&sc4_driver);
}

static void __exit sc4_exit(void)
{
	platform_driver_unregister(&sc4_driver);
}

late_initcall(sc4_init);
module_exit(sc4_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charger with switch cap module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
