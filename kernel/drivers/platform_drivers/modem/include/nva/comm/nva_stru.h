/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 */

#ifndef __NVA_STRU_H__
#define __NVA_STRU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * 示例
 */
typedef struct {
    unsigned int test1; /* 功能开关，1:开，0:关 */
    unsigned int reserved0;
    unsigned int reserved1;
    unsigned int reserved2;
} drv_example_s;

/* C核单独复位按需开关特性配置NV项 = 0xd134
 * 结构说明  : 控制C核单独复位特性是否打开，以及是否与接收来自ril的单独复位请求。
 */
typedef struct {
    /*
     * 是否开启C核单独复位特性。
     * 0：关
     * 1：开
     */
    unsigned int is_feature_on : 1;
    /*
     * 是否接收来自ril的C核单独复位请求。
     * 0：否
     * 1：是
     */
    unsigned int is_connected_ril : 1;
    /*
     * ccore 进idle超时是否整机复位。
     * 0：否
     * 1：是
     */
    unsigned int is_idle_timerout_reset : 1;
    unsigned int reserve : 29; /* bit 2-31 */
} drv_ccore_reset_stru_s;


typedef struct {
    unsigned int wait_usr_sele_uart : 1;
    unsigned int a_core_uart_num : 2;
    unsigned int c_core_uart_num : 2;
    unsigned int m_core_uart_num : 2;
    unsigned int a_shell : 1;
    unsigned int c_shell : 1;
    unsigned int uart_at : 1;
    unsigned int extendedbits : 22;
} drv_uart_shell_stru_s;

typedef struct {
    /*
     * 是否开启UART拨号特性。
     * 0：关
     * 1：开
     */
    unsigned int is_feature_on : 1;
    unsigned int reserve : 31;
} drv_dialup_uart_stru_s;

typedef struct {
    unsigned int ddr_dfs : 1;
    unsigned int ddr_high_tmon : 1;
    unsigned int ddr_ht_level : 3;
    unsigned int ddr_ht_freq_dl : 3;
    unsigned int ddr_low_tmon : 1;
    unsigned int lowtemp_data_flag : 1;
    unsigned int force_lowtemp : 1;
    unsigned int ddr_autogt_on : 1;
    unsigned int load_policy_on : 1;
    unsigned int ddr_freq_limit : 1;
    unsigned int reserved : 18;
} drv_ddr_switch_s;

typedef struct {
    unsigned int sleep;
    unsigned int dfs;
    unsigned int reserve1;
    unsigned int reserve2;
} drv_acore_pm_nv_s;

typedef struct {
    unsigned int up_threshold;   /* 负载上限，高于它需要把频率上调 */
    unsigned int down_threshold; /* 负载下限，低于它需要把频率下调 */
    unsigned int gap;            /* 负载调频频率间隔，当负载低于下限时，调整频率=当前频率-gap */
    unsigned int period;         /* 负载统计周期，单位ms */
    unsigned int cpumask;        /* 监控负载的cpumask，从低到高每bit代表对应thread是否需要监控 */
    unsigned int load_policy;    /* 负载统计方法 0：内核统计，1：硬件统计 */
} drv_acore_dfs_load_s;

typedef struct {
    unsigned int sleep_debug;
    unsigned int dfs_debug;
    unsigned int reserve1;
    unsigned int reserve2;
} drv_acore_pm_debug_nv_s;

#define AMON_NAME_LEN 16
/* amon监控ID NV配置 */
typedef struct {
    unsigned int id;
    unsigned int id_enable; /* 是否启动该id的监控功能 */
    unsigned int port;      /* 监控端口 */
    unsigned int master_id; /* 监控masterid */
    char master_name[AMON_NAME_LEN];
    unsigned int start_addr; /* 监控起始地址 */
    unsigned int end_addr;
    unsigned int opt_type; /* 监控的操作类型，01:读，10:写，11:读写，其余值:不监控 */
    unsigned int cnt_type; /* 复位标志，匹配到监控条件是否复位 */
} amon_ap_nv_id_cfg_s;

typedef struct {
    unsigned int amon_enable;
    amon_ap_nv_id_cfg_s id_cfg[0x8];
} amon_ap_nv_cfg_s;

/* 结构说明  : 内部使用 */
typedef struct {
    amon_ap_nv_cfg_s amon_nv_cfg[1];
} drv_amon_ap_nv_cfg_s;

