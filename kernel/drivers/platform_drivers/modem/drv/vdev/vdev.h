/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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

#ifndef __VCOM_H__
#define __VCOM_H__
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <bsp_mci.h>

struct vcom_map_s;
#define WRITE_ASYNC 1
#define QUERY_READY 2
#define INIT_DONE 3
#define WRITE_BLOCK 4
#define ACKNOWLEDGE 5
#define NET_STAT_CHG 6
#define NET_WRITE_CFG 7
#define NET_ACK_CFG 8
#define WRITE_MSC 9
#define WRITE_FLOW_CTRL 10
#define WRITE_WLEN 11
#define WRITE_STP 12
#define WRITE_EPS 13
#define WRITE_BAUD 14
#define NOTIFY_WATER_LINE 15
#define NOTIFY_ESCAPE 16
#define SET_TRANS_MODE 17
#define NOTIFY_BRIDGE_MODE 18
#define CHR_REQ 19
#define NOTIFY_READY 20
#define SET_PPP_INFO 21
#define ENABLE_CMUX 22
#define SET_CMUX_INFO 23
#define SET_CMUX_DLC 24
#define ENABLE_ESCAPE_DETECT 25
#define PAYLOAD_TYPE_MAX 26

/* dev ready def */
#define VCOM_DEV_NOT_READY  0
#define VCOM_DEV_READY      1
#define VCOM_MAX_DATA_SIZE   0x2000
#define VCOM_BUF_MAGIC 0x43

/* bastet */
#define VCOM_GET_MODEM_RAB_ID 0x5151
#define VCOM_GET_MODEM_RESET 0x5152
#define VCOM_GET_IPV6_MODEM_RAB_ID 0x5153

enum vcom_chn_id {
    VCOM_APP_0,
    VCOM_APP_1,
    VCOM_APP_2,
    VCOM_APP_3,
    VCOM_APP_4,
    VCOM_APP_5,
    VCOM_APP_6,
    VCOM_APP_7,
    VCOM_APP_8,
    VCOM_APP_9,
    VCOM_APP_10,
    VCOM_APP_11,
    VCOM_APP_12,
    VCOM_APP_13,
    VCOM_APP_14,
    VCOM_APP_15,
    VCOM_APP_16,
    VCOM_APP_17,
    VCOM_APP_18,
    VCOM_APP_19,
    VCOM_APP_20,
    VCOM_APP_21,
    VCOM_APP_22,
    VCOM_APP_23,
    VCOM_APP_24,
    VCOM_APP_25,
    VCOM_APP_26,
    VCOM_APP_27,
    VCOM_APP_28,
    VCOM_APP_29,
    VCOM_APP_30,
    VCOM_APP_31,
    VCOM_APP_32,
    VCOM_APP_33,
    VCOM_APP_34,
    VCOM_APP_35,
    VCOM_APP_36,
    VCOM_APP_37,
    VCOM_APP_38,
    VCOM_APP_39,
    VCOM_APP_40,
    VCOM_APP_41,
    VCOM_APP_42,
    VCOM_APP_43,
    VCOM_APP_44,
    VCOM_APP_45,
    VCOM_APP_46,
    VCOM_APP_47,
    VCOM_APP_48,
    VCOM_APP_49,
    VCOM_APP_50,
    VCOM_APP_51,
    VCOM_APP_52,
    VCOM_APP_53,
    VCOM_APP_54,
    VCOM_APP_55,
    VCOM_APP_56,
    VCOM_APP_57,
    VCOM_APP_58,
    VCOM_APP_59,
    VCOM_APP_60,
    VCOM_APP_61,
    VCOM_APP_62,
    VCOM_APP_63,
    VCOM_APP_ACT,
    VCOM_APP_NMCTRL,     /**< netmanager */
    VCOM_APP_BASTET,
    VCOM_APP_BRIDGE,     // 透传通道
    VCOM_APP_BRIDGE1,     // 透传通道
    VCOM_ATP_ID,
    VCOM_APP_TEST,       // VCOM_TEST
    VCOM_ACM_CTRL,       // Control Interface
    VCOM_ACM_AT,         // PC UI Interface
    VCOM_ACM_SHELL,      // VCOM_ACM2
    VCOM_ACM_LTE_DIAG,   // Application Interface
    VCOM_ACM_OM,         // VCOM_ACM4
    VCOM_ACM_MODEM,      // modem PPP
    VCOM_ACM_GPS,        // VCOM_ACM6
    VCOM_ACM_3G_GPS,     // VCOM_ACM7
    VCOM_ACM_3G_PCVOICE, // VCOM_ACM8
    VCOM_ACM_PCVOICE,    // VCOM_ACM9
    VCOM_ACM_SKYTONE,    // PCUI2(skytone)
    VCOM_ACM_CDMA_LOG,   // VCOM_ACM11
    VCOM_HSUART_ID,
    VCOM_SOCK_ID,
    VCOM_CHN_SOCKET_OM, // SOCKET CBT
    VCOM_CHN_PCIE_AT, /**< PCIE AT通道 */
    VCOM_CHN_UNET_AT, /**< USB NET AT通道 */
    VCOM_CHN_UNET_CTRL, /**< USB NET 控制通道 */
    VCOM_CHN_PNET_CTRL, /**< PCIE NET 控制通道 */
    VCOM_CBT_ID,        // VCOM_ACM_OM
    VCOM_CMUX0_ID,
    VCOM_CMUX1_ID,
    VCOM_CMUX2_ID,
    VCOM_CMUX3_ID,
    VCOM_CHN_BOTTOM,
};

