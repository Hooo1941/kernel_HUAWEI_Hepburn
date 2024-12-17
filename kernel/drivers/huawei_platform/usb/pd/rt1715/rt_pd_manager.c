/*
 * Copyright (C) 2020 Richtek Inc.
 *
 * Richtek RT PD Manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/extcon-provider.h>
#include <linux/iio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/regulator/consumer.h>
#include <linux/usb/typec.h>
#include <linux/version.h>
#ifdef CONFIG_TCPC_RT1715
#ifdef CONFIG_USB_PROXY_HCD
#include <linux/hisi/usb/hifi_usb.h>
#endif /* CONFIG_USB_PROXY_HCD */
#endif /* CONFIG_TCPC_RT1715 */

#ifdef CONFIG_TCPC_RT1715
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/huawei_charger.h>
#endif

#include "inc/tcpci_typec.h"
#include "inc/tcpm.h"
#include "inc/tcpci.h"

#define RT_PD_MANAGER_VERSION	"1.0.0_G"

#define PROBE_CNT_MAX			11
/* 10ms * 100 = 1000ms = 1s */
#define USB_TYPE_POLLING_INTERVAL	10
#define USB_TYPE_POLLING_CNT_MAX	100
#define USB_INIT_WORK_DEALY			1000
#define USB_WAIT_COMBOPHY_TIMEOUT	60

enum dr {
#ifdef CONFIG_TCPC_RT1715
	DR_IDLE_DEVICE,
#endif
	DR_IDLE_HOST,
	DR_DEVICE,
	DR_HOST,
	DR_DEVICE_TO_HOST,
	DR_HOST_TO_DEVICE,
	DR_MAX,
};

static const char * const dr_names[DR_MAX] = {
	"Idle_Device", "IDLE_host", "Device", "Host", "Device to Host", "Host to Device",
};

struct rt_pd_manager_data {
	struct device *dev;
	struct extcon_dev *extcon;
#ifdef CONFIG_TCPC_RT1715
	struct regulator *vbus_reg;
	bool vbus_reg_en;
	struct power_supply *usb_psy;
	struct delayed_work usb_init_work;
	int combphy_detect;
	int ctc_ocp_flag;
#endif
	struct delayed_work usb_dwork;
	struct tcpc_device *tcpc;
	struct notifier_block pd_nb;
	enum dr usb_dr;
	int usb_type_polling_cnt;

#ifdef NOT_USED_CODE
	struct typec_capability typec_caps;
	struct typec_port *typec_port;
	struct typec_partner *partner;
	struct typec_partner_desc partner_desc;
	struct usb_pd_identity partner_identity;
#endif
};
/*
static const unsigned int rpm_extcon_cable[] = {
	EXTCON_USB,
	EXTCON_USB_HOST,
	EXTCON_NONE,
};
*/

int tcpc_class_complete_init(void);
#ifdef NOT_USED_CODE 
static int extcon_init(struct rt_pd_manager_data *rpmd)
{
	int ret = 0; */

	/*
	 * associate extcon with the dev as it could have a DT
	 * node which will be useful for extcon_get_edev_by_phandle()
	 */
	rpmd->extcon = devm_extcon_dev_allocate(rpmd->dev, rpm_extcon_cable);
	if (IS_ERR(rpmd->extcon)) {
		ret = PTR_ERR(rpmd->extcon);
		dev_err(rpmd->dev, "%s extcon dev alloc fail(%d)\n",
				   __func__, ret);
		goto out;
	}

	ret = devm_extcon_dev_register(rpmd->dev, rpmd->extcon);
	if (ret) {
		dev_err(rpmd->dev, "%s extcon dev reg fail(%d)\n",
				   __func__, ret);
		goto out;
	}

	/* Support reporting polarity and speed via properties */
	extcon_set_property_capability(rpmd->extcon, EXTCON_USB,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(rpmd->extcon, EXTCON_USB,
				       EXTCON_PROP_USB_SS);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	extcon_set_property_capability(rpmd->extcon, EXTCON_USB,
				       EXTCON_PROP_USB_TYPEC_MED_HIGH_CURRENT);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)) */
	extcon_set_property_capability(rpmd->extcon, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(rpmd->extcon, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_SS);
out:
	return ret;
}

static inline void stop_usb_host(struct rt_pd_manager_data *rpmd)
{
	extcon_set_state_sync(rpmd->extcon, EXTCON_USB_HOST, false);
}

static inline void start_usb_host(struct rt_pd_manager_data *rpmd)
{
	union extcon_property_value val = {.intval = 0};

	val.intval = tcpm_inquire_cc_polarity(rpmd->tcpc);
	extcon_set_property(rpmd->extcon, EXTCON_USB_HOST,
			    EXTCON_PROP_USB_TYPEC_POLARITY, val);

	val.intval = 1;
	extcon_set_property(rpmd->extcon, EXTCON_USB_HOST,
			    EXTCON_PROP_USB_SS, val);

	extcon_set_state_sync(rpmd->extcon, EXTCON_USB_HOST, true);
}

static inline void stop_usb_peripheral(struct rt_pd_manager_data *rpmd)
{
	extcon_set_state_sync(rpmd->extcon, EXTCON_USB, false);
}

static inline void start_usb_peripheral(struct rt_pd_manager_data *rpmd)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	int rp = 0;
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)) */
	union extcon_property_value val = {.intval = 0};

	val.intval = tcpm_inquire_cc_polarity(rpmd->tcpc);
	extcon_set_property(rpmd->extcon, EXTCON_USB,
			    EXTCON_PROP_USB_TYPEC_POLARITY, val);

	val.intval = 1;
	extcon_set_property(rpmd->extcon, EXTCON_USB, EXTCON_PROP_USB_SS, val);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	rp = tcpm_inquire_typec_remote_rp_curr(rpmd->tcpc);
	val.intval = rp > 500 ? 1 : 0;
	extcon_set_property(rpmd->extcon, EXTCON_USB,
			    EXTCON_PROP_USB_TYPEC_MED_HIGH_CURRENT, val);
