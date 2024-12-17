/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: sdc protocol header file
 * Create: 2022/07/25
 */
#ifndef __SDC_PROTOCOL_H__
#define __SDC_PROTOCOL_H__

struct SdcDataHeader {
	unsigned int fifoCount;
	unsigned int fifoWriteIndex;
	unsigned long long reserved[2];
};

struct SdcDataVec3 {
	float x;
	float y;
	float z;
	unsigned int accuracy;
	long long timestamp;
	unsigned long long sysCnt;
};

struct SdcDataVec4 {
	float b;
	float c;
	float d;
	float w;
	long long timestamp;
	unsigned long long sysCnt;
};

#endif
