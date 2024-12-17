
/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 and is dual licensed,
 either GPL-2.0+
 or 'BSD 3-clause "New" or "Revised" License' , at your option.
 ******************************************************************************
 */




#ifndef _VL53L3_API_DEBUG_H_
#define _VL53L3_API_DEBUG_H_

#include "vl53l3_platform.h"
#include "vl53l3_nvm_structs.h"

#ifdef __cplusplus
extern "C" {
#endif





VL53L3_Error VL53L3_decode_calibration_data_buffer(
	uint16_t                   buf_size,
	uint8_t                   *pbuffer,
	VL53L3_calibration_data_t *pdata);






VL53L3_Error VL53L3_get_nvm_debug_data(
	VL53L3_DEV                 Dev,
	VL53L3_decoded_nvm_data_t *pdata);



VL53L3_Error VL53L3_get_histogram_debug_data(
	VL53L3_DEV                   Dev,
	VL53L3_histogram_bin_data_t *pdata);






VL53L3_Error VL53L3_get_additional_data(
	VL53L3_DEV                Dev,
	VL53L3_additional_data_t *pdata);






VL53L3_Error VL53L3_get_xtalk_debug_data(
	VL53L3_DEV                 Dev,
	VL53L3_xtalk_debug_data_t *pdata);




VL53L3_Error VL53L3_get_offset_debug_data(
	VL53L3_DEV                 Dev,
	VL53L3_offset_debug_data_t *pdata);

#ifdef VL53L3_LOG_ENABLE



void  VL53L3_signed_fixed_point_sprintf(
	int32_t    fp_value,
	uint8_t    frac_bits,
	uint16_t   buf_size,
	char      *pbuffer);




void VL53L3_print_static_nvm_managed(
	VL53L3_static_nvm_managed_t   *pdata,
	char                          *pprefix,
	uint32_t                       trace_flags);




void VL53L3_print_customer_nvm_managed(
	VL53L3_customer_nvm_managed_t *pdata,
	char                          *pprefix,
	uint32_t                       trace_flags);




void VL53L3_print_nvm_copy_data(
	VL53L3_nvm_copy_data_t        *pdata,
	char                          *pprefix,
	uint32_t                       trace_flags);




void VL53L3_print_histogram_bin_data(
	VL53L3_histogram_bin_data_t *pdata,
	char                        *pprefix,
	uint32_t                     trace_flags);




void VL53L3_print_xtalk_histogram_data(
	VL53L3_xtalk_histogram_data_t *pdata,
	char                          *pprefix,
	uint32_t                       trace_flags);




void VL53L3_print_xtalk_histogram_shape_data(
	VL53L3_xtalk_histogram_shape_t *pdata,
	char                           *pprefix,
	uint32_t                        trace_flags);




void VL53L3_print_range_results(
	VL53L3_range_results_t *pdata,
	char                   *pprefix,
	uint32_t                trace_flags);



void VL53L3_print_range_data(
	VL53L3_range_data_t *pdata,
	char                *pprefix,
	uint32_t             trace_flags);




void VL53L3_print_offset_range_results(
	VL53L3_offset_range_results_t *pdata,
	char                          *pprefix,
	uint32_t                       trace_flags);




void VL53L3_print_offset_range_data(
	VL53L3_offset_range_data_t *pdata,
	char                       *pprefix,
	uint32_t                    trace_flags);




void VL53L3_print_cal_peak_rate_map(
	VL53L3_cal_peak_rate_map_t *pdata,
	char                       *pprefix,
	uint32_t                    trace_flags);




void VL53L3_print_additional_offset_cal_data(
	VL53L3_additional_offset_cal_data_t *pdata,
	char                                *pprefix,
	uint32_t                             trace_flags);



void VL53L3_print_additional_data(
	VL53L3_additional_data_t *pdata,
	char                     *pprefix,
	uint32_t                 trace_flags);




void VL53L3_print_gain_calibration_data(
	VL53L3_gain_calibration_data_t *pdata,
	char                           *pprefix,
	uint32_t                        trace_flags);




void VL53L3_print_zone_calibration_data(
	VL53L3_zone_calibration_data_t *pdata,
	char                           *pprefix,
	uint32_t                        trace_flags);




void VL53L3_print_zone_calibration_results(
	VL53L3_zone_calibration_results_t *pdata,
	char                              *pprefix,
	uint32_t                           trace_flags);




void VL53L3_print_xtalk_range_results(
	VL53L3_xtalk_range_results_t *pdata,
	char                         *pprefix,
	uint32_t                      trace_flags);




void VL53L3_print_xtalk_range_data(
	VL53L3_xtalk_range_data_t *pdata,
	char                      *pprefix,
	uint32_t                   trace_flags);




void VL53L3_print_xtalk_calibration_results(
	VL53L3_xtalk_calibration_results_t *pdata,
	char                               *pprefix,
	uint32_t                            trace_flags);




void VL53L3_print_xtalk_config(
	VL53L3_xtalk_config_t *pdata,
	char                  *pprefix,
	uint32_t               trace_flags);



void VL53L3_print_xtalk_extract_config(
	VL53L3_xtalkextract_config_t *pdata,
	char                         *pprefix,
	uint32_t                      trace_flags);



void VL53L3_print_zone_cal_config(
	VL53L3_zonecal_config_t *pdata,
	char                    *pprefix,
	uint32_t                 trace_flags);



void VL53L3_print_offset_cal_config(
	VL53L3_offsetcal_config_t *pdata,
	char                      *pprefix,
	uint32_t                   trace_flags);




void VL53L3_print_dmax_calibration_data(
	VL53L3_dmax_calibration_data_t *pdata,
	char                           *pprefix,
	uint32_t                        trace_flags);




void VL53L3_print_calibration_data(
	VL53L3_calibration_data_t *pdata,
	char                      *pprefix,
	uint32_t                   trace_flags);




void VL53L3_print_xtalk_debug_data(
	VL53L3_xtalk_debug_data_t *pdata,
	char                      *pprefix,
	uint32_t                   trace_flags);



void VL53L3_print_offset_debug_data(
	VL53L3_offset_debug_data_t *pdata,
	char                       *pprefix,
	uint32_t                    trace_flags);




void VL53L3_print_optical_centre(
	VL53L3_optical_centre_t   *pdata,
	char                      *pprefix,
	uint32_t                   trace_flags);




void VL53L3_print_user_zone(
	VL53L3_user_zone_t   *pdata,
	char                 *pprefix,
	uint32_t              trace_flags);



void VL53L3_print_zone_config(
	VL53L3_zone_config_t *pdata,
	char                 *pprefix,
	uint32_t              trace_flags);




void VL53L3_print_spad_rate_data(
	VL53L3_spad_rate_data_t  *pspad_rates,
	char                     *pprefix,
	uint32_t                  trace_flags);




void VL53L3_print_spad_rate_map(
	VL53L3_spad_rate_data_t  *pspad_rates,
	char                     *pprefix,
	uint32_t                  trace_flags);


#endif

#ifdef __cplusplus
}
#endif

#endif

