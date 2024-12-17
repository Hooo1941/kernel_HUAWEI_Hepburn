/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: tof8801 driver
 */

#ifndef __TOF8801_BOOTLOADER_H
#define __TOF8801_BOOTLOADER_H

#include <linux/i2c.h>
#include "tof8801.h"

#define TOF8801_BL_IS_CMD_BUSY(x)     ((x) > 0xF)
#define TOF8801_BL_CMD_WAIT_MSEC      1
#define TOF_VALID_CHKSUM              0xFF
#define TOF8801_BL_DEFAULT_SALT       0x29
#define TOF8801_I2C_HEADER_SIZE       (TOF8801_I2C_CMD_SIZE + TOF8801_I2C_DATA_LEN_SIZE)
#define TOF8801_I2C_FOOTER_SIZE       TOF8801_I2C_CHKSUM_SIZE
#define TOF8801_I2C_CMD_MAX_SIZE      (TOF8801_I2C_HEADER_SIZE + TOF8801_I2C_MAX_DATA_SIZE + TOF8801_I2C_FOOTER_SIZE)
#define TOF8801_CALC_CHKSUM_SIZE(datasize) ((datasize) + TOF8801_I2C_HEADER_SIZE)
#define TOF8801_CALC_BL_CMD_SIZE(datasize) (TOF8801_CALC_CHKSUM_SIZE(datasize) + TOF8801_I2C_FOOTER_SIZE)
#define TOF8801_CALC_BL_RSP_SIZE(datasize) ((datasize) + TOF8801_I2C_HEADER_SIZE + TOF8801_I2C_FOOTER_SIZE)

enum tof8801_BL_regs {
	TOF8801_CMD_STAT   = 0x08,
	TOF8801_DATA_SIZE  = 0x09,
	TOF8801_DATA_0     = 0x0A,
	TOF8801_DATA_127   = 0x89,
	TOF8801_CHKSUM     = 0x8A,
};

enum tof8801_bootloader_cmd {
	BL_RESET           = 0x10,
	BL_RAMREMAP_RESET  = 0x11,
	BL_UPLOAD_INIT     = 0x14,
	BL_R_RAM           = 0x40,
	BL_W_RAM           = 0x41,
	BL_ADDR_RAM        = 0x43,
};

enum tof8801_bootloader_cmd_stat {
	READY        = 0x0,
	ERR_SIZE     = 0x1,
	ERR_CSUM     = 0x2,
	ERR_RES      = 0x3,
	ERR_APP      = 0x4,
	ERR_TIMEOUT  = 0x5,
	ERR_LOCK     = 0x6,
	ERR_RANGE    = 0x7,
	ERR_MORE     = 0x8,
	ERROR1       = 0x9,
	ERROR2       = 0xA,
	ERROR3       = 0xB,
	ERROR4       = 0xC,
	ERROR5       = 0xD,
	ERROR6       = 0xE,
	ERROR7       = 0xF,
	CMD_BUSY     = 0x10,
	MAX_BL_CMD_STAT,
};

extern const char *tof8801_bootloader_cmd_stat_str[MAX_BL_CMD_STAT];

/* Bootloader command responses */
struct tof8801_BL_short_resp {
	char status;
	char size;
	char reserved[TOF8801_I2C_CMD_MAX_SIZE - 3];
	char chksum;
};

struct tof8801_BL_read_ram_resp {
	char status;
	char size;
	char data[TOF8801_I2C_CMD_MAX_SIZE + 1]; /* chksum at flexible position */
};

struct tof8801_anon_resp {
	char data[TOF8801_I2C_CMD_MAX_SIZE];
};

union tof8801_BL_response {
	struct tof8801_anon_resp anon_resp;
	struct tof8801_BL_read_ram_resp read_ram_resp;
	struct tof8801_BL_short_resp short_resp;
};

/* Bootloader commands */
struct tof8801_BL_short_cmd {
	char command;
	char size;
	char chksum;
	char reserved[TOF8801_I2C_MAX_DATA_SIZE];
};

struct tof8801_BL_upload_init_cmd {
	char command;
	char size;
	char seed;
	char chksum;
	char reserved[TOF8801_I2C_MAX_DATA_SIZE - 1];
};

struct tof8801_BL_read_ram_cmd {
	char command;
	char size;
	char num_bytes;
	char chksum;
	char reserved[TOF8801_I2C_MAX_DATA_SIZE - 1];
};

struct tof8801_BL_write_ram_cmd {
	char command;
	char size;
	char data[TOF8801_I2C_MAX_DATA_SIZE + 1]; /* chksum in flexible position */
};

struct tof8801_BL_addr_ram_cmd {
	char command;
	char size;
	char addr_lsb;
	char addr_msb;
	char chksum;
	char reserved[TOF8801_I2C_MAX_DATA_SIZE - 2];
};

struct tof8801_BL_anon_cmd {
	char data[TOF8801_I2C_CMD_MAX_SIZE];
};

union tof8801_BL_command {
	struct tof8801_BL_anon_cmd anon_cmd;
	struct tof8801_BL_addr_ram_cmd addr_ram_cmd;
	struct tof8801_BL_write_ram_cmd write_ram_cmd;
	struct tof8801_BL_read_ram_cmd read_ram_cmd;
	struct tof8801_BL_upload_init_cmd upload_init_cmd;
	struct tof8801_BL_short_cmd short_cmd;
};

struct tof8801_BL_application {
	char app_id;
	int debug;
	union tof8801_BL_command BL_command;
	union tof8801_BL_response BL_response;
};

extern char tof8801_calc_chksum(const char *, char);
extern char *get_BL_rsp_buf(struct tof8801_BL_application *app);
extern char *get_BL_cmd_buf(struct tof8801_BL_application *app);
extern int is_BL_cmd_busy(struct i2c_client *);
extern int tof8801_BL_send_rcv_cmd(struct i2c_client *, struct tof8801_BL_application *);
extern int tof8801_BL_short_cmd(struct i2c_client *,
	struct tof8801_BL_application *, enum tof8801_bootloader_cmd);
extern int tof8801_BL_reset(struct i2c_client *, struct tof8801_BL_application *);
extern int tof8801_BL_addr_ram(struct i2c_client *, struct tof8801_BL_application *, int);
extern int tof8801_BL_read_ram(struct i2c_client *, struct tof8801_BL_application *, char *, int);
extern int tof8801_BL_write_ram(struct i2c_client *, struct tof8801_BL_application *, const char *, int);
extern int tof8801_BL_ram_remap(struct i2c_client *client, struct tof8801_BL_application *);
extern char *app_id_to_app_str(char app);
extern int tof8801_BL_upload_init(struct i2c_client *client,
	struct tof8801_BL_application *BL_app, char salt);
extern void tof8801_BL_init_app(struct tof8801_BL_application *BL_app);
extern int tof8801_BL_read_status(struct i2c_client *client,
	struct tof8801_BL_application *BL_app, int num_retries);

#endif /* __TOF8801_BOOTLOADER_H */
