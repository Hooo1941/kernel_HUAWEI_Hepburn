/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

#include <mdrv_pcdev.h>
#include <bsp_pcdev.h>
#include "pcie_cdev_portinfo.h"

#include <linux/usb/bsp_acm.h>
#include <linux/usb/drv_acm.h>
#include <linux/usb/drv_udi.h>

#define BUF_NUM_16  16
#define BUF_NUM_128 128
#define BUF_NUM_512 512

#define PUR_MSG_ERR (1 << 0)
#define PUR_MSG_INFO (1 << 1)
#define PUR_MSG_TRACE (1 << 2)
#define PUR_PRINT_PKG_U2P (1 << 3)
#define PUR_PRINT_PKG_P2U (1 << 4)

static unsigned int g_pur_msg_level = PUR_MSG_ERR;

#define PUR_ERR(fmt, ...) do { \
    if (g_pur_msg_level & PUR_MSG_ERR) {                     \
        pr_err("[pur]<%s> " fmt, __func__, ##__VA_ARGS__); \
    }                                                      \
} while (0)

#define PUR_INFO(fmt, ...) do { \
    if (g_pur_msg_level & PUR_MSG_INFO) {                    \
        pr_err("[pur]<%s> " fmt, __func__, ##__VA_ARGS__); \
    }                                                      \
} while (0)

#define PUR_TRACE(fmt, ...) do { \
    if (g_pur_msg_level & PUR_MSG_TRACE) {                   \
        pr_err("[pur]<%s> " fmt, __func__, ##__VA_ARGS__); \
    }                                                      \
} while (0)

/* ********************** STRUCT ***************************** */
struct pur_stat_s {
    unsigned int open;
    unsigned int opened; /* port has opened */
    unsigned int open_fail;
    unsigned int close;
    unsigned int get_rd_buf_fail;

    unsigned int ret_dirct; /* data didn't send, just return buffer directly */
    unsigned int ret_dirct_len;
    unsigned int ret_dirct_fail;
    unsigned int ret_buf; /* success free buf */
    unsigned int ret_buf_fail;
    unsigned int read;
    unsigned int read_len;
    unsigned int write;
    unsigned int write_len;
    unsigned int write_done;
    unsigned int write_done_len;
    unsigned int not_ready;
    unsigned int free_cb;
};

struct pur_adp_device {
    unsigned int dev_id;
    char *name;
    void *usb_handler;
    void *pcie_handler;

    unsigned int usb_ready;
    unsigned int print_pkt_en;

    struct pur_stat_s usb;
    struct pur_stat_s pcie;

    ACM_EVT_E port_status;
    pcdev_mdm_msc_stru *u_modem_msc;
    pcdev_mdm_msc_stru *p_modem_msc;
    unsigned int b_enable;

    ACM_READ_BUFF_INFO u_rd_buff_info;
    ACM_READ_DONE_CB_T u_read_done_cb;
    ACM_WRITE_DONE_CB_T u_write_done_cb;
    ACM_EVENT_CB_T u_evt_cb;
    ACM_MODEM_MSC_READ_CB_T u_msc_read_cb;
    ACM_MODEM_REL_IND_CB_T u_rel_ind_cb;
    ACM_FREE_CB_T u_free_cb;

    pcdev_read_buf_info_s p_rd_buff_info;
    pcdev_read_done_cb_t p_read_done_cb;
    pcdev_write_done_cb_t p_write_done_cb;
    pcdev_modem_msc_write_cb_t p_msc_write_cb;
};

void usb_read_done_cb(u32 dev_id);
void pcie_read_done_cb(u32 dev_id);

#define READ_DONE_CB(dev, _name)                 \
    static void dev##_read_done_cb_##_name(void) \
    {                                            \
        dev##_read_done_cb(_name);               \
    }

#define READ_DONE_CB_NAME(dev, _name) (dev##_read_done_cb_##_name)

READ_DONE_CB(usb, UDI_USB_ACM_CTRL);
READ_DONE_CB(usb, UDI_USB_ACM_AT);
READ_DONE_CB(usb, UDI_USB_ACM_SHELL);
READ_DONE_CB(usb, UDI_USB_ACM_LTE_DIAG);
READ_DONE_CB(usb, UDI_USB_ACM_OM);
READ_DONE_CB(usb, UDI_USB_ACM_MODEM);
READ_DONE_CB(usb, UDI_USB_ACM_GPS);
READ_DONE_CB(usb, UDI_USB_ACM_3G_GPS);
READ_DONE_CB(usb, UDI_USB_ACM_3G_PCVOICE);
READ_DONE_CB(usb, UDI_USB_ACM_PCVOICE);
READ_DONE_CB(usb, UDI_USB_ACM_SKYTONE);
READ_DONE_CB(usb, UDI_USB_ACM_CDMA_LOG);

