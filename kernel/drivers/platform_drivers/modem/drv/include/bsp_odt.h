/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef _BSP_ODT_H
#define _BSP_ODT_H

#include "osl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_ERR_ODT_BASE BSP_DEF_ERR(8, 0)
#define BSP_ERR_ODT_NULL (BSP_ERR_ODT_BASE + 0x1)
#define BSP_ERR_ODT_NOT_INIT (BSP_ERR_ODT_BASE + 0x2)
#define BSP_ERR_ODT_MEM_ALLOC (BSP_ERR_ODT_BASE + 0x3)
#define BSP_ERR_ODT_SEM_CREATE (BSP_ERR_ODT_BASE + 0x4)
#define BSP_ERR_ODT_TSK_CREATE (BSP_ERR_ODT_BASE + 0x5)
#define BSP_ERR_ODT_INVALID_CHAN (BSP_ERR_ODT_BASE + 0x6)
#define BSP_ERR_ODT_INVALID_PARA (BSP_ERR_ODT_BASE + 0x7)
#define BSP_ERR_ODT_NO_CHAN (BSP_ERR_ODT_BASE + 0x8)
#define BSP_ERR_ODT_SET_FAIL (BSP_ERR_ODT_BASE + 0x9)
#define BSP_ERR_ODT_TIMEOUT (BSP_ERR_ODT_BASE + 0xa)
#define BSP_ERR_ODT_NOT_8BYTESALIGN (BSP_ERR_ODT_BASE + 0xb)
#define BSP_ERR_ODT_CHAN_RUNNING (BSP_ERR_ODT_BASE + 0xc)
#define BSP_ERR_ODT_CHAN_MODE (BSP_ERR_ODT_BASE + 0xd)
#define BSP_ERR_ODT_DEST_CHAN (BSP_ERR_ODT_BASE + 0xe)
#define BSP_ERR_ODT_DECSRC_SET (BSP_ERR_ODT_BASE + 0xf)

#define ODT_CODER_DEST_CHAN 0x01
#define ODT_REAL_CHAN_ID(unique_chan_id) (unique_chan_id & 0xFFFF)
#define ODT_DEST_DSM_ENABLE 0x01
#define ODT_DEST_DSM_DISABLE 0x00

#define ODT_ENC_SRC_CHAN 0x00
#define ODT_CHAN_DEF(chan_type, chan_id) (((chan_type) << 16) | (chan_id))

typedef struct {
    u32 chan_enable;      /* 限流功能是否打开 */
    u32 rate_limits;      /* 所有通道总的限流上限，单位MB/s */
    u32 rev;              /* 保留 */
    u32 chan_rate_bits_l; /* 源通道低32个通道限流使能位 */
    u32 chan_rate_bits_h; /* 源通道高32个通道限流使能位 */
} DRV_DIAG_RATE_STRU;

