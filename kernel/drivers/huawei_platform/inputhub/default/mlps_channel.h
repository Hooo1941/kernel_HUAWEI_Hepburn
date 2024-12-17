/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mlps channel header file
 * Author: jiangzimu <jiangzimu@huawei.com>
 * Create: 2022-02-16
 */

#ifndef __MLPS_CHANNEL_H__
#define __MLPS_CHANNEL_H__
typedef enum {
	SOFTBUS_HEARTBEAT_ALIVE,
	SOFTBUS_DECISION_CENTER,
	HMSCORE_FIND_NETWORK,
	HMSCORE_NEARBY_BEACON,
	BASTET,
	SUPER_PRIVACY,
	MODULE_END,
} mlps_app_t;

typedef enum {
	MLPS_CMD_OPEN,  //  ap -> sensorhub
	MLPS_CMD_CLOSE,
	MLPS_CMD_CONFIG,
	MLPS_CMD_UPLOAD,  // sensorhub -> ap
	MLPS_CMD_ACK,
} mlps_cmd_list;

typedef enum {
	MLPS_MPXX,
	MAX_CONN_FUNCTION_NUM,
} mlps_connectivity_tag_list;

typedef enum {
	MLPS_WAIT_GNSS_OPERATION = -3,
	MLPS_NOT_SUPPORT_GNSS_OPERATION,
	MLPS_OPERATE_GNSS_FAIL,
	MLPS_OPERATE_GNSS_SUCCESS,
} mlps_operate_gnss_status_list;

typedef int (*connectivity_callback)(int para);
#ifdef CONFIG_MLPS_OPERATE_CONN
/*
 * Function    : mlps_ops_conn_function_register
 * Description : register function to make mlps operate connectivity function
 * Input       : [tag] tag for different connectivities
 *             : [func] connectivity function
 * Output      : none
 * Return      : 0:successs, -1:fail
 */
int mlps_ops_conn_function_register(unsigned int tag, connectivity_callback func);

/*
 * Function    : mlps_operate_gnss_power
 * Description : mlps operate gnss cpu power on/off for open/close the i3c route in c
 * Input       : [cmd] open : 0, close : 1 (mlps_cmd_list only support open/close)
 *             : [service] service : service of mlps (mlps_app_t)
 * Output      : none
 * Return      : 0:successs, -1:fail, -2:unsupported
 */
int mlps_operate_gnss_power(unsigned short cmd, unsigned int service);

/*
 * Function    : get_mlps_operate_gnss_status
 * Description : show whether mlps operates gnss success
 * Input       : none
 * Output      : none
 * Return      : 0:successs, -1:fail, -2:unsupported, -3:waiting operation
 */
int get_mlps_operate_gnss_status(void);
#endif
#endif