#define VCOM_APP_BOTTOM VCOM_ACM_AT

/* bind_pid */
#define VCOM_PID_LEN 10
#define VCOM_BIND_PID_CONFIG_IND 0x0006
struct vcom_nm_msg_s {
    unsigned int enMsgId;
    unsigned int ulMsgLen;
    unsigned int ulBindPid;
};

struct vcom_bindpid_ctx_s {
    unsigned char pid[VCOM_PID_LEN];
    struct vcom_nm_msg_s pid_msg;
    struct mutex write_lock;
};

enum vcom_buff_type {
    VCOM_USER_TYPE,
    VCOM_KERNEL_TYPE,
};

/* common */
enum vcom_dev_type {
    VCOM_DEV_ACM,
    VCOM_DEV_HSUART,
    VCOM_DEV_SOCK,
    VCOM_DEV_NETIF_CTRL, // net 控制面通道
    VCOM_DEV_UNET_AT, // usb net at通道
    VCOM_DEV_PCIE_VDEV, // PCIE 串口通道
    VCOM_DEV_APP,
    VCOM_DEV_ACK,
    VCOM_DEV_NORTIFY,
    VCOM_DEV_MAX
};

enum mci_socket_type {
    MCI_SOCKET_AT,
    MCI_SOCKET_OM,
    MCI_SOCKET_MAX,
};

struct vcom_msg_seg {
    unsigned char *buf;
    unsigned int len;
    struct list_head list;
};

struct vcom_buf {
    unsigned int id : 16;
    unsigned int magic : 8;
    unsigned int dev_type : 8;
    unsigned int payload_len : 16;
    unsigned int payload_type : 8;
    unsigned int payload_start : 8;
    unsigned int custom;
    unsigned int reserve;
    unsigned char data[0];
};

struct vcom_dump_item {
    unsigned int time;
    unsigned short length;
    unsigned short status;
    unsigned char msg[8];
};

struct mci_hids_item {
    unsigned int time;
    unsigned short channel_id;
    unsigned short length;
    unsigned short status;
    unsigned char msg[22];
};

struct mci_hids_node {
    unsigned char *hids_data;
    unsigned int hids_size;
    struct list_head list;
};

struct mci_hids_data {
    struct mci_hids_item items[32];
};

