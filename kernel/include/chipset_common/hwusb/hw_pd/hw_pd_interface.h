/*
 * hw_pd_interface.h
 *
 * Header file of common interface for huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HW_PD_INTERFACE_H_
#define _HW_PD_INTERFACE_H_

#define HW_PD_VERSION "1.1.25"

#include <linux/version.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/power_supply.h>

#include <chipset_common/hwusb/hw_pd/hw_pd_state.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_ops.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/platform_drivers/chip_usb.h>
#include <linux/platform_drivers/tca.h>
#else
#include <linux/hisi/usb/chip_usb.h>
#include <linux/hisi/usb/tca.h>
#endif

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd/hw_pd_usb.h>
#include <huawei_platform/usb/hw_pd/hw_pd_power.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include <huawei_platform/usb/hw_pd/hw_pd_vbus.h>

/* indicate whether or not enable debug acc(double rp or double rd) */
#define CONFIG_DPM_USB_PD_CUSTOM_DBGACC

/* indicate whether or not enable debug acc sink(double rp) */
#define CONFIG_DPM_TYPEC_CAP_DBGACC_SNK

/* indicate whether or not enable custom src(double 56k) */
#define CONFIG_DPM_TYPEC_CAP_CUSTOM_SRC

/*
 * huawei's second-generation dock inserts digital headphones will repeatedly reset,
 * so we use svid to avoid this bug.
 */
#define PD_DPM_HW_DOCK_SVID                     0x12d1

/* used for cc abnormal change handler(cc interrupt storm), also used as water check */
#define PD_DPM_CC_CHANGE_COUNTER_THRESHOLD      50
#define PD_DPM_CC_CHANGE_INTERVAL               2000 /* ms */
#define PD_DPM_CC_CHANGE_MSEC                   1000
#define PD_DPM_CC_CHANGE_BUF_SIZE               10
#define PD_DPM_CC_DMD_COUNTER_THRESHOLD         1
#define PD_DPM_CC_DMD_INTERVAL                  (24 * 60 * 60) /* s */
#define PD_DPM_CC_DMD_BUF_SIZE                  1024
#define PD_DPM_INVALID_VAL                      (-1)

/* discover id ack:product vdo type offset, this is used for huawei wireless case(1/2) */
#define PD_DPM_PDT_OFFSET                       12
#define PD_DPM_PDT_MASK                         0x7
#define PD_DPM_PDT_VID_OFFSET                   16
#define PD_PID_COVER_ONE                        0x3000 /* use bcd value indicate huawei case1 */
#define PD_PID_COVER_TWO                        0x3001 /* use bcd value indicate huawei case2 */
#define PD_DPM_HW_VID                           0x12d1
#define PD_DPM_HW_CHARGER_PID                   0x3b30 /* use to indicate huawei charger pid */
#define PD_DPM_HW_PDO_MASK                      0xffff

/* cc status for big data collect */
#define PD_DPM_CC_OPEN                          0x00
#define PD_DPM_CC_DFT                           0x01
#define PD_DPM_CC_1_5                           0x02
#define PD_DPM_CC_3_0                           0x03
#define PD_DPM_CC2_STATUS_OFFSET                0x02
#define PD_DPM_CC_STATUS_MASK                   0x03
#define PD_DPM_BOTH_CC_STATUS_MASK              0x0f

/* used for cc orientation indicate cc none status */
#define CC_ORIENTATION_FACTORY_SET              1
#define CC_ORIENTATION_FACTORY_UNSET            0

/* used for water detection */
#define WATER_DET_TRUE                          1
#define WATER_DET_FALSE                         0

/* defalt invalid value */
#define HW_PD_INVALID_STATE                     (-1)
#define HW_PD_IGNORE_STATE                      (-2)

/* typec cc mode(UFP, DRP) */
enum pd_cc_mode {
	PD_DPM_CC_MODE_UFP,
	PD_DPM_CC_MODE_DRP,
};

