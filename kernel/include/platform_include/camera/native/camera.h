/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: camera header file.
 * Create: 2016-03-15
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __HW_ALAN_MEDIA_CAMERA_H__
#define __HW_ALAN_MEDIA_CAMERA_H__

#include <linux/videodev2.h>
#include <linux/compat.h>
#include <securec.h>

#define CAM_MODEL_USER     "cam_userdev"
#define CAM_MODEL_CFG      "cam_cfgdev"
#define HWIGSCAM_MODEL_CFG "hwswingcam_cfgdev"

#define CAM_MICROSECOND_PER_MILISECOND 1000
#define CAM_MICROSECOND_PER_SECOND 1000000L
#define CAM_NANOSECOND_PER_MICROSECOND 1000
#define CAM_NANOSECOND_PER_MILISECOND 1000000L

typedef enum _tag_cam_device_id_constants {
	CAM_DEVICE_GROUP_ID      = 0x10,

	CAM_VNODE_GROUP_ID       = 0x8000,

	CAM_SUBDEV_SENSOR        = 0x8010,
	CAM_SUBDEV_EXTISP        = 0x8020,
	CAM_SUBDEV_IDI           = 0x8030,
	CAM_SUBDEV_PMIC          = 0x8040,
	CAM_SUBDEV_FLASH0        = 0x8050,
	CAM_SUBDEV_FLASH1        = 0x8060,
	CAM_SUBDEV_FLASH2        = 0x8070,
	CAM_SUBDEV_ISP           = 0x8080,
	CAM_SUBDEV_VCM0          = 0x8090,
	CAM_SUBDEV_VCM1          = 0x80a0,
	CAM_SUBDEV_OIS           = 0X80b0,
	CAM_SUBDEV_HISP          = 0X80c0,
	CAM_SUBDEV_LASER         = 0x80d0,
	CAM_SUBDEV_DEPTHISP      = 0x80e0,
	CAM_SUBDEV_HJPEG         = 0x80f0,
	CAM_SUBDEV_GAZELED       = 0x8100,
	CAM_SUBDEV_FPGA          = 0x8110,
	CAM_SUBDEV_DRIVER_IC     = 0x8120,
	CAM_SUBDEV_DOT_PROJECTOR = 0x8130,
	CAM_SUBDEV_IGS         = 0x8140,
	CAM_SUBDEV_IDI2AXI       = 0x8150,
	CAM_SUBDEV_IDI2AXI_SENSOR = 0X8160,
	CAM_SUBDEV_SENSORMOTOR   = 0x8170,
} cam_device_id_constants_t;

/* cdphy-CSI2 info */
typedef enum _phy_id_e {
	CDPHY_A = 0,
	CDPHY_B,
	CDPHY_C,
	CDPHY_D,
	CDPHY_MAX,
} phy_id_e;

typedef enum _phy_mode_e {
	PHY_MODE_DPHY = 0,
	PHY_MODE_CPHY,
	PHY_MODE_MAX,
} phy_mode_e;

typedef enum _phy_freq_mode_e {
	PHY_AUTO_FREQ = 0,
	PHY_MANUAL_FREQ,
	PHY_FREQ_MODE_MAX,
} phy_freq_mode_e;

typedef enum _phy_work_mode_e {
	PHY_SINGLE_MODE = 0,
	PHY_DUAL_MODE_SENSORA, /* dphy use DL1&3,cphy use DL2 */
	PHY_DUAL_MODE_SENSORB, /* dphy use DL0&2,cphy use DL0&1 */
	PHY_WORK_MODE_MAX,
} phy_work_mode_e;

#define PHY_INFO_IS_MASTER_SENSOR_LEN 2
#define PHY_INFO_IS_ID_LEN 2
#define PHY_INFO_IS_MODE_LEN 2
#define PHY_INFO_IS_FREQ_MODE_LEN 2
#define PHY_INFO_IS_FREQ_LEN 2
#define PHY_INFO_IS_WORD_MODE_LEN 2

typedef struct _phy_info_t {
	unsigned int is_master_sensor[PHY_INFO_IS_MASTER_SENSOR_LEN];
	phy_id_e phy_id[PHY_INFO_IS_ID_LEN];
	phy_mode_e phy_mode[PHY_INFO_IS_MODE_LEN];
	phy_freq_mode_e phy_freq_mode[PHY_INFO_IS_FREQ_MODE_LEN];
	unsigned int phy_freq[PHY_INFO_IS_FREQ_LEN];
	phy_work_mode_e phy_work_mode[PHY_INFO_IS_WORD_MODE_LEN];
} phy_info_t;

#define CAM_V4L2_IOCTL_THERMAL_GUARD \
	_IOWR('A', BASE_VIDIOC_PRIVATE + 0x22, struct v4l2_event)
#define CAM_V4L2_IOCTL_MOTION_DATA \
	_IOWR('A', BASE_VIDIOC_PRIVATE + 0x23, struct v4l2_event)

typedef enum _tag_cam_v4l2_cid_kind {
	CAM_V4L2_CID_PIPELINE_MIN = V4L2_CID_PRIVATE_BASE,
	CAM_V4L2_CID_PIPELINE_PARAM,
	CAM_V4L2_CID_PIPELINE_MAX,

	CAM_V4L2_CID_STREAM_MIN = V4L2_CID_PRIVATE_BASE + 0x1000,
	CAM_V4L2_CID_STREAM_PARAM,
	CAM_V4L2_CID_STREAM_MAX,
} cam_v4l2_cid_kind_t;

typedef enum _tag_cam_cfgreq_constants {
	CAM_V4L2_EVENT_TYPE     = V4L2_EVENT_PRIVATE_START + 0x00001000,
	CAM_CFGDEV_REQUEST      = 0x1000,
	CAM_CFGPIPELINE_REQUEST = 0x2000,
	CAM_CFGSTREAM_REQUEST   = 0x3000,
	CAM_SERVER_CRASH        = 0x4000,
	CAM_HARDWARE_SUSPEND    = 0x5001,
	CAM_HARDWARE_RESUME     = 0x5002,
	CAM_NOTIFY_USER         = 0x6000,
} cam_cfgreq_constants_t;

typedef enum _tag_cam_cfgreq2dev_kind {
	CAM_CFGDEV_REQ_MIN = CAM_CFGDEV_REQUEST,
	CAM_CFGDEV_REQ_GUARD_THERMAL,
	CAM_CFGDEV_REQ_DUMP_MEMINFO,
	CAM_CFGDEV_REQ_FBD_NOTIFY,
	CAM_CFGDEV_REQ_GET_MOTION_DATA,
	CAM_CFGDEV_REQ_MAX,
} cam_cfgreq2dev_kind_t;

typedef struct _tag_cam_cfgreq2dev {
	cam_cfgreq2dev_kind_t kind;
	union { /* can ONLY place 10 int fields here. */
		struct {
			int fd;
			int moduleID;
		} pipeline;
	};
} cam_cfgreq2dev_t;

#endif /* __HW_ALAN_MEDIA_CAMERA_H__ */
