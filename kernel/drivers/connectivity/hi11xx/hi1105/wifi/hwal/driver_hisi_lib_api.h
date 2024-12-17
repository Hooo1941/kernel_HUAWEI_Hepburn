/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: API for user calls
 * Author: wifi
 * Create: 2017年1月6日
 */
#ifndef DRIVER_HISI_LIB_API_H_
#define DRIVER_HISI_LIB_API_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define HISI_SUCC            0
#define HISI_EFAIL           1
#define HISI_EINVAL         22
#define HISI_NULL           (0L)

#define ETH_ALEN                 6
#define MAX_SSID_LEN             32
#define ETH_ADDR_LEN             6
#define HISI_RATE_INFO_FLAGS_MCS                    (1<<0)
#define HISI_RATE_INFO_FLAGS_VHT_MCS                (1<<1)
#define HISI_RATE_INFO_FLAGS_40_MHZ_WIDTH           (1<<2)
#define HISI_RATE_INFO_FLAGS_80_MHZ_WIDTH           (1<<3)
#define HISI_RATE_INFO_FLAGS_80P80_MHZ_WIDTH        (1<<4)
#define HISI_RATE_INFO_FLAGS_160_MHZ_WIDTH          (1<<5)
#define HISI_RATE_INFO_FLAGS_SHORT_GI               (1<<6)
#define HISI_RATE_INFO_FLAGS_60G                    (1<<7)

typedef enum {
    HISI_BOOL_FALSE = 0,
    HISI_BOOL_TRUE = 1,
    HISI_BOOL_BUTT
} hisi_bool_type_enum;

typedef enum {
    HISI_BAND_WIDTH_40PLUS = 0,
    HISI_BAND_WIDTH_40MINUS = 1,
    HISI_BAND_WIDTH_20M = 2,

    HISI_BAND_WIDTH_BUTT
} hisi_channel_bandwidth_enum;

typedef enum {
    HISI_MONITOR_SWITCH_OFF,
    HISI_MONITOR_SWITCH_MCAST_DATA,       // 上报组播(广播)数据包
    HISI_MONITOR_SWITCH_UCAST_DATA,       // 上报单播数据包
    HISI_MONITOR_SWITCH_MCAST_MANAGEMENT, // 上报组播(广播)管理包
    HISI_MONITOR_SWITCH_UCAST_MANAGEMENT, // 上报单播管理包
    HISI_MONITOR_SWITCH_BUTT
} hisi_monitor_switch_enum;

typedef enum {
    HISI_KEEPALIVE_OFF,
    HISI_KEEPALIVE_ON,
    HISI_KEEPALIVE_BUTT
} hisi_keepalive_switch_enum;

typedef int (*hisi_upload_frame_cb)(void *frame, unsigned int len);


/* 打印级别 */
typedef enum {
    HISI_MSG_EXCESSIVE = 0,
    HISI_MSG_MSGDUMP = 1,
    HISI_MSG_DEBUG = 2,
    HISI_MSG_INFO = 3,
    HISI_MSG_WARNING = 4,
    HISI_MSG_ERROR = 5,
    HISI_MSG_OAM_BUTT
} e_hisi_msg_type_t;

typedef struct databk_addr_info *(*get_databk_addr_info)(void);

typedef enum {
    HISI_WOW_EVENT_ALL_CLEAR = 0,               /* Clear all events */
    HISI_WOW_EVENT_MAGIC_PACKET = 1 << 0,       /* Wakeup on Magic Packet */
    HISI_WOW_EVENT_NETPATTERN_TCP = 1 << 1,     /* Wakeup on TCP NetPattern */
    HISI_WOW_EVENT_NETPATTERN_UDP = 1 << 2,     /* Wakeup on UDP NetPattern */
    HISI_WOW_EVENT_DISASSOC = 1 << 3,           /* 去关联/去认证，Wakeup on Disassociation/Deauth */
    HISI_WOW_EVENT_AUTH_RX = 1 << 4,            /* 对端关联请求，Wakeup on auth */
    HISI_WOW_EVENT_HOST_WAKEUP = 1 << 5,        /* Host wakeup */
    HISI_WOW_EVENT_TCP_UDP_KEEP_ALIVE = 1 << 6, /* Wakeup on TCP/UDP keep alive timeout */
    HISI_WOW_EVENT_OAM_LOG_WAKEUP = 1 << 7,     /* OAM LOG wakeup */
    HISI_WOW_EVENT_SSID_WAKEUP = 1 << 8,        /* SSID Scan wakeup */
} hisi_wow_event_enum;

