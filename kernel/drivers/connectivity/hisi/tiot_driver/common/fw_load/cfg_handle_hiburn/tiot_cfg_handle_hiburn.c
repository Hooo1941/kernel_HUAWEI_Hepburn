/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-05, Create file. \n
 */

#include "tiot_firmware.h"
#include "tiot_firmware_utils.h"
#include "tiot_board_log.h"
#include "tiot_board_uart_port.h"
#include "tiot_cfg_handle_hiburn.h"

#define TIOT_FW_FILE_STA        0x43
#define TIOT_FW_FILE_ACK        0x06
#define TIOT_FW_FILE_NAK        0x15

#define TIOT_FW_HIBURN_ACK_START  0xDEADBEEF
#define TIOT_FW_HIBURN_ACK_START_FIRST  0xEF
#define TIOT_FW_HIBURN_ACK_TYPE     0xE1
#define TIOT_FW_HIBURN_ACK_TYPE_R   0x1E
#define TIOT_FW_HIBURN_RESET_TYPE   0x87
#define TIOT_FW_HIBURN_ACK_SUCC   0x5A
#define TIOT_FW_HIBURN_ACK_FAIL   0xA5

#define TIOT_FW_HIBURN_ACK_HEADER_SIZE  8
#define TIOT_FW_HIBURN_ACK_MAX_SIZE     128

#define VERSION_STR_LEN 40
#define HIBURN_FRAME_HEAD_LEN 8

typedef enum {
    TIOT_FW_EXTCMD_ACK = 0x0,
    TIOT_FW_EXTCMD_INIT = 0x1,
    TIOT_FW_EXTCMD_HANDSHAKE = 0x2,
    TIOT_FW_EXTCMD_ACK_FRAME = 0x3,
    TIOT_FW_EXTCMD_GET_VERSION = 0x4,
    TIOT_FW_EXTCMD_NUM
} tiot_fw_ext_cmd_hiburn;

typedef struct {
    uint32_t start;
    uint16_t size;
    uint8_t frame_type;
    uint8_t frame_type_r;
} tiot_fw_ack_frame_head_hiburn;

typedef struct {
    uint8_t header[TIOT_FW_HIBURN_ACK_HEADER_SIZE];
    uint8_t data[TIOT_FW_HIBURN_ACK_MAX_SIZE];
} tiot_fw_ack_frame_hiburn;

typedef struct {
    uint32_t baudrate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_ctrl;
} tiot_hiburn_uartcfg;

typedef int32_t (*hiburn_ext_cmd_handle_func)(tiot_fw *fw, const uint8_t *cmd, uint16_t len);

static int32_t tiot_fw_ack_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len);
static int32_t tiot_fw_init_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len);
static int32_t tiot_fw_handshake_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len);
static int32_t tiot_fw_ack_frame_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len);
static int32_t tiot_fw_get_version_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len);

static const hiburn_ext_cmd_handle_func tiot_fw_hiburn_ext_cmd_table[TIOT_FW_EXTCMD_NUM] = {
    tiot_fw_ack_handle_hiburn,
    tiot_fw_init_handle_hiburn,
    tiot_fw_handshake_handle_hiburn,
    tiot_fw_ack_frame_handle_hiburn,
    tiot_fw_get_version_handle_hiburn
};

static int32_t tiot_fw_read_until(tiot_fw *fw, uint8_t *buff, uint32_t len)
{
    int32_t ret = 0;
    uint32_t read_len = len;
    uint32_t read_cnt = 0;
    while (read_len > 0) {
        ret = tiot_firmware_read(fw, &buff[read_cnt], read_len);
        if (ret <= 0) {
            return -1;
        }
        read_len -= (uint32_t)ret;
        read_cnt += (uint32_t)ret;
    }
    return len;
}

static int32_t tiot_fw_ack_frame_receive(tiot_fw *fw, tiot_fw_ack_frame_hiburn *frame, uint16_t header_received)
{
    int32_t ret = 0;
    tiot_fw_ack_frame_head_hiburn *frame_head = NULL;
    uint32_t receive_cnt = TIOT_FW_HIBURN_ACK_HEADER_SIZE - header_received;
    /* Header */
    ret = tiot_fw_read_until(fw, &frame->header[header_received], receive_cnt);
    if (ret != receive_cnt) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    frame_head = (tiot_fw_ack_frame_head_hiburn *)frame->header;
    /* Data & Checksum */
    if ((frame_head->size > TIOT_FW_HIBURN_ACK_MAX_SIZE) ||
        (frame_head->size < TIOT_FW_HIBURN_ACK_HEADER_SIZE)) {
        tiot_print_err("[TIoT:hiburn]ack frame data error 0x%x", frame_head->size);
        return TIOT_FW_LOAD_RET_ERR;
    }
    receive_cnt = frame_head->size - TIOT_FW_HIBURN_ACK_HEADER_SIZE;
    ret = tiot_fw_read_until(fw, frame->data, receive_cnt);
    if (ret != receive_cnt) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    return TIOT_FW_LOAD_RET_OK;
}

