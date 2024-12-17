/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_REGMAP_H
#define HVGR_REGMAP_H

#include <linux/types.h>

#include "hvgr_regmap_fcp.h"

/* Begin Register Offsets */
#define GPU_CONTROL_BASE        0x0
#define MISC_CONTROL_BASE       0x2000

#define GPU_ID                  0x000   /* (RO) GPU and revision identifier */
#define GPU_VERSION             0x004   /* (RO) */
#define L2RW_FEATURES           0x008   /* (RO) Level 2 cache features */
#define L2RO_FEATURES           0x00c   /* (RO) Level 2 cache features */
#define GPU_STATUS              0x010   /* (RO) */
#define GPU_PROT_MODE           0x014   /* (WO) */

#define GROUPS_L2_COHERENT      (1 << 0) /* Cores groups are l2 coherent */
#define NUM_L2_SLICES           1       /* One slice L2 */

#define CC_IRQ_RAWSTAT          0x018   /* (RW) */
#define CC_IRQ_CLEAR            0x01C   /* (WO) */
#define CC_IRQ_MASK             0x020   /* (RW) */
#define CC_IRQ_STATUS           0x024   /* (RO) */

#define GPC_PRESENT             0x400   /* (RO) */
#define BL_PRESENT              0x404   /* (RO) */
#define GPC_PWR_STATUS          0x408   /* (RO) */
#define BL_PWR_STATUS           0x40C   /* (RO) */
#define GPC_PWRON_CMD           0x410   /* (WO) */
#define BL_PWRON_CMD            0x414   /* (WO) */
#define GPC_PWROFF_CMD          0x418   /* (WO) */
#define BL_PWROFF_CMD           0x41C   /* (WO) */
#define SOFT_RESET_CMD          0x420   /* (WO) */

#define PWR_KEY                 0x600   /* (RO) */
#define PWR_OVERRIDE0           0x604   /* (RW) */
#define PWR_OVERRIDE1           0x608   /* (RW) */

#define GPU_TRACE_ADDR          0x800   /* (RW) Trace address configuration */
#define GPU_TRACE_DATA          0x804   /* (RW) Trace data configuration */
#define GPU_TRACE_CTRL          0x808   /* (RW) Trace control configuration */
#define GPU_TRACE_STATUS        0x80C   /* (RO) Trace status configuration */

#define GPU_DBG_CFG             0x810   /* (RW) Debug configuration settings */
#define GPU_DBG_STATUS          0x814   /* (RC) Debug status */
#define GPU_DBG_RDATA           0x818   /* (RO) Debug data */

/* MISC REG */
#define misc_control_reg(r)     (MISC_CONTROL_BASE + (r))
#define GPU_IRQ_RAWSTAT         misc_control_reg(0x000)   /* (RW) */
#define GPU_IRQ_CLEAR           misc_control_reg(0x004)   /* (WO) */
#define GPU_IRQ_MASK            misc_control_reg(0x008)   /* (RW) */
#define GPU_IRQ_STATUS          misc_control_reg(0x00C)   /* (RO) */

/* (RO) GPU exception type and fault status */
#define GPU_FAULTSTATUS         misc_control_reg(0x010)
/* (RO) GPU exception fault address, low word */
#define GPU_FAULTDATA0          misc_control_reg(0x014)
/* (RO) GPU exception fault address, high word */
#define GPU_FAULTDATA1          misc_control_reg(0x018)
/* (RO) GPU exception fault info, low word */
#define GPU_FAULTDATA2          misc_control_reg(0x01C)
/* (RO) GPU exception fault info, high word */
#define GPU_FAULTDATA3          misc_control_reg(0x020)
/* (RO) Job slots present */
#define JS_PRESENT              misc_control_reg(0x110)
/* (RO) GPC features */
#define GPC_FEATURES            misc_control_reg(0x114)
/* (RO) Maximum number of threads per core */
#define THREAD_MAX_THREADS      misc_control_reg(0x120)
/* (RO) Maximum workgroup size */
#define THREAD_MAX_WORKGROUP_SIZE misc_control_reg(0x124)
/* (RO) Maximum threads waiting at a barrier */
#define THREAD_MAX_BARRIER_SIZE misc_control_reg(0x128)
/* (RO) Thread features */
#define THREAD_FEATURES         misc_control_reg(0x12C)
/* (RO) Features of job slot 0 */
#define JS0_FEATURES            misc_control_reg(0x130)
/* (RO) Features of job slot 1 */
#define JS1_FEATURES            misc_control_reg(0x134)
/* (RO) Features of job slot 2 */
#define JS2_FEATURES            misc_control_reg(0x138)
/* (RO) Features of job slot 3 */
#define JS3_FEATURES            misc_control_reg(0x13C)
/* (RO) Features of job slot 4 */
#define JS4_FEATURES            misc_control_reg(0x140)
/* (RO) Features of job slot 5 */
#define JS5_FEATURES            misc_control_reg(0x144)
/* (RO) Features of job slot 6 */
#define JS6_FEATURES            misc_control_reg(0x148)
/* (RO) Features of job slot 7 */
#define JS7_FEATURES            misc_control_reg(0x14C)

#define js_features_reg(p, n) \
	gpu_control_reg((p), (JS0_FEATURES + ((u32)(n) << 2)))

#define GPU_COMMAND             misc_control_reg(0x160)   /* (WO) */

