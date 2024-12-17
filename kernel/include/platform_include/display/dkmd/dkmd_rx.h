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

#ifndef DKMD_RX_H
#define DKMD_RX_H

#define DISP_INPUTSRC_IOCTL_MAGIC 'I'
#define RX_EDID_LENGTH 512
#define HDR_DISPLAY_PRIMARIES_NUM 3
#define DSC_NUM_BUF_RANGES 15

enum {
	DISP_INPUTSRC_IOCTL_WAIT_VACTIVE_START = 0x10,
	DISP_INPUTSRC_IOCTL_ENABLE_VACTIVE_START,
	DISP_INPUTSRC_IOCTL_GET_PORT_STATUS,
	DISP_INPUTSRC_IOCTL_DISCONNECT,
	DISP_INPUTSRC_IOCTL_CONNECT,
	DISP_INPUTSRC_IOCTL_GET_TIMING_INFO,
	DISP_INPUTSRC_IOCTL_SET_VRR,
	DISP_INPUTSRC_IOCTL_GET_VRR,
	DISP_INPUTSRC_IOCTL_POWER_CTRL,
	DISP_INPUTSRC_IOCTL_UPDATE_EDID,
	DISP_INPUTSRC_IOCTL_GET_EDID
};

struct disp_input_edid {
	uint32_t edid_length;
	uint8_t edid_data[RX_EDID_LENGTH]; /* EDID value */
};

enum disp_input_cable_status {
	UNKNOWN = 0,
	CONNECTED = 1,
	DISCONNECTED = 2,
};
enum disp_input_signal_status {
	UNKNOWN_STATUS = 0,
	STABLE = 1,
	WAIT_STABLE = 2,
	NO_SIGNAL = 3
};

enum disp_input_pixel_encoding {
	PIXEL_ENCODING_RGB = 0,
	PIXEL_ENCODING_YUV444 = 1,
	PIXEL_ENCODING_YUV422 = 2,
	PIXEL_ENCODING_YUV420 = 3
};

enum disp_input_colorimetry {
	COLORIMETRY_UNSPECIFY = 0,
	COLORIMETRY_BT601 = 1,
	COLORIMETRY_BT709 = 2,
	COLORIMETRY_BT2020 = 3,
	COLORIMETRY_DCI_P3 = 4,
	COLORIMETRY_ADOBE_RGB = 5,
	COLORIMETRY_SRGB = 6
};

enum disp_input_color_depth {
	COLOR_DEPTH_DEFAULT = 0,
	COLOR_DEPTH_24BIT,
	COLOR_DEPTH_30BIT,
	COLOR_DEPTH_36BIT,
	COLOR_DEPTH_48BIT,
};

enum disp_input_color_transfer_curve {
	COLOR_TRANSFER_TYPE_SDR = 0,
	COLOR_TRANSFER_TYPE_HDR = 1,
	COLOR_TRANSFER_TYPE_HLG = 2
};

enum disp_input_video_quantization_range {
	VIDEO_QUANTIZATION_RANGE_LIMITED = 0,
	VIDEO_QUANTIZATION_RANGE_FULL,
	VIDEO_QUANTIZATION_RANGE_MAX
};

enum disp_input_psr_status {
	PSR_STATUS_DISABLE = 0,
	PSR_STATUS_PSR_ENABLE,
	PSR_STATUS_PSR2_ENABLE
};


struct disp_input_port_status {
	enum disp_input_cable_status cable_status;
	enum disp_input_signal_status signal_status;
};

struct disp_input_dsc_rc_range_parameters {
	/**
	 * @range_min_qp: Min Quantization Parameters allowed for this range
	 */
	uint8_t range_min_qp;
	/**
	 * @range_max_qp: Max Quantization Parameters allowed for this range
	 */
	uint8_t range_max_qp;
	/**
	 * @range_bpg_offset:
	 * Bits/group offset to apply to target for this group
	 */
	uint8_t range_bpg_offset;
};