enum pd_connect_result {
	PD_CONNECT_NONE,
	PD_CONNECT_TYPEC_ONLY,
	PD_CONNECT_TYPEC_ONLY_SNK_DFT,
	PD_CONNECT_TYPEC_ONLY_SNK,
	PD_CONNECT_TYPEC_ONLY_SRC,
	PD_CONNECT_PE_READY,
	PD_CONNECT_PE_READY_SNK,
	PD_CONNECT_PE_READY_SRC,
	PD_CONNECT_PE_READY_SNK_PD30,
	PD_CONNECT_PE_READY_SRC_PD30,
	PD_CONNECT_PE_READY_SNK_APDO,
	PD_CONNECT_HARD_RESET,
	PD_CONNECT_SOFT_RESET,

#ifdef CONFIG_DPM_USB_PD_CUSTOM_DBGACC
	PD_CONNECT_PE_READY_DBGACC_UFP,
	PD_CONNECT_PE_READY_DBGACC_DFP,
#endif /* CONFIG_USB_PD_CUSTOM_DBGACC */
};

/* used to policy engine report event */
enum pd_dpm_pe_event {
	PD_DPM_PE_EVT_DIS_VBUS_CTRL, /* disable vbus */
	PD_DPM_PE_EVT_SOURCE_VCONN, /* source vconn */
	PD_DPM_PE_EVT_SOURCE_VBUS, /* source vbus */
	PD_DPM_PE_EVT_SINK_VBUS, /* sink vbus */
	PD_DPM_PE_EVT_PR_SWAP, /* power role swap */
	PD_DPM_PE_EVT_DR_SWAP, /* data role swap */
	PD_DPM_PE_EVT_VCONN_SWAP, /* vconn swap */
	PD_DPM_PE_EVT_TYPEC_STATE, /* typec status */
	PD_DPM_PE_EVT_PD_STATE, /* pd status */
	PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER, /* used to count cc change */
	PD_DPM_PE_CABLE_VDO, /* emark cable vdo info */
	PD_DPM_PE_CABLE_DATA, /* cable data from pd */
};

/* typec status */
enum pd_typec_attach_type {
	PD_DPM_TYPEC_UNATTACHED,
	PD_DPM_TYPEC_ATTACHED_SNK, /* single rp */
	PD_DPM_TYPEC_ATTACHED_SRC, /* single rd or ra + rd */
	PD_DPM_TYPEC_ATTACHED_AUDIO, /* double ra */
	PD_DPM_TYPEC_ATTACHED_DEBUG, /* double rd */

#ifdef CONFIG_DPM_TYPEC_CAP_DBGACC_SNK
	PD_DPM_TYPEC_ATTACHED_DBGACC_SNK, /* double rp */
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK */

#ifdef CONFIG_DPM_TYPEC_CAP_CUSTOM_SRC
	PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC, /* double 56k rp */
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC */

	PD_DPM_TYPEC_ATTACHED_VBUS_ONLY, /* cc open and vbus in */
	PD_DPM_TYPEC_UNATTACHED_VBUS_ONLY, /* cc open and vbus out */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC2, /* double 22k or 10k rp */
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	PD_DPM_TYPEC_ATTACHED_NORP_SRC,
};

/* typec status */
enum pd_dpm_typec_status {
	PD_DPM_USB_TYPEC_NONE, /* typec status unknow */
	PD_DPM_USB_TYPEC_DETACHED, /* typec detached */
	PD_DPM_USB_TYPEC_DEVICE_ATTACHED, /* this device act as device */
	PD_DPM_USB_TYPEC_HOST_ATTACHED, /* this device act as host */
	PD_DPM_USB_TYPEC_AUDIO_ATTACHED, /* this device detect analog andio plug in */
	PD_DPM_USB_TYPEC_AUDIO_DETACHED, /* this device detect analog andio plug out */
};

/* used to lock or unlock typec wakelock */
enum pd_dpm_wake_lock_type {
	PD_WAKE_LOCK = 100,
	PD_WAKE_UNLOCK,
};

/* used to decribe typec status reported from policy engine */
struct pd_dpm_typec_state {
	bool polarity; /* cc direction */
	int cc1_status; /* cc1 voltage status */
	int cc2_status; /* cc1 voltage status */
	int old_state; /* typec old status: described by pd_dpm_typec_status */
	int new_state; /* typec new status: described by pd_dpm_typec_status */
};