/* (RW) Performance counter memory region base address, low word */
#define PRFCNT_BASE_LO          misc_control_reg(0x170)
/* (RW) Performance counter memory region base address, high word */
#define PRFCNT_BASE_HI          misc_control_reg(0x174)
/* (RW) Performance counter configuration */
#define PRFCNT_CONFIG           misc_control_reg(0x178)
/* (RW) Performance counter enable flags for Job Manager */
#define PRFCNT_JM_EN            misc_control_reg(0x17C)
/* (RW) Performance counter enable flags for shader cores */
#define PRFCNT_GPC_EN           misc_control_reg(0x180)
/* (RW) Performance counter enable flags for tiler */
#define PRFCNT_BTC_EN           misc_control_reg(0x184)
/* (RW) Performance counter enable flags for BVH */
#define PRFCNT_BVH_EN           misc_control_reg(0x188)
/* (RW) Performance counter enable flags for MMU/L2 cache */
#define PRFCNT_MMU_L2_EN        misc_control_reg(0x18C)
/* (RW) Power meter mode dump time interval */
#define PRFCNT_TIME_SPAN        misc_control_reg(0x190)
/* (RW) Power meter mode ep mask register */
#define PRFCNT_EP_MASK          misc_control_reg(0x194)
/* (RO) Cycle counter register */
#define CYCLE_COUNT_LO          misc_control_reg(0x1A0)
#define CYCLE_COUNT_HI          misc_control_reg(0x1A4)
/* (RO) Global time stamp counter */
#define TIMESTAMP_LO            misc_control_reg(0x1A8)
#define TIMESTAMP_HI            misc_control_reg(0x1AC)
/* (RW) Job Manager configuration register (Implementation specific register) */

#define JM_CONFIG               misc_control_reg(0x200)
#define JM_CONFIG_1             misc_control_reg(0x204)
#define JM_CONFIG_2             misc_control_reg(0x208)
#define JM_CONFIG_3             misc_control_reg(0x20C)

/* JM_MISC_CTRL CBIT REG */
/* (RW) Configuration of the L2 cache and MMU */
#define L2_MMU_CONFIG_0         misc_control_reg(0x220)
#define L2_MMU_CONFIG_1         misc_control_reg(0x224)
#define L2_MMU_CONFIG_2         misc_control_reg(0x228)
#define L2_MMU_CONFIG_3         misc_control_reg(0x22C)
/* (RW) BTC core configuration settings */
#define BTC_CONFIG_0            misc_control_reg(0x240)
#define BTC_CONFIG_1            misc_control_reg(0x244)
#define BTC_CONFIG_2            misc_control_reg(0x248)
#define BTC_CONFIG_3            misc_control_reg(0x24C)
#define BTC_CONFIG_4            misc_control_reg(0x250)
#define BTC_CONFIG_5            misc_control_reg(0x254)
#define BTC_CONFIG_6            misc_control_reg(0x258)
#define BTC_CONFIG_7            misc_control_reg(0x25C)
/* (RW) BVH core configuration settings */
#define BVH_CONFIG_0            misc_control_reg(0x260)
#define BVH_CONFIG_1            misc_control_reg(0x264)
#define BVH_CONFIG_2            misc_control_reg(0x268)
#define BVH_CONFIG_3            misc_control_reg(0x26C)
#define BVH_CONFIG_4            misc_control_reg(0x270)
#define BVH_CONFIG_5            misc_control_reg(0x274)
#define BVH_CONFIG_6            misc_control_reg(0x278)
#define BVH_CONFIG_7            misc_control_reg(0x27C)
/* (RW) GPC configuration settings */
#define GPC_CONFIG_0            misc_control_reg(0x280)
#define GPC_CONFIG_1            misc_control_reg(0x284)
#define GPC_CONFIG_2            misc_control_reg(0x288)
#define GPC_CONFIG_3            misc_control_reg(0x28C)
#define GPC_CONFIG_4            misc_control_reg(0x290)
#define GPC_CONFIG_5            misc_control_reg(0x294)
#define GPC_CONFIG_6            misc_control_reg(0x298)
#define GPC_CONFIG_7            misc_control_reg(0x29C)
/* (RW) FF core configuration settings */
#define GPC_CONFIG_8            misc_control_reg(0x2B0)
#define GPC_CONFIG_9            misc_control_reg(0x2B4)
#define GPC_CONFIG_10           misc_control_reg(0x2B8)
#define GPC_CONFIG_11           misc_control_reg(0x2BC)
#define GPC_CONFIG_12           misc_control_reg(0x2C0)
#define GPC_CONFIG_13           misc_control_reg(0x2C4)
#define GPC_CONFIG_14           misc_control_reg(0x2C8)
#define GPC_CONFIG_15           misc_control_reg(0x2CC)
#define GPC_CONFIG_16           misc_control_reg(0x2D0)
/* (RW) XBI configuration settings */
#define GPC_CONFIG_17           misc_control_reg(0x2E0)
#define GPC_CONFIG_18           misc_control_reg(0x2E4)

#define JOB_CONTROL_BASE        0x4000

#define JOB_IRQ_STATUS          0x000   /* Interrupt status register */
#define JOB_RJD_OS_CONFIG       0xF00

#define MAX_JOB_SLOT_NR              8
#define MAX_JOB_SLOT_CHAIN_NR        4

