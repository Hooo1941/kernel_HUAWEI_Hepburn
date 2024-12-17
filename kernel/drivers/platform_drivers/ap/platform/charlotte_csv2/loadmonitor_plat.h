#ifndef LOADMONITOR_PLAT_H
#define LOADMONITOR_PLAT_H 
#include <freqdump_plat_common.h>
#define CONFIG_ONSYS_AVS_LOADMONITOR 
#define CONFIG_DUBAI_LOADMONITOR 
#define MONITOR0_IP_MASK 0xFFFFFFFE
#define MAX_PERI_MONITOR0_DEVICE bit_count(MONITOR0_IP_MASK)
#ifdef CONFIG_ONSYS_AVS_LOADMONITOR
#define MONITOR1_IP_MASK 0x0
#define MAX_PERI_MONITOR1_DEVICE bit_count(MONITOR1_IP_MASK)
#else
#define MONITOR1_IP_MASK 0xFFFFFFFF
#define MAX_PERI_MONITOR1_DEVICE bit_count(MONITOR1_IP_MASK)
#endif
#define AO_MONITOR0_IP_MASK 0xFF01FF77
#define MAX_MONITOR_IP_DEVICE_AO bit_count(AO_MONITOR0_IP_MASK)
#define MAX_PERI_MONITOR_DEVICE (MAX_PERI_MONITOR0_DEVICE + MAX_PERI_MONITOR1_DEVICE)
#define MAX_MONITOR_IP_DEVICE (MAX_PERI_MONITOR_DEVICE + MAX_MONITOR_IP_DEVICE_AO)
#define LOADMONITOR_PERI_FREQ_CS 166000000
#define MONITOR_PERI_FREQ_CS 166000000
#define LOADMONITOR_PCLK 166
#define LOADMONITOR_AO_FREQ 61440000
#define MONITOR_AO_FREQ_CS 61440000
#define LOADMONITOR_AO_CLK 61440
#define CHANNEL_SIZE 16
#define CHANNEL_SUM CHANNEL_SIZE
#define PERI_VOTE_DATA 32U
#define DPM_PERIOD_MAX 4150000000
#define DPM_PERIOD_MIN 16600000
enum EN_FLAGS {
 ENABLE_NONE = 0x0,
 ENABLE_PERI0 = 0x2,
 ENABLE_PERI1 = 0x4,
 ENABLE_PERI = ENABLE_PERI0 | ENABLE_PERI1,
 ENABLE_AO = 0x10,
 ENABLE_DDR = 0x10000,
 ENABLE_VOTE = 0x20000,
 ENABLE_ALL = ENABLE_PERI | ENABLE_AO | ENABLE_DDR | ENABLE_VOTE,
};
struct loadmonitor_ips {
 struct ips_info peri0;
 struct ips_info peri1;
 struct ips_info ao;
};
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_vote_vol_rank_0 : 2;
        unsigned int peri_vote_vol_rank_1 : 2;
        unsigned int peri_vote_vol_rank_2 : 2;
        unsigned int peri_vote_vol_rank_3 : 2;
        unsigned int peri_vote_vol_rank_4 : 2;
        unsigned int peri_vote_vol_rank_5 : 2;
        unsigned int peri_vote_vol_rank_6 : 2;
        unsigned int peri_vote_vol_rank_7 : 2;
        unsigned int reserved : 16;
    } reg;
} PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_UNION;
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_vote_vol_rank_8 : 2;
        unsigned int peri_vote_vol_rank_9 : 2;
        unsigned int peri_vote_vol_rank_10 : 2;
        unsigned int peri_vote_vol_rank_11 : 2;
        unsigned int peri_vote_vol_rank_12 : 2;
        unsigned int peri_vote_vol_rank_13 : 2;
        unsigned int peri_vote_vol_rank_14 : 2;
        unsigned int peri_vote_vol_rank_15 : 2;
        unsigned int reserved : 16;
    } reg;
} PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_UNION;
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_vote_vol_valid_0 : 1;
        unsigned int peri_vote_vol_valid_1 : 1;
        unsigned int peri_vote_vol_valid_2 : 1;
        unsigned int peri_vote_vol_valid_3 : 1;
        unsigned int peri_vote_vol_valid_4 : 1;
        unsigned int peri_vote_vol_valid_5 : 1;
        unsigned int peri_vote_vol_valid_6 : 1;
        unsigned int peri_vote_vol_valid_7 : 1;
        unsigned int peri_vote_vol_valid_8 : 1;
        unsigned int peri_vote_vol_valid_9 : 1;
        unsigned int peri_vote_vol_valid_10 : 1;
        unsigned int peri_vote_vol_valid_11 : 1;
        unsigned int peri_vote_vol_valid_12 : 1;
        unsigned int peri_vote_vol_valid_13 : 1;
        unsigned int peri_vote_vol_valid_14 : 1;
        unsigned int peri_vote_vol_valid_15 : 1;
        unsigned int reserved : 16;
    } reg;
} PMCTRL_PERI_VOTE_VOL_VALID_UNION;
struct dubai_peri {
 PMCTRL_PERI_VOTE_VOL_RANK_SFT_0_UNION vote0;
 PMCTRL_PERI_VOTE_VOL_RANK_SFT_1_UNION vote1;
 PMCTRL_PERI_VOTE_VOL_VALID_UNION enable;
};
struct loadmonitor_data {
 struct loadmonitor_ips ips;
 struct loadmonitor_ddr ddr;
 struct dubai_peri peri;
};
#define MAX_NUM_LOGIC_IP_DEVICE (sizeof(struct loadmonitor_ips) / \
     sizeof(struct ips_info) * MAX_SENSOR_NUM)
struct delay_time_with_freq {
 unsigned int monitor_enable_flags;
 unsigned int peri0;
 unsigned int peri1;
 unsigned int ao;
};
#endif
