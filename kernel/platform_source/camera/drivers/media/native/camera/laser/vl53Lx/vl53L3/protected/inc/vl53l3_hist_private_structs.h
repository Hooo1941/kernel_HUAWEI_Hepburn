
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





#ifndef _VL53L3_HIST_PRIVATE_STRUCTS_H_
#define _VL53L3_HIST_PRIVATE_STRUCTS_H_

#include "vl53l3_types.h"
#include "vl53l3_hist_structs.h"

#define VL53L3_D_001         8

#ifdef __cplusplus
extern "C" {
#endif




typedef struct {

	uint8_t  VL53L3_p_022;

	uint8_t  VL53L3_p_023;

	uint8_t  VL53L3_p_024;

	uint8_t  VL53L3_p_030;

	int32_t  VL53L3_p_020;


	int32_t   VL53L3_p_048[VL53L3_HISTOGRAM_BUFFER_SIZE];
	int32_t   VL53L3_p_069[VL53L3_HISTOGRAM_BUFFER_SIZE];

	uint8_t   VL53L3_p_043[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_008[VL53L3_HISTOGRAM_BUFFER_SIZE];
	uint16_t  VL53L3_p_017[VL53L3_HISTOGRAM_BUFFER_SIZE];
	uint16_t  VL53L3_p_011[VL53L3_HISTOGRAM_BUFFER_SIZE];

} VL53L3_hist_gen1_algo_private_data_t;




typedef struct {

	uint8_t  VL53L3_p_022;

	uint8_t  VL53L3_p_023;

	uint8_t  VL53L3_p_024;

	uint16_t VL53L3_p_019;

	uint8_t  VL53L3_p_009;

	uint8_t  VL53L3_p_030;

	int32_t  VL53L3_p_004;

	int32_t  VL53L3_p_020;


	int32_t   VL53L3_p_003[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_018[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_001[VL53L3_HISTOGRAM_BUFFER_SIZE];


	int32_t   VL53L3_p_008[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_041[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_039[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_040[VL53L3_HISTOGRAM_BUFFER_SIZE];


} VL53L3_hist_gen2_algo_filtered_data_t;




typedef struct {

	uint8_t  VL53L3_p_022;

	uint8_t  VL53L3_p_023;

	uint8_t  VL53L3_p_024;

	int32_t  VL53L3_p_032;


	uint8_t   VL53L3_p_042[VL53L3_HISTOGRAM_BUFFER_SIZE];

	uint8_t   VL53L3_p_044[VL53L3_HISTOGRAM_BUFFER_SIZE];


	uint32_t  VL53L3_p_017[VL53L3_HISTOGRAM_BUFFER_SIZE];

	uint16_t  VL53L3_p_011[VL53L3_HISTOGRAM_BUFFER_SIZE];


	uint8_t   VL53L3_p_043[VL53L3_HISTOGRAM_BUFFER_SIZE];


} VL53L3_hist_gen2_algo_detection_data_t;




typedef struct {

	uint8_t  VL53L3_p_015;

	uint8_t  VL53L3_p_022;

	uint8_t  VL53L3_p_025;

	uint8_t  VL53L3_p_026;

	uint8_t  VL53L3_p_016;


	uint8_t  VL53L3_p_027;

	uint8_t  VL53L3_p_055;


	int32_t  VL53L3_p_020;

	int32_t  VL53L3_p_021;

	int32_t  VL53L3_p_013;


	uint32_t VL53L3_p_028;

	uint32_t VL53L3_p_014;

	uint32_t VL53L3_p_029;


	uint16_t VL53L3_p_005;


} VL53L3_hist_pulse_data_t;




typedef struct {

	uint8_t  VL53L3_p_022;

	uint8_t  VL53L3_p_023;

	uint8_t  VL53L3_p_024;

	uint8_t  VL53L3_p_031;

	uint8_t  VL53L3_p_045;

	int32_t  VL53L3_p_004;

	int32_t  VL53L3_p_032;


	uint8_t  VL53L3_p_043[VL53L3_HISTOGRAM_BUFFER_SIZE];

	uint8_t  VL53L3_p_046[VL53L3_HISTOGRAM_BUFFER_SIZE];

	uint8_t  VL53L3_p_047[VL53L3_HISTOGRAM_BUFFER_SIZE];


	int32_t  VL53L3_p_056[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t  VL53L3_p_048[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t  VL53L3_p_008[VL53L3_HISTOGRAM_BUFFER_SIZE];


	uint8_t  VL53L3_p_049;

	uint8_t  VL53L3_p_050;

	uint8_t  VL53L3_p_051;


	VL53L3_hist_pulse_data_t  VL53L3_p_002[VL53L3_D_001];




	VL53L3_histogram_bin_data_t   VL53L3_p_010;

	VL53L3_histogram_bin_data_t   VL53L3_p_038;

	VL53L3_histogram_bin_data_t   VL53L3_p_052;

	VL53L3_histogram_bin_data_t   VL53L3_p_053;

	VL53L3_histogram_bin_data_t   VL53L3_p_054;




} VL53L3_hist_gen3_algo_private_data_t;




typedef struct {

	uint8_t  VL53L3_p_022;

	uint8_t  VL53L3_p_023;

	uint8_t  VL53L3_p_024;


	int32_t   VL53L3_p_003[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_018[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_001[VL53L3_HISTOGRAM_BUFFER_SIZE];


	int32_t   VL53L3_p_039[VL53L3_HISTOGRAM_BUFFER_SIZE];

	int32_t   VL53L3_p_040[VL53L3_HISTOGRAM_BUFFER_SIZE];


	uint8_t  VL53L3_p_043[VL53L3_HISTOGRAM_BUFFER_SIZE];


} VL53L3_hist_gen4_algo_filtered_data_t;

#ifdef __cplusplus
}
#endif

#endif
