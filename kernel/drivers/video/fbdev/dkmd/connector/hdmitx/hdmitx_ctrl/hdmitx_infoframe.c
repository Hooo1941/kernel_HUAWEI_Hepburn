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

#include "hdmitx_infoframe.h"
#include "hdmitx_core_config.h"

#define HDMITX_IF_CHECKSUM_OFFSET 3
#define HDMITX_DRM_IF_VER_1       1
#define HDMITX_AVI_IF_VER_2       2
#define HDMITX_AVI_IF_VER_3       3
#define HDMITX_AVI_IF_VER_4       4

u8 hdmi_infoframe_checksum(const u8 *ptr, u32 size)
{
	u8 check_sum = 0;
	u32 i;
	u8 ret;
	dpu_check_and_return(!ptr, -1, err, "null ptr");

	/* count checksum */
	for (i = 0; i < size; i++)
		check_sum += ptr[i];

	ret = (u8)(0x100 - check_sum);
	return ret;
}

static void hdmi_infoframe_set_checksum(void *buffer, u32 size)
{
	u8 *ptr = buffer;

	ptr[HDMITX_IF_CHECKSUM_OFFSET] = hdmi_infoframe_checksum(buffer, size);
}

static s32 hdmitx_avi_infoframe_init(struct hdmitx_avi_infoframe *frame)
{
	if (frame == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	if (memset_s(frame, sizeof(*frame), 0, sizeof(*frame)) != EOK) {
		dpu_pr_err("memset_s err");
		return -1;
	}

	frame->type = HDMITX_INFOFRAME_TYPE_AVI;
	frame->version = HDMITX_AVI_IF_VER_2;
	frame->length = HDMITX_AVI_INFOFRAME_SIZE;

	return 0;
}

static s32 hdmitx_avi_infoframe_check(const struct hdmitx_avi_infoframe *frame)
{
	if (frame->type != HDMITX_INFOFRAME_TYPE_AVI ||
		(frame->version != HDMITX_AVI_IF_VER_2 &&
		frame->version != HDMITX_AVI_IF_VER_3 &&
		frame->version != HDMITX_AVI_IF_VER_4) ||
		frame->length != HDMITX_AVI_INFOFRAME_SIZE)
		return -1;

	if (frame->picture_aspect > HDMITX_PICTURE_ASPECT_256_135)
		return -1;

	return 0;
}

static void hdmitx_avi_infoframe_pack_remain(const struct hdmitx_avi_infoframe *frame, u8 *ptr)
{
	*ptr++ = frame->video_code;
	*ptr++ = (((u8)frame->ycc_quantization_range & 0x3) << 6) | /* bit[7:6] is ycc_quantization_range */
			 (((u8)frame->content_type & 0x3) << 4) | /* bit[5:4] is content_type */
			 (frame->pixel_repeat & 0xf);
	*ptr++ = frame->top_bar & 0xff;
	*ptr++ = (frame->top_bar >> 8) & 0xff; /* high 8bits */
	*ptr++ = frame->bottom_bar & 0xff;
	*ptr++ = (frame->bottom_bar >> 8) & 0xff; /* high 8bits */
	*ptr++ = frame->left_bar & 0xff;
	*ptr++ = (frame->left_bar >> 8) & 0xff; /* high 8bits */
	*ptr++ = frame->right_bar & 0xff;
	*ptr++ = (frame->right_bar >> 8) & 0xff; /* high 8bits */
}

static s32 hdmitx_avi_infoframe_pack(const struct hdmitx_avi_infoframe *frame, void *buffer, u32 size)
{
	u8 *ptr = buffer;
	u8 tmp;
	u32 length;
	s32 ret;

	if (frame == NULL || buffer == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	ret = hdmitx_avi_infoframe_check(frame);
	if (ret) {
		dpu_pr_err("avi check err");
		return ret;
	}

	length = HDMITX_INFOFRAME_HEADER_SIZE + frame->length;
	if (size < length) {
		dpu_pr_err("avi packet over size");
		return -1;
	}

	if (memset_s(buffer, size, 0, size) != EOK) {
		dpu_pr_err("avi memset_s err");
		return -1;
	}

	*ptr++ = frame->type;
	*ptr++ = frame->version;
	*ptr++ = frame->length;
	*ptr++ = 0; /* checksum */

	/* Data byte 1, bit[6:5] is colorspace, bit[1:0] is scan mode */
	tmp = (((u8)frame->colorspace & 0x3) << 5) | ((u8)frame->scan_mode & 0x3);
	/* Data byte 1, bit 4 has to be set if we provide the active format aspect ratio */
	if ((u8)frame->active_aspect & 0xf)
		tmp |= (1 << 4); /* bit 4 is set to 1 */

	/* Bit 3 and 2 indicate if we transmit horizontal/vertical bar data */
	if (frame->top_bar || frame->bottom_bar)
		tmp |= (1 << 3); /* bit 3 is set to 1 */

	if (frame->left_bar || frame->right_bar)
		tmp |= (1 << 2); /* bit 2 is set to 1 */

	*ptr++ = tmp;

	/* bit[7:6] is colorimetry, bit[5:4] is picture aspect */
	*ptr++ = (((u8)frame->colorimetry & 0x3) << 6) | (((u8)frame->picture_aspect & 0x3) << 4) |
			((u8)frame->active_aspect & 0xf);

	/* bit[6:4] is extended_colorimetry, bit[3:2] is quantization_range */
	tmp = (((u8)frame->extended_colorimetry & 0x7) << 4) | (((u8)frame->quantization_range & 0x3) << 2) |
			((u8)frame->nups & 0x3);
	if (frame->itc)
		tmp |= 1 << 7; /* bit7 is itc */

	*ptr++ = tmp;

	hdmitx_avi_infoframe_pack_remain(frame, ptr);

	hdmi_infoframe_set_checksum(buffer, length);

	return length;
}

static void hdmitx_avi_infoframe_config(struct hdmitx_soft_status *soft_status,
	struct hdmitx_avi_infoframe *frame)
{
	struct hdmitx_avi_data *avi_data = NULL;
	struct dpu_video_info *info = NULL;
	u32 colorimetry;
	u32 extended_colorimetry;

	info = &soft_status->info;
	avi_data = &soft_status->info.avi_data;
	colorimetry = avi_data->color.colorimetry & 0xf;
	extended_colorimetry = (avi_data->color.colorimetry >> 4) & 0xf; /* The upper 4 bits is extended colorimetry. */

	frame->pixel_repeat = avi_data->pixel_repeat;
	frame->video_code = (avi_data->vic > 255) ? 0 : (avi_data->vic & 0xff); /* vic > 255 is not CEA timing. */

	/*
	 * The video code must be 0 in the avi infoframe, when the timing
	 * is HDMITX_3840X2160P24_16_9(93), HDMITX_3840X2160P25_16_9(94),
	 * HDMITX_3840X2160P30_16_9(95), HDMITX_4096X2160P24_256_135(98) at 2D mode.
	 */
	if ((frame->video_code == VIC_3840X2160P24_16_9 ||
		frame->video_code == VIC_3840X2160P25_16_9 ||
		frame->video_code == VIC_3840X2160P30_16_9 ||
		frame->video_code == VIC_4096X2160P24_256_135) &&
		info->mode_3d == HDMITX_3D_NONE)
		frame->video_code = 0;

	frame->picture_aspect = avi_data->picture_aspect_ratio;
	frame->content_type = avi_data->it_content_type;
	frame->itc = avi_data->it_content_valid;
	frame->active_aspect = avi_data->active_aspect_ratio;
	frame->scan_mode = avi_data->scan_info;
	frame->colorspace = avi_data->color.color_format;
	frame->colorimetry = colorimetry;
	frame->extended_colorimetry = extended_colorimetry;
	frame->quantization_range = avi_data->color.rgb_quantization;
	frame->ycc_quantization_range = avi_data->color.ycc_quantization;
	frame->top_bar = avi_data->top_bar;
	frame->bottom_bar = avi_data->bottom_bar;
	frame->left_bar = avi_data->left_bar;
	frame->right_bar = avi_data->right_bar;
	frame->nups = avi_data->picture_scal;

	/* Y2 = 1 or vic >= 128, version shall use 3 */
	if (frame->video_code > VIC_5120X2160P100_64_27) {
		frame->version = 3; /* Avi infoframe version need be 3, when vic > 127. */
	} else if (frame->colorimetry == HDMITX_COLORIMETRY_EXTENDED &&
		frame->extended_colorimetry == HDMITX_EXTENDED_COLORIMETRY_RESERVED) {
		frame->version = 4; /* Avi infoframe version need be 4, when extended colorimetry is reserved. */
	}
}

static s32 hdmitx_set_avi_infoframe(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_avi_infoframe frame;
	u8 buffer[HDMITX_PACKET_SIZE] = {0};
	ssize_t err;

	if (g_hdmitx_avi_enable_debug == 0) {
		dpu_pr_warn("avi enable debug is false");
		return 0;
	}

	err = hdmitx_avi_infoframe_init(&frame);
	if (err < 0) {
		dpu_pr_warn("Failed to setup avi infoframe: %zd", err);
		return err;
	}

	hdmitx_avi_infoframe_config(&hdmitx->soft_status, &frame);

	err = hdmitx_avi_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		dpu_pr_warn("Failed to pack AVI infoframe: %zd", err);
		return err;
	}

	dpu_pr_info("avi infoframe : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		/* 0,1,2,3:header of avi infoframe */
		buffer[0], buffer[1], buffer[2], buffer[3],
		/* 4,5,6,7,8,9:avi infoframe body */
		buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9],
		/* 10,11,12,13,14,15:avi infoframe body */
		buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);

	core_hw_send_info_frame(hdmitx->base, buffer, sizeof(buffer));
	return 0;
}

