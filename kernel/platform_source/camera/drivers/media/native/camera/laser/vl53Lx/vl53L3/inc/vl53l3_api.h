
/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/******************************************************************************
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved

 This file is part of VL53L3 and is dual licensed,
 either GPL-2.0+
 or 'BSD 3-clause "New" or "Revised" License' , at your option.
 ******************************************************************************
 */

#ifndef _VL53L3_API_H_
#define _VL53L3_API_H_

#include "vl53l3_api_strings.h"
#include "vl53l3_api_core.h"
#include "vl53l3_preset_setup.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(VL53L3DevDataGet)
#warning "PALDevDataGet is deprecated define VL53L3DevDataGet instead"
#define VL53L3DevDataGet(Dev, field) (Dev->Data.field)
#endif

#if !defined(VL53L3DevDataSet)
#warning "PALDevDataSet is deprecated define VL53L3DevDataSet instead"
#define VL53L3DevDataSet(Dev, field, data) ((Dev->Data.field) = (data))
#endif

/** @defgroup VL53L3_cut11_group VL53L3 cut1.1 Function Definition
 *  @brief    VL53L3 cut1.1 Function Definition
 *  @{
 */

/** @defgroup VL53L3_general_group VL53L3 General Functions
 *  @brief    General functions and definitions
 *  @{
 */

/**
 * @brief Return the VL53L3 driver Version
 *
 * @note This function doesn't access to the device
 *
 * @param   pVersion              Rer to current driver Version
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetVersion(VL53L3_Version_t *pVersion);

/**
 * @brief Reads the Product Revision for a for given Device
 * This function can be used to distinguish cut1.0 from cut1.1.
 *
 * @param   Dev                 Device Handle
 * @param   pProductRevisionMajor  Pointer to Product Revision Major
 * for a given Device
 * @param   pProductRevisionMinor  Pointer to Product Revision Minor
 * for a given Device
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetProductRevision(VL53L3_DEV Dev,
	uint8_t *pProductRevisionMajor, uint8_t *pProductRevisionMinor);

/**
 * @brief Reads the Device information for given Device
 *
 * @note This function Access to the device
 *
 * @param   Dev                 Device Handle
 * @param   pVL53L3_DeviceInfo  Pointer to current device info for a given
 *  Device
 * @return  VL53L3_ERROR_NONE   Success
 * @return  "Other error code"  See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetDeviceInfo(VL53L3_DEV Dev,
	VL53L3_DeviceInfo_t *pVL53L3_DeviceInfo);

/**
 * @brief Reads the Device unique identifier
 *
 * @note This function Access to the device
 *
 * @param   Dev                 Device Handle
 * @param   pUid                Pointer to current device unique ID
 * @return  VL53L3_ERROR_NONE   Success
 * @return  "Other error code"  See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetUID(VL53L3_DEV Dev, uint64_t *pUid);

/**
 * @brief Human readable Range Status string for a given RangeStatus
 *
 * @note This function doesn't access to the device
 *
 * @param   RangeStatus         The RangeStatus code as stored on
 * @a VL53L3_RangingMeasurementData_t
 * @param   pRangeStatusString  The returned RangeStatus string. Shall be
 * defined as char buf[VL53L3_MAX_STRING_LENGTH]
 * @return  VL53L3_ERROR_NONE   Success
 * @return  "Other error code"  See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetRangeStatusString(uint8_t RangeStatus,
	char *pRangeStatusString);

/**
 * @brief Human readable error string for driver error status
 *
 * @note This function doesn't access to the device
 *
 * @param   PalErrorCode       The error code as stored on @a VL53L3_Error
 * @param   pPalErrorString    The error string corresponding to the
 * PalErrorCode. Shall be defined as char buf[VL53L3_MAX_STRING_LENGTH]
 * @return  VL53L3_ERROR_NONE  Success
 * @return  "Other error code" See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetPalErrorString(VL53L3_Error PalErrorCode,
	char *pPalErrorString);

/**
 * @brief Human readable driver State string
 *
 * @note This function doesn't access to the device
 *
 * @param   PalStateCode          The State code as stored on @a VL53L3_State
 * @param   pPalStateString       The State string corresponding to the
 * PalStateCode. Shall be defined as char buf[VL53L3_MAX_STRING_LENGTH]
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetPalStateString(VL53L3_State PalStateCode,
	char *pPalStateString);

/**
 * @brief Reads the internal state of the driver for a given Device
 *
 * @note This function doesn't access to the device
 *
 * @param   Dev                   Device Handle
 * @param   pPalState             Pointer to current state of the PAL for a
 * given Device
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetPalState(VL53L3_DEV Dev,
	VL53L3_State *pPalState);



/** @} VL53L3_general_group */

/** @defgroup VL53L3_init_group VL53L3 Init Functions
 *  @brief    VL53L3 Init Functions
 *  @{
 */

/**
 * @brief Set new device address
 *
 * After completion the device will answer to the new address programmed.
 * This function should be called when several devices are used in parallel
 * before start programming the sensor.
 * When a single device us used, there is no need to call this function.
 *
 * When it is requested for multi devices system this function MUST be called
 * prior to VL53L3_DataInit()
 *
 * @note This function Access to the device
 *
 * @param   Dev                   Device Handle
 * @param   DeviceAddress         The new Device address
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetDeviceAddress(VL53L3_DEV Dev,
	uint8_t DeviceAddress);

/**
 *
 * @brief One time device initialization
 *
 * To be called after device has been powered on and booted
 * see @a VL53L3_WaitDeviceBooted()
 *
 * @par Function Description
 * When not used after a fresh device "power up", it may return
 * @a #VL53L3_ERROR_CALIBRATION_WARNING meaning wrong calibration data
 * may have been fetched from device that can result in ranging offset error\n
 * If VL53L3_DataInit is called several times then the application must restore
 * calibration calling @a VL53L3_SetOffsetCalibrationData()
 * It implies application has gathered calibration data thanks to
 * @a VL53L3_GetOffsetCalibrationData() after an initial calibration stage.
 * This function will change the VL53L3_State from VL53L3_STATE_POWERDOWN to
 * VL53L3_STATE_WAIT_STATICINIT.
 *
 * @note This function Access to the device
 *
 * @param   Dev                   Device Handle
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_DataInit(VL53L3_DEV Dev);


/**
 * @brief Do basic device init (and eventually patch loading)
 * This function will change the VL53L3_State from
 * VL53L3_STATE_WAIT_STATICINIT to VL53L3_STATE_IDLE.
 * In this stage all default setting will be applied.
 *
 * @note This function Access to the device
 *
 * @param   Dev                   Device Handle
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_StaticInit(VL53L3_DEV Dev);

/**
 * @brief Wait for device booted after chip enable (hardware standby)
 * This function can be run only when VL53L3_State is VL53L3_STATE_POWERDOWN.
 *
 * @param   Dev                   Device Handle
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 *
 */
