
/* SPDX-License-Identifier: BSD-3-Clause */
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 Protected and is dual licensed,
 either 'STMicroelectronics Proprietary license'
 or 'BSD 3-clause "New" or "Revised" License' , at your option.

 ******************************************************************************

 'STMicroelectronics Proprietary license'

 ******************************************************************************

 License terms: STMicroelectronics Proprietary in accordance with licensing
 terms at www.st.com/sla0081

 ******************************************************************************
 */





#ifndef _VL53L3_HIST_FUNCS_H_
#define _VL53L3_HIST_FUNCS_H_

#include "vl53l3_types.h"
#include "vl53l3_ll_def.h"

#ifdef __cplusplus
extern "C"
{
#endif




VL53L3_Error VL53L3_hist_process_data(
	VL53L3_dmax_calibration_data_t    *pdmax_cal,
	VL53L3_hist_gen3_dmax_config_t    *pdmax_cfg,
	VL53L3_hist_post_process_config_t *ppost_cfg,
	VL53L3_histogram_bin_data_t       *pbins,
	VL53L3_xtalk_histogram_data_t     *pxtalk,
	uint8_t                           *pArea1,
	uint8_t                           *pArea2,
	VL53L3_range_results_t             *presults,
	uint8_t                            *HistMergeNumber);




VL53L3_Error VL53L3_hist_ambient_dmax(
	uint16_t                            target_reflectance,
	VL53L3_dmax_calibration_data_t     *pdmax_cal,
	VL53L3_hist_gen3_dmax_config_t     *pdmax_cfg,
	VL53L3_histogram_bin_data_t        *pbins,
	int16_t                            *pambient_dmax_mm);


#ifdef __cplusplus
}
#endif

#endif