#endif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	extcon_set_state_sync(rpmd->extcon, EXTCON_USB, true);
}
#endif

static void usb_dwork_handler(struct work_struct *work)
{
	unsigned long charger_type = CHARGER_REMOVED;
	struct pd_dpm_vbus_state local_state;
	struct delayed_work *dwork = to_delayed_work(work);
	struct rt_pd_manager_data *rpmd =
		container_of(dwork, struct rt_pd_manager_data, usb_dwork);
	enum dr usb_dr = rpmd->usb_dr;

	if (usb_dr < DR_IDLE_DEVICE || usb_dr >= DR_MAX) {
		dev_err(rpmd->dev, "%s invalid usb_dr = %d\n",
				   __func__, usb_dr);
		return;
	}

	dev_info(rpmd->dev, "%s %s\n", __func__, dr_names[usb_dr]);

	switch (usb_dr) {
#ifdef CONFIG_TCPC_RT1715
	case DR_IDLE_DEVICE:
		pd_dpm_report_device_detach();
		// stop_usb_peripheral(rpmd);
		// stop_usb_host(rpmd);
		break;
	case DR_IDLE_HOST:
		pd_dpm_report_host_detach();
#endif
	case DR_MAX:
		// stop_usb_peripheral(rpmd);
		// stop_usb_host(rpmd);
		break;
	case DR_DEVICE:
#ifdef CONFIG_TCPC_RT1715
		pd_dpm_report_device_attach();
		msleep(800);
		pd_dpm_get_charge_event(&charger_type, &local_state);
		dev_info(rpmd->dev, "%s polling_cnt = %d, charger_type = %ld\n",
				    __func__, ++rpmd->usb_type_polling_cnt, charger_type);
		if ((charger_type != CHARGER_TYPE_USB) && (charger_type != CHARGER_TYPE_BC_USB)) {
			break;
		}
#endif
        pr_err("DR_DEVICE fall througth to DR_HOST_TO_DEVICE\n");
	case DR_HOST_TO_DEVICE:
		// stop_usb_host(rpmd);
		// start_usb_peripheral(rpmd);
		break;
	case DR_HOST:
	case DR_DEVICE_TO_HOST:
#ifdef CONFIG_TCPC_RT1715
		pd_dpm_report_host_attach();
#endif
		// stop_usb_peripheral(rpmd);
		// start_usb_host(rpmd);
		break;
	}
}

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
static void hnt_detect_vbus_only_first_insert(struct rt_pd_manager_data *rpmd)
{
	uint16_t power_status = 0;
	uint32_t alert_status = 0;
	int rv;

	if (hnt_get_husb311_vid_flag()) {
		rv = tcpci_get_alert_status(rpmd->tcpc, &alert_status);
		if (rv < 0) {
			pr_err("%s: tcpci_get_alert_status fail\n", __func__);
			return;
		}

		rv = tcpci_get_power_status(rpmd->tcpc, &power_status);
		if (rv < 0) {
			pr_err("%s: tcpci_get_power_status fail\n", __func__);
			return;
		}

		pr_info("%s alert_status:%d , power_status: %d\n", __func__, alert_status, power_status);
		/* alert_status:0x10 bit0:CC_STATUS bit1:PWR_STATUS power_status:0x1E bit2:VBUS_PRESENT */
		if (!get_bit(alert_status, 0) && !get_bit(alert_status, 1) && get_bit(power_status, 2)) {
			pr_info("%s hnt identify vbus_only after reboot\n", __func__);
			tcpci_vbus_level_changed(rpmd->tcpc);
		}
	}
}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */

#ifdef CONFIG_TCPC_RT1715
static void usb_init_handler(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct rt_pd_manager_data *rpmd =
		container_of(dwork, struct rt_pd_manager_data, usb_init_work);
#if defined(CONFIG_TCPC_RT1715) && defined(CONFIG_CONTEXTHUB_PD)
	static int time_count = 0;

	if (rpmd->combphy_detect && time_count < USB_WAIT_COMBOPHY_TIMEOUT) {
		if (!pd_dpm_get_combophy_ready_status()) {
			dev_info(rpmd->dev, "combophy is not ready\n");
			time_count++;
			schedule_delayed_work(&rpmd->usb_init_work, msecs_to_jiffies(USB_INIT_WORK_DEALY));
			return;
		} else {
			msleep(200); /* wait 200ms for combophy */
		}
	}
#endif

	dev_info(rpmd->dev, "usb is ready, enable tcpc");
	tcpc_class_complete_init();

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
	hnt_detect_vbus_only_first_insert(rpmd);
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */
}

/*
static void wait_charger_manager_ready(struct rt_pd_manager_data *rpmd)
{
	int try_cnt;

	for (try_cnt = 0; try_cnt < PROBE_CNT_MAX; try_cnt++) {
		rpmd->usb_psy = power_supply_get_by_name("usb");
		if (rpmd->usb_psy == NULL) {
			if (try_cnt >= PROBE_CNT_MAX) {
				dev_info(rpmd->dev, "%s:try_cnt over max\n", __func__);
				return;
			} else {
				dev_err(rpmd->dev, "%s get usb psy fail, try_cnt=%d\n", __func__, ++try_cnt);
				msleep(500);
			}
		} else {
			dev_info(rpmd->dev, "%s:usb psy is ready\n", __func__);
			return;
		}
	}
}
*/
#endif