/* H14b-VSIF */
static s32 hdmitx_vendor_infoframe_init(struct hdmitx_vendor_infoframe *frame)
{
	if (frame == NULL) {
		dpu_pr_warn("null ptr!");
		return -1;
	}

	if (memset_s(frame, sizeof(*frame), 0, sizeof(*frame)) != EOK) {
		dpu_pr_warn("memset_s err");
		return -1;
	}

	frame->type = HDMITX_INFOFRAME_TYPE_VENDOR;
	frame->version = 1;
	frame->oui = HDMITX_IEEE_ID;
	/* 0 is a valid value for s3d_struct, so we use a special "not set" value */
	frame->s3d_struct = HDMITX_3D_STRUCTURE_INVALID;
	frame->length = HDMITX_VENDOR_INFOFRAME_SIZE_OF_NONE;

	return 0;
}

static s32 hdmitx_vendor_infoframe_check(const struct hdmitx_vendor_infoframe *frame)
{
	if (frame->type != HDMITX_INFOFRAME_TYPE_VENDOR ||
		frame->version != 1 ||
		frame->oui != HDMITX_IEEE_ID)
		return -1;

	if (frame->length == HDMITX_VENDOR_INFOFRAME_SIZE_OF_DOLBYV0)
		return 0;

	if (frame->s3d_struct != HDMITX_3D_STRUCTURE_INVALID) {
		if (frame->length != HDMITX_VENDOR_INFOFRAME_SIZE_OF_3D)
			return -1;
	} else if (frame->vic != 0) {
		if (frame->length != HDMITX_VENDOR_INFOFRAME_SIZE_OF_4K)
			return -1;
	} else {
		if (frame->length != HDMITX_VENDOR_INFOFRAME_SIZE_OF_NONE)
			return -1;
	}

	return 0;
}

