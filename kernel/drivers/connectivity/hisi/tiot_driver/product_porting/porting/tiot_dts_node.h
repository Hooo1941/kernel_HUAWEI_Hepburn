/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description: dts nodes. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-02-21, Create file. \n
 */
#ifndef TIOT_DTS_NODE_H
#define TIOT_DTS_NODE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
tiot {                                                   // 总节点名
    bs21 {                                               // 子节点名，选择bs21/bs25, 对应tiot_service_open参数
        compatible = "tiot,bsxx";                        // 匹配名，与对应device匹配，不可修改
        tiot,misc-dev-name = "bs21";                     // 默认设备节点名，可自定义
        tiot,om-misc-dev-name = "bs21-om";               // OM设备节点名，可自定义
        tiot,tty-dev-name = "ttyXXXN";                   // 使用的tty设备节点名
        tiot,power-on-gpio = <xxx>;                      // power on gpio管脚
        tiot,wake-out-gpio = <xxx>;                      // 唤醒对端 gpio管脚
        tiot,wake-in-gpio = <xxx>;                       // 被唤醒 gpio管脚
        tiot,cfg-path = "/path/to/cfg/xxx_cfg.bin";      // 固件文件完整路径（包含文件名）
    }
}
*/

#define DTS_NODE_TIOT               "tiot"

#define DTS_PROP_MISC_DEV_NAME      "tiot,misc-dev-name"
#define DTS_PROP_OM_MISC_DEV_NAME   "tiot,om-misc-dev-name"

#define DTS_PROP_TTY_DEV_NAME       "tiot,tty-dev-name"
#define DTS_PROP_TTY_BAUDRATE       "tiot,tty-baudrate"
#define DTS_PROP_GPIO_POWEN_ON      "tiot,power-on-gpio"
#define DTS_PROP_GPIO_WAKEOUT       "tiot,wake-out-gpio"
#define DTS_PROP_GPIO_WAKEIN        "tiot,wake-in-gpio"

#define DTS_PROP_CFG_PATH           "tiot,cfg-path"

// 用于在dts中设置不同产品使用不同波特率， 波特率归一后删除
uint32_t bsxx_board_get_baudrate(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif