// SPDX-License-Identifier: GPL-2.0
/*
 * ffc_control.c
 *
 * ffc control driver
 *
 * Copyright (c) 2020-2022 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_base.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_control.h>

#define HWLOG_TAG ffc_control
HWLOG_REGIST();

#define FFC_VTERM_START_FLAG                BIT(0)
#define FFC_VETRM_END_FLAG                  BIT(1)
#define FFC_CHARGE_EXIT_TIMES               2
#define FFC_DEFAULT_VTERM                   4450
#define FFC_DEFAULT_ITERM                   160

static int g_default_vterm = FFC_DEFAULT_VTERM;
static int g_default_iterm = FFC_DEFAULT_ITERM;
static struct ffc_ctrl_dev *g_ffc_ctrl_dev;

static struct ffc_ctrl_dev *get_ffc_ctrl_dev(void)
{
	if (!g_ffc_ctrl_dev) {
		hwlog_err("g_ffc_ctrl_dev is null\n");
		return NULL;
	}

	return g_ffc_ctrl_dev;
}

void ffc_ctrl_set_default_info(int vterm, int iterm)
{
	g_default_vterm = vterm;
	g_default_iterm = iterm;
}

static bool ffc_ctrl_need_enter_ffc(struct ffc_ctrl_dev *di)
{
	if (di->dc_adp || (di->fcp_support_ffc && di->fcp_adp))
		return true;

	return false;
}

int ffc_ctrl_get_incr_vterm(void)
{
	static int cnt;
	int ffc_vterm;
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di)
		return 0;

	if (!ffc_ctrl_need_enter_ffc(di))
		return 0;

	if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE)
		return ffc_get_buck_vterm_with_temp(di);

	if (!direct_charge_check_charge_done()) {
		hwlog_info("not sc siwtch to buck, no need ffc\n");
		return 0;
	}

	if (di->ffc_delay_cnt < di->delay_max_times) {
		ffc_vterm = ffc_get_buck_vterm_with_temp(di);
		di->ffc_delay_cnt++;
	} else {
		ffc_vterm = ffc_get_buck_vterm(di);
	}

	if (di->ffc_vterm_flag & FFC_VETRM_END_FLAG) {
		charge_update_iterm(ffc_get_buck_iterm(di));
		return 0;
	}

	if (ffc_vterm) {
		cnt = 0;
		di->ffc_vterm_flag |= FFC_VTERM_START_FLAG;
		return ffc_vterm;
	}

	if (di->ffc_vterm_flag & FFC_VTERM_START_FLAG) {
		cnt++;
		if (cnt < FFC_CHARGE_EXIT_TIMES)
			return ffc_get_buck_vterm_with_temp(di);
		di->ffc_vterm_flag |= FFC_VETRM_END_FLAG;
		cnt = 0;
	}

	return 0;
}

static void ffc_ctrl_set_ffc_info(struct ffc_ctrl_charge_info *data, bool flag, int iterm)
{
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di || !data)
		return;

	data->ffc_charge_flag = flag;
	if (iterm)
		data->iterm = iterm;
	else
		data->iterm = g_default_iterm;
}

static void ffc_ctrl_update_ffc_info(struct ffc_ctrl_charge_info *data)
{
	unsigned int vol = 0;
	int iterm;
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di || !data)
		return;

	if (direct_charge_in_charging_stage() == DC_NOT_IN_CHARGING_STAGE) {
		if (di->ffc_vterm_flag == FFC_VTERM_START_FLAG)
			ffc_ctrl_set_ffc_info(data, true, ffc_get_buck_ichg_th(di));
		else if (di->ffc_vterm_flag & FFC_VETRM_END_FLAG)
			ffc_ctrl_set_ffc_info(data, false, ffc_get_buck_iterm(di));
		else
			ffc_ctrl_set_ffc_info(data, false, g_default_iterm);
		data->dc_mode = false;
		return;
	}

	data->dc_mode = true;
	direct_charge_get_vmax(&vol);
	hwlog_info("vol_max=%u, vterm=%u\n", vol, g_default_vterm);
	if (vol > g_default_vterm) {
		iterm = ffc_get_buck_ichg_th(di);
		ffc_ctrl_set_ffc_info(data, true, iterm);
		return;
	}

	ffc_ctrl_set_ffc_info(data, false, g_default_iterm);
}

void ffc_ctrl_notify_ffc_info(void)
{
	struct ffc_ctrl_charge_info data = { 0 };

	ffc_ctrl_update_ffc_info(&data);
	hwlog_info("ffc_charge_flag=%u dc_mode=%u iterm=%d\n", data.ffc_charge_flag, data.dc_mode, data.iterm);
	power_event_bnc_notify(POWER_BNT_BUCK_CHARGE, POWER_NE_BUCK_FFC_CHARGE, &data);
}

static int ffc_ctrl_event_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_CHARGING_STOP:
		di->ffc_vterm_flag = 0;
		di->dc_adp = false;
		di->fcp_adp = false;
		di->ffc_delay_cnt = 0;
		charge_update_iterm(g_default_iterm);
		hwlog_info("charge stop, ffc charge set default\n");
		break;
	case POWER_NE_DC_CHECK_START:
		di->dc_adp = true;
		hwlog_info("dc check start\n");
		break;
	case POWER_NE_FCP_CHARGING_START:
		if (di->fcp_support_ffc) {
			di->fcp_adp = true;
			hwlog_info("fcp check start\n");
		}
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int ffc_ctrl_probe(struct platform_device *pdev)
{
	int ret;
	struct ffc_ctrl_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;

	ffc_ctrl_parse_dts(np, di);

	di->event_nb.notifier_call = ffc_ctrl_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &di->event_nb);
	if (ret) {
		hwlog_err("register POWER_BNT_CHARGING failed\n");
		goto fail_free_mem;
	}

	di->event_dc_nb.notifier_call = ffc_ctrl_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_DC, &di->event_dc_nb);
	if (ret) {
		hwlog_err("register POWER_BNT_DC failed\n");
		goto fail_bnc_register;
	}

	di->event_fcp_nb.notifier_call = ffc_ctrl_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_FCP, &di->event_fcp_nb);
	if (ret) {
		hwlog_err("register POWER_BNT_FCP failed\n");
		goto fail_bnc_fcp_register;
	}

	g_ffc_ctrl_dev = di;
	platform_set_drvdata(pdev, di);
	return 0;

fail_bnc_fcp_register:
	power_event_bnc_unregister(POWER_BNT_DC, &di->event_dc_nb);
fail_bnc_register:
	power_event_bnc_unregister(POWER_BNT_CHARGING, &di->event_nb);
fail_free_mem:
	kfree(di->ffc_term_para_group);
	kfree(di);
	return ret;
}

static int ffc_ctrl_remove(struct platform_device *pdev)
{
	struct ffc_ctrl_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_CHARGING, &di->event_nb);
	power_event_bnc_unregister(POWER_BNT_DC, &di->event_dc_nb);
	power_event_bnc_unregister(POWER_BNT_FCP, &di->event_fcp_nb);
	platform_set_drvdata(pdev, NULL);
	kfree(di->ffc_term_para_group);
	kfree(di);
	g_ffc_ctrl_dev = NULL;

	return 0;
}

static const struct of_device_id ffc_ctrl_match_table[] = {
	{
		.compatible = "huawei,ffc_control",
		.data = NULL,
	},
	{},
};

static struct platform_driver ffc_ctrl_driver = {
	.probe = ffc_ctrl_probe,
	.remove = ffc_ctrl_remove,
	.driver = {
		.name = "huawei,ffc_control",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ffc_ctrl_match_table),
	},
};

static int __init ffc_ctrl_init(void)
{
	return platform_driver_register(&ffc_ctrl_driver);
}

static void __exit ffc_ctrl_exit(void)
{
	platform_driver_unregister(&ffc_ctrl_driver);
}

device_initcall_sync(ffc_ctrl_init);
module_exit(ffc_ctrl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ffc control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