static s32 hdmitx_vendor_infoframe_pack(const struct hdmitx_vendor_infoframe *frame,
	void *buffer, u32 size)
{
	u8 *ptr = buffer;
	u32 length;
	s32 ret;

	if (frame == NULL || buffer == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	ret = hdmitx_vendor_infoframe_check(frame);
	if (ret) {
		dpu_pr_err("VSIF check err");
		return ret;
	}

	length = HDMITX_INFOFRAME_HEADER_SIZE + frame->length;
	if (size < length) {
		dpu_pr_err("VSIF over size");
		return -1;
	}

	if (memset_s(buffer, size, 0, size) != EOK) {
		dpu_pr_err("vsif memset_s err");
		return -1;
	}

	*ptr++ = frame->type;
	*ptr++ = frame->version;
	*ptr++ = frame->length;
	*ptr++ = 0; /* checksum */

	/* HDMITX OUI */
	*ptr++ = 0x03;
	*ptr++ = 0x0c;
	*ptr++ = 0x00;

	if (frame->s3d_struct != HDMITX_3D_STRUCTURE_INVALID) {
		*ptr++ = 0x2 << 5; /* bit[7:5] is video format */
		*ptr++ = ((u8)frame->s3d_struct & 0xf) << 4; /* bit[7:4] is s3d_struct */
		if (frame->s3d_struct >= HDMITX_3D_STRUCTURE_SIDE_BY_SIDE_HALF)
			*ptr++ = (frame->s3d_ext_data & 0xf) << 4; /* bit[7:4] is s3d_ext_data */
	} else if (frame->vic) {
		*ptr++ = 0x1 << 5; /* bit[7:5] is video format */
		*ptr++ = frame->vic;
	}

	hdmi_infoframe_set_checksum(buffer, length);

	return length;
}

static void hdmitx_vsif_infoframe_config(struct hdmitx_soft_status *soft_status,
	struct hdmitx_vendor_infoframe *frame)
{
	struct hdmitx_avi_data *avi_data = NULL;
	struct dpu_video_info *info = NULL;

	info = &soft_status->info;
	avi_data = &info->avi_data;

	if (info->mode_3d == HDMITX_3D_NONE) {
		if (avi_data->vic == VIC_3840X2160P30_16_9) {
			frame->vic = 1; /* hdmi vic is 1 */
		} else if (avi_data->vic == VIC_3840X2160P25_16_9) {
			frame->vic = 2; /* hdmi vic is 2 */
		} else if (avi_data->vic == VIC_3840X2160P24_16_9) {
			frame->vic = 3; /* hdmi vic is 3 */
		} else if (avi_data->vic == VIC_4096X2160P24_256_135) {
			frame->vic = 4; /* hdmi vic is 4 */
		}
	}

	frame->s3d_struct = (info->mode_3d == HDMITX_3D_NONE) ?
		HDMITX_3D_STRUCTURE_INVALID : info->mode_3d;

	if (frame->vic) {
		frame->length = 5; /* hdmi vic is not zero, the length must be 5. */
	} else if (frame->s3d_struct != HDMITX_3D_STRUCTURE_INVALID) {
		frame->length = 7; /* 3d struct is not none, the length must be 7. */
	}
}

static s32 hdmitx_set_vendor_specific_infoframe(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_vendor_infoframe frame;
	u8 buffer[HDMITX_PACKET_SIZE] = {0};
	ssize_t err;

	if (g_hdmitx_all_vsif_enable_debug == 0) {
		dpu_pr_warn("all vsif enable is false");
		return 0;
	}

	err = hdmitx_vendor_infoframe_init(&frame);
	if (err < 0) {
		dpu_pr_warn("Failed to setup vendor infoframe: %zd", err);
		return err;
	}

	hdmitx_vsif_infoframe_config(&hdmitx->soft_status, &frame);

	err = hdmitx_vendor_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		dpu_pr_warn("Failed to pack vendor infoframe: %zd", err);
		return err;
	}

	dpu_pr_info("vendor infoframe : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		/* 0,1,2,3:header of vendor infoframe */
		buffer[0], buffer[1], buffer[2], buffer[3],
		/* 4,5,6,7,8,9,10:vendor infoframe body */
		buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]);

	core_hw_send_info_frame(hdmitx->base, buffer, sizeof(buffer));
	return 0;
}

