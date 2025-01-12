/*
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/version.h>
#include <uapi/linux/sched/types.h>

#include "include/tcpci.h"
#include "include/tcpci_timer.h"
#include "include/tcpci_typec.h"

#define RT_MASK64(i)			(((uint64_t)1) << (i))

#define TIMEOUT_VAL(val)		((val) * 1000)
#define TIMEOUT_RANGE(min, max)		(((min) * 4000 + (max) * 1000) / 5)
#define TIMEOUT_VAL_US(val)		(val)

static uint64_t tcpc_get_timer_enable_mask(struct tcpc_device *tcpc)
{
	uint64_t data;
	unsigned long flags;

	down(&tcpc->timer_enable_mask_lock);
	raw_local_irq_save(flags);
	data = tcpc->timer_enable_mask;
	raw_local_irq_restore(flags);
	up(&tcpc->timer_enable_mask_lock);
	return data;
}

static void tcpc_reset_timer_enable_mask(struct tcpc_device *tcpc)
{
	unsigned long flags;

	down(&tcpc->timer_enable_mask_lock);
	raw_local_irq_save(flags);
	tcpc->timer_enable_mask = 0;
	raw_local_irq_restore(flags);
	up(&tcpc->timer_enable_mask_lock);
}

static void tcpc_clear_timer_enable_mask(struct tcpc_device *tcpc, uint32_t nr)
{
	unsigned long flags;

	down(&tcpc->timer_enable_mask_lock);
	raw_local_irq_save(flags);
	tcpc->timer_enable_mask &= ~RT_MASK64(nr);
	raw_local_irq_restore(flags);
	up(&tcpc->timer_enable_mask_lock);
}

static void tcpc_set_timer_enable_mask(struct tcpc_device *tcpc, uint32_t nr)
{
	unsigned long flags;

	down(&tcpc->timer_enable_mask_lock);
	raw_local_irq_save(flags);
	tcpc->timer_enable_mask |= RT_MASK64(nr);
	raw_local_irq_restore(flags);
	up(&tcpc->timer_enable_mask_lock);
}

static uint64_t tcpc_get_timer_tick(struct tcpc_device *tcpc)
{
	uint64_t data;
	unsigned long flags;

	spin_lock_irqsave(&tcpc->timer_tick_lock, flags);
	data = tcpc->timer_tick;
	spin_unlock_irqrestore(&tcpc->timer_tick_lock, flags);
	return data;
}

static void tcpc_clear_timer_tick(struct tcpc_device *tcpc, uint32_t nr)
{
	unsigned long flags;

	spin_lock_irqsave(&tcpc->timer_tick_lock, flags);
	tcpc->timer_tick &= ~RT_MASK64(nr);
	spin_unlock_irqrestore(&tcpc->timer_tick_lock, flags);
}

static void tcpc_set_timer_tick(struct tcpc_device *tcpc, uint32_t nr)
{
	unsigned long flags;

	spin_lock_irqsave(&tcpc->timer_tick_lock, flags);
	tcpc->timer_tick |= RT_MASK64(nr);
	spin_unlock_irqrestore(&tcpc->timer_tick_lock, flags);
}

static const char * const hisi_tcpc_timer_name[] = {
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	"PD_TIMER_BIST_CONT_MODE",
	"PD_TIMER_DISCOVER_ID",
	"PD_TIMER_HARD_RESET_COMPLETE",
	"PD_TIMER_NO_RESPONSE",
	"PD_TIMER_PS_HARD_RESET",
	"PD_TIMER_PS_SOURCE_OFF",
	"PD_TIMER_PS_SOURCE_ON",
	"PD_TIMER_PS_TRANSITION",
	"PD_TIMER_SENDER_RESPONSE",
	"PD_TIMER_SINK_ACTIVITY",
	"PD_TIMER_SINK_REQUEST",
	"PD_TIMER_SINK_WAIT_CAP",
	"PD_TIMER_SOURCE_ACTIVITY",
	"PD_TIMER_SOURCE_CAPABILITY",
	"PD_TIMER_SOURCE_START",
	"PD_TIMER_VCONN_ON",
	"PD_TIMER_VDM_MODE_ENTRY",
	"PD_TIMER_VDM_MODE_EXIT",
	"PD_TIMER_VDM_RESPONSE",
	"PD_TIMER_SOURCE_TRANSITION",
	"PD_TIMER_SRC_RECOVER",
	"PD_TIMER_VSAFE0V_DELAY",
	"PD_TIMER_VSAFE0V_TOUT",
	"PD_TIMER_DISCARD",
	"PD_TIMER_VBUS_STABLE",
	"PD_TIMER_VBUS_PRESENT",
	"PD_TIMER_UVDM_RESPONSE",
	"PD_TIMER_DFP_FLOW_DELAY",
	"PD_TIMER_UFP_FLOW_DELAY",
	"PD_PE_VDM_POSTPONE",
	"PD_TIMER_PE_IDLE_TOUT",
	"PD_TIMER_SRC_CAP_EXT_TOUT",

	"TYPEC_RT_TIMER_PE_IDLE",
	"TYPEC_RT_TIMER_SAFE0V_DELAY",
	"TYPEC_RT_TIMER_SAFE0V_TOUT",
	"TYPEC_RT_TIMER_SAFE5V_TOUT",
	"TYPEC_RT_TIMER_ROLE_SWAP_START",
	"TYPEC_RT_TIMER_ROLE_SWAP_STOP",
	"TYPEC_RT_TIMER_LEGACY",
	"TYPEC_RT_TIMER_NOT_LEGACY",

	"TYPEC_TRY_TIMER_DRP_TRY",
	"TYPEC_TRY_TIMER_DRP_TRYWAIT",

	"TYPEC_TIMER_CCDEBOUNCE",
	"TYPEC_TIMER_PDDEBOUNCE",
	"TYPEC_TIMER_ERROR_RECOVERY",
	"TYPEC_TIMER_DRP_SRC_TOGGLE",
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	"TYPEC_TIMER_NORP_SRC",
#endif
#else
	"TYPEC_RT_TIMER_SAFE0V_DELAY",
	"TYPEC_RT_TIMER_SAFE0V_TOUT",
	"TYPEC_RT_TIMER_SAFE5V_TOUT",
	"TYPEC_RT_TIMER_ROLE_SWAP_START",
	"TYPEC_RT_TIMER_ROLE_SWAP_STOP",
	"TYPEC_RT_TIMER_LEGACY",
	"TYPEC_RT_TIMER_NOT_LEGACY",

	"TYPEC_TRY_TIMER_DRP_TRY",
	"TYPEC_TRY_TIMER_DRP_TRYWAIT",

	"TYPEC_TIMER_CCDEBOUNCE",
	"TYPEC_TIMER_PDDEBOUNCE",
	"TYPEC_TIMER_DRP_SRC_TOGGLE",
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	"TYPEC_TIMER_NORP_SRC",
#endif
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */
};

