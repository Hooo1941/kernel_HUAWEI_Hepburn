
// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 and is dual licensed,
 either GPL-2.0+
 or 'BSD 3-clause "New" or "Revised" License' , at your option.
 ******************************************************************************
 */

/**
 * @file   vl53l3_platform_ipp.c
 *
 * @brief  EwokPlus25 IPP Wrapper Layer
 */

#include "vl53l3_platform.h"
#include "vl53l3_platform_ipp.h"
#include "vl53l3_ll_def.h"
#include "vl53l3_hist_structs.h"
#include "vl53l3_hist_funcs.h"
#include "vl53l3_xtalk.h"


#define LOG_FUNCTION_START(fmt, ...) \
	_LOG_FUNCTION_START(VL53L3_TRACE_MODULE_CORE, fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_END(status, ...) \
	_LOG_FUNCTION_END(VL53L3_TRACE_MODULE_CORE, status, ##__VA_ARGS__)


VL53L3_Error VL53L3_ipp_hist_process_data(
	VL53L3_DEV                         Dev,
	VL53L3_dmax_calibration_data_t    *pdmax_cal,
	VL53L3_hist_gen3_dmax_config_t    *pdmax_cfg,
	VL53L3_hist_post_process_config_t *ppost_cfg,
	VL53L3_histogram_bin_data_t       *pbins,
	VL53L3_xtalk_histogram_data_t     *pxtalk,
	uint8_t                           *pArea1,
	uint8_t                           *pArea2,
	uint8_t                           *phisto_merge_nb,
	VL53L3_range_results_t            *presults)
{

	/*
	 * IPP wrapper for histogram post processing function
	 */

	VL53L3_Error status         = VL53L3_ERROR_NONE;

	SUPPRESS_UNUSED_WARNING(Dev);

	status =
		VL53L3_hist_process_data(
			pdmax_cal,
			pdmax_cfg,
			ppost_cfg,
			pbins,
			pxtalk,
			pArea1,
			pArea2,
			presults,
			phisto_merge_nb);

	return status;
}


VL53L3_Error VL53L3_ipp_hist_ambient_dmax(
	VL53L3_DEV                         Dev,
	uint16_t                           target_reflectance,
	VL53L3_dmax_calibration_data_t    *pdmax_cal,
	VL53L3_hist_gen3_dmax_config_t    *pdmax_cfg,
	VL53L3_histogram_bin_data_t       *pbins,
	int16_t                           *pambient_dmax_mm)
{

    /*
     * IPP wrapper for histogram ambient DMAX function
     *
     * The target reflectance in percent for the DMAX calculation
	 * is set by target_reflectance input
	 *
	 * The fixed point format is 7.2
     */

    VL53L3_Error status         = VL53L3_ERROR_NONE;

	SUPPRESS_UNUSED_WARNING(Dev);

	status =
		VL53L3_hist_ambient_dmax(
			target_reflectance,
			pdmax_cal,
			pdmax_cfg,
			pbins,
			pambient_dmax_mm);

	return status;
}


VL53L3_Error VL53L3_ipp_xtalk_calibration_process_data(
	VL53L3_DEV                          Dev,
	VL53L3_xtalk_range_results_t       *pxtalk_ranges,
	VL53L3_xtalk_histogram_data_t      *pxtalk_shape,
	VL53L3_xtalk_calibration_results_t *pxtalk_cal)
{

	/*
	 * IPP wrapper for histogram post processing function
	 */

	VL53L3_Error status         = VL53L3_ERROR_NONE;

	SUPPRESS_UNUSED_WARNING(Dev);

	status =
		VL53L3_xtalk_calibration_process_data(
			pxtalk_ranges,
			pxtalk_shape,
			pxtalk_cal);

	return status;
}


