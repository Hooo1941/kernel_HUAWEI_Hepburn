/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    ippmessage_common.h
 * Description:
 *
 * Date        2022-06-25
 ******************************************************************/

#ifndef __IPP_MESSAGE_COMMON_H__
#define __IPP_MESSAGE_COMMON_H__

#define MATCHER_LAYER_MAX        (6)
#define ARFEATURE_MAX_LAYER      (10)

typedef enum {
	PIXEL_FMT_IPP_Y8    = 0,
	PIXEL_FMT_IPP_1PF8  = 1,
	PIXEL_FMT_IPP_2PF8  = 2,
	PIXEL_FMT_IPP_3PF8  = 3,
	PIXEL_FMT_IPP_1PF10 = 4,
	PIXEL_FMT_IPP_2PF14 = 5,
	PIXEL_FMT_IPP_D32   = 6,
	PIXEL_FMT_IPP_D48   = 7,
	PIXEL_FMT_IPP_D64   = 8,
	PIXEL_FMT_IPP_D128  = 9,
	PIXEL_FMT_IPP_MAX,
} pix_format_e;

typedef enum {
	// Commercial Scenarios
	KLT_ONLY           = 0, // new add -r02
	ENH_ARF            = 1,
	ENH_ARF_KLT        = 2, // new add -r02
	ENH_ARF_MATCHER    = 3,
	ENH_ARF_MATCHER_MC = 4,

	// UT test mode
	ENH_ONLY           = 5,
	ARFEATURE_ONLY     = 6, // same ENH_ARF(enh disable) ; liang -can delete
	MATCHER_ONLY       = 7,
	MC_ONLY            = 8,
	ARF_MATCHER        = 9, // same ENH_ARF_MATCHER(enh disable)
	MATCHER_MC         = 10,
	ARF_MATCHER_MC     = 11, // same ENH_ARF_MATCHER_MC(enh disable)
	WORK_MODULE_MAX,
} work_module_e;

typedef enum {
	INTERNAL_BUFF = 0,
	EXTERNAL_BUFF,
} fd_buffer_flag_e;

/**
* fd        : To ensure security, the address used by the kernel is mapped by the kernel based on the fd.
* prot      : Memory Properties of the memory that provided by the fd.
* total_size: The total size of the memory that provided by the fd.
* offset    : The offset of the address we can use in the memory provided by fd.
* fd_flag   : The memory provided by the fd is an external buffer or an internal buffer.
* buffer    : The value of buffer is filled by the kernel(buffer = fd_to_iova(fd) + offset). After receiving
*             req-message, kernel should to initialize buffer to zero.
**/
typedef struct _ipp_stream_t {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	int fd; // new add -r02
	int prot; // new add -r02
	unsigned int offset; // new add -r02
	unsigned int total_size; // new add -r02
	fd_buffer_flag_e fd_flag; // new add -r02
	pix_format_e format;

	unsigned int buffer; // new add -r02
} ipp_stream_t;

#endif // __IPP_MESSAGE_COMMON_H__