struct pd_dpm_pd_state {
	uint8_t connected;
};

struct pd_dpm_swap_state {
	uint8_t new_role;
};

enum pd_dpm_vbus_type {
	PD_DPM_VBUS_TYPE_TYPEC = 20,
	PD_DPM_VBUS_TYPE_PD,
};

/* used to describe abnormal interrupt from pd(cc int or vbus int) */
enum abnomal_change_type {
	PD_DPM_ABNORMAL_CC_CHANGE,
	PD_DPM_UNATTACHED_VBUS_ONLY,
};

/* emark cable current capability */
enum cur_cap {
	PD_DPM_CURR_1P5A, /* 1.5A */
	PD_DPM_CURR_3A, /* 3A */
	PD_DPM_CURR_5A, /* 5A */
	PD_DPM_CURR_6A, /* 6A */
	PD_DPM_CURR_7A, /* 7A */
};

#define CABLE_CUR_CAP_SHIFT        5 /* cable current shift */
#define CABLE_EXT_CUR_CAP_SHIFT    13 /* cable current shift */
/* cable current MAST(bit5 and bit6 is defined in pd spec) */
#define CABLE_CUR_CAP_MASK         (BIT(5) | BIT(6))
/* addtitional curret MASK(bit13 bit14 and bit15 are defined in product vdo) */
#define CABLE_EXT_CUR_CAP_MASK     (BIT(13) | BIT(14) | BIT(15))

/* used for emark delay time */
#define EMARK_WITH_CABLE_AUTH                   600
#define EAMRK_WAIT_TIME                         200
#define EMARK_WAIT_TIME_UFCS                    300
#define CABLE_AUTH_RANDOM_NUM                   7

/* Result of cable authority */
enum cable_ret {
	PD_DPM_CABLE_INIT,
	PD_DPM_CABLE_SUCC,            /* key succ */
	PD_DPM_CABLE_FAIL,            /* 7A KEY FAIL */
	PD_DPM_RESULT_6A_TIMEOUT,     /* 6A TIMEOUT */
	PD_DPM_RESULT_THIRDPARTY,     /* not huawei cable */
};

/* base on huawei uvdm protocol */
enum pd_product_type {
	PD_PDT_PD_ADAPTOR, /* adaptor */
	PD_PDT_PD_POWER_BANK, /* power bank */
	PD_PDT_WIRELESS_CHARGER, /* wireless charger */
	PD_PDT_WIRELESS_COVER, /* wireless case generation one */
	PD_PDT_WIRELESS_COVER_TWO, /* wireless case generation two */
	PD_PDT_TOTAL,
};

/* Random data for cable authority */
struct pd_dpm_cable_data {
	unsigned int cnt;
	unsigned int msg[CABLE_AUTH_RANDOM_NUM];
};

/*
 * Global context of Huawei PD driver.
 * Platform should allocate an instance of this structure and
 * register to chip_common by calling 'hw_pd_register_instance'.
 */
struct pd_dpm_info {
	struct device *dev;

	/* lock for process pending event */
	struct mutex sink_vbus_lock;
	struct mutex usb_lock;
	int pending_usb_event;

	/* Notifier for PD event, such as pd_dpm_vbus_type or chip_charger_type */
	struct blocking_notifier_head pd_evt_nh;
	/* Notifier for TYPEC event as pd_dpm_typec_status */
	struct blocking_notifier_head pd_port_status_nh;
	/* Notifier for PD lock event as pd_dpm_wake_lock_type */
	struct atomic_notifier_head pd_wake_unlock_evt_nh;

	struct workqueue_struct *usb_wq;
	struct delayed_work usb_state_update_work;
	struct delayed_work cc_moisture_flag_restore_work;
	struct delayed_work reinit_pd_work;
	struct delayed_work otg_restore_work;
	struct delayed_work cc_short_work;
	struct work_struct fb_work;

