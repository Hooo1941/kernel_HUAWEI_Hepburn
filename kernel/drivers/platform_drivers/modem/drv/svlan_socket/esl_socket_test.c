/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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


#include "esl_socket_api.h"
#include <osl_types.h>
#include <osl_sem.h>
#include <linux/in.h>
#include "securec.h"
#include <bsp_print.h>
#include <linux/kthread.h>
#include <osl_malloc.h>

#define SERV_PORT 4506
#define LISTENQ 12
#define THIS_MODU mod_socket
#define CHSIZE 16
#define SENDBUFSIZE (1024 * 8)
#define RECVBUFSIZE (1024 * 8)
#define SENDTOBUFSIZE 200
#define SHUT_FLAG 2
#define BUF_SIZE 200

struct clientdata {
    int clientfd;
    struct sockaddr_in cli_addr;
    int cli_len;
    int recv_len;
    int send_len;
    char recv_buf[RECVBUFSIZE];
    char send_buf[SENDBUFSIZE];
    int used;
    struct task_struct *clthread;
};

struct test_socket {
    struct mutex lock;
    int sockfd;
    struct sockaddr_in serv_addr;
    int udpfd;
    struct clientdata client_sock[CHSIZE];
    char sendto_buf[BUF_SIZE];
    unsigned int sendto_len;
};

struct test_socket* g_test_sock = NULL;
unsigned int g_svlan_init_ok = 0;
char g_svlan_data[BUF_SIZE] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, \
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
};

#define __SK_NFDBITS (8 * sizeof(unsigned int))
#define __SK_FDMASK(d) (1UL << ((d) % __SK_NFDBITS))

#define SOCKET_DBG(format, arg...) do { \
    bsp_err(format, ##arg);    \
} while (0)
#define SOCK_NULL (0)

void svlan_test_init(void)
{
    int ret;

    g_test_sock = osl_malloc(sizeof(struct test_socket));
    if (g_test_sock == NULL) {
        bsp_err("osl_malloc failed\n");
        return;
    }

    ret = memset_s(g_test_sock, sizeof(*g_test_sock), 0, sizeof(struct test_socket));
    if (ret != 0) {
        bsp_err("memset_s failed\n");
        return;
    }

    g_svlan_init_ok = 1;
    return;
}

void sim_test_creat(int type)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    ret = bsp_esl_socket(AF_INET, type, 0);
    if (ret < 0) {
        bsp_err("creat err\n");
        return;
    }
    g_test_sock->sockfd = ret;
    SOCKET_DBG("creat ok, sockfd:%d\n", ret);
    return;
}

void sim_test_bind(unsigned int sim_port)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    g_test_sock->serv_addr.sin_family = AF_INET;
    g_test_sock->serv_addr.sin_port = htons(sim_port);
    g_test_sock->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bsp_esl_bind(g_test_sock->sockfd, (void *)&g_test_sock->serv_addr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        bsp_err("bind err\n");
        return;
    }
    SOCKET_DBG("bind ok\n");
    return;
}

void sim_test_listen(void)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    ret = bsp_esl_listen(g_test_sock->sockfd, LISTENQ);
    if (ret < 0) {
        bsp_err("listen err\n");
        return;
    }
    SOCKET_DBG("listen ok\n");
    return;
}

int sim_test_accept(int index)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return -1;
    }

    ret = bsp_esl_accept(g_test_sock->sockfd, (void *)&g_test_sock->client_sock[index].cli_addr,
        &g_test_sock->client_sock[index].cli_len);
    if (ret < 0) {
        bsp_err("accept err\n");
        return -1;
    }
    g_test_sock->client_sock[index].clientfd = ret;
    SOCKET_DBG("accept ok,client sockfd:%d\n", ret);
    return 0;
}

void sim_test_recv(int index)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    ret = bsp_esl_recv(g_test_sock->client_sock[index].clientfd, g_test_sock->client_sock[index].recv_buf,
        sizeof(g_test_sock->client_sock[index].recv_buf), 0);
    if (ret < 0) {
        bsp_err("recv err!\n");
        return;
    }

    g_test_sock->client_sock[index].recv_len = ret;
    SOCKET_DBG("recv %d bytes\n", ret);

    return;
}

