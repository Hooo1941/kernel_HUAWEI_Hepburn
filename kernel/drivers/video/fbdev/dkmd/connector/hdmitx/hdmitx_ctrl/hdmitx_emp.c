/**
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

#include <securec.h>
#include "dkmd_log.h"

#include "hdmitx_core_config.h"
#include "hdmitx_reg.h"
#include "hdmitx_vrr.h"
#include "hdmitx_infoframe.h"
#include "hdmitx_emp.h"

#define HDMITX_IF_CHECKSUM_OFFSET 0
#define HDMITX_EMP_CHECK_NUM_SIZE 1
#define HDMITX_EMP_HEADER_SIZE 3
#define HDMITX_EMP_PACKET_SIZE 32

#define HDMITX_META_LOAD_FLAG_MASK BIT(15)
#define HDMITX_META_1ST_EMP_END_MASK GENMASK(14, 9)
#define HDMITX_META_VRR_EN_MASK BIT(2)
#define HDMITX_META_EMP_EN_MASK BIT(0)

#define HDMITX_EMP_CONTENT_HEADER_SIZE 7

u32 g_sequence_index = 0;

static void hdmi_emp_set_checksum(u8 *buffer, u32 size)
{
	buffer[0] = hdmi_infoframe_checksum(buffer, size);
}

static void hdmitx_set_emp_header(u8 *emp_ptr, const struct hdmitx_ext_metadata *em_data)
{
	/* packet header */
	*emp_ptr++ = em_data->type;
	*emp_ptr++ = ((em_data->first & 0x1) << 7) | ((em_data->last & 0x1) << 6); /* bit7 is first, bit6 is last */
	*emp_ptr++ = em_data->sequence_index;

	/* em data set header */
	*emp_ptr++ = ((em_data->pkt_new & 0x1) << 7) | ((em_data->pkt_end & 0x1) << 6) | /* bit7 is pkt_new, bit6 is pkt_end */
		((em_data->ds_type & 0x3) << 4) | ((em_data->afr & 0x1) << 3) | /* bit[5:4] is ds_type, bit3 is afr */
		((em_data->vfr & 0x1) << 2) | ((em_data->sync & 0x1) << 1);  /* bit2 is vfr, bit1 is sync */
	*emp_ptr++ = 0; /* Reserved */
	*emp_ptr++ = em_data->organization_id;
	*emp_ptr++ = (em_data->data_set_tag >> 8) & 0xff; /* high 8bits */
	*emp_ptr++ = em_data->data_set_tag & 0xff;
	*emp_ptr++ = (em_data->data_set_length >> 8) & 0xff; /* high 8bits */
	*emp_ptr++ = em_data->data_set_length & 0xff;
}


static s32 hdmitx_vrr_emp_init(struct hdmitx_vt_ext_metadata *frame, struct vrr_property *vrr)
{
	if (memset_s(frame, sizeof(*frame), 0, sizeof(*frame)) != EOK) {
		dpu_pr_err("memset_s err");
		return -1;
	}

	frame->ext_metadata.type = HDMITX_PACKET_TYPE_EXTENDED_METADATA;
	frame->ext_metadata.first = 1;
	frame->ext_metadata.last = 1;
	frame->ext_metadata.sequence_index = g_sequence_index;
	g_sequence_index++;
	frame->ext_metadata.pkt_new = 1;
	frame->ext_metadata.pkt_end = 0;
	frame->ext_metadata.ds_type = 0;
	frame->ext_metadata.sync = 0;
	frame->ext_metadata.afr = 0;
	frame->ext_metadata.vfr = 1;
	frame->ext_metadata.organization_id = HDMITX_ORGANIZATION_ID;
	frame->ext_metadata.data_set_tag = 1;
	frame->ext_metadata.data_set_length = HDMITX_VRR_EM_DATA_SIZE;

	if (vrr->cinema_vrr == 0) {
		frame->m_const = 0; // when m_const is clear, VRR is constrained to the indicated range.
	} else {
		frame->m_const = 1;
	}
	frame->vrr_en = 1;
	return 0;
}

