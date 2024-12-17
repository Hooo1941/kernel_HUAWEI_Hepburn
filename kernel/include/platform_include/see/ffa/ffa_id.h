/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: ffa IDs
 *
 * Create: 2022-6-28
 */
#pragma once

#ifndef __FFA_INTERL_USE
#error "never use ffa ffa id outside, use interface ffa.h"
#endif

/*
 * The macros below are used to identify FFA calls from the SMC function ID.
 * It must be same with spmd in el3.
 */
#define FFA_FNUM_MIN_VALUE                  0x60U
#define FFA_FNUM_MAX_VALUE                  0x87U
#define FFA_CALL_MAX_VALUE                  0xFFU
#define ffa_call_id(fid)                    ((fid) & FFA_CALL_MAX_VALUE)

/* FF-A error codes. */
#define FFA_NOT_SUPPORTED                   (-1)
#define FFA_INVALID_PARAMETERS              (-2)
#define FFA_NO_MEMORY                       (-3)
#define FFA_BUSY                            (-4)
#define FFA_INTERRUPTED                     (-5)
#define FFA_DENIED                          (-6)
#define FFA_RETRY                           (-7)
#define FFA_ABORTED                         (-8)
#define FFA_NO_DATA                         (-9)

/* FF-A function identifiers. */
#define FFA_ERROR_32                        0x84000060U
#define FFA_SUCCESS_32                      0x84000061U
#define FFA_SUCCESS_64                      0xC4000061U
#define FFA_INTERRUPT_32                    0x84000062U
#define FFA_VERSION_32                      0x84000063U
#define FFA_FEATURES_32                     0x84000064U
#define FFA_RX_RELEASE_32                   0x84000065U
#define FFA_RXTX_MAP_32                     0x84000066U
#define FFA_RXTX_MAP_64                     0xC4000066U
#define FFA_RXTX_UNMAP_32                   0x84000067U
#define FFA_PARTITION_INFO_GET_32           0x84000068U
#define FFA_ID_GET_32                       0x84000069U
#define FFA_MSG_POLL_32                     0x8400006AU /* Legacy FF-A v1.0 */
#define FFA_MSG_WAIT_32                     0x8400006BU
#define FFA_YIELD_32                        0x8400006CU
#define FFA_RUN_32                          0x8400006DU
#define FFA_MSG_SEND_32                     0x8400006EU /* Legacy FF-A v1.0 */
#define FFA_MSG_SEND_DIRECT_REQ_32          0x8400006FU
#define FFA_MSG_SEND_DIRECT_REQ_64          0xC400006FU
#define FFA_MSG_SEND_DIRECT_RESP_32         0x84000070U
#define FFA_MSG_SEND_DIRECT_RESP_64         0xC4000070U
#define FFA_MEM_DONATE_32                   0x84000071U
#define FFA_MEM_LEND_32                     0x84000072U
#define FFA_MEM_SHARE_32                    0x84000073U
#define FFA_MEM_RETRIEVE_REQ_32             0x84000074U
#define FFA_MEM_RETRIEVE_RESP_32            0x84000075U
#define FFA_MEM_RELINQUISH_32               0x84000076U
#define FFA_MEM_RECLAIM_32                  0x84000077U
#define FFA_MEM_FRAG_RX_32                  0x8400007AU
#define FFA_MEM_FRAG_TX_32                  0x8400007BU
#define FFA_NORMAL_WORLD_RESUME             0x8400007CU

/* FF-A v1.1 */
#define FFA_NOTIFICATION_BITMAP_CREATE_32   0x8400007DU
#define FFA_NOTIFICATION_BITMAP_DESTROY_32  0x8400007EU
#define FFA_NOTIFICATION_BIND_32            0x8400007FU
#define FFA_NOTIFICATION_UNBIND_32          0x84000080U
#define FFA_NOTIFICATION_SET_32             0x84000081U
#define FFA_NOTIFICATION_GET_32             0x84000082U
#define FFA_NOTIFICATION_INFO_GET_64        0xC4000083U
#define FFA_RX_ACQUIRE_32                   0x84000084U
#define FFA_SPM_ID_GET_32                   0x84000085U
#define FFA_MSG_SEND2_32                    0x84000086U
#define FFA_SECONDARY_EP_REGISTER_64        0xC4000087U
#define FFA_MEM_PERM_GET_32                 0x84000088U
#define FFA_MEM_PERM_SET_32                 0x84000089U
#define FFA_MEM_PERM_GET_64                 0xC4000088U
#define FFA_MEM_PERM_SET_64                 0xC4000089U
#define FFA_THEE_TEST_ID                    0x840000FFU