/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HDMITX_EMP_H__
#define __HDMITX_EMP_H__

#include "hdmitx_ctrl.h"
#include <linux/types.h>

#define HDMITX_ORGANIZATION_ID 1
#define HDMITX_HDR_EM_DATA_SIZE 21
#define HDMITX_VRR_EM_DATA_SIZE 4
#define HDMITX_METADATA_CTRL0_REG 0x8C

enum hdmitx_extended_infoframe_type {
	HDMITX_EXTENDED_INFOFRAME_TYPE_NULL = 0x00,
	HDMITX_EXTENDED_INFOFRAME_TYPE_HDR1 = 0x01,
	HDMITX_EXTENDED_INFOFRAME_TYPE_HDR2 = 0x02,
	HDMITX_EXTENDED_INFOFRAME_TYPE_HDR3 = 0x03,
	HDMITX_EXTENDED_INFOFRAME_TYPE_HDR4 = 0x04,
	HDMITX_EXTENDED_INFOFRAME_TYPE_OTHER = 0xff
};

struct hdmitx_ext_metadata {
	enum hdmitx_packet_type type;
	u8 first;
	u8 last;
	u8 sequence_index;
	u8 pkt_new;
	u8 pkt_end;
	u8 ds_type;
	u8 afr;
	u8 vfr;
	u8 sync;
	u8 organization_id;
	u16 data_set_tag;
	u16 data_set_length;
};

struct hdmitx_vt_ext_metadata {
	struct hdmitx_ext_metadata ext_metadata;
	u8 m_const;
	u8 vrr_en;
};

void hdmitx_set_emp(struct hdmitx_ctrl *hdmitx);

#endif /* __HDMITX_EMP_H__ */