READ_DONE_CB(pcie, UDI_USB_ACM_CTRL);
READ_DONE_CB(pcie, UDI_USB_ACM_AT);
READ_DONE_CB(pcie, UDI_USB_ACM_SHELL);
READ_DONE_CB(pcie, UDI_USB_ACM_LTE_DIAG);
READ_DONE_CB(pcie, UDI_USB_ACM_OM);
READ_DONE_CB(pcie, UDI_USB_ACM_MODEM);
READ_DONE_CB(pcie, UDI_USB_ACM_GPS);
READ_DONE_CB(pcie, UDI_USB_ACM_3G_GPS);
READ_DONE_CB(pcie, UDI_USB_ACM_3G_PCVOICE);
READ_DONE_CB(pcie, UDI_USB_ACM_PCVOICE);
READ_DONE_CB(pcie, UDI_USB_ACM_SKYTONE);
READ_DONE_CB(pcie, UDI_USB_ACM_CDMA_LOG);

void usb_write_done_cb(u32 dev_id, char *vir_addr, char *phy_addr, int size);

#define U_WRITE_DONE_CB(_name)                                                      \
    static void usb_write_done_cb_##_name(char *vir_addr, char *phy_addr, int size) \
    {                                                                               \
        usb_write_done_cb(_name, vir_addr, phy_addr, size);                         \
    }

#define U_WRITE_DONE_CB_NAME(_name) (usb_write_done_cb_##_name)

U_WRITE_DONE_CB(UDI_USB_ACM_CTRL);
U_WRITE_DONE_CB(UDI_USB_ACM_AT);
U_WRITE_DONE_CB(UDI_USB_ACM_SHELL);
U_WRITE_DONE_CB(UDI_USB_ACM_LTE_DIAG);
U_WRITE_DONE_CB(UDI_USB_ACM_OM);
U_WRITE_DONE_CB(UDI_USB_ACM_MODEM);
U_WRITE_DONE_CB(UDI_USB_ACM_GPS);
U_WRITE_DONE_CB(UDI_USB_ACM_3G_GPS);
U_WRITE_DONE_CB(UDI_USB_ACM_3G_PCVOICE);
U_WRITE_DONE_CB(UDI_USB_ACM_PCVOICE);
U_WRITE_DONE_CB(UDI_USB_ACM_SKYTONE);
U_WRITE_DONE_CB(UDI_USB_ACM_CDMA_LOG);

void pcie_write_done_cb(u32 dev_id, char *pVirAddr, char *pPhyAddr, int size);

#define P_WRITE_DONE_CB(_name)                                                       \
    static void pcie_write_done_cb_##_name(char *pVirAddr, char *pPhyAddr, int size) \
    {                                                                                \
        pcie_write_done_cb(_name, pVirAddr, pPhyAddr, size);                         \
    }

#define P_WRITE_DONE_CB_NAME(_name) (pcie_write_done_cb_##_name)

P_WRITE_DONE_CB(UDI_USB_ACM_CTRL);
P_WRITE_DONE_CB(UDI_USB_ACM_AT);
P_WRITE_DONE_CB(UDI_USB_ACM_SHELL);
P_WRITE_DONE_CB(UDI_USB_ACM_LTE_DIAG);
P_WRITE_DONE_CB(UDI_USB_ACM_OM);
P_WRITE_DONE_CB(UDI_USB_ACM_MODEM);
P_WRITE_DONE_CB(UDI_USB_ACM_GPS);
P_WRITE_DONE_CB(UDI_USB_ACM_3G_GPS);
P_WRITE_DONE_CB(UDI_USB_ACM_3G_PCVOICE);
P_WRITE_DONE_CB(UDI_USB_ACM_PCVOICE);
P_WRITE_DONE_CB(UDI_USB_ACM_SKYTONE);
P_WRITE_DONE_CB(UDI_USB_ACM_CDMA_LOG);

void usb_evt_cb(u32 dev_id, ACM_EVT_E port_status);
void *g_pcie_handler = NULL;

#define EVT_CB(dev, _name)                                  \
    static void dev##_evt_cb_##_name(ACM_EVT_E port_status) \
    {                                                       \
        dev##_evt_cb(_name, port_status);                   \
    }

#define EVT_CB_NAME(dev, _name) (dev##_evt_cb_##_name)

EVT_CB(usb, UDI_USB_ACM_CTRL);
EVT_CB(usb, UDI_USB_ACM_AT);
EVT_CB(usb, UDI_USB_ACM_SHELL);
EVT_CB(usb, UDI_USB_ACM_LTE_DIAG);
EVT_CB(usb, UDI_USB_ACM_OM);
EVT_CB(usb, UDI_USB_ACM_MODEM);
EVT_CB(usb, UDI_USB_ACM_GPS);
EVT_CB(usb, UDI_USB_ACM_3G_GPS);
EVT_CB(usb, UDI_USB_ACM_3G_PCVOICE);
EVT_CB(usb, UDI_USB_ACM_PCVOICE);
EVT_CB(usb, UDI_USB_ACM_SKYTONE);
EVT_CB(usb, UDI_USB_ACM_CDMA_LOG);

void umodem_msc_read_cb(MODEM_MSC_STRU *u_modem_msc);
void umodem_rel_ind_cb(unsigned int b_enable);
void pmodem_msc_write_cb(pcdev_mdm_msc_stru *p_modem_msc);
void umodem_free_cb(char *buf);

