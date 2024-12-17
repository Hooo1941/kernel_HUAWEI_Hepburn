/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#ifndef _DRIVERS_HIUSB_CLIENT_H_
#define _DRIVERS_HIUSB_CLIENT_H_

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Define for hiusb client */
#define HIUSB_CLIENT_TYPE_DEVICE        1u
#define HIUSB_CLIENT_TYPE_HOST          (1u << 1)
#define HIUSB_CLIENT_TYPE_HIFI          (1u << 2)
#define HIUSB_CLIENT_TYPE_MASK          0x7u

enum hiusb_port_client_operation {
	HIUSB_CLIENT_IOCTRL_RESET_HIFI,
	HIUSB_CLIENT_IOCTRL_STOP_HIFI,
	HIUSB_CLIENT_IOCTRL_STOP_PERIPHERIAL,
	HIUSB_CLIENT_IOCTRL_START_PERIPHERIAL,
};

static inline bool hiusb_client_valid_type(u8 type)
{
	return (type & HIUSB_CLIENT_TYPE_MASK) != 0;
}

struct hiusb_client_desc {
	u8 type;
	const char *name;
	struct module *owner;
};

enum hiusb_port_usb_mode {
	HIUSB_PORT_USB_MODE_NONE,
	HIUSB_PORT_USB_MODE_DEVICE,
	HIUSB_PORT_USB_MODE_HOST,
	HIUSB_PORT_USB_MODE_HIFI,
};
struct hiusb_client;
struct hiusb_port;
struct hiusb_client_ops {
	int (*bind)(struct hiusb_client *);
	int (*unbind)(struct hiusb_client *);
	int (*assign)(struct hiusb_client *, enum hiusb_port_usb_mode, struct device *parent);
	int (*deassign)(struct hiusb_client *);
	int (*suspend)(struct hiusb_client *, bool remote_wakeup);
	int (*resume)(struct hiusb_client *, bool remote_wakeup);
	int (*get_portid)(struct hiusb_client *);
	int (*init_hw)(struct hiusb_client *);
	int (*deinit_hw)(struct hiusb_client *, unsigned int keep_power);
	int (*hibernate)(struct hiusb_client *);
	int (*wakeup)(struct hiusb_client *);
	int (* ioctl)(struct hiusb_client *, enum hiusb_port_client_operation, void*);
};

struct list_head;
struct hiusb_client {
	struct device dev;
	const struct hiusb_client_desc *desc;
	const struct hiusb_client_ops *ops;
	struct list_head list;
	struct hiusb_port *port;
};

/* API of hiusb client */
struct hiusb_client *hiusb_client_create(struct device *parent,
					 const struct hiusb_client_desc *desc,
					 const struct hiusb_client_ops *ops);
void hiusb_client_destroy(struct hiusb_client *client);
void hiusb_client_lock_port(struct hiusb_client *client);
void hiusb_client_unlock_port(struct hiusb_client *client);
void *hiusb_client_get_drvdata(struct hiusb_client *client);
void hiusb_client_set_drvdata(struct hiusb_client *client, void *data);
int hiusb_client_unbind_port(struct hiusb_client *client);
int hiusb_client_bind_port(struct hiusb_client *client, struct hiusb_port *port);
int hiusb_client_get_protid(struct hiusb_client *client);
int hiusb_client_detect_charger_type(struct hiusb_client *client);
bool hiusb_client_enumerate_allowed(struct hiusb_client *client);
int hiusb_client_entry_device_mode(struct hiusb_client *client);
void hiusb_client_exit_device_mode(struct hiusb_client *client);
void hiusb_client_handle_no_charger(struct hiusb_client *client);
int hiusb_client_is_device_mode(struct hiusb_client *client);
void hiusb_client_set_port_power_flag(struct hiusb_client *client, int val);
bool hiusb_client_is_usb3_phy(struct hiusb_client *client);
bool hiusb_client_is_dp_mode(struct hiusb_client *client);
/*
 * API for which should be called after lock port,
 * except in callback of struct hiusb_client_ops.
 */
int hiusb_client_init_usb2(struct hiusb_client *client);
int hiusb_client_deinit_usb2(struct hiusb_client *client, unsigned int keep_power);
int hiusb_client_init_usb3(struct hiusb_client *client);
int hiusb_client_deinit_usb3(struct hiusb_client *client);

#endif /* _DRIVERS_HIUSB_CLIENT_H_ */