enum ODT_CODER_SRC_ENUM {
    ODT_CODER_SRC_ACPU_CNF = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 0),               /* ACPU CNF */
    ODT_CODER_SRC_ACPU_MDM_BSP_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 1),       /* ACPU BSP IND */
    ODT_CODER_SRC_ACPU_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 2),               /* ACPU PS IND */
    ODT_CODER_SRC_ACPU_USER_MODE_APP_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 3), /* APPLOG IND */
    ODT_CODER_SRC_ACPU_TEE_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 4),           /* TEE OS IND */
    ODT_CODER_SRC_LPM3_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 5),               /* LPM3 IND */
    ODT_CODER_SRC_HIFI_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 6),               /* HIFI IND */
    ODT_CODER_SRC_CCPU_CNF = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 7),               /* TSP CNF */
    ODT_CODER_SRC_CCPU_BSP_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 8),           /* TSP BSP IND */
    ODT_CODER_SRC_TSP_PS_0_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 9),           /* TSP PS 0 IND */
    ODT_CODER_SRC_TSP_PHY_0_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 10),         /* TSP PHY 0 IND */
    ODT_CODER_SRC_TSP_PHY_1_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 11),         /* TSP PHY 1 IND PSA*/
    ODT_CODER_SRC_TSP_PHY_2_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 12),         /* TSP PHY 2 IND PSA */
    ODT_CODER_SRC_TSP_PHY_3_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 13),         /* TSP PHY 3 IND PSA */
    ODT_CODER_SRC_TSP_PHY_4_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 14),         /* TSP PHY 4 IND PSA */
    ODT_CODER_SRC_TSP_PHY_5_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 15),         /* TSP PHY 5 IND PSA */
    ODT_CODER_SRC_TSP_PHY_6_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 16),         /* TSP PHY 6 IND PSA */
    ODT_CODER_SRC_TSP_PHY_7_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 17),         /* TSP PHY 7 IND PSA */
    ODT_CODER_SRC_TSP_PHY_8_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 18),         /* TSP PHY 8 IND PSA */
    ODT_CODER_SRC_TSP_PHY_9_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 19),         /* TSP PHY 9 IND PSA */
    ODT_CODER_SRC_TSP_PHY_10_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 20),        /* TSP PHY 10 IND PSA */
    ODT_CODER_SRC_TSP_PHY_11_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 21),        /* TSP PHY 11 IND PSA */
    ODT_CODER_SRC_TSP_PHY_12_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 22),        /* TSP PHY 12 IND PSA */
    ODT_CODER_SRC_TSP_PHY_13_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 23),        /* TSP PHY 13 IND PSA */
    ODT_CODER_SRC_TSP_PHY_14_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 24),        /* TSP PHY 14 IND PSA */
    ODT_CODER_SRC_TSP_PHY_15_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 25),        /* TSP PHY 15 IND PSA */
    ODT_CODER_SRC_TSP_PHY_16_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 26),        /* TSP PHY 16 IND PSA */
    ODT_CODER_SRC_BBP_BUS_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 27),           /* BBP BUS Sample */
    ODT_CODER_SRC_BBP_DATA_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 28),          /* BBP DATA Sample */
    ODT_CODER_SRC_EASYRF0_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 29),           /* EasyRF 0 IND */
    ODT_CODER_SRC_EASYRF1_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 30),           /* EasyRF 1 IND */
    ODT_CODER_SRC_PDE0_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 31),              /* PDE CORE0 IND */
    ODT_CODER_SRC_PDE1_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 32),              /* PDE CORE1 IND */
    ODT_CODER_SRC_PDE2_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 33),              /* PDE CORE2 IND */
    ODT_CODER_SRC_PDE3_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 34),              /* PDE CORE3 IND */
    ODT_CODER_SRC_PDE4_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 35),              /* PDE CORE4 IND */
    ODT_CODER_SRC_PDE5_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 36),              /* PDE CORE5 IND */
    ODT_CODER_SRC_PDE6_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 37),              /* PDE CORE6 IND */
    ODT_CODER_SRC_PDE7_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 38),              /* PDE CORE7 IND */
    ODT_CODER_SRC_PDE8_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 39),              /* PDE CORE8 IND */
    ODT_CODER_SRC_L2_FPHAC_0_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 40),        /* L2DLE 0 IND */
    ODT_CODER_SRC_L2_FPHAC_1_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 41),        /* L2DLE 1 IND */
    ODT_CODER_SRC_L2_FPHAC_2_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 42),        /* L2DLE 2 IND */
    ODT_CODER_SRC_L2_FPHAC_3_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 43),        /* L2DLE 3 IND */
    ODT_CODER_SRC_TSP_SEC_DUMP = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 44),          /* TSP SEC DUMP (list) */
    ODT_CODER_SRC_PFA_TFT_IND = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 45),           /* IPF */
    ODT_CODER_SRC_LPM3_SEC_DUMP = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 46),         /* LPM3 SEC DUMP (list) */
    ODT_CODER_SRC_TSP_MPROBE = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 47),            /* MPROBE CCPU */
    ODT_CODER_SRC_AP_MPROBE = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 48),             /* MPROBE ACPU */
    /* 50~64 reserved */
    ODT_CODER_SRC_BUTT = ODT_CHAN_DEF(ODT_ENC_SRC_CHAN, 64)
};