#define PCIE_USB_MAX_CDEV (UDI_USB_ACM_CDMA_LOG + 1)

struct pur_adp_device g_pur_adp_devices[PCIE_USB_MAX_CDEV] = {
    [UDI_USB_ACM_CTRL] = {
        .name = "USB_ACM_CTRL",
        .u_rd_buff_info.u32BuffNum = BUF_NUM_128,
        .u_rd_buff_info.u32BuffSize = CTRL_EP_BUFSIZE,
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_CTRL),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_CTRL),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_CTRL),

        .p_rd_buff_info.buf_size = CTRL_RC_BUFSIZE,
        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_CTRL),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_CTRL),
    },

    [UDI_USB_ACM_AT] = {
        .name = "USB_ACM_AT",
        .u_rd_buff_info.u32BuffNum = BUF_NUM_128,
        .u_rd_buff_info.u32BuffSize = PCUI_EP_BUFSIZE,
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_AT),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_AT),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_AT),

        .p_rd_buff_info.buf_size = PCUI_RC_BUFSIZE,
        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_AT),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_AT),
    },

    [UDI_USB_ACM_SHELL] = {
        .name = "USB_ACM_SHELL",
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_SHELL),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_SHELL),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_SHELL),

        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_SHELL),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_SHELL),
    },

    [UDI_USB_ACM_OM] = {
        .name = "ACM_OM",
        .u_rd_buff_info.u32BuffNum = BUF_NUM_128,
        .u_rd_buff_info.u32BuffSize = G3DIAG_EP_BUFSIZE,
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_OM),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_OM),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_OM),

        .p_rd_buff_info.buf_size = G3DIAG_RC_BUFSIZE,
        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_OM),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_OM),
    },

    [UDI_USB_ACM_MODEM] = {
        .name = "ACM_MODEM",
        .u_rd_buff_info.u32BuffNum = BUF_NUM_16,
        .u_rd_buff_info.u32BuffSize = MODEM_EP_BUFSIZE,
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_MODEM),
        .u_write_done_cb = NULL,
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_MODEM),
        .u_msc_read_cb = umodem_msc_read_cb,
        .u_rel_ind_cb = umodem_rel_ind_cb,
        .u_free_cb = umodem_free_cb,

        .p_rd_buff_info.buf_size = MODEM_RC_BUFSIZE,
        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_MODEM),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_MODEM),
        .p_msc_write_cb = pmodem_msc_write_cb,
    },

    [UDI_USB_ACM_3G_GPS] = {
        .name = "ACM_3G_GPS",
        .u_rd_buff_info.u32BuffNum = BUF_NUM_512,
        .u_rd_buff_info.u32BuffSize = ASHELL_EP_BUFSIZE,
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_3G_GPS),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_3G_GPS),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_3G_GPS),

        .p_rd_buff_info.buf_size = ASHELL_RC_BUFSIZE,
        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_3G_GPS),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_3G_GPS),
    },

    [UDI_USB_ACM_SKYTONE] = {
        .name = "ACM_SKYTONE",
        .u_rd_buff_info.u32BuffNum = BUF_NUM_128,
        .u_rd_buff_info.u32BuffSize = SKYTONE_EP_BUFSIZE,
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_SKYTONE),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_SKYTONE),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_SKYTONE),

        .p_rd_buff_info.buf_size = SKYTONE_RC_BUFSIZE,
        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_SKYTONE),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_SKYTONE),
    },

    [UDI_USB_ACM_CDMA_LOG] = {
        .name = "USB_ACM_CDMA_LOG",
        .u_read_done_cb = READ_DONE_CB_NAME(usb, UDI_USB_ACM_CDMA_LOG),
        .u_write_done_cb = U_WRITE_DONE_CB_NAME(UDI_USB_ACM_CDMA_LOG),
        .u_evt_cb = EVT_CB_NAME(usb, UDI_USB_ACM_CDMA_LOG),

        .p_read_done_cb = READ_DONE_CB_NAME(pcie, UDI_USB_ACM_CDMA_LOG),
        .p_write_done_cb = P_WRITE_DONE_CB_NAME(UDI_USB_ACM_CDMA_LOG),
    },
};

int pur_usb_acm_set_ioctl(struct pur_adp_device *pur_dev, void *usb_handler);

int pur_usb_acm_open(unsigned int dev_id)
{
    int ret;
    void *usb_handler = NULL;
    struct pur_adp_device *pur_dev = NULL;

    pur_dev = &g_pur_adp_devices[dev_id];
    /* check is already open */
    if (pur_dev->usb_handler != NULL) {
        pur_dev->usb.opened++;
        PUR_ERR("usb acm handler already open\n");
        return -EBUSY;
    };

    pur_dev->usb_handler = bsp_acm_open(dev_id);
    if (IS_ERR_OR_NULL(pur_dev->usb_handler)) {
        pur_dev->usb.open_fail++;
        PUR_ERR("usb acm open failed\n");
        pur_dev->usb_handler = NULL;
        return -ENOENT;
    }
    pur_dev->usb.open++;
    usb_handler = pur_dev->usb_handler;

    pur_dev->dev_id = dev_id;

    ret = pur_usb_acm_set_ioctl(pur_dev, usb_handler);
    if (ret) {
        goto error;
    }
    PUR_ERR("usb acm open succ, dev_id: %d, dev_name:%s\n", dev_id, pur_dev->name);

    return ret;

error:
    if (usb_handler != NULL) {
        (void)bsp_acm_close(usb_handler);
        pur_dev->usb_handler = NULL;
        pur_dev->usb.close++;
    }
    return ret;
}