/* HF-VSIF */
static s32 hdmitx_forum_vendor_infoframe_init(struct hdmitx_forum_vendor_infoframe *frame)
{
	if (frame == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	if (memset_s(frame, sizeof(*frame), 0, sizeof(*frame)) != EOK) {
		dpu_pr_err("memset_s err");
		return -1;
	}

	frame->type = HDMITX_INFOFRAME_TYPE_VENDOR;
	frame->version = 1;
	frame->oui = HDMITX_FORUM_IEEE_ID;
	/* 0 is a valid value for s3d_struct, so we use a special "not set" value */
	frame->length = 6; /* 6: min length */
	frame->hf_version = 1;

	return 0;
}

static s32 hdmitx_forum_vendor_infoframe_check(const struct hdmitx_forum_vendor_infoframe *frame)
{
	if (frame->type != HDMITX_INFOFRAME_TYPE_VENDOR ||
		frame->version != 1 ||
		frame->oui != HDMITX_FORUM_IEEE_ID)
		return -1;

	return 0;
}

static void hdmitx_forum_vendor_3d_pack(const struct hdmitx_3d_infoframe *s3d_infoframe, void *buffer)
{
	int i;
	u8 *ptr = buffer;

	*ptr++ = (((u8)s3d_infoframe->s3d_struct & 0xf) << 4) | /* high 4 bits is s3d_struct */
			((u8)s3d_infoframe->additional_info_present << 3) | /* bit3 is additional_info_present */
			((u8)s3d_infoframe->disparity_data_present << 2) | /* bit2 is disparity_data_present */
			((u8)s3d_infoframe->meta_present << 1); /* bit1 is meta_present. */
	if (s3d_infoframe->s3d_struct >= HDMITX_3D_STRUCTURE_SIDE_BY_SIDE_HALF)
		*ptr++ = (s3d_infoframe->s3d_ext_data & 0xf) << 4; /* bit[7:4] is s3d_ext_data */

	if (s3d_infoframe->additional_info_present)
		*ptr++ = ((u8)s3d_infoframe->dual_view << 4) | /* bit[5:4] is dual_view */
				 ((s3d_infoframe->view_dependency & 0x3) << 2) | /* bit[3:2] is dual_view */
				 (s3d_infoframe->preferred_2d_view & 0x3);

	if (s3d_infoframe->disparity_data_present) {
		*ptr++ = ((s3d_infoframe->disparity_data_version & 0x7) << 5) | /* bit[7:5] is disparity_data_version. */
				 (s3d_infoframe->disparity_data_length & 0x1f);
		for (i = 0; i < s3d_infoframe->disparity_data_length; i++)
			*ptr++ = s3d_infoframe->disparity_data[i];
	}
	if (s3d_infoframe->meta_present) {
		*ptr++ = ((s3d_infoframe->metadata_type & 0x7) << 5) | /* bit[7:5] is metadata_type. */
				 (s3d_infoframe->metadata_length & 0x1f);
		for (i = 0; i < s3d_infoframe->metadata_length; i++)
			*ptr++ = s3d_infoframe->metadata[i];
	}
}

static s32 hdmitx_forum_vendor_infoframe_pack(const struct hdmitx_forum_vendor_infoframe *frame,
	void *buffer, u32 size)
{
	s32 ret;
	u8 *ptr = buffer;
	u32 length;

	if (frame == NULL || buffer == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	ret = hdmitx_forum_vendor_infoframe_check(frame);
	if (ret) {
		dpu_pr_err("forum vendor infoframe check fail");
		return ret;
	}

	length = HDMITX_INFOFRAME_HEADER_SIZE + frame->length;
	if (size < length) {
		dpu_pr_err("size err");
		return -1;
	}

	if (memset_s(buffer, size, 0, size) != EOK) {
		dpu_pr_err("memset_s err");
		return -1;
	}

	*ptr++ = frame->type;
	*ptr++ = frame->version;
	*ptr++ = frame->length;
	*ptr++ = 0; /* checksum */

	/* HDMITX OUI */
	*ptr++ = frame->oui & 0xff;
	*ptr++ = (frame->oui >> 8) & 0xff; /* oui ringt shift 8 bits. */
	*ptr++ = (frame->oui >> 16) & 0xff; /* oui ringt shift 16 bits. */
	*ptr++ = frame->hf_version;

	/* high 4bits is ccpbc, bit1 is allm. */
	*ptr++ = ((frame->ccpbc & 0xf) << 4) | (((u8)frame->allm & 0x1) << 1) | (u8)frame->s3d_valid;

	/* 3d pack */
	if (frame->s3d_valid)
		hdmitx_forum_vendor_3d_pack(&frame->s3d_infoframe, ptr);

	hdmi_infoframe_set_checksum(buffer, length);
	return length;
}

static void hdmitx_hf_infoframe_config(struct hdmitx_soft_status *soft_status,
	struct hdmitx_forum_vendor_infoframe *frame)
{
	struct dpu_video_info *info = NULL;
	struct hdmitx_3d_infoframe *s3d_infoframe = NULL;
	s32 i;

	info = &soft_status->info;

	frame->allm = info->allm_enable;
	frame->s3d_valid = info->s3d_valid_enable;

	if (frame->s3d_valid) {
		frame->length++;
		s3d_infoframe = &frame->s3d_infoframe;

		if (s3d_infoframe->s3d_struct >= HDMITX_3D_STRUCTURE_SIDE_BY_SIDE_HALF)
			frame->length++;

		if (s3d_infoframe->additional_info_present)
			frame->length++;

		if (s3d_infoframe->disparity_data_present) {
			frame->length++;
			for (i = 0; i < s3d_infoframe->disparity_data_length; i++)
				frame->length++;
		}
		if (s3d_infoframe->meta_present) {
			frame->length++;
			for (i = 0; i < s3d_infoframe->metadata_length; i++)
				frame->length++;
		}
	}
}

static s32 hdmitx_set_forum_vendor_specific_infoframe(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_forum_vendor_infoframe frame;
	u8 buffer[HDMITX_PACKET_SIZE] = {0};
	ssize_t err;

	if (g_hdmitx_all_vsif_enable_debug == 0) {
		dpu_pr_warn("all vsif_enable is false");
		return 0;
	}

	err = hdmitx_forum_vendor_infoframe_init(&frame);
	if (err < 0) {
		dpu_pr_warn("Failed to setup HF vendor infoframe: %zd", err);
		return err;
	}

	hdmitx_hf_infoframe_config(&hdmitx->soft_status, &frame);
	err = hdmitx_forum_vendor_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		dpu_pr_warn("Failed to pack HF vendor infoframe: %zd", err);
		return err;
	}

	dpu_pr_info("HF vendor infoframe : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		/* 0,1,2,3:header of vendor infoframe */
		buffer[0], buffer[1], buffer[2], buffer[3],
		/* 4,5,6,7,8,9,10:vendor infoframe body */
		buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]);

	core_hw_send_info_frame(hdmitx->base, buffer, sizeof(buffer));
	return 0;
}

