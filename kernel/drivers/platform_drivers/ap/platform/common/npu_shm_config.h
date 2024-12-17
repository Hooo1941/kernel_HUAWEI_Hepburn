#ifndef __NPU_SHM_CONFIG_H__
#define __NPU_SHM_CONFIG_H__ 
#include <npu_ddr_map.h>
#define NPU_LOAD_TSFW_CONFIG_SIZE 0x1000
struct npu_chip_cfg {
 unsigned int valid_magic;
 unsigned int aicore_disable_bitmap;
 unsigned int platform_specification;
};
typedef union {
 struct {
  struct npu_chip_cfg chip_cfg;
 } cfg;
 unsigned char reserved[NPU_S_NPU_CONFIG_SIZE];
} npu_shm_cfg;
#pragma pack(push)
#pragma pack(4)
typedef union
{
 struct {
  unsigned char load_random;
  unsigned int canary;
  volatile unsigned int boot_status;
  volatile unsigned int ts_pd_stage;
 } cfg;
 unsigned char reserved[NPU_LOAD_TSFW_CONFIG_SIZE];
} npu_load_tsfw_cfg;
#pragma pack(pop)
#endif
