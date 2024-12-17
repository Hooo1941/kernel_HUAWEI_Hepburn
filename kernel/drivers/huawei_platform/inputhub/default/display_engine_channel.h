/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: display engine channel header file
 * Author: zouzhiting <zouzhiting@huawei.com>
 * Create: 2023-05-18
 */

#ifndef __DISPLAY_ENGINE_CHANNEL_H__
#define __DISPLAY_ENGINE_CHANNEL_H__
#include "contexthub_route.h"

#include <linux/ioctl.h>

/* ioctl cmd define */
#define DEHBIO                         0xB1

#define DEHB_IOCTL_DEHB_DEBUG_CMD			_IOWR(DEHBIO, 0x10, short)
#define DEHB_IOCTL_DEHB_CONFIG_CMD			_IOWR(DEHBIO, 0x11, short)
#define DEHB_IOCTL_DEHB_OPEN_CMD			_IOWR(DEHBIO, 0x12, short)
#define DEHB_IOCTL_DEHB_CLOSE_CMD			_IOWR(DEHBIO, 0x13, short)
#define DEHB_IOCTL_DEHB_STATUS_CMD			_IOWR(DEHBIO, 0x14, short)
#define DEHB_IOCTL_DEHB_ALPHA_CONFIG_CMD	_IOWR(DEHBIO, 0x15, short)
#define DEHB_IOCTL_DEHB_PARAM_GET_CMD       _IOWR(DEHBIO, 0x16, short)

/* need synchronize with /legacy/app_external/display_engine.h */
/* -------------------- begin -------------------- */
enum de_iocfg_type {
	IOCFG_TYPE_INVALID = -1,
	IOCFG_TYPE_IMMUTABLE_PARAM = 0,
	IOCFG_TYPE_MUTABLE_PARAM = 3,
	IOCFG_TYPE_STATUS_PARAM = 4,
	IOCFG_TYPE_APLHA_PARAM = 5,
	IOCFG_TYPE_BRIGHTNESS_PARAM = 6,
	IOCFG_TYPE_DC_PARAM = 7,
	IOCFG_TYPE_COMPENSATION_PARAM = 8,
	IOCFG_TYPE_MAX,
};
/* -------------------- end -------------------- */

struct display_engine_common_data {
	uint32_t size;
	uint32_t cmd_type;
	uint32_t data[0];
};

typedef struct {
	struct pkt_header hd;
	uint32_t value[0];
} pkt_display_engine_report_t;
#endif
