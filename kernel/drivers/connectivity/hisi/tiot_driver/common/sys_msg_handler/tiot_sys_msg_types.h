/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description: TIoT sys msg types. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-03-11, Create file. \n
 */
#ifndef TIOT_SYS_MSG_TYPES_H
#define TIOT_SYS_MSG_TYPES_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_types TIoT sys msg types.
 * @ingroup  tiot_common_interface
 * @{
 */
enum SYS_INF_MSG_VALUE_ENUM {
    SYS_INF_PF_INIT = 0x00,                /* 平台软件初始化完成 */
    SYS_INF_BT_INIT = 0x01,                /* BT软件初始化完成 */
    SYS_INF_GNSS_INIT = 0x02,              /* GNSS软件初始化完成 */
    SYS_INF_FM_INIT = 0x03,                /* FM软件初始化完成 */
    SYS_INF_BT_DISABLE = 0x04,             /* BT禁能 */
    SYS_INF_GNSS_DISABLE = 0x05,           /* GNSS禁能 */
    SYS_INF_FM_DISABLE = 0x06,             /* FM禁能 */
    SYS_INF_BT_EXIT = 0x07,                /* BT退出 */
    SYS_INF_GNSS_EXIT = 0x08,              /* GNSS退出 */
    SYS_INF_FM_EXIT = 0x09,                /* FM退出 */
    SYS_INF_GNSS_WAIT_DOWNLOAD = 0x0A,     /* 等待GNSS代码下载 */
    SYS_INF_GNSS_DOWNLOAD_COMPLETE = 0x0B, /* GNSS代码下载完毕 */
    SYS_INF_BFG_HEART_BEAT = 0x0C,         /* 心跳信号 */
    SYS_INF_DEV_AGREE_HOST_SLP = 0x0D,     /* device回复host可睡 */
    SYS_INF_DEV_NOAGREE_HOST_SLP = 0x0E,   /* device回复host不可睡 */
    SYS_INF_WIFI_OPEN = 0x0F,              /* WCPU上电完成 */
    SYS_INF_IR_INIT = 0x10,                /* IR软件初始化完成 */
    SYS_INF_IR_EXIT = 0x11,                /* IR退出 */
    SYS_INF_NFC_INIT = 0x12,               /* NFC软件初始化完成 */
    SYS_INF_NFC_EXIT = 0x13,               /* NFC退出 */
    SYS_INF_WIFI_CLOSE = 0x14,             /* WCPU下电完成 */
    SYS_INF_RF_TEMP_NORMAL = 0x15,         /* RF温度正常 */
    SYS_INF_RF_TEMP_OVERHEAT = 0x16,       /* RF温度过热 */
    SYS_INF_MEM_DUMP_COMPLETE = 0x17,      /* bfgx异常时，MEM DUMP已完成 */
    SYS_INF_WIFI_MEM_DUMP_COMPLETE = 0X18, /* bfgx异常时，MEM DUMP已完成 */
    SYS_INF_UART_HALT_WCPU = 0x19,         /* uart halt wcpu ok */
    SYS_INF_UART_LOOP_SET_DONE = 0x1a,     /* device 设置uart环回ok */
    SYS_INF_CHR_ERRNO_REPORT = 0x1b,       /* device向host上报CHR异常码 */

    SYS_INF_FAKE_CLOSE_BT =  0x1c,         /* FAKE CLOSE BT的ACK */
    SYS_INF_RSV1 = 0x1d,                   /* rsv */
    SYS_INF_RSV2 = 0x1e,                   /* rsv */

    SYS_INF_GNSS_LPPE_INIT = 0x1F,         /* MP13 GNSS 新增的线程初始化完成 */
    SYS_INF_GNSS_LPPE_EXIT = 0x20,         /* MP13 GNSS 新增的线程退出 */
    SYS_INF_GNSS_TRICKLE_SLEEP = 0x21,     /* MP13 GNSS_TRICKLE_SLEEP */
    SYS_INF_ENABLE_GPIO_WKUP_BFG_RSP = 0x22, /* 配置uart or gpio唤醒 */
    SYS_INF_BOOT_UP_ACK = 0x23,             /* device启动时通过消息回复ack的场景（通常是通过GPIO回复ack） */
    SYS_INF_WAKE_UP_ACK = 0x24,             /* device被唤醒时通过消息回复ack的场景（通常是通过GPIO回复ack） */
    SYS_INF_SLE_INIT = 0x25,                /* SLE软件初始化完成 */
    SYS_INF_SLE_EXIT = 0x26,                /* SLE退出 */
    SYS_INF_REQUEST_SLEEP = 0x27,           /* device请求睡眠 */

    SYS_INF_BUTT /* 枚举定义最大不能超过127，因为128~255被用来当做心跳的时间戳 */
};

enum SYS_PL_MSG_VALUE_ENUM {
    SYS_PL_OPEN_BT = 0x0,
    SYS_PL_CLOSE_BT = 0x1,
    SYS_PL_OPEN_GNSS = 0x2,
    SYS_PL_CLOSE_GNSS = 0x3,
    SYS_PL_OPEN_FM = 0x4,
    SYS_PL_CLOSE_FM = 0x5,
    SYS_PL_OPEN_NFC = 0x6,
    SYS_PL_CLOSE_NFC = 0x7,
    SYS_PL_OPEN_IR = 0x8,
    SYS_PL_CLOSE_IR = 0x9,
    SYS_PL_OPEN_WIFI = 0xa,                  /* host通过uart打开WCPU */
    SYS_PL_CLOSE_WIFI = 0xb,                 /* host通过uart关闭WCPU */

    SYS_PL_READ_STACK = 0xc,                 /* host通过uart读栈，心跳超时时使用 */
    SYS_PL_QUERY_RF_TEMP = 0xd,              /* host通过uart查询rf温度 */

    SYS_PL_BFGNI_HOST_ASKTO_SLP = 0xe,       /* host allow device sleep */
    SYS_PL_BFGNI_HOST_DISALLOW_DEVSLP = 0xf, /* host disallow device sleep */

    SYS_PL_BFGNI_SHUTDOWN = 0x10,
    SYS_PL_ENABLE_PM = 0x11,                     /* enable plat dev lowpower feature */
    SYS_PL_DISABLE_PM = 0x12,                    /* disable plat dev lowpower feature */
    SYS_PL_BT_ENABLE_PM = 0x13,                  /* enable BT dev lowpower feature */
    SYS_PL_BT_DISABLE_PM = 0x14,                 /* disable BT dev lowpower feature */
    SYS_PL_GNSS_ENABLE_PM = 0x15,                /* enable GNSS dev lowpower feature */
    SYS_PL_GNSS_DISABLE_PM = 0x16,               /* disable GNSS dev lowpower feature */
    SYS_PL_NFC_ENABLE_PM = 0x17,                 /* enable NFC dev lowpower feature */
    SYS_PL_NFC_DISABLE_PM = 0x18,                /* disable NFC dev lowpower feature */

    SYS_PL_TEST_CAUSE_PANIC = 0x19,              /* HOST发命令，触发device进入exception */
    SYS_PL_DUMP_RESET_WCPU = 0x1a,               /* host通过uart不掉电复位WCPU */

    SYS_PL_HALT_WCPU = 0x1b,                     /* halt WCPU */
    SYS_PL_READ_WLAN_PUB_REG = 0x1c,             /* 读取wcpu的公共寄存器 */
    SYS_PL_READ_WLAN_PRIV_REG = 0x1d,            /* 读取wcpu的私有寄存器 */
    SYS_PL_READ_WLAN_MEM = 0x1e,                 /* 读取wcpu的mem */

    SYS_PL_EXCEP_RESET_WCPU = 0x1f,              /* wifi DFR WCPU复位 */

    SYS_PL_SET_UART_LOOP_HANDLER = 0x20,         /* 设置device侧uart环回处理函数 */
    SYS_PL_SET_UART_LOOP_FINISH = 0x21,          /* UART环回test ok，恢复gnss消息处理函数 */

    SYS_PL_PM_DEBUG = 0x22,                      /* 低功耗维测开关 */
    SYS_PL_BAUT_CHG_REQ_ACK = 0x23,              /* 波特率切换请求ACK */
    SYS_PL_BAUT_CHG_COMPLETE = 0x24,             /* 波特率切换完成 */

    SYS_PL_WIFI_OPEN_NOTIFY = 0x25,              /* wifi open  通知 */
    SYS_PL_WIFI_CLOSE_NOTIFY = 0x26,             /* wifi close 通知 */

    SYS_PL_CFG_OPEN_GLE  = 0x28,                 /* host通过uart打开GLE */
    SYS_PL_CFG_CLOSE_GLE = 0x29,                 /* host通过uart打开GLE */

    SYS_PL_PM_SET_WAKEUP_HOST_VALID = 0x30,     /* 根据板级连接配置device可以唤醒host */
    SYS_PL_PM_SET_WAKEUP_HOST_INVALID = 0x31,   /* 根据板级连接配置device不需要唤醒host */

    SYS_PL_BUTT
};

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif