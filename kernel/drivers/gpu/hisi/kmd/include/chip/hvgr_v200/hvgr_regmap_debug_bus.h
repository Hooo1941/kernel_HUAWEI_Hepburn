/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_REGMAP_DEBUG_BUS_H
#define HVGR_REGMAP_DEBUG_BUS_H

#define HVGR_DEBUG_BUS_MAX_CORE_ID              15

#define HVGR_DEBUG_BUS_MAX_GPC_DEST_ID          36
#define HVGR_DEBUG_BUS_MAX_BVH_DEST_ID          5
#define HVGR_DEBUG_BUS_MAX_FCM_DEST_ID          6
#define HVGR_DEBUG_BUS_MAX_L2MMU_DEST_ID        8
#define HVGR_DEBUG_BUS_MAX_BIN_DEST_ID          17

#define HVGR_DEBUG_BUS_RESERVED_MIN_CORE_ID  8
#define HVGR_DEBUG_BUS_RESERVED_MAX_CODE_ID  12

#define HVGR_DEBUG_BUS_MAX_GPC_CORE_ID       7
#define HVGR_DEBUG_BUS_MAX_BVH_CORE_ID       11
#define HVGR_DEBUG_BUS_MAX_COM_CTRL_CORE_ID  12

#define HVGR_DEBUG_BUS_FCM_CORE_ID              13
#define HVGR_DEBUG_BUS_L2MMU_CORE_ID            14
#define HVGR_DEBUG_BUS_BIN_COER_ID              15

#define HVGR_DEBUG_BUS_MAX_STATE_TYPE           15

#define HVGR_DEBUG_BUS_INVALID_INDEX           0xff
#define HVGR_DEBUG_BUS_DUMP_MAX_TIMES          2
#define HVGR_DEBUG_BUS_DUMP_FIR                0
#define HVGR_DEBUG_BUS_DUMP_SEC                1
#define HVGR_DEBUG_BUS_CNT_FIR                1
#define HVGR_DEBUG_BUS_CNT_SEC                2
#define HVGR_DEBUG_BUS_BUFFER_SIZE_MISC        0x1000000
#define HVGR_DEBUG_BUS_BUFFER_SIZE_PRE_GPC     0x200000
#define HVGR_DEBUG_BUS_DEFAULT_GPC_NUMS        8

#define HVGR_DEBUG_BUS_DATA_DIR                "/data/hisi_logs/debugbus/"
#ifdef CONFIG_LIBLINUX
#define HVGR_DEBUG_BUS_DATA_FILE               "gpu_debugbus_%s_%s.dbb"
#define HVGR_DEBUG_BUS_DATA_FILE_DUPLICATED    "gpu_debugbus_%s_%s_duplicated.dbb"
#elif defined(CONFIG_HVGR_DFX_MNTN_REPORT)
#define HVGR_DEBUG_BUS_DATA_FILE               "gpu_debugbus_v1.dbb"
#define HVGR_DEBUG_BUS_DATA_FILE_DUPLICATED    "gpu_debugbus_v1_duplicated.dbb"
#else
#define HVGR_DEBUG_BUS_DATA_FILE               "/data/hisi_logs/debugbus/gpu_debugbus_%s_%s.dbb"
#define HVGR_DEBUG_BUS_DATA_FILE_DUPLICATED    "/data/hisi_logs/debugbus/gpu_debugbus_%s_%s_duplicated.dbb"
#endif
#define HVGR_DEBUG_BUS_TIME                    "%4d-%02d-%02d_%02d-%02d-%02d"
#define HVGR_DEBUG_BUS_DATA_DIR_MODE           0777
#define DATA_DIR                               "/data"
#define HVGR_BTC_CONFIG_OFFSET            ((u32)1 << 25)

#define HVGR_DEBUG_BUS_STATE_TYPE_OFFSET     11
#define HVGR_DEBUG_BUS_DEST_ID_OFFSET        15
#define HVGR_DEBUG_BUS_CORE_ID_OFFSET        23
#define HVGR_DEBUG_BUS_TU_NUM                2
#define HVGR_DEBUG_BUS_DM_NUM                2
#define HVGR_DEBUG_BUS_DM_NUM_OFFSET         15
#define HVGR_DEBUG_BUS_TU_NUM_OFFSET         18

#define MAX_DUMP_FILE_NUMBER            50
#define FAULT_NUMBER                    5

#define MAX_MEM_SIZE                    4096
#define MEM_STORE_SIZE                  4000
#define MIN_FREE_SPACE                  1024
#define DUMP_DATA_MAX_SIZE              64
#define MAX_TIME_SIZE                   20