#define job_slot_x_reg(p, x, r) \
	((p)->dm_dev.reg_base.job_reg_base + ((x) << 5) + (r))

#define JSX_IRQ_RAWSTAT         0x04U
#define JSX_IRQ_MASK            0x08U
#define JSX_IRQ_CLEAR           0x0CU
#define JSX_IRQ_STATUS          0x10U
#define JSX_STOP_COMMAND        0x14U

#define job_jc_y_reg(p, y, r) \
	((p)->dm_dev.reg_base.job_reg_base + 0x200 + \
	((y) << 5) + ((y) << 4) + (r))

#define JSX_JCY_HEAD_LO         0x00
#define JSX_JCY_HEAD_HI         0x04
#define JSX_JCY_AFFINITY        0x08
#define JSX_JCY_CONFIG          0x0C
#define JSX_JCY_COMMAND         0x10
#define JSX_JCY_JCD_LO          0x14
#define JSX_JCY_JCD_HI          0x18
#define JSX_JCY_JD_MSB          0x1C
#define JSX_JCY_WAIT_CYCLE      0x20

#define MEMORY_MANAGEMENT_BASE  0x1000

#define MMU_CONFIG              0x004   /* (RW) Raw interrupt status register */
#define MMU_IRQ_CLEAR           0x008   /* (WO) Interrupt clear register */
#define MMU_IRQ_MASK            0x00C   /* (RW) Interrupt mask register */
#define MMU_IRQ_RAWSTAT         0x010   /* (RW) Raw interrupt status register */
#define MMU_IRQ_STATUS          0x014   /* (RO) Interrupt status register */
#define MMU_FEATURES            0x018   /* (RO) MMU features */
#define AS_PRESENT              0x01C   /* (RO) Address space slots present */
#define MMU_INVALID_STATE       0x020   /* (RO) mmu flush pt status register */
#define MMU_CMD_CFG_WARN        0x024   /* (RO) mmu cmd cfg warning register */
#define MMU_CMD_DONE            0x030   /* (RO) mmu cmd done status register */

#define mmu_as_reg(n, r)         (((n) << 7) + (r))
/* (WO) MMU command register for address space n */
#define AS_COMMAND              0x34
/* (RW) Translation table configuration for address space n, low word */
#define AS_TRANSCFG_LO          0x38
/* (RW) Translation Table Base Address for address space n, low word */
#define AS_TRANSTAB_LO          0x40
/* (RW) Translation Table Base Address for address space n, high word */
#define AS_TRANSTAB_HI          0x44
/* (RW) Memory attributes for address space n, low word */
#define AS_MEMATTR_LO           0x50
/* (RW) Memory attributes for address space n, high word */
#define AS_MEMATTR_HI           0x54
/* (RW) Lock region address for address space n, low word */
#define AS_LOCKADDR_LO          0x60
/* (RW) Lock region address for address space n, high word */
#define AS_LOCKADDR_HI          0x64
/* (RO) MMU fault status register for address space n */
#define AS_FAULTSTATUS          0x68
/* (RO) Fault Address for address space n, low word */
#define AS_FAULTADDRESS_LO      0x70
/* (RO) Fault Address for address space n, high word */
#define AS_FAULTADDRESS_HI      0x74
/* (RO) Secondary fault address for address space n, low word */
#define AS_FAULTEXTRA_LO        0x80
/* (RO) Secondary fault address for address space n, high word */
#define AS_FAULTEXTRA_HI        0x84

/* Begin COM register vaulues */
/* GPU_STATUS values */
/* Set if protected mode is active */
#define GPU_STATUS_PROTECTED_MODE_ACTIVE   ((u32)1 << 11)
/* DBGEN wire status */
#define GPU_DBGEN                          ((u32)1 << 12)

/* GPU_PROT_MODE values */
/* Places the GPU in protected mode */
#define GPU_COMMAND_SET_PROTECTED_MODE 0x01

/* CC IRQ flags */
#define GPC_PWRUP_DONE          ((u32)1 << 0)
#define GPC_PWROFF_DONE         ((u32)1 << 8)
#define BL_PWRUP_DONE           ((u32)1 << 16)
#define BL_PWROFF_DONE          ((u32)1 << 17)
/*
 * Set when all cores have finished powering up or down
 * and the power manager is idle.
 */
#define POWER_CHANGED_ALL       ((u32)1 << 18)
/*
 * Set when a reset has completed. Intended to use with SOFT_RESET
 * commands which may take time.
 */
#define RESET_COMPLETED         ((u32)1 << 19)

#define CC_IRQ_REG_ALL          (RESET_COMPLETED)
/* SOFT_RESET_CMD values */
/* Stop all external bus interfaces, and then reset the entire GPU */
#define GPU_COMMAND_SOFT_RESET         0x01

/* GPU IRQ flags */
#define JOB_FAULT               ((u32)1 << 0)
#define CQ_FAULT                ((u32)1 << 1)
#define JMCP_FAULT              ((u32)1 << 2)
#define JMCP_CTRL_FAULT         ((u32)1 << 3)
#define MISC_CTRL_FAULT         ((u32)1 << 4)
#define CYCLECOUNT_DONE         ((u32)1 << 5)
/* Set when a performance count sample has completed. */
#define PRFCNT_SAMPLE_COMPLETED ((u32)1 << 6)
/* Set when a cache clean operation has completed. */
#define CLEAN_CACHES_COMPLETED  ((u32)1 << 7)
/* A GPU Fault has occurred */
#define GPU_FAULT               (JOB_FAULT | \
	CQ_FAULT | JMCP_FAULT | JMCP_CTRL_FAULT | MISC_CTRL_FAULT)