/* 编码目的通道ID枚举定义 */
typedef enum {
    ODT_CODER_DST_OM_CNF = ODT_CHAN_DEF(ODT_CODER_DEST_CHAN, 0),    /**< diag cnf channel */
    ODT_CODER_DST_OM_IND = ODT_CHAN_DEF(ODT_CODER_DEST_CHAN, 1),    /**< diag ind channel */
    ODT_CODER_DST_DUMP = ODT_CHAN_DEF(ODT_CODER_DEST_CHAN, 2),      /**< secure dump channel */
    ODT_CODER_DST_LOGSERVER = ODT_CHAN_DEF(ODT_CODER_DEST_CHAN, 3), /**< log server channel */
    ODT_CODER_DST_BUTT
} odt_coder_dst_e;

typedef enum {
    ODT_HDLC_ENABLE = 0,  /**< odt hdlc enable */
    ODT_HDLC_DISABLE = 1, /**< odt hdlc disable */
    ODT_HDLC_FLAG_BUTT
} odt_hdlc_flag_e;

typedef enum {
    ODT_TRANS_ID_DIS = 0, /**< trans id disable, default */
    ODT_TRANS_ID_EN,      /**< trans id enable */
    ODT_TRANS_ID_EN_BUTT
} odt_trans_id_en_e;

typedef enum {
    ODT_PTR_IMG_DIS = 0, /**< pointer image disable, default */
    ODT_PTR_IMG_EN,      /**< pointer image enable */
    ODT_PTR_IMG_EN_BUTT  /**< pointer image butt */
} odt_ptr_img_en_e;

typedef enum {
    ODT_DATA_TYPE_0 = 0, /**< lte oam data type */
    ODT_DATA_TYPE_1,     /**< gu oam data type */
    ODT_DATA_TYPE_2,     /**< reserve */
    ODT_DATA_TYPE_3,     /**< reserve */
    ODT_DATA_TYPE_BUTT   /**< data type butt */
} odt_data_type_e;

typedef enum {
    ODT_DATA_TYPE_EN = 0, /**< data type enable, default */
    ODT_DATA_TYPE_DIS,    /**< data type disable */
    ODT_DATA_TYPE_EN_BUTT /**< data type butt */
} odt_data_type_en_e;

typedef enum {
    ODT_ENC_DEBUG_DIS = 0, /**< debug disable, default */
    ODT_ENC_DEBUG_EN,      /**< debug enable */
    ODT_ENC_DEBUG_EN_BUTT  /**< debug state butt */
} odt_enc_debug_en_e;

typedef enum {
    ODT_ENCSRC_CHNMODE_CTSPACKET = 0, /**< coder source block ring buffer */
    ODT_ENCSRC_CHNMODE_FIXPACKET,     /**< coder source fix length ring buffer */
    ODT_ENCSRC_CHNMODE_LIST,          /**< coder source list ring buffer */
    ODT_ENCSRC_CHNMODE_BUTT
} odt_encsrc_chnmode_e;

typedef enum {
    ODT_CHAN_PRIORITY_0 = 0, /**< lowest priority */
    ODT_CHAN_PRIORITY_1,     /**< lower priority */
    ODT_CHAN_PRIORITY_2,     /**< higher priority */
    ODT_CHAN_PRIORITY_3,     /**< highest priority */
    ODT_CHAN_PRIORITY_BUTT   /**< priority butt */
} odt_chan_priority_e;

typedef struct {
    unsigned char *input_start;    /**< block buffer start address */
    unsigned char *input_end;      /**< block buffer end address */
    unsigned char *rd_input_start; /**< rd buffer start address */
    unsigned char *rd_input_end;   /**< rd buffer end address */
    unsigned int rd_threshold;     /**< rd buffer report threshold */
    unsigned short rd_timeout;     /**< rd buffer time out cost */
    unsigned short rd_irq_mask;    /**< rd buffer irq mask */
} odt_src_setbuf_s;

