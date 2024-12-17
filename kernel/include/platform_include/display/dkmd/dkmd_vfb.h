/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 */
#ifndef DKMD_VFB_H
#define DKMD_VFB_H

#define VIRTIO_FB_PRESENT_LAYER_NUM 1
#define VIRTIO_FB_MAGIC_NUM 0xff

#define DISP_VIRTUAL_FB_PRESENT _IOWR(DISP_IOCTL_MAGIC, DISP_IOCTL_PRESENT, struct vfb_present_data)

#define VFB_PRESENT_LAYER_NUM 1

struct vfb_layer_info {
	int32_t share_fd;
	int32_t acquired_fence;
	uint32_t format;
	uint32_t stride;
};

struct vfb_present_data {
	uint32_t frame_no;
	uint32_t magic_num;
	uint32_t layer_nums;
	struct vfb_layer_info layer_infos[VIRTIO_FB_PRESENT_LAYER_NUM];
	int32_t out_present_fence;
};

struct vfb_product_config {
	// fix information
	uint32_t width;
	uint32_t height;
	union driver_bits drv_feature; // driver and device feature

	uint16_t display_count;
	union feature_bits feature_switch; // product feature

	uint32_t dim_info_count;
	struct rog_dim dim_info[DIMENSION_ID_MAX];

	uint32_t fps_info_count;
	uint32_t fps_info[FPS_LEVEL_MAX];

	uint32_t dsc_out_width;
	uint32_t dsc_out_height;
	bool is_ltpo;
};

#endif