VL53L3_Error VL53L3_WaitDeviceBooted(VL53L3_DEV Dev);


/** @} VL53L3_init_group */

/** @defgroup VL53L3_parameters_group VL53L3 Parameters Functions
 *  @brief    Functions used to prepare and setup the device
 *  @{
 */

/**
 * @brief  Set a new Preset Mode
 * @par Function Description
 * Set device to a new Operating Mode (High speed ranging, Multi objects ...)
 *
 * @note This function doesn't Access to the device
 *
 * @warning This function change the timing budget to 16 ms and the inter-
 * measurement period to 1000 ms. Also the VL53L3_DISTANCEMODE_LONG is used.
 *
 * @param   Dev                   Device Handle
 * @param   PresetMode            New Preset mode to apply
 * <br>Valid values are:
 */
/**
 * @li VL53L3_PRESETMODE_MULTIZONES_SCANNING
 * @li VL53L3_PRESETMODE_RANGING
 * @li VL53L3_PRESETMODE_AUTONOMOUS
 * @li VL53L3_PRESETMODE_LOWPOWER_AUTONOMOUS
 * @li VL53L3_PRESETMODE_LITE_RANGING
 * @li VL53L3_PRESETMODE_OLT
 */
/**
 *
 * @return  VL53L3_ERROR_NONE               Success
 * @return  VL53L3_ERROR_MODE_NOT_SUPPORTED This error occurs when PresetMode is
 *                                          not in the supported list
 */
VL53L3_Error VL53L3_SetPresetMode(VL53L3_DEV Dev,
		VL53L3_PresetModes PresetMode);

/**
 * @brief  Get current Preset Mode
 * @par Function Description
 * Get actual mode of the device(ranging, histogram ...)
 *
 * @note This function doesn't Access to the device
 *
 * @param   Dev                   Device Handle
 * @param   pPresetMode           Pointer to current apply mode value
 *
 * @return  VL53L3_ERROR_NONE                   Success
 * @return  VL53L3_ERROR_MODE_NOT_SUPPORTED     This error occurs when
 * DeviceMode is not in the supported list
 */
VL53L3_Error VL53L3_GetPresetMode(VL53L3_DEV Dev,
		VL53L3_PresetModes *pPresetMode);


/**
 * @brief  Set the distance mode
 * @par Function Description
 * Set the distance mode to be used for the next ranging.<br>
 * The modes Short, Medium and Long are used to optimize the ranging accuracy
 * in a specific range of distance.<br> The user select one of these modes to
 * select the distance range. <br>
 * Two additional modes are supported: AUTO and AUTO_LITE the difference between
 * these modes is the following.<br>
 * The mode AUTO take into account both the ranging distance (RangeMilliMeter)
 * and the dmax distance (DmaxMilliMeter).<br> The algorithm uses the ranging
 * distance when the range status is ok and uses the dmax distance when the
 * range status is not ok.<br>
 * The AUTO_LITE take into account only the ranging distance, so nothing is done
 * in case of range error i.e. the distance mode will not be changed.
 * @note This function doesn't Access to the device
 *
 * @warning This function should be called after @a VL53L3_SetPresetMode().

 * @param   Dev                   Device Handle
 * @param   DistanceMode          Distance mode to apply valid values are:
 * @li VL53L3_DISTANCEMODE_SHORT
 * @li VL53L3_DISTANCEMODE_MEDIUM
 * @li VL53L3_DISTANCEMODE_LONG
 * @li VL53L3_DISTANCEMODE_AUTO_LITE
 * @li VL53L3_DISTANCEMODE_AUTO
 * @return  VL53L3_ERROR_NONE               Success
 * @return  VL53L3_ERROR_MODE_NOT_SUPPORTED This error occurs when DistanceMode
 *                                          is not in the supported list
 * @return  "Other error code"              See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetDistanceMode(VL53L3_DEV Dev,
		VL53L3_DistanceModes DistanceMode);

/**
 * @brief  Get the distance mode
 * @par Function Description
 * Get the distance mode used for the next ranging.
 *
 * @param   Dev                   Device Handle
 * @param   *pDistanceMode        Pointer to Distance mode
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetDistanceMode(VL53L3_DEV Dev,
		VL53L3_DistanceModes *pDistanceMode);


/**
 * @brief  Set the output mode
 * @par Function Description
 * Set the output mode to be used for the next ranging. The output mode is used
 * to select, in case of multiple objects, which one will be used in
 * function @a VL53L3_GetRangingMeasurementData().
 * VL53L3_SetOutputMode also sets the object used by automatic
 * distance mode algorithm when @a VL53L3_SetDistanceMode() is
 * set to automatic mode.
 *
 * @note This function doesn't Access to the device
 *
 * @warning This function should be called after @a VL53L3_SetPresetMode().

 * @param   Dev                   Device Handle
 * @param   OutputMode            Output mode to apply valid values are:
 * @li VL53L3_OUTPUTMODE_NEAREST
 * @li VL53L3_OUTPUTMODE_STRONGEST
 *
 * @return  VL53L3_ERROR_NONE               Success
 * @return  VL53L3_ERROR_MODE_NOT_SUPPORTED This error occurs when OutputMode
 *                                          is not in the supported list
 * @return  "Other error code"              See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetOutputMode(VL53L3_DEV Dev,
		VL53L3_OutputModes OutputMode);

/**
 * @brief  Get the output mode
 * @par Function Description
 * Get the output mode used for the next ranging.
 *
 * @param   Dev                   Device Handle
 * @param   *pOutputMode          Pointer to Output mode
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetOutputMode(VL53L3_DEV Dev,
		VL53L3_OutputModes *pOutputMode);


/**
 * @brief Set Ranging Timing Budget in microseconds
 *
 * @par Function Description
 * Defines the maximum time allowed by the user to the device to run a
 * full ranging sequence for the current mode (ranging, histogram, ASL ...)
 *
 * @param   Dev                                Device Handle
 * @param MeasurementTimingBudgetMicroSeconds  Max measurement time in
 * microseconds.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  VL53L3_ERROR_INVALID_PARAMS  Error timing parameter not
 *                                       supported.
 *                                       The maximum accepted value for the
 *                                       computed timing budget is 10 seconds
 *                                       the minimum value depends on the preset
 *                                       mode selected.
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetMeasurementTimingBudgetMicroSeconds(
	VL53L3_DEV Dev, uint32_t MeasurementTimingBudgetMicroSeconds);

/**
 * @brief Get Ranging Timing Budget in microseconds
 *
 * @par Function Description
 * Returns the programmed the maximum time allowed by the user to the
 * device to run a full ranging sequence for the current mode
 * (ranging, histogram, ASL ...)
 *
 * @param   Dev                                    Device Handle
 * @param   pMeasurementTimingBudgetMicroSeconds   Max measurement time in
 * microseconds.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetMeasurementTimingBudgetMicroSeconds(
	VL53L3_DEV Dev, uint32_t *pMeasurementTimingBudgetMicroSeconds);


/**
 * Program continuous mode Inter-Measurement period in milliseconds
 *
 * @par Function Description
 * When trying to set too short time return  INVALID_PARAMS minimal value
 *
 * @param   Dev                                  Device Handle
 * @param   InterMeasurementPeriodMilliSeconds   Inter-Measurement Period in ms.
 *  this value should be greater than the duration set in
 *  @a VL53L3_SetMeasurementTimingBudgetMicroSeconds() to ensure smooth ranging
 *  operation.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetInterMeasurementPeriodMilliSeconds(
	VL53L3_DEV Dev, uint32_t InterMeasurementPeriodMilliSeconds);

/**
 * Get continuous mode Inter-Measurement period in milliseconds
 *
 * @par Function Description
 *
 * @param   Dev                                  Device Handle
 * @param   pInterMeasurementPeriodMilliSeconds  Pointer to programmed
 *  Inter-Measurement Period in milliseconds.
 * @return  VL53L3_ERROR_NONE
 */