static int pd_tcp_notifier_call(struct notifier_block *nb,
				unsigned long event, void *data)
{
#ifdef CONFIG_TCPC_RT1715
	int cc_detection = 0;
	struct pd_dpm_swap_state swap_state;
#endif
	struct tcp_notify *noti = data;
	struct pd_dpm_typec_state tc_state = {0};
	struct rt_pd_manager_data *rpmd =
		container_of(nb, struct rt_pd_manager_data, pd_nb);
	uint8_t old_state = TYPEC_UNATTACHED, new_state = TYPEC_UNATTACHED;
#ifdef CONFIG_TCPC_RT1715
	struct pd_dpm_info *di = get_pd_dpm_info();
	bool vconn_swap_flag;
	int cable_current_limit = 0;
	struct pd_dpm_cable_info cable_vdo = { 0 };

	/* For the first startup */
	// wait_charger_manager_ready(rpmd);
	tc_state.polarity = noti->typec_state.polarity;
#endif
	dev_info(rpmd->dev, "%s event = %ld, polarity = %d\n", __func__, event, tc_state.polarity);
	switch (event) {
#ifdef CONFIG_TCPC_RT1715
	case TCP_NOTIFY_ABNORMAL_CC_CHANGE_HANDLER:
		dev_info(rpmd->dev, "%s:%lu\n", __func__, event);
		cc_detection = get_abnormal_cc_detection();
		if (cc_detection && (direct_charge_get_stage_status() < DC_STAGE_CHARGE_INIT))
			pd_dpm_handle_abnomal_change(PD_DPM_ABNORMAL_CC_CHANGE);
		break;
#endif
	case TCP_NOTIFY_SINK_VBUS:
		dev_info(rpmd->dev, "%s sink vbus %dmV %dmA type(0x%02X) flag(%d)\n",
			__func__, noti->vbus_state.mv, noti->vbus_state.ma,
			noti->vbus_state.type, noti->vbus_state.pd_connect_flag);
#ifdef CONFIG_TCPC_RT1715
		if (noti->vbus_state.pd_connect_flag &&
		    !(noti->vbus_state.type & TCP_VBUS_CTRL_PD_DETECT) &&
		      rpmd->ctc_ocp_flag) {
			dev_info(rpmd->dev, "%s: type state is fail, not sink vbus\n", __func__);
			break;
		}
		pd_dpm_report_pd_sink_vbus(di, (void *)(&noti->vbus_state));
		break;
	case TCP_NOTIFY_DIS_VBUS_CTRL:
		dev_info(rpmd->dev, "%s  Disable VBUS Control first\n", __func__);
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_DIS_VBUS_CTRL, NULL);
#ifdef NOT_USED_CODE
		if ((di->pd_finish_flag == true) || pd_dpm_get_pd_source_vbus()) {
			struct pd_dpm_vbus_state vbus_state;
			dev_info(rpmd->dev, "%s  Disable VBUS Control\n", __func__);
			vbus_state.mv = 0;
			vbus_state.ma = 0;
			pd_dpm_vbus_notifier_call(di, CHARGER_TYPE_NONE, &vbus_state);
			if (di->pd_source_vbus)
				pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SOURCE);
			else
				pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SINK);

			mutex_lock(&di->sink_vbus_lock);
			set_pd_dpm_info_flag(false, false);
			mutex_unlock(&di->sink_vbus_lock);
		}
#endif
#endif
		break;
	case TCP_NOTIFY_SOURCE_VBUS:
		dev_info(rpmd->dev, "%s source vbus %dmV %dmA type(0x%02X)\n",
			__func__, noti->vbus_state.mv, noti->vbus_state.ma, noti->vbus_state.type);
		/* enable/disable OTG power output */
#ifdef CONFIG_TCPC_RT1715
		pd_dpm_report_pd_source_vbus(di, (void *)(&noti->vbus_state));
		break;
	case TCP_NOTIFY_SOURCE_VCONN:
		dev_info(rpmd->dev, "%s:TCP_NOTIFY_SOURCE_VCONN\n", __func__);
		pd_dpm_report_pd_source_vconn((void *)(&noti->en_state.en));