typedef struct {
    unsigned int ap_enable : 1; /* Ap coresight使能控制 1：使能，0：不使能 */
    unsigned int cp_enable : 1; /* CP coersight使能控制1：使能，0：不使能 */
    unsigned int ap_store : 1;  /* AP coresight store flag */
    unsigned int cp_store : 1;  /* Cp etb数据保存控制1：使能，0：不使能 */
    unsigned int reserved : 28;
} drv_coresight_cfg_s;

/* watchpoint begin */
typedef struct {
    unsigned int enable; /* 使能标志，0，去使能；1，使能， 如果该标志为去使能，配置庇忽略 */
    unsigned int type;       /* 监控类型: 1，读；2，写；3，读写 */
    unsigned int start_addr; /* 监控起始地址 */
    unsigned int end_addr;   /* 监控崾刂? */
} watchpoint_cfg_s;
typedef struct {
    unsigned int enable; /* 使能标志，0，去使能；1，使能， 如果该标志为去使能，配置庇忽略 */
    unsigned int type;            /* 监控类型: 1，读；2，写；3，读写 */
    unsigned int start_addr_low;  /* 监控起始地址低32bit */
    unsigned int start_addr_high; /* 监控起始地址高32bit */
    unsigned int end_addr_low;    /* 监控崾刂返?2bit */
    unsigned int end_addr_high;   /* 监控崾刂犯?2bit */
} watchpoint_cfg_64_s;

typedef struct {
    watchpoint_cfg_64_s ap_cfg[4]; /* A核配置，最多支持4个watchpoint */
    watchpoint_cfg_s cp_cfg[4];    /* A核配置，最多支持4个watchpoint */
} drv_watchpoint_cfg_s;

/* TEST support module nv define */
/* 结构说明  : 该NV实现控制各个模块是否打开的功能，1：打开；0：关闭。
 * 和drv_module_support_s不同，这个nv仅作为测试功能
 * */
typedef struct {
    unsigned int lcd : 1;  /* 是否支持lcd模块，1: support; 0: not support */
    unsigned int emi : 1;  /* 是否支持emi模块 */
    unsigned int pwm : 1;  /* 是否支持pwm模块 */
    unsigned int i2c : 1;  /* 是否支持i2c模块 */
    unsigned int leds : 1; /* 是否支持leds模块 */
    unsigned int reserved : 27;
} drv_module_dev_test_s;

/*
 * 结构名    : LED_CONTROL_NV_STRU
 * 结构说明  : LED_CONTROL_NV结构
 */
typedef struct {
    unsigned char ucLedColor;   /* 三色灯颜色，对应LED_COLOR的值 */
    unsigned char ucTimeLength; /* 该配置持续的时间长度，单位100ms */
} drv_led_control_s;

/* 结构说明  : 用来标识产品板各硬件模块是否支持，1：支持，0不支持。
 * 各产品形态可以在该产品的NV DIFF文件中，根据硬件ID号来定制是否支持某个模块，也可据此扩展对应产品形态特有的其他模块。
 */
typedef struct {
    /*
     * 是否支持SD卡。
     * 1：支持；0：不支持。
     */
    unsigned int sdcard : 1;
    unsigned int charge : 1;     /* 是否支持充电。 */
    unsigned int wifi : 1;       /* 是否支持Wi-Fi。 */
    unsigned int oled : 1;       /* 是否支持LCD、OLED等屏。 */
    unsigned int hifi : 1;       /* 是否支持Hi-Fi语音模块。 */
    unsigned int onoff : 1;      /* 是否支持开关机操作。 */
    unsigned int hsic : 1;       /* 是否HSIC接口。 */
    unsigned int localflash : 1; /* 是否支持本地Flash存储。 */
    unsigned int reserved : 24;  /* 保留bit位，供扩展使用。 */
} drv_module_support_s;

/* 结构说明  : HKADC的物理通道与逻辑通道对应关系。 */
typedef struct {
    unsigned char
        chan_map[32]; /* 32个HKADC物理通道和逻辑通道对应关系。数组的索引为HKADC的逻辑通道号；数组中的值为物理通道号。
                       */
} temp_hkadc_phy_logic_array_s;

typedef struct {
    signed int temperature;
    unsigned short code;
    unsigned short reserved;
} drv_convert_table_type;

