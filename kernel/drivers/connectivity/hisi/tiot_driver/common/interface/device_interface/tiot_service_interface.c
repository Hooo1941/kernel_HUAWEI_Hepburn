/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-10-28, Create file. \n
 */

#include "tiot_service_interface.h"
#include "tiot_service_interface_ext.h"
#include "tiot_controller.h"
#include "tiot_pm_wakelock.h"
#include "tiot_board_log.h"
#ifdef CONFIG_DEV_GNSS
#include "gnss_service.h"
#endif
#ifdef CONFIG_DEV_W33
#include "w33_service.h"
#endif
#ifdef CONFIG_DEV_CAXX
#include "caxx_service.h"
#endif
#ifdef CONFIG_DEV_BSXX
#include "bsxx_service.h"
#endif
#ifdef CONFIG_DEV_TIOT_SLAVE
#include "tiot_slave_service.h"
#endif

typedef struct {
    int32_t (* init)(void);
    void (* deinit)(void);
} tiot_service_entry;

typedef struct {
    const char *name;
    uint8_t dev_max;
    uintptr_t (* get_ctrl)(uint8_t dev_id);
} tiot_service_desc;

static const tiot_service_entry tiot_service_list[] = {
#ifdef CONFIG_DEV_GNSS
    { gnss_service_init, gnss_service_deinit },
#endif
#ifdef CONFIG_DEV_W33
    { w33_service_init, w33_service_deinit },
#endif
#ifdef CONFIG_DEV_CAXX
    { caxx_service_init, caxx_service_deinit },
#endif
#ifdef CONFIG_DEV_BSXX
    { bsxx_service_init, bsxx_service_deinit },
#endif
#ifdef CONFIG_DEV_TIOT_SLAVE
    { tiot_slave_service_init, tiot_slave_service_deinit },
#endif
};

static const tiot_service_desc tiot_service_desc_list[] = {
#ifdef CONFIG_DEV_GNSS
    { "gn71", CONFIG_GNSS_DEV_NUM, gnss_service_get_ctrl },
    { "gnss01", CONFIG_GNSS_DEV_NUM, gnss_service_get_ctrl },
#endif
#ifdef CONFIG_DEV_W33
    { "w33", CONFIG_W33_DEV_NUM, w33_service_get_ctrl },
#endif
#ifdef CONFIG_DEV_CAXX
    { "ca01", CONFIG_CAXX_DEV_NUM, caxx_service_get_ctrl },
#endif
#ifdef CONFIG_DEV_BSXX
    { "bs21", CONFIG_BSXX_DEV_NUM, bsxx_service_get_ctrl },
    { "bs25", CONFIG_BSXX_DEV_NUM, bsxx_service_get_ctrl },
#endif
#ifdef CONFIG_DEV_TIOT_SLAVE
    { "tiot_slave", CONFIG_TIOT_SLAVE_DEV_NUM, tiot_slave_service_get_ctrl },
#endif
};

static const tiot_service_desc *tiot_service_get_description(const char *device_name, uint8_t *dev_id)
{
    uint32_t i;
    uint32_t len = strlen(device_name);
    uint32_t name_len = 0;
    for (i = 0; i < (sizeof(tiot_service_desc_list) / sizeof(tiot_service_desc)); i++) {
        name_len = strlen(tiot_service_desc_list[i].name);
        /* devcie_name长度小于描述名时不可能匹配，避免memcmp对device_name访问溢出. */
        if (len < name_len) {
            continue;
        }
        if (memcmp(device_name, tiot_service_desc_list[i].name, name_len) == 0) {
            break;
        }
    }
    if (i == (sizeof(tiot_service_desc_list) / sizeof(tiot_service_desc))) {
        tiot_print_err("[TIoT]invalid device_name string %s.\r\n", device_name);
        return NULL;
    }
    /* device_name格式 */
    /* "xxxx" 其中xxxx为对应device名字, dev_id默认为0 */
    /* "xxxx,n" 其中n为单个数字字符，n < 10，dev_id设为n */
    if (len > name_len) {
        if (len - name_len != 2) { // n为单个字符，加上分隔符，长度固定为2
            tiot_print_err("[TIoT]device_name length %d invalid.\r\n", len);
            return NULL;
        }
        if (device_name[name_len] != ',') {
            tiot_print_err("[TIoT]device_name seprator invalid %c.\r\n", device_name[name_len]);
            return NULL;
        }
        if ((device_name[name_len + 1] < '0') || (device_name[name_len + 1] > '9')) {
            tiot_print_err("[TIoT]dev id invalid %c.\r\n", device_name[name_len + 1]);
            return NULL;
        }
        *dev_id = device_name[name_len + 1] - '0';
    }
    if (*dev_id >= tiot_service_desc_list[i].dev_max) {
        tiot_print_err("[TIoT]device_name dev id exceed max %c.\r\n", device_name[name_len + 1]);
        return NULL;
    }
    return &tiot_service_desc_list[i];
}