#define VERSION_100             "Version0.97\n"
#define VERSION_120             "Version1.20\n"
#define VERSION_200             "Version2.00\n"
#define VERSION_300             "Version3.00\n"
#define RECORD_FORMAT           "0x%08x:0x%08x\n"
#define MAX_READ_TIMES          50
#define RECORD_LENGHT           22
#define GPU_CONTROL_TITLE       "gpu control register:\n"
#define MISC_CONTROL_TITLE      "misc control register:\n"
#define JOB_CONTROL_TITLE       "job control register:\n"
#define MMU_CONTROL_TITLE       "mmu control register:\n"
#define FCP_CONTROL_TITLE       "fcp control register:\n"
#define CQ_CONTROL_TITLE        "cq control register:\n"


#define JOB_CHAIN_NUM           4
#define MAX_ADDR_SPACE_NUM      8

#define MAX_REQ_NUM             4
#define MAX_WINDOW_NUM          4

#define MAX_MB_CMD_MSG_NUM      4
#define MAX_MB_STATUE_MSG_NUM   2
#define MAX_DMA_CHANNEL_NUMBER  32

#define MAX_TIMER_NUMBER        2

#define STATE_TYEP_0            0
#define STATE_TYEP_1            1
#define STATE_TYEP_2            2
#define STATE_TYEP_3            3
#define STATE_TYEP_4            4
#define STATE_TYEP_5            5
#define STATE_TYEP_6            6
#define STATE_TYEP_7            7
#define STATE_TYEP_8            8
#define STATE_TYEP_9            9
#define STATE_TYEP_10           10
#define STATE_TYEP_11           11
#define STATE_TYEP_12           12
#define STATE_TYEP_13           13
#define STATE_TYEP_14           14
#define STATE_TYEP_15           15

#define DEST_ID_GPC_VTC        0
#define DEST_ID_GPC_SDC        1
#define DEST_ID_GPC_LSC        2
#define DEST_ID_GPC_GTC        3
#define DEST_ID_GPC_WM         4
#define DEST_ID_GPC_RTU        5
#define DEST_ID_GPC_RTTC       6
#define DEST_ID_GPC_DM         8
#define DEST_ID_GPC_DM1        9
#define DEST_ID_GPC_SQ         10
#define DEST_ID_GPC_DG         11
#define DEST_ID_GPC_RCP        16
#define DEST_ID_GPC_TS         17
#define DEST_ID_GPC_RAST       18
#define DEST_ID_GPC_ZS         19
#define DEST_ID_GPC_BLND       20
#define DEST_ID_GPC_VARY       21
#define DEST_ID_GPC_CBI        24
#define DEST_ID_GPC_PEC        25
#define DEST_ID_GPC_TAG        32
#define DEST_ID_GPC_TSG        33
#define DEST_ID_GPC_TSM        34
#define DEST_ID_GPC_TCC        35
#define DEST_ID_GPC_TFU        36
#define DEST_ID_GPC_TL1C       37
#define DEST_ID_GPC_TAG1       40
#define DEST_ID_GPC_TSG1       41
#define DEST_ID_GPC_TSM1       42
#define DEST_ID_GPC_TCC1       43
#define DEST_ID_GPC_TFU1       44

#define DEST_ID_BVH_BVHM        0
#define DEST_ID_BVH_BSETUP      1
#define DEST_ID_BVH_BRANK       2
#define DEST_ID_BVH_BEMIT       3
#define DEST_ID_BVH_BSAH        4
#define DEST_ID_BVH_BCACHE      5
#define DEST_ID_BVH_PEC         8

#define DEST_ID_COM_CTRL        0

#define DEST_ID_FCM_JOB         0
#define DEST_ID_FCM_CQ          1
#define DEST_ID_FCM_FCP         2
#define DEST_ID_FCM_FCP_CTRL    3
#define DEST_ID_FCM_MISC_CTRL   4
#define DEST_ID_FCM_PEC         5
#define DEST_ID_FCM_TOP         6

#define DEST_ID_L2MMU_L2RO      0
#define DEST_ID_L2MMU_L2RW      1
#define DEST_ID_L2MMU_L2MIF     2
#define DEST_ID_L2MMU_MMU       3
#define DEST_ID_L2MMU_JBI       4
#define DEST_ID_L2MMU_GBI       5
#define DEST_ID_L2MMU_MCB       6
#define DEST_ID_L2MMU_CPB       7
#define DEST_ID_L2MMU_PEU       8

