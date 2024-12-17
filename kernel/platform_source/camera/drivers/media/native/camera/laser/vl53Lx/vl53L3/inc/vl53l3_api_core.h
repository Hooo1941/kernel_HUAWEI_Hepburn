
/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 and is dual licensed,
 either GPL-2.0+
 or 'BSD 3-clause "New" or "Revised" License' , at your option.
 ******************************************************************************
 */




#ifndef _VL53L3_API_CORE_H_
#define _VL53L3_API_CORE_H_

#include "vl53l3_platform.h"

#ifdef __cplusplus
extern "C" {
#endif




VL53L3_Error VL53L3_get_version(
	VL53L3_DEV            Dev,
	VL53L3_ll_version_t  *pversion);




VL53L3_Error VL53L3_get_device_firmware_version(
	VL53L3_DEV         Dev,
	uint16_t          *pfw_version);




VL53L3_Error VL53L3_data_init(
	VL53L3_DEV         Dev,
	uint8_t            read_p2p_data);




VL53L3_Error VL53L3_read_p2p_data(
	VL53L3_DEV      Dev);




VL53L3_Error VL53L3_software_reset(
	VL53L3_DEV      Dev);




VL53L3_Error VL53L3_set_part_to_part_data(
	VL53L3_DEV                            Dev,
	VL53L3_calibration_data_t            *pcal_data);




VL53L3_Error VL53L3_get_part_to_part_data(
	VL53L3_DEV                            Dev,
	VL53L3_calibration_data_t            *pcal_data);




VL53L3_Error VL53L3_get_tuning_debug_data(
	VL53L3_DEV                            Dev,
	VL53L3_tuning_parameters_t            *ptun_data);




VL53L3_Error VL53L3_set_inter_measurement_period_ms(
	VL53L3_DEV          Dev,
	uint32_t            inter_measurement_period_ms);




VL53L3_Error VL53L3_get_inter_measurement_period_ms(
	VL53L3_DEV          Dev,
	uint32_t           *pinter_measurement_period_ms);




VL53L3_Error VL53L3_set_timeouts_us(
	VL53L3_DEV          Dev,
	uint32_t            phasecal_config_timeout_us,
	uint32_t            mm_config_timeout_us,
	uint32_t            range_config_timeout_us);




VL53L3_Error VL53L3_get_timeouts_us(
	VL53L3_DEV          Dev,
	uint32_t           *pphasecal_config_timeout_us,
	uint32_t           *pmm_config_timeout_us,
	uint32_t           *prange_config_timeout_us);




VL53L3_Error VL53L3_set_calibration_repeat_period(
	VL53L3_DEV          Dev,
	uint16_t            cal_config__repeat_period);




VL53L3_Error VL53L3_get_calibration_repeat_period(
	VL53L3_DEV          Dev,
	uint16_t           *pcal_config__repeat_period);




VL53L3_Error VL53L3_set_sequence_config_bit(
	VL53L3_DEV                   Dev,
	VL53L3_DeviceSequenceConfig  bit_id,
	uint8_t                      value);




VL53L3_Error VL53L3_get_sequence_config_bit(
	VL53L3_DEV                   Dev,
	VL53L3_DeviceSequenceConfig  bit_id,
	uint8_t                     *pvalue);




VL53L3_Error VL53L3_set_interrupt_polarity(
	VL53L3_DEV                       Dev,
	VL53L3_DeviceInterruptPolarity  interrupt_polarity);




VL53L3_Error VL53L3_get_interrupt_polarity(
	VL53L3_DEV                      Dev,
	VL53L3_DeviceInterruptPolarity  *pinterrupt_polarity);



VL53L3_Error VL53L3_get_refspadchar_config_struct(
	VL53L3_DEV                     Dev,
	VL53L3_refspadchar_config_t   *pdata);



VL53L3_Error VL53L3_set_refspadchar_config_struct(
	VL53L3_DEV                     Dev,
	VL53L3_refspadchar_config_t   *pdata);



VL53L3_Error VL53L3_set_range_ignore_threshold(
	VL53L3_DEV              Dev,
	uint8_t                 range_ignore_thresh_mult,
	uint16_t                range_ignore_threshold_mcps);



VL53L3_Error VL53L3_get_range_ignore_threshold(
	VL53L3_DEV              Dev,
	uint8_t                *prange_ignore_thresh_mult,
	uint16_t               *prange_ignore_threshold_mcps_internal,
	uint16_t               *prange_ignore_threshold_mcps_current);




VL53L3_Error VL53L3_set_user_zone(
	VL53L3_DEV          Dev,
	VL53L3_user_zone_t *puser_zone);




VL53L3_Error VL53L3_get_user_zone(
	VL53L3_DEV          Dev,
	VL53L3_user_zone_t *puser_zone);




VL53L3_Error VL53L3_get_mode_mitigation_roi(
	VL53L3_DEV          Dev,
	VL53L3_user_zone_t *pmm_roi);




VL53L3_Error VL53L3_set_zone_config(
	VL53L3_DEV             Dev,
	VL53L3_zone_config_t  *pzone_cfg);




VL53L3_Error VL53L3_get_zone_config(
	VL53L3_DEV             Dev,
	VL53L3_zone_config_t  *pzone_cfg);




VL53L3_Error VL53L3_set_preset_mode(
	VL53L3_DEV                   Dev,
	VL53L3_DevicePresetModes     device_preset_mode,
	uint16_t                     dss_config__target_total_rate_mcps,
	uint32_t                     phasecal_config_timeout_us,
	uint32_t                     mm_config_timeout_us,
	uint32_t                     range_config_timeout_us,
	uint32_t                     inter_measurement_period_ms);




VL53L3_Error VL53L3_get_preset_mode_timing_cfg(
	VL53L3_DEV                   Dev,
	VL53L3_DevicePresetModes     device_preset_mode,
	uint16_t                    *pdss_config__target_total_rate_mcps,
	uint32_t                    *pphasecal_config_timeout_us,
	uint32_t                    *pmm_config_timeout_us,
	uint32_t                    *prange_config_timeout_us);



VL53L3_Error VL53L3_set_zone_preset(
	VL53L3_DEV               Dev,
	VL53L3_DeviceZonePreset  zone_preset);



VL53L3_Error VL53L3_enable_xtalk_compensation(
	VL53L3_DEV                 Dev);



VL53L3_Error VL53L3_disable_xtalk_compensation(
	VL53L3_DEV                 Dev);




void VL53L3_get_xtalk_compensation_enable(
	VL53L3_DEV    Dev,
	uint8_t       *pcrosstalk_compensation_enable);



VL53L3_Error VL53L3_init_and_start_range(
	VL53L3_DEV                      Dev,
	uint8_t                         measurement_mode,
	VL53L3_DeviceConfigLevel        device_config_level);




VL53L3_Error VL53L3_stop_range(
	VL53L3_DEV  Dev);




VL53L3_Error VL53L3_get_measurement_results(
	VL53L3_DEV                  Dev,
	VL53L3_DeviceResultsLevel   device_result_level);




VL53L3_Error VL53L3_get_device_results(
	VL53L3_DEV                 Dev,
	VL53L3_DeviceResultsLevel  device_result_level,
	VL53L3_range_results_t    *prange_results);




VL53L3_Error VL53L3_clear_interrupt_and_enable_next_range(
	VL53L3_DEV       Dev,
	uint8_t          measurement_mode);




VL53L3_Error VL53L3_get_histogram_bin_data(
	VL53L3_DEV                   Dev,
	VL53L3_histogram_bin_data_t *phist_data);




void VL53L3_copy_sys_and_core_results_to_range_results(
	int32_t                           gain_factor,
	VL53L3_system_results_t          *psys,
	VL53L3_core_results_t            *pcore,
	VL53L3_range_results_t           *presults);



VL53L3_Error VL53L3_set_zone_dss_config(
	  VL53L3_DEV                      Dev,
	  VL53L3_zone_private_dyn_cfg_t  *pzone_dyn_cfg);




VL53L3_Error VL53L3_calc_ambient_dmax(
	VL53L3_DEV    Dev,
	uint16_t      target_reflectance,
	int16_t      *pambient_dmax_mm);




VL53L3_Error VL53L3_set_GPIO_interrupt_config(
	VL53L3_DEV                      Dev,
	VL53L3_GPIO_Interrupt_Mode	intr_mode_distance,
	VL53L3_GPIO_Interrupt_Mode	intr_mode_rate,
	uint8_t				intr_new_measure_ready,
	uint8_t				intr_no_target,
	uint8_t				intr_combined_mode,
	uint16_t			thresh_distance_high,
	uint16_t			thresh_distance_low,
	uint16_t			thresh_rate_high,
	uint16_t			thresh_rate_low
	);



VL53L3_Error VL53L3_set_GPIO_interrupt_config_struct(
	VL53L3_DEV                      Dev,
	VL53L3_GPIO_interrupt_config_t	intconf);



VL53L3_Error VL53L3_get_GPIO_interrupt_config(
	VL53L3_DEV                      Dev,
	VL53L3_GPIO_interrupt_config_t	*pintconf);




VL53L3_Error VL53L3_set_dmax_mode(
	VL53L3_DEV              Dev,
	VL53L3_DeviceDmaxMode   dmax_mode);



VL53L3_Error VL53L3_get_dmax_mode(
	VL53L3_DEV               Dev,
	VL53L3_DeviceDmaxMode   *pdmax_mode);




VL53L3_Error VL53L3_get_dmax_calibration_data(
	VL53L3_DEV                      Dev,
	VL53L3_DeviceDmaxMode           dmax_mode,
	uint8_t                         zone_id,
	VL53L3_dmax_calibration_data_t *pdmax_cal);




VL53L3_Error VL53L3_set_hist_dmax_config(
	VL53L3_DEV                      Dev,
	VL53L3_hist_gen3_dmax_config_t *pdmax_cfg);



VL53L3_Error VL53L3_get_hist_dmax_config(
	VL53L3_DEV                      Dev,
	VL53L3_hist_gen3_dmax_config_t *pdmax_cfg);




VL53L3_Error VL53L3_set_offset_calibration_mode(
	VL53L3_DEV                      Dev,
	VL53L3_OffsetCalibrationMode   offset_cal_mode);




VL53L3_Error VL53L3_get_offset_calibration_mode(
	VL53L3_DEV                      Dev,
	VL53L3_OffsetCalibrationMode  *poffset_cal_mode);




VL53L3_Error VL53L3_set_offset_correction_mode(
	VL53L3_DEV                     Dev,
	VL53L3_OffsetCalibrationMode   offset_cor_mode);




VL53L3_Error VL53L3_get_offset_correction_mode(
	VL53L3_DEV                    Dev,
	VL53L3_OffsetCorrectionMode  *poffset_cor_mode);




VL53L3_Error VL53L3_set_zone_calibration_data(
	VL53L3_DEV                         Dev,
	VL53L3_zone_calibration_results_t *pzone_cal);




VL53L3_Error VL53L3_get_zone_calibration_data(
	VL53L3_DEV                         Dev,
	VL53L3_zone_calibration_results_t *pzone_cal);




VL53L3_Error VL53L3_get_lite_xtalk_margin_kcps(
	VL53L3_DEV                          Dev,
	int16_t                           *pxtalk_margin);



VL53L3_Error VL53L3_set_lite_xtalk_margin_kcps(
	VL53L3_DEV                          Dev,
	int16_t                             xtalk_margin);




VL53L3_Error VL53L3_get_histogram_xtalk_margin_kcps(
	VL53L3_DEV                          Dev,
	int16_t                           *pxtalk_margin);



VL53L3_Error VL53L3_set_histogram_xtalk_margin_kcps(
	VL53L3_DEV                          Dev,
	int16_t                             xtalk_margin);



VL53L3_Error VL53L3_get_histogram_phase_consistency(
	VL53L3_DEV                          Dev,
	uint8_t                            *pphase_consistency);



VL53L3_Error VL53L3_set_histogram_phase_consistency(
	VL53L3_DEV                          Dev,
	uint8_t                             phase_consistency);



VL53L3_Error VL53L3_get_histogram_event_consistency(
	VL53L3_DEV                          Dev,
	uint8_t                            *pevent_consistency);



VL53L3_Error VL53L3_set_histogram_event_consistency(
	VL53L3_DEV                          Dev,
	uint8_t                             event_consistency);



VL53L3_Error VL53L3_get_histogram_ambient_threshold_sigma(
	VL53L3_DEV                          Dev,
	uint8_t                            *pamb_thresh_sigma);



VL53L3_Error VL53L3_set_histogram_ambient_threshold_sigma(
	VL53L3_DEV                          Dev,
	uint8_t                             amb_thresh_sigma);



VL53L3_Error VL53L3_get_lite_min_count_rate(
	VL53L3_DEV                          Dev,
	uint16_t                           *plite_mincountrate);




VL53L3_Error VL53L3_set_lite_min_count_rate(
	VL53L3_DEV                          Dev,
	uint16_t                            lite_mincountrate);





VL53L3_Error VL53L3_get_lite_sigma_threshold(
	VL53L3_DEV                          Dev,
	uint16_t                           *plite_sigma);




VL53L3_Error VL53L3_set_lite_sigma_threshold(
	VL53L3_DEV                          Dev,
	uint16_t                            lite_sigma);




VL53L3_Error VL53L3_restore_xtalk_nvm_default(
	VL53L3_DEV                     Dev);



VL53L3_Error VL53L3_get_xtalk_detect_config(
	VL53L3_DEV                          Dev,
	int16_t                            *pmax_valid_range_mm,
	int16_t                            *pmin_valid_range_mm,
	uint16_t                           *pmax_valid_rate_kcps,
	uint16_t                           *pmax_sigma_mm);



VL53L3_Error VL53L3_set_xtalk_detect_config(
	VL53L3_DEV                          Dev,
	int16_t                             max_valid_range_mm,
	int16_t                             min_valid_range_mm,
	uint16_t                            max_valid_rate_kcps,
	uint16_t                            max_sigma_mm);



VL53L3_Error VL53L3_get_target_order_mode(
	VL53L3_DEV                          Dev,
	VL53L3_HistTargetOrder             *phist_target_order);



VL53L3_Error VL53L3_set_target_order_mode(
	VL53L3_DEV                          Dev,
	VL53L3_HistTargetOrder              hist_target_order);




VL53L3_Error VL53L3_set_dmax_reflectance_values(
	VL53L3_DEV                          Dev,
	VL53L3_dmax_reflectance_array_t    *pdmax_reflectances);



VL53L3_Error VL53L3_get_dmax_reflectance_values(
	VL53L3_DEV                          Dev,
	VL53L3_dmax_reflectance_array_t    *pdmax_reflectances);



VL53L3_Error VL53L3_set_vhv_config(
	VL53L3_DEV                   Dev,
	uint8_t                      vhv_init_en,
	uint8_t                      vhv_init_value);



VL53L3_Error VL53L3_get_vhv_config(
	VL53L3_DEV                   Dev,
	uint8_t                     *pvhv_init_en,
	uint8_t                     *pvhv_init_value);



VL53L3_Error VL53L3_set_vhv_loopbound(
	VL53L3_DEV                   Dev,
	uint8_t                      vhv_loopbound);



VL53L3_Error VL53L3_get_vhv_loopbound(
	VL53L3_DEV                   Dev,
	uint8_t                     *pvhv_loopbound);



VL53L3_Error VL53L3_get_tuning_parm(
	VL53L3_DEV                     Dev,
	VL53L3_TuningParms             tuning_parm_key,
	int32_t                       *ptuning_parm_value);



VL53L3_Error VL53L3_set_tuning_parm(
	VL53L3_DEV                     Dev,
	VL53L3_TuningParms             tuning_parm_key,
	int32_t                        tuning_parm_value);



VL53L3_Error VL53L3_dynamic_xtalk_correction_enable(
	VL53L3_DEV                     Dev
	);



VL53L3_Error VL53L3_dynamic_xtalk_correction_disable(
	VL53L3_DEV                     Dev
	);




VL53L3_Error VL53L3_dynamic_xtalk_correction_apply_enable(
	VL53L3_DEV                          Dev
	);



VL53L3_Error VL53L3_dynamic_xtalk_correction_apply_disable(
	VL53L3_DEV                          Dev
	);



VL53L3_Error VL53L3_dynamic_xtalk_correction_single_apply_enable(
	VL53L3_DEV                          Dev
	);



VL53L3_Error VL53L3_dynamic_xtalk_correction_single_apply_disable(
	VL53L3_DEV                          Dev
	);



VL53L3_Error VL53L3_dynamic_xtalk_correction_set_scalers(
	VL53L3_DEV	Dev,
	int16_t		x_scaler_in,
	int16_t		y_scaler_in,
	uint8_t		user_scaler_set_in
	);



VL53L3_Error VL53L3_get_current_xtalk_settings(
	VL53L3_DEV                          Dev,
	VL53L3_xtalk_calibration_results_t *pxtalk
	);



VL53L3_Error VL53L3_set_current_xtalk_settings(
	VL53L3_DEV                          Dev,
	VL53L3_xtalk_calibration_results_t *pxtalk
	);

VL53L3_Error VL53L3_load_patch(VL53L3_DEV Dev);

VL53L3_Error VL53L3_unload_patch(VL53L3_DEV Dev);

#ifdef __cplusplus
}
#endif

#endif