VL53L3_Error VL53L3_GetInterMeasurementPeriodMilliSeconds(
	VL53L3_DEV Dev, uint32_t *pInterMeasurementPeriodMilliSeconds);

/**
 * @brief  target reflectance for Dmax setting
 * @par Function Description
 * Allow user to set the value for target reflectance @ 940nm to calculate the
 * ambient DMAX values for. Set to 50% by default by @a VL53L3_DataInit()
 *
 * @param   Dev                   Device Handle
 * @param   DmaxReflectance       Reflectance % in 16.16 fixed point
 * @return  VL53L3_ERROR_NONE     Success
 * @return  VL53L3_ERROR_INVALID_PARAMS     in case input value is not in range
 * from 0 to 100. Note that this is a fix point value so the max value is
 * 100 * 65536.
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetDmaxReflectance(VL53L3_DEV Dev,
		FixPoint1616_t DmaxReflectance);

/**
 * @brief  Get target reflectance for Dmax
 * @par Function Description
 * Retrieves the value for target reflectance @ 940nm to calculate the
 * ambient DMAX values for. Set to 50% by default by @a VL53L3_DataInit()
 *
 * @param   Dev                   Device Handle
 * @param   pDmaxReflectance      pointer to Reflectance % in 16.16 fixed point
 * @return  VL53L3_ERROR_NONE     Success
 * @return  "Other error code"    See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetDmaxReflectance(VL53L3_DEV Dev,
		FixPoint1616_t *pDmaxReflectance);
/**
 * @brief Set function for ambient Dmax mode
 *
 *
 * @param    Dev                  Device Handle
 * @param    DmaxMode             DMAX mode to be used in ranging
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */


VL53L3_Error VL53L3_SetDmaxMode(VL53L3_DEV Dev,
		VL53L3_DeviceDmaxModes DmaxMode);

/**
 * @brief Get function for ambient Dmax mode
 *
 * @param	Dev              Device Handle
 * @param	pDmaxMode        output pointer to DMAX mode currently in use
 *
 * @return   VL53L3_ERROR_NONE    Success
 * @return  "Other error code"    See ::VL53L3_Error
 */

VL53L3_Error VL53L3_GetDmaxMode(VL53L3_DEV Dev,
	VL53L3_DeviceDmaxModes *pDmaxMode);

/** @} VL53L3_parameters_group */


/** @defgroup VL53L3_limitcheck_group VL53L3 Limit Check Functions
 *  @brief    Functions used for the Limit checks
 *  @{
 */



