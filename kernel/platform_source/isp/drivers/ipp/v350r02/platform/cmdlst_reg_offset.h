/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name     :  cmdlst_ipp_reg_offset.h
 * Project line  :  
 * Department    :  K5
 * Author        :  AnthonySixta
 * Version       :  1.0
 * Date          :  
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  AnthonySixta 2021/09/07 16:32:03 Create file
 */

#ifndef __CMDLST_REG_OFFSET_H__
#define __CMDLST_REG_OFFSET_H__

/* cmdlst_ipp Base address of Module's Register */

/******************************************************************************/
/*                     cmdlst_ipp Registers' Definitions                         */
/******************************************************************************/

#define CMDLST_ST_LD_REG                0x0   /* Read/Write register for correct Stat read support (store / load) */
#define CMDLST_START_ADDR_REG           0x4   /* CMDLST start address */
#define CMDLST_END_ADDR_REG             0x8   /* CMDLST end address */
#define CMDLST_CHECK_BYP_START_ADDR_REG 0xC   /* Address range to bypass WR to RD conversion in check mode */
#define CMDLST_CHECK_BYP_END_ADDR_REG   0x10  /* Address range to bypass WR to RD conversion in check mode */
#define CMDLST_TRANS_START_REG          0x14  
#define CMDLST_TRANS_END_REG            0x18  
#define CMDLST_TRANS_OFFSET_REG         0x1C  
#define CMDLST_CFG_REG                  0x2C  /* Configuration */
#define CMDLST_CHECK_ERROR_STATUS_0_REG 0x3C  /* check error */
#define CMDLST_CHECK_ERROR_STATUS_1_REG 0x40  /* check error */
#define CMDLST_DEBUG_0_REG              0x48  /* Debug register (available over the debug bus, lower part) */
#define CMDLST_DEBUG_1_REG              0x4C  /* Debug register */
#define CMDLST_DEBUG_2_REG              0x50  /* Debug register */
#define CMDLST_DEBUG_3_REG              0x54  /* Debug register */
#define CMDLST_DEBUG_4_0_REG            0x58  /* Debug register (available over the debug bus, upper part) */
#define CMDLST_DEBUG_4_1_REG            0x5C  /* Debug register (available over the debug bus, upper part) */
#define CMDLST_DEBUG_4_2_REG            0x60  /* Debug register (available over the debug bus, upper part) */
#define CMDLST_DEBUG_4_3_REG            0x64  /* Debug register (available over the debug bus, upper part) */
#define CMDLST_CH_CFG_0_REG             0x80  /* Channel configuration register. */
#define CMDLST_CH_CFG_1_REG             0x100 /* Channel configuration register. */
#define CMDLST_CH_CFG_2_REG             0x180 /* Channel configuration register. */
#define CMDLST_CH_CFG_3_REG             0x200 /* Channel configuration register. */
#define CMDLST_CH_CFG_4_REG             0x280 /* Channel configuration register. */
#define CMDLST_CH_CFG_5_REG             0x300 /* Channel configuration register. */
#define CMDLST_CH_CFG_6_REG             0x380 /* Channel configuration register. */
#define CMDLST_CH_CFG_7_REG             0x400 /* Channel configuration register. */
#define CMDLST_CH_CFG_8_REG             0x480 /* Channel configuration register. */
#define CMDLST_CH_CFG_9_REG             0x500 /* Channel configuration register. */
#define CMDLST_CH_CFG_10_REG            0x580 /* Channel configuration register. */
#define CMDLST_CH_CFG_11_REG            0x600 /* Channel configuration register. */
#define CMDLST_CH_CFG_12_REG            0x680 /* Channel configuration register. */
#define CMDLST_CH_CFG_13_REG            0x700 /* Channel configuration register. */
#define CMDLST_CH_CFG_14_REG            0x780 /* Channel configuration register. */
#define CMDLST_CH_CFG_15_REG            0x800 /* Channel configuration register. */
#define CMDLST_CHECK_ERROR_PTR_0_REG    0x84  /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_1_REG    0x104 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_2_REG    0x184 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_3_REG    0x204 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_4_REG    0x284 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_5_REG    0x304 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_6_REG    0x384 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_7_REG    0x404 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_8_REG    0x484 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_9_REG    0x504 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_10_REG   0x584 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_11_REG   0x604 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_12_REG   0x684 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_13_REG   0x704 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_14_REG   0x784 /* error information ptr */
#define CMDLST_CHECK_ERROR_PTR_15_REG   0x804 /* error information ptr */
#define CMDLST_CHECK_ERROR_TASK_0_REG   0x88  /* error information */
#define CMDLST_CHECK_ERROR_TASK_1_REG   0x108 /* error information */
#define CMDLST_CHECK_ERROR_TASK_2_REG   0x188 /* error information */
#define CMDLST_CHECK_ERROR_TASK_3_REG   0x208 /* error information */
#define CMDLST_CHECK_ERROR_TASK_4_REG   0x288 /* error information */
#define CMDLST_CHECK_ERROR_TASK_5_REG   0x308 /* error information */
#define CMDLST_CHECK_ERROR_TASK_6_REG   0x388 /* error information */
#define CMDLST_CHECK_ERROR_TASK_7_REG   0x408 /* error information */
#define CMDLST_CHECK_ERROR_TASK_8_REG   0x488 /* error information */
#define CMDLST_CHECK_ERROR_TASK_9_REG   0x508 /* error information */
#define CMDLST_CHECK_ERROR_TASK_10_REG  0x588 /* error information */
#define CMDLST_CHECK_ERROR_TASK_11_REG  0x608 /* error information */
#define CMDLST_CHECK_ERROR_TASK_12_REG  0x688 /* error information */
#define CMDLST_CHECK_ERROR_TASK_13_REG  0x708 /* error information */
#define CMDLST_CHECK_ERROR_TASK_14_REG  0x788 /* error information */
#define CMDLST_CHECK_ERROR_TASK_15_REG  0x808 /* error information */
#define CMDLST_CHECK_ERROR_CMD_0_REG    0x8C  /* error information */
#define CMDLST_CHECK_ERROR_CMD_1_REG    0x10C /* error information */
#define CMDLST_CHECK_ERROR_CMD_2_REG    0x18C /* error information */
#define CMDLST_CHECK_ERROR_CMD_3_REG    0x20C /* error information */
#define CMDLST_CHECK_ERROR_CMD_4_REG    0x28C /* error information */
#define CMDLST_CHECK_ERROR_CMD_5_REG    0x30C /* error information */
#define CMDLST_CHECK_ERROR_CMD_6_REG    0x38C /* error information */
#define CMDLST_CHECK_ERROR_CMD_7_REG    0x40C /* error information */
#define CMDLST_CHECK_ERROR_CMD_8_REG    0x48C /* error information */
#define CMDLST_CHECK_ERROR_CMD_9_REG    0x50C /* error information */
#define CMDLST_CHECK_ERROR_CMD_10_REG   0x58C /* error information */
#define CMDLST_CHECK_ERROR_CMD_11_REG   0x60C /* error information */
#define CMDLST_CHECK_ERROR_CMD_12_REG   0x68C /* error information */
#define CMDLST_CHECK_ERROR_CMD_13_REG   0x70C /* error information */
#define CMDLST_CHECK_ERROR_CMD_14_REG   0x78C /* error information */
#define CMDLST_CHECK_ERROR_CMD_15_REG   0x80C /* error information */
#define CMDLST_SW_BRANCH_0_REG          0x90  /* Branching request */
#define CMDLST_SW_BRANCH_1_REG          0x110 /* Branching request */
#define CMDLST_SW_BRANCH_2_REG          0x190 /* Branching request */
#define CMDLST_SW_BRANCH_3_REG          0x210 /* Branching request */
#define CMDLST_SW_BRANCH_4_REG          0x290 /* Branching request */
#define CMDLST_SW_BRANCH_5_REG          0x310 /* Branching request */
#define CMDLST_SW_BRANCH_6_REG          0x390 /* Branching request */
#define CMDLST_SW_BRANCH_7_REG          0x410 /* Branching request */
#define CMDLST_SW_BRANCH_8_REG          0x490 /* Branching request */
#define CMDLST_SW_BRANCH_9_REG          0x510 /* Branching request */
#define CMDLST_SW_BRANCH_10_REG         0x590 /* Branching request */
#define CMDLST_SW_BRANCH_11_REG         0x610 /* Branching request */
#define CMDLST_SW_BRANCH_12_REG         0x690 /* Branching request */
#define CMDLST_SW_BRANCH_13_REG         0x710 /* Branching request */
#define CMDLST_SW_BRANCH_14_REG         0x790 /* Branching request */
#define CMDLST_SW_BRANCH_15_REG         0x810 /* Branching request */
#define CMDLST_LAST_EXEC_PTR_0_REG      0x94  /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_1_REG      0x114 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_2_REG      0x194 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_3_REG      0x214 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_4_REG      0x294 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_5_REG      0x314 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_6_REG      0x394 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_7_REG      0x414 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_8_REG      0x494 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_9_REG      0x514 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_10_REG     0x594 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_11_REG     0x614 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_12_REG     0x694 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_13_REG     0x714 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_14_REG     0x794 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_PTR_15_REG     0x814 /* last CVDR_RD_DATA_3 information used to configure the CVDR */
#define CMDLST_LAST_EXEC_TASK_0_REG     0x98  /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_1_REG     0x118 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_2_REG     0x198 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_3_REG     0x218 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_4_REG     0x298 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_5_REG     0x318 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_6_REG     0x398 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_7_REG     0x418 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_8_REG     0x498 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_9_REG     0x518 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_10_REG    0x598 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_11_REG    0x618 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_12_REG    0x698 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_13_REG    0x718 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_14_REG    0x798 /* last executed TASK ID */
#define CMDLST_LAST_EXEC_TASK_15_REG    0x818 /* last executed TASK ID */
#define CMDLST_SW_RESOURCES_0_REG       0x9C  /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_1_REG       0x11C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_2_REG       0x19C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_3_REG       0x21C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_4_REG       0x29C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_5_REG       0x31C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_6_REG       0x39C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_7_REG       0x41C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_8_REG       0x49C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_9_REG       0x51C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_10_REG      0x59C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_11_REG      0x61C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_12_REG      0x69C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_13_REG      0x71C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_14_REG      0x79C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_RESOURCES_15_REG      0x81C /* Priority/resource required for next pointer ! */
#define CMDLST_SW_TASK_0_REG            0xA0  /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_1_REG            0x120 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_2_REG            0x1A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_3_REG            0x220 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_4_REG            0x2A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_5_REG            0x320 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_6_REG            0x3A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_7_REG            0x420 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_8_REG            0x4A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_9_REG            0x520 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_10_REG           0x5A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_11_REG           0x620 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_12_REG           0x6A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_13_REG           0x720 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_14_REG           0x7A0 /* Task ID required for next pointer ! */
#define CMDLST_SW_TASK_15_REG           0x820 /* Task ID required for next pointer ! */
#define CMDLST_SW_CVDR_RD_ADDR_0_REG    0xA4 
#define CMDLST_SW_CVDR_RD_ADDR_1_REG    0x124
#define CMDLST_SW_CVDR_RD_ADDR_2_REG    0x1A4
#define CMDLST_SW_CVDR_RD_ADDR_3_REG    0x224
#define CMDLST_SW_CVDR_RD_ADDR_4_REG    0x2A4
#define CMDLST_SW_CVDR_RD_ADDR_5_REG    0x324
#define CMDLST_SW_CVDR_RD_ADDR_6_REG    0x3A4
#define CMDLST_SW_CVDR_RD_ADDR_7_REG    0x424
#define CMDLST_SW_CVDR_RD_ADDR_8_REG    0x4A4
#define CMDLST_SW_CVDR_RD_ADDR_9_REG    0x524
#define CMDLST_SW_CVDR_RD_ADDR_10_REG   0x5A4
#define CMDLST_SW_CVDR_RD_ADDR_11_REG   0x624
#define CMDLST_SW_CVDR_RD_ADDR_12_REG   0x6A4
#define CMDLST_SW_CVDR_RD_ADDR_13_REG   0x724
#define CMDLST_SW_CVDR_RD_ADDR_14_REG   0x7A4
#define CMDLST_SW_CVDR_RD_ADDR_15_REG   0x824
#define CMDLST_SW_CVDR_RD_DATA_0_0_REG  0xA8  /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_1_REG  0x128 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_2_REG  0x1A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_3_REG  0x228 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_4_REG  0x2A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_5_REG  0x328 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_6_REG  0x3A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_7_REG  0x428 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_8_REG  0x4A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_9_REG  0x528 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_10_REG 0x5A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_11_REG 0x628 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_12_REG 0x6A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_13_REG 0x728 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_14_REG 0x7A8 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_0_15_REG 0x828 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_SW_CVDR_RD_DATA_1_0_REG  0xAC  /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_1_REG  0x12C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_2_REG  0x1AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_3_REG  0x22C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_4_REG  0x2AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_5_REG  0x32C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_6_REG  0x3AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_7_REG  0x42C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_8_REG  0x4AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_9_REG  0x52C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_10_REG 0x5AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_11_REG 0x62C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_12_REG 0x6AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_13_REG 0x72C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_14_REG 0x7AC /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_1_15_REG 0x82C /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_SW_CVDR_RD_DATA_2_0_REG  0xB0  /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_1_REG  0x130 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_2_REG  0x1B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_3_REG  0x230 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_4_REG  0x2B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_5_REG  0x330 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_6_REG  0x3B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_7_REG  0x430 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_8_REG  0x4B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_9_REG  0x530 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_10_REG 0x5B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_11_REG 0x630 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_12_REG 0x6B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_13_REG 0x730 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_14_REG 0x7B0 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_2_15_REG 0x830 /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_SW_CVDR_RD_DATA_3_0_REG  0xB4  /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_1_REG  0x134 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_2_REG  0x1B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_3_REG  0x234 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_4_REG  0x2B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_5_REG  0x334 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_6_REG  0x3B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_7_REG  0x434 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_8_REG  0x4B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_9_REG  0x534 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_10_REG 0x5B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_11_REG 0x634 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_12_REG 0x6B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_13_REG 0x734 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_14_REG 0x7B4 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_SW_CVDR_RD_DATA_3_15_REG 0x834 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_RESOURCES_0_REG       0xB8 
#define CMDLST_HW_RESOURCES_1_REG       0x138
#define CMDLST_HW_RESOURCES_2_REG       0x1B8
#define CMDLST_HW_RESOURCES_3_REG       0x238
#define CMDLST_HW_RESOURCES_4_REG       0x2B8
#define CMDLST_HW_RESOURCES_5_REG       0x338
#define CMDLST_HW_RESOURCES_6_REG       0x3B8
#define CMDLST_HW_RESOURCES_7_REG       0x438
#define CMDLST_HW_RESOURCES_8_REG       0x4B8
#define CMDLST_HW_RESOURCES_9_REG       0x538
#define CMDLST_HW_RESOURCES_10_REG      0x5B8
#define CMDLST_HW_RESOURCES_11_REG      0x638
#define CMDLST_HW_RESOURCES_12_REG      0x6B8
#define CMDLST_HW_RESOURCES_13_REG      0x738
#define CMDLST_HW_RESOURCES_14_REG      0x7B8
#define CMDLST_HW_RESOURCES_15_REG      0x838
#define CMDLST_HW_TASK_0_REG            0xBC  
#define CMDLST_HW_TASK_1_REG            0x13C 
#define CMDLST_HW_TASK_2_REG            0x1BC 
#define CMDLST_HW_TASK_3_REG            0x23C 
#define CMDLST_HW_TASK_4_REG            0x2BC 
#define CMDLST_HW_TASK_5_REG            0x33C 
#define CMDLST_HW_TASK_6_REG            0x3BC 
#define CMDLST_HW_TASK_7_REG            0x43C 
#define CMDLST_HW_TASK_8_REG            0x4BC 
#define CMDLST_HW_TASK_9_REG            0x53C 
#define CMDLST_HW_TASK_10_REG           0x5BC 
#define CMDLST_HW_TASK_11_REG           0x63C 
#define CMDLST_HW_TASK_12_REG           0x6BC 
#define CMDLST_HW_TASK_13_REG           0x73C 
#define CMDLST_HW_TASK_14_REG           0x7BC 
#define CMDLST_HW_TASK_15_REG           0x83C 
#define CMDLST_HW_CVDR_RD_ADDR_0_REG    0xC0 
#define CMDLST_HW_CVDR_RD_ADDR_1_REG    0x140
#define CMDLST_HW_CVDR_RD_ADDR_2_REG    0x1C0
#define CMDLST_HW_CVDR_RD_ADDR_3_REG    0x240
#define CMDLST_HW_CVDR_RD_ADDR_4_REG    0x2C0
#define CMDLST_HW_CVDR_RD_ADDR_5_REG    0x340
#define CMDLST_HW_CVDR_RD_ADDR_6_REG    0x3C0
#define CMDLST_HW_CVDR_RD_ADDR_7_REG    0x440
#define CMDLST_HW_CVDR_RD_ADDR_8_REG    0x4C0
#define CMDLST_HW_CVDR_RD_ADDR_9_REG    0x540
#define CMDLST_HW_CVDR_RD_ADDR_10_REG   0x5C0
#define CMDLST_HW_CVDR_RD_ADDR_11_REG   0x640
#define CMDLST_HW_CVDR_RD_ADDR_12_REG   0x6C0
#define CMDLST_HW_CVDR_RD_ADDR_13_REG   0x740
#define CMDLST_HW_CVDR_RD_ADDR_14_REG   0x7C0
#define CMDLST_HW_CVDR_RD_ADDR_15_REG   0x840
#define CMDLST_HW_CVDR_RD_DATA_0_0_REG  0xC4  /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_1_REG  0x144 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_2_REG  0x1C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_3_REG  0x244 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_4_REG  0x2C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_5_REG  0x344 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_6_REG  0x3C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_7_REG  0x444 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_8_REG  0x4C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_9_REG  0x544 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_10_REG 0x5C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_11_REG 0x644 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_12_REG 0x6C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_13_REG 0x744 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_14_REG 0x7C4 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_0_15_REG 0x844 /* 1st Data to write (VP_RD_CFG) */
#define CMDLST_HW_CVDR_RD_DATA_1_0_REG  0xC8  /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_1_REG  0x148 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_2_REG  0x1C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_3_REG  0x248 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_4_REG  0x2C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_5_REG  0x348 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_6_REG  0x3C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_7_REG  0x448 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_8_REG  0x4C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_9_REG  0x548 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_10_REG 0x5C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_11_REG 0x648 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_12_REG 0x6C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_13_REG 0x748 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_14_REG 0x7C8 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_1_15_REG 0x848 /* 2nd data to write (VP_RD_LWG) */
#define CMDLST_HW_CVDR_RD_DATA_2_0_REG  0xCC  /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_1_REG  0x14C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_2_REG  0x1CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_3_REG  0x24C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_4_REG  0x2CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_5_REG  0x34C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_6_REG  0x3CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_7_REG  0x44C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_8_REG  0x4CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_9_REG  0x54C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_10_REG 0x5CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_11_REG 0x64C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_12_REG 0x6CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_13_REG 0x74C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_14_REG 0x7CC /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_2_15_REG 0x84C /* 3rd Data to write (VP_RD_FHG) */
#define CMDLST_HW_CVDR_RD_DATA_3_0_REG  0xD0  /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_1_REG  0x150 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_2_REG  0x1D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_3_REG  0x250 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_4_REG  0x2D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_5_REG  0x350 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_6_REG  0x3D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_7_REG  0x450 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_8_REG  0x4D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_9_REG  0x550 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_10_REG 0x5D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_11_REG 0x650 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_12_REG 0x6D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_13_REG 0x750 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_14_REG 0x7D0 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_HW_CVDR_RD_DATA_3_15_REG 0x850 /* 4th Data to write (VP_RD_AXI_FS) */
#define CMDLST_CMD_CFG_0_REG            0xD4  /* Command configuration */
#define CMDLST_CMD_CFG_1_REG            0x154 /* Command configuration */
#define CMDLST_CMD_CFG_2_REG            0x1D4 /* Command configuration */
#define CMDLST_CMD_CFG_3_REG            0x254 /* Command configuration */
#define CMDLST_CMD_CFG_4_REG            0x2D4 /* Command configuration */
#define CMDLST_CMD_CFG_5_REG            0x354 /* Command configuration */
#define CMDLST_CMD_CFG_6_REG            0x3D4 /* Command configuration */
#define CMDLST_CMD_CFG_7_REG            0x454 /* Command configuration */
#define CMDLST_CMD_CFG_8_REG            0x4D4 /* Command configuration */
#define CMDLST_CMD_CFG_9_REG            0x554 /* Command configuration */
#define CMDLST_CMD_CFG_10_REG           0x5D4 /* Command configuration */
#define CMDLST_CMD_CFG_11_REG           0x654 /* Command configuration */
#define CMDLST_CMD_CFG_12_REG           0x6D4 /* Command configuration */
#define CMDLST_CMD_CFG_13_REG           0x754 /* Command configuration */
#define CMDLST_CMD_CFG_14_REG           0x7D4 /* Command configuration */
#define CMDLST_CMD_CFG_15_REG           0x854 /* Command configuration */
#define CMDLST_TOKEN_CFG_0_REG          0xD8  /* Token configuration */
#define CMDLST_TOKEN_CFG_1_REG          0x158 /* Token configuration */
#define CMDLST_TOKEN_CFG_2_REG          0x1D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_3_REG          0x258 /* Token configuration */
#define CMDLST_TOKEN_CFG_4_REG          0x2D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_5_REG          0x358 /* Token configuration */
#define CMDLST_TOKEN_CFG_6_REG          0x3D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_7_REG          0x458 /* Token configuration */
#define CMDLST_TOKEN_CFG_8_REG          0x4D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_9_REG          0x558 /* Token configuration */
#define CMDLST_TOKEN_CFG_10_REG         0x5D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_11_REG         0x658 /* Token configuration */
#define CMDLST_TOKEN_CFG_12_REG         0x6D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_13_REG         0x758 /* Token configuration */
#define CMDLST_TOKEN_CFG_14_REG         0x7D8 /* Token configuration */
#define CMDLST_TOKEN_CFG_15_REG         0x858 /* Token configuration */
#define CMDLST_EOS_0_REG                0xDC  /* EOS configuration */
#define CMDLST_EOS_1_REG                0x15C /* EOS configuration */
#define CMDLST_EOS_2_REG                0x1DC /* EOS configuration */
#define CMDLST_EOS_3_REG                0x25C /* EOS configuration */
#define CMDLST_EOS_4_REG                0x2DC /* EOS configuration */
#define CMDLST_EOS_5_REG                0x35C /* EOS configuration */
#define CMDLST_EOS_6_REG                0x3DC /* EOS configuration */
#define CMDLST_EOS_7_REG                0x45C /* EOS configuration */
#define CMDLST_EOS_8_REG                0x4DC /* EOS configuration */
#define CMDLST_EOS_9_REG                0x55C /* EOS configuration */
#define CMDLST_EOS_10_REG               0x5DC /* EOS configuration */
#define CMDLST_EOS_11_REG               0x65C /* EOS configuration */
#define CMDLST_EOS_12_REG               0x6DC /* EOS configuration */
#define CMDLST_EOS_13_REG               0x75C /* EOS configuration */
#define CMDLST_EOS_14_REG               0x7DC /* EOS configuration */
#define CMDLST_EOS_15_REG               0x85C /* EOS configuration */
#define CMDLST_CHECK_CTRL_0_REG         0xE0  /* check control register */
#define CMDLST_CHECK_CTRL_1_REG         0x160 /* check control register */
#define CMDLST_CHECK_CTRL_2_REG         0x1E0 /* check control register */
#define CMDLST_CHECK_CTRL_3_REG         0x260 /* check control register */
#define CMDLST_CHECK_CTRL_4_REG         0x2E0 /* check control register */
#define CMDLST_CHECK_CTRL_5_REG         0x360 /* check control register */
#define CMDLST_CHECK_CTRL_6_REG         0x3E0 /* check control register */
#define CMDLST_CHECK_CTRL_7_REG         0x460 /* check control register */
#define CMDLST_CHECK_CTRL_8_REG         0x4E0 /* check control register */
#define CMDLST_CHECK_CTRL_9_REG         0x560 /* check control register */
#define CMDLST_CHECK_CTRL_10_REG        0x5E0 /* check control register */
#define CMDLST_CHECK_CTRL_11_REG        0x660 /* check control register */
#define CMDLST_CHECK_CTRL_12_REG        0x6E0 /* check control register */
#define CMDLST_CHECK_CTRL_13_REG        0x760 /* check control register */
#define CMDLST_CHECK_CTRL_14_REG        0x7E0 /* check control register */
#define CMDLST_CHECK_CTRL_15_REG        0x860 /* check control register */
#define CMDLST_VP_WR_FLUSH_0_REG        0xE8  /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_1_REG        0x168 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_2_REG        0x1E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_3_REG        0x268 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_4_REG        0x2E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_5_REG        0x368 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_6_REG        0x3E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_7_REG        0x468 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_8_REG        0x4E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_9_REG        0x568 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_10_REG       0x5E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_11_REG       0x668 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_12_REG       0x6E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_13_REG       0x768 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_14_REG       0x7E8 /* Force VP WR close */
#define CMDLST_VP_WR_FLUSH_15_REG       0x868 /* Force VP WR close */
#define CMDLST_DEBUG_EOS_0_REG          0xEC  /* EOS processing debug */
#define CMDLST_DEBUG_EOS_1_REG          0x16C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_2_REG          0x1EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_3_REG          0x26C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_4_REG          0x2EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_5_REG          0x36C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_6_REG          0x3EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_7_REG          0x46C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_8_REG          0x4EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_9_REG          0x56C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_10_REG         0x5EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_11_REG         0x66C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_12_REG         0x6EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_13_REG         0x76C /* EOS processing debug */
#define CMDLST_DEBUG_EOS_14_REG         0x7EC /* EOS processing debug */
#define CMDLST_DEBUG_EOS_15_REG         0x86C /* EOS processing debug */
#define CMDLST_DEBUG_CHANNEL_0_REG      0xF0  /* Debug register */
#define CMDLST_DEBUG_CHANNEL_1_REG      0x170 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_2_REG      0x1F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_3_REG      0x270 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_4_REG      0x2F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_5_REG      0x370 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_6_REG      0x3F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_7_REG      0x470 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_8_REG      0x4F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_9_REG      0x570 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_10_REG     0x5F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_11_REG     0x670 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_12_REG     0x6F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_13_REG     0x770 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_14_REG     0x7F0 /* Debug register */
#define CMDLST_DEBUG_CHANNEL_15_REG     0x870 /* Debug register */

#endif // __CMDLST_REG_OFFSET_H__