/* Gamut */
static s32 hdmitx_gamut_packet_pack(void *buffer, u32 size)
{
	u8 *ptr = buffer;
	u32 length;

	if (buffer == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	if (memset_s(buffer, size, 0, size) != EOK) {
		dpu_pr_err("memset_s err");
		return -1;
	}

	*ptr++ = 0x0a;
	*ptr++ = 0; // HB1
	*ptr++ = 0x0; // HB2
	*ptr++ = 0x1; // PB0
	length = 4;

	// to fix : checksum
	return length;
}

static s32 hdmitx_set_gamut_packet(struct hdmitx_ctrl *hdmitx)
{
	u8 buffer[HDMITX_PACKET_SIZE] = {0};

	ssize_t err;

	if (g_hdmitx_gmp_enable_debug == 0) {
		dpu_pr_warn("gcp_enable is false");
		return 0;
	}

	err = hdmitx_gamut_packet_pack(buffer, sizeof(buffer));
	if (err < 0) {
		dpu_pr_warn("Failed to pack gamut: %zd", err);
		return err;
	}

	dpu_pr_info("gamut metadata packet: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		/* 0,1,2,3:header of vendor infoframe */
		buffer[0], buffer[1], buffer[2], buffer[3],
		/* 4,5,6,7,8,9,10:vendor infoframe body */
		buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]);

	core_hw_send_info_frame(hdmitx->base, buffer, sizeof(buffer));
	return 0;
}