/**
 * @brief  Get the number of the check limit managed by a given Device
 *
 * @par Function Description
 * This function give the number of the check limit managed by the Device
 *
 * @param   pNumberOfLimitCheck           Pointer to the number of check limit.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetNumberOfLimitCheck(
	uint16_t *pNumberOfLimitCheck);

/**
 * @brief  Return a description string for a given limit check number
 *
 * @par Function Description
 * This function returns a description string for a given limit check number.
 * The limit check is identified with the LimitCheckId.
 *
 * @param   LimitCheckId                  Limit Check ID
 * (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   pLimitCheckString             Pointer to the description string of
 * the given check limit. Shall be defined as char buf[VL53L3_MAX_STRING_LENGTH]
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetLimitCheckInfo(uint16_t LimitCheckId,
	char *pLimitCheckString);

/**
 * @brief  Return a the Status of the specified check limit
 *
 * @par Function Description
 * This function returns the Status of the specified check limit.
 * The value indicate if the check is fail or not.
 * The limit check is identified with the LimitCheckId.
 *
 * @param   Dev                           Device Handle
 * @param   LimitCheckId                  Limit Check ID
 (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   pLimitCheckStatus             Pointer to the
 Limit Check Status of the given check limit.
 * LimitCheckStatus :
 * 0 the check is not fail or not enabled
 * 1 the check if fail
 *
 * <p><ul>
 *    <li>VL53L3_CHECKENABLE_SIGMA_FINAL_RANGE: the sigma indicate the quality
 *    of the measure. The more it is little the better it is.
 *    The status is 1 when current sigma is greater then the limit.</li>
 *    <li>VL53L3_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE: the signal rate indicate
 *    the strength of the returned signal. The more it is big the better it is.
 *    The status is 1 when current signal is lower then the limit.</li>
 * </ul></p>
 *
 *
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetLimitCheckStatus(VL53L3_DEV Dev,
	uint16_t LimitCheckId, uint8_t *pLimitCheckStatus);

/**
 * @brief  Enable/Disable a specific limit check
 *
 * @par Function Description
 * This function Enable/Disable a specific limit check.
 * The limit check is identified with the LimitCheckId.
 *
 * @note This function doesn't Access to the device
 *
 * @param   Dev                           Device Handle
 * @param   LimitCheckId                  Limit Check ID
 *  (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   LimitCheckEnable
 * @li set LimitCheckEnable=1 enables the LimitCheckId limit
 * @li set LimitCheckEnable=0 disables the LimitCheckId limit
 * @return  VL53L3_ERROR_NONE            Success
 * @return  VL53L3_ERROR_INVALID_PARAMS   This error is returned
 *  when LimitCheckId value is out of range.
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetLimitCheckEnable(VL53L3_DEV Dev,
	uint16_t LimitCheckId, uint8_t LimitCheckEnable);

/**
 * @brief  Get specific limit check enable state
 *
 * @par Function Description
 * This function get the enable state of a specific limit check.
 * The limit check is identified with the LimitCheckId.
 *
 * @note This function Access to the device
 *
 * @param   Dev                           Device Handle
 * @param   LimitCheckId                  Limit Check ID
 *  (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   pLimitCheckEnable             Pointer to the check limit enable
 * value.
 * @li if 1 the check limit corresponding to LimitCheckId is Enabled
 * @li if 0 the check limit corresponding to LimitCheckId is disabled
 * @return  VL53L3_ERROR_NONE            Success
 * @return  VL53L3_ERROR_INVALID_PARAMS   This error is returned
 *  when LimitCheckId value is out of range.
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetLimitCheckEnable(VL53L3_DEV Dev,
	uint16_t LimitCheckId, uint8_t *pLimitCheckEnable);

/**
 * @brief  Set a specific limit check value
 *
 * @par Function Description
 * This function set a specific limit check value.
 * The limit check is identified with the LimitCheckId.
 *
 * @note Note that the value written with that function will not be applied if
 * the limit is not enabled. In other words this function will not enable the
 * limit but change only the value. In case the limit is not enabled the value
 * is saved internally and applied with VL53L3_SetLimitCheckEnable.
 *
 * @param   Dev                           Device Handle
 * @param   LimitCheckId                  Limit Check ID
 *  (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   LimitCheckValue               Limit check Value for a given
 * LimitCheckId
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetLimitCheckValue(VL53L3_DEV Dev,
	uint16_t LimitCheckId, FixPoint1616_t LimitCheckValue);

/**
 * @brief  Get a specific limit check value
 *
 * @par Function Description
 * This function get a specific limit check value from device then it updates
 * internal values and check enables.
 * The limit check is identified with the LimitCheckId.
 *
 * @note This function get the current value from device if zero then the value
 * returned is the one stored by the user, but in that case the check is store
 * as disabled. If the value from device is not zero, this is returned and set
 * into the memory at the same way that user call VL53L3_SetLimitCheckValue()
 *
 * @param   Dev                           Device Handle
 * @param   LimitCheckId                  Limit Check ID
 *  (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   pLimitCheckValue              Pointer to Limit
 *  check Value for a given LimitCheckId.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetLimitCheckValue(VL53L3_DEV Dev,
	uint16_t LimitCheckId, FixPoint1616_t *pLimitCheckValue);

/**
 * @brief  Get the current value of the signal used for the limit check
 *
 * @par Function Description
 * This function get a the current value of the signal used for the limit check.
 * To obtain the latest value you should run a valid ranging before.
 * The value reported is linked to the limit check identified with the
 * LimitCheckId.
 *
 * @param   Dev                           Device Handle
 * @param   LimitCheckId                  Limit Check ID
 *  (0<= LimitCheckId < VL53L3_GetNumberOfLimitCheck() ).
 * @param   pLimitCheckCurrent            Pointer to current Value for a
 * given LimitCheckId.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetLimitCheckCurrent(VL53L3_DEV Dev,
	uint16_t LimitCheckId, FixPoint1616_t *pLimitCheckCurrent);

/** @} VL53L3_limitcheck_group */



/** @defgroup VL53L3_ROI_group VL53L3 ROI Functions
 *  @brief    Functions used to select ROIs
 *  @{
 */