#define GPU_IRQ_MASK_CONFIG (JOB_FAULT | CQ_FAULT | JMCP_FAULT | JMCP_CTRL_FAULT | \
	MISC_CTRL_FAULT | PRFCNT_SAMPLE_COMPLETED | CLEAN_CACHES_COMPLETED)

#define MULTIPLE_GPU_FAULTS     0       /* More than one GPU Fault occurred. */

/* Begin CONFIG register values */
#define BTC_CFG4_TOP_MUX_SEL_2V                 ((u32)1 << 24)
#define GPC_CFG6_LSC_NONGPC_STORE_MASK_DISABLE  ((u32)1 << 5)
#define GPC_CFG6_LSC_PUSH_VTC_GTC_PROP          ((u32)7 << 15)
#define L2_CFG0_MIF_TRACE_DISABLE_BYPASS        ((u32)1 << 8)
#define FCM_CFG0_TIMESTAMP_SEL                  ((u32)1 << 5)
#define FCM_CFG0_TIMESTAMP_OVERRIDE             ((u32)1 << 0)

/*
 * MMU_IRQ_RAWSTAT register values. Values are valid also for
 * MMU_IRQ_CLEAR, MMU_IRQ_MASK, MMU_IRQ_STATUS registers.
 */
#define MMU_PAGE_FAULT_FLAGS    8

/*
 * Macros returning a bitmask to retrieve page fault or bus error flags from
 * MMU registers
 */
#define mmu_page_fault(n)      ((u32)1 << (n))
#define mmu_bus_error(n)       ((u32)1 << ((n) + MMU_PAGE_FAULT_FLAGS))
#define AS_N_TRANSTAB_LPAE_ADDR_SPACE_MASK   0xfffffff0

/*
 * Begin MMU_CONFIG register values
 */
#define MMU_DIS_IN   ((u32)1 << 1)
#define MMU_DIS_OUT  ((u32)1 << 2)
#define MMU_SC_MODE_OFFSET  10

/*
 * Begin AARCH64 MMU TRANSTAB register values
 */
#define AS_TRANSTAB_LPAE_ADDR_SPACE_MASK   ((u64)0xfffffff0)
#define MMU_HW_OUTA_BITS        40
#define AS_TRANSTAB_BASE_MASK  ((1ULL << MMU_HW_OUTA_BITS) - (1ULL << 4))

/*
 * Begin MMU STATUS register values
 */
#define AS_STATUS_AS_ACTIVE     1

#define AS_EXCEPTION_CODE_OFFSET                              13
#define AS_SOURCE_ID_OFFSET                                   9
#define AS_SOURCE_ID_MASK                                     0xF

#define AS_EXCEPTION_CODE_MASK                    ((u32)7 << 3)
#define AS_EXCEPTION_CODE_TRANSLATION_FAULT       ((u32)0 << 3)
#define AS_EXCEPTION_CODE_PERMISSION_FAULT        ((u32)1 << 3)
#define AS_EXCEPTION_CODE_TRANSTAB_BUS_FAULT      ((u32)2 << 3)
#define AS_EXCEPTION_CODE_ACCESS_FLAG             ((u32)3 << 3)
#define AS_EXCEPTION_CODE_ADDRESS_SIZE_FAULT      ((u32)4 << 3)
#define AS_EXCEPTION_CODE_MEMORY_ATTRIBUTES_FAULT ((u32)5 << 3)

#define AS_ACCESS_TYPE_OFFSET                0
#define AS_ACCESS_TYPE_MASK                  3U
#define AS_ACCESS_TYPE_ATOMIC                0
#define AS_ACCESS_TYPE_EX                    1
#define AS_ACCESS_TYPE_READ                  2
#define AS_ACCESS_TYPE_WRITE                 3

/*
 * Begin MMU TRANSCFG register values
 */
#define AS_TRANSCFG_PTW_MEMATTR_MASK              ((u32)3 << 15)
#define AS_TRANSCFG_PTW_MEMATTR_NON_CACHEABLE     ((u32)1 << 15)
#define AS_TRANSCFG_PTW_MEMATTR_WRITE_BACK        ((u32)2 << 15)
#define AS_TRANSCFG_PTW_MEMATTR_SHAREABLE_MASK    ((u32)3 << 17)
#define AS_TRANSCFG_PTW_MEMATTR_OUT_SHAREABLE     ((u32)2 << 17)
#define AS_TRANSCFG_PTW_SC_MASK                   ((u32)0xF << 24)
#define as_transcfg_ptw_sc_id(id)                 ((u32)(id) << 24)

#define AS_TRANSCFG_ADRMODE_UNMAPPED      1
#define AS_TRANSCFG_ADRMODE_IDENTITY      2
#define AS_TRANSCFG_ADRMODE_AARCH64_4K    6
#define AS_TRANSCFG_ADRMODE_AARCH64_64K   8
#define AS_TRANSCFG_ADRMODE_LEGACY_TABLE  15U
#define AS_TRANSCFG_ADRMODE_MASK          0xF
#define AS_TRANSCFG_LPAE_READ_INNER       (1u << 19)

