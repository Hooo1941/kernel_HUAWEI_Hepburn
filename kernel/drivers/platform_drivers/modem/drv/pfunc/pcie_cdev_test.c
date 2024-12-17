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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <securec.h>
#include "pcie_cdev.h"
#include "pcie_cdev_dbg.h"
#include <bsp_pcdev.h>
#include <mdrv_timer.h>

#define PDEV_TEST_MAX_BUFSIZE 2048
#define PDEV_TEST_MAX_TXBUF_SIZE 0X100
#define PDEV_TEST_DELAY_COUNT 200
#define PDEV_TEST_DELAY_TMIE  10
#define PDEV_TEST_DATA_LEN 8
#define PDEV_PRINT_PKT_LEN 64

static char *g_st_pcdev_buf = "abcdef";
static char *g_st_pcdev_buf2 = NULL;
static void *g_st_pcdev_filep[pcdev_port_bottom] = {NULL};
static unsigned int g_st_pcdev_port_n = 0;
static struct task_struct *g_pcie_cdev_stress_task = NULL;
static unsigned int g_pcie_cdev_stress_task_running = 0;
static unsigned int g_pcie_cdev_stress_run_count = 0;

#ifdef CONFIG_PCIE_CDEV
static volatile int g_pcdev_timer_enable = 0x1;
static u32 g_pcdev_test_count = 20;
static u32 g_pcdev_wake_timer_ms = 2000;
static u32 g_pcdev_wake_timer_count = 0;
static timer_attr_s g_pcdev_timer_attr = {"pcdev_timer", 0, TIMER_WAKE, NULL, 0};
static timer_handle g_pcdev_timer_handle;
#endif

/* ************************************  ST  *************************************** */
void st_pcdev_write_cb_0(char *pVirAddr, unsigned int size, unsigned int MuxId, void *pDrvPriv)
{
    return;
}

void st_pcdev_read_cb_0(void)
{
    pcdev_wr_async_info_s rw_info;
    int ret;

    unsigned int port_n = 0;

    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_GET_RD_BUFF, (void *)&rw_info);

    if (rw_info.size > 0 && rw_info.size <= PDEV_TEST_MAX_BUFSIZE && rw_info.p_vaddr) {
        print_pkt(port_n, rw_info.p_vaddr, rw_info.size);
        ret = memset_s(rw_info.p_vaddr, PDEV_TEST_MAX_BUFSIZE, 0, rw_info.size);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
    } else {
        PCDEV_ERR("err   buf:%lx  size:%d \n", (uintptr_t)rw_info.p_vaddr, rw_info.size);
    }

    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_RETURN_BUFF, (void *)&rw_info);
    return;
}

void st_pcdev_write_cb_1(char *pVirAddr, unsigned int size, unsigned int MuxId, void *pDrvPriv)
{
    return;
}

void st_pcdev_read_cb_1(void)
{
    pcdev_wr_async_info_s rw_info;
    int ret;

    unsigned int port_n = 1;

    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_GET_RD_BUFF, (void *)&rw_info);

    if (rw_info.size > 0 && rw_info.size <= PDEV_TEST_MAX_BUFSIZE && rw_info.p_vaddr) {
        ret = memset_s(rw_info.p_vaddr, PDEV_TEST_MAX_BUFSIZE, 0, rw_info.size);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
    } else {
        printk("st_pcdev_read_cb_1  err   buf:%lx  size:%d \n", (uintptr_t)rw_info.p_vaddr, rw_info.size);
    }
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_RETURN_BUFF, (void *)&rw_info);
    return;
}

void st_pcdev_event_cb_0(pcdev_evt_e evt)
{
    PCDEV_ERR("event:%d \n", evt);
    return;
}

void st_pcdev_event_cb_1(pcdev_evt_e evt)
{
    PCDEV_ERR("event:%d \n", evt);
    return;
}

