/*
 * hw_pd_usb.c
 *
 * Source file of usb interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <chipset_common/hwusb/hw_usb.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_core.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_state.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/usb/switch/usbswitch_common.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/dp_aux_switch/dp_aux_switch.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_usb_con
HWLOG_REGIST();
#endif /* HWLOG_TAG */

/* used for combphy event process */
#define COMBPHY_MAX_PD_EVENT_COUNT              40
#define COMBPHY_PD_EVENT_INVALID_VAL            (-1)
#define ACK_BUSY_MAX_COUNT                      10

#ifdef CONFIG_CONTEXTHUB_PD
#define hw_pd_wait_combphy_configdone() do { \
	wait_for_completion_timeout(&g_pd_combphy_configdone_completion, msecs_to_jiffies(11500)); \
	reinit_completion(&g_pd_combphy_configdone_completion); \
	} while (0)

static struct pd_dpm_combphy_event g_last_combphy_notify_event;
static struct mutex g_pd_combphy_notify_lock;
static struct workqueue_struct *g_pd_combphy_wq;
static struct delayed_work g_pd_combphy_event_work;
static struct completion g_pd_combphy_configdone_completion;
static struct pd_dpm_combphy_event g_combphy_event_buffer[COMBPHY_MAX_PD_EVENT_COUNT];

static int g_combphy_ack;
static int g_combphy_iput;
static int g_combphy_iget;
static int g_combphy_event_num;

void hw_pd_set_combophy_ready_status(void)
{
	pd_state(combophy_ready_flag) = true;
}

void hw_pd_set_combphy_mode(TCPC_MUX_CTRL_TYPE mode)
{
	pd_state(combphy_mode) = mode;
}

void hw_pd_set_last_hpd_status(bool hpd_status)
{
	pd_state(last_hpd_status) = hpd_status;
}

static inline int addring (int i)
{
	return ((i + 1) == COMBPHY_MAX_PD_EVENT_COUNT) ? 0 : (i + 1);
}

static void hw_pd_init_combphy_buffer(void)
{
	int i;

	for (i = 0; i < COMBPHY_MAX_PD_EVENT_COUNT; i++) {
		g_combphy_event_buffer[i].irq_type = COMBPHY_PD_EVENT_INVALID_VAL;
		g_combphy_event_buffer[i].mode_type = COMBPHY_PD_EVENT_INVALID_VAL;
		g_combphy_event_buffer[i].dev_type = COMBPHY_PD_EVENT_INVALID_VAL;
		g_combphy_event_buffer[i].typec_orien = COMBPHY_PD_EVENT_INVALID_VAL;
	}
}

static void hw_pd_combphy_event_copy(struct pd_dpm_combphy_event *dst_event,
	struct pd_dpm_combphy_event src_event)
{
	(*dst_event).dev_type = src_event.dev_type;
	(*dst_event).irq_type = src_event.irq_type;
	(*dst_event).mode_type = src_event.mode_type;
	(*dst_event).typec_orien = src_event.typec_orien;
}

static void hw_pd_print_combphy_buffer(int idx)
{
#ifdef COMBPHY_NOTIFY_BUFFER_PRINT
	hwlog_info("\n+++++++++++++++++++++++++++++++++\n");
	hwlog_info("\nbuffer[%d].irq_type %d\n", idx,
		g_combphy_event_buffer[idx].irq_type);
	hwlog_info("\nbuffer[%d].mode_type %d\n", idx,
		g_combphy_event_buffer[idx].mode_type);
	hwlog_info("\nbuffer[%d].dev_type %d\n", idx,
		g_combphy_event_buffer[idx].dev_type);
	hwlog_info("\nbuffer[%d].typec_orien %d\n", idx,
		g_combphy_event_buffer[idx].typec_orien);
	hwlog_info("\n+++++++++++++++++++++++++++++++++\n");
#endif /* COMBPHY_NOTIFY_BUFFER_PRINT */
}

static int hw_pd_put_combphy_event(struct pd_dpm_combphy_event event)
{
	if (g_combphy_event_num < COMBPHY_MAX_PD_EVENT_COUNT) {
		hw_pd_combphy_event_copy(&(g_combphy_event_buffer[g_combphy_iput]), event);
		hw_pd_print_combphy_buffer(g_combphy_iput);
		g_combphy_iput = addring(g_combphy_iput);
		g_combphy_event_num++;
		hwlog_info("%s - input = %d, combphy_event_num = %d \n",
			__func__, g_combphy_iput, g_combphy_event_num);
		return 0;
	} else {
		hwlog_info("%s Buffer is full\n", __func__);
		return HW_PD_INVALID_STATE;
	}
}