#define AS_MEMATTR_ENTRY_BITS_SIZE      8
/*
 * Begin AS Command Values
 */
#define AS_MMU_COMMAND_DONE     0xFF

/* AS_COMMAND register commands */
#define AS_COMMAND_NOP          0x00    /* NOP Operation */
/* Broadcasts the values in ASn_TRANSTAB and ASn_MEMATTR to all MMUs */
#define AS_COMMAND_UPDATE       0x01
#define AS_COMMAND_LOCK         0x02    /* Issue a lock region command to all MMUs */
#define AS_COMMAND_UNLOCK       0x03    /* Issue a flush region command to all MMUs */
/*
 * Flush all L2 caches then issue a flush region command to all MMUs
 * (deprecated - only for use with T60x)
 */
#define AS_COMMAND_FLUSH        0x04
/* Flush all L2 caches then issue a flush region command to all MMUs */
#define AS_COMMAND_FLUSH_PT     0x04
/*
 * Wait for memory accesses to complete, flush all the L1s cache then
 * flush all L2 caches then issue a flush region command to all MMUs
 */
#define AS_COMMAND_FLUSH_MEM    0x05


/* NOP Operation. Writing this value is ignored */
#define JS_COMMAND_NOP         0x00
/* Start processing a job chain. Writing this value is ignored */
#define JS_COMMAND_START       0x01
#define JS_COMMAND_SOFT_STOP   0x02   /* Gently stop processing a job chain */
#define JS_COMMAND_HARD_STOP   0x03   /* Rudely stop processing a job chain */
/* Execute SOFT_STOP if JOB_CHAIN_FLAG is 0 */
#define JS_COMMAND_SOFT_STOP_0 0x04
/* Execute HARD_STOP if JOB_CHAIN_FLAG is 0 */
#define JS_COMMAND_HARD_STOP_0 0x05
/* Execute SOFT_STOP if JOB_CHAIN_FLAG is 1 */
#define JS_COMMAND_SOFT_STOP_1 0x06
/* Execute HARD_STOP if JOB_CHAIN_FLAG is 1 */
#define JS_COMMAND_HARD_STOP_1 0x07
/* Mask of bits currently in use by the HW */
#define JS_COMMAND_MASK    0x07u

#define JS_N_CONFIG_BARRIER                        (1u << 4)
#define JS_N_CONFIG_END_FLUSH_L2_CLEAN_INVALIDATE  (3u << 8)
#define JS_N_CONFIG_END_FLUSH_L1_CLEAN_INVALIDATE  (3u << 14)
#define JS_CONFIG_END_FLUSH_BINNING                (1u << 10)
#define JS_CONFIG_END_FLUSH_RENDERING              (1u << 11)
#define JS_CONFIG_END_FLUSH_COMPUTE                (1u << 12)
#define JS_CONFIG_END_FLUSH_RAYTRACING             (1u << 13)
#define JS_CONFIG_END_FLUSH_NO_OPERATION            0u
#define JS_CONFIG_END_FLUSH_ALL                    (0xFu << 10)

/* JS_XAFFINITY register values */
#define JS_XAFFINITY_XAFFINITY_ENABLE (1u << 0)
#define JS_XAFFINITY_TILER_ENABLE     (1u << 8)
#define JS_XAFFINITY_CACHE_ENABLE     (1u << 16)

/*
 * JS_STATUS register values
 * The values are separated to avoid dependency of userspace and kernel code
 *
 * Group of values representing the job status insead a particular fault
 */
#define JS_STATUS_NO_EXCEPTION_BASE   0x00
/* 0x02 means INTERRUPTED */
#define JS_STATUS_INTERRUPTED         (JS_STATUS_NO_EXCEPTION_BASE + 0x02)
/* 0x03 means STOPPED */
#define JS_STATUS_STOPPED             (JS_STATUS_NO_EXCEPTION_BASE + 0x03)
/* 0x04 means TERMINATED */
#define JS_STATUS_TERMINATED          (JS_STATUS_NO_EXCEPTION_BASE + 0x04)

/* General fault values */
#define JS_STATUS_FAULT_BASE          0x40
/* 0x40 means CONFIG FAULT */
#define JS_STATUS_CONFIG_FAULT        (JS_STATUS_FAULT_BASE)
/* 0x41 means POWER FAULT */
#define JS_STATUS_POWER_FAULT         (JS_STATUS_FAULT_BASE + 0x01)
/* 0x42 means READ FAULT */
#define JS_STATUS_READ_FAULT          (JS_STATUS_FAULT_BASE + 0x02)
/* 0x43 means WRITE FAULT */
#define JS_STATUS_WRITE_FAULT         (JS_STATUS_FAULT_BASE + 0x03)
/* 0x44 means AFFINITY FAULT */
#define JS_STATUS_AFFINITY_FAULT      (JS_STATUS_FAULT_BASE + 0x04)
/* 0x48 means BUS FAULT */
#define JS_STATUS_BUS_FAULT           (JS_STATUS_FAULT_BASE + 0x08)