int st_pcdev_event_send(unsigned int port_n, pcdev_evt_e evt)
{
    if (!g_st_pcdev_filep[port_n]) {
        PCDEV_ERR("filep is null \n");
        return -1;
    }

    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SEND_EVT, (void *)&evt);
    return 0;
}

void st_pcdev_enum_done_cb_0(void)
{
    PCDEV_ERR("in \n");
    return;
}

void st_pcdev_enum_done_cb_1(void)
{
    PCDEV_ERR("in \n");
    return;
}

void st_pcdev_enumdone_regist(void)
{
    bsp_pcdev_reg_enablecb(st_pcdev_enum_done_cb_0);
    bsp_pcdev_reg_enablecb(st_pcdev_enum_done_cb_1);
}

void st_pcdev_open_0(void)
{
    unsigned int port_n = 0;
    void *pcdev_filep = NULL;

    pcdev_filep = bsp_pcdev_open(port_n);
    if (pcdev_filep == NULL) {
        PCDEV_ERR("st_pcdev_open port(%d) fail\n", port_n);
        return;
    }
    g_st_pcdev_filep[port_n] = pcdev_filep;
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_READ_CB, st_pcdev_read_cb_0);
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_WRITE_CB, st_pcdev_write_cb_0);
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_EVT_CB, st_pcdev_event_cb_0);

    return;
}

void st_pcdev_open_1(void)
{
    unsigned int port_n = 1;
    void *pcdev_filep = NULL;

    pcdev_filep = bsp_pcdev_open(port_n);
    if (pcdev_filep == NULL) {
        PCDEV_ERR("port(%d) fail\n", port_n);
        return;
    }
    g_st_pcdev_filep[port_n] = pcdev_filep;
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_READ_CB, st_pcdev_read_cb_1);
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_WRITE_CB, st_pcdev_write_cb_1);
    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_EVT_CB, st_pcdev_event_cb_1);

    return;
}

int st_pcdev_send(unsigned int port_n)
{
    pcdev_wr_async_info_s rw_info;

    if (!g_st_pcdev_filep[port_n]) {
        return -1;
    }

    rw_info.p_vaddr = g_st_pcdev_buf;
    rw_info.size = PDEV_TEST_DATA_LEN;
    rw_info.p_priv = NULL;
    rw_info.p_paddr = NULL;

    bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_WRITE_ASYNC, &rw_info);

    return 0;
}

int st_pcdev_send_n(unsigned int port_n, unsigned int packet_n)
{
    pcdev_wr_async_info_s rw_info;
    unsigned int i;

    if (!g_st_pcdev_filep[port_n]) {
        return -1;
    }

    rw_info.p_vaddr = g_st_pcdev_buf;
    rw_info.size = PDEV_TEST_DATA_LEN;
    rw_info.p_priv = NULL;
    rw_info.p_paddr = NULL;

    for (i = 0; i < packet_n; i++) {
        bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
    }

    return 0;
}

int st_pcdev_sync_send_n(unsigned int port_n, unsigned int packet_n)
{
    unsigned int i;

    if (!g_st_pcdev_filep[port_n]) {
        return -1;
    }

    for (i = 0; i < packet_n; i++) {
        bsp_pcdev_write(g_st_pcdev_filep[port_n], g_st_pcdev_buf, PDEV_TEST_DATA_LEN);
    }

    return 0;
}

