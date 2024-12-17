
/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 and is dual licensed,
 either GPL-2.0+
 or 'BSD 3-clause "New" or "Revised" License' , at your option.
 ******************************************************************************
 */




#ifndef _VL53L3_API_CALIBRATION_H_
#define _VL53L3_API_CALIBRATION_H_

#include "vl53l3_platform.h"

#ifdef __cplusplus
extern "C" {
#endif




VL53L3_Error VL53L3_run_ref_spad_char(VL53L3_DEV Dev,
		VL53L3_Error           * pcal_status);




VL53L3_Error VL53L3_run_device_test(
	VL53L3_DEV                 Dev,
	VL53L3_DeviceTestMode      device_test_mode);




VL53L3_Error VL53L3_run_spad_rate_map(
	VL53L3_DEV                 Dev,
	VL53L3_DeviceTestMode      device_test_mode,
	VL53L3_DeviceSscArray      array_select,
	uint32_t                   ssc_config_timeout_us,
	VL53L3_spad_rate_data_t   *pspad_rate_data);




VL53L3_Error   VL53L3_run_xtalk_extraction(
	VL53L3_DEV	                        Dev,
	VL53L3_Error                       *pcal_status);



VL53L3_Error VL53L3_get_and_avg_xtalk_samples(
		VL53L3_DEV	                  Dev,
		uint8_t                       num_of_samples,
		uint8_t                       measurement_mode,
		int16_t                       xtalk_filter_thresh_max_mm,
		int16_t                       xtalk_filter_thresh_min_mm,
		uint16_t                      xtalk_max_valid_rate_kcps,
		uint8_t                       xtalk_result_id,
		uint8_t                       xtalk_histo_id,
		VL53L3_xtalk_range_results_t *pxtalk_results,
		VL53L3_histogram_bin_data_t  *psum_histo,
		VL53L3_histogram_bin_data_t  *pavg_histo);



VL53L3_Error   VL53L3_run_offset_calibration(
	VL53L3_DEV	                  Dev,
	int16_t                       cal_distance_mm,
	uint16_t                      cal_reflectance_pc,
	VL53L3_Error                 *pcal_status);




VL53L3_Error   VL53L3_run_phasecal_average(
	VL53L3_DEV	            Dev,
	uint8_t                 measurement_mode,
	uint8_t                 phasecal_result__vcsel_start,
	uint16_t                phasecal_num_of_samples,
	VL53L3_range_results_t *prange_results,
	uint16_t               *pphasecal_result__reference_phase,
	uint16_t               *pzero_distance_phase);




VL53L3_Error VL53L3_run_zone_calibration(
	VL53L3_DEV	                  Dev,
	VL53L3_DevicePresetModes      device_preset_mode,
	VL53L3_DeviceZonePreset       zone_preset,
	VL53L3_zone_config_t         *pzone_cfg,
	int16_t                       cal_distance_mm,
	uint16_t                      cal_reflectance_pc,
	VL53L3_Error                 *pcal_status);




void VL53L3_hist_xtalk_extract_data_init(
	VL53L3_hist_xtalk_extract_data_t   *pxtalk_data);



VL53L3_Error VL53L3_hist_xtalk_extract_update(
	int16_t                             target_distance_mm,
	uint16_t                            target_width_oversize,
	VL53L3_histogram_bin_data_t        *phist_bins,
	VL53L3_hist_xtalk_extract_data_t   *pxtalk_data);



VL53L3_Error VL53L3_hist_xtalk_extract_fini(
	VL53L3_histogram_bin_data_t        *phist_bins,
	VL53L3_hist_xtalk_extract_data_t   *pxtalk_data,
	VL53L3_xtalk_calibration_results_t *pxtalk_cal,
	VL53L3_xtalk_histogram_shape_t     *pxtalk_shape);




VL53L3_Error   VL53L3_run_hist_xtalk_extraction(
	VL53L3_DEV	                  Dev,
	int16_t                       cal_distance_mm,
	VL53L3_Error                 *pcal_status);


#ifdef __cplusplus
}
#endif

#endif