/* Instruction or data faults */
#define JS_STATUS_INSTRUCTION_FAULT_BASE  0x50
/* 0x50 means INSTR INVALID PC */
#define JS_STATUS_INSTR_INVALID_PC     (JS_STATUS_INSTRUCTION_FAULT_BASE)
/* 0x51 means INSTR INVALID ENC */
#define JS_STATUS_INSTR_INVALID_ENC    (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x01)
/* 0x52 means INSTR TYPE MISMATCH */
#define JS_STATUS_INSTR_TYPE_MISMATCH  (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x02)
/* 0x53 means INSTR OPERAND FAULT */
#define JS_STATUS_INSTR_OPERAND_FAULT  (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x03)
/* 0x54 means INSTR TLS FAULT */
#define JS_STATUS_INSTR_TLS_FAULT      (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x04)
/* 0x55 means INSTR BARRIER FAULT */
#define JS_STATUS_INSTR_BARRIER_FAULT  (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x05)
/* 0x56 means INSTR ALIGN FAULT */
#define JS_STATUS_INSTR_ALIGN_FAULT    (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x06)
/* NOTE: No fault with 0x57 code defined in spec */

/* 0x58 means DATA INVALID FAULT */
#define JS_STATUS_DATA_INVALID_FAULT   (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x08)
/* 0x59 means TILE RANGE FAULT */
#define JS_STATUS_TILE_RANGE_FAULT     (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x09)
/* 0x5A means ADDRESS RANGE FAULT */
#define JS_STATUS_ADDRESS_RANGE_FAULT  (JS_STATUS_INSTRUCTION_FAULT_BASE + 0x0A)

/* Other faults */
#define JS_STATUS_MEMORY_FAULT_BASE    0x60
/* 0x60 means OUT OF MEMORY */
#define JS_STATUS_OUT_OF_MEMORY        (JS_STATUS_MEMORY_FAULT_BASE)
#define JS_STATUS_UNKNOWN              0x7F   /* 0x7F means UNKNOWN */

/* GPU_COMMAND values */
/* No operation, nothing happens */
#define GPU_COMMAND_NOP                0x00
/* Starts the cycle counter, and system timestamp propagation */
#define GPU_COMMAND_CYCLE_COUNT_START  0x01
/* Stops the cycle counter, and system timestamp propagation */
#define GPU_COMMAND_CYCLE_COUNT_STOP   0x02
/* Config all performance counters */
#define GPU_COMMAND_PRFCNT_CONFIG      0x03
/* Clear all performance counters, setting them all to zero. */
#define GPU_COMMAND_PRFCNT_CLEAR       0x04
/* Sample all performance counters, writing them out to memory */
#define GPU_COMMAND_PRFCNT_SAMPLE      0x05
/* Clean all caches */
#define GPU_COMMAND_CLEAN_CACHES       0x06
/* Clean and invalidate all caches */
#define GPU_COMMAND_CLEAN_INV_CACHES   0x07

#define GPU_FLUSH_BINNING_ENABLE       (1 << 4)
#define GPU_FLUSH_RENDER_ENABLE        (1 << 5)
#define GPU_FLUSH_COMPUTE_ENABLE       (1 << 6)
#define GPU_FLUSH_RAYTRACING_ENABLE    (1 << 7)

#define GPU_FLUSH_ALL_SHADER_TYPE (GPU_FLUSH_BINNING_ENABLE | \
	GPU_FLUSH_RENDER_ENABLE | GPU_FLUSH_COMPUTE_ENABLE | \
	GPU_FLUSH_RAYTRACING_ENABLE)

/* PRFCNT_CONFIG register values */
#define PRFCNT_CONFIG_MODE_SHIFT       0    /* Counter mode position. */
#define PRFCNT_CONFIG_AS_SHIFT         4    /* Address space bitmap position. */
#define PRFCNT_CONFIG_SETSELECT_SHIFT  8    /* Set select position. */
#define PRFCNT_CONFIG_MAXSET_NUM       19    /* Set select num. */

/* The performance counters are disabled */
#define PRFCNT_CONFIG_MODE_OFF         0
/*
 * The performance counters are enabled, but are only written out when a
 * PRFCNT_SAMPLE command is issued using the GPU_COMMAND register
 */
#define PRFCNT_CONFIG_MODE_MANUAL      1
/*
 * The perfromance counters are enabled, perf counters of BTC and SC0(default)
 * will be written out per millisecond
 */
#define PRFCNT_CONFIG_MODE_MANUAL_NOCLEAR   2

#define PRFCNT_CONFIG_MODE_POWER_METER      3

/* checkbit register config in JM_CONFIG_0 */
#define POWER_METER_ENABLE              ((u32)1 << 23)
#define JM_CFG0_JMCP_SELECT_NEW_DMA_IP  ((u32)1 << 24)

/* main clock of power meter */
#define POWER_METER_CLOCK              35000U

#define HVGR_PWR_KEY_VALUE             0x2968a819

#define HVGR_ICG_CLK_ON                0x1000

/* AS<n>_MEMATTR values */

/* Use GPU implementation-defined caching policy */
#define AS_MEMATTR_IMPL_DEF_CACHE_POLICY      ((u64)0x88)
/* The attribute set to force all resources to be cached */
#define AS_MEMATTR_FORCE_TO_CACHE_ALL         ((u64)0x8F)
/* Inner write-alloc cache setup, no outer caching */
#define AS_MEMATTR_WRITE_ALLOC                ((u64)0x8D)
/* Set to implementation defined, outer caching */
#define AS_MEMATTR_AARCH64_OUTER_IMPL_DEF     ((u64)0x88)
/* Set to write back memory, outer caching */
#define AS_MEMATTR_AARCH64_OUTER_WA           ((u64)0x8D)