static int32_t tiot_fw_ack_frame_check_ack(const tiot_fw_ack_frame_hiburn *frame)
{
    tiot_fw_ack_frame_head_hiburn *frame_head = (tiot_fw_ack_frame_head_hiburn *)frame->header;
    /* ACK check */
    if ((frame_head->start != TIOT_FW_HIBURN_ACK_START) || (frame_head->frame_type != TIOT_FW_HIBURN_ACK_TYPE) ||
        (frame_head->frame_type_r != TIOT_FW_HIBURN_ACK_TYPE_R)) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    tiot_print_info("[TIoT:hiburn]ack result 0x%x, 0x%x\r\n", frame->data[0], frame->data[1]);
    if (frame->data[0] != TIOT_FW_HIBURN_ACK_SUCC) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    return TIOT_FW_LOAD_RET_OK;
}

static int32_t tiot_fw_ack_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    int32_t ret = 0;
    uint8_t ack = 0;
    tiot_print_dbg("[TIoT:hiburn]ack handle...\r\n");
    ret = tiot_firmware_read(fw, &ack, sizeof(uint8_t));
    if (ret != sizeof(uint8_t)) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    /* Y-modem ACK. */
    tiot_print_dbg("[TIoT:hiburn]ack 0x%x\r\n", ack);
    /* 空命令用于接收STA */
    if ((len == 0) && (ack == TIOT_FW_FILE_STA)) {
        return TIOT_FW_LOAD_RET_OK;
    }
    if (ack == TIOT_FW_FILE_ACK) {
        return TIOT_FW_LOAD_RET_OK;
    }
    /* 非空命令时收到STA说明数据错误或超时，等待下一个STA后重发。 */
    if (ack == TIOT_FW_FILE_STA) {
        ret = tiot_firmware_read(fw, &ack, sizeof(uint8_t));
        if (ret != sizeof(uint8_t)) {
            return TIOT_FW_LOAD_RET_ERR;
        }
        if (ack == TIOT_FW_FILE_STA) {
            return TIOT_FW_LOAD_RET_RETRY;
        }
    }
    return TIOT_FW_LOAD_RET_ERR;
}

static int32_t tiot_fw_init_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    const uint8_t boot_str[] = "boot.\r\n";
    int32_t ret = 0;
    uint8_t init_str_buff[sizeof(boot_str) - 1] = { 0 };
    tiot_print_dbg("[TIoT:hiburn]waiting init str...\r\n");
    ret = tiot_fw_read_until(fw, init_str_buff, sizeof(boot_str) - 1);
    if ((ret <= 0) || ((uint32_t)ret != sizeof(boot_str) - 1)) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    if (memcmp(boot_str, init_str_buff, sizeof(boot_str) - 1) != 0) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    return TIOT_FW_LOAD_RET_OK;
}

static inline uint8_t tiot_fw_hiburn_uart_databits_convert(uint8_t hiburn_databits)
{
    const uint8_t hiburn_databits_5 = 5;
    const uint8_t hiburn_databits_8 = 8;
    if (hiburn_databits >= hiburn_databits_5 ||hiburn_databits <= hiburn_databits_8) {
        return hiburn_databits_8 - hiburn_databits;   /* hiburn格式databits到tiot databits枚举转换。 */
    }
    return TIOT_UART_ATTR_DATABIT_BUTT;
}

static int32_t tiot_fw_handshake_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    int32_t ret;
    uint8_t i;
    const uint8_t handshake_ack_count = 3;
    tiot_controller *controller = tiot_container_of(fw, tiot_controller, firmware);
    const tiot_hiburn_uartcfg *hiburn_uartcfg = (const tiot_hiburn_uartcfg *)(cmd + TIOT_FW_HIBURN_ACK_HEADER_SIZE);
    tiot_uart_config uart_cfg = {
        .baudrate = hiburn_uartcfg->baudrate,
        .attr.data_bits = tiot_fw_hiburn_uart_databits_convert(hiburn_uartcfg->data_bits),
        .attr.parity = hiburn_uartcfg->parity,
        .attr.stop_bits = hiburn_uartcfg->stop_bits,
        .attr.flow_ctrl = (hiburn_uartcfg->flow_ctrl != TIOT_UART_ATTR_FLOW_CTRL_DISABLE) ?
                          TIOT_UART_ATTR_FLOW_CTRL_ENABLE : TIOT_UART_ATTR_FLOW_CTRL_DISABLE
    };
    /* ACK * 3 */
    tiot_fw_ack_frame_hiburn frame = { 0 };
    tiot_print_dbg("[TIoT:hiburn]hand shaking..\r\n");
    for (i = 0; i < handshake_ack_count; i++) {
        ret = tiot_fw_ack_frame_receive(fw, &frame, 0);
        if (ret != TIOT_FW_LOAD_RET_OK) {
            return TIOT_FW_LOAD_RET_ERR;
        }
        ret = tiot_fw_ack_frame_check_ack(&frame);
        if (ret != TIOT_FW_LOAD_RET_OK) {
            return TIOT_FW_LOAD_RET_ERR;
        }
    }
    /* Set uart config */
    if (tiot_xfer_set_config(&controller->transfer, &uart_cfg) != 0) {
        return TIOT_FW_LOAD_RET_ERR;
    }
    return TIOT_FW_LOAD_RET_OK;
}

