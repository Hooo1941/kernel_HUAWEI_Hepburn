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

#ifndef __BSP_ESL_SOCKET_H__
#define __BSP_ESL_SOCKET_H__

#ifdef FEATURE_SVLSOCKET
int bsp_esl_socket(int family, int type, int protocol);

int bsp_esl_bind(int fd, struct sockaddr *addr, int addrlen);

int bsp_esl_listen(int fd, int backlog);

int bsp_esl_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen);

int bsp_esl_connect(int fd, struct sockaddr *addr, int addrlen);

int bsp_esl_select(int n, sk_fd_set *inp, sk_fd_set *outp, sk_fd_set *exp_fdset, struct timespec64 *tvp);

int bsp_esl_close(int fd);

int bsp_esl_send(int fd, void *buff, size_t len, int flags);

int bsp_esl_recv(int fd, void *ubuf, size_t size, int flags);

int bsp_esl_sendto(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int addr_len);

int bsp_esl_recvfrom(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int *addr_len);

int bsp_esl_getsockname(int fd, struct sockaddr *addr, int *addrlen);

int bsp_esl_getpeername(int fd, struct sockaddr *addr, int *addrlen);

int bsp_esl_getsockopt(int fd, int level, int optname, char *optval, int *optlen);

int bsp_esl_setsockopt(int fd, int level, int optname, char *optval, unsigned int optlen);

int bsp_esl_ioctl(int fd, int cmd, unsigned int arg);

int bsp_esl_shutdown(int fd, enum sock_shutdown_cmd how);

unsigned int bsp_esl_inet_addr(const char *str);
#else
static inline int bsp_esl_socket(int family, int type, int protocol)
{
    return 0;
}

static inline int bsp_esl_bind(int fd, struct sockaddr *addr, int addrlen)
{
    return 0;
}

static inline int bsp_esl_listen(int fd, int backlog)
{
    return 0;
}

static inline int bsp_esl_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
    return 0;
}

static inline int bsp_esl_connect(int fd, struct sockaddr *addr, int addrlen)
{
    return 0;
}

static inline int bsp_esl_select(int n, sk_fd_set *inp, sk_fd_set *outp, sk_fd_set *exp_fdset, struct timespec64 *tvp)
{
    return 0;
}

static inline int bsp_esl_close(int fd)
{
    return 0;
}

static inline int bsp_esl_send(int fd, void *buff, size_t len, int flags)
{
    return 0;
}

static inline int bsp_esl_recv(int fd, void *ubuf, size_t size, int flags)
{
    return 0;
}

static inline int bsp_esl_sendto(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int addr_len)
{
    return 0;
}

static inline int bsp_esl_recvfrom(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int *addr_len)
{
    return 0;
}

static inline int bsp_esl_getsockname(int fd, struct sockaddr *addr, int *addrlen)
{
    return 0;
}

static inline int bsp_esl_getpeername(int fd, struct sockaddr *addr, int *addrlen)
{
    return 0;
}

static inline int bsp_esl_getsockopt(int fd, int level, int optname, char *optval, int *optlen)
{
    return 0;
}

static inline int bsp_esl_setsockopt(int fd, int level, int optname, char *optval, unsigned int optlen)
{
    return 0;
}

static inline int bsp_esl_ioctl(int fd, int cmd, unsigned int arg)
{
    return 0;
}

static inline int bsp_esl_shutdown(int fd, enum sock_shutdown_cmd how)
{
    return 0;
}

static inline unsigned int bsp_esl_inet_addr(const char *str)
{
    return 0;
}
#endif   

#endif /* __BSP_ESL_SOCKET_H__ */