/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "ipc_resource.h"

struct ipc_handle g_ipc_res_lpmcu2acpu_ns[] = {
    {
        .res_id = IPC_ACPU_INT_SRC_MCU_DUMP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_CMCPU_WAKEUP,
        .reg_idx = 0x1,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_CMCPU_LOCKUP,
        .reg_idx = 0x2,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_MCPU_VERIFYMDM,
        .reg_idx = 0x3,
    }
};

struct ipc_handle g_ipc_res_modem2acpu_ns[] = {
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_DUMP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_RESET_DSS0_IDLE,
        .reg_idx = 0x1,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_RESET_DSS1_IDLE,
        .reg_idx = 0x2,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_RESET_DSS2_IDLE,
        .reg_idx = 0x3,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_RESET_DSS3_IDLE,
        .reg_idx = 0x4,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_RESET_SUCC,
        .reg_idx = 0x5,
    },
    {
        .res_id = IPC_ACORE_INT_SRC_ANY_IPCNS_TEST,
        .reg_idx = 0x6,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_TSP_HIFIRESET,
        .reg_idx = 0x7,
    }
};

struct ipc_handle g_ipc_res_acpu2lpmcu_ns[] = {
    {
        .res_id = IPC_MCU_INT_SRC_ACPU_DUMP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_MCU_INT_SRC_ACPU_CCPU_START,
        .reg_idx = 0x1,
    },
    {
        .res_id = IPC_MCU_INT_SRC_ACPU_POWER_DOWN,
        .reg_idx = 0x2,
    }
};

struct ipc_handle g_ipc_res_acpu2modem0_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_DUMP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_DSS0_STAYUP,
        .reg_idx = 0x1,
    },
    {
        .res_id = IPC_TSP_INT_SRC_ANY_IPCNS_TEST,
        .reg_idx = 0x2,
    },
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_HIFIRESET,
        .reg_idx = 0x3,
    },
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_RFILE_NOTIFY,
        .reg_idx = 0x4,
    }
};

struct ipc_handle g_ipc_res_acpu2modem1_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_DSS1_STAYUP,
        .reg_idx = 0x0,
    }
};

struct ipc_handle g_ipc_res_acpu2modem2_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_DSS2_STAYUP,
        .reg_idx = 0x0,
    }
};

struct ipc_handle g_ipc_res_acpu2modem3_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_ACPU_DSS3_STAYUP,
        .reg_idx = 0x0,
    }
};

struct ipc_handle g_ipc_res_lpmcu2modem0_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS0_WAKAEUP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS0_LOCKUP,
        .reg_idx = 0x1,
    }
};

struct ipc_handle g_ipc_res_lpmcu2modem1_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS1_WAKAEUP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS1_LOCKUP,
        .reg_idx = 0x1,
    }
};

struct ipc_handle g_ipc_res_lpmcu2modem2_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS2_WAKAEUP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS2_LOCKUP,
        .reg_idx = 0x1,
    }
};

struct ipc_handle g_ipc_res_lpmcu2modem3_ns[] = {
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS3_WAKAEUP,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_TSP_INT_SRC_AMCPU_DSS3_LOCKUP,
        .reg_idx = 0x1,
    }
};

struct ipc_handle g_ipc_res_dpa2acpu_ns[] = {
    {
        .res_id = IPC_ACPU_INT_SRC_DPA_POWERDOWN,
        .reg_idx = 0x0,
    },
    {
        .res_id = IPC_ACPU_INT_SRC_DPA_DUMP,
        .reg_idx = 0x1,
    }
};

