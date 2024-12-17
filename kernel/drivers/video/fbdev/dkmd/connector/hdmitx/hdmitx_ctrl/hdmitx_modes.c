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

#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>
#include "dkmd_log.h"

#include "hdmitx_connector.h"
#include "hdmitx_common.h"
#include "hdmitx_ctrl.h"
#include "hdmitx_vrr.h"
#include "hdmitx_modes.h"

/*
 * vic		  |color_format   | color_depth
 * min_frl_rate |frl_uncompress |frl_compress |min_dsc_frl_rate |dsc_bpp_target |dsc_hcactive |dsc_hcblank
 */
static struct frl_requirements g_frl_req_table[] = {
	/* 1 */
	{{ VIC_640X480P60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_640X480P60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_640X480P60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_640X480P60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 2 */
	{{ VIC_720X480P60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 3 */
	{{ VIC_720X480P60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 4 */
	{{ VIC_1280X720P60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 5 */
	{{ VIC_1920X1080I60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 6 */
	{{ VIC_1440X480I60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 7 */
	{{ VIC_1440X480I60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 8 */
	{{ VIC_1440X240P60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X240P60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X240P60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X240P60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 9 */
	{{ VIC_1440X240P60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X240P60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X240P60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X240P60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 10 */
	{{ VIC_2880X480I60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480I60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480I60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480I60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 11 */
	{{ VIC_2880X480I60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480I60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480I60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480I60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 12 */
	{{ VIC_2880X240P60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X240P60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X240P60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X240P60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 13 */
	{{ VIC_2880X240P60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X240P60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X240P60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X240P60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 14 */
	{{ VIC_1440X480P60_4_3, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480P60_4_3, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480P60_4_3, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480P60_4_3, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 15 */
	{{ VIC_1440X480P60_16_9, YCBCR444, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480P60_16_9, YCBCR444, CD_30 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480P60_16_9, YCBCR444, CD_36 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480P60_16_9, YCBCR422, CD_24 }, FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 16 */
	{{ VIC_1920X1080P60_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P60_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P60_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P60_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 17 */
	{{ VIC_720X576P50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 18 */
	{{ VIC_720X576P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 19 */
	{{ VIC_1280X720P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 20 */
	{{ VIC_1920X1080I50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 21 */
	{{ VIC_1440X576I50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 22 */
	{{ VIC_1440X576I50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 23 */
	{{ VIC_1440X288P50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X288P50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X288P50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X288P50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 24 */
	{{ VIC_1440X288P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X288P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X288P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X288P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 25 */
	{{ VIC_2880X576I50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576I50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576I50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576I50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 26 */
	{{ VIC_2880X576I50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576I50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576I50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576I50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 27 */
	{{ VIC_2880X288P50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X288P50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X288P50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X288P50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 28 */
	{{ VIC_2880X288P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X288P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X288P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X288P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 29 */
	{{ VIC_1440X576P50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576P50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576P50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576P50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 30 */
	{{ VIC_1440X576P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 31 */
	{{ VIC_1920X1080P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 32 */
	{{ VIC_1920X1080P24_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P24_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P24_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P24_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 33 */
	{{ VIC_1920X1080P25_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P25_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P25_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P25_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 34 */
	{{ VIC_1920X1080P30_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P30_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P30_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P30_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 35 */
	{{ VIC_2880X480P60_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480P60_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480P60_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480P60_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 36 */
	{{ VIC_2880X480P60_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480P60_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480P60_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X480P60_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 37 */
	{{ VIC_2880X576P50_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576P50_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576P50_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576P50_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 38 */
	{{ VIC_2880X576P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2880X576P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 39 */
	{{ VIC_1920X1080I50_1250_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I50_1250_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I50_1250_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I50_1250_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 40 */
	{{ VIC_1920X1080I100_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I100_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I100_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I100_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 41 */
	{{ VIC_1280X720P100_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P100_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P100_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P100_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 42 */
	{{ VIC_720X576P100_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P100_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P100_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P100_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 43 */
	{{ VIC_720X576P100_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P100_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P100_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P100_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 44 */
	{{ VIC_1440X576I100_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I100_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I100_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I100_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 45 */
	{{ VIC_1440X576I100_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I100_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I100_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I100_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 47 */
	{{ VIC_1920X1080I120_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I120_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I120_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080I120_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 47 */
	{{ VIC_1280X720P120_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P120_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P120_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P120_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 48 */
	{{ VIC_720X480P120_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P120_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P120_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P120_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 49 */
	{{ VIC_720X480P120_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P120_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P120_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P120_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 50 */
	{{ VIC_1440X480I120_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I120_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I120_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I120_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 51 */
	{{ VIC_1440X480I120_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I120_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I120_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I120_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 52 */
	{{ VIC_720X576P200_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P200_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P200_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P200_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 53 */
	{{ VIC_720X576P200_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P200_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P200_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X576P200_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 54 */
	{{ VIC_1440X576I200_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I200_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I200_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I200_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 55 */
	{{ VIC_1440X576I200_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I200_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I200_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X576I200_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 56 */
	{{ VIC_720X480P240_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P240_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P240_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P240_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 57 */
	{{ VIC_720X480P240_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P240_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P240_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_720X480P240_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 58 */
	{{ VIC_1440X480I240_4_3, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I240_4_3, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I240_4_3, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I240_4_3, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 59 */
	{{ VIC_1440X480I240_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I240_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I240_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1440X480I240_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 60 */
	{{ VIC_1280X720P24_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P24_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P24_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P24_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 61 */
	{{ VIC_1280X720P25_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P25_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P25_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P25_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 62 */
	{{ VIC_1280X720P30_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P30_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P30_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P30_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 63 */
	{{ VIC_1920X1080P120_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P120_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P120_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P120_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 64 */
	{{ VIC_1920X1080P100_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P100_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P100_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P100_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 65 */
	{{ VIC_1280X720P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 66 */
	{{ VIC_1280X720P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 67 */
	{{ VIC_1280X720P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 68 */
	{{ VIC_1280X720P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 69 */
	{{ VIC_1280X720P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 70 */
	{{ VIC_1280X720P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 71 */
	{{ VIC_1280X720P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 72 */
	{{ VIC_1920X1080P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 73 */
	{{ VIC_1920X1080P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 74 */
	{{ VIC_1920X1080P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 75 */
	{{ VIC_1920X1080P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 76 */
	{{ VIC_1920X1080P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 77 */
	{{ VIC_1920X1080P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 78 */
	{{ VIC_1920X1080P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 79 */
	{{ VIC_1680X720P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 80 */
	{{ VIC_1680X720P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 81 */
	{{ VIC_1680X720P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 82 */
	{{ VIC_1680X720P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 83 */
	{{ VIC_1680X720P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 84 */
	{{ VIC_1680X720P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 85 */
	{{ VIC_1680X720P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 86 */
	{{ VIC_2560X1080P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 87 */
	{{ VIC_2560X1080P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 88 */
	{{ VIC_2560X1080P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 89 */
	{{ VIC_2560X1080P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 90 */
	{{ VIC_2560X1080P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 91 */
	{{ VIC_2560X1080P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 92 */
	{{ VIC_2560X1080P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 93 */
	{{ VIC_3840X2160P24_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P24_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P24_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P24_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 94 */
	{{ VIC_3840X2160P25_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P25_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P25_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P25_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 95 */
	{{ VIC_3840X2160P30_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P30_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P30_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P30_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 96 */
	{{ VIC_3840X2160P50_16_9, YCBCR444, CD_24 }, FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	{{ VIC_3840X2160P50_16_9, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	{{ VIC_3840X2160P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P50_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	{{ VIC_3840X2160P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P50_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	/* 97 */
	{{ VIC_3840X2160P60_16_9, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true ,FRL_RATE_3G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P60_16_9, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	{{ VIC_3840X2160P60_16_9, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	{{ VIC_3840X2160P60_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P60_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	{{ VIC_3840X2160P60_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P60_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	/* 98 */
	{{ VIC_4096X2160P24_256_135, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P24_256_135, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P24_256_135, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P24_256_135, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 99 */
	{{ VIC_4096X2160P25_256_135, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P25_256_135, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P25_256_135, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P25_256_135, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 100 */
	{{ VIC_4096X2160P30_256_135, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P30_256_135, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P30_256_135, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_4096X2160P30_256_135, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 101 */
	{{ VIC_4096X2160P50_256_135, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 2048, 592 },
	{{ VIC_4096X2160P50_256_135, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 592 },
	{{ VIC_4096X2160P50_256_135, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 2048, 592 },
	{{ VIC_4096X2160P50_256_135, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2048, 592 },
	{{ VIC_4096X2160P50_256_135, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 592 },
	{{ VIC_4096X2160P50_256_135, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2048, 592 },
	{{ VIC_4096X2160P50_256_135, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 592 },
	/* 102 */
	{{ VIC_4096X2160P60_256_135, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 2048, 152 },
	{{ VIC_4096X2160P60_256_135, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 152 },
	{{ VIC_4096X2160P60_256_135, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 2048, 152 },
	{{ VIC_4096X2160P60_256_135, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2048, 152 },
	{{ VIC_4096X2160P60_256_135, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 152 },
	{{ VIC_4096X2160P60_256_135, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2048, 152 },
	{{ VIC_4096X2160P60_256_135, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 152 },
	/* 103 */
	{{ VIC_3840X2160P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 104 */
	{{ VIC_3840X2160P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 105 */
	{{ VIC_3840X2160P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_3840X2160P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 106 */
	{{ VIC_3840X2160P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	{{ VIC_3840X2160P50_64_27, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	{{ VIC_3840X2160P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P50_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	{{ VIC_3840X2160P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P50_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 720 },
	/* 107 */
	{{ VIC_3840X2160P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	{{ VIC_3840X2160P60_64_27, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	{{ VIC_3840X2160P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P60_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	{{ VIC_3840X2160P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P60_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 280 },
	/* 108 */
	{{ VIC_1280X720P48_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P48_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P48_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P48_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 109 */
	{{ VIC_1280X720P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1280X720P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 110 */
	{{ VIC_1680X720P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1680X720P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 111 */
	{{ VIC_1920X1080P48_16_9, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P48_16_9, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P48_16_9, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P48_16_9, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 112 */
	{{ VIC_1920X1080P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_1920X1080P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 113 */
	{{ VIC_2560X1080P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_2560X1080P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	/* 114 */
	{{ VIC_3840X2160P48_16_9, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 1920, 828 },
	{{ VIC_3840X2160P48_16_9, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	{{ VIC_3840X2160P48_16_9, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	{{ VIC_3840X2160P48_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 828 },
	{{ VIC_3840X2160P48_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	{{ VIC_3840X2160P48_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 828 },
	{{ VIC_3840X2160P48_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	/* 115 */
	{{ VIC_4096X2160P48_256_135, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 2048, 700 },
	{{ VIC_4096X2160P48_256_135, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 700 },
	{{ VIC_4096X2160P48_256_135, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 2048, 700 },
	{{ VIC_4096X2160P48_256_135, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2048, 700 },
	{{ VIC_4096X2160P48_256_135, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 700 },
	{{ VIC_4096X2160P48_256_135, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2048, 700 },
	{{ VIC_4096X2160P48_256_135, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2048, 700 },
	/* 116 */
	{{ VIC_3840X2160P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 192, 1920, 828 },
	{{ VIC_3840X2160P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	{{ VIC_3840X2160P48_64_27, YCBCR420, CD_24 } ,FRL_RATE_3G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	{{ VIC_3840X2160P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 828 },
	{{ VIC_3840X2160P48_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	{{ VIC_3840X2160P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 1920, 828 },
	{{ VIC_3840X2160P48_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 1920, 828 },
	/* 117 */
	{{ VIC_3840X2160P100_16_9, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P100_16_9, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	{{ VIC_3840X2160P100_16_9, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	{{ VIC_3840X2160P100_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P100_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	{{ VIC_3840X2160P100_16_9, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P100_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	/* 118 */
	{{ VIC_3840X2160P120_16_9, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P120_16_9, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	{{ VIC_3840X2160P120_16_9, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	{{ VIC_3840X2160P120_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P120_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	{{ VIC_3840X2160P120_16_9, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P120_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	/* 119 */
	{{ VIC_3840X2160P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	{{ VIC_3840X2160P100_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	{{ VIC_3840X2160P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P100_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	{{ VIC_3840X2160P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 1920, 720 },
	{{ VIC_3840X2160P100_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 274, 1370, 104 },
	/* 120 */
	{{ VIC_3840X2160P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	{{ VIC_3840X2160P120_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	{{ VIC_3840X2160P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P120_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	{{ VIC_3840X2160P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 1920, 280 },
	{{ VIC_3840X2160P120_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 226, 1130, 104 },
	/* 121 */
	{{ VIC_5120X2160P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 122 */
	{{ VIC_5120X2160P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 123 */
	{{ VIC_5120X2160P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_5120X2160P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	/* 124 */
	{{ VIC_5120X2160P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2560, 256 },
	{{ VIC_5120X2160P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 384, 2560, 256 },
	{{ VIC_5120X2160P48_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2560, 256 },
	{{ VIC_5120X2160P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 192, 2560, 256 },
	{{ VIC_5120X2160P48_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2560, 256 },
	{{ VIC_5120X2160P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 192, 2560, 256 },
	{{ VIC_5120X2160P48_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2560, 256 },
	/* 125 */
	{{ VIC_5120X2160P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 192, 2560, 484 },
	{{ VIC_5120X2160P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 384, 2560, 484 },
	{{ VIC_5120X2160P50_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2560, 484 },
	{{ VIC_5120X2160P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 192, 2560, 484 },
	{{ VIC_5120X2160P50_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2560, 484 },
	{{ VIC_5120X2160P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 192, 2560, 484 },
	{{ VIC_5120X2160P50_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 384, 2560, 484 },
	/* 126 */
	{{ VIC_5120X2160P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 173, 2307, 144 },
	{{ VIC_5120X2160P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 346, 2307, 144 },
	{{ VIC_5120X2160P60_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 346, 2307, 144 },
	{{ VIC_5120X2160P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 173, 2307, 144 },
	{{ VIC_5120X2160P60_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 346, 2307, 144 },
	{{ VIC_5120X2160P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 173, 2307, 144 },
	{{ VIC_5120X2160P60_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 346, 2307, 144 },
	/* 127 */
	{{ VIC_5120X2160P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 2560, 484 },
	{{ VIC_5120X2160P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 384, 2560, 484 },
	{{ VIC_5120X2160P100_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_6G3L, 384, 2560, 484 },
	{{ VIC_5120X2160P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 192, 2560, 484 },
	{{ VIC_5120X2160P100_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 384, 2560, 484 },
	{{ VIC_5120X2160P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 192, 2560, 484 },
	{{ VIC_5120X2160P100_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 384, 2560, 484 },
	/* 193 */
	{{ VIC_5120X2160P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 175, 2334, 104 },
	{{ VIC_5120X2160P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 350, 2334, 104 },
	{{ VIC_5120X2160P120_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_6G3L, 350, 2334, 104 },
	{{ VIC_5120X2160P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 175, 2334, 104 },
	{{ VIC_5120X2160P120_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 350, 2334, 104 },
	{{ VIC_5120X2160P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 175, 2334, 104 },
	{{ VIC_5120X2160P120_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 350, 2334, 104 },
	/* 194 */
	{{ VIC_7680X4320P24_16_9, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1660 },
	{{ VIC_7680X4320P24_16_9, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P24_16_9, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P24_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1660 },
	{{ VIC_7680X4320P24_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P24_16_9, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1660 },
	{{ VIC_7680X4320P24_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	/* 195 */
	{{ VIC_7680X4320P25_16_9, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1560 },
	{{ VIC_7680X4320P25_16_9, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P25_16_9, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P25_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1560 },
	{{ VIC_7680X4320P25_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P25_16_9, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1560 },
	{{ VIC_7680X4320P25_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	/* 196 */
	{{ VIC_7680X4320P30_16_9, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 3840, 660 },
	{{ VIC_7680X4320P30_16_9, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	{{ VIC_7680X4320P30_16_9, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	{{ VIC_7680X4320P30_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3840, 660 },
	{{ VIC_7680X4320P30_16_9, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	{{ VIC_7680X4320P30_16_9, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 3840, 660 },
	{{ VIC_7680X4320P30_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	/* 197 */
	{{ VIC_7680X4320P48_16_9, YCBCR444, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1292 },
	{{ VIC_7680X4320P48_16_9, YCBCR422, CD_24 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P48_16_9, YCBCR420, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P48_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1292 },
	{{ VIC_7680X4320P48_16_9, YCBCR420, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P48_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1292 },
	{{ VIC_7680X4320P48_16_9, YCBCR420, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 246, 2460, 816 },
	/* 198 */
	{{ VIC_7680X4320P50_16_9, YCBCR444, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1180 },
	{{ VIC_7680X4320P50_16_9, YCBCR422, CD_24 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P50_16_9, YCBCR420, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P50_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1180 },
	{{ VIC_7680X4320P50_16_9, YCBCR420, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P50_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1180 },
	{{ VIC_7680X4320P50_16_9, YCBCR420, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 246, 2460, 732 },
	/* 199 */
	{{ VIC_7680X4320P60_16_9, YCBCR444, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 159, 3182, 140 },
	{{ VIC_7680X4320P60_16_9, YCBCR422, CD_24 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 238, 2380, 116 },
	{{ VIC_7680X4320P60_16_9, YCBCR420, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 238, 2380, 116 },
	{{ VIC_7680X4320P60_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 159, 3182, 140 },
	{{ VIC_7680X4320P60_16_9, YCBCR420, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 238, 2380, 116 },
	{{ VIC_7680X4320P60_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G4L, false, true ,FRL_RATE_6G4L, 159, 3182, 140 },
	{{ VIC_7680X4320P60_16_9, YCBCR420, CD_36 } ,FRL_RATE_12G4L, true, true ,FRL_RATE_6G3L, 238, 2380, 116 },
	/* 200 */
	{{ VIC_7680X4320P100_16_9, YCBCR444, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 134, 2680, 784 },
	{{ VIC_7680X4320P100_16_9, YCBCR422, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 268, 2680, 784 },
	{{ VIC_7680X4320P100_16_9, YCBCR420, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 224, 2240, 480 },
	{{ VIC_7680X4320P100_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 134, 2680, 784 },
	{{ VIC_7680X4320P100_16_9, YCBCR420, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 224, 2240, 480 },
	{{ VIC_7680X4320P100_16_9, YCBCR444, CD_36 } ,FRL_RATE_10G4L, false, true ,FRL_RATE_10G4L, 134, 2680, 784 },
	{{ VIC_7680X4320P100_16_9, YCBCR420, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 224, 2240, 480 },
	/* 201 */
	{{ VIC_7680X4320P120_16_9, YCBCR444, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 130, 2600, 100 },
	{{ VIC_7680X4320P120_16_9, YCBCR422, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 260, 2600, 100 },
	{{ VIC_7680X4320P120_16_9, YCBCR420, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 206, 2062, 108 },
	{{ VIC_7680X4320P120_16_9, YCBCR444, CD_30 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 130, 2600, 100 },
	{{ VIC_7680X4320P120_16_9, YCBCR420, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 206, 2062, 108 },
	{{ VIC_7680X4320P120_16_9, YCBCR444, CD_36 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 130, 2600, 100 },
	{{ VIC_7680X4320P120_16_9, YCBCR420, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 206, 2062, 108 },
	/* 202 */
	{{ VIC_7680X4320P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1660 },
	{{ VIC_7680X4320P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P24_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1660 },
	{{ VIC_7680X4320P24_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1660 },
	{{ VIC_7680X4320P24_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 816 },
	/* 203 */
	{{ VIC_7680X4320P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1560 },
	{{ VIC_7680X4320P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P25_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1560 },
	{{ VIC_7680X4320P25_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 3840, 1560 },
	{{ VIC_7680X4320P25_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 246, 2460, 732 },
	/* 204 */
	{{ VIC_7680X4320P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 3840, 660 },
	{{ VIC_7680X4320P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	{{ VIC_7680X4320P30_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	{{ VIC_7680X4320P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3840, 660 },
	{{ VIC_7680X4320P30_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	{{ VIC_7680X4320P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 3840, 660 },
	{{ VIC_7680X4320P30_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 236, 2360, 144 },
	/* 205 */
	{{ VIC_7680X4320P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1292 },
	{{ VIC_7680X4320P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P48_64_27, YCBCR420, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1292 },
	{{ VIC_7680X4320P48_64_27, YCBCR420, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 246, 2460, 816 },
	{{ VIC_7680X4320P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1292 },
	{{ VIC_7680X4320P48_64_27, YCBCR420, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 246, 2460, 816 },
	/* 206 */
	{{ VIC_7680X4320P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1180 },
	{{ VIC_7680X4320P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, false, true,  FRL_RATE_6G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P50_64_27, YCBCR420, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1180 },
	{{ VIC_7680X4320P50_64_27, YCBCR420, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 246, 2460, 732 },
	{{ VIC_7680X4320P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 157, 3142, 1180 },
	{{ VIC_7680X4320P50_64_27, YCBCR420, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 246, 2460, 732 },
	/* 207 */
	{{ VIC_7680X4320P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 159, 3182, 140 },
	{{ VIC_7680X4320P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 238, 2380, 116 },
	{{ VIC_7680X4320P60_64_27, YCBCR420, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 238, 2380, 116 },
	{{ VIC_7680X4320P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 159, 3182, 140 },
	{{ VIC_7680X4320P60_64_27, YCBCR420, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 238, 2380, 116 },
	{{ VIC_7680X4320P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 159, 3182, 140 },
	{{ VIC_7680X4320P60_64_27, YCBCR420, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 238, 2380, 116 },
	/* 208 */
	{{ VIC_7680X4320P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 134, 2680, 784 },
	{{ VIC_7680X4320P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 268, 2680, 784 },
	{{ VIC_7680X4320P100_64_27, YCBCR420, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 224, 2240, 480 },
	{{ VIC_7680X4320P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 134, 2680, 784 },
	{{ VIC_7680X4320P100_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 224, 2240, 480 },
	{{ VIC_7680X4320P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 134, 2680, 784 },
	{{ VIC_7680X4320P100_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 224, 2240, 480 },
	/* 209 */
	{{ VIC_7680X4320P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 130, 2600, 100 },
	{{ VIC_7680X4320P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 260, 2600, 100 },
	{{ VIC_7680X4320P120_64_27, YCBCR420, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 206, 2062, 108 },
	{{ VIC_7680X4320P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 130, 2600, 100 },
	{{ VIC_7680X4320P120_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 206, 2062, 108 },
	{{ VIC_7680X4320P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 130, 2600, 100 },
	{{ VIC_7680X4320P120_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 206, 2062, 108 },
	/* 210 */
	{{ VIC_10240X4320P24_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 182, 4854, 912 },
	{{ VIC_10240X4320P24_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 364, 4854, 912 },
	{{ VIC_10240X4320P24_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 196, 2614, 172 },
	{{ VIC_10240X4320P24_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 182, 4854, 912 },
	{{ VIC_10240X4320P24_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 196, 2614, 172 },
	{{ VIC_10240X4320P24_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 182, 4854, 912 },
	{{ VIC_10240X4320P24_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 196, 2614, 172 },
	/* 211 */
	{{ VIC_10240X4320P25_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 181, 4827, 1536 },
	{{ VIC_10240X4320P25_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 362, 4827, 1536 },
	{{ VIC_10240X4320P25_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 196, 2614, 500 },
	{{ VIC_10240X4320P25_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 181, 4827, 1536 },
	{{ VIC_10240X4320P25_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 196, 2614, 500 },
	{{ VIC_10240X4320P25_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 181, 4827, 1536 },
	{{ VIC_10240X4320P25_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 196, 2614, 500 },
	/* 212 */
	{{ VIC_10240X4320P30_64_27, YCBCR444, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 177, 4720, 128 },
	{{ VIC_10240X4320P30_64_27, YCBCR422, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 354, 4720, 128 },
	{{ VIC_10240X4320P30_64_27, YCBCR420, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_6G3L, 354, 4720, 128 },
	{{ VIC_10240X4320P30_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 177, 4720, 128 },
	{{ VIC_10240X4320P30_64_27, YCBCR420, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 354, 4720, 128 },
	{{ VIC_10240X4320P30_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 177, 4720, 128 },
	{{ VIC_10240X4320P30_64_27, YCBCR420, CD_36 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 354, 4720, 128 },
	/* 213 */
	{{ VIC_10240X4320P48_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 163, 4347, 756 },
	{{ VIC_10240X4320P48_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 252, 3360, 420 },
	{{ VIC_10240X4320P48_64_27, YCBCR420, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 196, 2614, 172 },
	{{ VIC_10240X4320P48_64_27, YCBCR444, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 163, 4347, 756 },
	{{ VIC_10240X4320P48_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 196, 2614, 172 },
	{{ VIC_10240X4320P48_64_27, YCBCR444, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 163, 4347, 756 },
	{{ VIC_10240X4320P48_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 196, 2614, 172 },
	/* 214 */
	{{ VIC_10240X4320P50_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 162, 4320, 1376 },
	{{ VIC_10240X4320P50_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 250, 3334, 892 },
	{{ VIC_10240X4320P50_64_27, YCBCR420, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G4L, 250, 3334, 892 },
	{{ VIC_10240X4320P50_64_27, YCBCR444, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 162, 4320, 1376 },
	{{ VIC_10240X4320P50_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 250, 3334, 892 },
	{{ VIC_10240X4320P50_64_27, YCBCR444, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 162, 4320, 1376 },
	{{ VIC_10240X4320P50_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 250, 3334, 892 },
	/* 215 */
	{{ VIC_10240X4320P60_64_27, YCBCR444, CD_24 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 157, 4187, 124 },
	{{ VIC_10240X4320P60_64_27, YCBCR422, CD_24 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 234, 3120, 124 },
	{{ VIC_10240X4320P60_64_27, YCBCR420, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G4L, 234, 3120, 124 },
	{{ VIC_10240X4320P60_64_27, YCBCR444, CD_30 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 157, 4187, 124 },
	{{ VIC_10240X4320P60_64_27, YCBCR420, CD_30 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 234, 3120, 124 },
	{{ VIC_10240X4320P60_64_27, YCBCR444, CD_36 } ,FRL_RATE_8G4L, false, true, FRL_RATE_8G4L, 157, 4187, 124 },
	{{ VIC_10240X4320P60_64_27, YCBCR420, CD_36 } ,FRL_RATE_6G4L, false, true, FRL_RATE_6G4L, 234, 3120, 124 },
	/* 216 */
	{{ VIC_10240X4320P100_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, false, false, 0, 0, 0, 0 },
	{{ VIC_10240X4320P100_64_27, YCBCR422, CD_24 } ,FRL_RATE_12G4L, false, true, FRL_RATE_12G4L, 250, 3334, 764 },
	{{ VIC_10240X4320P100_64_27, YCBCR420, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 214, 2854, 520 },
	{{ VIC_10240X4320P100_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, false, 0, 0, 0, 0 },
	{{ VIC_10240X4320P100_64_27, YCBCR420, CD_30 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 214, 2854, 520 },
	{{ VIC_10240X4320P100_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, false, 0, 0, 0, 0 },
	{{ VIC_10240X4320P100_64_27, YCBCR420, CD_36 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 214, 2854, 520 },
	/* 217 */
	{{ VIC_10240X4320P120_64_27, YCBCR444, CD_24 } ,FRL_RATE_6G3L, false, false, 0, 0, 0, 0 },
	{{ VIC_10240X4320P120_64_27, YCBCR422, CD_24 } ,FRL_RATE_12G4L, false, true, FRL_RATE_12G4L, 234, 3120, 124 },
	{{ VIC_10240X4320P120_64_27, YCBCR420, CD_24 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 194, 2587, 120 },
	{{ VIC_10240X4320P120_64_27, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, false, 0, 0, 0, 0 },
	{{ VIC_10240X4320P120_64_27, YCBCR420, CD_30 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 194, 2587, 120 },
	{{ VIC_10240X4320P120_64_27, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, false, 0, 0, 0, 0 },
	{{ VIC_10240X4320P120_64_27, YCBCR420, CD_36 } ,FRL_RATE_10G4L, false, true, FRL_RATE_10G4L, 194, 2587, 120 },
	/* 218 */
	{{ VIC_4096X2160P100_256_135, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 2048, 592 },
	{{ VIC_4096X2160P100_256_135, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 384, 2048, 592 },
	{{ VIC_4096X2160P100_256_135, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 258, 1376, 96 },
	{{ VIC_4096X2160P100_256_135, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 2048, 592 },
	{{ VIC_4096X2160P100_256_135, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 258, 1376, 96 },
	{{ VIC_4096X2160P100_256_135, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 2048, 592 },
	{{ VIC_4096X2160P100_256_135, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 258, 1376, 96 },
	/* 219 */
	{{ VIC_4096X2160P120_256_135, YCBCR444, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 192, 2048, 152 },
	{{ VIC_4096X2160P120_256_135, YCBCR422, CD_24 } ,FRL_RATE_8G4L, true, true, FRL_RATE_6G3L, 384, 2048, 152 },
	{{ VIC_4096X2160P120_256_135, YCBCR420, CD_24 } ,FRL_RATE_6G3L, true, true, FRL_RATE_3G3L, 212, 1131, 96 },
	{{ VIC_4096X2160P120_256_135, YCBCR444, CD_30 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 2048, 152 },
	{{ VIC_4096X2160P120_256_135, YCBCR420, CD_30 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 212, 1131, 96 },
	{{ VIC_4096X2160P120_256_135, YCBCR444, CD_36 } ,FRL_RATE_12G4L, true, true, FRL_RATE_6G3L, 192, 2048, 152 },
	{{ VIC_4096X2160P120_256_135, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 212, 1131, 96 },
	/* 220 */
	{{ VIC_6016X3384P60_16_9, YCBCR444, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 192, 3008, 147 },
	{{ VIC_6016X3384P60_16_9, YCBCR422, CD_24 } ,FRL_RATE_10G4L, true, true, FRL_RATE_6G3L, 384, 3008, 147 },
	{{ VIC_6016X3384P60_16_9, YCBCR420, CD_24 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 384, 3008, 147 },
	{{ VIC_6016X3384P60_16_9, YCBCR444, CD_30 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 192, 3008, 147 },
	{{ VIC_6016X3384P60_16_9, YCBCR420, CD_30 } ,FRL_RATE_8G4L, true, true, FRL_RATE_3G3L, 384, 3008, 147 },
	{{ VIC_6016X3384P60_16_9, YCBCR444, CD_36 } ,FRL_RATE_6G3L, false, true, FRL_RATE_6G3L, 192, 3008, 147 },
	{{ VIC_6016X3384P60_16_9, YCBCR420, CD_36 } ,FRL_RATE_6G4L, true, true, FRL_RATE_3G3L, 384, 3008, 147 },
	/* VESA TIMING */
	{{ VIC_VESA_800X600P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_800X600P56, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P56, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P56, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P56, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_640X480P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_640X480P72, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P72, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P72, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P72, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_APPLE_640X480P67, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_640X480P67, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_640X480P67, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_640X480P67, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_IBM_640X480P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_640X480P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_640X480P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_640X480P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_IBM_720X400P88, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_720X400P88, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_720X400P88, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_720X400P88, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_IBM_720X400P70, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_720X400P70, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_720X400P70, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_720X400P70, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X1024P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1024X768P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1024X768P70, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P70, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P70, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P70, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1024X768P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_IBM_1024X768I43, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_1024X768I43, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_1024X768I43, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_IBM_1024X768I43, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_APPLE_832X624P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_832X624P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_832X624P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_832X624P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_800X600P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_800X600P72, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P72, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P72, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P72, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_APPLE_1152X864P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_1152X864P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_1152X864P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_APPLE_1152X864P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_640X350P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X350P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X350P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X350P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_640X400P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X400P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X400P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X400P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_720X400P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_720X400P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_720X400P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_720X400P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_640X480P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_640X480P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_640X480P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_800X600P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_800X600P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_800X600P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_848X480P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_848X480P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_848X480P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_848X480P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1024X768I43, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768I43, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768I43, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768I43, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1024X768P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1024X768P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1024X768P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1152X846P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1152X846P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1152X846P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1152X846P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X768P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X768P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X768P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X768P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X800P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X800P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X800P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X800P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X960P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X960P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X960P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X960P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X1024P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X1024P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X1024P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X1024P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1360X768P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1360X768P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1360X768P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1360X768P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1360X768P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1360X768P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1360X768P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1360X768P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1366X768P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1366X768P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1366X768P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1366X768P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1400X1050P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1400X1050P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1400X1050P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1400X1050P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1440X900P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1440X900P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1440X900P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1440X900P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X900P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X900P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X900P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X900P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X1200P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X1200P65, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P65, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P65, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P65, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X1200P70, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P70, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P70, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P70, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X1200P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X1200P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1600X1200P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1600X1200P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1680X1050P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1680X1050P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1680X1050P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1680X1050P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1792X1344P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1792X1344P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1792X1344P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1792X1344P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1856X1392P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1856X1392P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1856X1392P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1856X1392P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1080P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1080P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1080P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1080P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1200P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1200P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1200P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P85, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P85, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1200P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1440P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1440P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P75, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P75, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1440P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P120, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P120, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1440P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2048X1152P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2048X1152P60, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2048X1152P60, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2048X1152P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2560X1600P60, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P60, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P60, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P60, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2560X1600P75, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P75, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P75, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P75, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2560X1600P85, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P85, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P85, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P85, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2560X1600P120, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P120, YCBCR444, CD_30 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P120, YCBCR444, CD_36 } ,FRL_RATE_6G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P120, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2560X1440P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1440P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1440P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1440P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1680X1050P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1680X1050P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X768P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X768P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1280X800P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1280X800P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1400X1050P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1400X1050P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1440X900P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1440X900P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_1920X1200P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_1920X1200P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },

	{{ VIC_VESA_2560X1600P60RB, YCBCR444, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P60RB, YCBCR444, CD_30 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P60RB, YCBCR444, CD_36 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
	{{ VIC_VESA_2560X1600P60RB, YCBCR422, CD_24 } ,FRL_RATE_3G3L, true, false, 0, 0, 0, 0 },
};

static u32 get_table_index_by_vic(u32 vic)
{
	if (vic > 0 && vic <= VIC_5120X2160P100_64_27)
		return vic;
	else if (vic >= VIC_5120X2160P120_64_27 && vic <= VIC_6016X3384P60_16_9)
		return vic - 65; /* 65 = (192-128+1) */
	else if (vic >= VIC_VESA_800X600P60 && vic <= VIC_VESA_2560X1600P60RB)
		return vic - 102; /* 102 = (256-220+1)-(192-128 + 1) */
	else
		return 0;
}

static struct hdmitx_timing_mode *get_timing_mode_by_vic(u32 vic)
{
	u32 idx;

	idx = get_table_index_by_vic(vic);
	if (idx > 0 && (idx < hdmitx_array_size(g_timing_mode_table)))
		return &g_timing_mode_table[idx];
	else
		return NULL;
}

static struct hdmitx_display_mode *create_mode(void)
{
	struct hdmitx_display_mode *nmode = NULL;

	nmode = kmalloc(sizeof(struct hdmitx_display_mode), GFP_KERNEL);
	if (nmode == NULL) {
		dpu_pr_warn("kmalloc fail");
		return NULL;
	}

	if (memset_s(nmode, sizeof(*nmode), 0, sizeof(*nmode)) != EOK) {
		dpu_pr_warn("memset_s fail");
		kfree(nmode);
		nmode = NULL;
		return NULL;
	}

	return nmode;
}

static void destroy_mode(struct hdmitx_display_mode *mode)
{
	if (mode != NULL)
		kfree(mode);

	mode = NULL;
}

static void timing_mode2display_mode(const struct hdmitx_timing_mode *timing_mode,
	struct hdmitx_display_mode *d_mode)
{
	struct hdmi_detail *detail = &d_mode->detail;

	d_mode->vic = timing_mode->vic;
	detail->vic = timing_mode->vic;
	detail->pixel_clock = timing_mode->pixel_clock;
	detail->progressive = timing_mode->progressive;

	detail->h_active = timing_mode->h_active;
	detail->h_back = timing_mode->h_back;
	detail->h_blank = timing_mode->h_blank;
	detail->h_front = timing_mode->h_front;
	detail->h_pol = timing_mode->h_pol;
	detail->h_sync = timing_mode->h_sync;
	detail->h_total = timing_mode->h_total;

	detail->v_active = timing_mode->v_active;
	detail->v_back = timing_mode->v_back;
	detail->v_blank = timing_mode->v_blank;
	detail->v_front = timing_mode->v_front;
	detail->v_pol = timing_mode->v_pol;
	detail->v_sync = timing_mode->v_sync;
	detail->v_total = timing_mode->v_total;

	detail->pic_asp_ratio = timing_mode->pic_aspect;

	detail->field_rate = timing_mode->field_rate;

	if (strncpy_s(d_mode->name, sizeof(d_mode->name), timing_mode->name, sizeof(d_mode->name) - 1) != EOK)
		dpu_pr_warn("strncpy_s failed!");
}

struct hdmitx_timing_mode *hdmitx_modes_get_timing_mode_by_vic(u32 vic)
{
	return get_timing_mode_by_vic(vic);
}

struct frl_requirements *hdmitx_modes_get_frl_req_by_band(const struct band_mode *in)
{
	u32 i;
	struct band_mode band;

	if (in == NULL) {
		dpu_pr_warn("ptr is null");
		return NULL;
	}

	band.vic = in->vic;
	band.color_depth = in->color_depth;
	band.color_format = (in->color_format == RGB444) ? YCBCR444 : in->color_format;

	if (get_table_index_by_vic(band.vic) == 0) {
		dpu_pr_debug("index is zero");
		return NULL;
	}

	for (i = 0; i < hdmitx_array_size(g_frl_req_table); i++) {
		if (band.vic == g_frl_req_table[i].band_mode.vic &&
			band.color_format == g_frl_req_table[i].band_mode.color_format &&
			band.color_depth == g_frl_req_table[i].band_mode.color_depth)
			return &g_frl_req_table[i];
	}

	return NULL;
}

struct hdmitx_display_mode *hdmitx_modes_create_mode(void)
{
	return create_mode();
}

struct hdmitx_display_mode *hdmitx_modes_create_mode_by_vic(u32 vic)
{
	struct hdmitx_display_mode *nmode = NULL;
	struct hdmitx_timing_mode *timing_mode = NULL;

	timing_mode = get_timing_mode_by_vic(vic);
	if (timing_mode == NULL) {
		dpu_pr_debug("timing_mode is null");
		return NULL;
	}

	nmode = create_mode();
	if (nmode == NULL) {
		dpu_pr_warn("nmode for vic is null");
		return NULL;
	}

	timing_mode2display_mode(timing_mode, nmode);

	return nmode;
}

struct hdmitx_display_mode *hdmitx_modes_create_mode_by_detail_timing(const struct hdmi_detail *detail)
{
	u32 i;
	struct hdmitx_display_mode *nmode = NULL;
	struct hdmitx_timing_mode *timing_mode = NULL;

	if (detail == NULL) {
		dpu_pr_warn("ptr is null");
		return NULL;
	}

	for (i = 0; i < hdmitx_array_size(g_timing_mode_table); i++) {
		timing_mode = &g_timing_mode_table[i];
		if (timing_mode->h_active == detail->h_active &&
			timing_mode->v_active == detail->v_active &&
			timing_mode->h_blank == detail->h_blank &&
			timing_mode->v_blank == detail->v_blank &&
			timing_mode->progressive == detail->progressive &&
			/* hdmitx_div_round_closest(pixel_clk * 1001, 1000) use to due with the error of frational refresh rate */
			timing_mode->pixel_clock <= hdmitx_div_round_closest(detail->pixel_clock * 1001, 1000) &&
			/* hdmitx_div_round_closest(pixel_clk * 1000, 1001) use to due with the error of frational refresh rate */
			timing_mode->pixel_clock >= hdmitx_div_round_closest(detail->pixel_clock * 1000, 1001)) {
			break;
		}
	}

	if (timing_mode == NULL || i >= hdmitx_array_size(g_timing_mode_table)) {
		dpu_pr_info("not find timing mode in the table, h_active = %u, v_active = %u, freq = %u",
					detail->h_active, detail->v_active, detail->field_rate);
		return NULL;
	}

	nmode = create_mode();
	if (nmode == NULL) {
		dpu_pr_warn("nmode for detail is null");
		return NULL;
	}

	timing_mode2display_mode(timing_mode, nmode);

	return nmode;
}

struct hdmitx_display_mode *hdmitx_modes_create_mode_by_std_timing(u32 hsize,
	u32 vsize, u32 refresh)
{
	u32 i;

	struct hdmitx_display_mode *nmode = NULL;
	struct hdmitx_timing_mode *timing_mode = NULL;

	for (i = VIC_4096X2160P120_256_135; i < hdmitx_array_size(g_timing_mode_table); i++) {
		timing_mode = &g_timing_mode_table[i];
		if (timing_mode->h_active == hsize && timing_mode->v_active == vsize &&
			timing_mode->field_rate == refresh) {
			break;
		}
	}

	if (timing_mode == NULL || i >= hdmitx_array_size(g_timing_mode_table)) {
		dpu_pr_info("std not find timing mode in the table h_active = %u, v_active = %u, freq = %u",
					hsize, vsize, refresh);
		return NULL;
	}

	nmode = create_mode();
	if (nmode == NULL) {
		dpu_pr_warn("nmode for std is null");
		return NULL;
	}

	timing_mode2display_mode(timing_mode, nmode);

	return nmode;
}

void hdmitx_modes_destroy_mode(struct hdmitx_display_mode *mode)
{
	if (mode == NULL) {
		dpu_pr_warn("ptr is null");
		return;
	}
	destroy_mode(mode);
}

void hdmitx_modes_add_probed_mode(struct hdmitx_connector *connector,
	struct hdmitx_display_mode *mode)
{
	struct list_head *n = NULL;
	struct list_head *p = NULL;
	struct hdmitx_display_mode *display_mode = NULL;

	if (connector == NULL || mode == NULL) {
		dpu_pr_warn("Input params is null pointer!");
		return;
	}

	if (list_empty(&connector->probed_modes))
		goto end;

	/* if this mode is the same mode, merge into one mode. */
	list_for_each_safe(p, n, &connector->probed_modes) {
		display_mode = list_entry(p, struct hdmitx_display_mode, head);
		/* Filter the same mode. */
		if (display_mode->vic == mode->vic) {
			display_mode->parse_type |= mode->parse_type;
			if (mode->native_mode)
				display_mode->native_mode = mode->native_mode;

			if (mode->first_mode)
				display_mode->first_mode = mode->first_mode;

			display_mode->detail.mode_3d |= mode->detail.mode_3d;
			destroy_mode(mode);
			return;
		}
	}
end:
	list_add_tail(&mode->head, &connector->probed_modes);
	connector->probed_mode_cnt++;
	dpu_pr_debug("probed_modes count:%d, mode vic:%u", connector->probed_mode_cnt, mode->vic);
}

u32 hdmitx_modes_get_vic_by_detail_info(const struct hdmitx_connector *connector,
	const struct hdmi_detail *detail)
{
	u32 vic = 0;
	struct list_head *n = NULL;
	struct list_head *p = NULL;
	struct hdmitx_display_mode *display_mode = NULL;

	if (connector == NULL || detail == NULL) {
		dpu_pr_warn("Input params is null pointer!");
		return vic;
	}

	if (list_empty(&connector->probed_modes)) {
		dpu_pr_debug("this list is empty!");
		return vic;
	}

	list_for_each_safe(p, n, &connector->probed_modes) {
		display_mode = list_entry(p, struct hdmitx_display_mode, head);
		if (display_mode->detail.h_active == detail->h_active &&
			display_mode->detail.v_active == detail->v_active &&
			display_mode->detail.field_rate == detail->field_rate &&
			display_mode->detail.pixel_clock == detail->pixel_clock) {
			vic = display_mode->vic;
			return vic;
		}
	}

	return vic;
}

u32 hdmitx_modes_get_vrefresh(const struct hdmi_detail *detail)
{
	u32 refresh = 0;
	u32 num, den;
	u64 temp;

	if (detail == NULL) {
		dpu_pr_warn("ptr is null");
		return refresh;
	}

	if (detail->field_rate > 0) {
		refresh = detail->field_rate;
	} else if (detail->h_total > 0 && detail->v_total > 0) {
		temp = detail->pixel_clock * 1000; /* 1kHz is 1000Hz */
		num = (u32)temp;
		temp = detail->h_total * detail->v_total;
		den = (u32)temp;

		if (!detail->progressive)
			num = num << 1; /* multiplied by 2 when interlace .is << 1 */

		refresh = hdmitx_div_round_closest(num, den);
	}

	return refresh;
}

u32 hdmitx_modes_get_tmds_clk(u32 pix_clk, u32 color_depth, u32 color_format)
{
	u32 tmds_clock = pix_clk;

	switch (color_depth) {
	case CD_24:
		break;
	case CD_30:
		tmds_clock = tmds_clock * 5 / 4; /* 5/4 means 1.25 times */
		break;
	case CD_36:
		tmds_clock = tmds_clock * 3 / 2; /* 3/2 means 1.5 times */
		break;
	case CD_48:
		tmds_clock = tmds_clock << 1;    /* 2 times is << 1 */
		break;
	default:
		dpu_pr_warn("err color_depth=%d", color_depth);
		break;
	}

	if (color_format == YCBCR420)
		tmds_clock >>= 1;    /* shift 1 means 0.5 times */

	return tmds_clock;
}

s32 hdmitx_get_vrr_mode_by_vic(struct hdmitx_vrr_mode *vrr_mode, u32 vic)
{
	if (vrr_mode == NULL) {
		dpu_pr_warn("ptr is null");
		return -1;
	}
	if (vic >= hdmitx_array_size(g_timing_mode_table) || vic == 0) {
		dpu_pr_err("can not get vrr mode");
		return -1;
	}

	vrr_mode->pixel_clock = g_timing_mode_table[vic].pixel_clock;
	vrr_mode->h_total = g_timing_mode_table[vic].h_total;
	vrr_mode->v_total = g_timing_mode_table[vic].v_total;
	vrr_mode->field_rate_max = g_timing_mode_table[vic].field_rate;

	return 0;
}