typedef enum {
    HISI_WOW_WKUP_REASON_TYPE_NULL = 0,               /* None */
    HISI_WOW_WKUP_REASON_TYPE_MAGIC_PACKET = 1,       /* Wakeup on Magic Packet */
    HISI_WOW_WKUP_REASON_TYPE_NETPATTERN_TCP = 2,     /* Wakeup on TCP NetPattern */
    HISI_WOW_WKUP_REASON_TYPE_NETPATTERN_UDP = 3,     /* Wakeup on UDP NetPattern */
    HISI_WOW_WKUP_REASON_TYPE_DISASSOC_RX = 4,        /* 对端去关联/去认证，Wakeup on Disassociation/Deauth */
    HISI_WOW_WKUP_REASON_TYPE_DISASSOC_TX = 5,        /* 对端去关联/去认证，Wakeup on Disassociation/Deauth */
    HISI_WOW_WKUP_REASON_TYPE_AUTH_RX = 6,            /* 本端端关联请求，Wakeup on auth */
    HISI_WOW_WKUP_REASON_TYPE_TCP_UDP_KEEP_ALIVE = 7, /* Wakeup on TCP/UDP keep alive timeout */
    HISI_WOW_WKUP_REASON_TYPE_HOST_WAKEUP = 8,        /* Host wakeup */
    HISI_WOW_WKUP_REASON_TYPE_OAM_LOG = 9,            /* OAM LOG wakeup */
    HISI_WOW_WKUP_REASON_TYPE_SSID_SCAN = 10,         /* SSID Scan wakeup */
    HISI_WOW_WKUP_REASON_TYPE_BUT
} hisi_wow_wakeup_reason_type_enum;

typedef struct _hisi_rf_customize_stru {
    int l_11b_scaling_value;
    int l_11g_u1_scaling_value;
    int l_11g_u2_scaling_value;
    int l_11n_20_u1_scaling_value;
    int l_11n_20_u2_scaling_value;
    int l_11n_40_u1_scaling_value;
    int l_11n_40_u2_scaling_value;
    int l_ban1_ref_value;
    int l_ban2_ref_value;
    int l_ban3_ref_value;
    int l_customize_enable;
    int l_disable_bw_40;
    int l_dtim_setting;
    int l_pm_switch;
    int l_rssi_descend_protocol_limit;
    int high_power_switch;
} hisi_rf_customize_stru;

typedef struct _hisi_wpa_status_stru {
    unsigned char auc_ssid[MAX_SSID_LEN];
    unsigned char auc_bssid[ETH_ADDR_LEN];
    unsigned char auc_rsv[2];
    unsigned int ul_status;
} hisi_wpa_status_stru;

typedef struct {
    unsigned char uc_channel_num;
    uint8_t uc_channel_bandwidth;
} hisi_channel_stru;

typedef struct _hisi_tcp_params_stru {
    unsigned int ul_sess_id;
    unsigned char auc_dst_mac[6];         /* 目的MAC地址 */
    unsigned char auc_resv[2];
    unsigned char auc_src_ip[4];          /* 源IP地址 */
    unsigned char auc_dst_ip[4];          /* 目的IP地址 */
    unsigned short us_src_port;           /* 源端口号 */
    unsigned short us_dst_port;           /* 目的端口号 */
    unsigned int ul_seq_num;              /* 序列号 */
    unsigned int ul_ack_num;              /* 确认号 */
    unsigned short us_window;             /* TCP窗口大小 */
    unsigned short us_retry_max_count;    /* 最大重传次数 */
    unsigned int ul_interval_timer;       /* 心跳包发送周期 */
    unsigned int ul_retry_interval_timer; /* 重传时心跳包发送周期 */
    unsigned int ul_time_value;
    unsigned int ul_time_echo;
    unsigned char *puc_tcp_payload;
    unsigned int ul_payload_len;
} hisi_tcp_params_stru;

struct databk_addr_info {
    unsigned long databk_addr;            /* flash addr, store data backup data */
    unsigned int databk_length;           /* data length, the length of the data backup data */
    get_databk_addr_info get_databk_info; /* get data backup info,include databk_addr and databk_length */
};

struct board_info {
    unsigned int wlan_irq;

    unsigned int wifi_data_intr_gpio_group;
    unsigned int wifi_data_intr_gpio_offset;

    unsigned int dev_wak_host_gpio_group;
    unsigned int dev_wak_host_gpio_offset;

    unsigned int host_wak_dev_gpio_group;
    unsigned int host_wak_dev_gpio_offset;

    void (*wifi_power_set)(unsigned char);
    void (*wifi_rst_set)(unsigned char);
    void (*wifi_sdio_detect)(void);
    void (*host_pow_off)(void);
};

struct station_info {
    int l_signal;                             /* 信号强度 */
    int l_txrate;                             /* TX速率 */
    unsigned char auc_bssid[ETH_ALEN];        /* BSSID */
    unsigned char auc_ssid[MAX_SSID_LEN + 1]; /* SSID */
};