#define XO_TABLE_SIZE 166
/* 结构说明：内部使用 */
typedef struct {
    drv_convert_table_type convert_table[XO_TABLE_SIZE];
} drv_xo_convert_table;
#define PA_TABLE_SIZE 32
/* 结构说明：内部使用 */
typedef struct {
    drv_convert_table_type convert_table[PA_TABLE_SIZE];
} drv_pa_convert_table;

/* 结构说明：内部使用 */
typedef struct {
    unsigned int smp_sch : 1;
    unsigned int smp_hp : 1;
    unsigned int mulwan : 1;
    unsigned int pfa_tft_extdesc : 1;
    unsigned int ack_extdesc : 1;
    unsigned int direct_fw : 1;
    unsigned int reserve : 26;
} drv_pfa_feature_s;

/* ID=0xd111 begin */
typedef struct {
    /*
     * Dump模式。
     * 00: excdump；
     * 01: usbdump；
     * 1x: no dump。
     */
    unsigned int dump_switch : 2;
    unsigned int ARMexc : 1;       /* ARM异常检测开关。 */
    unsigned int stackFlow : 1;    /* 堆栈溢出检测开关。 */
    unsigned int taskSwitch : 1;   /* 任务切换记录开关。 */
    unsigned int intSwitch : 1;    /* 中断记录开关。 */
    unsigned int intLock : 1;      /* 锁中断记录开关。 */
    unsigned int appRegSave1 : 1;  /* ACORE寄存器组1记录开关。 */
    unsigned int appRegSave2 : 1;  /* ACORE寄存器组2记录开关。 */
    unsigned int appRegSave3 : 1;  /* ACORE寄存器组3记录开关。 */
    unsigned int commRegSave1 : 1; /* CCORE寄存器组1记录开关。 */
    unsigned int commRegSave2 : 1; /* CCORE寄存器组2记录开关。 */
    unsigned int commRegSave3 : 1; /* CCORE寄存器组3记录开关。 */
    unsigned int sysErrReboot : 1; /* SystemError复位开关。 */
    unsigned int reset_log : 1;    /* 强制记录开关，暂未使用。 */
    unsigned int fetal_err : 1;    /* 强制记录开关，暂未使用。 */
    unsigned int log_ctrl : 2;     /* log开关。 */
    unsigned int dumpTextClip : 1; /* ddr保存时text段裁剪特性 */
    unsigned int secDump : 1;
    unsigned int mccnt : 1;
    unsigned int pccnt : 1;
    unsigned int rrt : 1;
    unsigned int reserved1 : 9; /* 保留。 */
} dump_cfg_s;

/* 结构说明  : 用来控制可维可测功能。 */
typedef struct {
    unsigned int thread0_channel : 1;
    unsigned int thread1_channel : 1;
    unsigned int thread2_channel : 1;
    unsigned int thread3_channel : 1;
    unsigned int thread4_channel : 1;
    unsigned int thread5_channel : 1;
    unsigned int thread6_channel : 1;
    unsigned int thread7_channel : 1;
    unsigned int thread8_channel : 1;
    unsigned int thread9_channel : 1;
    unsigned int thread10_channel : 1;
    unsigned int thread11_channel : 1;
    unsigned int thread12_channel : 1;
    unsigned int thread13_channel : 1;
    unsigned int thread14_channel : 1;
    unsigned int thread15_channel : 1;
    unsigned int reserved : 16; /* 保留。 */
} dump_schdule_ctrl_s;


/* 结构说明  : 用来控制可维可测功能。 */
typedef struct {
    union {
        unsigned int uintValue;
        dump_cfg_s Bits;
    } dump_cfg;
    union {
        unsigned int thread_cfg;
        dump_schdule_ctrl_s thread_bits;
    } schudule_cfg;

    unsigned int appRegAddr1; /* ACORE保存寄存器组地址1 */
    unsigned int appRegSize1; /* ACORE保存寄存器组长度1 */
    unsigned int appRegAddr2; /* ACORE保存寄存器组地址2 */
    unsigned int appRegSize2; /* ACORE保存寄存器组长度2 */
    unsigned int appRegAddr3; /* ACORE保存寄存器组地址3 */
    unsigned int appRegSize3; /* ACORE保存寄存器组长度3 */

    unsigned int commRegAddr1; /* CCORE保存寄存器组地址1 */
    unsigned int commRegSize1; /* CCORE保存寄存器组长度1 */
    unsigned int commRegAddr2; /* CCORE保存寄存器组地址2 */
    unsigned int commRegSize2; /* CCORE保存寄存器组长度2 */
    unsigned int commRegAddr3; /* CCORE保存寄存器组地址3 */
    unsigned int commRegSize3; /* CCORE保存寄存器组长度3 */

    /*
     * Trace采集开关。
     * 1:开机启动Trace；
     * 非1:开机不启动Trace。
     */
    unsigned int traceOnstartFlag;
    /*
     * Trace采集配置。
     * bit1：采集app核；
     * bit2：采集modem核。
     */
    unsigned int traceCoreSet;
    /*
     * 防总线挂四功能开关。
     * 0：开机不启动防总线挂死功能；
     * 非0：开机启动防总线挂死功能。
     */
    unsigned int busErrFlagSet;
} nv_dump_s;

