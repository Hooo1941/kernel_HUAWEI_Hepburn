/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: This module is used to start vcom between ril and kernel
 * Author: lizhenzhou1@huawei.com
 * Create: 2022-11-26
 */

#ifndef __VCOM_NODES_H__
#define __VCOM_NODES_H__

#define VCOM_SYNCCOM 0
#define VCOM_TYPE_VIRTUAL 0x5
#define VCOM_STREAM 0xab

enum com_id_e {
	COM_ID_STAVCOM_0,
	COM_ID_BOTTOM,
};

struct com_name_id_map_s {
	char* name;
	enum com_id_e id;
	unsigned int type;
	unsigned int flag;
	unsigned int format;
	unsigned int bufsize;
	unsigned int closed;
};

static const struct com_name_id_map_s com_name_id_map[] = {
	{"stavcom", COM_ID_STAVCOM_0, VCOM_TYPE_VIRTUAL, VCOM_SYNCCOM, VCOM_STREAM, 4096, 0}
};

#endif