#pragma pack(push)
#pragma pack(4)
typedef struct {
    char *buffer;
    char *rb_buffer;
    unsigned int size;
    unsigned int rb_size;
} odt_buffer_rw_s;

typedef struct {
    unsigned int dest_chan_id;               /**< dest channel id */
    odt_hdlc_flag_e bypass_en;              /**< bypass enable or not */
    odt_trans_id_en_e trans_id_en;          /**< trans id enable or not */
    odt_ptr_img_en_e rptr_img_en;           /**< pointer image enable or not */
    odt_data_type_e data_type;              /**< data type */
    odt_data_type_en_e data_type_en;        /**< data type enable or not */
    odt_enc_debug_en_e debug_en;            /**< debug enable or not */
    odt_encsrc_chnmode_e mode;              /**< channel mode */
    odt_chan_priority_e priority;           /**< channel priority */
    unsigned long read_ptr_img_phy_addr;     /**< block, read pointer image phyical address */
    unsigned long read_ptr_img_vir_addr;     /**< block, read pointer image virtual address */
    unsigned int rd_write_ptr_img_addr_low;  /**< rd, write pointer image low address  */
    unsigned int rd_write_ptr_img_addr_high; /**< rd, write pointer image high address */
    unsigned int chan_group;                 /**< odt source channel group number */
    odt_src_setbuf_s coder_src_setbuf;      /**< source channel buffer set for */
} odt_src_chan_cfg_s;
#pragma pack(pop)

typedef enum {
    ODT_EVENT_PKT_HEADER_ERROR = 0x1,               /**< odt packet header is error */
    ODT_EVENT_OUTBUFFER_OVERFLOW = 0x2,             /**< odt dest channel is overflow */
    ODT_EVENT_RDBUFFER_OVERFLOW = 0x4,              /**< odt dest channel rd buffer is overflow */
    ODT_EVENT_DECODER_UNDERFLOW = 0x8,              /**< odt deccode source channel is underflow */
    ODT_EVENT_PKT_LENGTH_ERROR = 0x10,              /**< odt deccode packet length error */
    ODT_EVENT_CRC_ERROR = 0x20,                     /**< odt deccode packet crc is error */
    ODT_EVENT_DATA_TYPE_ERROR = 0x40,               /**< odt deccode data type is error */
    ODT_EVENT_HDLC_HEADER_ERROR = 0x80,             /**< odt deccode hdlc header is error */
    ODT_EVENT_OUTBUFFER_THRESHOLD_OVERFLOW = 0x100, /**< odt dest channel is threshold overflow */
    ODT_EVENT_BUTT
} odt_event_e;

typedef enum {
    ODT_BD_DATA = 0, /**< data type  */
    ODT_BD_LIST = 1, /**< list type */
    ODT_BD_BUTT      /**< hdlc flag butt */
} odt_bd_type_e;

typedef enum {
    ODT_TIMEOUT_TFR_START,
    ODT_TIMEOUT_TFR_LONG,
    ODT_TIMEOUT_TFR_SHORT,
    ODT_TIMEOUT_TFR_BUTT
}odt_timeout_en_e;

typedef enum {
    ODT_DST_CHAN_NOT_CFG = 0,      /* 内存无配置，默认值 */
    ODT_DST_CHAN_DELAY,            /* 使用预留内存 */
    ODT_DST_CHAN_DTS,              /* 根据DTS配置申请，用于无预留buffer的场景 */
    ODT_DST_CHAN_MIN               /* 申请的保护性内存，64KBytes */
}odt_dst_chan_cfg_type_e;

typedef enum {
    ODT_IND_MODE_DIRECT, /* *< direct report mode */
    ODT_IND_MODE_DELAY,  /* *< delay report mode, buffers for buffer */
    ODT_IND_MODE_CYCLE   /* *< cycle report mode, buffer override */
} odt_ind_mode_e;

struct odt_enc_dst_log_cfg {
    void*           virt_addr;
    unsigned long   phy_addr;
    unsigned int    buff_size;
    unsigned int    over_time;
    unsigned int    log_on_flag;
    unsigned int    cur_time_out;
};