struct disp_input_dsc_pps {
	/**
	 * @line_buf_depth:
	 * Bits per component for previous reconstructed line buffer
	 */
	uint8_t line_buf_depth;
	/**
	 * @bits_per_component: Bits per component to code (8/10/12)
	 */
	uint8_t bits_per_component;
	/**
	 * @convert_rgb:
	 * Flag to indicate if RGB - YCoCg conversion is needed
	 * True if RGB input, False if YCoCg input
	 */
	bool convert_rgb;
	/**
	 * @slice_count: Number fo slices per line used by the DSC encoder
	 */
	uint8_t slice_count;
	/**
	 *  @slice_width: Width of each slice in pixels
	 */
	uint16_t slice_width;
	/**
	 * @slice_height: Slice height in pixels
	 */
	uint16_t slice_height;
	/**
	 * @simple_422: True if simple 4_2_2 mode is enabled else False
	 */
	bool simple_422;
	/**
	 * @pic_width: Width of the input display frame in pixels
	 */
	uint16_t pic_width;
	/**
	 * @pic_height: Vertical height of the input display frame
	 */
	uint16_t pic_height;
	/**
	 * @rc_tgt_offset_high:
	 * Offset to bits/group used by RC to determine QP adjustment
	 */
	uint8_t rc_tgt_offset_high;
	/**
	 * @rc_tgt_offset_low:
	 * Offset to bits/group used by RC to determine QP adjustment
	 */
	uint8_t rc_tgt_offset_low;
	/**
	 * @bits_per_pixel:
	 * Target bits per pixel with 4 fractional bits, bits_per_pixel << 4
	 */
	uint16_t bits_per_pixel;
	/**
	 * @rc_edge_factor:
	 * Factor to determine if an edge is present based on the bits produced
	 */
	uint8_t rc_edge_factor;
	/**
	 * @rc_quant_incr_limit1:
	 * Slow down incrementing once the range reaches this value
	 */
	uint8_t rc_quant_incr_limit1;
	/**
	 * @rc_quant_incr_limit0:
	 * Slow down incrementing once the range reaches this value
	 */
	uint8_t rc_quant_incr_limit0;
	/**
	 * @initial_xmit_delay:
	 * Number of pixels to delay the initial transmission
	 */
	uint16_t initial_xmit_delay;
	/**
	 * @initial_dec_delay:
	 * Initial decoder delay, number of pixel times that the decoder
	 * accumulates data in its rate buffer before starting to decode
	 * and output pixels.
	 */
	uint16_t  initial_dec_delay;
	/**
	 * @block_pred_enable:
	 * True if block prediction is used to code any groups within the
	 * picture. False if BP not used
	 */
	bool block_pred_enable;
	/**
	 * @first_line_bpg_offset:
	 * Number of additional bits allocated for each group on the first
	 * line of slice.
	 */
	uint8_t first_line_bpg_offset;
	/**
	 * @initial_offset: Value to use for RC model offset at slice start
	 */
	uint16_t initial_offset;
	/**
	 * @rc_buf_thresh: Thresholds defining each of the buffer ranges
	 */
	uint16_t rc_buf_thresh[DSC_NUM_BUF_RANGES - 1];
	/**
	 * @rc_range_params:
	 * Parameters for each of the RC ranges defined in
	 * &struct drm_dsc_rc_range_parameters
	 */
	struct disp_input_dsc_rc_range_parameters rc_range_params[DSC_NUM_BUF_RANGES];
	/**
	 * @rc_model_size: Total size of RC model
	 */
	uint16_t rc_model_size;
	/**
	 * @flatness_min_qp: Minimum QP where flatness information is sent
	 */
	uint8_t flatness_min_qp;
	/**
	 * @flatness_max_qp: Maximum QP where flatness information is sent
	 */
	uint8_t flatness_max_qp;
	/**
	 * @initial_scale_value: Initial value for the scale factor
	 */
	uint8_t initial_scale_value;
	/**
	 * @scale_decrement_interval:
	 * Specifies number of group times between decrementing the scale factor
	 * at beginning of a slice.
	 */
	uint16_t scale_decrement_interval;
	/**
	 * @scale_increment_interval:
	 * Number of group times between incrementing the scale factor value
	 * used at the beginning of a slice.
	 */
	uint16_t scale_increment_interval;
	/**
	 * @nfl_bpg_offset: Non first line BPG offset to be used
	 */
	uint16_t nfl_bpg_offset;
	/**
	 * @slice_bpg_offset: BPG offset used to enforce slice bit
	 */
	uint16_t slice_bpg_offset;
	/**
	 * @final_offset: Final RC linear transformation offset value
	 */
	uint16_t final_offset;
	/**
	 * @vbr_enable: True if VBR mode is enabled, false if disabled
	 */
	bool vbr_enable;
	/**
	 * @mux_word_size: Mux word size (in bits) for SSM mode
	 */
	uint8_t mux_word_size;
	/**
	 * @slice_chunk_size:
	 * The (max) size in bytes of the "chunks" that are used in slice
	 * multiplexing.
	 */
	uint16_t slice_chunk_size;
	/**
	 * @rc_bits: Rate control buffer size in bits
	 */
	uint16_t rc_bits;
	/**
	 * @dsc_version_minor: DSC minor version
	 */
	uint8_t dsc_version_minor;
	/**
	 * @dsc_version_major: DSC major version
	 */
	uint8_t dsc_version_major;
	/**
	 * @native_422: True if Native 4:2:2 supported, else false
	 */
	bool native_422;
	/**
	 * @native_420: True if Native 4:2:0 supported else false.
	 */
	bool native_420;
	/**
	 * @second_line_bpg_offset:
	 * Additional bits/grp for seconnd line of slice for native 4:2:0
	 */
	uint8_t second_line_bpg_offset;
	/**
	 * @nsl_bpg_offset:
	 * Num of bits deallocated for each grp that is not in second line of
	 * slice
	 */
	uint16_t nsl_bpg_offset;
	/**
	 * @second_line_offset_adj:
	 * Offset adjustment for second line in Native 4:2:0 mode
	 */
	uint16_t second_line_offset_adj;
};