#ifdef CONFIG_USB_PD_SAFE0V_DELAY
#define PD_TIMER_VSAFE0V_DLY_TOUT		TIMEOUT_VAL(50)
#else
#define PD_TIMER_VSAFE0V_DLY_TOUT		TIMEOUT_VAL(400)
#endif

#ifdef CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT
#define TYPEC_RT_TIMER_SAFE0V_DLY_TOUT		TIMEOUT_VAL(35)
#else
#define TYPEC_RT_TIMER_SAFE0V_DLY_TOUT		TIMEOUT_VAL(100)
#endif

/*
 * The unit of these immediate number is "ms".
 */
static const uint32_t tcpc_timer_timeout[PD_TIMER_NR] = {
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	TIMEOUT_RANGE(30, 60), /* PD_TIMER_BIST_CONT_MODE */
	TIMEOUT_RANGE(40, 50), /* PD_TIMER_DISCOVER_ID */
	TIMEOUT_RANGE(4, 5), /* PD_TIMER_HARD_RESET_COMPLETE */
	TIMEOUT_RANGE(4500, 5500), /* PD_TIMER_NO_RESPONSE */
	TIMEOUT_RANGE(25, 35), /* PD_TIMER_PS_HARD_RESET */
	TIMEOUT_RANGE(750, 920), /* PD_TIMER_PS_SOURCE_OFF */
	TIMEOUT_RANGE(390, 480), /* PD_TIMER_PS_SOURCE_ON, */
	TIMEOUT_RANGE(450, 550), /* PD_TIMER_PS_TRANSITION */
	TIMEOUT_RANGE(27, 30), /* PD_TIMER_SENDER_RESPONSE */
	TIMEOUT_RANGE(120, 150), /* PD_TIMER_SINK_ACTIVITY */
	TIMEOUT_RANGE(100, 100), /* PD_TIMER_SINK_REQUEST */
	TIMEOUT_RANGE(310, 620), /* PD_TIMER_SINK_WAIT_CAP */
	TIMEOUT_RANGE(40, 50), /* PD_TIMER_SOURCE_ACTIVITY */
	TIMEOUT_RANGE(100, 200), /* PD_TIMER_SOURCE_CAPABILITY */
	TIMEOUT_VAL(20), /* PD_TIMER_SOURCE_START */
	TIMEOUT_VAL(100), /* PD_TIMER_VCONN_ON */
	TIMEOUT_RANGE(40, 50), /* PD_TIMER_VDM_MODE_ENTRY */
	TIMEOUT_RANGE(40, 50), /* PD_TIMER_VDM_MODE_EXIT */
	TIMEOUT_RANGE(24, 30), /* PD_TIMER_VDM_RESPONSE */
	TIMEOUT_RANGE(25, 35), /* PD_TIMER_SOURCE_TRANSITION */
	TIMEOUT_RANGE(660, 660), /* PD_TIMER_SRC_RECOVER */

	/* PD_TIMER (out of spec) */
	PD_TIMER_VSAFE0V_DLY_TOUT, /* PD_TIMER_VSAFE0V_DELAY */
	TIMEOUT_VAL(650), /* PD_TIMER_VSAFE0V_TOUT */
	TIMEOUT_VAL(3), /* PD_TIMER_DISCARD */
	/* PD_TIMER_VBUS_STABLE */
	TIMEOUT_VAL(CONFIG_USB_PD_VBUS_STABLE_TOUT),
	/* PD_TIMER_VBUS_PRESENT */
	TIMEOUT_VAL(CONFIG_USB_PD_VBUS_PRESENT_TOUT),
	/* PD_TIMER_UVDM_RESPONSE */
	TIMEOUT_VAL(CONFIG_USB_PD_UVDM_TOUT),
	TIMEOUT_VAL(30), /* PD_TIMER_DFP_FLOW_DELAY */
	TIMEOUT_VAL(300), /* PD_TIMER_UFP_FLOW_DELAY */
	TIMEOUT_VAL_US(3000), /* PD_PE_VDM_POSTPONE */
	TIMEOUT_VAL(10), /* PD_TIMER_PE_IDLE_TOUT */
	TIMEOUT_VAL(200), /* PD_TIMER_SRC_CAP_EXT_TOUT */
	/* TYPEC-RT-TIMER */
	TIMEOUT_VAL(1), /* TYPEC_RT_TIMER_PE_IDLE */
	TYPEC_RT_TIMER_SAFE0V_DLY_TOUT, /* TYPEC_RT_TIMER_SAFE0V_DELAY */
	TIMEOUT_VAL(650), /* TYPEC_RT_TIMER_SAFE0V_TOUT */
	TIMEOUT_VAL(450), /* TYPEC_RT_TIMER_SAFE5V_TOUT */
	/* TYPEC_RT_TIMER_ROLE_SWAP */
	TIMEOUT_VAL(20),
	TIMEOUT_VAL(CONFIG_TYPEC_CAP_ROLE_SWAP_TOUT),
	TIMEOUT_VAL(50), /* TYPEC_RT_TIMER_LEGACY */
	TIMEOUT_VAL(5000), /* TYPEC_RT_TIMER_NOT_LEGACY */

	/* TYPEC-TRY-TIMER */
	TIMEOUT_RANGE(75, 150), /* TYPEC_TRY_TIMER_DRP_TRY */
	TIMEOUT_RANGE(400, 800), /* TYPEC_TRY_TIMER_DRP_TRYWAIT */

	/* TYPEC-DEBOUNCE-TIMER */
	TIMEOUT_RANGE(100, 200), /* TYPEC_TIMER_CCDEBOUNCE */
	TIMEOUT_RANGE(10, 10), /* TYPEC_TIMER_PDDEBOUNCE */
	TIMEOUT_RANGE(25, 25), /* TYPEC_TIMER_ERROR_RECOVERY */
	TIMEOUT_VAL(60), /* TYPEC_TIMER_DRP_SRC_TOGGLE */
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	TIMEOUT_VAL(300), /* TYPEC_TIMER_NORP_SRC */
#endif
#else /* CONFIG_USB_POWER_DELIVERY_SUPPORT */
	/* TYPEC-RT-TIMER */
	TYPEC_RT_TIMER_SAFE0V_DLY_TOUT, /* TYPEC_RT_TIMER_SAFE0V_DELAY */
	TIMEOUT_VAL(650), /* TYPEC_RT_TIMER_SAFE0V_TOUT */
	TIMEOUT_VAL(450), /* TYPEC_RT_TIMER_SAFE5V_TOUT */
	/* TYPEC_RT_TIMER_ROLE_SWAP */
	TIMEOUT_VAL(20),
	TIMEOUT_VAL(CONFIG_TYPEC_CAP_ROLE_SWAP_TOUT),
	TIMEOUT_VAL(50), /* TYPEC_RT_TIMER_LEGACY */
	TIMEOUT_VAL(5000), /* TYPEC_RT_TIMER_NOT_LEGACY */

	/* TYPEC-TRY-TIMER */
	TIMEOUT_RANGE(75, 150), /* TYPEC_TRY_TIMER_DRP_TRY */
	TIMEOUT_RANGE(400, 800), /* TYPEC_TRY_TIMER_DRP_TRYWAIT */
	TIMEOUT_RANGE(100, 200), /* TYPEC_TIMER_CCDEBOUNCE */
	TIMEOUT_RANGE(10, 10), /* TYPEC_TIMER_PDDEBOUNCE */
	TIMEOUT_VAL(60), /* TYPEC_TIMER_DRP_SRC_TOGGLE */
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	TIMEOUT_VAL(300), /* TYPEC_TIMER_NORP_SRC */
#endif
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */
};