typedef struct {
    void*           virt_addr;
    unsigned long   phy_addr;
    unsigned int    buff_size;
    unsigned int    buff_useable;
    unsigned int    timeout;
    unsigned int    init_flag;
}odt_rsv_mem_s;

typedef struct {
    unsigned long long data_addr; /**< bd data buffer address */
    unsigned short data_len;      /**< bd data buffer length */
    unsigned short data_type;     /**< bd data buffer type */
    unsigned int reservd;         /**< reservd */
} odt_bd_data_s;

typedef struct {
    unsigned long long data_addr; /**< rd data buffer address */
    unsigned short data_len;      /**< rd data buffer length */
    unsigned short data_type;     /**< rd data buffer type */
    unsigned int reservd;         /**< reservd */
} odt_rd_data_s;

typedef struct {
    unsigned char *output_start; /**< output buffer start address */
    unsigned char *output_end;   /**< output buffer end address */
    unsigned int threshold;      /**< buffer report threshold */
    unsigned int rsv;            /**< reserve */
} odt_dst_setbuf_s;

typedef struct {
    unsigned int encdst_thrh;           /**< coder dest channel buffer threshold */
    unsigned int encdst_timeout_mode;   /**< coder dest channel timeout mode, after odt2.06 use */
    odt_dst_setbuf_s coder_dst_setbuf; /**< dest channel buffer set for */
} odt_dst_chan_cfg_s;

typedef struct {
    unsigned long long vir_buffer;              /**< odt coder dest channel buffer virtual address */
    unsigned long long phy_buffer_addr; /**< odt coder dest channel buffer phyical address */
    unsigned int buffer_size;      /**< odt coder dest channel buffer size */
    unsigned int over_time;        /**< odt coder dest channel buffer timeout */
    unsigned int log_on_flag;      /**< odt coder dest channel buffer log is on flag */
    unsigned int cur_time_out;     /**< odt coder dest channel buffer current timeout cost */
} odt_encdst_buf_log_cfg_s;

typedef int (*odt_event_cb)(unsigned int chan_id, unsigned int event, unsigned int param);

typedef int (*odt_read_cb)(unsigned int chan_id);

typedef int (*odt_rd_cb)(unsigned int chan_id);

#ifndef CONFIG_ODT_DISABLE
void bsp_odt_encdst_dsm_init(u32 enc_dst_chan_id, u32 b_enable);

void bsp_odt_data_send_manager(u32 enc_dst_chan_id, u32 b_enable);

s32 bsp_odt_coder_set_src_chan(unsigned int src_chan_id, odt_src_chan_cfg_s *src_attr);

s32 bsp_odt_coder_set_dst_chan(u32 dst_chan_id, odt_dst_chan_cfg_s *dst_attr);

s32 bsp_odt_free_channel(u32 chan_id);

s32 bsp_odt_start(u32 src_chan_id);

s32 bsp_odt_stop(u32 src_chan_id);

void bsp_odt_set_timeout(odt_timeout_en_e time_out_en, u32 time_out);

s32 bsp_odt_get_write_buff(u32 src_chan_id, odt_buffer_rw_s *p_rw_buff);

s32 bsp_odt_write_done(u32 src_chan_id, u32 write_size);

s32 bsp_odt_register_rd_cb(u32 src_chan_id, odt_rd_cb rd_cb);

s32 bsp_odt_get_rd_buffer(u32 src_chan_id, odt_buffer_rw_s *p_rw_buff);

s32 bsp_odt_read_rd_done(u32 src_chan_id, u32 rd_size);

s32 bsp_odt_register_read_cb(u32 dst_chan_id, odt_read_cb read_cb);

s32 bsp_odt_get_read_buff(u32 dst_chan_id, odt_buffer_rw_s *buffer);

s32 bsp_odt_read_data_done(u32 dst_chan_id, u32 read_size);

void bsp_odt_set_enc_dst_threshold(u32 mode, u32 dst_chan_id);

s32 bsp_odt_set_ind_mode(odt_ind_mode_e mode);

u32 bsp_get_odt_ind_dst_int_slice(void);