/* Use GPU implementation-defined  caching policy */
#define AS_MEMATTR_LPAE_IMPL_DEF_CACHE_POLICY ((u64)0x48)
/* The attribute set to force all resources to be cached */
#define AS_MEMATTR_LPAE_FORCE_TO_CACHE_ALL    ((u64)0x4F)
/* Inner write-alloc cache setup, no outer caching */
#define AS_MEMATTR_LPAE_WRITE_ALLOC           ((u64)0x4D)
/* Set to implementation defined, outer caching */
#define AS_MEMATTR_LPAE_OUTER_IMPL_DEF        ((u64)0x88)
/* Set to write back memory, outer caching */
#define AS_MEMATTR_LPAE_OUTER_WA              ((u64)0x8D)

#define AS_MEMATTR_LPAE                       ((u64)0x8d884d4f48)

/* Symbols for default MEMATTR to use */

/* Default is - HW implementation defined caching */
#define AS_MEMATTR_INDEX_DEFAULT               0
#define AS_MEMATTR_INDEX_DEFAULT_ACE           3

/* HW implementation defined caching */
#define AS_MEMATTR_INDEX_IMPL_DEF_CACHE_POLICY 0
/* Force cache on */
#define AS_MEMATTR_INDEX_FORCE_TO_CACHE_ALL    1
/* Write-alloc */
#define AS_MEMATTR_INDEX_WRITE_ALLOC           2
/* Outer coherent, inner implementation defined policy */
#define AS_MEMATTR_INDEX_OUTER_IMPL_DEF        3
/* Outer coherent, write alloc inner */
#define AS_MEMATTR_INDEX_OUTER_WA              4

/*
 * THREAD_* registers
 * THREAD_FEATURES IMPLEMENTATION_TECHNOLOGY values
 */
#define IMPLEMENTATION_SILICON           0
#define IMPLEMENTATION_FPGA              1

/*
 * Default values when registers are not supported by the implemented hardware
 */
#define THREAD_MT_DEFAULT                256
#define THREAD_MWS_DEFAULT               256
#define THREAD_MBS_DEFAULT               256
#define THREAD_MR_DEFAULT                1024
#define THREAD_MTQ_DEFAULT               4
#define THREAD_MTGS_DEFAULT              10

/* SHADER_CONFIG register */

#define SC_ALT_COUNTERS             ((u32)(1ul << 3))
#define SC_OVERRIDE_FWD_PIXEL_KILL  ((u32)(1ul << 4))
#define SC_SDC_DISABLE_OQ_DISCARD   ((u32)(1ul << 6))
#define SC_LS_PAUSEBUFFER_DISABLE   ((u32)(1ul << 16))
#define SC_ENABLE_TEXGRD_FLAGS      ((u32)(1ul << 25))

#define SHADER_CONFIG_0_ICG        0x37
#define SHADER_CONFIG_2_ICG        0x79
#define SHADER_CONFIG_5_ICG        0x200
/* End SHADER_CONFIG register */

#define JOB_HEADER_ID_MAX                 0xffff

#define GPU_IRQ_BITMAP 7

#define GPU_FAULT_SET (GPU_FAULT)

#define GPU_CONTROL_BASE_ADDR (GPU_CONTROL_BASE)

#define MMU_STREAM_ID_OFFSET 2u
#define MMU_STREAM_ID_MASK 0x1Fu
#define MMU_UTLB_ID_OFFSET 7u
#define MMU_UTLB_ID_MASK 0x3u

/*
 * Bits 11 of PWR_OVERRIDE0 are bvh clk gate enable, so
 * the power trans number value is 0x800.
 */
#define HVGR_PWR_TRANS_NUM_VALUE 0x800

#define GPU_FAULT_REG_STRIDE 0x20

/* macro stub for v1xx compile */
#define GPU_CONTROL_BASE_BEFORE_V120 0u
#define JOB_CONTROL_BASE_BEFORE_V120 0u
#define MEMORY_MANAGEMENT_BASE_BEFORE_V120 0u
#define JMCP_DTCM_START_ADDR_BEFORE_120 0u
#define JMCP_BASE_BEFORE_120 0u
#define JMCP_MB_MCU_KEY_BEFORE_120 0u
#define JMCP_DMA_CHANEL_BASE_BEFORE_120 0u
#define JMCP_TIMER_LOAD_BASE_BEFORE_120 0u
#define JMCP_UART_LOAD_BASE_BEFORE_120 0u
#define JMCP_UART_LOAD_BASE 0u

/* CQ register */
#define CQ_REG_BASE     0x8000

#define CQ_IRQ_TOP      (0x0)
#define CQ_ENABLE       (0x10)
#define CQ_DBG_EN       (0x14)
#define CQ_CHN_NUM      (0x18)
#define CQ_SCH_MODE     (0x1c)
#define CQ_DTCM_BASE    (0x24)
#define CQ_BC_SEND_MODE (0x28)

#define CQ_SUBMIT       (0x1000)

#define cq_irq_rawstat(chn)  (0x2000 + (chn) * 0x100)
#define cq_irq_clear(chn)    (0x2004 + (chn) * 0x100)
#define cq_irq_mask(chn)     (0x2008 + (chn) * 0x100)
#define cq_irq_status(chn)   (0x200c + (chn) * 0x100)
#define cq_bind_addr_lo(chn)    (0x2010 + (chn) * 0x100)
#define cq_bind_addr_hi(chn)    (0x2014 + (chn) * 0x100)
#define cq_bind_asid(chn)       (0x2018 + (chn) * 0x100)
#define cq_bind_queue_id(chn)   (0x201c + (chn) * 0x100)

