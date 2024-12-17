
// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 and is dual licensed,
 either GPL-2.0+
 or 'BSD 3-clause "New" or "Revised" License' , at your option.
 ******************************************************************************
 */





#include "vl53l3_ll_def.h"
#include "vl53l3_ll_device.h"
#include "vl53l3_platform.h"
#include "vl53l3_core.h"
#include "vl53l3_silicon_core.h"
#include "vl53l3_wait.h"
#include "vl53l3_register_settings.h"


#define LOG_FUNCTION_START(fmt, ...) \
	_LOG_FUNCTION_START(VL53L3_TRACE_MODULE_CORE, fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_END(status, ...) \
	_LOG_FUNCTION_END(VL53L3_TRACE_MODULE_CORE, status, ##__VA_ARGS__)
#define LOG_FUNCTION_END_FMT(status, fmt, ...) \
	_LOG_FUNCTION_END_FMT(VL53L3_TRACE_MODULE_CORE, status, \
		fmt, ##__VA_ARGS__)


VL53L3_Error VL53L3_wait_for_boot_completion(
	VL53L3_DEV     Dev)
{



	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint8_t      fw_ready  = 0;

	LOG_FUNCTION_START("");

	if (pdev->wait_method == VL53L3_WAIT_METHOD_BLOCKING) {



		status =
			VL53L3_poll_for_boot_completion(
				Dev,
				VL53L3_BOOT_COMPLETION_POLLING_TIMEOUT_MS);

	} else {



		fw_ready = 0;
		while (fw_ready == 0x00 && status == VL53L3_ERROR_NONE) {
			status = VL53L3_is_boot_complete(
				Dev,
				&fw_ready);

			if (status == VL53L3_ERROR_NONE) {
				status = VL53L3_WaitMs(
					Dev,
					VL53L3_POLLING_DELAY_MS);
			}
		}
	}

	LOG_FUNCTION_END(status);

	return status;

}


VL53L3_Error VL53L3_wait_for_firmware_ready(
	VL53L3_DEV     Dev)
{



	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint8_t      fw_ready  = 0;
	uint8_t      mode_start  = 0;

	LOG_FUNCTION_START("");


	mode_start =
		pdev->sys_ctrl.system__mode_start &
		VL53L3_DEVICEMEASUREMENTMODE_MODE_MASK;



	if ((mode_start == VL53L3_DEVICEMEASUREMENTMODE_TIMED) ||
		(mode_start == VL53L3_DEVICEMEASUREMENTMODE_SINGLESHOT)) {

		if (pdev->wait_method == VL53L3_WAIT_METHOD_BLOCKING) {



			status =
			VL53L3_poll_for_firmware_ready(
				Dev,
				VL53L3_RANGE_COMPLETION_POLLING_TIMEOUT_MS);

		} else {



			fw_ready = 0;
			while (fw_ready == 0x00 && status ==
					VL53L3_ERROR_NONE) {
				status = VL53L3_is_firmware_ready(
					Dev,
					&fw_ready);

				if (status == VL53L3_ERROR_NONE) {
					status = VL53L3_WaitMs(
						Dev,
						VL53L3_POLLING_DELAY_MS);
				}
			}
		}
	}

	LOG_FUNCTION_END(status);

	return status;
}


VL53L3_Error VL53L3_wait_for_range_completion(
	VL53L3_DEV     Dev)
{



	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint8_t      data_ready  = 0;

	LOG_FUNCTION_START("");

	if (pdev->wait_method == VL53L3_WAIT_METHOD_BLOCKING) {



		status =
			VL53L3_poll_for_range_completion(
				Dev,
				VL53L3_RANGE_COMPLETION_POLLING_TIMEOUT_MS);

	} else {



		data_ready = 0;
		while (data_ready == 0x00 && status == VL53L3_ERROR_NONE) {
			status = VL53L3_is_new_data_ready(
				Dev,
				&data_ready);

			if (status == VL53L3_ERROR_NONE) {
				status = VL53L3_WaitMs(
					Dev,
					VL53L3_POLLING_DELAY_MS);
			}
		}
	}

	LOG_FUNCTION_END(status);

	return status;
}


VL53L3_Error VL53L3_wait_for_test_completion(
	VL53L3_DEV     Dev)
{



	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint8_t      data_ready  = 0;

	LOG_FUNCTION_START("");

	if (pdev->wait_method == VL53L3_WAIT_METHOD_BLOCKING) {



		status =
			VL53L3_poll_for_range_completion(
				Dev,
				VL53L3_TEST_COMPLETION_POLLING_TIMEOUT_MS);

	} else {



		data_ready = 0;
		while (data_ready == 0x00 && status == VL53L3_ERROR_NONE) {
			status = VL53L3_is_new_data_ready(
				Dev,
				&data_ready);

			if (status == VL53L3_ERROR_NONE) {
				status = VL53L3_WaitMs(
					Dev,
					VL53L3_POLLING_DELAY_MS);
			}
		}
	}

	LOG_FUNCTION_END(status);

	return status;
}




VL53L3_Error VL53L3_is_boot_complete(
	VL53L3_DEV     Dev,
	uint8_t       *pready)
{


	VL53L3_Error status = VL53L3_ERROR_NONE;
	uint8_t  firmware__system_status = 0;

	LOG_FUNCTION_START("");



	status =
		VL53L3_RdByte(
			Dev,
			VL53L3_FIRMWARE__SYSTEM_STATUS,
			&firmware__system_status);



	if ((firmware__system_status & 0x01) == 0x01) {
		*pready = 0x01;
		VL53L3_init_ll_driver_state(
			Dev,
			VL53L3_DEVICESTATE_SW_STANDBY);
	} else {
		*pready = 0x00;
		VL53L3_init_ll_driver_state(
			Dev,
			VL53L3_DEVICESTATE_FW_COLDBOOT);
	}

	LOG_FUNCTION_END(status);

	return status;
}


VL53L3_Error VL53L3_is_firmware_ready(
	VL53L3_DEV     Dev,
	uint8_t       *pready)
{


	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	LOG_FUNCTION_START("");

	status = VL53L3_is_firmware_ready_silicon(
				Dev,
				pready);

	pdev->fw_ready = *pready;

	LOG_FUNCTION_END(status);

	return status;
}


VL53L3_Error VL53L3_is_new_data_ready(
	VL53L3_DEV     Dev,
	uint8_t       *pready)
{


	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint8_t  gpio__mux_active_high_hv = 0;
	uint8_t  gpio__tio_hv_status      = 0;
	uint8_t  interrupt_ready          = 0;

	LOG_FUNCTION_START("");

	gpio__mux_active_high_hv =
			pdev->stat_cfg.gpio_hv_mux__ctrl &
			VL53L3_DEVICEINTERRUPTLEVEL_ACTIVE_MASK;

	if (gpio__mux_active_high_hv == VL53L3_DEVICEINTERRUPTLEVEL_ACTIVE_HIGH)
		interrupt_ready = 0x01;
	else
		interrupt_ready = 0x00;



	status = VL53L3_RdByte(
					Dev,
					VL53L3_GPIO__TIO_HV_STATUS,
					&gpio__tio_hv_status);



	if ((gpio__tio_hv_status & 0x01) == interrupt_ready)
		*pready = 0x01;
	else
		*pready = 0x00;

	LOG_FUNCTION_END(status);

	return status;
}




VL53L3_Error VL53L3_poll_for_boot_completion(
	VL53L3_DEV    Dev,
	uint32_t      timeout_ms)
{


	VL53L3_Error status       = VL53L3_ERROR_NONE;

	LOG_FUNCTION_START("");



	status = VL53L3_WaitUs(
			Dev,
			VL53L3_FIRMWARE_BOOT_TIME_US);

	if (status == VL53L3_ERROR_NONE)
		status =
			VL53L3_WaitValueMaskEx(
				Dev,
				timeout_ms,
				VL53L3_FIRMWARE__SYSTEM_STATUS,
				0x01,
				0x01,
				VL53L3_POLLING_DELAY_MS);

	if (status == VL53L3_ERROR_NONE)
		VL53L3_init_ll_driver_state(Dev, VL53L3_DEVICESTATE_SW_STANDBY);

	LOG_FUNCTION_END(status);

	return status;
}


VL53L3_Error VL53L3_poll_for_firmware_ready(
	VL53L3_DEV    Dev,
	uint32_t      timeout_ms)
{


	VL53L3_Error status          = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint32_t     start_time_ms   = 0;
	uint32_t     current_time_ms = 0;
	int32_t      poll_delay_ms   = VL53L3_POLLING_DELAY_MS;
	uint8_t      fw_ready        = 0;



	VL53L3_GetTickCount(Dev, &start_time_ms);
	pdev->fw_ready_poll_duration_ms = 0;



	while ((status == VL53L3_ERROR_NONE) &&
		   (pdev->fw_ready_poll_duration_ms < timeout_ms) &&
		   (fw_ready == 0)) {

		status = VL53L3_is_firmware_ready(
			Dev,
			&fw_ready);

		if (status == VL53L3_ERROR_NONE &&
			fw_ready == 0 &&
			poll_delay_ms > 0) {
			status = VL53L3_WaitMs(
				Dev,
				poll_delay_ms);
		}


		VL53L3_GetTickCount(Dev, &current_time_ms);
		pdev->fw_ready_poll_duration_ms =
				current_time_ms - start_time_ms;
	}

	if (fw_ready == 0 && status == VL53L3_ERROR_NONE)
		status = VL53L3_ERROR_TIME_OUT;

	LOG_FUNCTION_END(status);

	return status;
}


VL53L3_Error VL53L3_poll_for_range_completion(
	VL53L3_DEV     Dev,
	uint32_t       timeout_ms)
{


	VL53L3_Error status = VL53L3_ERROR_NONE;
	VL53L3_LLDriverData_t *pdev = VL53L3DevStructGetLLDriverHandle(Dev);

	uint8_t  gpio__mux_active_high_hv = 0;
	uint8_t  interrupt_ready          = 0;

	LOG_FUNCTION_START("");

	gpio__mux_active_high_hv =
			pdev->stat_cfg.gpio_hv_mux__ctrl &
			VL53L3_DEVICEINTERRUPTLEVEL_ACTIVE_MASK;

	if (gpio__mux_active_high_hv == VL53L3_DEVICEINTERRUPTLEVEL_ACTIVE_HIGH)
		interrupt_ready = 0x01;
	else
		interrupt_ready = 0x00;

	status =
		VL53L3_WaitValueMaskEx(
			Dev,
			timeout_ms,
			VL53L3_GPIO__TIO_HV_STATUS,
			interrupt_ready,
			0x01,
			VL53L3_POLLING_DELAY_MS);

	LOG_FUNCTION_END(status);

	return status;
}