static s32 hdmitx_drm_infoframe_init(struct hdmitx_drm_infoframe *frame)
{
	if (frame == NULL) {
		dpu_pr_err("null ptr!");
		return -1;
	}

	if (memset_s(frame, sizeof(*frame), 0, sizeof(*frame)) != EOK) {
		dpu_pr_err("memset_s err");
		return -1;
	}

	frame->type = HDMITX_INFOFRAME_TYPE_DRM;
	frame->version = HDMITX_DRM_IF_VER_1;
	frame->length = HDMITX_DRM_INFOFRAME_SIZE;

	return 0;
}

static void hdmitx_drm_infoframe_config(struct hdmitx_soft_status *soft_status,
	struct hdmitx_hdr_metadata *hdr_metadata, struct hdmitx_drm_infoframe *frame)
{
	u32 type;
	int drm_type = HDR_SDR;
	struct hdmitx_hdr_data *hdr_data = NULL;
	struct dpu_video_info *info = NULL;
	struct hdmitx_meta_descriptor_1st *type1 = &frame->meta_descriptor.type1;

	hdr_data = &soft_status->info.hdr_data;
	type = hdr_data->hdr_mode_type;

	info = &soft_status->info;

	if (type == HDMITX_HDR_MODE_STATIC_TRD_HDR) { // fixme
		drm_type = HDR_HDR10;
		frame->eotf_type = HDMITX_EOTF_SMPTE_ST_2084;
	} else if (type == HDMITX_HDR_MODE_STATIC_HLG) {
		drm_type = HDR_HLG;
		frame->eotf_type = HDMITX_EOTF_HLG;
	} else {
		drm_type = HDR_SDR;
		frame->eotf_type = HDMITX_EOTF_SDR_LUMIN;
	}

	type = HDMITX_HDR_MODE_STATIC_TRD_HDR; // to fix
	frame->eotf_type = HDMITX_EOTF_SMPTE_ST_2084; // to fix

	frame->metadata_type = HDMITX_HDR_METADATA_ID_0;
	if (type == HDMITX_HDR_MODE_STATIC_HLG || type == HDMITX_HDR_MODE_STATIC_TRD_HDR) {
		type1->primaries0_x = hdr_metadata->red_primary_x;
		type1->primaries0_y = hdr_metadata->red_primary_y;
		type1->primaries1_x = hdr_metadata->green_primary_x;
		type1->primaries1_y = hdr_metadata->green_primary_y;
		type1->primaries2_x = hdr_metadata->blue_primary_x;
		type1->primaries2_y = hdr_metadata->blue_primary_y;
		type1->white_point_x = hdr_metadata->white_point_x;
		type1->white_point_y = hdr_metadata->white_point_y;
		type1->max_luminance = hdr_metadata->max_display_mastering_luminance;
		type1->min_luminance = hdr_metadata->min_display_mastering_luminance;
		type1->max_light_level = hdr_metadata->max_content_light_level;
		type1->average_light_level = hdr_metadata->max_frame_average_light_level;

		// to fix
		type1->primaries0_x = 13250;
		type1->primaries0_y = 34500;
		type1->primaries1_x = 7500;
		type1->primaries1_y = 3000;
		type1->primaries2_x = 34000;
		type1->primaries2_y = 16000;
		type1->white_point_x = 15635;
		type1->white_point_y = 16450;
		type1->max_luminance = 1000;
		type1->min_luminance = 50;
		type1->max_light_level = 1000;
		type1->average_light_level = 200;
	} else {
		if (memset_s(type1, sizeof(*type1), 0x0, sizeof(*type1)) != EOK) {
			dpu_pr_err("memset_s fail");
			return;
		}
	}
}

