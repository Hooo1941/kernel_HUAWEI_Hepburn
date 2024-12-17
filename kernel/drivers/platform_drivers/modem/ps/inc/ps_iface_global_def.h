/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

 */

#ifndef PS_IFACE_GLOBAL_DEF_H
#define PS_IFACE_GLOBAL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Global ps iface defination
 */
enum PS_IFACE_Id {
	PS_IFACE_ID_INVAL = 0,
	PS_IFACE_ID_PPP0,               /**< PPP */

	PS_IFACE_ID_UNET_BEGIN,         /**< USB NET */
	PS_IFACE_ID_UNET0 = PS_IFACE_ID_UNET_BEGIN,
	PS_IFACE_ID_UNET1,
	PS_IFACE_ID_UNET2,
	PS_IFACE_ID_UNET3,
	PS_IFACE_ID_UNET4,
	PS_IFACE_ID_UNET5,
	PS_IFACE_ID_UNET_END = PS_IFACE_ID_UNET5,

	PS_IFACE_ID_PNET_BEGIN,         /**< PCIE NET */
	PS_IFACE_ID_PNET0 = PS_IFACE_ID_PNET_BEGIN,
	PS_IFACE_ID_PNET1,
	PS_IFACE_ID_PNET2,
	PS_IFACE_ID_PNET3,
	PS_IFACE_ID_PNET4,
	PS_IFACE_ID_PNET5,
	PS_IFACE_ID_PNET6,
	PS_IFACE_ID_PNET7,
	PS_IFACE_ID_PNET_END = PS_IFACE_ID_PNET7,

	PS_IFACE_ID_RMNET_DATA_BEGIN,   /**< RNIC */
	PS_IFACE_ID_RMNET0 = PS_IFACE_ID_RMNET_DATA_BEGIN,
	PS_IFACE_ID_RMNET1,
	PS_IFACE_ID_RMNET2,
	PS_IFACE_ID_RMNET3,
	PS_IFACE_ID_RMNET4,
	PS_IFACE_ID_RMNET5,
	PS_IFACE_ID_RMNET6,
	PS_IFACE_ID_RMNET7,
	PS_IFACE_ID_RMNET8,
	PS_IFACE_ID_RMNET9,
	PS_IFACE_ID_RMNET10,
	PS_IFACE_ID_RMNET11,
	PS_IFACE_ID_RMNET_DATA_END = PS_IFACE_ID_RMNET11,

	PS_IFACE_ID_RMNET_D2D,          /**< D2D */
	PS_IFACE_ID_RMNET_MBS,          /**< MBS */

	PS_IFACE_ID_RMNET_IMS_BEGIN,    /**< 3GPP IMS MODEM0 VIDEO */
	PS_IFACE_ID_RMNET_IMS00 = PS_IFACE_ID_RMNET_IMS_BEGIN,
	PS_IFACE_ID_RMNET_IMS10,        /**< 3GPP IMS MODEM1 VIDEO */
	PS_IFACE_ID_RMNET_EMC0,         /**< 3GPP IMS MODEM0 EMC */
	PS_IFACE_ID_RMNET_EMC1,         /**< 3GPP IMS MODEM1 EMC */
	PS_IFACE_ID_RMNET_IMS_END = PS_IFACE_ID_RMNET_EMC1,

	PS_IFACE_ID_RMNET_R_IMS_BEGIN,  /**< IWLAN IMS MODEM0 SIG */
	PS_IFACE_ID_RMNET_R_IMS00 = PS_IFACE_ID_RMNET_R_IMS_BEGIN,
	PS_IFACE_ID_RMNET_R_IMS01,      /**< IWLAN IMS MODEM0 EMC */
	PS_IFACE_ID_RMNET_R_IMS10,      /**< IWLAN IMS MODEM1 SIG */
	PS_IFACE_ID_RMNET_R_IMS11,      /**< IWLAN IMS MODEM1 EMC */
	PS_IFACE_ID_RMNET_R_IMS_END = PS_IFACE_ID_RMNET_R_IMS11,

	PS_IFACE_ID_NIC_IMS00,          /**< IMS MODEM0 NORMAL */
	PS_IFACE_ID_NIC_IMS10,          /**< IMS MODEM1 NORMAL */
	PS_IFACE_ID_NIC_EMC0,           /**< IMS MODEM0 EMC */
	PS_IFACE_ID_NIC_EMC1,           /**< IMS MODEM1 EMC */

	PS_IFACE_ID_BIP_BEGIN, /* BIP MODEM0 */
	PS_IFACE_ID_BIP0 = PS_IFACE_ID_BIP_BEGIN,
	PS_IFACE_ID_BIP1,  /* BIP MODEM1 */
	PS_IFACE_ID_BIP_END = PS_IFACE_ID_BIP1,

	PS_IFACE_ID_LTEV,               /**< LTEV */
	PS_IFACE_ID_L2_CUSTOM,
	PS_IFACE_ID_TEST,                /**< TEST */
	PS_IFACE_ID_TEST_END = PS_IFACE_ID_TEST + 8,

	PS_IFACE_ID_BUTT
};

#ifdef __cplusplus
}
#endif

#endif /* PS_IFACE_GLOBAL_DEF_H */
