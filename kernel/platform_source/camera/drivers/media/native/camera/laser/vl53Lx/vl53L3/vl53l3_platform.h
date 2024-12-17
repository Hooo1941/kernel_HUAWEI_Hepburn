/*
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _VL53L3_PLATFORM_H_
#define _VL53L3_PLATFORM_H_

#include "vl53l3_ll_def.h"
#include "vl53l3_platform_log.h"

#define VL53L3_IPP_API
#include "vl53l3_platform_ipp_imports.h"
#include "vl53l3_platform_user_data.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file   vl53l3_platform.h
 *
 * @brief  All end user OS/platform/application porting
 */



/**
 * @brief  Initialise platform comms.
 *
 * @param[in]   pdev            : pointer to device structure (device handle)
 * @param[in]   comms_type      : selects between I2C and SPI
 * @param[in]   comms_speed_khz : unsigned short containing the I2C speed in kHz
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_CommsInitialise(
	VL53L3_Dev_t *pdev,
	uint8_t       comms_type,
	uint16_t      comms_speed_khz);


/**
 * @brief  Close platform comms.
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_CommsClose(
	VL53L3_Dev_t *pdev);


/**
 * @brief Writes the supplied byte buffer to the device
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[in]   pdata     : pointer to uint8_t (byte) buffer containing the data
 *                          to be written
 * @param[in]   count     : number of bytes in the supplied byte buffer
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WriteMulti(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint8_t      *pdata,
		uint32_t      count);


/**
 * @brief  Reads the requested number of bytes from the device
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[out]  pdata     : pointer to the uint8_t (byte) buffer to store read
 *                          data
 * @param[in]   count     : number of bytes to read
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_ReadMulti(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint8_t      *pdata,
		uint32_t      count);


/**
 * @brief  Writes a single byte to the device
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[in]   data      : uint8_t data value to write
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WrByte(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint8_t       data);


/**
 * @brief  Writes a single word (16-bit unsigned) to the device
 *
 * Manages the big-endian nature of the device register map
 * (first byte written is the MS byte).
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[in]   data      : uin16_t data value write
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WrWord(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint16_t      data);


/**
 * @brief  Writes a single dword (32-bit unsigned) to the device
 *
 * Manages the big-endian nature of the device register map
 * (first byte written is the MS byte).
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[in]   data      : uint32_t data value to write
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WrDWord(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint32_t      data);



/**
 * @brief  Reads a single byte from the device
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index
 * @param[out]  pdata     : pointer to uint8_t data value
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 *
 */

VL53L3_Error VL53L3_RdByte(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint8_t      *pdata);


/**
 * @brief  Reads a single word (16-bit unsigned) from the device
 *
 * Manages the big-endian nature of the device (first byte read is the MS byte).
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[out]  pdata     : pointer to uint16_t data value
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_RdWord(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint16_t     *pdata);


/**
 * @brief  Reads a single dword (32-bit unsigned) from the device
 *
 * Manages the big-endian nature of the device (first byte read is the MS byte).
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   index     : uint16_t register index value
 * @param[out]  pdata     : pointer to uint32_t data value
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_RdDWord(
		VL53L3_Dev_t *pdev,
		uint16_t      index,
		uint32_t     *pdata);



/**
 * @brief  Implements a programmable wait in us
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   wait_us   : integer wait in micro seconds
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WaitUs(
		VL53L3_Dev_t *pdev,
		int32_t       wait_us);


/**
 * @brief  Implements a programmable wait in ms
 *
 * @param[in]   pdev      : pointer to device structure (device handle)
 * @param[in]   wait_ms   : integer wait in milliseconds
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WaitMs(
		VL53L3_Dev_t *pdev,
		int32_t       wait_ms);


/**
* @brief Get the frequency of the timer used for ranging results time stamps
*
* @param[out] ptimer_freq_hz : pointer for timer frequency
*
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
*/

VL53L3_Error VL53L3_GetTimerFrequency(int32_t *ptimer_freq_hz);

/**
* @brief Get the timer value in units of timer_freq_hz (see
*        VL53L3_get_timestamp_frequency())
*
* @param[out] ptimer_count : pointer for timer count value
*
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
*/

VL53L3_Error VL53L3_GetTimerValue(int32_t *ptimer_count);


/**
 * @brief Set the mode of a specified GPIO pin
 *
 * @param  pin - an identifier specifying the pin being modified - defined per
 *         platform
 *
 * @param  mode - an identifier specifying the requested mode - defined per
 *         platform
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GpioSetMode(uint8_t pin, uint8_t mode);


/**
 * @brief Set the value of a specified GPIO pin
 *
 * @param  pin - an identifier specifying the pin being modified - defined per
 *         platform
 *
 * @param  value - a value to set on the GPIO pin - typically 0 or 1
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GpioSetValue(uint8_t pin, uint8_t value);


/**
 * @brief Get the value of a specified GPIO pin
 *
 * @param  pin - an identifier specifying the pin being modified - defined per
 *         platform
 *
 * @param  pvalue - a value retrieved from the GPIO pin - typically 0 or 1
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GpioGetValue(uint8_t pin, uint8_t *pvalue);


/**
 * @brief Sets and clears the XShutdown pin on the Ewok
 *
 * @param  value - the value for xshutdown - 0 = in reset, 1 = operational
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GpioXshutdown(uint8_t value);


/**
 * @brief Sets and clears the Comms Mode pin (NCS) on the Ewok
 *
 * @param  value - the value for comms select - 0 = I2C, 1 = SPI
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GpioCommsSelect(uint8_t value);


/**
 * @brief Enables and disables the power to the Ewok module
 *
 * @param  value - the state of the power supply - 0 = power off, 1 = power on
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GpioPowerEnable(uint8_t value);

/**
 * @brief Enables callbacks to the supplied funtion pointer when Ewok interrupts
 *        ocurr
 *
 * @param  function - a function callback supplies by the caller, for interrupt
 *         notification
 * @param  edge_type - falling edge or rising edge interrupt detection
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error  VL53L3_GpioInterruptEnable(void (*function)(void),
	uint8_t edge_type);


/**
 * @brief Disables the callback on Ewok interrupts
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error  VL53L3_GpioInterruptDisable(void);


/*
 * @brief Gets current system tick count in [ms]
 *
 * @param[in]   pdev          : pointer to device structure (device handle)
 * @return  time_ms : current time in [ms]
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GetTickCount(
	VL53L3_Dev_t *pdev,
	uint32_t *ptime_ms);


/**
 * @brief Register "wait for value" polling routine
 *
 * Port of the V2WReg Script function  WaitValueMaskEx()
 *
 * @param[in]   pdev          : pointer to device structure (device handle)
 * @param[in]   timeout_ms    : timeout in [ms]
 * @param[in]   index         : uint16_t register index value
 * @param[in]   value         : value to wait for
 * @param[in]   mask          : mask to be applied before comparison with value
 * @param[in]   poll_delay_ms : polling delay been each read transaction in [ms]
 *
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_WaitValueMaskEx(
		VL53L3_Dev_t *pdev,
		uint32_t      timeout_ms,
		uint16_t      index,
		uint8_t       value,
		uint8_t       mask,
		uint32_t      poll_delay_ms);

#ifdef __cplusplus
}
#endif

#endif

