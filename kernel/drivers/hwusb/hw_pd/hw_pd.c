/*
 * hw_pd.c
 *
 * this module is used for dpm of pd policy engine.
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <linux/device.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG hw_pd
HWLOG_REGIST();

void pd_dpm_usb_host_on(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;

	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void pd_dpm_usb_host_off(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;

	event.typec_orien = pd_dpm_get_cc_orientation();
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	pd_dpm_set_combphy_status(TCPC_NC);
	pd_dpm_handle_combphy_event(event);
#endif /* CONFIG_CONTEXTHUB_PD */
}

static void pd_dpm_report_device_attach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_CHARGER_CONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(CHARGER_CONNECT_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */

	water_detection_entrance();
}

static void pd_dpm_report_host_attach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif /* CONFIG_CONTEXTHUB_PD */

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_SWITCH_FSA9685
	if (switch_manual_enable) {
		usbswitch_common_dcd_timeout_enable(true);
		usbswitch_common_manual_sw(FSA9685_USB1_ID_TO_IDBYPASS);
	}
#endif /* CONFIG_SWITCH_FSA9685 */

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(ID_FALL_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */

	uvdm_init_work();
}

static void pd_dpm_report_device_detach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_CHARGER_DISCONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(CHARGER_DISCONNECT_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
}

static void pd_dpm_report_host_detach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif /* CONFIG_CONTEXTHUB_PD */

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_SWITCH_FSA9685
	usbswitch_common_dcd_timeout_enable(false);
#endif /* CONFIG_SWITCH_FSA9685 */

#ifdef CONFIG_CONTEXTHUB_PD
	event.typec_orien = pd_dpm_get_cc_orientation();
	event.mode_type = pd_dpm_get_combphy_status();
	if (pd_dpm_get_last_hpd_status()) {
		event.dev_type = TCA_DP_OUT;
		event.irq_type = TCA_IRQ_HPD_OUT;
		pd_dpm_handle_combphy_event(event);
		pd_dpm_set_last_hpd_status(false);

		hw_usb_send_event(DP_CABLE_OUT_EVENT);
	}
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	pd_dpm_set_combphy_status(TCPC_NC);
	pd_dpm_handle_combphy_event(event);
	/* set aux uart switch low */
	if (support_dp)
		dp_aux_uart_switch_disable();
#else
	chip_usb_otg_event(ID_RISE_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
	uvdm_cancel_work();
}

static void pd_dpm_report_attach(int new_state)
{
	switch (new_state) {
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
		pd_dpm_report_device_attach();
		break;
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		pd_dpm_report_host_attach();
		break;
	default:
		break;
	}
}

static void pd_dpm_report_detach(int last_state)
{
	switch (last_state) {
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
		pd_dpm_report_device_detach();
		break;
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		pd_dpm_report_host_detach();
		break;
	default:
		break;
	}
	hw_usb_host_abnormal_event_notify(USB_HOST_EVENT_NORMAL);
}

static void pd_dpm_usb_update_state(struct work_struct *work)
{
	int new_ev, last_ev;
	struct pd_dpm_info *usb_cb_data =
		container_of(to_delayed_work(work),
			struct pd_dpm_info, usb_state_update_work);

	mutex_lock(&usb_cb_data->usb_lock);
	new_ev = usb_cb_data->pending_usb_event;
	mutex_unlock(&usb_cb_data->usb_lock);

	last_ev = usb_cb_data->last_usb_event;

	if (last_ev == new_ev)
		return;

	switch (new_ev) {
	case PD_DPM_USB_TYPEC_DETACHED:
		pd_dpm_report_detach(last_ev);
		break;
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		if (last_ev != PD_DPM_USB_TYPEC_DETACHED)
			pd_dpm_report_detach(last_ev);
		pd_dpm_report_attach(new_ev);
		break;
	default:
		return;
	}

	usb_cb_data->last_usb_event = new_ev;
}

typedef struct __pd_event_handler {
	unsigned long event;
	int (*handler)(struct hw_pd_device *di);
} pd_event_handler_t;

#define decl_pd_event_handler((event), (xhandler)) { \
	.event = (event), \
	.handler = (xhandler), \
}

int pd_event_cc_change_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_typec_state_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_pd_state_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_disable_vbus_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_sink_vbus_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_source_vbus_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_source_vconn_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_dr_swap_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_pr_swap_handler(struct hw_pd_device *di)
{
	return 0;
}

int pd_event_cable_vdo_handler(struct hw_pd_device *di)
{
	return 0;
}