static int read_write_cb_set(struct pur_adp_device *pur_dev, void *usb_handler)
{
    int ret;
    /* set usb read cb */
    ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_SET_READ_CB, pur_dev->u_read_done_cb);
    if (ret) {
        PUR_ERR("usb acm set read cb failed\n");
        return ret;
    }

    /* set usb wrtie do not copy */
    ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_WRITE_DO_COPY, NULL);
    if (ret) {
        PUR_ERR("usb acm set wrtie do copy failed\n");
        return ret;
    }

    /* set write cb */
    ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_SET_WRITE_CB, pur_dev->u_write_done_cb);
    if (ret) {
        PUR_ERR("usb set write cb failed\n");
        return ret;
    }
    return 0;
}

int pur_usb_acm_set_ioctl(struct pur_adp_device *pur_dev, void *usb_handler)
{
    int ret = 0;
    ACM_READ_BUFF_INFO u_buf_info = {0};

    /* set usb read buff */
    if (pur_dev->u_rd_buff_info.u32BuffNum && pur_dev->u_rd_buff_info.u32BuffSize) {
        u_buf_info.u32BuffNum = pur_dev->u_rd_buff_info.u32BuffNum;
        u_buf_info.u32BuffSize = pur_dev->u_rd_buff_info.u32BuffSize;

        ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_RELLOC_READ_BUFF, (void *)&u_buf_info);
        if (ret) {
            PUR_ERR("usb acm realloc buf failed\n");
            return ret;
        }
    }

    ret = read_write_cb_set(pur_dev, usb_handler);
    if (ret) {
        return ret;
    }

    /* set event cb */
    ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_SET_EVT_CB, pur_dev->u_evt_cb);
    if (ret) {
        PUR_ERR("usb set event cb failed\n");
        return ret;
    }

    ret = bsp_acm_ioctl(usb_handler, ACM_MODEM_IOCTL_SET_MSC_READ_CB, pur_dev->u_msc_read_cb);
    if (ret) {
        PUR_ERR("usb set msc read cb failed\n");
        return ret;
    }

    ret = bsp_acm_ioctl(usb_handler, ACM_MODEM_IOCTL_SET_REL_IND_CB, pur_dev->u_rel_ind_cb);
    if (ret) {
        PUR_ERR("usb set rel ind cb failed\n");
        return ret;
    }

    if (pur_dev->dev_id == UDI_USB_ACM_MODEM) {
        ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_SET_FREE_CB, pur_dev->u_free_cb);
        if (ret) {
            PUR_ERR("usb set free cb failed\n");
            return ret;
        }
    }

    return 0;
}

int pur_usb_acm_close(u32 dev_id)
{
    int ret = -EBUSY;
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];

    if (NULL == pur_dev->usb_handler) {
        PUR_ERR("usb acm handler not open\n");
        return ret;
    }

    ret = bsp_acm_close(pur_dev->usb_handler);
    pur_dev->usb_handler = NULL;

    PUR_ERR("usb acm close succ, dev_id: %d, dev_name:%s\n", dev_id, pur_dev->name);
    pur_dev->usb.close++;
    return ret;
}

/* **************** usb *************** */
void usb_read_done_cb(u32 dev_id)
{
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];
    void *usb_handler = pur_dev->usb_handler;
    void *pcie_handler = pur_dev->pcie_handler;
    ACM_WR_ASYNC_INFO u_rw_info = {0};
    pcdev_wr_async_info_s p_rw_info = {0};
    int ret;

    PUR_TRACE("dev_id:%d\n", dev_id);

    if (unlikely(usb_handler == NULL)) {
        PUR_ERR("[%d]usb handler not ready\n", dev_id);
        return;
    }

    pur_dev->usb.read++;
    ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_GET_RD_BUFF, &u_rw_info);
    if (unlikely(ret)) {
        pur_dev->usb.get_rd_buf_fail++;
        PUR_ERR("dev_id:%d, get read buf failed\n", dev_id);
        return;
    }

    PUR_TRACE("len:%d\n", u_rw_info.u32Size);
    pur_dev->usb.read_len += u_rw_info.u32Size;

    p_rw_info.p_vaddr = u_rw_info.pVirAddr;
    p_rw_info.p_paddr = NULL;
    p_rw_info.size = u_rw_info.u32Size;

    if (likely(pcie_handler != NULL)) {
        pur_dev->pcie.write++;
        ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_WRITE_ASYNC, &p_rw_info);
    } else {
        ret = -EBUSY;
        pur_dev->pcie.not_ready++;
        PUR_TRACE("[%d]usb handler not ready\n", dev_id);
    }

    if (unlikely(ret)) {
        PUR_TRACE("dev_id:%d, return buffer directly\n", dev_id);
        ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_RETURN_BUFF, &u_rw_info);
        if (ret) {
            pur_dev->usb.ret_dirct_fail++;
        } else {
            pur_dev->usb.ret_dirct++;
        }
        pur_dev->usb.ret_dirct_len += u_rw_info.u32Size;
    } else {
        pur_dev->pcie.write_len += p_rw_info.size;
    }
}

