/*
 * hw_laser_dmd.h
 *
 * laser dmd driver header file
 *
 * Copyright (C) 2014-2024 Huawei Technology Co., Ltd.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef _KERNEL_LASER_DMD_H_
#define _KERNEL_LASER_DMD_H_

#include "securec.h"
#include <dsm/dsm_pub.h>

/* camera laser */
#define DSM_LASER_PROBE_ERROR_NO			927013082
#define DSM_LASER_I2C_ERROR_NO				927013083
#define DSM_LASER_FW_DOWNLOAD_ERROR_NO		927013084
#define DSM_LASER_MEASURE_DISTANCE_ERROR_NO	927013085

typedef enum {
	HWLASER_VL53L1,
	HWLASER_VI5300,
	HWLASER_VL53L3,
	HWLASER_TYPE_MAX,
} device_type;

typedef enum {
	PROBE_SUCCESS,
	PROBE_FAIL,
	PROBE_STATE_MAX,
} probe_state;

int laser_dsm_report_dmd(int type, int err_no);
void report_laser_probe_state(device_type, probe_state);

#endif