int32_t tiot_service_init(void)
{
    int32_t ret;
    uint32_t i;
    uint32_t j;
    if (tiot_pm_wake_lock_init() != 0) {
        return -1;
    }
    for (i = 0; i < (sizeof(tiot_service_list) / sizeof(tiot_service_entry)); i++) {
        ret = tiot_service_list[i].init();
        if (ret != 0) {
            tiot_print_err("[TIoT]service init fail @%d", i);
            goto fail;
        }
    }
    return 0;
fail:
    for (j = 0; j < i; j++) {
        tiot_service_list[i].deinit();
    }
    tiot_pm_wake_lock_destroy();
    return ret;
}

void tiot_service_deinit(void)
{
    uint32_t i;
    for (i = 0; i < (sizeof(tiot_service_list) / sizeof(tiot_service_entry)); i++) {
        tiot_service_list[i].deinit();
    }
    tiot_pm_wake_lock_destroy();
}

tiot_handle tiot_service_open(const char *device_name, void *param)
{
    tiot_controller *ctrl = NULL;
    uint8_t dev_id = 0;
    const tiot_service_desc *desc;
    if (device_name == NULL) {
        tiot_print_err("[TIoT]device_name is NULL.\r\n");
        return 0;
    }
    desc = tiot_service_get_description(device_name, &dev_id);
    if (desc == NULL) {
        return 0;
    }
    ctrl = (tiot_controller *)desc->get_ctrl(dev_id);
    if (tiot_controller_open(ctrl, param) != 0) {
        ctrl = NULL;
    }
    return (tiot_handle)ctrl;
}

void tiot_service_close(tiot_handle handle)
{
    tiot_controller *ctrl = (tiot_controller *)handle;
    tiot_controller_close(ctrl);
}

int32_t tiot_service_write_ext(tiot_handle handle, const uint8_t *data,
                               uint32_t len, const tiot_service_tx_param *param)
{
    tiot_controller *ctrl = (tiot_controller *)handle;
    if ((data == NULL) || (len == 0)) {
        tiot_print_dbg("[TIoT]send data invalid.\n");
        return -1;
    }
    if (tiot_controller_check_opened(ctrl) != true) {
        return -1;
    }
    return ctrl->ops.write(ctrl, data, len, (const tiot_xfer_tx_param *)param);
}

int32_t tiot_service_write(tiot_handle handle, const uint8_t *data, uint32_t len)
{
    /* 默认发送subsys为0的数据 */
    const tiot_service_tx_param param = { 0 };
    return tiot_service_write_ext(handle, data, len, &param);
}

int32_t tiot_service_read_ext(tiot_handle handle, uint8_t *buff, uint32_t buff_len, const tiot_service_rx_param *param)
{
    int32_t buff_invalid;
    tiot_controller *ctrl = (tiot_controller *)handle;
    if (param == NULL) {
        tiot_print_dbg("[TIoT]rx param invalid.\n");
        return -1;
    }
    buff_invalid = (buff == NULL) || (buff_len == 0);
#ifdef __KERNEL__
    /* 用作poll接口时，buff & buff_len可以为空. */
    if (((param->filp == NULL) || (param->poll_wait == NULL)) && (buff_invalid != 0)) {
#else
    if (buff_invalid != 0) {
#endif
        tiot_print_dbg("[TIoT]read buff invalid.\n");
        return -1;
    }
    if (tiot_controller_check_opened(ctrl) != true) {
        return -1;
    }
    return ctrl->ops.read(ctrl, buff, buff_len, (const tiot_xfer_rx_param *)param);
}

int32_t tiot_service_read(tiot_handle handle, uint8_t *buff, uint32_t buff_len, uint32_t timeout_ms)
{
    /* 默认接收queueID为0的数据 */
    /* tiot_service_rx_para结构体有kernel编译宏区分, 不能列表初始化, 否则参数未知. */
    tiot_service_rx_param param = { 0 };
    param.timeout_ms = timeout_ms;
    return tiot_service_read_ext(handle, buff, buff_len, &param);
}