/* call pcie ioctl return buff */
void usb_write_done_cb(u32 dev_id, char *vir_addr, char *phy_addr, int size)
{
    int ret;
    pcdev_wr_async_info_s p_rw_info = {0};
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];
    void *pcie_handler = pur_dev->pcie_handler;

    p_rw_info.p_vaddr = vir_addr;
    p_rw_info.p_paddr = NULL;
    p_rw_info.size = (unsigned int)size;
    p_rw_info.p_priv = NULL;

    pur_dev->usb.write_done++;
    pur_dev->usb.write_done_len += (unsigned int)size;

    PUR_TRACE("dev_id:%d,len:%d\n", dev_id, size);

    ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_RETURN_BUFF, &p_rw_info);
    if (unlikely(ret)) {
        PUR_ERR("dev_id:%d, return read buffer failed ret:%d\n", dev_id, ret);
        pur_dev->pcie.ret_buf_fail++;
    } else {
        pur_dev->pcie.ret_buf++;
    }
}

void usb_evt_cb(u32 dev_id, ACM_EVT_E port_status)
{
    int ret;
    void *pcie_handler = NULL;
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];

    PUR_TRACE("in device:%d, port_status:%d\n", dev_id, port_status);

    pur_dev->port_status = port_status;
    pcie_handler = pur_dev->pcie_handler;

    if (pcie_handler == NULL) {
        PUR_ERR("device(%d) pcie handler is null\n", dev_id);
        return;
    }

    if (port_status == ACM_EVT_DEV_SUSPEND) {
        pur_dev->usb_ready = 0;
        ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_SEND_EVT, &port_status);
        if (ret) {
            PUR_ERR("send disconnect event status to pcie fail\n");
            return;
        }
        PUR_INFO("Receive USB disconnect...\n");
    } else if (port_status == ACM_EVT_DEV_READY) {
        pur_dev->usb_ready = 1;
        ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_SEND_EVT, &port_status);
        if (ret) {
            PUR_ERR("send connect event status to pcie fail, ret:%d\n", ret);
            return;
        }
        PUR_INFO("Receive USB connect...\n");
    } else {
        PUR_ERR("Receive USB status is unknow\n");
    }
    PUR_TRACE("out device:%d\n", dev_id);
}

void umodem_msc_read_cb(MODEM_MSC_STRU *u_modem_msc)
{
    int ret;
    void *pcie_handler = NULL;
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[UDI_USB_ACM_MODEM];

    pur_dev->u_modem_msc = (pcdev_mdm_msc_stru *)u_modem_msc;
    pcie_handler = pur_dev->pcie_handler;

    if (pcie_handler == NULL) {
        PUR_ERR("device(%d) pcie handler is null\n", UDI_USB_ACM_MODEM);
        return;
    }

    ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_SEND_MSC_READ, u_modem_msc);
    if (ret) {
        PUR_ERR("send msc read status to pcie fail, ret:%d\n", ret);
        return;
    }
    PUR_INFO("Receive USB msc read status...\n");
}

void umodem_rel_ind_cb(unsigned int b_enable)
{
    int ret;
    void *pcie_handler = NULL;
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[UDI_USB_ACM_MODEM];

    PUR_TRACE("in b_enable:%d\n", b_enable);

    pur_dev->b_enable = b_enable;
    pcie_handler = pur_dev->pcie_handler;

    if (pcie_handler == NULL) {
        PUR_ERR("device(%d) pcie handler is null\n", UDI_USB_ACM_MODEM);
        return;
    }

    ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_SEND_REL_IND, &b_enable);
    if (ret) {
        PUR_ERR("send rel ind status to pcie fail, ret:%d\n", ret);
        return;
    }
    PUR_INFO("Receive USB rel ind status...\n");
}

void umodem_free_cb(char *buf)
{
    struct sk_buff *free_skb = (struct sk_buff *)buf;

    dev_kfree_skb_any(free_skb);
    g_pur_adp_devices[UDI_USB_ACM_MODEM].usb.free_cb++;
    return;
}

/* **************** pcie ******************* */
void pmodem_msc_write_cb(pcdev_mdm_msc_stru *p_modem_msc)
{
    int ret;
    void *usb_handler = NULL;
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[UDI_USB_ACM_MODEM];

    pur_dev->p_modem_msc = p_modem_msc;
    usb_handler = pur_dev->usb_handler;

    if (usb_handler == NULL) {
        PUR_ERR("device(%d) usb handler is null\n", UDI_USB_ACM_MODEM);
        return;
    }

    ret = bsp_acm_ioctl(usb_handler, ACM_MODEM_IOCTL_MSC_WRITE_CMD, p_modem_msc);
    if (ret) {
        PUR_ERR("send msc read status to pcie fail\n");
        return;
    }

    PUR_INFO("Receive PCDEV msc read status...\n");
}

