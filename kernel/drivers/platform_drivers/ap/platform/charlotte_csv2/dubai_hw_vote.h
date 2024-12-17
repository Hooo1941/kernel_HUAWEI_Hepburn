#ifndef DUBAI_HW_VOTE_H
#define DUBAI_HW_VOTE_H 
#include "ddr_vote_statistic_struct.h"
#define DDR_VOTE_NUM 24
static struct ddr_vote_ip g_ddr_vote_table[DDR_VOTE_NUM] = {
  { LIT_CLUSTER_ID, "lit_cluster" },
  { BIG_CLUSTER_ID, "big_cluster" },
  { GPU_ID, "gpu" },
  { IOMCU_ID, "iomcu" },
  { MODEM_ID, "modem" },
  { TEMPERTURE_ID, "temperture" },
  { MID_CLUSTER_ID, "mid_cluster" },
  { L3_CLUSTER_ID, "l3_cluster" },
  { MODEM_5G_ID, "modem_5g"},
  { FLOOR_VOTE_HW_AP_ID, "hw_ap" },
  { FLOOR_VOTE_HW_HIFI_ID, "hw_hifi" },
  { FLOOR_VOTE_HW_IOMCU_ID, "hw_iomcu" },
  { FLOOR_VOTE_HW_LIT_LAT_ID, "hw_lit_lat" },
  { FLOOR_VOTE_HW_BIG_LAT_ID, "hw_big_lat" },
  { FLOOR_VOTE_HW_MID_LAT_ID, "hw_mid_lat" },
  { FLOOR_VOTE_HW_LATSTAT_ID, "hw_latstat" },
  { FLOOR_VOTE_HW_MODEM_ID, "hw_modem" },
  { FLOOR_VOTE_HW_CPU_XPU_ID, "hw_cpu_xpu" },
  { FLOOR_VOTE_HW_GPU_XPU_ID, "hw_gpu_xpu" },
 { FLOOR_VOTE_HW_RESERVED11_ID, "hw_reserved11" },
 { FLOOR_VOTE_HW_RESERVED12_ID, "hw_reserved12" },
 { FLOOR_VOTE_HW_RESERVED13_ID, "hw_reserved13" },
 { FLOOR_VOTE_HW_RESERVED14_ID, "hw_reserved14" },
 { FLOOR_VOTE_HW_DSS_ID, "hw_dss" },
};
#endif
