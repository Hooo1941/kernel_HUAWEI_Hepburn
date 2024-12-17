/*
 * drv_venc_intf_check.c
 *
 * This is for venc drv param check.
 *
 * Copyright (c) 2023-2023 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include "drv_venc_intf_check.h"
#include "venc_regulator.h"
#include "hal_capability.h"

static int32_t venc_check_encode_channel_info(struct channel_info *channel)
{
	if (channel->id >= MAX_INSTANCES_COUNT) {
		VCODEC_FATAL_VENC("channel id invalid %u", channel->id);
		return VCODEC_FAILURE;
	}

	if (channel->is_reset_rc != 0 && channel->is_reset_rc != 1) {
		VCODEC_FATAL_VENC("param error: is_reset_rc %u", channel->is_reset_rc);
		return VCODEC_FAILURE;
	}

	return 0;
}

static int32_t venc_check_encode_reg_info(S_HEVC_AVC_REGS_TYPE_CFG *all_reg)
{
	uint32_t width = all_reg->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgwidth_pix + 1;
	uint32_t height = all_reg->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgheight_pix + 1;
	uint32_t protocol = all_reg->VEDU_VCPI_MODE.bits.vcpi_protocol;

	if (width > VCODEC_VENC_MAX_WIDTH || height > VCODEC_VENC_MAX_HEIGHT) {
		VCODEC_FATAL_VENC("size is error, width %u, height %u", width, height);
		return VCODEC_FAILURE;
	}

	if (protocol != VEDU_H264 && protocol != VEDU_H265) {
		VCODEC_FATAL_VENC("protocol is error, protocol %u", protocol);
		return VCODEC_FAILURE;
	}

	return 0;
}

int32_t venc_check_encode_info(struct encode_info *encode_info)
{
	int32_t ret;

	if (!encode_info) {
		VCODEC_FATAL_VENC("encode info is null");
		return VCODEC_FAILURE;
	}

	if (encode_info->is_protected == 1) {
		VCODEC_FATAL_VENC("not support secure encode");
		return VCODEC_FAILURE;
	}
	ret = venc_check_encode_channel_info(&encode_info->channel);
	if (ret)
		return VCODEC_FAILURE;

	ret = venc_check_encode_reg_info(&encode_info->all_reg);
	if (ret)
		return VCODEC_FAILURE;
#ifndef VCODECV700
	if (encode_info->clock_info.core_num <= 0 ||
		encode_info->clock_info.core_num > venc_get_core_num() ||
		encode_info->clock_info.clock_type >= VENC_CLK_BUTT) {
		VCODEC_FATAL_VENC("check clock info param is invalid");
		return VCODEC_FAILURE;
	}
#endif
	return 0;
}