#endif
		break;
	case TCP_NOTIFY_TYPEC_STATE:
		old_state = noti->typec_state.old_state;
		new_state = noti->typec_state.new_state;
		dev_info(rpmd->dev, "%s TCP_NOTIFY_TYPEC_STATE, polarity = %d, new_state=%d, old_state=%d\n",
				__func__, noti->typec_state.polarity, new_state, old_state);
		if (old_state == TYPEC_UNATTACHED &&
		    (new_state == TYPEC_ATTACHED_SNK ||
		     new_state == TYPEC_ATTACHED_NORP_SRC ||
		     new_state == TYPEC_ATTACHED_CUSTOM_SRC ||
		     new_state == TYPEC_ATTACHED_DBGACC_SNK ||
			 new_state == TYPEC_ATTACHED_VBUS_ONLY)) {
			dev_info(rpmd->dev, "%s Charger plug in, polarity = %d, new_state=%d, old_state=%d\n",
				__func__, noti->typec_state.polarity, new_state, old_state);
			/*
			 * start charger type detection,
			 * and enable device connection
			 */
			// tc_state.new_state = PD_DPM_TYPEC_ATTACHED_SNK;
			if (new_state == TYPEC_ATTACHED_VBUS_ONLY)
				tc_state.new_state = PD_DPM_TYPEC_ATTACHED_VBUS_ONLY;
			else if (new_state == TYPEC_ATTACHED_SNK)
				tc_state.new_state = PD_DPM_TYPEC_ATTACHED_SNK;
			else
				tc_state.new_state = PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC;
			pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
#ifdef NOT_USED_CODE
#ifdef CONFIG_TCPC_RT1715
			pd_dpm_set_source_sink_state(POWER_SUPPLY_START_SINK);
#endif
			cancel_delayed_work_sync(&rpmd->usb_dwork);
			rpmd->usb_dr = DR_DEVICE;
			rpmd->usb_type_polling_cnt = 0;
			schedule_delayed_work(&rpmd->usb_dwork,
					      msecs_to_jiffies(
					      USB_TYPE_POLLING_INTERVAL));
			typec_set_data_role(rpmd->typec_port, TYPEC_DEVICE);
			typec_set_pwr_role(rpmd->typec_port, TYPEC_SINK);
			typec_set_pwr_opmode(rpmd->typec_port,
					     noti->typec_state.rp_level -
					     TYPEC_CC_VOLT_SNK_DFT);
			typec_set_vconn_role(rpmd->typec_port, TYPEC_SINK);
#endif
		} else if ((old_state == TYPEC_ATTACHED_SNK ||
			    old_state == TYPEC_ATTACHED_NORP_SRC ||
			    old_state == TYPEC_ATTACHED_CUSTOM_SRC ||
			    old_state == TYPEC_ATTACHED_DBGACC_SNK ||
				old_state == TYPEC_ATTACHED_VBUS_ONLY) &&
			    (new_state == TYPEC_UNATTACHED ||
				new_state == TYPEC_UNATTACHED_VBUS_ONLY)) {
			dev_info(rpmd->dev, "%s Charger plug out, new_state=%d, old_state=%d\n", __func__, new_state, old_state);
			/*
			 * report charger plug-out,
			 * and disable device connection
			 */
			 if (new_state == TYPEC_UNATTACHED_VBUS_ONLY)
				tc_state.new_state = PD_DPM_TYPEC_UNATTACHED_VBUS_ONLY;
			else
				tc_state.new_state = PD_DPM_TYPEC_UNATTACHED;
			pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
#ifdef NOT_USED_CODE
#ifdef CONFIG_TCPC_RT1715
			pd_dpm_handle_abnomal_change(PD_DPM_UNATTACHED_VBUS_ONLY);
			pd_dpm_set_source_sink_state(POWER_SUPPLY_STOP_SINK);
#endif
			cancel_delayed_work_sync(&rpmd->usb_dwork);
			rpmd->usb_dr = DR_IDLE_DEVICE;
			schedule_delayed_work(&rpmd->usb_dwork, 0);
#endif
		} else if (old_state == TYPEC_UNATTACHED &&
			   (new_state == TYPEC_ATTACHED_SRC ||
			    new_state == TYPEC_ATTACHED_DEBUG)) {
			dev_info(rpmd->dev, "%s OTG plug in, polarity = %d\n",
				__func__, noti->typec_state.polarity);
			/* enable host connection */
			tc_state.new_state = PD_DPM_TYPEC_ATTACHED_SRC;
			pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
#ifdef NOT_USED_CODE
			cancel_delayed_work_sync(&rpmd->usb_dwork);
			rpmd->usb_dr = DR_HOST;
			schedule_delayed_work(&rpmd->usb_dwork, 0);
			typec_set_data_role(rpmd->typec_port, TYPEC_HOST);
			typec_set_pwr_role(rpmd->typec_port, TYPEC_SOURCE);
			switch (noti->typec_state.local_rp_level) {
			case TYPEC_RP_3_0:
				opmode = TYPEC_PWR_MODE_3_0A;
				break;
			case TYPEC_RP_1_5:
				opmode = TYPEC_PWR_MODE_1_5A;
				break;
			case TYPEC_RP_DFT:
			default:
				opmode = TYPEC_PWR_MODE_USB;
				break;
			}
			typec_set_pwr_opmode(rpmd->typec_port, opmode);
			typec_set_vconn_role(rpmd->typec_port, TYPEC_SOURCE);
#endif
		} else if ((old_state == TYPEC_ATTACHED_SRC ||
			    old_state == TYPEC_ATTACHED_DEBUG) &&
			    new_state == TYPEC_UNATTACHED) {
			dev_info(rpmd->dev, "%s OTG plug out\n", __func__);
			/* disable host connection */
			tc_state.new_state = PD_DPM_TYPEC_UNATTACHED;
			pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
#ifdef NOT_USED_CODE
			cancel_delayed_work_sync(&rpmd->usb_dwork);
			rpmd->usb_dr = DR_IDLE_HOST;
			schedule_delayed_work(&rpmd->usb_dwork, 0);
#endif
		} else if (old_state == TYPEC_UNATTACHED &&
			   new_state == TYPEC_ATTACHED_AUDIO) {
			dev_info(rpmd->dev, "%s Audio plug in\n", __func__);
			tc_state.new_state = PD_DPM_TYPEC_ATTACHED_AUDIO;
			pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
			/* enable AudioAccessory connection */
		} else if (old_state == TYPEC_ATTACHED_AUDIO &&
			   new_state == TYPEC_UNATTACHED) {
			dev_info(rpmd->dev, "%s Audio plug out\n", __func__);
			tc_state.new_state = PD_DPM_TYPEC_UNATTACHED;
			pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
			/* disable AudioAccessory connection */
		}

#ifdef NOT_USED_CODE
		if (new_state == TYPEC_UNATTACHED) {
			typec_unregister_partner(rpmd->partner);
			rpmd->partner = NULL;
			if (rpmd->typec_caps.prefer_role == TYPEC_SOURCE) {
				typec_set_data_role(rpmd->typec_port,
						    TYPEC_HOST);
				typec_set_pwr_role(rpmd->typec_port,
						   TYPEC_SOURCE);
				typec_set_pwr_opmode(rpmd->typec_port,
						     TYPEC_PWR_MODE_USB);
				typec_set_vconn_role(rpmd->typec_port,
						     TYPEC_SOURCE);
			} else {
				typec_set_data_role(rpmd->typec_port,
						    TYPEC_DEVICE);
				typec_set_pwr_role(rpmd->typec_port,
						   TYPEC_SINK);
				typec_set_pwr_opmode(rpmd->typec_port,
						     TYPEC_PWR_MODE_USB);
				typec_set_vconn_role(rpmd->typec_port,
						     TYPEC_SINK);
			}
		} else if (!rpmd->partner) {
			memset(&rpmd->partner_identity, 0,
			       sizeof(rpmd->partner_identity));
			rpmd->partner_desc.usb_pd = false;
			switch (new_state) {
			case TYPEC_ATTACHED_AUDIO:
				rpmd->partner_desc.accessory =
					TYPEC_ACCESSORY_AUDIO;
				break;
			case TYPEC_ATTACHED_DEBUG:
			case TYPEC_ATTACHED_DBGACC_SNK:
			case TYPEC_ATTACHED_CUSTOM_SRC:
				rpmd->partner_desc.accessory =
					TYPEC_ACCESSORY_DEBUG;
				break;
			default:
				rpmd->partner_desc.accessory =
					TYPEC_ACCESSORY_NONE;
				break;
			}
			rpmd->partner = typec_register_partner(rpmd->typec_port,
					&rpmd->partner_desc);
			if (IS_ERR(rpmd->partner)) {
				ret = PTR_ERR(rpmd->partner);
				dev_err(rpmd->dev,
				"%s typec register partner fail(%d)\n",
					__func__, ret);
			}
		}