int pcdev_stress_test_thread(void *data)
{
    pcdev_wr_async_info_s rw_info;
    unsigned int i;

    if (!g_st_pcdev_filep[0]) {
        return -1;
    }
    if (!g_st_pcdev_filep[1]) {
        return -1;
    }

    rw_info.p_vaddr = g_st_pcdev_buf;
    rw_info.size = PDEV_TEST_DATA_LEN;
    rw_info.p_priv = NULL;
    rw_info.p_paddr = NULL;

    for (i = 0;; i++) {
        if (!g_pcie_cdev_stress_task_running) {
            break;
        }
        bsp_pcdev_ioctl(g_st_pcdev_filep[0], PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
        bsp_pcdev_ioctl(g_st_pcdev_filep[1], PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
        if (!(i % PDEV_TEST_DELAY_COUNT)) {
            msleep(PDEV_TEST_DELAY_TMIE);
        }
        g_pcie_cdev_stress_run_count++;
    }

    return 0;
}

int st_pcdev_stress_start(void)
{
    if (g_pcie_cdev_stress_task_running) {
        return 0;
    }

    g_pcie_cdev_stress_task = kthread_run(pcdev_stress_test_thread, NULL, "PCDEV_Test");
    if (IS_ERR(g_pcie_cdev_stress_task)) {
        g_pcie_cdev_stress_task = NULL;
        PCDEV_ERR("kthread_run %s fail\n", "PCDEV_Test");
        return -1;
    }

    g_pcie_cdev_stress_run_count = 0;
    g_pcie_cdev_stress_task_running = 1;

    return 0;
}

int st_pcdev_stress_stop(void)
{
    g_pcie_cdev_stress_task_running = 0;

    if (g_pcie_cdev_stress_task != NULL) {
        return kthread_stop(g_pcie_cdev_stress_task);
    }

    return 0;
}

void pcie_stress_test_info(void)
{
    PCDEV_ERR("stress_run_count:%d\n ", g_pcie_cdev_stress_run_count);
}

void st_pcdev_close_0(void)
{
    unsigned int port_n = 0;
    void *pcdev_filep = NULL;
    int ret;

    pcdev_filep = g_st_pcdev_filep[port_n];
    if (pcdev_filep == NULL) {
        PCDEV_ERR("port(%d) is closed\n", port_n);
        return;
    }

    g_st_pcdev_filep[port_n] = NULL;

    ret = bsp_pcdev_close(pcdev_filep);
    if (ret) {
        PCDEV_ERR("port(%d) bsp_pcdev_close fail\n", port_n);
    }

    return;
}

void st_pcdev_realloc_0(unsigned int buf_size)
{
    unsigned int port_n = 0;
    void *pcdev_filep = NULL;
    pcdev_read_buf_info_s buf_info;
    int ret;

    pcdev_filep = bsp_pcdev_open(port_n);
    if (pcdev_filep == NULL) {
        PCDEV_ERR("port(%d) fail\n", port_n);
        return;
    }

    buf_info.buf_size = buf_size;
    bsp_pcdev_ioctl(pcdev_filep, PCDEV_IOCTL_RELLOC_READ_BUFF, (void *)&buf_info);

    ret = bsp_pcdev_close(pcdev_filep);
    if (ret) {
        PCDEV_ERR("port(%d) bsp_pcdev_close fail\n", port_n);
    }

    return;
}

void st_pcdev_write_cb_test(char *pVirAddr, char *pPhyAddr, unsigned int size)
{
    PCDEV_ERR("buf:%lx  size:%d  paddr:%lx\n", (uintptr_t)pVirAddr, size, (uintptr_t)pPhyAddr);
    if (g_st_pcdev_buf2 != NULL) {
        kfree(g_st_pcdev_buf2);
        g_st_pcdev_buf2 = NULL;
    }
    return;
}

static void print_data(unsigned char *buf, int len)
{
    int i;
    int j;
    int count;

    count = len > PCDEV_MAX_PRINT_COUNT ? PCDEV_MAX_PRINT_COUNT : len;

    printk("[pcdev]:buf addr: 0x%llx  len: %d\n", (unsigned long long)(uintptr_t)buf, len);
    for (j = 0; j < count; j += PCDEV_PRINT_LEN) {
        printk(KERN_CONT "%03x: ", j);
        for (i = 0; i < PCDEV_PRINT_LEN; i++) {
            printk(KERN_CONT "%02x ", buf[j + i]);
        }
        printk(KERN_CONT "\n");
    }
    printk("\n");
}

void st_pcdev_read_cb_test(void)
{
    pcdev_wr_async_info_s rw_info;
    int ret;

    bsp_pcdev_ioctl(g_st_pcdev_filep[g_st_pcdev_port_n], PCDEV_IOCTL_GET_RD_BUFF, (void *)&rw_info);

    if (rw_info.size > 0 && rw_info.size <= PDEV_TEST_MAX_BUFSIZE && rw_info.p_vaddr) {
        print_data(rw_info.p_vaddr, rw_info.size);
        ret = memset_s(rw_info.p_vaddr, PDEV_TEST_MAX_BUFSIZE, 0, rw_info.size);
        if (ret) {
            PCDEV_ERR("memset_s failed, line: %d \n", __LINE__);
        }
    } else {
        PCDEV_ERR("err   buf:%lx  size:%d \n", (uintptr_t)rw_info.p_vaddr, rw_info.size);
    }

    bsp_pcdev_ioctl(g_st_pcdev_filep[g_st_pcdev_port_n], PCDEV_IOCTL_RETURN_BUFF, (void *)&rw_info);
    return;
}

void st_pcdev_open_test(void)
{
    void *pcdev_filep = NULL;

    pcdev_filep = bsp_pcdev_open(g_st_pcdev_port_n);
    if (pcdev_filep == NULL) {
        PCDEV_ERR("port(%d) fail\n", g_st_pcdev_port_n);
        return;
    }
    g_st_pcdev_filep[g_st_pcdev_port_n] = pcdev_filep;
    bsp_pcdev_ioctl(g_st_pcdev_filep[g_st_pcdev_port_n], PCDEV_IOCTL_SET_READ_CB, st_pcdev_read_cb_test);
    bsp_pcdev_ioctl(g_st_pcdev_filep[g_st_pcdev_port_n], PCDEV_IOCTL_SET_WRITE_CB, st_pcdev_write_cb_test);

    return;
}

void st_pcdev_close_test(void)
{
    void *pcdev_filep = NULL;
    int ret;

    pcdev_filep = g_st_pcdev_filep[g_st_pcdev_port_n];
    if (pcdev_filep == NULL) {
        PCDEV_ERR("port(%d) is closed\n", g_st_pcdev_port_n);
        return;
    }

    g_st_pcdev_filep[g_st_pcdev_port_n] = NULL;

    ret = bsp_pcdev_close(pcdev_filep);
    if (ret) {
        PCDEV_ERR("port(%d) bsp_pcdev_close fail\n", g_st_pcdev_port_n);
    }

    return;
}

void st_pcdev_open_send_test(unsigned int port_n, int num)
{
    void *pcdev_filep = NULL;
    pcdev_wr_async_info_s rw_info;
    int ret;
    char buf[PDEV_TEST_DATA_LEN] = "AT\r";

    if (port_n >= pcdev_port_bottom) {
        PCDEV_ERR("port(%d) invalid\n", port_n);
        return;
    }
    g_st_pcdev_port_n = port_n;

    pcdev_filep = g_st_pcdev_filep[port_n];
    if (pcdev_filep == NULL) {
        pcdev_filep = bsp_pcdev_open(port_n);
        if (pcdev_filep == NULL) {
            PCDEV_ERR("port(%d) fail\n", port_n);
            return;
        }
        g_st_pcdev_filep[port_n] = pcdev_filep;
        st_pcdev_event_send(port_n, PCDEV_EVT_DEV_READY);
        bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_READ_CB, st_pcdev_read_cb_test);
        bsp_pcdev_ioctl(g_st_pcdev_filep[port_n], PCDEV_IOCTL_SET_WRITE_CB, st_pcdev_write_cb_test);
    }

    while (num > 0) {
        g_st_pcdev_buf2 = kzalloc(PDEV_TEST_MAX_TXBUF_SIZE, GFP_KERNEL);
        ret = memcpy_s(g_st_pcdev_buf2, PDEV_TEST_MAX_TXBUF_SIZE, buf, sizeof(buf));
        if (ret) {
            PCDEV_ERR("memcpy_s fail\n");
        }
        print_data(g_st_pcdev_buf2, PDEV_PRINT_PKT_LEN);
        rw_info.p_vaddr = g_st_pcdev_buf2;
        rw_info.size = sizeof(buf) + 1;
        rw_info.p_priv = NULL;
        rw_info.p_paddr = NULL;

        bsp_pcdev_ioctl(pcdev_filep, PCDEV_IOCTL_WRITE_ASYNC, &rw_info);
        num--;
    }
    return;
}

#ifdef CONFIG_PCIE_CDEV
void pcdev_set_send_count(u32 num)
{
    g_pcdev_test_count = num;
}

void pcdev_set_timer_timeout(u32 wait_ms)
{
    g_pcdev_wake_timer_ms = wait_ms;
}

void pcdev_enable_timer(void)
{
    g_pcdev_timer_enable = 0x1;
}

void pcdev_disable_timer(void)
{
    g_pcdev_timer_enable = 0x0;
}

static int pcdev_wake_timeout_isr(int arg)
{
    g_pcdev_wake_timer_count++;
    st_pcdev_open_send_test(g_pcdev_test_count);
    if (g_pcdev_timer_enable) {
        PCDEV_ERR("vcom wake timer count: %d", g_pcdev_wake_timer_count);
        g_pcdev_timer_handle = mdrv_timer_add(&g_pcdev_timer_attr);
        if (g_pcdev_timer_handle < MDRV_OK) {
            PCDEV_ERR("fail to add pcdev_timer. handle=%d.\n", (int)g_pcdev_timer_handle);
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

void st_pcdev_wake_timer(void)
{
    g_pcdev_timer_attr.time = g_pcdev_wake_timer_ms * 1000; /* 乘1000转换为微秒时间 */
    g_pcdev_timer_attr.callback = pcdev_wake_timeout_isr;
    g_pcdev_timer_handle = mdrv_timer_add(&g_pcdev_timer_attr);
    if (g_pcdev_timer_handle < MDRV_OK) {
        PCDEV_ERR("fail to add pcdev_timer. handle=%d.\n", (int)g_pcdev_timer_handle);
        return;
    }

    return;
}
#endif

struct file *g_pcdev_filp = NULL;

void pcdev_open_test(unsigned int i)
{
    struct file *filp = NULL;

    filp = bsp_pcdev_open(i);
    if (filp == NULL) {
        PCDEV_ERR("open failed");
        return;
    }
    g_pcdev_filp = filp;
}

void pcdev_close_test(void)
{
    int ret;

    if (g_pcdev_filp != NULL) {
        ret = bsp_pcdev_close(g_pcdev_filp);
        if (ret) {
            PCDEV_ERR("close failed");
            return;
        }
        g_pcdev_filp = NULL;
    }
}

void pcdev_write_test(void)
{
    char buf[PDEV_TEST_DATA_LEN] = "at\r";
    int ret;

    if (g_pcdev_filp != NULL) {
        ret = bsp_pcdev_write(g_pcdev_filp, buf, sizeof(buf));
        if (ret <= 0) {
            PCDEV_ERR("write failed");
        }
    }
}

void pcdev_read_test(void)
{
    char buf[PDEV_TEST_DATA_LEN] = "0";
    int ret;

    while (1) {
        if (g_pcdev_filp != NULL) {
            ret = bsp_pcdev_read(g_pcdev_filp, buf, PDEV_TEST_DATA_LEN);
            if (ret <= 0) {
                PCDEV_ERR("read failed");
            }

            PCDEV_ERR("read buff:%s\n", buf);
        }
        msleep(100); /* sleep 100 miliseconds */
    }
}