s32 bsp_clear_odt_src_buffer(u32 src_chan_id);

void bsp_odt_mntn_enc_dst_int_info(u32 *trf_info, u32 *thrh_ovf_info);

s32 bsp_odt_dst_channel_enable(u32 dst_chan_id);

s32 bsp_odt_dst_channel_disable(u32 dst_chan_id);

s32 bsp_odt_dst_trans_id_disable(u32 dst_chan_id);

void bsp_odt_clear_dst_buffer(u32 dst_chan_id);

void bsp_odt_get_log_cfg(odt_encdst_buf_log_cfg_s *cfg);

void bsp_odt_set_rate_threshold(short rate_limits);

s32 bsp_odt_free_enc_dst_chan(u32 dst_chan_id);

s32 bsp_odt_set_rate_ctrl(DRV_DIAG_RATE_STRU *p_rate_ctrl);

u8 *bsp_odt_get_pdev_addr(void);

void bsp_odt_set_stress_flag(u32 flag);

s32 bsp_odt_get_ind_chan_mode(void);

struct platform_device *bsp_get_odt_pdev(void);

/* 返回编码源通道状态, 0:idle; 1:busy */
u32 bsp_odt_get_single_src_state(u32 src_chan_id);
#else
static inline void bsp_odt_set_stress_flag(u32 flag)
{
    return;
}

static inline s32 bsp_odt_get_write_buff(u32 src_chan_id, odt_buffer_rw_s *p_rw_buff)
{
    return 0;
}

static inline s32 bsp_odt_write_done(u32 src_chan_id, u32 write_size)
{
    return 0;
}

static inline s32 bsp_odt_coder_set_dst_chan(u32 dst_chan_id, odt_dst_chan_cfg_s *dst_attr)
{
    return 0;
}
static inline s32 bsp_odt_coder_set_src_chan(unsigned int src_chan_id, odt_src_chan_cfg_s *src_attr)
{
    return 0;
}

static inline s32 bsp_odt_start(u32 src_chan_id)
{
    return 0;
}
static inline u32 bsp_get_odt_ind_dst_int_slice(void)
{
    return 0;
}
static inline s32 bsp_odt_set_ind_mode(odt_ind_mode_e mode)
{
    return 0;
}
static inline s32 bsp_odt_get_read_buff(u32 dst_chan_id, odt_buffer_rw_s *p_buffer)
{
    return 0;
}
static inline void bsp_odt_get_log_cfg(odt_encdst_buf_log_cfg_s *cfg)
{
    return;
}
static inline s32 bsp_odt_read_data_done(u32 dst_chan_id, u32 read_size)
{
    return 0;
}
static inline s32 bsp_odt_register_read_cb(u32 dst_chan_id, odt_read_cb read_cb)
{
    return 0;
}
static inline void bsp_odt_encdst_dsm_init(u32 enc_dst_chan_id, u32 b_enable)
{
    return;
}

static inline void bsp_odt_mntn_enc_dst_int_info(u32 *trf_info, u32 *thrh_ovf_info)
{
    return;
}

static inline s32 bsp_odt_dst_channel_enable(u32 dst_chan_id)
{
    return 0;
}

static inline s32 bsp_odt_dst_channel_disable(u32 dst_chan_id)
{
    return 0;
}

static inline s32 bsp_odt_set_rate_ctrl(DRV_DIAG_RATE_STRU *p_rate_ctrl)
{
    return 0;
}
static inline s32 bsp_odt_dst_trans_id_disable(u32 dst_chan_id)
{
    return 0;
}
static inline void bsp_odt_clear_dst_buffer(u32 dst_chan_id)
{
    return;
}
static inline void bsp_odt_set_rate_threshold(short rate_limits)
{
    return;
}
static inline s32 bsp_odt_get_ind_chan_mode(void)
{
    return 0;
}

struct platform_device *bsp_get_odt_pdev(void)
{
    return NULL;
}

static inline u32 bsp_odt_get_single_src_state(u32 src_chan_id)
{
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* end of _BSP_ODT_H */