#endif

		if (new_state == TYPEC_ATTACHED_SNK) {
			switch (noti->typec_state.rp_level) {
			/* SNK_RP_3P0 */
			case TYPEC_CC_VOLT_SNK_3_0:
				break;
			/* SNK_RP_1P5 */
			case TYPEC_CC_VOLT_SNK_1_5:
				break;
			/* SNK_RP_STD */
			case TYPEC_CC_VOLT_SNK_DFT:
			default:
				break;
			}
		} else if (new_state == TYPEC_ATTACHED_CUSTOM_SRC ||
			   new_state == TYPEC_ATTACHED_DBGACC_SNK) {
			switch (noti->typec_state.rp_level) {
			/* DAM_3000 */
			case TYPEC_CC_VOLT_SNK_3_0:
				break;
			/* DAM_1500 */
			case TYPEC_CC_VOLT_SNK_1_5:
				break;
			/* DAM_500 */
			case TYPEC_CC_VOLT_SNK_DFT:
			default:
				break;
			}
		} else if (new_state == TYPEC_ATTACHED_NORP_SRC) {
			/* Both CCs are open */
		}
		break;
	case TCP_NOTIFY_PR_SWAP:
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_PR_SWAP, NULL);
		dev_info(rpmd->dev, "%s power role swap, new role = %d\n",
				    __func__, noti->swap_state.new_role);
#ifdef NOT_USED_CODE
		if (noti->swap_state.new_role == PD_ROLE_SINK) {
			dev_info(rpmd->dev, "%s swap power role to sink\n",
					    __func__);
			/*
			 * report charger plug-in without charger type detection
			 * to not interfering with USB2.0 communication
			 */
			typec_set_pwr_role(rpmd->typec_port, TYPEC_SINK);
		} else if (noti->swap_state.new_role == PD_ROLE_SOURCE) {
			dev_info(rpmd->dev, "%s swap power role to source\n",
					    __func__);
			/* report charger plug-out */

			typec_set_pwr_role(rpmd->typec_port, TYPEC_SOURCE);
		}
#endif
		break;
	case TCP_NOTIFY_DR_SWAP:
		swap_state.new_role = noti->swap_state.new_role;
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_DR_SWAP, (void*)&swap_state);
		dev_info(rpmd->dev, "%s data role swap, new role = %d\n",
				    __func__, noti->swap_state.new_role);
#ifdef NOT_USED_CODE
		if (noti->swap_state.new_role == PD_ROLE_UFP) {
			dev_info(rpmd->dev, "%s swap data role to device\n",
					    __func__);
			/*
			 * disable host connection,
			 * and enable device connection
			 */
			cancel_delayed_work_sync(&rpmd->usb_dwork);
			rpmd->usb_dr = DR_HOST_TO_DEVICE;
			schedule_delayed_work(&rpmd->usb_dwork, 0);
			typec_set_data_role(rpmd->typec_port, TYPEC_DEVICE);
		} else if (noti->swap_state.new_role == PD_ROLE_DFP) {
			dev_info(rpmd->dev, "%s swap data role to host\n",
					    __func__);
			/*
			 * disable device connection,
			 * and enable host connection
			 */
			cancel_delayed_work_sync(&rpmd->usb_dwork);
			rpmd->usb_dr = DR_DEVICE_TO_HOST;
			schedule_delayed_work(&rpmd->usb_dwork, 0);
			typec_set_data_role(rpmd->typec_port, TYPEC_HOST);
		}
#endif
		break;

	case TCP_NOTIFY_VCONN_SWAP:
		dev_info(rpmd->dev, "%s vconn role swap, new role = %d\n",
			__func__, noti->swap_state.new_role);
		vconn_swap_flag = noti->swap_state.new_role;
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SOURCE_VCONN, &vconn_swap_flag);
#ifdef NOT_USED_CODE
		if (noti->swap_state.new_role) {
			dev_info(rpmd->dev, "%s swap vconn role to on\n",
					    __func__);
			typec_set_vconn_role(rpmd->typec_port, TYPEC_SOURCE);
		} else {
			dev_info(rpmd->dev, "%s swap vconn role to off\n",
					    __func__);
			typec_set_vconn_role(rpmd->typec_port, TYPEC_SINK);
		}
		break;
#endif
	case TCP_NOTIFY_EXT_DISCHARGE:
		dev_info(rpmd->dev, "%s ext discharge = %d\n",
				    __func__, noti->en_state.en);
		/* enable/disable VBUS discharge */
		break;
#ifdef NOT_USED_CODE
	case TCP_NOTIFY_PD_MODE:
		typec_set_pwr_opmode(rpmd->typec_port, TYPEC_PWR_MODE_PD);
		break;
#endif
	case TCP_NOTIFY_PD_STATE:
		dev_info(rpmd->dev, "%s pd state = %d\n",
				    __func__, noti->pd_state.connected);
		switch (noti->pd_state.connected) {
		case PD_CONNECT_NONE:
			break;
		case PD_CONNECT_HARD_RESET:
			break;
		case PD_CONNECT_PE_READY_SNK_APDO:
		case PD_CONNECT_PE_READY_SNK:
		case PD_CONNECT_PE_READY_SNK_PD30:
		case PD_CONNECT_PE_READY_SRC:
		case PD_CONNECT_PE_READY_SRC_PD30:
			/* /1000:mv-->v; -2:enumerated value of synchronous */
			cable_current_limit = (tcpm_inquire_cable_current(rpmd->tcpc) / 1000 - 2);
			cable_current_limit = (cable_current_limit < 0) ? 0 : cable_current_limit;
			dev_info(rpmd->dev, "%s cable current limit = %d\n",
				__func__, cable_current_limit);
			if (pd_dpm_support_voltage_up())
				tcpm_dpm_pd_request(rpmd->tcpc, VBUS_9000MV, IBUS_2000MA, NULL);
			/* enumerated value of synchronous charging */
			cable_vdo.cable_vdo = ((cable_current_limit << CABLE_CUR_CAP_SHIFT) & CABLE_CUR_CAP_MASK);
			cable_vdo.cable_vdo_ext = 0;
			pd_dpm_handle_pe_event(PD_DPM_PE_CABLE_VDO, (void *)(&cable_vdo));
			break;
		};
		break;
	case TCP_NOTIFY_HARD_RESET_STATE:
		dev_info(rpmd->dev, "%s Occurred HARD_RESET\n", __func__);
		switch (noti->hreset_state.state) {
		case TCP_HRESET_SIGNAL_SEND:
		case TCP_HRESET_SIGNAL_RECV:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	};
	return NOTIFY_OK;
}

