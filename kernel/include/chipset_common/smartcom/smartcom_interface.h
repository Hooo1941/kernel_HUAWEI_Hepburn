/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#ifndef _SMARTCOM_INTERFACE_H
#define _SMARTCOM_INTERFACE_H

#include <linux/socket.h>
#include <net/sock.h>
#include "smartcom.h"

void smartcom_bind2device(struct sock *sk, struct sockaddr *uaddr);

void smartcom_tcp_fallback(struct sock *sk, int reason, int state);

#endif /* _SMARTCOM_INTERFACE_H */