ipcmsg_channel_dts_t g_ipcmsg_ns_channels_dts[] = {
    {
        .chn_cfg = IPCMSG_CHN_LPMCU2ACPU_IPC, /* LPMCU -> ACPU */
        .ipc_res_num = sizeof(g_ipc_res_lpmcu2acpu_ns) / sizeof(g_ipc_res_lpmcu2acpu_ns[0]),
        .ipc_res = g_ipc_res_lpmcu2acpu_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_MODEM2ACPU_IPC, /* MODEM -> ACPU */
        .ipc_res_num = sizeof(g_ipc_res_modem2acpu_ns) / sizeof(g_ipc_res_modem2acpu_ns[0]),
        .ipc_res = g_ipc_res_modem2acpu_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_ACPU2LPMCU_IPC, /* ACPU -> LPMCU */
        .ipc_res_num = sizeof(g_ipc_res_acpu2lpmcu_ns) / sizeof(g_ipc_res_acpu2lpmcu_ns[0]),
        .ipc_res = g_ipc_res_acpu2lpmcu_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_ACPU2MODEM0_IPC, /* ACPU -> MODEM0 */
        .ipc_res_num = sizeof(g_ipc_res_acpu2modem0_ns) / sizeof(g_ipc_res_acpu2modem0_ns[0]),
        .ipc_res = g_ipc_res_acpu2modem0_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_ACPU2MODEM1_IPC, /* ACPU -> MODEM1 */
        .ipc_res_num = sizeof(g_ipc_res_acpu2modem1_ns) / sizeof(g_ipc_res_acpu2modem1_ns[0]),
        .ipc_res = g_ipc_res_acpu2modem1_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_ACPU2MODEM2_IPC, /* ACPU -> MODEM2 */
        .ipc_res_num = sizeof(g_ipc_res_acpu2modem2_ns) / sizeof(g_ipc_res_acpu2modem2_ns[0]),
        .ipc_res = g_ipc_res_acpu2modem2_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_ACPU2MODEM3_IPC, /* ACPU -> MODEM3 */
        .ipc_res_num = sizeof(g_ipc_res_acpu2modem3_ns) / sizeof(g_ipc_res_acpu2modem3_ns[0]),
        .ipc_res = g_ipc_res_acpu2modem3_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_LPMCU2MODEM0_IPC, /* LPMCU -> MODEM0 */
        .ipc_res_num = sizeof(g_ipc_res_lpmcu2modem0_ns) / sizeof(g_ipc_res_lpmcu2modem0_ns[0]),
        .ipc_res = g_ipc_res_lpmcu2modem0_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_LPMCU2MODEM1_IPC, /* LPMCU -> MODEM1 */
        .ipc_res_num = sizeof(g_ipc_res_lpmcu2modem1_ns) / sizeof(g_ipc_res_lpmcu2modem1_ns[0]),
        .ipc_res = g_ipc_res_lpmcu2modem1_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_LPMCU2MODEM2_IPC, /* LPMCU -> MODEM2 */
        .ipc_res_num = sizeof(g_ipc_res_lpmcu2modem2_ns) / sizeof(g_ipc_res_lpmcu2modem2_ns[0]),
        .ipc_res = g_ipc_res_lpmcu2modem2_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_LPMCU2MODEM3_IPC, /* LPMCU -> MODEM3 */
        .ipc_res_num = sizeof(g_ipc_res_lpmcu2modem3_ns) / sizeof(g_ipc_res_lpmcu2modem3_ns[0]),
        .ipc_res = g_ipc_res_lpmcu2modem3_ns,
    },
    {
        .chn_cfg = IPCMSG_CHN_DPA2ACPU_IPC, /* DPA -> ACPU */
        .ipc_res_num = sizeof(g_ipc_res_dpa2acpu_ns) / sizeof(g_ipc_res_dpa2acpu_ns[0]),
        .ipc_res = g_ipc_res_dpa2acpu_ns,
    }
};

ipcmsg_device_dts_t g_ipcmsg_devices_dts[] = {
    /* IPCMSG_NS */
    {
        .channel_cnt = sizeof(g_ipcmsg_ns_channels_dts) / sizeof(g_ipcmsg_ns_channels_dts[0]),
        .ipcmsg_channel_dts = g_ipcmsg_ns_channels_dts,
    },
};

ipcmsg_root_dts_t g_ipcmsg_dtsroot = {
    .device_cnt = sizeof(g_ipcmsg_devices_dts) / sizeof(g_ipcmsg_devices_dts[0]),
    .ipcmsg_device_dts = g_ipcmsg_devices_dts,
};

ipcmsg_root_dts_t *ipcmsg_of_find_root_node(void)
{
    return &g_ipcmsg_dtsroot;
}