int hw_pd_get_combphy_event_num(void)
{
	return g_combphy_event_num;
}

static int hw_pd_get_combphy_event(struct pd_dpm_combphy_event *event)
{
	int pos;

	if (g_combphy_event_num > 0) {
		pos = g_combphy_iget;
		g_combphy_iget = addring(g_combphy_iget);
		g_combphy_event_num--;
		hw_pd_combphy_event_copy(event, g_combphy_event_buffer[pos]);
		hw_pd_print_combphy_buffer(pos);
		hwlog_info("%s:iget=%d, num = %d\n", __func__, g_combphy_iget, g_combphy_event_num);
	} else {
		hwlog_info("%s Buffer is empty\n", __func__);
		return HW_PD_INVALID_STATE;
	}

	return g_combphy_event_num;
}

void hw_pd_combphy_config_done(TCPC_MUX_CTRL_TYPE mode_type, int ack)
{
	g_combphy_ack = ack;
	hwlog_info("%s: mode=%d, ret=%d \n", __func__, (int)mode_type, g_combphy_ack);
	complete(&g_pd_combphy_configdone_completion);
}

/* enable/disable combphy vbus control */
static void hw_pd_ldo_supply_ctrl(struct pd_dpm_combphy_event event, bool enable)
{
	if ((event.mode_type != TCPC_NC) && (enable == true))
		hw_usb_ldo_supply_enable(HW_USB_LDO_CTRL_COMBOPHY);

	if ((g_combphy_ack == 0) && (event.mode_type == TCPC_NC) && (enable == false))
		hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_COMBOPHY);
}

/* notify valid event to combphy */
static void hw_pd_combphy_valid_event(struct pd_dpm_combphy_event event)
{
	int busy_count = ACK_BUSY_MAX_COUNT;

	hwlog_info("%s +\n", __func__);

	hw_pd_ldo_supply_ctrl(event, true);
	pd_event_notify(event.irq_type, event.mode_type, event.dev_type, event.typec_orien);
	hw_pd_wait_combphy_configdone();
	if (g_combphy_ack == -EBUSY) {
		do {
			mdelay(100); /* 100: delay 100ms */
			busy_count--;
			pd_event_notify(event.irq_type, event.mode_type, event.dev_type, event.typec_orien);
			hw_pd_wait_combphy_configdone();
			if (g_combphy_ack != -EBUSY) {
				hwlog_info("%s %d exit busy succ\n", __func__, __LINE__);
				break;
			}
		} while (busy_count != 0);
		if (busy_count == 0)
			hwlog_err("%s %d BUSY!\n", __func__, __LINE__);
	}
	hw_pd_ldo_supply_ctrl(event, false);
	hwlog_info("%s -\n", __func__);
}

/* get event and determine the validity of an event  */
static void hw_pd_combphy_event_notify(struct work_struct *work)
{
	int event_count = 0;
	struct pd_dpm_combphy_event event;

	hwlog_info("%s +\n", __func__);

	do {
		mutex_lock(&g_pd_combphy_notify_lock);
		event_count = hw_pd_get_combphy_event(&event);
		mutex_unlock(&g_pd_combphy_notify_lock);

		if (event_count < 0)
			break;

		if (!pd_dpm_support_dp() && ((event.dev_type == TCA_DP_OUT) ||
			(event.dev_type == TCA_DP_IN)))
			continue;

		if ((event.irq_type == COMBPHY_PD_EVENT_INVALID_VAL) ||
			(event.mode_type == COMBPHY_PD_EVENT_INVALID_VAL) ||
			(event.dev_type == COMBPHY_PD_EVENT_INVALID_VAL) ||
			(event.typec_orien == COMBPHY_PD_EVENT_INVALID_VAL)) {
			hwlog_err("%s invalid val\n", __func__);
		} else {
			hw_pd_combphy_valid_event(event);
		}
	} while (event_count);
	hwlog_info("%s -\n", __func__);
}

static bool hw_pd_combphy_event_compare(struct pd_dpm_combphy_event eventa,
	struct pd_dpm_combphy_event eventb)
{
	return ((eventa.dev_type == eventb.dev_type) &&
		(eventa.irq_type == eventb.irq_type) &&
		(eventa.mode_type == eventb.mode_type));
}