/*
 * 功能描述: 获取wifi驱动databk addr info
 * 1.日  期: 2017年02月14日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern struct databk_addr_info *hisi_wlan_get_databk_addr_info(void);

/* ****************************************************************************
  11.3 wifi相关对外暴露接口
**************************************************************************** */
extern int hisi_wlan_wifi_deinit(void);

/* ****************************************************************************
  11.4 hilink及smartconfig相关对外暴露接口
**************************************************************************** */
/*
 * 功能描述: 设置monitor模式开关API接口
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_monitor(unsigned char monitor_switch, char rssi_level);

/*
 * 功能描述: 设置信道的API接口
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_channel(hisi_channel_stru *channel_info);

#ifndef WIN32
/*
 * 功能描述: 获取信道
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_get_channel(hisi_channel_stru *channel_info);

/*
 * 功能描述: 设置国家码
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_country(unsigned char *puc_country);

/*
 * 功能描述: 获取国家码
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern char *hisi_wlan_get_country(void);

/*
 * 功能描述: 获取收包数
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_rx_fcs_info(int *pl_rx_succ_num);

/*
 * 功能描述: 设置常发
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_always_tx(char *pc_param);

/*
 * 功能描述: 设置低功耗开关
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_pm_switch(unsigned int uc_pm_switch);

/*
 * 功能描述: 设置常发
 * 1.日  期: 2017年6月29日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_always_rx(char *pc_param);
#endif

/*
 * 功能描述: 设置mac地址
 * 1.日  期: 2017年01月19日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_set_macaddr(unsigned char *puc_mac_addr);

/*
 * 功能描述: hilink或smartconfig上报数据帧的回调
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern unsigned int hisi_wlan_register_upload_frame_cb(hisi_upload_frame_cb func);

/* ****************************************************************************
  11.5 TCP保活相关对外暴露接口
**************************************************************************** */
/*
 * 功能描述: 设置保活TCP链路参数的API接口
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_tcp_params(hisi_tcp_params_stru *tcp_params);

/*
 * 功能描述: 设置保活TCP链路功能开关的API接口
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_set_keepalive_switch(unsigned char keepalive_switch, unsigned int keepalive_num);

/*
 * 功能描述: 获取Mac地址
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern unsigned char *hisi_wlan_get_macaddr(void);

/*
 * 功能描述: 通知驱动IP地址变化
 * 1.日  期: 2017年01月16日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_ip_notify(unsigned int ip, unsigned int mode);

/*
 * 功能描述: 获取TCP断链后tcp断链ID
 * 1.日  期: 2017年02月20日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_get_lose_tcpid(void);

/*
 * 功能描述: 获取wifi驱动board info handler
 * 1.日  期: 2017年02月20日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern struct board_info *hisi_get_board_info_handler(void);

/*
 * 功能描述: wifi驱动无文件系统时文件保存配置
 * 1.日  期: 2017年02月20日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern void hisi_wlan_no_fs_config(unsigned long ul_base_addr, unsigned int u_length);

/*
 * 功能描述: 获取station信息的API接口(只支持station模式)
 * 1.日  期: 2017年7月1日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern int hisi_wlan_get_station(struct station_info *pst_sta);

/*
 * 功能描述: 调用wifi_debug接口
 * 1.日  期: 2017年7月26日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern void hisi_wifi_debug_info(void);

/*
 * 功能描述: 切换打印级别
 * 1.日  期: 2018年5月17日
 * 作    者: wifi
 * 修改内容: 新增函数
 */
extern void hisi_log_level_switch(unsigned int ul_level);

/*
 * 功能描述: 启动网卡配置命令
 * 1.日  期: 2016年6月7日
 * 修改内容: 新增函数
 */
extern void hisi_netcfg(char *pc_netdev_name, char *pc_mode);

/*
 * 功能描述: DC和单音测试
 * 1.日  期: 2016年6月7日
 * 修改内容: 新增函数
 */
extern void hisi_wlan_dc_test(int l_channel, char *pc_mode, char *pc_type);

/*
 * 功能描述: 打开OTA测试
 * 1.日  期: 2019年6月22日
 * 修改内容: 新增函数
 */
extern int hisi_wlan_ota_test(void);

/* ****************************************************************************
 驱动对外提供的OAM维测日志接口
**************************************************************************** */
#if (_HI113X_SW_VERSION == _HI113X_SW_DEBUG)
extern unsigned int oam_log_sdt_out(unsigned short us_level, const signed char *pc_func_name,
    const signed char *pc_fmt, ...);
#define wpa_printf(level, fmt, ...)                                                                     \
    do {                                                                                                \
        oam_log_sdt_out((unsigned short)level, (const signed char *)__func__, (const signed char *)fmt, \
            ##__VA_ARGS__);                                                                             \
    } while (0)
#elif (_HI113X_SW_VERSION == _HI113X_SW_RELEASE)
extern void wpa_printf(int level, const char *fmt, ...);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* DRIVER_HISI_LIB_API_H_ */
