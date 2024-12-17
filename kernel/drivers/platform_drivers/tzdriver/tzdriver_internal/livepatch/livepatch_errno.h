/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: static definitions of livepatch error code
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef LIVEPATCH_ERRNO_H
#define LIVEPATCH_ERRNO_H

enum livepatch_err_code {
	/*
	 * Common error codes
	 * ranging from 0x0 to 0xff
	 */
	LIVEPATCH_SUCCESS = 0x0,                       /* success */
	LIVEPATCH_GENERIC_ERR,                         /* generic error occurs */
	LIVEPATCH_PARAM_INVALID,                       /* invalid paramters */
	LIVEPATCH_OUT_OF_MEM,                          /* out buffer is not enough for current request */
	LIVEPATCH_CMD_TIME_OUT,                        /* command timeout */
	LIVEPATCH_NODE_ACCESS_DENIED,                  /* kernel node can not access */
	LIVEPATCH_CONFIG_INVALID,                      /* manifest configs value invalid */

	/*
	 * Livepatch specific error codes
	 * ranging from 0x100 to 0xfff
	 */
	LIVEPATCH_FUNC_HASH_ERR = 0x100,               /* patch func hash value is incorrect */
	LIVEPATCH_FILE_SIZE_ERR,                       /* patch sec file length is too large or zero */
	LIVEPATCH_VERSION_INVALID,                     /* patch version does not match the tee version */
	LIVEPATCH_PARSE_SEC_FAILED,                    /* verify patch sec file signature failed */
	LIVEPATCH_ELF_FILE_INVALID,                    /* elf file format invalid */
	LIVEPATCH_RELOCATE_FAILED,                     /* symbol relocate failed */
	LIVEPATCH_PROC_NOT_EXIST,                      /* target process is not exist */
	LIVEPATCH_MODULE_NOT_LOADED,                   /* module is not loaded */
	LIVEPATCH_MODULE_LOADED,                       /* module loaded */
	LIVEPATCH_MODULE_DISABLED,                     /* module is disabled */
	LIVEPATCH_MODULE_ENABLED,                      /* module enabled */
	LIVEPATCH_MODULE_IN_R_ENABLE,                  /* module stat is r_enable */
	LIVEPATCH_MODULE_IN_R_DISABLE,                 /* module stat is r_disable */
	LIVEPATCH_MODULE_IN_R_LOAD,                    /* module stat is r_load */
};

enum livepatch_err_code_origin {
	LIVEPATCH_ORIGIN_API = 0x1000,                 /* error occurs in handling client API */
	LIVEPATCH_ORIGIN_COMMS = 0x2000,               /* error occurs in communicating between REE and TEE */
	LIVEPATCH_ORIGIN_TEE = 0x3000                  /* error occurs in TEE */
};

#endif