struct mci_record_info {
    const void *buffer;
    unsigned int time;
    unsigned short size;
    unsigned short status;
};

struct mci_hids_ctx {
    struct mci_hids_data *hids_data;
    unsigned int hids_size;
    struct mutex record_lock;
    struct list_head report_list;
    spinlock_t report_lock;
    struct delayed_work report_work;
    struct workqueue_struct *report_work_queue;
    unsigned int report_succ;
    unsigned int report_fail;
};

struct mci_dump_ctx {
    unsigned char *dump_buffer;
    spinlock_t dump_lock;
};

struct vcom_debug_stax {
    struct mci_hids_ctx hids_ctx;
    struct mci_dump_ctx dump_ctx;
    unsigned int msg_rcv;
    unsigned int down_kmalloc;
    unsigned int fail_free;

    unsigned int up_kmalloc;
    unsigned int up_kfree;
    unsigned int write;
    unsigned int write_suc;

    unsigned int check_fail;
    unsigned int parse_app;
    unsigned int parse_uart;
    unsigned int parse_pcdev;
    unsigned int parse_acm;
    unsigned int parse_net_ctrl;
    unsigned int parse_socket;
    unsigned int parse_notify;
    unsigned int parse_err;
    unsigned int mci_write_succ;
    unsigned int mci_write_fail;
    unsigned int mci_read_succ;
    unsigned int mci_read_fail;
    unsigned int chr_record;
    unsigned int chr_report;
};

#define MCI_RECORD_LEN 21
struct vcom_map_stax {
    unsigned int enqueue;
    unsigned int read_drop;
    unsigned int write_drop;
    unsigned int requeue;
    unsigned int req_fail;
    unsigned int dequeue;
    unsigned int read;
    unsigned int copy_fail;
    unsigned int remain;
    unsigned int succ_free;
    unsigned int fail_free;

    struct {
        unsigned char read_record[MCI_RECORD_LEN];
        unsigned char read_size;
        unsigned char write_record[MCI_RECORD_LEN];
        unsigned char write_size;
        unsigned char app_record[MCI_RECORD_LEN];
        unsigned char app_size;
    };
};

struct vcom_cb_ops {
    int (*tx_cb)(unsigned int id, unsigned char *data, unsigned int size);
    int (*rx_cb)(unsigned int id, const unsigned char *data, unsigned int size);
};

struct vcom_link_ops {
    int (*init)(struct vcom_map_s *map);
    int (*idle)(struct vcom_map_s *map);
    int (*write)(struct vcom_map_s *map, unsigned char *buf, unsigned int size);
    int (*read)(struct vcom_map_s *map, unsigned char *buf);
    int (*enqueue)(struct vcom_map_s *map, unsigned char *buf);
    int (*queue_head)(struct vcom_map_s *map, unsigned char *buf);
    unsigned char *(*dequeue)(struct vcom_map_s *map);
};

struct vcom_map_s {
    unsigned int id;
    struct vcom_hd *hd;
    unsigned int flag;
    struct vcom_map_stax stax;
    spinlock_t link_lock;
    struct list_head high_list;
    unsigned int high_list_len;
    struct list_head low_list;
    struct vcom_link_ops *link_ops;
    struct vcom_cb_ops cb_ops;
    unsigned char ccore_ready;
};

struct vcom_hd {
    char *name;
    struct cdev cdev;
    struct vcom_map_s *map;
    bool switch_support;
    enum vcom_dev_type dev_type;
    int dev_id;
    unsigned int dump_enable;
    unsigned int dump_index;
    unsigned int relay_type;
    bool wakeup;
    int enable;
    int (*init)(struct vcom_hd *hd);
};

struct vcom_cdev {
    struct device *dev;
    struct cdev cdev;
    struct vcom_hd *hd;
    bool ready;
    wait_queue_head_t wait;
    struct mutex read_lock;
    struct mutex write_lock;
};

