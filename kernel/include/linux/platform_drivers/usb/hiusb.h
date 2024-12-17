/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DRIVERS_HIUSB_H_
#define _DRIVERS_HIUSB_H_

#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/notifier.h>
#include <linux/platform_drivers/usb/hiusb_common.h>

/* hiusb cable */
#define HIUSB_CABLE_FEILD_NONE    0u
#define HIUSB_CABLE_FEILD_VBUS    1u
#define HIUSB_CABLE_FEILD_USB     (1u << 1)
#define HIUSB_CABLE_FEILD_DP      (1u << 2)

#define HIUSB_CABLE_STATE_OUT     0u
#define HIUSB_CABLE_STATE_IN      1u

/* hiusb prop */
#define HIUSB_PROP_FEILD_NONE         0u
#define HIUSB_PROP_FEILD_VBUS_INPUT   0u
#define HIUSB_PROP_FEILD_VBUS_OUTPUT  1u
#define HIUSB_PROP_FEILD_USB_DEVICE   (0u << 1)
#define HIUSB_PROP_FEILD_USB_HOST     (1u << 1)
#define HIUSB_PROP_FEILD_ORIEN_NORMAL (0u << 2)
#define HIUSB_PROP_FEILD_ORIEN_FLIP   (1u << 2)
#define HIUSB_PROP_FEILD_DP_0LANE     (0u << 3)
#define HIUSB_PROP_FEILD_DP_2LANE     (1u << 3)
#define HIUSB_PROP_FEILD_DP_4LANE     (1u << 4)
#define HIUSB_PROP_FEILD_DP_HPD_IN    (0u << 5)
#define HIUSB_PROP_FEILD_DP_HPD_OUT   (1u << 5)
/* property of generic USB port */
#define HIUSB_PROP_VBUS_INPUT         0u
#define HIUSB_PROP_VBUS_OUTPUT        1u
#define HIUSB_PROP_USB_OFF            0u
#define HIUSB_PROP_USB_DEVICE         1u
#define HIUSB_PROP_USB_HOST           2u
/* property of type-c port */
#define HIUSB_PROP_TYPEC_ORIEN_NORMAL 0u
#define HIUSB_PROP_TYPEC_ORIEN_FLIP   1u
#define HIUSB_PROP_DP_0LANE           0u
#define HIUSB_PROP_DP_2LANE           1u
#define HIUSB_PROP_DP_4LANE           2u
#define HIUSB_PROP_DP_HPD_OUT         0u
#define HIUSB_PROP_DP_HPD_IN          1u
#define HIUSB_PROP_DP_SHORT           2u

/* property of hifi port */
#define HIUSB_PROP_HIFI_OFF               0u
#define HIUSB_PROP_HIFI_START             1u
#define HIUSB_PROP_HIFI_START_RESET_VBUS  2u
#define HIUSB_PROP_HIFI_STOP              3u
#define HIUSB_PROP_HIFI_STOP_RESET_VBUS   4u
#define HIUSB_PROP_HIFI_HIBERNATE         5u
#define HIUSB_PROP_HIFI_WAKEUP            6u

#define HIUSB_PROP_HOST_PLUG_OUT         0u
#define HIUSB_PROP_WAKEUP_CALLBACK       1u

#define HIUSB_DEVICE_CONNECT   0u

#define hiusb_state_initializer(c, p) {						\
		.cable.feild = (u32)(c),					\
		.prop.feild = (u32)(p),						\
	}

#define declare_hiusb_state(n, c, p)						\
	struct hiusb_state n = hiusb_state_initializer(c, p)

struct list_head;
struct fwnode_handle;
struct device;
struct hiusb_port;

struct hiusb_prop_state {
	unsigned power_role : 1;
	unsigned orien : 1;
	unsigned usb_role : 2;
	unsigned lane_num : 2;
	unsigned hpd : 2;
	unsigned hifi_state : 3;
};

union hiusb_prop {
	u32 feild;
	struct hiusb_prop_state state;
};

struct hiusb_cable_state {
	unsigned vbus : 1;
	unsigned usb : 1;
	unsigned dp : 1;
};

union hiusb_cable {
	u32 feild;
	struct hiusb_cable_state state;
};

struct hiusb_port_state {
	union hiusb_cable cable;
	union hiusb_prop prop;
};

struct hiusb_port_handle {
	struct list_head list;
	struct fwnode_handle *port_fwnode;
	void *data;
};

enum hiusb_power_state {
	HIUSB_POWER_OFF = 0,
	HIUSB_POWER_ON = 1,
	HIUSB_POWER_HOLD = 2,
};

