/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fm1230_ecdsa_api.c
 *
 * interface for fm1230_api.c
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef FM1230_ECSDA_API_H
#define FM1230_ECSDA_API_H
int fm1230_ecdsa_precombpoint(const unsigned char *combtable, size_t tablen,
	const unsigned char *pa, size_t palen);

int fm1230_ecdsa_verify(const unsigned char *combtable, size_t tablen,
	const unsigned char *pa, size_t palen, const unsigned char *hash,
	size_t hlen, const unsigned char *sig, size_t slen);

int fm1230_sha256(const unsigned char *msg, size_t msglen,
	unsigned char *outhash);

#endif /* END FM1230_ECSDA_API_H */