struct disp_input_dsc_info {
	uint32_t dsc_enable;  /* disable 0; enable 1 */
	struct disp_input_dsc_pps pps;
};

struct disp_input_hdr_static_metadata {
	uint16_t display_primaries_x[HDR_DISPLAY_PRIMARIES_NUM];
	uint16_t display_primaries_y[HDR_DISPLAY_PRIMARIES_NUM];
	uint16_t white_point_x;
	uint16_t white_point_y;
	uint32_t max_display_mastering_luminance;
	uint32_t min_display_mastering_luminance;
	uint32_t max_content_light_level;
	uint32_t max_frame_average_light_level;
};

struct disp_input_timing {
	uint32_t width; /* hactive */
	uint32_t height; /* vactive */
	uint32_t frame_rate; /* frequency of vsync */
	enum disp_input_pixel_encoding pixel_encoding; /* RGB/YUV444/YUV422/YUV420 */
	enum disp_input_colorimetry colorimetry; /* BT601/BT2020 */
	enum disp_input_color_depth depth; /* 24/30bit */
	enum disp_input_color_transfer_curve transfer_type;  /* SDR/HDR/HLG */
	struct disp_input_hdr_static_metadata metadata;
	struct disp_input_dsc_info dsc;
	uint32_t vrr_enable; /* disable 0; enable 1; current vrr status */
	enum disp_input_psr_status psr_status; /* disable 0; psr enable 1; psr2 enable 2 */
	enum disp_input_video_quantization_range quantization_range;
};

#define DISP_INPUTSRC_WAIT_VACTIVE_START \
									_IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_WAIT_VACTIVE_START, int)
#define DISP_INPUTSRC_ENABLE_VACTIVE_START \
									_IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_ENABLE_VACTIVE_START, int)
#define DISP_INPUTSRC_GET_PORT_STATUS \
									_IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_GET_PORT_STATUS, struct disp_input_port_status)
#define DISP_INPUTSRC_DISCONNECT _IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_DISCONNECT, int)
#define DISP_INPUTSRC_CONNECT _IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_CONNECT, int)
#define DISP_INPUTSRC_GET_TIMING_INFO \
									_IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_GET_TIMING_INFO, struct disp_input_timing)
#define DISP_INPUTSRC_SET_VRR _IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_SET_VRR, int)
#define DISP_INPUTSRC_GET_VRR _IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_GET_VRR, int)
#define DISP_INPUTSRC_POWER_CTRL _IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_POWER_CTRL, int)
#define DISP_INPUTSRC_UPDATE_EDID \
									_IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_UPDATE_EDID, struct disp_input_edid)
#define DISP_INPUTSRC_GET_EDID _IOWR(DISP_INPUTSRC_IOCTL_MAGIC, DISP_INPUTSRC_IOCTL_GET_EDID, struct disp_input_edid)

struct rx_interface {
	int32_t (*wait_vactive_start)(struct rx_interface *rx, int *flag);
	int32_t (*enable_vactive_start)(struct rx_interface *rx, int enable);
	int32_t (*get_port_status)(struct rx_interface *rx, struct disp_input_port_status port_status);
	int32_t (*disconnect)(struct rx_interface *rx);
	int32_t (*connect)(struct rx_interface *rx);
	int32_t (*get_timing_info)(struct rx_interface *rx, struct disp_input_timing *timing);
	int32_t (*set_vrr)(struct rx_interface *rx, uint32_t enable);
	int32_t (*get_vrr)(struct rx_interface *rx, uint32_t *enable);
	int32_t (*power_ctrl)(struct rx_interface *rx, uint32_t power_on);
	int32_t (*update_edid)(struct rx_interface *rx, struct disp_input_edid *edid);
	int32_t (*get_edid)(struct rx_interface *rx, struct disp_input_edid *edid);
};

#endif