static int32_t tiot_fw_ack_frame_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    int32_t ret = 0;
    uint8_t ack = 0;
    const uint32_t last_byte_timeout = 50; /* 50ms内连续接收字符作为同一段ACK处理. */
    tiot_xfer_rx_param rx_param = { TIOT_XFER_RECV_WAIT_FOREVER, 0 };
    tiot_controller *controller = tiot_container_of(fw, tiot_controller, firmware);
    tiot_fw_ack_frame_hiburn frame = { 0 };
    const tiot_fw_ack_frame_head_hiburn *cmd_head = (const tiot_fw_ack_frame_head_hiburn *)cmd;
    tiot_print_dbg("[TIoT:hiburn]ack frame..\r\n");
    /* 复位特殊处理 */
    if ((cmd_head != NULL) && (cmd_head->frame_type == TIOT_FW_HIBURN_RESET_TYPE)) {
        return TIOT_FW_LOAD_RET_OK;
    }
    while (1) {
        /* 此处理内，必定为连续字符串或HiBurn格式ACK，第一个字符一直等待;
           之后字符接收超时即为此段数据结束，退出循环。 */
        ret = tiot_xfer_recv(&controller->transfer, &ack, sizeof(uint8_t), &rx_param);
        if (ret == 0) {
            break;
        }
        if (ret != sizeof(uint8_t)) {
            return TIOT_FW_LOAD_RET_ERR;
        }
        if (ack == TIOT_FW_HIBURN_ACK_START_FIRST) {
            frame.header[0] = ack;
            ret = tiot_fw_ack_frame_receive(fw, &frame, 1);
            if (ret != TIOT_FW_LOAD_RET_OK) {
                return ret;
            }
            ret = tiot_fw_ack_frame_check_ack(&frame);
            if (ret != TIOT_FW_LOAD_RET_OK) {
                return ret;
            }
        }
        rx_param.timeout = last_byte_timeout;
    }
    return TIOT_FW_LOAD_RET_OK;
}

static int32_t tiot_fw_send_reset_cmd(tiot_fw *fw)
{
    int32_t ret;
    uint8_t reset_cmd[] = {0xef, 0xbe, 0xad, 0xde, 0x0c, 0x00, 0x87, 0x78, 0x00, 0x00, 0x61, 0x94};

    ret = tiot_firmware_write(fw, reset_cmd, sizeof(reset_cmd));
    if (ret != sizeof(reset_cmd)) {
        tiot_print_err("[TIoT:fw]reset cmd send fail.\r\n");
        return TIOT_FW_LOAD_RET_ERR;
    }
    return TIOT_FW_LOAD_RET_OK;
}

static int32_t tiot_fw_get_version_handle_hiburn(tiot_fw *fw, const uint8_t *cmd, uint16_t len)
{
    int32_t ret;
    const char *fwpkg_version;
    char device_version[VERSION_STR_LEN];
    tiot_fw_ack_frame_hiburn frame = { 0 };
    /* Hiburn get version命令不需要参数，这里为了方便处理，在命令的参数位置加上了`fwpkg version`
     * (notes: 对端不会校验命令参数) */
    fwpkg_version = (const char *)cmd + HIBURN_FRAME_HEAD_LEN;

    ret = tiot_fw_read_until(fw, device_version, VERSION_STR_LEN);
    if ((ret <= 0) || (ret != VERSION_STR_LEN)) {
        return TIOT_FW_LOAD_RET_ERR;
    }

    /* 版本号不匹配，就继续加载 */
    if (memcmp(fwpkg_version, device_version, VERSION_STR_LEN) != 0) {
        return TIOT_FW_LOAD_RET_OK;
    }

    /* 版本号匹配，就重新复位device，然后退出加载流程 */
    ret = tiot_fw_send_reset_cmd(fw);
    if (ret != TIOT_FW_LOAD_RET_OK) {
        return ret;
    }
    ret = tiot_fw_ack_frame_receive(fw, &frame, 0);
    if (ret != TIOT_FW_LOAD_RET_OK) {
        return ret;
    }
    return TIOT_FW_LOAD_RET_QUIT;
}

int32_t tiot_fw_ext_cmd_handle_hiburn(tiot_fw *fw, uint16_t ext_cmd, const uint8_t *cmd, uint16_t len)
{
    if (ext_cmd >= TIOT_FW_EXTCMD_NUM) {
        tiot_print_err("[TIoT:fw]hiburn ext cmd exceed max!\r\n");
        return -1;
    }
    return tiot_fw_hiburn_ext_cmd_table[ext_cmd](fw, cmd, len);
}