static int pur_dev_check(struct pur_adp_device *pur_dev, unsigned int dev_id)
{
    /* check is already open */
    if (pur_dev->pcie_handler != NULL) {
        pur_dev->pcie.opened++;
        PUR_ERR("pcie acm handler already open\n");
        return -EBUSY;
    }

    pur_dev->pcie_handler = bsp_pcdev_open(dev_id);
    if (IS_ERR_OR_NULL(pur_dev->pcie_handler)) {
        pur_dev->pcie.open_fail++;
        PUR_ERR("pcie acm open failed\n");
        pur_dev->pcie_handler = NULL;
        return -ENOENT;
    }
    pur_dev->pcie.open++;
    pur_dev->dev_id = dev_id;
    return 0;
}

int pur_pcie_acm_open(unsigned int dev_id)
{
    int ret;
    void *pcie_handler = NULL;
    pcdev_read_buf_info_s p_buf_info = {0};
    struct pur_adp_device *pur_dev = NULL;
    ACM_EVT_E port_status = ACM_EVT_DEV_READY;

    pur_dev = &g_pur_adp_devices[dev_id];
    ret = pur_dev_check(pur_dev, dev_id);
    if (ret) {
        return ret;
    }

    pcie_handler = pur_dev->pcie_handler;

    /* set pcie read buff */
    if (pur_dev->p_rd_buff_info.buf_size) {
        p_buf_info.buf_size = pur_dev->p_rd_buff_info.buf_size;

        ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_RELLOC_READ_BUFF, (void *)&p_buf_info);
        if (ret) {
            PUR_ERR("pcie acm realloc buf failed\n");
            goto error;
        }
    }

    /* set pcie read cb */
    ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_SET_READ_CB, pur_dev->p_read_done_cb);
    if (ret) {
        PUR_ERR("pcie acm set read cb failed\n");
        goto error;
    }

    /* set write cb */
    ret = bsp_pcdev_ioctl(pcie_handler, ACM_IOCTL_SET_WRITE_CB, pur_dev->p_write_done_cb);
    if (ret) {
        PUR_ERR("set write cb failed\n");
        goto error;
    }

    ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_MODEM_IOCTL_SET_MSC_WRITE_CB, pur_dev->p_msc_write_cb);
    if (ret) {
        PUR_ERR("set msc write cb failed\n");
        goto error;
    }

    /* pcie virtual port is ready */
    if (pur_dev->usb_ready) {
        ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_SEND_EVT, &port_status);
    }

    PUR_ERR("pcie acm open succ, dev_id: %d, dev_name:%s\n", dev_id, pur_dev->name);
    return 0;

error:
    if (pcie_handler != NULL) {
        (void)bsp_pcdev_close(pcie_handler);
        pur_dev->pcie_handler = NULL;
    }
    pur_dev->pcie.close++;
    return ret;
}

int pur_pcie_acm_close(u32 dev_id)
{
    int ret = -EBUSY;
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];

    if (NULL == pur_dev->pcie_handler) {
        PUR_ERR("pcie acm handler not open\n");
        return ret;
    }

    /* no need send ACM_EVT_DEV_SUSPEND event */
    ret = bsp_pcdev_close(pur_dev->pcie_handler);
    pur_dev->pcie_handler = NULL;

    PUR_ERR("pcie acm close succ, dev_id: %d, dev_name:%s\n", dev_id, pur_dev->name);
    pur_dev->pcie.close++;
    return ret;
}

void pcie_read_done_cb(u32 dev_id)
{
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];
    void *usb_handler = pur_dev->usb_handler;
    void *pcie_handler = pur_dev->pcie_handler;
    ACM_WR_ASYNC_INFO u_rw_info = {0};
    pcdev_wr_async_info_s p_rw_info = {0};
    int ret;

    PUR_TRACE("dev_id:%d\n", dev_id);
    if (unlikely(pcie_handler == NULL)) {
        PUR_ERR("[%d]pcie handler not ready\n", dev_id);
        return;
    }
    pur_dev->pcie.read++;
    ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_GET_RD_BUFF, &p_rw_info);
    if (unlikely(ret)) {
        pur_dev->pcie.get_rd_buf_fail++;
        PUR_ERR("dev_id:%d, get read buf failed\n", dev_id);
        return;
    }
    pur_dev->pcie.read_len += p_rw_info.size;
    u_rw_info.pVirAddr = p_rw_info.p_vaddr;
    u_rw_info.pPhyAddr = NULL;
    u_rw_info.u32Size = p_rw_info.size;

    if (likely(usb_handler != NULL)) {
        pur_dev->usb.write++;
        ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_WRITE_ASYNC, &u_rw_info);
    } else {
        ret = -EBUSY;
        pur_dev->usb.not_ready++;
        PUR_TRACE("[%d]usb handler not ready\n", dev_id);
    }

    if (unlikely(ret)) {
        ret = bsp_pcdev_ioctl(pcie_handler, PCDEV_IOCTL_RETURN_BUFF, &p_rw_info);
        if (ret) {
            pur_dev->pcie.ret_dirct_fail++;
        } else {
            pur_dev->pcie.ret_dirct++;
        }
        pur_dev->pcie.ret_dirct_len += p_rw_info.size;
        if (g_pur_adp_devices[dev_id].u_free_cb) {
            g_pur_adp_devices[dev_id].u_free_cb(p_rw_info.p_vaddr);
        }
    } else {
        pur_dev->usb.write_len += u_rw_info.u32Size;
    }
}