static void hdmitx_drm_infoframe_pack_remain(const struct hdmitx_meta_descriptor_1st *descriptor_1st,
	u8 *ptr)
{
	*ptr++ = descriptor_1st->primaries0_x & 0xff;
	*ptr++ = (descriptor_1st->primaries0_x >> 8) & 0xff; /* high 8bits */
	*ptr++ = descriptor_1st->primaries0_y & 0xff;
	*ptr++ = (descriptor_1st->primaries0_y >> 8) & 0xff; /* high 8bits */

	*ptr++ = descriptor_1st->primaries1_x & 0xff;
	*ptr++ = (descriptor_1st->primaries1_x >> 8) & 0xff; /* high 8bits */
	*ptr++ = descriptor_1st->primaries1_y & 0xff;
	*ptr++ = (descriptor_1st->primaries1_y >> 8) & 0xff; /* high 8bits */

	*ptr++ = descriptor_1st->primaries2_x & 0xff;
	*ptr++ = (descriptor_1st->primaries2_x >> 8) & 0xff; /* high 8bits */
	*ptr++ = descriptor_1st->primaries2_y & 0xff;
	*ptr++ = (descriptor_1st->primaries2_y >> 8) & 0xff; /* high 8bits */

	*ptr++ = descriptor_1st->white_point_x & 0xff;
	*ptr++ = (descriptor_1st->white_point_x >> 8) & 0xff; /* high 8bits */
	*ptr++ = descriptor_1st->white_point_y & 0xff;
	*ptr++ = (descriptor_1st->white_point_y >> 8) & 0xff; /* high 8bits */

	*ptr++ = descriptor_1st->max_luminance & 0xff;
	*ptr++ = (descriptor_1st->max_luminance >> 8) & 0xff; /* high 8bits */
	*ptr++ = descriptor_1st->min_luminance & 0xff;
	*ptr++ = (descriptor_1st->min_luminance >> 8) & 0xff; /* high 8bits */

	*ptr++ = descriptor_1st->max_light_level & 0xff;
	*ptr++ = (descriptor_1st->max_light_level >> 8) & 0xff; /* high 8bits */

	*ptr++ = descriptor_1st->average_light_level & 0xff;
	*ptr++ = (descriptor_1st->average_light_level >> 8) & 0xff; /* high 8bits */
}

static s32 hdmitx_drm_infoframe_check(const struct hdmitx_drm_infoframe *frame)
{
	if (frame->type != HDMITX_INFOFRAME_TYPE_DRM ||
		frame->version != 1) {
		dpu_pr_err("type fail");
		return -1;
	}

	if (frame->length != HDMITX_DRM_INFOFRAME_SIZE) {
		dpu_pr_err("size fail");
		return -1;
	}

	if (frame->eotf_type > HDMITX_EOTF_HLG ||
		frame->metadata_type != HDMITX_HDR_METADATA_ID_0) {
		dpu_pr_err("type mismatch");
		return -1;
	}

	return 0;
}

static s32 hdmitx_drm_infoframe_pack(const struct hdmitx_drm_infoframe *frame, void *buffer, u32 size)
{
	u8 *ptr = buffer;
	u32 length;
	s32 ret;
	const struct hdmitx_meta_descriptor_1st *descriptor_1st = NULL;

	if (frame == NULL || buffer == NULL) {
		dpu_pr_err("null ptr");
		return -1;
	}

	ret = hdmitx_drm_infoframe_check(frame);
	if (ret) {
		dpu_pr_err("drm check err");
		return ret;
	}

	descriptor_1st = &frame->meta_descriptor.type1;

	length = HDMITX_INFOFRAME_HEADER_SIZE + frame->length;
	if (size < length) {
		dpu_pr_err("drm packet over size");
		return -1;
	}

	if (memset_s(buffer, size, 0, size) != EOK) {
		dpu_pr_err("drm memset_s err");
		return -1;
	}

	*ptr++ = frame->type;
	*ptr++ = frame->version;
	*ptr++ = frame->length;
	*ptr++ = 0; /* checksum */

	*ptr++ = frame->eotf_type;
	*ptr++ = frame->metadata_type;

	hdmitx_drm_infoframe_pack_remain(descriptor_1st, ptr);

	hdmi_infoframe_set_checksum(buffer, length);

	return length;
}

