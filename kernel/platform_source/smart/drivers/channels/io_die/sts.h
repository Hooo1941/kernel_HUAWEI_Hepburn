/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Contexthub 9030 driver.
 * Create: 2022-11-30
 */
#ifndef __STS_H__
#define __STS_H__

#define STS_REG_LEN             4
#define STS_ERR_INFO_LEN        STS_REG_LEN
#define STS_STATUS_LEN          1

typedef enum {
	STS_CMD_START,
	STS_CMD_REQUEST_NORMAL = 0x01,
	STS_CMD_REQUEST_NORMAL_RESP = 0x02,
	STS_CMD_RELEASE_NORMAL = 0x03,
	STS_CMD_RELEASE_NORMAL_RESP = 0x04,
	STS_CMD_REG_OP = 0x05,
	STS_CMD_REG_OP_RESP = 0x06,
	STS_CMD_NOTIFY_REGISTER = 0x07,
	STS_CMD_NOTIFY_REGISTER_RESP = 0x08,
	STS_CMD_GET_STATUS = 0x09,
	STS_CMD_GET_STATUS_RESP = 0x0A,
	STS_CMD_STATUS_NOTIFY = 0x0B,
	STS_CMD_STATUS_NOTIFY_RESP = 0x0C,
	STS_CMD_CLK_OUT_CFG = 0x0D,
	STS_CMD_CLK_OUT_CFG_RESP = 0x0E,
	STS_CMD_SPI_RST = 0x0F,
	STS_CMD_SPI_RST_RESP = 0x10,
	STS_CMD_SPI_CONFIG = 0x11,
	STS_CMD_SPI_CONFIG_RESP = 0x12,
}sts_ipc_cmd_enum;

enum ioide_sts_err_code{
	STS_ESD_ERR,
	STS_ERR_END,
};

typedef enum {
	STS_ST_BEGIN,
	STS_ST_ERROR = STS_ST_BEGIN,
	STS_ST_PRE_INITED,
	STS_ST_INITED,
	STS_ST_NORMAL,
	STS_ST_LOW_SPEED,
	STS_ST_NON_EXIST,
	STS_ST_END,
}sts_sys_status_enum;

typedef enum {
	STS_USER_BEGIN,
	STS_USER_AUDIO = STS_USER_BEGIN,
	STS_USER_SPI,
	STS_USER_I2C,
	STS_USER_TP,
	STS_USER_CAMERA,
	STS_USER_END,
} sts_ap_user_enum;

typedef enum {
	STS_SUB_CMD_REG_WRITE,
	STS_SUB_CMD_REG_READ,
}sts_reg_sub_cmd_enum;

typedef enum {
	STS_SUB_CMD_OPEN,
	STS_SUB_CMD_CLOSE,
}sts_clk_out_cfg_enum;

typedef struct {
	uint32_t op;        /* write or read, see sts_reg_sub_cmd_enum */
	uint32_t slave_id;  /* 0: slave A, 1: slave B */
	uint32_t reg_addr;
	uint32_t val;       /* write vlaue, if read, ignore it */
} sts_reg_op_t;

typedef struct {
	uint32_t speed;
} sts_spi_config_t;

typedef struct {
	uint8_t a_side[STS_ERR_INFO_LEN];
	uint8_t b_side[STS_ERR_INFO_LEN];
} sts_err_info_t;

typedef struct {
	uint32_t op_id;
} sts_clk_out_t;

typedef void (*sts_user_func)(sts_sys_status_enum status, sts_err_info_t *info);

sts_sys_status_enum sts_get_status(void);
int sts_notify_register(sts_ap_user_enum user, sts_user_func func);
int sts_reg_op(sts_reg_op_t *data);
int sts_do_reg_op(sts_reg_op_t *data);
int sts_normal_request(sts_ap_user_enum src);
int sts_normal_release(sts_ap_user_enum src);
int sts_clk_out_sys(sts_clk_out_t *data);
int sts_spi_config(sts_spi_config_t *data);

void* send_sts_data_to_dubai(void);
void sts_data_reset(void);
/* used for ESD scene */
int sts_spi_rst(sts_ap_user_enum src, enum ioide_sts_err_code err_code);

#endif