/**
 * @brief Get the Maximum number of ROI Zones managed by the Device
 *
 * @par Function Description
 * Get Maximum number of ROI Zones managed by the Device.
 *
 * @note The number of Zone depends on the preset mode used so to have the
 * right number this function should be call after @a VL53L3_SetPresetMode()
 * @note This function doesn't Access to the device
 *
 * @param   Dev                    Device Handle
 * @param   pMaxNumberOfROI   Pointer to the Maximum Number
 *  of ROI Zones value.
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetMaxNumberOfROI(VL53L3_DEV Dev,
	uint8_t *pMaxNumberOfROI);
/**
 * @brief Set the ROI  to be used for ranging
 *
 * @par Function Description
 * The user defined ROIs are rectangles described as per the following system
 * from the Top Left corner to the Bottom Right corner.
 * <br>Minimal ROI size is 4x4 spads
 * @image html roi_coord.png
 *
 * @param   Dev                      Device Handle
 * @param   pRoiConfig               Pointer to the Structure containing all the
 * ROI to be used.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetROI(VL53L3_DEV Dev,
		VL53L3_RoiConfig_t *pRoiConfig);

/**
 * @brief Get the ROI managed by the Device
 *
 * @par Function Description
 * Get the ROI managed by the Device
 *
 * @param   Dev                   Device Handle
 * @param   pRoiConfig            Pointer to the Structure containing all the
 * ROI to be used.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetROI(VL53L3_DEV Dev,
		VL53L3_RoiConfig_t *pRoiConfig);

/** @} VL53L3_ROI_group */

/* \internal */
/** @defgroup VL53L3_sequencestep_group VL53L3 Sequence Step Functions
 *  @brief    Functions used to select Steps done on each ranging
 *  @{
 */

/**
 * @brief Gets number of sequence steps managed by the API.
 *
 * @par Function Description
 * This function retrieves the number of sequence steps currently managed
 * by the API
 *
 * @note This function Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   pNumberOfSequenceSteps       Out parameter reporting the number of
 *                                       sequence steps.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetNumberOfSequenceSteps(VL53L3_DEV Dev,
	uint8_t *pNumberOfSequenceSteps);

/**
 * @brief Gets the name of a given sequence step.
 *
 * @par Function Description
 * This function retrieves the name of sequence steps corresponding to
 * SequenceStepId.
 *
 * @note This function doesn't Accesses the device
 *
 * @param   SequenceStepId               Sequence step identifier.
 * @param   pSequenceStepsString         Pointer to Info string. Shall be
 * defined as char buf[VL53L3_MAX_STRING_LENGTH]
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetSequenceStepsInfo(
	VL53L3_SequenceStepId SequenceStepId, char *pSequenceStepsString);



/**
 * @brief Sets the (on/off) state of a requested sequence step.
 *
 * @par Function Description
 * This function enables/disables a requested sequence step.
 *
 * @note This function Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   SequenceStepId	         Sequence step identifier.
 * @param   SequenceStepEnabled          Demanded state {0=Off,1=On}
 *                                       is enabled.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  VL53L3_ERROR_INVALID_PARAMS  Error SequenceStepId parameter not
 *                                       supported.
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetSequenceStepEnable(VL53L3_DEV Dev,
	VL53L3_SequenceStepId SequenceStepId, uint8_t SequenceStepEnabled);

/**
 * @brief Gets the (on/off) state of a requested sequence step.
 *
 * @par Function Description
 * This function retrieves the state of a requested sequence step, i.e. on/off.
 *
 * @note This function Accesses the device
 *
 * @param   Dev                    Device Handle
 * @param   SequenceStepId         Sequence step identifier.
 * @param   pSequenceStepEnabled   Out parameter reporting if the sequence step
 *                                 is enabled {0=Off,1=On}.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  VL53L3_ERROR_INVALID_PARAMS  Error SequenceStepId parameter not
 *                                       supported.
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetSequenceStepEnable(VL53L3_DEV Dev,
	VL53L3_SequenceStepId SequenceStepId, uint8_t *pSequenceStepEnabled);


/** @} VL53L3_sequencestep_group */
/* \endinternal */



/** @defgroup VL53L3_measurement_group VL53L3 Measurement Functions
 *  @brief    Functions used for the measurements
 *  @{
 */

/**
 * @brief Start device measurement
 *
 * @details Started measurement will depend on preset parameters set through
 * @a VL53L3_SetPreseMode()
 * This function will change the VL53L3_State from VL53L3_STATE_IDLE to
 * VL53L3_STATE_RUNNING.
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @return  VL53L3_ERROR_NONE                  Success
 * @return  VL53L3_ERROR_MODE_NOT_SUPPORTED    This error occurs when
 * PresetMode programmed with @a VL53L3_SetPresetMode
 * @return  VL53L3_ERROR_TIME_OUT    Time out on start measurement
 * @return  VL53L3_ERROR_INVALID_PARAMS This error might occur in timed mode
 * when inter measurement period is smaller or too close to the timing budget.
 * In such case measurements are not started and user must correct the timings
 * passed to @a VL53L3_SetMeasurementTimingBudgetMicroSeconds() and
 * @a VL53L3_SetInterMeasurementPeriodMilliSeconds() functions.
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_StartMeasurement(VL53L3_DEV Dev);

/**
 * @brief Stop device measurement
 *
 * @details Will set the device in standby mode at end of current measurement\n
 *          Not necessary in single mode as device shall return automatically
 *          in standby mode at end of measurement.
 *          This function will change the VL53L3_State from VL53L3_STATE_RUNNING
 *          to VL53L3_STATE_IDLE.
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @return  VL53L3_ERROR_NONE    Success
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_StopMeasurement(VL53L3_DEV Dev);

/**
 * @brief Clear the Interrupt flag and start new measurement
 * *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @return  VL53L3_ERROR_NONE    Success
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_ClearInterruptAndStartMeasurement(VL53L3_DEV Dev);

/**
 * @brief Return Measurement Data Ready
 *
 * @par Function Description
 * This function indicate that a measurement data is ready.
 * This function is used for non-blocking capture.
 *
 * @note This function Access to the device
 *
 * @param   Dev                    Device Handle
 * @param   pMeasurementDataReady  Pointer to Measurement Data Ready.
 * 0 = data not ready, 1 = data ready
 * @return  VL53L3_ERROR_NONE      Success
 * @return  "Other error code"     See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetMeasurementDataReady(VL53L3_DEV Dev,
	uint8_t *pMeasurementDataReady);

/**
 * @brief Wait for measurement data ready.
 * Blocking function.
 * Note that the timeout is given by:
 * VL53L3_RANGE_COMPLETION_POLLING_TIMEOUT_MS defined in def.h
 *
 *
 * @note This function Access to the device
 *
 * @param   Dev      Device Handle
 * @return  VL53L3_ERROR_NONE        Success
 * @return  VL53L3_ERROR_TIME_OUT In case of timeout
 */