int hw_pd_handle_combphy_event(struct pd_dpm_combphy_event event)
{
	int ret;

	hwlog_info("%s +\n", __func__);
	mutex_lock(&g_pd_combphy_notify_lock);
	if ((g_last_combphy_notify_event.dev_type == TCA_DP_IN) ||
		(g_last_combphy_notify_event.dev_type == TCA_DP_OUT) ||
		(g_last_combphy_notify_event.dev_type == TCA_ID_FALL_EVENT)) {
		if ((event.dev_type == TCA_CHARGER_CONNECT_EVENT) ||
			(event.dev_type == TCA_CHARGER_DISCONNECT_EVENT)) {
			hwlog_info("%s invlid event sequence\n", __func__);
			mutex_unlock(&g_pd_combphy_notify_lock);
			return HW_PD_INVALID_STATE;
		}
	}
	if ((g_last_combphy_notify_event.mode_type ==
		TCPC_NC) && (event.mode_type == TCPC_NC)) {
		hwlog_info("%s repeated TCPC_NC event\n", __func__);
		mutex_unlock(&g_pd_combphy_notify_lock);
		return HW_PD_INVALID_STATE;
	}

	if (!hw_pd_combphy_event_compare(g_last_combphy_notify_event, event)) {
		hw_pd_combphy_event_copy(&(g_last_combphy_notify_event), event);
		ret = hw_pd_put_combphy_event(event);
		if (ret < 0) {
			hwlog_err("%s hw_pd_put_combphy_event fail\n", __func__);
			mutex_unlock(&g_pd_combphy_notify_lock);
			return HW_PD_INVALID_STATE;
		}

		queue_delayed_work(g_pd_combphy_wq, &g_pd_combphy_event_work, msecs_to_jiffies(0));
	} else {
		hwlog_info("%s Pending event is same --> ignore this event\n", __func__);
	}
	mutex_unlock(&g_pd_combphy_notify_lock);
	hwlog_info("%s -\n", __func__);
	return 0;
}
#endif /* CONFIG_CONTEXTHUB_PD */

void hw_pd_usb_host_on(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;

	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_state(cc_orientation);
	pd_dpm_handle_combphy_event(event);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void hw_pd_usb_host_off(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;

	event.typec_orien = pd_state(cc_orientation);
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	pd_dpm_set_combphy_status(TCPC_NC);
	pd_dpm_handle_combphy_event(event);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void hw_pd_report_device_attach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_CHARGER_CONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_state(cc_orientation);
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(CHARGER_CONNECT_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */

	water_detection_entrance();
}

void hw_pd_report_host_attach(void)
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
	event.typec_orien = pd_state(cc_orientation);
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(ID_FALL_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void hw_pd_report_device_detach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_CHARGER_DISCONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	event.typec_orien = pd_state(cc_orientation);
	pd_dpm_handle_combphy_event(event);
#else
	chip_usb_otg_event(CHARGER_DISCONNECT_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void hw_pd_report_host_detach(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif /* CONFIG_CONTEXTHUB_PD */

	hwlog_info("%s \r\n", __func__);

#ifdef CONFIG_SWITCH_FSA9685
	usbswitch_common_dcd_timeout_enable(false);
#endif /* CONFIG_SWITCH_FSA9685 */

#ifdef CONFIG_CONTEXTHUB_PD
	event.typec_orien = pd_state(cc_orientation);
	event.mode_type = pd_state(combphy_mode);
	hwlog_info("%s:mode_type=%d,typec_orien=%d\n",
		__func__, event.mode_type, event.typec_orien);
	if (pd_state(last_hpd_status)) {
		event.dev_type = TCA_DP_OUT;
		event.irq_type = TCA_IRQ_HPD_OUT;
		pd_dpm_handle_combphy_event(event);
		pd_dpm_set_last_hpd_status(false);
		hwlog_info("%s 1:mode_type=%d,typec_orien=%d\n",
			__func__, event.mode_type, event.typec_orien);
		hw_usb_send_event(DP_CABLE_OUT_EVENT);
	}
	hwlog_info("%s 2:mode_type=%d,typec_orien=%d\n",
		__func__, event.mode_type, event.typec_orien);
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	pd_dpm_set_combphy_status(TCPC_NC);
	pd_dpm_handle_combphy_event(event);
	/* set aux uart switch low */
	if (pd_dpm_support_dp())
		dp_aux_uart_switch_disable();
#else
	chip_usb_otg_event(ID_RISE_EVENT);
#endif /* CONFIG_CONTEXTHUB_PD */
}

void hw_pd_usb_connect_init(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	mutex_init(&g_pd_combphy_notify_lock);
	hw_pd_init_combphy_buffer();
	g_pd_combphy_wq = create_workqueue("pd_combphy_event_notify_workque");
	INIT_DELAYED_WORK(&g_pd_combphy_event_work, hw_pd_combphy_event_notify);
	init_completion(&g_pd_combphy_configdone_completion);
#endif /* CONFIG_CONTEXTHUB_PD */
}