/* call usb ioctl return buffer */
void pcie_write_done_cb(u32 dev_id, char *pVirAddr, char *pPhyAddr, int size)
{
    int ret;
    ACM_WR_ASYNC_INFO u_rw_info = {0};
    struct pur_adp_device *pur_dev = &g_pur_adp_devices[dev_id];
    void *usb_handler = pur_dev->usb_handler;

    u_rw_info.pVirAddr = pVirAddr;
    u_rw_info.pPhyAddr = NULL;
    u_rw_info.u32Size = (unsigned int)size;
    u_rw_info.pDrvPriv = NULL;

    pur_dev->pcie.write_done++;
    pur_dev->pcie.write_done_len += (unsigned int)size;

    PUR_TRACE("dev_id:%d, len:%d\n", dev_id, size);

    ret = bsp_acm_ioctl(usb_handler, ACM_IOCTL_RETURN_BUFF, &u_rw_info);
    if (unlikely(ret)) {
        PUR_ERR("dev_id:%d, return read buffer failed ret=%d\n", dev_id, ret);
        pur_dev->usb.ret_buf_fail++;
    } else {
        pur_dev->usb.ret_buf++;
    }
}

static unsigned int g_pur_acm_port[] = {
    UDI_USB_ACM_CTRL,           /* 0 */
    UDI_USB_ACM_AT,             /* 1 */
    UDI_USB_ACM_SHELL,          /* 2 */
                                //    UDI_USB_ACM_LTE_DIAG,     /* 3 */
    UDI_USB_ACM_OM,             /* 4 */
    UDI_USB_ACM_MODEM,          /* 5 */
                                //    UDI_USB_ACM_GPS,          /* 6 */  /*HISO*/
    UDI_USB_ACM_3G_GPS,        /* 7 */ /* ashell,cbt */
    UDI_USB_ACM_SKYTONE,       /* a */
    UDI_USB_ACM_CDMA_LOG,      /* b */
};

#define PUR_ACM_PORT_USED_CNT (sizeof(g_pur_acm_port) / sizeof(g_pur_acm_port[0]))

void pur_usb_relay_init(void)
{
    unsigned int ret;
    unsigned int dev_id;
    unsigned int i;

    for (i = 0; i < PUR_ACM_PORT_USED_CNT; i++) {
        dev_id = g_pur_acm_port[i];
        PUR_TRACE("open device(%d) start\n", dev_id);
        ret = pur_usb_acm_open(dev_id);
        if (ret) {
            PUR_ERR("dev_id:%d, usb acm open err\n", dev_id);
            continue;
        }
        PUR_TRACE("open device(%d) usb acm succ\n", dev_id);
    }
}

void pur_usb_relay_exit(void)
{
    unsigned int ret;
    unsigned int dev_id;
    unsigned int i;

    for (i = 0; i < PUR_ACM_PORT_USED_CNT; i++) {
        dev_id = g_pur_acm_port[i];
        PUR_TRACE("close device(%d) start\n", dev_id);
        ret = pur_usb_acm_close(dev_id);
        if (ret) {
            PUR_ERR("dev_id:%d, usb acm close err\n", dev_id);
            continue;
        }
        PUR_TRACE("close device(%d) usb acm succ\n", dev_id);
    }
}

void pur_pcie_relay_init(void)
{
    unsigned int ret;
    unsigned int dev_id;
    unsigned int i = 0;

    for (i = 0; i < PUR_ACM_PORT_USED_CNT; i++) {
        dev_id = g_pur_acm_port[i];
        PUR_TRACE("open device(%d) start\n", dev_id);
        ret = pur_pcie_acm_open(dev_id);
        PUR_TRACE("open device(%d) return(%d)\n", dev_id, ret);
        if (ret) {
            PUR_ERR("dev_id:%d, pcie acm open err\n", dev_id);
            continue;
        }
        PUR_TRACE("open device(%d) pcie acm succ\n", dev_id);
    }
}

void pur_pcie_relay_exit(void)
{
    unsigned int ret;
    unsigned int dev_id;
    unsigned int i;

    for (i = 0; i < PUR_ACM_PORT_USED_CNT; i++) {
        dev_id = g_pur_acm_port[i];
        PUR_TRACE("close device(%d) start\n", dev_id);
        ret = pur_pcie_acm_close(dev_id);
        if (ret) {
            PUR_ERR("dev_id:%d, pcie acm close err\n", dev_id);
            continue;
        }
        PUR_TRACE("close device(%d) pcie acm succ\n", dev_id);
    }
}