#define cq_bind_aff_bin(chn, n)    (0x2020 + (chn) * 0x100 + (n) * 0x4)
#define cq_bind_config(chn, n)     (0x2040 + (chn) * 0x100 + (n) * 0x4)

#define cq_bind_jd_msb(chn)     (0x2060 + (chn) * 0x100)
#define cq_bind_waitcycle(chn)  (0x2064 + (chn) * 0x100)
#define cq_bind_timeout(chn)    (0x2068 + (chn) * 0x100)
#define cq_bind_timeslice(chn)  (0x2070 + (chn) * 0x100)
#define cq_bind_misccfg(chn)    (0x2074 + (chn) * 0x100)
#define cq_bind_timeout_1(chn)  (0x2078 + (chn) * 0x100)
#define cq_bind_timeout_2(chn)  (0x207c + (chn) * 0x100)
#define cq_bind_cmd(chn)        (0x2080 + (chn) * 0x100)
#define cq_bind_status_0(chn)   (0x2084 + (chn) * 0x100)
#define cq_bind_status_1(chn)   (0x2088 + (chn) * 0x100)

#define cq_bind_status_2(chn)   (0x208c + (chn) * 0x100)
#define cq_bind_status_3(chn)   (0x2090 + (chn) * 0x100)
#define cq_bind_status_4(chn)   (0x2094 + (chn) * 0x100)
#define cq_bind_status_5(chn)   (0x2098 + (chn) * 0x100)
#define cq_bind_status_6(chn)   (0x209c + (chn) * 0x100)

#define cq_bind_subchn_os(chn)  (0x20a0 + (chn) * 0x100)

#define cq_bind_affinity(chn)   (0x2020 + (chn) * 0x100)

#define cq_bind_status_sub_channel(chn, sub_chn)   \
	(cq_bind_status_2(chn) + 4 * (sub_chn))
#define cq_bind_affinity_sub_channel(chn, sub_chn) \
	(cq_bind_affinity(chn) + 4 * (sub_chn))

#define CQ_SUBMIT_FOR_UMD       (CQ_REG_BASE + 0x1000)

#define CQ_CMD_START    1
#define CQ_CMD_STOP     2

#define CQ_IRQ_STATUS_TIMEOUT_BIT_START 3
#define CQ_IRQ_STATUS_TIMEOUT_BIT_NUMS  7
#define CQ_IRQ_STATUS_TIMEOUT_BIT_MASK \
	(((1U << CQ_IRQ_STATUS_TIMEOUT_BIT_NUMS) - 1) << CQ_IRQ_STATUS_TIMEOUT_BIT_START)

#define CQ_IRQ_STATUS_SIGNAL_BIT_START  10
#define CQ_IRQ_STATUS_SIGNAL_BIT_NUMS   5
#define CQ_IRQ_STATUS_SIGNAL_BIG_MASK \
	(((1U << CQ_IRQ_STATUS_SIGNAL_BIT_NUMS) - 1) << CQ_IRQ_STATUS_SIGNAL_BIT_START)

union cq_irq {
	struct {
		uint32_t notify : 1;
		uint32_t stop : 1;
		uint32_t svd : 1;

		uint32_t timeout_empty : 1;
		uint32_t timeout_wait : 1;
		uint32_t timeout_wait_render : 1;
		uint32_t timeout_wait_bind : 1;
		uint32_t timeout_wait_computer : 1;
		uint32_t timeout_wait_bvh : 1;
		uint32_t timeout_wait_rt : 1;

		uint32_t signal_render : 1;
		uint32_t signal_binning : 1;
		uint32_t signal_computer : 1;
		uint32_t signal_bvh : 1;
		uint32_t signal_rt : 1;
	} st;
	uint32_t value;
};

union cq_bind_notify_status {
	struct {
		uint32_t bind_status:3;
		uint32_t rsvd:13;
		uint32_t bind_token:14;
		uint32_t rsvd1:2;
	} st;
	uint32_t value;
};

union cq_bind_svd_status {
	struct {
		uint32_t bind_svd_token:14;
		uint32_t bind_svd_jsid:3;
		uint32_t bind_svd_token_vld:1;
	} st;
	uint32_t value;
};

union cq_bind_signal_status {
	struct {
		uint32_t token:14;
		uint32_t rsvd:3;
		uint32_t token_vld:1;
	} st;
	uint32_t value;
};

#define CQ_BIND_STATUS_NOP          (0u)
#define CQ_BIND_STATUS_EMPTY        (1u)
#define CQ_BIND_STATUS_PENDING      (2u)
#define CQ_BIND_STATUS_SEM_ACQURING (3u)
#define CQ_BIND_STATUS_HOLD         (4u)
#define CQ_BIND_STATUS_STOPPING     (5u)
#define CQ_BIND_STATUS_STOPPED      (6u)
#define CQ_BIND_SATAUS_RUNNING      (7u)

static inline bool hvgr_is_job_slot_support(void)
{
	return true;
}

#define HVGR_MMU_SID_START_OFFSET 2
#define HVGR_MMU_SID_END_OFFSET 9

#endif /* HVGR_REGMAP_H */