VL53L3_Error VL53L3_WaitMeasurementDataReady(VL53L3_DEV Dev);


/**
 * @brief Retrieve the measurements from device for a given setup
 *
 * @par Function Description
 * Get data from last successful Ranging measurement
 */
/**
 * @warning this function will return only the first ROI data and only the
 * first object. For multi objects or multi ROI use:
 * @a Vl53L3_GetMultiRangingData.
 * In case of RANGING only one output is given, this can
 * be selected with the help of @a VL53L3_SetOutputMode()
 * In case of MULTIZONES_SCANNING and error will be raised because not
 * supported in that function.
 */
/**
 *
 * @warning USER must call @a VL53L3_ClearInterruptAndStartMeasurement() prior
 * to call again this function
 *
 * @note This function Access to the device
 *
 * @note The first valid value returned by this function will have a range
 * status equal to VL53L3_RANGESTATUS_RANGE_VALID_NO_WRAP_CHECK which means that
 * the data is valid but no wrap around check have been done. User should take
 * care about that.
 *
 * @param   Dev                      Device Handle
 * @param   pRangingMeasurementData  Pointer to the data structure to fill up.
 * @return  VL53L3_ERROR_NONE        Success
 * @return  VL53L3_ERROR_MODE_NOT_SUPPORTED    in case of MULTIZONES_SCANNING
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetRangingMeasurementData(VL53L3_DEV Dev,
	VL53L3_RangingMeasurementData_t *pRangingMeasurementData);

/**
 * @brief Retrieve all ROI's measurements from device for a given setup
 *
 * @par Function Description
 * Get data from last successful Ranging measurement
 * @warning USER should take care about  @a VL53L3_GetNumberOfROI()
 * before get data.
 * Bare driver will fill a NumberOfROI times the corresponding data
 * structure used in the measurement function.
 *
 * @warning USER must call @a VL53L3_ClearInterruptAndStartMeasurement() prior
 * to call again this function
 *
 * @note This function Access to the device
 *
 * @note The first valid value returned by this function will have a range
 * status equal to VL53L3_RANGESTATUS_RANGE_VALID_NO_WRAP_CHECK which means that
 * the data is valid but no wrap around check have been done. User should take
 * care about that.
 *
 * @param   Dev                      Device Handle
 * @param   pMultiRangingData        Pointer to the data structure to fill up.
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetMultiRangingData(VL53L3_DEV Dev,
		VL53L3_MultiRangingData_t *pMultiRangingData);

/**
 * @brief Get Additional Data
 *
 * @par Function Description
 * This function is used to get lld debugging data on the last histogram
 * measurement. shall be called when a new measurement is ready (interrupt or
 * positive VL53L3_GetMeasurementDataReady() polling) and before a call to
 * VL53L3_ClearInterruptAndStartMeasurement(). Depending on the PresetMode
 * currently set parts of the returned data structure may be not relevant.
 *
 * @param   Dev                      Device Handle
 * @param   pAdditionalData          Pointer to Additional data
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetAdditionalData(VL53L3_DEV Dev,
		VL53L3_AdditionalData_t *pAdditionalData);


/** @} VL53L3_measurement_group */

/** @defgroup VL53L3_Calibration_group VL53L3 Calibration Functions
 *  @brief    Functions used for Calibration
 *  @{
 */