int pur_init(void)
{
    int ret;
    PUR_ERR("[init] start\n");
    ret = BSP_USB_RegUdiEnableCB(pur_usb_relay_init);
    if (unlikely(ret)) {
        PUR_ERR("usb acm port init cb set fail\n");
        return ret;
    }
    ret = BSP_USB_RegUdiDisableCB(pur_usb_relay_exit);
    if (unlikely(ret)) {
        PUR_ERR("usb acm port close cb set fail\n");
        return ret;
    }

    ret = bsp_pcdev_reg_enablecb(pur_pcie_relay_init);
    if (unlikely(ret)) {
        PUR_ERR("pcie enablecb set fail\n");
        return ret;
    }

    ret = bsp_pcdev_reg_disablecb(pur_pcie_relay_exit);
    if (unlikely(ret)) {
        PUR_ERR("pcie disablecb cb set fail\n");
        return ret;
    }

    PUR_ERR("[init] ok\n");
    return ret;
}

void pur_set_msg_level(unsigned int level)
{
    g_pur_msg_level = level;
}

void pur_set_print_port(unsigned int dev_id, unsigned int en)
{
    struct pur_adp_device *pur_dev = NULL;
    if (dev_id >= PCIE_USB_MAX_CDEV) {
        return;
    }
    pur_dev = &g_pur_adp_devices[dev_id];
    pur_dev->print_pkt_en = !!en;
}

static void pur_port_stat(struct pur_stat_s *dev);

void pur_port_info(unsigned int dev_id)
{
    struct pur_adp_device *pur_dev = NULL;

    if (dev_id >= PCIE_USB_MAX_CDEV) {
        return;
    }
    pur_dev = &g_pur_adp_devices[dev_id];

    pr_err("num                : %d\n", dev_id);
    pr_err("name               : %s\n", pur_dev->name);
    pr_err("port_status        : %d\n", pur_dev->port_status);
    pr_err("usb  u32BuffNum    : %d\n", pur_dev->u_rd_buff_info.u32BuffNum);
    pr_err("usb  u32BuffSize   : 0x%x\n", pur_dev->u_rd_buff_info.u32BuffSize);
    pr_err("pcie u32BuffSize   : 0x%x\n", pur_dev->p_rd_buff_info.buf_size);

    pr_err("u_read_done_cb     : %s\n", pur_dev->u_read_done_cb ? "good" : "null");
    pr_err("u_write_done_cb    : %s\n", pur_dev->u_write_done_cb ? "good" : "null");
    pr_err("u_evt_cb           : %s\n", pur_dev->u_evt_cb ? "good" : "null");
    pr_err("u_msc_read_cb      : %s\n", pur_dev->u_msc_read_cb ? "good" : "null");
    pr_err("u_rel_ind_cb       : %s\n", pur_dev->u_rel_ind_cb ? "good" : "null");
    pr_err("p_read_done_cb     : %s\n", pur_dev->p_read_done_cb ? "good" : "null");
    pr_err("p_write_done_cb    : %s\n", pur_dev->p_write_done_cb ? "good" : "null");
    pr_err("p_msc_write_cb     : %s\n", pur_dev->p_msc_write_cb ? "good" : "null");

    pr_err("USB\n");
    pur_port_stat(&pur_dev->usb);
    pr_err("PCIE\n");
    pur_port_stat(&pur_dev->pcie);
}

static void pur_port_stat(struct pur_stat_s *dev)
{
    pr_err("  open             : 0x%x\n", dev->open);
    pr_err("  opened           : 0x%x\n", dev->opened);
    pr_err("  open_fail        : 0x%x\n", dev->open_fail);
    pr_err("  close            : 0x%x\n", dev->close);
    pr_err("  get_rd_buf_fail  : 0x%x\n", dev->get_rd_buf_fail);
    pr_err("  ret_dirct        : 0x%x\n", dev->ret_dirct);
    pr_err("  ret_dirct_len    : 0x%x\n", dev->ret_dirct_len);
    pr_err("  ret_dirct_fail   : 0x%x\n", dev->ret_dirct_fail);
    pr_err("  ret_buf          : 0x%x\n", dev->ret_buf);
    pr_err("  ret_buf_fail     : 0x%x\n", dev->ret_buf_fail);
    pr_err("  read             : 0x%x\n", dev->read);
    pr_err("  read_len         : 0x%x\n", dev->read_len);
    pr_err("  write            : 0x%x\n", dev->write);
    pr_err("  write_len        : 0x%x\n", dev->write_len);
    pr_err("  write_done       : 0x%x\n", dev->write_done);
    pr_err("  write_done_len   : 0x%x\n", dev->write_done_len);
    pr_err("  not_ready        : 0x%x\n", dev->not_ready);
    pr_err("  free_cb          : 0x%x\n", dev->free_cb);
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(pur_init);
#endif
