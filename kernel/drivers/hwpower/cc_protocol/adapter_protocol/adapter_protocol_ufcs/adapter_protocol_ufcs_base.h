/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_protocol_ufcs_base.h
 *
 * ufcs protocol base function
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

#ifndef _ADAPTER_PROTOCOL_UFCS_BASE_H_
#define _ADAPTER_PROTOCOL_UFCS_BASE_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_ufcs.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_ufcs_auth.h>

#define HWUFCS_REQUEST_COMMUNICATION_RETRY       10

void hwufcs_init_msg_number_lock(void);
void hwufcs_destroy_msg_number_lock(void);
void hwufcs_clear_power_change_data(void);
int hwufcs_get_power_change_curr(unsigned int mode);
int hwufcs_updata_power_change_data(struct hwufcs_package_data *pkt);
int hwufcs_request_communication(bool flag);
int hwufcs_receive_msg(struct hwufcs_package_data *pkt, bool need_wait_msg);
int hwufcs_receive_control_msg(u8 cmd, bool check_msg_num);
int hwufcs_send_control_msg(u8 cmd, bool ack);
int hwufcs_send_specific_msg(void);
int hwufcs_send_control_msg_to_cable(u8 cmd, bool ack);
int hwufcs_send_vdm_control_msg(u8 cmd, bool ack);
int hwufcs_send_request_data_msg(struct hwufcs_request_data *p);
int hwufcs_send_device_information_data_msg(struct hwufcs_dev_info_data *p);
int hwufcs_send_sink_information_data_msg(struct hwufcs_sink_info_data *p);
int hwufcs_send_error_information_data_msg(struct hwufcs_error_info_data *p);
int hwufcs_send_config_watchdog_data_msg(struct hwufcs_wtg_data *p);
int hwufcs_send_verify_request_data_msg(struct hwufcs_verify_request_data *p);
int hwufcs_send_refuse_data_msg(u8 reason, struct hwufcs_package_data *pkt);
void hwufcs_handle_communication_result(int ret, struct hwufcs_package_data *pkt);
int hwufcs_receive_output_capabilities_data_msg(struct hwufcs_capabilities_data *p, u8 *ret_len);
int hwufcs_receive_source_info_data_msg(struct hwufcs_source_info_data *p);
int hwufcs_receive_cable_info_data_msg(struct hwufcs_cable_info_data *p);
int hwufcs_receive_dev_info_data_msg(struct hwufcs_dev_info_data *p);
int hwufcs_receive_error_info_data_msg(struct hwufcs_error_info_data *p);
int hwufcs_receive_verify_response_data_msg(struct hwufcs_verify_response_data *p);
int hwufcs_receive_vdm_source_id_data_msg(u16 *p);
int hwufcs_receive_vdm_scpb_pwr_data_msg(struct adp_scpb_pwr_data *p, u8 *ret_len);

#endif /* _ADAPTER_PROTOCOL_UFCS_BASE_H_ */