#define MCI_CHR_RECORD_SIZE 8
#define MCI_CHR_RECORD_NUM 10
struct mci_chr_item {
    char at_report[MCI_CHR_RECORD_SIZE];
    unsigned int count;
};

struct mci_ind_data {
    struct mci_chr_item items[MCI_CHR_RECORD_NUM];
};
typedef struct mci_ind_data MCI_IND_DATA_S;

struct vcom_app_ctx {
    struct vcom_cdev *vc;
    dev_t devt;
    dev_t id;
    struct class *class;
    struct file_operations *fop;
    struct proc_dir_entry *dir;
};

/******** mci_atp **********/
struct mci_atp_msg {
    unsigned int len;
    void __user *buffer;
}__attribute__((packed));

extern struct vcom_link_ops g_vcom_link_list_ops;
extern struct vcom_map_s *g_vdev_map;
extern int g_vcom_debug;

#define VCOM_TRACE(debug)   do {    \
                            if (debug) {   \
                                bsp_err("<%s> @line:%d\n", __func__, __LINE__);   \
                            }   \
                    } while (0)
#define VCOM_PRINT(x, ...) do {               \
                        if (g_vcom_debug)      \
                            bsp_err("<%s> " x, __func__, ##__VA_ARGS__);  \
                    } while (0)

#define VCOM_PRINT_ERROR(x, ...) do {     \
        bsp_err("<%s> " x, __func__, ##__VA_ARGS__);  \
} while (0)

int vcom_app_op(struct vcom_buf *vb);
int vdev_cdev_init(void);
int vcom_hd_app_init(struct vcom_hd *hd);
int vcom_hd_proc_init(struct vcom_hd *hd);
int vcom_ioctl(struct vcom_hd *hd, const void *buf, unsigned int size, unsigned int type);
int vcom_write(struct vcom_hd *hd, const void *buf, unsigned int size, unsigned int type);
unsigned char *vcom_read(struct vcom_hd *hd, unsigned int *size);
int vcom_requeue(struct vcom_hd *hd, struct vcom_buf *vb);
struct vcom_hd *vcom_get_hd(unsigned int id);
unsigned int vcom_get_hd_num(void);
struct vcom_hd *vcom_get_hd_from_set(unsigned int index);
int vcom_acm_op(struct vcom_buf *vb);
int vcom_uart_op(struct vcom_buf *vb);
int vcom_net_op(struct vcom_buf *vb);
int vcom_socket_op(struct vcom_buf *vb);
int vcom_hd_relay_init(struct vcom_hd *hd);
int vcom_socket_ready_init(struct vcom_hd *hd);
int vcom_msg_parse(unsigned char *buf, unsigned int len);
int vdev_msg_init(void);
void vcom_wakeup_notify(void);
struct device *vcom_create_dev(struct cdev *cdev, char *name, unsigned int id, void *priv);
int mci_atp_init(struct vcom_hd *hd);
long vdev_cdev_change_bridge_mode(struct vcom_hd *hd, unsigned int cmd, unsigned long arg);
int vcom_app_chr_report(struct vcom_buf *vb);


/* mci switch interface */
int mci_switch_init(void);
int mci_switch_te_vb_receive(struct vcom_hd *hd, struct vcom_buf *vb);
int mci_switch_mt_vb_receive(struct vcom_hd *hd, struct vcom_buf *vb);
int mci_msg_dispatch(struct vcom_buf *vb);
int mci_msg_send(struct vcom_hd *hd, struct vcom_buf *vb);

int mci_debug_init(void);
void mci_debug_record(struct vcom_hd *hd, const void *buff, unsigned int size, enum vcom_buff_type type, unsigned short status);
struct vcom_debug_stax *vcom_get_debug_stax(void);

struct vcom_map_s *vcom_get_map(unsigned int id);
int vcom_test_init(void);
struct vcom_app_ctx *get_vcom_app_ctx(void);
#ifdef CONFIG_VDEV_PHONE
int vcom_hd_bindpid_init(struct vcom_hd *hd);
#endif

#endif