#define  DEST_ID_BIN_JCU        0
#define  DEST_ID_BIN_VF         1
#define  DEST_ID_BIN_PA         2
#define  DEST_ID_BIN_TESS       3
#define  DEST_ID_BIN_UTC        4
#define  DEST_ID_BIN_OA         5
#define  DEST_ID_BIN_GPC_ARB    6
#define  DEST_ID_BIN_BTC_TOP    7
#define  DEST_ID_BIN_BTS        8
#define  DEST_ID_BIN_BRAST      9
#define  DEST_ID_BIN_BCZ        10
#define  DEST_ID_BIN_REJ_SYNC   11
#define  DEST_ID_BIN_SPU        12
#define  DEST_ID_BIN_SC         13
#define  DEST_ID_BIN_BBI        14
#define  DEST_ID_BIN_PEC        15

/*
 * We need to balance the code size and the dbb file size.
 * The previous code was designed to divide valid offset addresses into groups,
 * and to traverse each group from 0 to 2047. This solution results in a huge dbb file
 * size. At some platform, we have not enough space to save this file.
 * So, we need to reduce the dbb file size. Now, we record the max valid address for each
 * group as follow, so traverse each group from 0 to the max value.
 */
#define DEST_ID_GPC_VTC_STATE_0_SIZE         0x27
#define DEST_ID_GPC_SDC_STATE_0_SIZE         0x7FF /* Because [0x100 + 0x840(2112)] > 0x800 */
#define DEST_ID_GPC_SDC_STATE_1_SIZE         0x140 /* = 0x940 - 0x800 * 0x1 */
#define DEST_ID_GPC_LSC_STATE_0_SIZE         0x705
#define DEST_ID_GPC_LSC_STATE_5_SIZE         0x195 /* = 0x2905 - 0x800 * 0x5 + 0x90[((80/32+1)*48)] */
#define DEST_ID_GPC_LSC_STATE_6_SIZE         0x205 /* = 0x3105 - 0x800 * 0x6 + 0x100[(256/32)*32] */
#define DEST_ID_GPC_LSC_STATE_7_SIZE         0x605 /* = 0x3905 - 0x800 * 0x7 + 0x500[(256/32)*160] */
#define DEST_ID_GPC_GTC_STATE_0_SIZE         0x51
#define DEST_ID_GPC_GTC_STATE_1_SIZE         0x47  /* = 0x847 - 0x800 * 0x1 */
#define DEST_ID_GPC_WM_STATS_0_SIZE          0x48
#define DEST_ID_GPC_RTU_STATS_0_SIZE         0x61
#define DEST_ID_GPC_RTTC_STATS_0_SIZE        0x1A
#define DEST_ID_GPC_DM_STATS_0_SIZE          0x6E
#define DEST_ID_GPC_SQ_STATS_0_SIZE          0x7FF
#define DEST_ID_GPC_SQ_STATS_1_SIZE          0x7FF
#define DEST_ID_GPC_SQ_STATS_2_SIZE          0x7FF
#define DEST_ID_GPC_SQ_STATS_3_SIZE          0x7FF
#define DEST_ID_GPC_SQ_STATS_4_SIZE          0x7FF
#define DEST_ID_GPC_DG_STATS_0_SIZE          0xA1
#define DEST_ID_GPC_RCP_STATS_0_SIZE         0x2C
#define DEST_ID_GPC_RCP_STATS_1_SIZE         0x7F /* = 0x87F - 0x800 * 0x1 */
#define DEST_ID_GPC_TS_STATS_0_SIZE          0x40
#define DEST_ID_GPC_RAST_STATS_0_SIZE        0x2A
#define DEST_ID_GPC_ZS_STATE_0_SIZE          0x40
#define DEST_ID_GPC_ZS_STATE_2_SIZE          0x51
#define DEST_ID_GPC_ZS_STATE_4_SIZE          0x14
#define DEST_ID_GPC_ZS_STATE_6_SIZE          0x30
#define DEST_ID_GPC_ZS_STATE_8_SIZE          0x80
#define DEST_ID_GPC_BLND_STATE_0_SIZE        0x22
#define DEST_ID_GPC_BLND_STATE_2_SIZE        0x9
#define DEST_ID_GPC_BLND_STATE_4_SIZE        0x5
#define DEST_ID_GPC_BLND_STATE_6_SIZE        0x1
#define DEST_ID_GPC_BLND_STATE_8_SIZE        0x10B
#define DEST_ID_GPC_BLND_STATE_10_SIZE       0x1A
#define DEST_ID_GPC_BLND_STATE_12_SIZE       0x6
#define DEST_ID_GPC_VARY_STATE_0_SIZE        0x8C
#define DEST_ID_GPC_CBI_STATE_0_SIZE         0x7FF
#define DEST_ID_GPC_CBI_STATE_1_SIZE         0x7FF
#define DEST_ID_GPC_PEC_STATE_0_SIZE         0x1
#define DEST_ID_GPC_TAG_STATE_0_SIZE         0xB
#define DEST_ID_GPC_TSG_STATE_0_SIZE         0x1
#define DEST_ID_GPC_TSM_STATE_SIZE           0x7FF
#define DEST_ID_GPC_TCC_STATE_0_SIZE         0x7FF
#define DEST_ID_GPC_TUF_STATE_0_SIZE         0xC
#define DEST_ID_GPC_TL1C_STATE_0_SIZE        0x10
#define DEST_ID_BVH_BVHM_STATE_0_SIZE        0x706
#define DEST_ID_BVH_BSETUP_STATE_0_SIZE      0xE
#define DEST_ID_BVH_BRANK_STATE_0_SIZE       0x39
#define DEST_ID_BVH_BEMIT_STATE_0_SIZE       0x702
#define DEST_ID_BVH_BEMIT_STATE_1_SIZE       0x4
#define DEST_ID_BVH_BSAH_STATE_0_SIZE        0x16
#define DEST_ID_BVH_BCACHE_STATE_0_SIZE      0x3A
#define DEST_ID_BVH_PEC_STATE_0_SIZE         0x1
#define DEST_ID_FCM_JOB_STATE_0_SIZE         0x501
#define DEST_ID_FCM_JOB_STATE_SIZE           0x7FF
#define DEST_ID_FCM_CQ_STATE_0_SIZE          0xDB /* = 0xB5 + 0x26 */
#define DEST_ID_FCM_FCP_STATE_0_SIZE         0x151
#define DEST_ID_FCM_FCP_CTRL_STATE_0_SIZE    0xE
#define DEST_ID_FCM_MISC_CTRL_STATE_0_SIZE   0xA
#define DEST_ID_FCM_PEC_STATE_0_SIZE         0x1
#define DEST_ID_FCM_TOP_STATE_0_SIZE         0x8
#define DEST_ID_L2MMU_L2RO_STATE_0_SIZE      0x2d4 /* = 0x200 + 0xF + 0xC5 */
#define DEST_ID_L2MMU_L2RW_STATE_0_SIZE      0x602
#define DEST_ID_L2MMU_L2MIF_STATE_0_SIZE     0x75
#define DEST_ID_L2MMU_MMU_SIZE               0x7FF
#define DEST_ID_L2MMU_MMU_STATE_8_SIZE       0x6C1
#define DEST_ID_L2MMU_JBI_STATE_0_SIZE       0x360
#define DEST_ID_L2MMU_GBI_STATE_0_SIZE       0xB
#define DEST_ID_L2MMU_MCB_STATE_0_SIZE       0x0
#define DEST_ID_L2MMU_CPB_STATE_0_SIZE       0x7
#define DEST_ID_L2MMU_PEB_STATE_0_SIZE       0x1
#define DEST_ID_BIN_JCU_STATE_0_SIZE         0x14
#define DEST_ID_BIN_VF_STATE_0_SIZE          0x4
#define DEST_ID_BIN_VF_STATE_1_SIZE          0x11
#define DEST_ID_BIN_PA_STATE_0_SIZE          0x7
#define DEST_ID_BIN_TESS_STATE_0_SIZE        0x5
#define DEST_ID_BIN_TESS_STATE_1_SIZE        0x0
#define DEST_ID_BIN_UTC_STATE_0_SIZE         0x13
#define DEST_ID_BIN_OA_STATE_0_SIZE          0x7
#define DEST_ID_BIN_OA_STATE_1_SIZE          0x3
#define DEST_ID_BIN_GPC_ARB_STATE_0_SIZE     0x27
#define DEST_ID_BIN_BTC_TOP_STATE_0_SIZE     0x2
#define DEST_ID_BIN_BTS_STATE_0_SIZE         0x2D
#define DEST_ID_BIN_BRAST_STATE_0_SIZE       0x19
#define DEST_ID_BIN_BCZ_STATE_0_SIZE         0x32
#define DEST_ID_BIN_REJ_SYNC_STATE_0_SIZE    0x9
#define DEST_ID_BIN_SPU_STATE_0_SIZE         0x1A
#define DEST_ID_BIN_SC_STATE_0_SIZE          0x3
#define DEST_ID_BIN_BBI_STATE_0_SIZE         0x200 /* = 0x180 + 0x7F[((52/32)+1)*64] */
#define DEST_ID_BIN_PEC_STATE_0_SIZE         0x1
#define DEST_ID_COM_CTRL_STATE_0_SIZE        0x11

#define FCP_REG_SIZE                                    4
#define FILE_NAME_SIZE                                  255
#define HVGR_DEBUG_BUS_MAX_NUMBER                       99
#define HVGR_DEBUG_BUS_FILE_PREFIX                      "gpu_debugbus_"

#endif  /* HVGR_REGMAP_DEBUG_BUS_H */