typedef enum hrtimer_restart (*tcpc_hrtimer_call)(struct hrtimer *timer);

static void on_pe_timer_timeout(struct tcpc_device *tcpc_dev, uint32_t timer_id)
{
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	pd_event_t pd_event;

	D("timer_id %s(%u)\n", hisi_tcpc_timer_name[timer_id], timer_id);

	pd_event.event_type = PD_EVT_TIMER_MSG;
	pd_event.msg = timer_id;
	pd_event.msg_sec = 0;
	pd_event.pd_msg = NULL;

	switch (timer_id) {
	case PD_TIMER_VDM_MODE_ENTRY:
	case PD_TIMER_VDM_MODE_EXIT:
	case PD_TIMER_VDM_RESPONSE:
	case PD_TIMER_UVDM_RESPONSE:
		hisi_pd_put_vdm_event(tcpc_dev, &pd_event, false);
		break;

	case PD_TIMER_VSAFE0V_DELAY:
		hisi_pd_put_vbus_safe0v_event(tcpc_dev);
		break;

#ifdef CONFIG_USB_PD_SAFE0V_TIMEOUT
	case PD_TIMER_VSAFE0V_TOUT:
		{
			uint16_t power_status = 0;
			int rv;

			rv = tcpci_get_power_status(tcpc_dev, &power_status);
			if (rv < 0)
				break;
			hisi_tcpci_vbus_level_init(tcpc_dev, power_status);
			if (!tcpci_check_vbus_valid(tcpc_dev))
				hisi_pd_put_vbus_safe0v_event(tcpc_dev);
		}
		break;
#endif

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	case PD_TIMER_DISCARD:
		tcpc_dev->pd_discard_pending = false;
		pd_put_hw_event(tcpc_dev, PD_HW_TX_FAILED);
		break;
#endif

#if CONFIG_USB_PD_VBUS_STABLE_TOUT
	case PD_TIMER_VBUS_STABLE:
		hisi_pd_put_vbus_stable_event(tcpc_dev);
		break;
#endif

#if CONFIG_USB_PD_VBUS_PRESENT_TOUT
	case PD_TIMER_VBUS_PRESENT:
		hisi_pd_put_vbus_present_event(tcpc_dev);
		break;
#endif

	case PD_PE_VDM_POSTPONE:
		tcpc_dev->pd_postpone_vdm_timeout = true;
		atomic_inc(&tcpc_dev->pending_event);
		wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
		break;

	case PD_TIMER_PE_IDLE_TOUT:
		pd_put_pe_event(&tcpc_dev->pd_port, PD_PE_IDLE);
		break;

	default:
		hisi_pd_put_event(tcpc_dev, &pd_event, false);
		break;
	}
#endif

	hisi_tcpc_disable_timer(tcpc_dev, timer_id);
}