/*
 * 结构说明  : 本NV项的功能是实现DMSS中断处理流程的使能控制和触发DMSS中断后是否复位的控制；
 */
typedef struct {
    unsigned int enable; /* enable用于使能DMSS中断处理流程，若要使能，配置enable为0x1，不使能配置为0x0 */
    unsigned int
        reset_enable; /* reset_enable用于控制触发DMSS中断后是否复位，如要复位，配置reset_enable为0x1，不复位则配置为0x0
                       */
} drv_dmss_cfg_s;

typedef struct {
    unsigned int mdm_dump : 1;     /* modem_dump.bin */
    unsigned int mdm_sram : 1;     /* modem_sram.bin */
    unsigned int mdm_share : 1;    /* modem_share.bin */
    unsigned int mdm_ddr : 1;      /* modem_ddr.bin */
    unsigned int lphy_tcm : 1;     /* lphy_tcm.bin */
    unsigned int lpm3_tcm : 1;     /* lpm3_tcm.bin */
    unsigned int ap_etb : 1;       /* ap_etb.bin */
    unsigned int mdm_etb : 1;      /* modem_etb.bin */
    unsigned int reset_log : 1;    /* reset.log */
    unsigned int cphy_tcm : 1;     /* cphy_tcm.bin */
    unsigned int mdm_secshare : 1; /* secshare.bin */
    unsigned int mdm_dts : 1;      /* modem_dts.bin */
    unsigned int llram_share : 1;  /* modem_llram.bin */
    unsigned int reserved2 : 19;
} dump_file_bits_s;

/*
 * 结构说明  : 可维可测设置当前需要保存的异常文件NV。
 */
typedef struct {
    union {
        unsigned int file_value;
        /*
         * bit0：是否保存modem_dump.bin异常文件。
         * bit1：是否保存modem_sram.bin。
         * bit2：是否保存modem_share.bin异常文件。
         * bit3：是否保存modem_ddr.bin异常文件。
         * bit4：是否保存lphy_tcm.bin异常文件。
         * bit5：是否保存lpm3_tcm.bin异常文件。
         * bit6：是否保存ap_etb.bin异常文件。
         * bit7：是否保存modem _etb.bin异常文件。
         * bit8: 是否保存reset.log文件
         * bit9：是否保存cphy_tcm.bin异常文件。
         * bit10：是否保存modem_secshare.bin异常文件。
         * 其它保留。
         */
        dump_file_bits_s file_bits;
    } file_list;
    unsigned int file_cnt; /* 记录异常文件份数 */
} dump_file_cfg_s;

/*
 * 结构说明  : 该NV实现对各组件进行开机log通知和各组件开机log级别配置的功能
 */
typedef struct {
    /*
     * 0：关闭开机LOG特性；
     * 1：打开开机LOG特性。
     */
    unsigned char cMasterSwitch;
    /*
     * 0：开机LOG内存不可用；
     * 1：开机LOG内存可用。
     */
    unsigned char cBufUsable;
    /*
     * 0：开机LOG内存不使能；
     * 1：开机LOG内存使能。
     */
    unsigned char cBufEnable;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswACPUBsp;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswACPUDiag;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswACPUHifi;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswLRMBsp;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswLRMDiag;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswLRMTLPhy;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswLRMGUPhy;
    unsigned char cswLRMCPhy; /* Range:[0,3] 4G Modem CPHY开机log profile */
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswLRMEasyRf;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswNRMBsp;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswNRMDiag;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswNRMHAC;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswNRMPhy;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswNRMHL1C;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    unsigned char cswNRMPDE;
    unsigned char cReserved[6]; /* 保留6字段 */
} drv_nv_power_on_log_switch_s;