	/* ocp context */
	int max_ocp_count;
	int ocp_delay_time;
	int pd_reinit_enable;
	const char *ldo_name;
	bool is_ocp;
	struct regulator *pd_vdd_ldo;
	struct wakeup_source *vdd_ocp_lock;
};

/* Register notifier for pd_evt_nh */
int hw_pd_reg_event_notifier(struct notifier_block *nb);

/* Unregister notifier for pd_evt_nh */
int hw_pd_unreg_event_notifier(struct notifier_block *nb);

/* Register notifier for pd_wake_unlock_evt_nh */
int hw_pd_reg_wakeunlock_notifier(struct notifier_block *nb);

/* Unregister notifier for pd_wake_unlock_evt_nh */
int hw_pd_unreg_wakeunlock_notifier(struct notifier_block *nb);

/* Register notifier for pd_port_status_nh */
int hw_pd_reg_port_notifier(struct notifier_block *nb);

/* Unregister notifier for pd_port_status_nh */
int hw_pd_unreg_port_notifier(struct notifier_block *nb);

/* Handle PD/TYPEC event(pd_dpm_pe_event) sending from PD phisical driver */
int hw_pd_handle_pe_event(unsigned long event, void *data);

/* Get the current state of typec port, output state as pd_dpm_typec_status */
void hw_pd_get_typec_state(int *typec_detach);

/* Charger driver call this function to notify the max power of curret charge-adapter */
void hw_pd_set_max_power(int max_power);

/* Disable/Enable PD communication */
int hw_pd_disable_pd(bool disable);

/* Notify the cable capability */
void hw_pd_set_cable_vdo(void *data);

/* Notify the product information of typec port of remote device */
void hw_pd_set_product_id(unsigned int vid, unsigned int pid, unsigned int type);

/* Check whether is otg mode */
int hw_pd_get_otg_channel(void);

/* Call hw_pd_emark_detect to do emark-detect and call this function to get result */
void hw_pd_emark_detect_finish(void);

/* Operation for self-protect in accident of cc_vbus_short  */
void hw_pd_cc_dynamic_protect(void);

/* Get the status of 2 pins of typec port */
int hw_pd_get_cc_state(unsigned int *cc1, unsigned int *cc2);

/* Send the power-supply state to charger driver */
void hw_pd_set_source_sink(enum power_supply_sinksource_type type);

/* Notify typec port to switch to drp mode */
void hw_pd_enable_drp(void);

/* Charger dirver call this to send direct-charge state to PD physical dirver */
int hw_pd_notify_dc_status(bool dc);

#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
/* Broadcast wakelock event(pd_dpm_wake_lock_type) of huawei PD driver */
void hw_pd_wakelock_ctrl(unsigned long event);

/*
 * PD phy driver call this to notify enable/diasble vbus power for OTG
 * event: CHARGER_TYPE_NONE, PLEASE_PROVIDE_POWER
 */
void hw_pd_vbus_ctrl(unsigned long event);

/* Check whether cc_vbus_short has occured */
bool hw_pd_check_ccvbus_short(void);

/* Broadcast vbus state(chip_charger_type ) of huawei PD driver */
int hw_pd_vbus_event_notify(unsigned long event, void *data);

/* Check whether remote device is wireless-cover product */
bool hw_pd_judge_is_cover(void);

/* Get capability for charging of USB cable */
enum cur_cap hw_pd_get_vdo_cap(void);

/* Check whether support cable authority */
int hw_pd_support_cable_auth(void);
#else
static inline void hw_pd_wakelock_ctrl(unsigned long event) {};
static inline void hw_pd_vbus_ctrl(unsigned long event) {};
static inline bool hw_pd_judge_is_cover(void) { return false; }
static inline enum cur_cap hw_pd_get_vdo_cap(void) { return 0; }
static inline bool hw_pd_check_ccvbus_short(void) { return false; }
static inline int hw_pd_vbus_event_notify(unsigned long event, void *data) { return 0; }
static inline int hw_pd_support_cable_auth(void) {return 0;}
#endif /* CONFIG_TCPC_CLASS */

#endif /* _HW_PD_DEV_H_ */