#define TCPC_TIMER_TRIGGER()	\
	do {								\
		tcpc_set_timer_tick(tcpc_dev, index);			\
		wake_up_interruptible(&tcpc_dev->timer_wait_que);	\
	} while (0)

static inline struct tcpc_device *tcpc_hrtimer_to_dev(struct hrtimer *timer,
		int index)
{
	return container_of(timer, struct tcpc_device, tcpc_timer[index]);
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static enum hrtimer_restart tcpc_timer_bist_cont_mode(struct hrtimer *timer)
{
	int index = PD_TIMER_BIST_CONT_MODE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_discover_id(struct hrtimer *timer)
{
	int index = PD_TIMER_DISCOVER_ID;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_hard_reset_complete(
		struct hrtimer *timer)
{
	int index = PD_TIMER_HARD_RESET_COMPLETE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_no_response(struct hrtimer *timer)
{
	int index = PD_TIMER_NO_RESPONSE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_ps_hard_reset(struct hrtimer *timer)
{
	int index = PD_TIMER_PS_HARD_RESET;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_ps_source_off(struct hrtimer *timer)
{
	int index = PD_TIMER_PS_SOURCE_OFF;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_ps_source_on(struct hrtimer *timer)
{
	int index = PD_TIMER_PS_SOURCE_ON;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_ps_transition(struct hrtimer *timer)
{
	int index = PD_TIMER_PS_TRANSITION;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_sender_response(struct hrtimer *timer)
{
	int index = PD_TIMER_SENDER_RESPONSE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_sink_activity(struct hrtimer *timer)
{
	int index = PD_TIMER_SINK_ACTIVITY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_sink_request(struct hrtimer *timer)
{
	int index = PD_TIMER_SINK_REQUEST;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_sink_wait_cap(struct hrtimer *timer)
{
	int index = PD_TIMER_SINK_WAIT_CAP;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_source_activity(struct hrtimer *timer)
{
	int index = PD_TIMER_SOURCE_ACTIVITY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_source_capability(struct hrtimer *timer)
{
	int index = PD_TIMER_SOURCE_CAPABILITY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_source_start(struct hrtimer *timer)
{
	int index = PD_TIMER_SOURCE_START;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vconn_on(struct hrtimer *timer)
{
	int index = PD_TIMER_VCONN_ON;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vdm_mode_entry(struct hrtimer *timer)
{
	int index = PD_TIMER_VDM_MODE_ENTRY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vdm_mode_exit(struct hrtimer *timer)
{
	int index = PD_TIMER_VDM_MODE_EXIT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vdm_response(struct hrtimer *timer)
{
	int index = PD_TIMER_VDM_RESPONSE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_source_transition(struct hrtimer *timer)
{
	int index = PD_TIMER_SOURCE_TRANSITION;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_src_recover(struct hrtimer *timer)
{
	int index = PD_TIMER_SRC_RECOVER;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vsafe0v_delay(struct hrtimer *timer)
{
	int index = PD_TIMER_VSAFE0V_DELAY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vsafe0v_tout(struct hrtimer *timer)
{
	int index = PD_TIMER_VSAFE0V_TOUT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_error_recovery(struct hrtimer *timer)
{
	int index = TYPEC_TIMER_ERROR_RECOVERY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_pd_discard(struct hrtimer *timer)
{
	int index = PD_TIMER_DISCARD;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vbus_stable(struct hrtimer *timer)
{
	int index = PD_TIMER_VBUS_STABLE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_vbus_present(struct hrtimer *timer)
{
	int index = PD_TIMER_VBUS_PRESENT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_uvdm_response(struct hrtimer *timer)
{
	int index = PD_TIMER_UVDM_RESPONSE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_dfp_flow_delay(struct hrtimer *timer)
{
	int index = PD_TIMER_DFP_FLOW_DELAY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_ufp_flow_delay(struct hrtimer *timer)
{
	int index = PD_TIMER_UFP_FLOW_DELAY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart pd_pe_vdm_postpone_timeout(struct hrtimer *timer)
{
	int index = PD_PE_VDM_POSTPONE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_pe_idle_tout(struct hrtimer *timer)
{
	int index = PD_TIMER_PE_IDLE_TOUT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_src_cap_ext_tout(struct hrtimer *timer)
{
	int index = PD_TIMER_SRC_CAP_EXT_TOUT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_rt_pe_idle(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_PE_IDLE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

static enum hrtimer_restart tcpc_timer_rt_vsafe0v_delay(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_SAFE0V_DELAY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_rt_vsafe0v_tout(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_SAFE0V_TOUT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_rt_vsafe5v_tout(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_SAFE5V_TOUT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}
static enum hrtimer_restart tcpc_timer_rt_role_swap_start(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_ROLE_SWAP_START;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_rt_role_swap_stop(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_ROLE_SWAP_STOP;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_rt_legacy(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_LEGACY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_rt_not_legacy(struct hrtimer *timer)
{
	int index = TYPEC_RT_TIMER_NOT_LEGACY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_try_drp_try(struct hrtimer *timer)
{
	int index = TYPEC_TRY_TIMER_DRP_TRY;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_try_drp_trywait(struct hrtimer *timer)
{
	int index = TYPEC_TRY_TIMER_DRP_TRYWAIT;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_ccdebounce(struct hrtimer *timer)
{
	int index = TYPEC_TIMER_CCDEBOUNCE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_pddebounce(struct hrtimer *timer)
{
	int index = TYPEC_TIMER_PDDEBOUNCE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart tcpc_timer_drp_src_toggle(struct hrtimer *timer)
{
	int index = TYPEC_TIMER_DRP_SRC_TOGGLE;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
static enum hrtimer_restart tcpc_timer_norp_src(struct hrtimer *timer)
{
	int index = TYPEC_TIMER_NORP_SRC;
	struct tcpc_device *tcpc_dev = tcpc_hrtimer_to_dev(timer, index);

	TCPC_TIMER_TRIGGER();
	return HRTIMER_NORESTART;
}
#endif

static tcpc_hrtimer_call tcpc_timer_call[PD_TIMER_NR] = {
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	[PD_TIMER_BIST_CONT_MODE] = tcpc_timer_bist_cont_mode,
	[PD_TIMER_DISCOVER_ID] = tcpc_timer_discover_id,
	[PD_TIMER_HARD_RESET_COMPLETE] = tcpc_timer_hard_reset_complete,
	[PD_TIMER_NO_RESPONSE] = tcpc_timer_no_response,
	[PD_TIMER_PS_HARD_RESET] = tcpc_timer_ps_hard_reset,
	[PD_TIMER_PS_SOURCE_OFF] = tcpc_timer_ps_source_off,
	[PD_TIMER_PS_SOURCE_ON] = tcpc_timer_ps_source_on,
	[PD_TIMER_PS_TRANSITION] = tcpc_timer_ps_transition,
	[PD_TIMER_SENDER_RESPONSE] = tcpc_timer_sender_response,
	[PD_TIMER_SINK_ACTIVITY] = tcpc_timer_sink_activity,
	[PD_TIMER_SINK_REQUEST] = tcpc_timer_sink_request,
	[PD_TIMER_SINK_WAIT_CAP] = tcpc_timer_sink_wait_cap,
	[PD_TIMER_SOURCE_ACTIVITY] = tcpc_timer_source_activity,
	[PD_TIMER_SOURCE_CAPABILITY] = tcpc_timer_source_capability,
	[PD_TIMER_SOURCE_START] = tcpc_timer_source_start,
	[PD_TIMER_VCONN_ON] = tcpc_timer_vconn_on,
	[PD_TIMER_VDM_MODE_ENTRY] = tcpc_timer_vdm_mode_entry,
	[PD_TIMER_VDM_MODE_EXIT] = tcpc_timer_vdm_mode_exit,
	[PD_TIMER_VDM_RESPONSE] = tcpc_timer_vdm_response,
	[PD_TIMER_SOURCE_TRANSITION] = tcpc_timer_source_transition,
	[PD_TIMER_SRC_RECOVER] = tcpc_timer_src_recover,
	[PD_TIMER_VSAFE0V_DELAY] = tcpc_timer_vsafe0v_delay,
	[PD_TIMER_VSAFE0V_TOUT] = tcpc_timer_vsafe0v_tout,
	[PD_TIMER_DISCARD] = tcpc_timer_pd_discard,
	[PD_TIMER_VBUS_STABLE] = tcpc_timer_vbus_stable,
	[PD_TIMER_VBUS_PRESENT] = tcpc_timer_vbus_present,
	[PD_TIMER_UVDM_RESPONSE] = tcpc_timer_uvdm_response,
	[PD_TIMER_DFP_FLOW_DELAY] = tcpc_timer_dfp_flow_delay,
	[PD_TIMER_UFP_FLOW_DELAY] = tcpc_timer_ufp_flow_delay,
	[PD_PE_VDM_POSTPONE] = pd_pe_vdm_postpone_timeout,
	[PD_TIMER_PE_IDLE_TOUT] = tcpc_timer_pe_idle_tout,
	[PD_TIMER_SRC_CAP_EXT_TOUT]  = tcpc_timer_src_cap_ext_tout,

	[TYPEC_RT_TIMER_PE_IDLE] = tcpc_timer_rt_pe_idle,
	[TYPEC_RT_TIMER_SAFE0V_DELAY] = tcpc_timer_rt_vsafe0v_delay,
	[TYPEC_RT_TIMER_SAFE0V_TOUT] = tcpc_timer_rt_vsafe0v_tout,
	[TYPEC_RT_TIMER_SAFE5V_TOUT] = tcpc_timer_rt_vsafe5v_tout,
	[TYPEC_RT_TIMER_ROLE_SWAP_START] = tcpc_timer_rt_role_swap_start,
	[TYPEC_RT_TIMER_ROLE_SWAP_STOP] = tcpc_timer_rt_role_swap_stop,
	[TYPEC_RT_TIMER_LEGACY] = tcpc_timer_rt_legacy,
	[TYPEC_RT_TIMER_NOT_LEGACY] = tcpc_timer_rt_not_legacy,

	[TYPEC_TRY_TIMER_DRP_TRY] = tcpc_timer_try_drp_try,
	[TYPEC_TRY_TIMER_DRP_TRYWAIT] = tcpc_timer_try_drp_trywait,

	[TYPEC_TIMER_CCDEBOUNCE] = tcpc_timer_ccdebounce,
	[TYPEC_TIMER_PDDEBOUNCE] = tcpc_timer_pddebounce,
	[TYPEC_TIMER_ERROR_RECOVERY] = tcpc_timer_error_recovery,
	[TYPEC_TIMER_DRP_SRC_TOGGLE] = tcpc_timer_drp_src_toggle,
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	[TYPEC_TIMER_NORP_SRC] = tcpc_timer_norp_src,
#endif
#else
	[TYPEC_RT_TIMER_SAFE0V_DELAY] = tcpc_timer_rt_vsafe0v_delay,
	[TYPEC_RT_TIMER_SAFE0V_TOUT] = tcpc_timer_rt_vsafe0v_tout,
	[TYPEC_RT_TIMER_SAFE5V_TOUT] = tcpc_timer_rt_vsafe5v_tout,
	[TYPEC_RT_TIMER_ROLE_SWAP_START] = tcpc_timer_rt_role_swap_start,
	[TYPEC_RT_TIMER_ROLE_SWAP_STOP] = tcpc_timer_rt_role_swap_stop,
	[TYPEC_RT_TIMER_LEGACY] = tcpc_timer_rt_legacy,
	[TYPEC_RT_TIMER_NOT_LEGACY] = tcpc_timer_rt_not_legacy,

	[TYPEC_TRY_TIMER_DRP_TRY] = tcpc_timer_try_drp_try,
	[TYPEC_TRY_TIMER_DRP_TRYWAIT] = tcpc_timer_try_drp_trywait,

	[TYPEC_TIMER_CCDEBOUNCE] = tcpc_timer_ccdebounce,
	[TYPEC_TIMER_PDDEBOUNCE] = tcpc_timer_pddebounce,
	[TYPEC_TIMER_DRP_SRC_TOGGLE] = tcpc_timer_drp_src_toggle,
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	[TYPEC_TIMER_NORP_SRC] = tcpc_timer_norp_src,
#endif
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */
};

static bool valid_timer_id(uint32_t timer_id)
{
	if (timer_id >= PD_TIMER_NR) {
		E("illegal timer_id %u\n", timer_id);
		return false;
	}

	return true;
}

static inline void tcpc_reset_timer_range(struct tcpc_device *tcpc,
		uint32_t start, uint32_t end)
{
	uint32_t i;
	uint64_t mask;

	D("timer_id %u ~ %u\n", start, end);
	mask = tcpc_get_timer_enable_mask(tcpc);

	for (i = start; i <= end; i++) {
		if (mask & RT_MASK64(i)) {
			hrtimer_try_to_cancel(&tcpc->tcpc_timer[i]);
			tcpc_clear_timer_enable_mask(tcpc, i);
		}
	}
}

void hisi_tcpc_restart_timer(struct tcpc_device *tcpc, uint32_t timer_id)
{
	uint64_t mask;

	if (!valid_timer_id(timer_id))
		return;

	D("Restart %s\n", hisi_tcpc_timer_name[timer_id]);
	mask = tcpc_get_timer_enable_mask(tcpc);
	if (mask & RT_MASK64(timer_id))
		hisi_tcpc_disable_timer(tcpc, timer_id);
	hisi_tcpc_enable_timer(tcpc, timer_id);
}

void hisi_tcpc_enable_timer(struct tcpc_device *tcpc, uint32_t timer_id)
{
	uint32_t r, mod, tout;

	if (!valid_timer_id(timer_id))
		return;

	mutex_lock(&tcpc->timer_lock);
	if ((timer_id >= TYPEC_TIMER_START_ID) &&
			(timer_id <= TYPEC_TIMER_DRP_SRC_TOGGLE))
		tcpc_reset_timer_range(tcpc, TYPEC_TIMER_START_ID,
				TYPEC_TIMER_DRP_SRC_TOGGLE);

	D("Enable %s\n", hisi_tcpc_timer_name[timer_id]);
	tcpc_set_timer_enable_mask(tcpc, timer_id);

	tout = tcpc_timer_timeout[timer_id];

#ifdef CONFIG_USB_PD_RANDOM_FLOW_DELAY
	if (timer_id == PD_TIMER_DFP_FLOW_DELAY ||
			timer_id == PD_TIMER_UFP_FLOW_DELAY)
		tout += TIMEOUT_VAL(jiffies & 0x07);
#endif

	r =  tout / 1000000; /* 1000000: tout to seconds */
	mod = tout % 1000000; /* 1000000: tout to nseconds */

	mutex_unlock(&tcpc->timer_lock);
	hrtimer_start(&tcpc->tcpc_timer[timer_id],
			ktime_set(r, mod * 1000), HRTIMER_MODE_REL); /* mod * 1000: ns to ms */
}

void hisi_tcpc_disable_timer(struct tcpc_device *tcpc_dev, uint32_t timer_id)
{
	uint64_t mask;

	if (!valid_timer_id(timer_id))
		return;

	D("Disable %s\n", hisi_tcpc_timer_name[timer_id]);
	mask = tcpc_get_timer_enable_mask(tcpc_dev);
	if (mask & RT_MASK64(timer_id)) {
		hrtimer_try_to_cancel(&tcpc_dev->tcpc_timer[timer_id]);
		tcpc_clear_timer_enable_mask(tcpc_dev, timer_id);
	}
}

void hisi_tcpc_timer_reset(struct tcpc_device *tcpc_dev)
{
	uint64_t mask;
	unsigned int i;

	D("\n");
	mask = tcpc_get_timer_enable_mask(tcpc_dev);
	for (i = 0; i < PD_TIMER_NR; i++)
		if (mask & RT_MASK64(i))
			hrtimer_try_to_cancel(&tcpc_dev->tcpc_timer[i]);
	tcpc_reset_timer_enable_mask(tcpc_dev);
}

bool hisi_tcpc_timer_enabled(struct tcpc_device *tcpc, uint32_t timer_id)
{
	unsigned long flags;
	bool enable = false;

	down(&tcpc->timer_enable_mask_lock);
	raw_local_irq_save(flags);
	enable = (tcpc->timer_enable_mask & RT_MASK64(timer_id));
	raw_local_irq_restore(flags);
	up(&tcpc->timer_enable_mask_lock);

	return enable;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
void hisi_tcpc_reset_pe_timer(struct tcpc_device *tcpc_dev)
{
	mutex_lock(&tcpc_dev->timer_lock);
	tcpc_reset_timer_range(tcpc_dev, 0, PD_PE_TIMER_END_ID);
	mutex_unlock(&tcpc_dev->timer_lock);
}
#endif

void hisi_tcpc_reset_typec_debounce_timer(struct tcpc_device *tcpc)
{
	D("\n");
	mutex_lock(&tcpc->timer_lock);
	tcpc_reset_timer_range(tcpc, TYPEC_TIMER_START_ID,
			TYPEC_TIMER_DRP_SRC_TOGGLE);
	mutex_unlock(&tcpc->timer_lock);
}

void hisi_tcpc_reset_typec_try_timer(struct tcpc_device *tcpc)
{
	D("\n");
	mutex_lock(&tcpc->timer_lock);
	tcpc_reset_timer_range(tcpc,
			TYPEC_TRY_TIMER_START_ID, TYPEC_TIMER_START_ID);
	mutex_unlock(&tcpc->timer_lock);
}

static void tcpc_handle_timer_triggered(struct tcpc_device *tcpc_dev)
{
	uint64_t triggered_timer;
	uint32_t i = 0;

	triggered_timer = tcpc_get_timer_tick(tcpc_dev);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	for (i = 0; i < PD_PE_TIMER_END_ID; i++) {
		if (triggered_timer & RT_MASK64(i)) {
			on_pe_timer_timeout(tcpc_dev, i);
			tcpc_clear_timer_tick(tcpc_dev, i);
		}
	}
#endif

	mutex_lock(&tcpc_dev->typec_lock);
	for (; i < PD_TIMER_NR; i++) {
		if (triggered_timer & RT_MASK64(i)) {
			hisi_tcpc_typec_handle_timeout(tcpc_dev, i);
			tcpc_clear_timer_tick(tcpc_dev, i);
		}
	}
	mutex_unlock(&tcpc_dev->typec_lock);
}

static int hisi_tcpc_timer_thread(void *param)
{
	struct tcpc_device *tcpc_dev = param;
	volatile uint64_t *timer_tick = NULL;
	struct sched_param sch_param = {.sched_priority = MAX_RT_PRIO - 1};

	timer_tick = &tcpc_dev->timer_tick;

	sched_setscheduler(current, SCHED_FIFO, &sch_param);
	while (true) {
		wait_event_interruptible(tcpc_dev->timer_wait_que,
				((*timer_tick) ? true : false) ||
					tcpc_dev->timer_thead_stop);

		if (kthread_should_stop() || tcpc_dev->timer_thead_stop)
			break;

		do {
			tcpc_handle_timer_triggered(tcpc_dev);
		} while (*timer_tick);
	}

	return 0;
}

int hisi_tcpci_timer_init(struct tcpc_device *tcpc_dev)
{
	int i;

	D("+\n");

	tcpc_dev->timer_task = kthread_create(hisi_tcpc_timer_thread, tcpc_dev,
			"hisi_tcpc_timer_%s.%pK",
			dev_name(&tcpc_dev->dev),
			tcpc_dev);

	init_waitqueue_head(&tcpc_dev->timer_wait_que);

	tcpc_dev->timer_tick = 0;
	tcpc_dev->timer_enable_mask = 0;
	wake_up_process(tcpc_dev->timer_task);
	for (i = 0; i < PD_TIMER_NR; i++) {
		hrtimer_init(&tcpc_dev->tcpc_timer[i],
				CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		tcpc_dev->tcpc_timer[i].function = tcpc_timer_call[i];
	}

	D("-\n");
	return 0;
}

int hisi_tcpci_timer_deinit(struct tcpc_device *tcpc_dev)
{
	uint64_t mask;
	unsigned int i;

	mask = tcpc_get_timer_enable_mask(tcpc_dev);

	mutex_lock(&tcpc_dev->timer_lock);
	wake_up_interruptible(&tcpc_dev->timer_wait_que);
	kthread_stop(tcpc_dev->timer_task);
	for (i = 0; i < PD_TIMER_NR; i++) {
		if (mask & RT_MASK64(i))
			hrtimer_try_to_cancel(&tcpc_dev->tcpc_timer[i]);
	}

	mutex_unlock(&tcpc_dev->timer_lock);
	return 0;
}