#define PRODUCT_NAME_LEN 32
/* 结构说明  : 控制硬件版本号。 */
typedef struct {
    unsigned int index;                      /* 硬件版本号数值(大版本号1+大版本号2)，区分不同产品 */
    unsigned int hw_id_sub;                  /* 硬件子版本号，区分产品的不同的版本 */
    char name[PRODUCT_NAME_LEN];             /* 内部产品名 */
    char name_plus[PRODUCT_NAME_LEN];        /* 内部产品名PLUS */
    char hw_ver[PRODUCT_NAME_LEN];           /* 硬件版本名称 */
    char download_id[PRODUCT_NAME_LEN];      /* 升级中使用的名称 */
    char product_id[PRODUCT_NAME_LEN];       /* 外部产品名 */
} product_info_nv_s;

/* 结构说明  : 该NV实现控制HKADC校准策略版本号 */
typedef struct {
    unsigned char version; /* 当版本号为1时，hkadc使用新的校准策略 */
    unsigned char rsv1;
    unsigned char rsv2;
    unsigned char rsv3;
} drv_hkadc_cal_version_s;

/* 结构说明  : M3上控制低功耗的NV。 */
typedef struct {
    /*
     * M3是否进Normal wfi。
     * 0表示不进；
     * 1表示进。
     */
    unsigned char normalwfi_flag;
    /*
     * M3是否进深睡的标记。
     * 0表示不进；
     * 1表示进。
     */
    unsigned char deepsleep_flag;
    /*
     * M3 是否走Buck3掉电的流程。
     * 0表示Buck3不掉电；
     * 1表示Buck3掉电。
     */
    unsigned char buck3off_flag;
    /*
     * M3 如果Buck3不掉电的话，外设是否掉电。
     * 0表示外设不掉电；
     * 1表示外设掉电（注意，如果Buck3掉电的话，此NV配置不起作用，Buck3掉电，外设一定掉电）。
     */
    unsigned char peridown_flag;
    unsigned int deepsleep_Tth; /* C核深睡时间阈值。 */
    unsigned int TLbbp_Tth;     /* C核深睡，BBP寄存器的阈值。 */
} drv_nv_pm_type_s;

/* drx delay flag */
/* 结构说明  : 低功耗delay打点NV。 */
typedef struct {
    unsigned char lpm3_flag; /* Lpm3标记，默认值为0x11，为了区别c核和msp delay阶段。 */
    /*
     * Delay开关
     * 1：打开；
     * 0：关闭。
     */
    unsigned char lpm3_0;
    /*
     * Delay开关
     * 1：打开；
     * 0：关闭。
     */
    unsigned char lpm3_1;
    /*
     * Delay开关
     * 1：打开；
     * 0：关闭。
     */
    unsigned char lpm3_2;
    /*
     * Delay开关
     * 1：打开；
     * 0：关闭。
     */
    unsigned char lpm3_3;
    /*
     * Delay开关
     * 1：打开；
     * 0：关闭。
     */
    unsigned char lpm3_4;
    /*
     * Delay开关
     * 1：打开；
     * 0：关闭。
     */
    unsigned char lpm3_5;
    unsigned char drv_flag; /* drv标记，默认值为0x22，为了区别lpm3和msp delay阶段。 */
    unsigned char drv_0;    /* 1打开delay,其他关闭delay */
    unsigned char drv_1;
    unsigned char drv_2;
    unsigned char drv_3;
    unsigned char drv_4;
    unsigned char drv_5;
    unsigned char msp_flag; /* msp标记，默认值为0x33，为了区别c核和lpm3阶段。 */
    unsigned char msp_0;    /* 1打开delay,其他关闭delay */
    unsigned char msp_1;
    unsigned char msp_2;
    unsigned char msp_3;
    unsigned char msp_4;
    unsigned char msp_5;
    unsigned char msp_6;
    unsigned char msp_7;
    unsigned char msp_8;
} drv_drx_delay_s;