void sim_test_send(int index, int send_mode)
{
    int ret;
    int clientfd = g_test_sock->client_sock[index].clientfd;
    char *sendbuf = g_test_sock->client_sock[index].send_buf;
    char *recvbuf = g_test_sock->client_sock[index].recv_buf;
    int recv_len = g_test_sock->client_sock[index].recv_len;
    int send_len;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    if (send_mode) {
        send_len = strlen("hello,this is sim_sock_serv");
        strcpy_s(sendbuf, send_len, "hello,this is sim_sock_serv");  //lint !e421
    } else {
        send_len = recv_len;
        ret = memcpy_s((void *)sendbuf, sizeof(char) * SENDBUFSIZE, (void *)recvbuf, recv_len);
        if (ret) {
            bsp_err("memcpy_s failed\n");
        }
    }
    ret = bsp_esl_send(clientfd, sendbuf, send_len, 0);
    if (ret < 0) {
        bsp_err("send err\n");
        return;
    }

    SOCKET_DBG("send ok\n");
    return;
}

void sim_test_shutdown(int index)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    ret = bsp_esl_shutdown(g_test_sock->client_sock[index].clientfd, SHUT_FLAG);
    if (ret < 0) {
        bsp_err("shutdown err\n");
        return;
    }
    SOCKET_DBG("shutdown ok\n");
    return;
}

void sim_test_close(int sockfd)
{
    int ret;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    ret = bsp_esl_close(sockfd);

    if (ret < 0) {
        bsp_err("close err\n");
        return;
    }
    SOCKET_DBG("close ok\n");
    return;
}

void sim_test_sendrecv(int type, int serv_port, unsigned int sendrecv_times)
{
    unsigned int forever = (0 == sendrecv_times) ? 1 : 0;
    int index = 0;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    (void)sim_test_creat(type);
    (void)sim_test_bind(serv_port);
    (void)sim_test_listen();
    (void)sim_test_accept(index);

    do {
        (void)sim_test_recv(index);
        (void)sim_test_send(index, 0);
        sendrecv_times--;
    } while (sendrecv_times || forever);

    (void)sim_test_close(g_test_sock->client_sock[index].clientfd);
    (void)sim_test_close(g_test_sock->sockfd);

    SOCKET_DBG("test_sendrecv finished!\n");
}

void svlan_test_sendto(unsigned long long ipv4, int serv_port, unsigned int times, unsigned int data_len)
{
    int ret;
    unsigned int forever = (0 == times) ? 1 : 0;
    struct hisk_info hiskinfo;

    if (g_svlan_init_ok != 1) {
        bsp_err("please run svlan_test_init first\n");
        return;
    }

    if (ipv4 == 0) {
        bsp_err("please input ipv4\n");
        return;
    }

    (void)sim_test_creat(SOCK_DGRAM);

    g_test_sock->serv_addr.sin_family = AF_INET;
    g_test_sock->serv_addr.sin_port = htons(serv_port);
    g_test_sock->serv_addr.sin_addr.s_addr = ipv4;

    g_test_sock->sendto_len = data_len;
    ret = memcpy_s(g_test_sock->sendto_buf, sizeof(g_test_sock->sendto_buf), g_svlan_data, g_test_sock->sendto_len);
    if (ret != 0) {
        return;
    }

    do {
        hiskinfo.fd = g_test_sock->sockfd;
        hiskinfo.buff = g_test_sock->sendto_buf;
        hiskinfo.len = g_test_sock->sendto_len;
        ret = bsp_esl_sendto(&hiskinfo, 0, (void *)&g_test_sock->serv_addr, sizeof(g_test_sock->serv_addr));
        if (ret < 0) {
            bsp_err("sendto err\r\n");
            return;
        }

        SOCKET_DBG("sendto ok, %d bytes\r\n", ret);

        times--;

    } while (times || forever);

    (void)sim_test_close(g_test_sock->sockfd);

    SOCKET_DBG("test_sendto finished!");
}