static s32 hdmitx_set_drm_infoframe(struct hdmitx_ctrl *hdmitx)
{
	// check rx support static hdr
	struct hdmitx_drm_infoframe frame;
	u8 buffer[HDMITX_PACKET_SIZE] = {0};
	ssize_t err;

	if (g_hdmitx_static_hdr_enable_debug == 0) {
		dpu_pr_warn("static_hdr is false");
		return 0;
	}

	if (!hdmitx->hdmitx_connector->hdr.st_metadata.s_type1) {
		dpu_pr_info("rx not support static hdr");
		return 0;
	}

	err = hdmitx_drm_infoframe_init(&frame);
	if (err < 0) {
		dpu_pr_warn("Failed to setup drm infoframe: %zd", err);
		return err;
	}

	hdmitx_drm_infoframe_config(&hdmitx->soft_status, &hdmitx->hdr_metadata, &frame); // from user or stub

	err = hdmitx_drm_infoframe_pack(&frame, buffer, sizeof(buffer));
	if (err < 0) {
		dpu_pr_warn("Failed to pack drm infoframe: %zd", err);
		return err;
	}

	dpu_pr_info("drm infoframe : %02x %02x %02x \
		%02x %02x %02x %02x %02x %02x %02x \
		%02x %02x %02x %02x %02x %02x \
		%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\
		%02x %02x %02x %02x",
		/* 0,1,2:header of drm infoframe */
		buffer[0], buffer[1], buffer[2],
		/* 3,4,5,6,7,8,9:drm infoframe body */
		buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9],
		/* 10,11,12,13,14,15:drm infoframe body */
		buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15],
		buffer[16], buffer[17], buffer[18], buffer[19], buffer[20], buffer[21],
		buffer[22], buffer[23], buffer[24], buffer[25], buffer[26]);

	core_hw_send_info_frame(hdmitx->base, buffer, sizeof(buffer));

	return 0;
}

static bool hdmitx_is_need_hf_infoframe(struct hdmitx_ctrl *hdmitx)
{
	struct dpu_video_info *info = NULL;

	info = &hdmitx->soft_status.info;
	if (info->allm_enable || info->s3d_valid_enable) {
		dpu_pr_info("hf infoframe is need");
		return true;
	}
	return false;
}

s32 hdmitx_set_infoframe(struct hdmitx_ctrl *hdmitx)
{
	s32 ret;

	if (hdmitx == NULL) {
		dpu_pr_err("NULL pointer");
		return -1;
	}

	ret = hdmitx_set_avi_infoframe(hdmitx);
	if (ret) {
		dpu_pr_warn("Send avi infoframe fail %d", ret);
		return ret;
	}

	if (hdmitx_is_need_hf_infoframe(hdmitx)) {
		ret = hdmitx_set_forum_vendor_specific_infoframe(hdmitx);
		if (ret) {
			dpu_pr_warn("Send vendor specific infoframe fail %d", ret);
			return ret;
		}
	} else {
		ret = hdmitx_set_vendor_specific_infoframe(hdmitx);
		if (ret) {
			dpu_pr_warn("Send vendor specific infoframe fail %d", ret);
			return ret;
		}
	}

	ret = hdmitx_set_gamut_packet(hdmitx);
	if (ret) {
		dpu_pr_warn("Send gammut infoframe fail %d", ret);
		return ret;
	}

	ret = hdmitx_set_drm_infoframe(hdmitx);
	if (ret) {
		dpu_pr_warn("Send drm infoframe fail %d", ret);
		return ret;
	}

	return ret;
}

static void hdmitx_avi_data_init(struct hdmitx_display_mode *select_mode, struct dpu_video_info *info)
{
	struct hdmitx_avi_data *avi = &info->avi_data;
	struct hdmi_detail *detail = &select_mode->detail;

	avi->vic = detail->vic;

	avi->scan_info = HDMITX_SCAN_MODE_NONE; // 0
	avi->bar_present = HDMITX_BAR_DATA_NO_PRESENT; // 0
	avi->active_aspect_present = false;
	avi->color.color_format = HDMITX_COLOR_FORMAT_RGB; // 0

	avi->active_aspect_ratio = HDMITX_ACT_ASP_RATIO_ATSC_SAME_PIC; // 1000
	avi->picture_aspect_ratio = detail->pic_asp_ratio;
	avi->color.colorimetry = HDMITX_COLORIMETRY_NO_DATA; // 0

	avi->it_content_valid = false;
	avi->color.rgb_quantization = HDMITX_RGB_QUANTIZEION_DEFAULT; // 0
	avi->color.ycc_quantization = HDMITX_RGB_QUANTIZEION_DEFAULT; // 0
	avi->picture_scal = HDMITX_NUPS_UNKNOWN; // 0

	avi->it_content_type = 0;
	avi->pixel_repeat = 0;

	avi->top_bar = 0;
	avi->bottom_bar = 0;
	avi->left_bar = 0;
	avi->right_bar = 0;
}

void hdmitx_init_infoframe(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_display_mode *select_mode = NULL;
	struct dpu_video_info *info = NULL;

	if (hdmitx == NULL) {
		dpu_pr_err("NULL pointer");
		return;
	}

	select_mode = &hdmitx->select_mode;
	info = &hdmitx->soft_status.info;
	hdmitx_avi_data_init(select_mode, info);
	info->mode_3d = HDMITX_3D_NONE;
	info->allm_enable = g_hdmitx_allm_debug; // to fix
}