/* 结构说明  : 该NV实现对nr低功耗特性的开关控制。 */
typedef struct {
    unsigned char nrcpudeepsleep; /* 控制NR R8 CPU低功耗特性开关 */
    unsigned char l2hacdeepsleep; /* 控制L2HAC R8 CPU低功耗特性开关 */
    unsigned char cpubusdfs;      /* 调频调压开关 */
    unsigned char hotplug;        /* CPU 热插拔，当前特性不用打开 */
    unsigned char tickless;       /* Tickless，当前特性不打开 */
    unsigned char drxnrbbpinit;   /* nrbbp回片初始化打桩 */
    unsigned char dxrbbpaxidfs;   /* nrbbp自动降频打桩 */
    unsigned char drxnrbbpclk;    /* 保留 */
    unsigned char drxnrbbppll;    /* bbppll开关打桩 */
    unsigned char drxnrbbppmu;    /* nrbbp上下电打桩 */
    unsigned char drxbbainit;     /* 保留 */
    unsigned char drxbbapwr;      /* bba上电打桩 */
    unsigned char drxbbaclk;      /* 保留 */
    unsigned char drxbbapll;      /* 保留 */
    unsigned char drxbbapmu;      /* 保留 */
    unsigned char drxl1c;         /* 保留 */
    unsigned char drxl1cpll;      /* 保留 */
    unsigned char reserved;       /* 保留 */
    unsigned char reserved1;      /* 保留 */
    unsigned char reserved2;      /* 保留 */
} st_pwc_nr_powerctrl_s;

/* 结构说明  : USB软硬件特性控制NV项。仅支持MBB产品。 */
typedef struct {
    /*
     * bit 0：wwan_flags。判断PC网卡是否为无线网卡。
     * 0：no；1：yes。
     * bit 1：stick mode。选择网络模式。
     * 0：stick模式；1：E5 模式。
     * bit 2：vbus connect，vbus连接芯片的vbus引脚（通常为0，部分车载为1）。
     * 0：disconnect；1：connect。
     * bit 3：regulator ctrl by usb。usb phy的供电由usb代码控制，不随peri-on。
     * 0：no；1：yes。
     * bit 4：phy_auto_pd。不使用的usb phy，随枚举完成关闭特性，不能和hibernation_support同时使用。
     * 0：no；1：yes。
     * bit 5：force fast speed。USB是否强制配置USB2.0和切换到SGMII功能。
     * 0：no；1：yes。
     * bit6：usb shell。是否使用usb shell。
     * 0：no；1：yes。
     * bit7：ssp。USB配置为usb3.1 gen10模式。
     */
    unsigned char flags;
    /*
     * bit 0：配置usbtypec正插或者反插。
     * 0：默认正插检测；1：反插检测。
     * bit 1：resv。
     *
     * bit 2：usb rndis网卡对接Linux主机端配置水线选择。
     * 0：组包；1：Linux rndis场景不组包。
     * bit 3: resv。
     *
     * bit 4:7：usb tx req。USB网卡发包req缓存数量。
     */
    unsigned char flags2;
    /*
     * usb 电源管理功能。
     * bit 0：支持usb hibernation可以令acore在usb连接时进入睡眠。
     * 0：no；1：yes。
     *
     * bit 1：charger 识别模式。
     * 0 : test volt is set to D+
     * 1 : test volt is set to D-
     *
     * bit 2：支持usb 带内唤醒。
     * 0：no；1：yes。
     */
    unsigned char power_management;
    /*
     * PC 驱动的类型。
     * 0：resv；1：自研；2：模块驱动。
     */
    unsigned char pc_driver;
    /*
     * USB插拔检测模式。
     * 0：pmu detect VBUS连接PMU，PMU上报中断检测USB插拔；
     * 1：no detect 不进行检测，USB保持常开；
     * 2：car module detect车载模块T-box检测模式，GPIO检测插入，芯片vbus检测拔出。
     */
    unsigned char detect_mode;
    /*
     * 部分版本的usb ip存在在使用USB3.0的U1U2省电模式时会重枚举的芯片逻辑问题，为了规避这个问题添加此宏。
     * 0：关闭u1u2功能；
     * 1：使能u1u2，并使能synopsis提供的规避方案；
     * 2：使能u1u2功能，不使用规避方案。
     */
    unsigned char enable_u1u2_workaround : 4;
    /*
     * USB端口定制模式。
     * 0：模式0，使能dts_cust所有端口
     * 1：模式1，instance_1
     * 2：模式2, instance_2
     * ...
     * 15：模式15, instance_15。
     */
    unsigned char port_cust_mode : 4;
    /*
     * 使用gpio端口进行usb id高低电平检测。用于OTG模式中的形态识别。
     * 0：关闭检测；
     * 1：打开检测。
     */
    unsigned char usb_gpio_support : 4;
    /*
     * USB的模式选择。
     * 0：device模式；
     * 1：host模式；
     * 2：otg模式。
     */
    unsigned char usb_mode : 4;
    /*
     * bit 0:3 : 网卡模式选择。
     * 0：NCM网卡自动协商模式，可以与主机协商为nth16或者nth32；
     * 1：RNDIS网卡模式；
     * 2：ECM网卡模式；
     * 3：NCM网卡强制模式，强制单板支持nth16。
     * bit 4:7 : USB网卡枚举数量，最高支持4个网卡。
     */
    unsigned char network_card;
} nv_usb_feature_s;