void hiusb_register_notify(struct notifier_block *nb);
void hiusb_unregister_notify(struct notifier_block *nb);
void hiusb_notify_device_connect(enum usb_device_speed *speed);

/* API of hiusb port handle */
#ifdef CONFIG_USB_HIUSB
struct hiusb_port_handle *hiusb_port_handle_create_by_dev(struct device *dev);
void hiusb_port_handle_destroy(struct hiusb_port_handle *handle);
int hiusb_set_state(struct hiusb_port *port, struct hiusb_port_state state);
int hiusb_set_prop(const struct hiusb_port_handle *handle, struct hiusb_port_state state);
int hiusb_switch_client_by_type(const struct hiusb_port_handle *handle, u8 type);
int hiusb_start_hifi_usb(int portid, bool reset_power);
void hiusb_stop_hifi_usb(int portid, bool reset_power);
int hiusb_find_portid_by_udev(struct usb_device *udev);
int hiusb_find_portid_by_bus(struct usb_bus *bus);
struct hiusb_port *udev_to_port(struct usb_device *udev);
int hiusb_start_hifi_wakeup(int portid);
int hiusb_start_hifi_hibernate(int portid);
int hiusb_wakeup_hifi_usb(int portid);
void hiusb_cancel_bc_again(int sync);
int hiusb_plug_event_notifier_register(struct notifier_block *nb);
int hiusb_plug_event_notifier_unregister(struct notifier_block *nb);
enum usb_state hiusb_get_state(const struct hiusb_port_handle *handle);
int hiusb_runtime_enable(struct device *dev);

int hiusb_host_start(struct hiusb_port *port);
int hiusb_host_stop(struct hiusb_port *port);
int hiusb_device_start(struct hiusb_port *port);
int hiusb_device_stop(struct hiusb_port *port);
int hiusb_dp_in(struct hiusb_port *port);
int hiusb_dp_out(struct hiusb_port *port);
int hiusb_atomic_notifier_call_chain(unsigned long val, void *v);
int hiusb_usb20_phy_calibrate(struct usb_device *udev);
#else
static inline struct hiusb_port_handle *hiusb_port_handle_create_by_dev(struct device *dev)
{
	return NULL;
}
static inline void hiusb_port_handle_destroy(struct hiusb_port_handle *handle)
{
	return;
}
static inline int hiusb_set_state(struct hiusb_port *port, struct hiusb_port_state state)
{
	return 0;
}
static inline int hiusb_set_prop(const struct hiusb_port_handle * handle, struct hiusb_port_state state)
{
	return 0;
}
static inline int hiusb_switch_client_by_type(const struct hiusb_port_handle *handle, u8 type)
{
	return 0;
}

static inline int hiusb_start_hifi_usb(int portid, bool reset_power)
{
	return 0;
}
static inline void hiusb_stop_hifi_usb(int portid, bool reset_power)
{
	return;
}
static inline int hiusb_find_portid_by_udev(struct usb_device *udev)
{
	return 0;
}

static inline int hiusb_find_portid_by_bus(struct usb_bus *bus)
{
	return 0;
}

static inline struct hiusb_port *udev_to_port(struct usb_device *udev)
{
	return NULL;
}
static inline int hiusb_start_hifi_wakeup(int portid)
{
	return 0;
}
static inline int hiusb_start_hifi_hibernate(int portid)
{
	return 0;
}
static inline int hiusb_wakeup_hifi_usb(int portid)
{
	return 0;
}

static inline void hiusb_cancel_bc_again(int sync)
{
	return;
}
static inline int hiusb_plug_event_notifier_register(struct notifier_block *nb)
{
	return 0;
}
static inline int hiusb_plug_event_notifier_unregister(struct notifier_block *nb)
{
	return 0;
}
static inline enum usb_state hiusb_get_state(const struct hiusb_port_handle *handle)
{
	return 0;
}
static inline int hiusb_runtime_enable(struct device *dev)
{
	return 0;
}
static inline int hiusb_host_start(struct hiusb_port *port)
{
	return 0;
}
static inline int hiusb_host_stop(struct hiusb_port *port)
{
	return 0;
}
static inline int hiusb_device_start(struct hiusb_port *port)
{
	return 0;
}
static inline int hiusb_device_stop(struct hiusb_port *port)
{
	return 0;
}
static inline int hiusb_dp_in(struct hiusb_port *port)
{
	return 0;
}
static inline int hiusb_dp_out(struct hiusb_port *port)
{
	return 0;
}
static inline int hiusb_atomic_notifier_call_chain(unsigned long val, void *v)
{
	return 0;
}
static inline int hiusb_usb20_phy_calibrate(struct usb_device *udev)
{
	return 0;
}
#endif

#endif /* _DRIVERS_HIUSB_H_ */