/**
 * @brief Set Tuning Parameter value for a given parameter ID
 *
 * @par Function Description
 * This function is used to improve the performance of the device. It permit to
 * change a particular value used for a timeout or a threshold or a constant
 * in an algorithm. The function will change the value of the parameter
 * identified by an unique ID.
 *
 * @note This function doesn't Access to the device
 *
 * @param   Dev                          Device Handle
 * @param   TuningParameterId            Tuning Parameter ID
 * @param   TuningParameterValue         Tuning Parameter Value
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetTuningParameter(VL53L3_DEV Dev,
		uint16_t TuningParameterId, int32_t TuningParameterValue);

/**
 * @brief Get Tuning Parameter value for a given parameter ID
 *
 * @par Function Description
 * This function is used to get the value of the parameter
 * identified by an unique ID.
 *
 * @note This function doesn't Access to the device
 *
 * @param   Dev                          Device Handle
 * @param   TuningParameterId            Tuning Parameter ID
 * @param   pTuningParameterValue        Pointer to Tuning Parameter Value
 * for a given TuningParameterId.
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetTuningParameter(VL53L3_DEV Dev,
		uint16_t TuningParameterId, int32_t *pTuningParameterValue);

/**
 * @brief Performs Reference Spad Management
 *
 * @par Function Description
 * The reference SPAD initialization procedure determines the minimum amount
 * of reference spads to be enables to achieve a target reference signal rate
 * and should be performed once during initialization.
 *
 * @note This function Access to the device
 *
 * @param   Dev                          Device Handle
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_PerformRefSpadManagement(VL53L3_DEV Dev);

/**
 * @brief Enable/Disable dynamic Xtalk compensation feature
 *
 * Enable/Disable dynamic Xtalk compensation (aka smudge correction).
 *
 * @param   Dev    Device Handle
 * @param   Mode   Set the smudge correction mode
 * See ::VL53L3_SmudgeCorrectionModes
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SmudgeCorrectionEnable(VL53L3_DEV Dev,
		VL53L3_SmudgeCorrectionModes Mode);

/**
 * @brief Enable/Disable Cross talk compensation feature
 *
 * Enable/Disable Cross Talk correction.
 *
 * @param   Dev                       Device Handle
 * @param   XTalkCompensationEnable   Cross talk compensation
 *  to be set 0 = disabled or 1 = enabled.
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetXTalkCompensationEnable(VL53L3_DEV Dev,
uint8_t XTalkCompensationEnable);

/**
 * @brief Get Cross talk compensation rate enable
 *
 * Get if the Cross Talk is Enabled or Disabled.
 *
 * @note This function doesn't access to the device
 *
 * @param   Dev                        Device Handle
 * @param   pXTalkCompensationEnable   Pointer to the Cross talk compensation
 *  state 0=disabled or 1 = enabled
 * @return  VL53L3_ERROR_NONE        Success
 * @return  "Other error code"       See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetXTalkCompensationEnable(VL53L3_DEV Dev,
	uint8_t *pXTalkCompensationEnable);


/**
 * @brief Perform XTalk Calibration
 *
 * @details Perform a XTalk calibration of the Device.
 * This function will launch a ranging measurement, if interrupts
 * are enabled an interrupt will be done.
 * This function will clear the interrupt generated automatically.
 * This function will program a new value for the XTalk compensation
 * and it will enable the cross talk before exit.
 *
 * @warning This function is a blocking function
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @param   CalibrationOption    Select the Calibration to be run :
 * @param                        CalibrationOption
 * @li VL53L3_XTALKCALIBRATIONMODE_SINGLE_TARGET the calibration uses current
 * preset and distance mode without altering them.<br>
 * User must call @a VL53L3_SetPresetMode() with VL53L3_PRESETMODE_AUTONOMOUS,
 * VL53L3_PRESETMODE_LITE_RANGING or VL53L3_PRESETMODE_LOWPOWER_AUTONOMOUS
 * parameter prior to launch calibration
 * @li VL53L3_XTALKCALIBRATIONMODE_NO_TARGET the calibration sets appropriate
 * preset and distance mode and thus override existing ones<br>
 * User must call @a VL53L3_SetPresetMode() again after calibration to set the
 * desired one. during this calibration mode no object must be put below a 80cm
 * distance from the target
 * @li VL53L3_XTALKCALIBRATIONMODE_FULL_ROI the calibration sets appropriate
 * preset and distance mode and thus override existing ones<br>
 * User must call @a VL53L3_SetPresetMode() again after calibration to set the
 * desired one.
 * The ROI settings must define a single 16x16 ROI before to launch this
 * function.
 * The calibration uses a target which should be located at least @60cm from the
 * device. The actual location of the target shall be passed
 * through the bare driver tuning parameters table
 *
 * @return  VL53L3_ERROR_NONE    Success
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_PerformXTalkCalibration(VL53L3_DEV Dev,
		uint8_t CalibrationOption);

/**
 * @brief Define the mode to be used for the offset calibration
 *
 * Define the mode to be used for the offset calibration. This function should
 * be called before run the @a VL53L3_PerformOffsetCalibration()
 *
 * @param   Dev                       Device Handle
 * @param   OffsetCalibrationMode     Offset Calibration Mode valid values are:
 * @li                                VL53L3_OFFSETCALIBRATIONMODE_STANDARD
 * @li                                VL53L3_OFFSETCALIBRATIONMODE_PRERANGE_ONLY
 * @li                                VL53L3_OFFSETCALIBRATIONMODE_MULTI_ZONE
 *
 * @return  VL53L3_ERROR_NONE         Success
 * @return  "Other error code"        See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetOffsetCalibrationMode(VL53L3_DEV Dev,
		VL53L3_OffsetCalibrationModes OffsetCalibrationMode);

/**
 * @brief Define the mode to be used for the offset correction
 *
 * Define the mode to be used for the offset correction.
 *
 * @param   Dev                       Device Handle
 * @param   OffsetCorrectionMode      Offset Correction Mode valid values are:
 * @li                                VL53L3_OFFSETCORRECTIONMODE_STANDARD
 * @li                                VL53L3_OFFSETCORRECTIONMODE_PERZONE
 * @li                                VL53L3_OFFSETCORRECTIONMODE_PERVCSEL
 *
 * @return  VL53L3_ERROR_NONE         Success
 * @return  "Other error code"        See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetOffsetCorrectionMode(VL53L3_DEV Dev,
		VL53L3_OffsetCorrectionModes OffsetCorrectionMode);


/**
 * @brief Perform Offset Calibration
 *
 * @details Perform a Offset calibration of the Device.
 * This function will launch a ranging measurement, if interrupts are
 * enabled interrupts will be done.
 * This function will program a new value for the Offset calibration value
 *
 * @warning This function is a blocking function
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @param   CalDistanceMilliMeter     Calibration distance value used for the
 * offset compensation.
 * @param   CalReflectancePercent     Calibration Target reflectance @ 940nm
 * in percentage.
 *
 * @return  VL53L3_ERROR_NONE
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_PerformOffsetCalibration(VL53L3_DEV Dev,
	int32_t CalDistanceMilliMeter,
	FixPoint1616_t CalReflectancePercent);

/**
 * @brief Perform Offset simple Calibration
 *
 * @details Perform a very simple offset calibration of the Device.
 * This function will launch few ranging measurements and computes offset
 * calibration. The preset mode and the distance mode MUST be set by the
 * application before to call this function.
 *
 * @warning This function is a blocking function
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @param   CalDistanceMilliMeter     Calibration distance value used for the
 * offset compensation.
 *
 * @return  VL53L3_ERROR_NONE
 * @return  VL53L3_ERROR_OFFSET_CAL_NO_SAMPLE_FAIL the calibration failed by
 * lack of valid measurements
 * @return  VL53L3_WARNING_OFFSET_CAL_SIGMA_TOO_HIGH means that the target
 * distance combined to the number of loops performed in the calibration lead to
 * an internal overflow. Try to reduce the distance of the target (140 mm)
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_PerformOffsetSimpleCalibration(VL53L3_DEV Dev,
		int32_t CalDistanceMilliMeter);

/**
 * @brief Perform Offset simple Calibration with a "zero distance" target
 *
 * @details Perform a simple offset calibration of the Device.
 * This function will launch few ranging measurements and computes offset
 * calibration. The preset mode and the distance mode MUST be set by the
 * application before to call this function.
 * A target must be place very close to the device.
 * Ideally the target shall be touching the coverglass.
 *
 * @warning This function is a blocking function
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 *
 * @return  VL53L3_ERROR_NONE
 * @return  VL53L3_ERROR_OFFSET_CAL_NO_SAMPLE_FAIL the calibration failed by
 * lack of valid measurements
 * @return  VL53L3_WARNING_OFFSET_CAL_SIGMA_TOO_HIGH means that the target
 * distance is too large, try to put the target closer to the device
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_PerformOffsetZeroDistanceCalibration(VL53L3_DEV Dev);

/**
 * @brief Perform Offset per Vcsel Calibration. i.e. per distance mode
 *
 * @details Perform offset calibration of the Device depending on the
 * three distance mode settings: short, medium and long.
 * This function will launch few ranging measurements and computes offset
 * calibration in each of the three distance modes.
 * The preset mode MUST be set by the application before to call this function.
 *
 * @warning This function is a blocking function
 *
 * @note This function Access to the device
 *
 * @param   Dev                  Device Handle
 * @param   CalDistanceMilliMeter     Distance of the target used for the
 * offset compensation calibration.
 *
 * @return  VL53L3_ERROR_NONE
 * @return  VL53L3_ERROR_OFFSET_CAL_NO_SAMPLE_FAIL the calibration failed by
 * lack of valid measurements
 * @return  VL53L3_WARNING_OFFSET_CAL_SIGMA_TOO_HIGH means that the target
 * distance combined to the number of loops performed in the calibration lead to
 * an internal overflow. Try to reduce the distance of the target (140 mm)
 * @return  "Other error code"   See ::VL53L3_Error
 */