/* 结构说明 : XGMAC光口特性控制NV项，仅支持MBB产品 */
typedef struct {
    unsigned int sfp_support : 1; /* XGE光口特性开关  */
    unsigned int pma_control : 3; /* PMA控制档位选择  */
    unsigned int reserved0 : 28;  /* 保留 */
    unsigned int reserved1;       /* 保留 */
    unsigned int reserved2;       /* 保留 */
    unsigned int reserved3;       /* 保留 */
} nv_gmac_feature_s;

typedef struct {
    /* if lockirq time length greater than threshold,this cfg decide whether system error */
    unsigned int lpmcu_lockirq_syserr : 1;
    unsigned int rsv : 31;
    unsigned int lpmcu_irq_monitor_threshold; /* record lpmcu lockirq time length threshold,slice number */
} nv_lpmcu_irq_cfg_s;

#ifndef CHANNEL_MAX_NUM
#define CHANNEL_MAX_NUM 2
#define RANK_MAX_NUM 2
#define EDGETYPE_MAX_NUM 2
#define BYTELANE_MAX_NUM 2
#define CHANNEL_MAX_NUM 2
#define DDR_AVS_TEMP_RECOUP_NUM 4
#define DDR_SECTION_RDQSCYC_NUM 11
#define DDR_SECTION_BASE_NUM 1
#define DDR_SECTION_MAX_NUM (DDR_SECTION_RDQSCYC_NUM + DDR_SECTION_BASE_NUM)
#endif
#define FREQ_NT_NUM_MAX 6
#define HMAC_DIGEST_LENGTH 32
typedef struct {
    unsigned int acaddrbdl0[CHANNEL_MAX_NUM];
    unsigned int acaddrbdl1[CHANNEL_MAX_NUM];
    unsigned int acaddrbdl2[CHANNEL_MAX_NUM];
    unsigned int acaddrbdl3[CHANNEL_MAX_NUM];
    unsigned int addrphbound[CHANNEL_MAX_NUM];
    unsigned int acbdlctrl2[CHANNEL_MAX_NUM];
    unsigned int dxnwdqnbdl0[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnwdqnbdl1[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnwdqnbdl2[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnrdqnbdl0[CHANNEL_MAX_NUM][EDGETYPE_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnrdqnbdl1[CHANNEL_MAX_NUM][EDGETYPE_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnrdqnbdl2[CHANNEL_MAX_NUM][EDGETYPE_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnoebdl[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnrdqsdly[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnrdqsdlysub[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnwdqsdly[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnwdqdly[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnwlsl[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxngds[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxnrdqsgdly[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int hvreft_status[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int advref_status[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM]; /* bit 0:5 advref bit6:range */
    unsigned int dvreft_status[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM]; /* bit0:5 dvref; bit6: range */
    unsigned int ck_dcc_status[CHANNEL_MAX_NUM];
    unsigned int wdqs_dcc_status[CHANNEL_MAX_NUM][RANK_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int accmdbdl0[CHANNEL_MAX_NUM];
    unsigned int accmdbdl2[CHANNEL_MAX_NUM];
    unsigned int accmdbdl3[CHANNEL_MAX_NUM];
    unsigned int osc_count0[CHANNEL_MAX_NUM][RANK_MAX_NUM];
    unsigned int avs_rtc_time;
    unsigned int avs_volt;
    unsigned char avs_temp_recoup[DDR_AVS_TEMP_RECOUP_NUM];
    unsigned int section_impstatus[DDR_SECTION_MAX_NUM]; /* zcode add */
    unsigned short section_rdqscyc[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM][DDR_SECTION_MAX_NUM];
    unsigned int section_rdqscyc_actual_num;
    unsigned int dx_odtctrl1[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dxctl_bypass[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dx_oebdl_ctl0[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dx_rx_set[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dx_pectrl1[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dx_iorefctrl0[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dx_iorefctrl1[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
    unsigned int dx_iorefctrl2[CHANNEL_MAX_NUM][BYTELANE_MAX_NUM];
} ddr_train_param_s;

typedef struct {
    unsigned int data_flag;
    unsigned char hmac[HMAC_DIGEST_LENGTH];
    ddr_train_param_s train_data[FREQ_NT_NUM_MAX];
} nv_ddr_train_data_s;

typedef struct {
    /*
   0 : hiboot直接启动到kernel
   1 : 启动停留在hiboot
   */
    unsigned char update_mode_flag;
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
} nva_hiburn_s;

/* 结构说明：总线异常检测功能，控制中断、复位是否开启 */
typedef struct buserr_cfg {
    unsigned char int_en;    /* mbb&phone ap侧buserr 中断使能标志, 0, 去使能;  1, 使能 */
    unsigned char axi_rst;   /* mbb&phone ap侧axi总线异常复位使能标志, 0, 去使能; 1, 使能 */
    unsigned char noc_rst;   /* mbb&phone ap侧noc总线异常复位使能标志, 0, 去使能; 1, 使能 */
    unsigned char qic_rst;   /* mbb&phone ap侧qic总线异常复位使能标志, 0, 去使能; 1, 使能 */
    unsigned char other_rst; /* mbb&phone ap侧非noc/axi/qic总线异常复位使能标志, 0, 去使能; 1, 使能 */
    unsigned char rsv[3]; /* 预留3个字段 */
} buserr_cfg_s;


/* pinctrl 结构说明 */
struct pinctrl_mask_info {
    unsigned int mask_value;
    unsigned int mask_bit_offset;
};

typedef struct {
    struct pinctrl_mask_info pull_mask;
    struct pinctrl_mask_info oe_mask;
    struct pinctrl_mask_info ds_mask;
    struct pinctrl_mask_info st_mask;
} nv_pinctrl_data_s;

/* lpmcu base 结构说明 */
typedef struct {
    unsigned int acore_ddr_addr;
    unsigned int acore_tcm_addr;
    unsigned int mcore_ddr_addr;
    unsigned int mcore_tcm_addr;
} nv_lpmcu_base_data_s;

#define SYSCTRL_MAX_NUM 10
typedef struct {
    unsigned int reg_addr[SYSCTRL_MAX_NUM];
    unsigned int reg_sum;
} drv_sysctrl_nv_s;

typedef struct {
    unsigned int trace_size;
} drv_trace_nv_s;

/* memory map 结构说明 */
typedef struct {
    unsigned int acore_ddr_addr;
    unsigned int acore_tcm_addr;
    unsigned int mcore_ddr_addr;
    unsigned int mcore_tcm_addr;
} drv_memory_map_info_s;

/* usb gnet 结构说明 */
#define GNET_RESERVED_NUM 6
typedef struct {
    unsigned char om_feature;
    unsigned char ethernet_feature;
    unsigned char rsv[GNET_RESERVED_NUM];
} nv_gnet_feature_s;

/* pcie nic net 结构说明 */
#define PCIE_NIC_NET_RESERVED_NUM 7
typedef struct {
    unsigned char ethernet_feature;
    unsigned char rsv[PCIE_NIC_NET_RESERVED_NUM];
} nv_pcie_nic_net_feature_s;

/* usb vendor info */
#define USB_VENDOR_STR_INFO_NUM 32
typedef struct {
    unsigned int vendor_id;
    unsigned int product_id; /* kernel */
    unsigned char string_manu[USB_VENDOR_STR_INFO_NUM];
    unsigned char string_prod[USB_VENDOR_STR_INFO_NUM];
} drv_usb_vendor_info_s;

typedef struct {
    unsigned int onoff;
} nv_uart_2l_wakeup_s;

/* MCI通道定制，每bit对应一个通道 */
#define MCI_CUST_CHN_MAX_NUM 16
typedef struct {
    unsigned char chn_state[MCI_CUST_CHN_MAX_NUM];
} nv_mci_chn_cust_feature_s;

#define MCI_FEATURE_RESERVE_NUM 7
typedef struct {
    unsigned char switch_support; /* MCI通道是否支持AT拦截功能 */
    unsigned char reserved[MCI_FEATURE_RESERVE_NUM];
} nv_mci_feature_s;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
