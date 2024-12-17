/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef _EMCOM_DSCP_H
#define _EMCOM_DSCP_H

#define DSCP_TOP_APP_MAX_NUM 16
#define ROOT_UID_MASK 100000
#define DSCP_INVALID_VALUE (-1)

#define DSCP_SYS_TOS_VALUE 0xB8

typedef enum {
	DSCP_APP_ADD = 1,
	DSCP_APP_DEL = 3,
}enum_dscp_update_app_op;

typedef struct {
	uint32_t uid;
	int val;
} dscp_app_info;

typedef struct {
	uint32_t app_sum;
	dscp_app_info allapp[DSCP_TOP_APP_MAX_NUM];
}dscp_all_app_info;

typedef struct {
	uint32_t app_sum;
	dscp_app_info data[0];
}dscp_parse_start_info;

typedef struct {
	uint32_t uid;
	uint32_t operation;
	int val;
}dscp_parse_update_app_info;

void emcom_xengine_dscp_init(void);
void emcom_xengine_dscp_clear(void);
void emcom_xengine_dscp_config_start(const char *pdata, uint16_t len);
void emcom_xengine_dscp_add_app(uint32_t uid, int val);
void emcom_xengine_dscp_update_app_info(const char *pdata, uint16_t len);

#endif