VL53L3_Error VL53L3_PerformOffsetPerVcselCalibration(VL53L3_DEV Dev,
	int32_t CalDistanceMilliMeter);
/**
 * @brief Sets the Calibration Data.
 *
 * @par Function Description
 * This function set all the Calibration Data issued from the functions
 * @a VL53L3_PerformRefSpadManagement(), @a VL53L3_PerformXTalkCalibration,
 * @a VL53L3_PerformOffsetCalibration()
 *
 * @note This function doesn't Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   *pCalibrationData            Pointer to Calibration data to be set.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  VL53L3_ERROR_INVALID_PARAMS  pCalibrationData points to an older
 * version of the inner structure. Need for support to convert its content.
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetCalibrationData(VL53L3_DEV Dev,
		VL53L3_CalibrationData_t *pCalibrationData);

/**
 * @brief Gets the Calibration Data.
 *
 * @par Function Description
 * This function get all the Calibration Data issued from the functions
 * @a VL53L3_PerformRefSpadManagement(), @a VL53L3_PerformXTalkCalibration,
 * @a VL53L3_PerformOffsetCalibration()
 *
 * @note This function doesn't Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   *pCalibrationData            pointer where to store Calibration
 *  data.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetCalibrationData(VL53L3_DEV Dev,
		VL53L3_CalibrationData_t  *pCalibrationData);

/**
 * @brief Sets the Zone Calibration Data.
 *
 * @par Function Description
 * This function set all the Zone nCalibration Data issued from the functions
 * @a VL53L3_PerformOffsetCalibration() in multi zone
 *
 * @note This function doesn't Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   *pZoneCalibrationData        Pointer to Zone Calibration data to be
 *  set.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_SetZoneCalibrationData(VL53L3_DEV Dev,
		VL53L3_ZoneCalibrationData_t *pZoneCalibrationData);

/**
 * @brief Gets the Zone Calibration Data.
 *
 * @par Function Description
 * This function get all the Zone Calibration Data issued from the functions
 * @a VL53L3_PerformOffsetCalibration()
 *
 * @note This function doesn't Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   *pZoneCalibrationData        pointer where to store Zone Calibration
 *  data.
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetZoneCalibrationData(VL53L3_DEV Dev,
		VL53L3_ZoneCalibrationData_t *pZoneCalibrationData);
/**
 * @brief Gets the optical center.
 *
 * @par Function Description
 * This function get the optical center issued from the nvm set at FTM stage
 * expressed in the same coordinate system as the ROI are
 *
 * @note This function doesn't Accesses the device
 *
 * @param   Dev                          Device Handle
 * @param   pOpticalCenterX              pointer to the X position of center
 * in 16.16 fix point
 * @param   pOpticalCenterY              pointer to the Y position of center
 * in 16.16 fix point
 * @return  VL53L3_ERROR_NONE            Success
 * @return  "Other error code"           See ::VL53L3_Error
 */
VL53L3_Error VL53L3_GetOpticalCenter(VL53L3_DEV Dev,
		FixPoint1616_t *pOpticalCenterX,
		FixPoint1616_t *pOpticalCenterY);

/** @} VL53L3_Calibration_group */

/** @defgroup VL53L3_Thresholds_group VL53L3 IRQ Triggered events Functions
 *  @brief    Functions used to configure interrupt to be triggered only when
 *  a measurement satisfies some thresholds parameters
 *  @{
 */

/**
 * @brief Configure the interrupt config, from the given structure
 *
 * @param[in]    Dev     : Device Handle
 * @param[in]    pConfig : pointer to configuration structure
 */

VL53L3_Error VL53L3_SetThresholdConfig(VL53L3_DEV Dev,
		VL53L3_DetectionConfig_t *pConfig);

/**
 * @brief Retrieves the interrupt config structure currently programmed
 *                             into the API
 *
 * @param[in]    Dev     : Device Handle
 * @param[out]   pConfig : pointer to configuration structure
 */

VL53L3_Error VL53L3_GetThresholdConfig(VL53L3_DEV Dev,
		VL53L3_DetectionConfig_t *pConfig);


/** @} VL53L3_Thresholds_group */


/** @} VL53L3_cut11_group */

#ifdef __cplusplus
}
#endif

#endif /* _VL53L3_API_H_ */
