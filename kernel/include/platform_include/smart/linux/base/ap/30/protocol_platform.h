/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: protocol header file
 * Create: 2021/12/05
 */
#ifndef __PROTOCOL_PLATFORM_H__
#define __PROTOCOL_PLATFORM_H__

#include "protocol_base.h"

typedef enum {
	/* system status */
	ST_NULL = 0,
	ST_BEGIN,
	ST_POWERON = ST_BEGIN,
	ST_MINSYSREADY,
	ST_DYNLOAD,
	ST_MCUREADY,
	ST_TIMEOUTSCREENOFF,
	ST_SCREENON, /* 6 */
	ST_SCREENOFF, /* 7 */
	ST_SLEEP, /* 8 */
	ST_WAKEUP, /* 9 */
	ST_POWEROFF,
	ST_RECOVERY_BEGIN, /* for ar notify modem when iom3 recovery */
	ST_RECOVERY_FINISH, /* for ar notify modem when iom3 recovery */
	ST_END
} sys_status_t;

struct modem_pkt_header_t {
	unsigned char tag;
	unsigned char cmd;
	unsigned char core : 4;
	unsigned char resp : 1;
	unsigned char partial_order : 3;
	unsigned char length;
};

typedef struct {
	struct pkt_header hd;
	/* 1:aux sensorlist 0:filelist 2: loading */
	unsigned char file_flg;
	/*
	 * num must less than
	 * (MAX_PKT_LENGTH-sizeof(PKT_HEADER)-sizeof(End))/sizeof(UINT16)
	 */
	unsigned char file_count; /* num of file or aux sensor */
	unsigned short file_list[]; /* fileid or aux sensor tag */
} pkt_sys_dynload_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned short status;
	unsigned short version;
} pkt_sys_statuschange_req_t;

#define RET_SUCC  0
#define RET_FAIL  (-1)
#define STARTUP_IOM3_CMD 0x00070001
#define RELOAD_IOM3_CMD 0x0007030D
#define IPC_SHM_MAGIC 0x1a2b3c4d
#define IPC_SHM_BUSY 0x67
#define IPC_SHM_FREE 0xab
#define MID_PKT_LEN (128 - sizeof(struct pkt_header))

struct ipc_shm_ctrl_hdr {
	signed int module_id;
	unsigned int buf_size;
	unsigned int offset;
	signed int msg_type;
	signed int checksum;
	unsigned int priv;
};

struct shmem_ipc_ctrl_package {
	struct pkt_header hd;
	struct ipc_shm_ctrl_hdr sh_hdr;
};

struct ipcshm_data_hdr {
	unsigned int magic_word;
	unsigned char data_free;
	unsigned char reserved[3]; /* reserved */
	struct pkt_header pkt;
};

#endif
