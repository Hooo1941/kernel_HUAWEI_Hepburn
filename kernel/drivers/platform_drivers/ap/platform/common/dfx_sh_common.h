#ifndef __DFX_SH_COMMON_H__
#define __DFX_SH_COMMON_H__ 
#define FREQ_MAX_NUM_CPU_LIT 20
#define FREQ_MAX_NUM_CPU_MID 20
#define FREQ_MAX_NUM_CPU_BIG 20
#define FREQ_MAX_NUM_L3 20
#define FREQ_MAX_NUM_PERI 10
#define SH_CFG_INFO 513
#define SH_CFG_PV 514
#define CPU_SELF_HEALING_MAGIC 0x12889641
#define CPU_SH_MAX_CLUSTER_NUM 15
enum sh_type {
 sh_type_cpu_lit,
 sh_type_cpu_mid,
 sh_type_cpu_big,
 sh_type_l3,
 sh_type_peri,
 sh_type_max,
};
struct sh_sw_cnt {
 unsigned char count:3;
 unsigned char threshold:3;
 unsigned char reserved:1;
 unsigned char enable:1;
};
struct sh_clear_enable {
 unsigned char cpu_lit:1;
 unsigned char cpu_mid:1;
 unsigned char cpu_big:1;
 unsigned char peri:1;
 unsigned char reserved:4;
};
struct sh_cfg_info {
 struct sh_sw_cnt sw_cnt[sh_type_max];
 unsigned int panic_time_cpu_lit;
 unsigned int panic_time_cpu_mid;
 unsigned int panic_time_cpu_big;
 unsigned int panic_time_peri;
 struct sh_clear_enable clear_interval_enable;
 unsigned int clear_interval_cpu_lit;
 unsigned int clear_interval_cpu_mid;
 unsigned int clear_interval_cpu_big;
 unsigned int clear_interval_peri;
 struct sh_clear_enable clear_enable;
 unsigned int version_rtc_time;
 unsigned char version;
}__attribute__((aligned(1)));
#endif