const pd_event_handler_t pd_event_handlers[] = {
	decl_pd_event_handler(PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER, pd_event_cc_change_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_TYPEC_STATE, pd_event_typec_state_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_PD_STATE, pd_event_pd_state_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_DIS_VBUS_CTRL, pd_event_disable_vbus_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_SINK_VBUS, pd_event_sink_vbus_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_SOURCE_VBUS, pd_event_source_vbus_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_SOURCE_VCONN, pd_event_source_vconn_handler),
	decl_pd_event_handler(PD_DPM_PE_EVT_DR_SWAP, pd_event_dr_swap_handler),
	decl_pd_event_handler(PD_DPM_PE_ABNORMAL_PR_SWAP_HANDLER, pd_event_pr_swap_handler),
	decl_pd_event_handler(PD_DPM_PE_ABNORMAL_CABLE_VDO_HANDLER, pd_event_cable_vdo_handler),
}

int pd_dpm_handle_pe_event(unsigned long event, void *data)
{
	int i;

	if (!g_pd_di) {
		hwlog_err("%s g_pd_di is null\n", __func__);
		return -1;
	}

	if (pogopin_ignore_typec_event(event, data)) {
		hwlog_info("pogo inset device ignore typec event:%d", event);
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(pd_event_handlers); i++) {
		if (pd_event_handlers[i].event == event) {
			if (pd_event_handlers[i].handler != 0)
				pd_event_handlers[i].handler(g_pd_di);
		}
	}

	if (i > ARRAY_SIZE(pd_event_handlers))
		hwlog_info("%s  unkonw event \n", __func__);

	return 0;
}

static void pd_cc_protection_dts_parser(struct hw_pd_device *di,
	struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_cc_detection", &abnormal_cc_detection, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_dynamic_protect", &cc_dynamic_protect, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_cc_interval", &abnormal_cc_interval, PD_DPM_CC_CHANGE_INTERVAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"moisture_detection_by_cc_enable", &moisture_detection_by_cc_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_moisture_status_report", &cc_moisture_status_report, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_abnormal_dmd_report_enable", &g_cc_abnormal_dmd_report_enable, 1);
}

static void pd_misc_dts_parser(struct hw_pd_device *di, struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_manual_enable", &switch_manual_enable, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_dp", &support_dp, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_channel", &g_otg_channel, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibus_check", &g_ibus_check, 1);
}

static void pd_accessory_dts_parser(struct hw_pd_device *di, struct device_node *np)
{
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_smart_holder", &support_smart_holder, 0);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_analog_audio", &support_analog_audio, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"emark_detect_enable", &emark_detect_enable, 1);
	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"tcp_name", &di->tcpc_name))
		di->tcpc_name = "type_c_port0";
}

static int pd_reinit_dts_parser(struct hw_pd_device *di, struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pd_reinit_enable", &di->pd_reinit_enable, 0))
		return -EINVAL;

	if (!di->pd_reinit_enable) {
		hwlog_err("pd reinit not enable\n");
		return -EINVAL;
	}

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"ldo_name", &di->ldo_name))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_ocp_count", &di->max_ocp_count, PD_DPM_MAX_OCP_COUNT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ocp_delay_time", &di->ocp_delay_time, OCP_DELAY_TIME);

	return 0;
}

static void hw_pd_parse_dts(struct hw_pd_device *di)
{
	struct device_node *np = di->dev->of_node;

	if (!di || !np) {
		hwlog_err("di or np is null\n");
		return;
	}

	cc_protection_dts_parser(di, np);
	misc_dts_parser(di, np);
	accessory_dts_parser(di, np);
	hwlog_info("parse_dts success\n");
}

static void pd_dpm_init_resource(struct hw_pd_device *di)
{
	hw_pd_parse_dts(di);
}

static int hw_pd_probe(struct platform_device *pdev)
{
	struct hw_pd_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	g_hw_pd_di = di;
	pd_dpm_init_resource(di);
	return 0;
}

static const struct of_device_id hw_pd_match_table[] = {
	{
		.compatible = "hw_pd",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_pd_driver = {
	.probe = hw_pd_probe,
	.driver = {
		.name = "hw_pd",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_pd_match_table),
	},
};

static int __init hw_pd_init(void)
{
	return platform_driver_register(&hw_pd_driver);
}

static void __exit hw_pd_exit(void)
{
	platform_driver_unregister(&hw_pd_driver);
}

late_initcall(hw_pd_init);
module_exit(hw_pd_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pd dpm driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
