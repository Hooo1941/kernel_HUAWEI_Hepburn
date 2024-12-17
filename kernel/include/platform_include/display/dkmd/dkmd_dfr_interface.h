/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef DKMD_DFR_INTERFACE_H
#define DKMD_DFR_INTERFACE_H

enum SAFE_FRM_DISABLE_TYPE {
	LHBM_DISABLE = 0,
	SWITCH_GAMMA_DISABLE,
	INVALID_DISABLE
};

/** after called, dss exit vsync idle
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_exit_current_vsync_idle_period(uint32_t conn_id, uint32_t panel_type);

/** after called, dss use the latest vsync to send refresh cmd
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_dfr_send_refresh(uint32_t conn_id, uint32_t panel_type);

/** after called, dss(acpu) send safe frame rate to dacc.
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @param[in] safe_frm_rate decision module to send safe frame rate.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_dfr_send_safe_frm_rate(uint32_t conn_id, uint32_t panel_type, uint32_t safe_frm_rate);

/** after called, dss enable dimming
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_dfr_enable_dimming(uint32_t conn_id, uint32_t panel_type);

/** after called, dss disable dimming
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_dfr_disable_dimming(uint32_t conn_id, uint32_t panel_type);

/** after called, dss enable safe frm rate
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_dfr_enable_safe_frm_rate(uint32_t conn_id, uint32_t panel_type);

/** after called, dss disable safe frm rate
 *
 * @param[in] conn_id find the compose based on the conn_id.
 * @param[in] panel_type.
 * @return Operation status; 0 if the operation succeeded
 */
extern int32_t dkmd_dfr_disable_safe_frm_rate(uint32_t conn_id, uint32_t panel_type, int32_t disable_type);
#endif