
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




#ifndef _VL53L3_DMAX_H_
#define _VL53L3_DMAX_H_

#include "vl53l3_types.h"
#include "vl53l3_hist_structs.h"
#include "vl53l3_dmax_private_structs.h"
#include "vl53l3_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif




VL53L3_Error VL53L3_f_001(
	uint16_t                              target_reflectance,
	VL53L3_dmax_calibration_data_t	     *pcal,
	VL53L3_hist_gen3_dmax_config_t	     *pcfg,
	VL53L3_histogram_bin_data_t          *pbins,
	VL53L3_hist_gen3_dmax_private_data_t *pdata,
	int16_t                              *pambient_dmax_mm);




uint32_t VL53L3_f_002(
	uint32_t     events_threshold,
	uint32_t     ref_signal_events,
	uint32_t	 ref_distance_mm,
	uint32_t     signal_thresh_sigma);


#ifdef __cplusplus
}
#endif

#endif