#ifdef NOT_USED_CODE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int tcpc_typec_try_role(struct typec_port *port, int role)
{
	struct rt_pd_manager_data *rpmd = typec_get_drvdata(port);
#else
static int tcpc_typec_try_role(const struct typec_capability *cap, int role)
{
	struct rt_pd_manager_data *rpmd =
		container_of(cap, struct rt_pd_manager_data, typec_caps);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */
	uint8_t typec_role = TYPEC_ROLE_UNKNOWN;

	dev_info(rpmd->dev, "%s role = %d\n", __func__, role);

	switch (role) {
	case TYPEC_NO_PREFERRED_ROLE:
		typec_role = TYPEC_ROLE_DRP;
		break;
	case TYPEC_SINK:
		typec_role = TYPEC_ROLE_TRY_SNK;
		break;
	case TYPEC_SOURCE:
		typec_role = TYPEC_ROLE_TRY_SRC;
		break;
	default:
		return 0;
	}

	return tcpm_typec_change_role_postpone(rpmd->tcpc, typec_role, true);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int tcpc_typec_dr_set(struct typec_port *port, enum typec_data_role role)
{
	struct rt_pd_manager_data *rpmd = typec_get_drvdata(port);
#else
static int tcpc_typec_dr_set(const struct typec_capability *cap,
			     enum typec_data_role role)
{
	struct rt_pd_manager_data *rpmd =
		container_of(cap, struct rt_pd_manager_data, typec_caps);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */
	int ret = 0;
	uint8_t data_role = tcpm_inquire_pd_data_role(rpmd->tcpc);
	bool do_swap = false;

	dev_info(rpmd->dev, "%s role = %d\n", __func__, role);

	if (role == TYPEC_HOST) {
		if (data_role == PD_ROLE_UFP) {
			do_swap = true;
			data_role = PD_ROLE_DFP;
		}
	} else if (role == TYPEC_DEVICE) {
		if (data_role == PD_ROLE_DFP) {
			do_swap = true;
			data_role = PD_ROLE_UFP;
		}
	} else {
		dev_err(rpmd->dev, "%s invalid role\n", __func__);
		return -EINVAL;
	}

	if (do_swap) {
		ret = tcpm_dpm_pd_data_swap(rpmd->tcpc, data_role, NULL);
		if (ret != TCPM_SUCCESS) {
			dev_err(rpmd->dev, "%s data role swap fail(%d)\n",
					   __func__, ret);
			return -EPERM;
		}
	}

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int tcpc_typec_pr_set(struct typec_port *port, enum typec_role role)
{
	struct rt_pd_manager_data *rpmd = typec_get_drvdata(port);
#else
static int tcpc_typec_pr_set(const struct typec_capability *cap,
			     enum typec_role role)
{
	struct rt_pd_manager_data *rpmd =
		container_of(cap, struct rt_pd_manager_data, typec_caps);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */
	int ret = 0;
	uint8_t power_role = tcpm_inquire_pd_power_role(rpmd->tcpc);
	bool do_swap = false;

	dev_info(rpmd->dev, "%s role = %d\n", __func__, role);

	if (role == TYPEC_SOURCE) {
		if (power_role == PD_ROLE_SINK) {
			do_swap = true;
			power_role = PD_ROLE_SOURCE;
		}
	} else if (role == TYPEC_SINK) {
		if (power_role == PD_ROLE_SOURCE) {
			do_swap = true;
			power_role = PD_ROLE_SINK;
		}
	} else {
		dev_err(rpmd->dev, "%s invalid role\n", __func__);
		return -EINVAL;
	}

	if (do_swap) {
		ret = tcpm_dpm_pd_power_swap(rpmd->tcpc, power_role, NULL);
		if (ret == TCPM_ERROR_NO_PD_CONNECTED)
			ret = tcpm_typec_role_swap(rpmd->tcpc);
		if (ret != TCPM_SUCCESS) {
			dev_err(rpmd->dev, "%s power role swap fail(%d)\n",
					   __func__, ret);
			return -EPERM;
		}
	}

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int tcpc_typec_vconn_set(struct typec_port *port, enum typec_role role)
{
	struct rt_pd_manager_data *rpmd = typec_get_drvdata(port);
#else
static int tcpc_typec_vconn_set(const struct typec_capability *cap,
				enum typec_role role)
{
	struct rt_pd_manager_data *rpmd =
		container_of(cap, struct rt_pd_manager_data, typec_caps);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */
	int ret = 0;
	uint8_t vconn_role = tcpm_inquire_pd_vconn_role(rpmd->tcpc);
	bool do_swap = false;

	dev_info(rpmd->dev, "%s role = %d\n", __func__, role);

	if (role == TYPEC_SOURCE) {
		if (vconn_role == PD_ROLE_VCONN_OFF) {
			do_swap = true;
			vconn_role = PD_ROLE_VCONN_ON;
		}
	} else if (role == TYPEC_SINK) {
		if (vconn_role != PD_ROLE_VCONN_OFF) {
			do_swap = true;
			vconn_role = PD_ROLE_VCONN_OFF;
		}
	} else {
		dev_err(rpmd->dev, "%s invalid role\n", __func__);
		return -EINVAL;
	}

	if (do_swap) {
		ret = tcpm_dpm_pd_vconn_swap(rpmd->tcpc, vconn_role, NULL);
		if (ret != TCPM_SUCCESS) {
			dev_err(rpmd->dev, "%s vconn role swap fail(%d)\n",
					   __func__, ret);
			return -EPERM;
		}
	}

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static int tcpc_typec_port_type_set(struct typec_port *port,
				    enum typec_port_type type)
{
	struct rt_pd_manager_data *rpmd = typec_get_drvdata(port);
	const struct typec_capability *cap = &rpmd->typec_caps;
#else
static int tcpc_typec_port_type_set(const struct typec_capability *cap,
				    enum typec_port_type type)
{
	struct rt_pd_manager_data *rpmd =
		container_of(cap, struct rt_pd_manager_data, typec_caps);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */
	bool as_sink = tcpc_typec_is_act_as_sink_role(rpmd->tcpc);
	uint8_t typec_role = TYPEC_ROLE_UNKNOWN;

	dev_info(rpmd->dev, "%s type = %d, as_sink = %d\n",
			    __func__, type, as_sink);

	switch (type) {
	case TYPEC_PORT_SNK:
		if (as_sink)
			return 0;
		break;
	case TYPEC_PORT_SRC:
		if (!as_sink)
			return 0;
		break;
	case TYPEC_PORT_DRP:
		if (cap->prefer_role == TYPEC_SOURCE)
			typec_role = TYPEC_ROLE_TRY_SRC;
		else if (cap->prefer_role == TYPEC_SINK)
			typec_role = TYPEC_ROLE_TRY_SNK;
		else
			typec_role = TYPEC_ROLE_DRP;
		return tcpm_typec_change_role(rpmd->tcpc, typec_role);
	default:
		return 0;
	}

	return tcpm_typec_role_swap(rpmd->tcpc);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
const struct typec_operations tcpc_typec_ops = {
	.try_role = tcpc_typec_try_role,
	.dr_set = tcpc_typec_dr_set,
	.pr_set = tcpc_typec_pr_set,
	.vconn_set = tcpc_typec_vconn_set,
	.port_type_set = tcpc_typec_port_type_set,
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */

static int typec_init(struct rt_pd_manager_data *rpmd)
{
	int ret = 0;

	rpmd->typec_caps.type = TYPEC_PORT_DRP;
	rpmd->typec_caps.data = TYPEC_PORT_DRD;
	rpmd->typec_caps.revision = 0x0120;
	rpmd->typec_caps.pd_revision = 0x0316;
	switch (rpmd->tcpc->desc.role_def) {
	case TYPEC_ROLE_SRC:
	case TYPEC_ROLE_TRY_SRC:
		rpmd->typec_caps.prefer_role = TYPEC_SOURCE;
		break;
	case TYPEC_ROLE_SNK:
	case TYPEC_ROLE_TRY_SNK:
		rpmd->typec_caps.prefer_role = TYPEC_SINK;
		break;
	default:
		rpmd->typec_caps.prefer_role = TYPEC_NO_PREFERRED_ROLE;
		break;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	rpmd->typec_caps.driver_data = rpmd;
	rpmd->typec_caps.ops = &tcpc_typec_ops;
#else
	rpmd->typec_caps.try_role = tcpc_typec_try_role;
	rpmd->typec_caps.dr_set = tcpc_typec_dr_set;
	rpmd->typec_caps.pr_set = tcpc_typec_pr_set;
	rpmd->typec_caps.vconn_set = tcpc_typec_vconn_set;
	rpmd->typec_caps.port_type_set = tcpc_typec_port_type_set;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) */

	rpmd->typec_port = typec_register_port(rpmd->dev, &rpmd->typec_caps);
	if (IS_ERR(rpmd->typec_port)) {
		ret = PTR_ERR(rpmd->typec_port);
		dev_err(rpmd->dev, "%s typec register port fail(%d)\n",
				   __func__, ret);
		goto out;
	}

	rpmd->partner_desc.identity = &rpmd->partner_identity;
out:
	return ret;
}
#endif

#ifdef CONFIG_USB_POWER_DELIVERY
#ifdef CONFIG_RECV_BAT_ABSENT_NOTIFY
static int fg_bat_notifier_call(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct pd_port *pd_port = container_of(nb, struct pd_port, fg_bat_nb);
	struct tcpc_device *tcpc = pd_port->tcpc;

	switch (event) {
	case EVENT_BATTERY_PLUG_OUT:
		dev_info(&tcpc->dev, "%s: fg battery absent\n", __func__);
		schedule_work(&pd_port->fg_bat_work);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}
#endif /* CONFIG_RECV_BAT_ABSENT_NOTIFY */
#endif /* CONFIG_USB_POWER_DELIVERY */

static int __tcpc_class_complete_work(struct device *dev, void *data)
{
	struct tcpc_device *tcpc = dev_get_drvdata(dev);
#ifdef CONFIG_USB_POWER_DELIVERY
#ifdef CONFIG_RECV_BAT_ABSENT_NOTIFY
	struct notifier_block *fg_bat_nb = &tcpc->pd_port.fg_bat_nb;
	int ret = 0;
#endif /* CONFIG_RECV_BAT_ABSENT_NOTIFY */
#endif /* CONFIG_USB_POWER_DELIVERY */

	if (tcpc != NULL) {
		pr_info("%s = %s\n", __func__, dev_name(dev));
		tcpc_device_irq_enable(tcpc);
#ifdef CONFIG_USB_POWER_DELIVERY
#ifdef CONFIG_RECV_BAT_ABSENT_NOTIFY
		fg_bat_nb->notifier_call = fg_bat_notifier_call;
		ret = register_battery_notifier(fg_bat_nb);
		if (ret < 0) {
			pr_err("%s: register bat notifier fail\n", __func__);
			return -EINVAL;
		}
#endif /* CONFIG_RECV_BAT_ABSENT_NOTIFY */
#endif /* CONFIG_USB_POWER_DELIVERY */
	}
	return 0;
}

int tcpc_class_complete_init(void)
{
	if (!IS_ERR(tcpc_class)) {
		class_for_each_device(tcpc_class, NULL, NULL,
			__tcpc_class_complete_work);
	}
	return 0;
}

#ifdef CONFIG_TCPC_RT1715
void tcpc_usb_typec_cc_alert(uint8_t cc1, uint8_t cc2)
{
	pr_info("%s CC_status: %d--%d\n", __func__, cc1, cc2);
#ifdef CONFIG_USB_PROXY_HCD
	check_hifi_usb_status(HIFI_USB_TCPC);
#endif /* CONFIG_USB_PROXY_HCD */
}

static void rt_pd_manager_parse_dts(struct rt_pd_manager_data *di, struct device_node *np)
{
	if (!di || !np) {
		pr_err("di or np is null\n");
		return;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hw_combphy_detect", &di->combphy_detect, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ctc_ocp_flag", &di->ctc_ocp_flag, 0);
	pr_info("%s parse_dts success\n", __func__);
}
#endif

static int rt_pd_manager_probe(struct platform_device *pdev)
{
	static int probe_cnt;
	int ret = 0;
	struct rt_pd_manager_data *rpmd = NULL;
#ifdef CONFIG_TCPC_RT1715
	struct device_node *np = NULL;
#endif

	dev_info(&pdev->dev, "%s (%s) probe_cnt = %d\n",
			     __func__, RT_PD_MANAGER_VERSION, ++probe_cnt);

	rpmd = devm_kzalloc(&pdev->dev, sizeof(*rpmd), GFP_KERNEL);
	if (!rpmd)
		return -ENOMEM;

	rpmd->dev = &pdev->dev;
#ifdef CONFIG_TCPC_RT1715
	np = rpmd->dev->of_node;
	rt_pd_manager_parse_dts(rpmd, np);
#endif
	rpmd->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (!rpmd->tcpc) {
		dev_err(rpmd->dev, "%s get tcpc dev fail\n", __func__);
		ret = -EPROBE_DEFER;
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_get_tcpc_dev;
	}

	INIT_DELAYED_WORK(&rpmd->usb_dwork, usb_dwork_handler);
	INIT_DELAYED_WORK(&rpmd->usb_init_work, usb_init_handler);
	rpmd->usb_dr = DR_IDLE_DEVICE;
	rpmd->usb_type_polling_cnt = 0;

#ifdef NOT_USED_CODE
	ret = typec_init(rpmd);
	if (ret < 0) {
		dev_err(rpmd->dev, "%s init typec fail(%d)\n", __func__, ret);
		ret = -EPROBE_DEFER;
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_init_typec;
	}
#endif

	rpmd->pd_nb.notifier_call = pd_tcp_notifier_call;
	ret = register_tcp_dev_notifier(rpmd->tcpc, &rpmd->pd_nb,
					TCP_NOTIFY_TYPE_ALL);
	if (ret < 0) {
		dev_err(rpmd->dev, "%s register tcpc notifier fail(%d)\n",
				   __func__, ret);
		ret = -EPROBE_DEFER;
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_reg_tcpc_notifier;
	}
#ifdef CONFIG_TCPC_RT1715
	if (rpmd->combphy_detect)
		schedule_delayed_work(&rpmd->usb_init_work, msecs_to_jiffies(USB_INIT_WORK_DEALY));
	else
		schedule_delayed_work(&rpmd->usb_init_work, msecs_to_jiffies(4 * USB_INIT_WORK_DEALY));
#else
	tcpc_class_complete_init();
#endif
out:
	platform_set_drvdata(pdev, rpmd);
	boost_5v_enable(1, BOOST_CTRL_PD_VCONN);
	dev_info(rpmd->dev, "%s %s!!\n", __func__, ret == -EPROBE_DEFER ?
			    "Over probe cnt max" : "OK");
	return 0;

err_reg_tcpc_notifier:
	// typec_unregister_port(rpmd->typec_port);
// err_init_typec:
err_get_tcpc_dev:
// err_init_extcon:
	return ret;
}

static int rt_pd_manager_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct rt_pd_manager_data *rpmd = platform_get_drvdata(pdev);

	if (!rpmd)
		return -EINVAL;

	ret = unregister_tcp_dev_notifier(rpmd->tcpc, &rpmd->pd_nb,
					  TCP_NOTIFY_TYPE_ALL);
	if (ret < 0)
		dev_err(rpmd->dev, "%s unregister tcpc notifier fail(%d)\n",
				   __func__, ret);
	// typec_unregister_port(rpmd->typec_port);

	return ret;
}

static const struct of_device_id rt_pd_manager_of_match[] = {
	{ .compatible = "richtek,rt_pd_manager" },
	{ }
};
MODULE_DEVICE_TABLE(of, rt_pd_manager_of_match);

static struct platform_driver rt_pd_manager_driver = {
	.driver = {
		.name = "rt_pd_manager",
		.of_match_table = of_match_ptr(rt_pd_manager_of_match),
	},
	.probe = rt_pd_manager_probe,
	.remove = rt_pd_manager_remove,
};

module_platform_driver(rt_pd_manager_driver);

MODULE_AUTHOR("Jeff Chang");
MODULE_DESCRIPTION("Richtek pd manager driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(RT_PD_MANAGER_VERSION);

/*
 * Release Note
 * 1.0.0
 * (1) Bump version to 1.0.0
 *
 * 0.0.9
 * (1) Add more information for source vbus log
 *
 * 0.0.8
 * (1) Add support for msm-5.4
 *
 * 0.0.7
 * (1) Set properties of usb_psy
 *
 * 0.0.6
 * (1) Register typec_port
 *
 * 0.0.5
 * (1) Control USB mode in delayed work
 * (2) Remove param_lock because pd_tcp_notifier_call() is single-entry
 *
 * 0.0.4
 * (1) Limit probe count
 *
 * 0.0.3
 * (1) Add extcon for controlling USB mode
 *
 * 0.0.2
 * (1) Initialize old_state and new_state
 *
 * 0.0.1
 * (1) Add all possible notification events
 */