static s32 hdmitx_vrr_emp_pack(const struct hdmitx_vt_ext_metadata *frame, void *buffer, u32 size)
{
	u8 *emp_ptr = buffer;

	if (frame == NULL || buffer == NULL) {
		dpu_pr_err("null emp ptr");
		return -1;
	}

	if (memset_s(buffer, HDMITX_EMP_PACKET_SIZE, 0, size) != EOK) {
		dpu_pr_err("VRR memset err");
		return -1;
	}

	// chip constrant : first byte is for checksum
	hdmitx_set_emp_header(emp_ptr + HDMITX_EMP_CHECK_NUM_SIZE, &frame->ext_metadata);

	emp_ptr = (u8 *)buffer + HDMITX_EMP_CHECK_NUM_SIZE + HDMITX_EMP_HEADER_SIZE + HDMITX_EMP_CONTENT_HEADER_SIZE;

	// to fix : m_const if need
	*emp_ptr = frame->vrr_en & 0x1;
	hdmi_emp_set_checksum((u8 *)buffer, size);

	return 0;
}

static void hdmitx_send_emp_packet(void *hdmi_reg_base, const u8 *buffer, u8 len)
{
	const u8 *ptr = buffer;
	u32 i;
	u32 data;

	if (buffer == NULL || hdmi_reg_base == NULL) {
		dpu_pr_err("ptr null err");
		return;
	}
	if (len < HDMITX_EMP_PACKET_SIZE) {
		dpu_pr_err("buffer size less than send buffer size");
		return;
	}

	for (i = 0; i < HDMITX_EMP_PACKET_SIZE / 4; i++) {
		data = reg_sub_pktx_pb0(*ptr++);
		data |= reg_sub_pktx_pb1(*ptr++);
		data |= reg_sub_pktx_pb2(*ptr++);
		data |= reg_sub_pktx_pb3(*ptr++);
		/* the address offset of two subpackages is 8bytes */
		hdmi_writel(hdmi_reg_base, 0x90000, data);
	}
}

static s32 hdmitx_set_vrr_emp(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_vt_ext_metadata frame;
	u8 buffer[HDMITX_EMP_PACKET_SIZE] = {0};
	s32 ret;

	ret = hdmitx_vrr_emp_init(&frame, &hdmitx->hdmitx_connector->vrr);
	if (ret < 0) {
		dpu_pr_err("Failed to pack VRR emp");
		return -1;
	}

	ret = hdmitx_vrr_emp_pack(&frame, buffer, sizeof(buffer));
	if (ret < 0) {
		dpu_pr_err("Failed to pack VRR emp");
		return -1;
	}

	dpu_pr_info("EMP header : %02x %02x %02x\n"
		"VTEM header: %02x %02x %02x %02x %02x %02x %02x\n"
		"VTEM body: %02x %02x %02x %02x\n",
		/* header of vendor emp */
		buffer[1], buffer[2], buffer[3],
		/* header of em data set */
		buffer[4], buffer[5], buffer[6], buffer[7],
		buffer[8], buffer[9], buffer[10],
		/* body of vendor emp */
		buffer[11], buffer[12], buffer[13], buffer[14]);

	hdmitx_send_emp_packet(hdmitx->base, buffer, sizeof(buffer));

	/* enable vrr */
	hdmi_write_bits(hdmitx->sysctrl_base, HDMITX_METADATA_CTRL0_REG,
		HDMITX_META_1ST_EMP_END_MASK, 0x2);
	hdmi_write_bits(hdmitx->sysctrl_base, HDMITX_METADATA_CTRL0_REG,
		HDMITX_META_VRR_EN_MASK, 0x1);
	hdmi_write_bits(hdmitx->sysctrl_base, HDMITX_METADATA_CTRL0_REG,
		HDMITX_META_LOAD_FLAG_MASK, 0x1);
	hdmi_write_bits(hdmitx->sysctrl_base, HDMITX_METADATA_CTRL0_REG,
		HDMITX_META_EMP_EN_MASK, 0x1);

	return 0;
}

void hdmitx_set_emp(struct hdmitx_ctrl *hdmitx)
{
	s32 ret;
	dpu_pr_info("+");

	if (!g_hdmitx_vrr_emp_enable_debug) {
		dpu_pr_warn("vrr_emp_enable is false");
		return;
	}

	if (hdmitx == NULL) {
		dpu_pr_err("Input params is NULL pointer");
		return;
	}

	// g_sequence_index to fix;

	ret = hdmitx_set_vrr_emp(hdmitx);
	if (ret)
		dpu_pr_err("Send VRR emp fail %d", ret);

	dpu_pr_info("-");
	return;
}

