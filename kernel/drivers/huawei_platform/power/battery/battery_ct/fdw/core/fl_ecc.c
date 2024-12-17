/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_ecc.c
 *
 * ecc
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

#include "fl_type.h"
#include "fl_define.h"
#include "fl_rae_struct.h"
#include "fl_public.h"

uint8_t const g_bitset_8[] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

uint8_t const g_bitset_4[] = {
	0x08, 0x04, 0x02, 0x01
};

/*
 * ecc param of nist p256k1
 * p,a,b,n,Gx,Gy
 */
uint32_t const g_p256k1_param[] = {
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFEU, 0xFFFFFC2FU,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000007U,
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFEU,
	0xBAAEDCE6U, 0xAF48A03BU, 0xBFD25E8CU, 0xD0364141U,
	0x79BE667EU, 0xF9DCBBACU, 0x55A06295U, 0xCE870B07U,
	0x029BFCDBU, 0x2DCE28D9U, 0x59F2815BU, 0x16F81798U,
	0x483ADA77U, 0x26A3C465U, 0x5DA4FBFCU, 0x0E1108A8U,
	0xFD17B448U, 0xA6855419U, 0x9C47D08FU, 0xFB10D4B8U
};

uint32_t const g_p256k1_comb_table_g[] = {
	0x9981E643U, 0xE9089F48U, 0x979F48C0U, 0x33FD129CU,
	0x231E2953U, 0x29BC66DBU, 0xD7362E5AU, 0x487E2097U,
	0xCF3F851FU, 0xD4A582D6U, 0x70B6B59AU, 0xAC19C136U,
	0x8DFC5D5DU, 0x1F1DC64DU, 0xB15EA6D2U, 0xD3DBABE2U,
	0x4C276680U, 0xCCC851A3U, 0x9CDD9C46U, 0x2EF8A9CFU,
	0x3FE27C6AU, 0x195EDBF5U, 0xB6B50EDAU, 0x272E9751U,
	0x911A49B7U, 0x7F23719CU, 0xDCED315CU, 0x48B8FA3EU,
	0x61480000U, 0x6B0BA84CU, 0xC8E079A4U, 0x6F0A9EF3U,
	0x503326C0U, 0x02C41303U, 0xFEBA2B9FU, 0x2A1EAA19U,
	0x8250462AU, 0x6F1AD00AU, 0xF8466C6EU, 0x453CA394U,
	0xBF21C497U, 0xA776CE88U, 0x5680EBA9U, 0x5CA1F8D9U,
	0xB858B949U, 0x23E869DBU, 0x6BB3B08CU, 0x51BA0022U,
	0x44E88D51U, 0x4AD0157EU, 0xA68B3DB0U, 0x772FFF7DU,
	0xF1D4A4E1U, 0x1C8821FBU, 0x0F9D275DU, 0x3B9955D0U,
	0xA2AFB709U, 0xCFDF1C78U, 0x65939879U, 0x278783B8U,
	0xBE06C395U, 0xCC8B83A3U, 0x30215FB5U, 0xB2CD6A45U,
	0x71662B3EU, 0x47D9B35CU, 0x6BEE8E98U, 0x4F7F2FEAU,
	0x72D2DF1AU, 0x423EB4A0U, 0x62C93557U, 0x38539EFDU,
	0xE0A87BC8U, 0xF601D8D7U, 0x9753E2AAU, 0x54F1DCD6U,
	0xD0055381U, 0x7E56D175U, 0xCE1105D2U, 0x280E606EU,
	0xBC1AAAC7U, 0x2099D690U, 0x9B561027U, 0x4D63FDBFU,
	0xAA6EC255U, 0xA7FD0117U, 0xC8FA84EDU, 0xDDCD3A20U,
	0x3E99283AU, 0xBF222EBAU, 0xADC52EF1U, 0xB046D87DU,
	0x18753B06U, 0x9A4746CBU, 0x26AD48D7U, 0x06F85BF6U,
	0x8EACBCA1U, 0x8180A5BAU, 0x83D30C4BU, 0xAF3CF8B9U,
	0x4343106AU, 0xB84F2EA0U, 0xB10D1ACBU, 0x6BE91603U,
	0x546546C0U, 0x521DAFCAU, 0xC837892DU, 0xD5C93171U,
	0x73E91BA2U, 0x0499245EU, 0xDE546CB4U, 0xFEB50208U,
	0xA3F1F2C4U, 0x6EB36131U, 0xBA22B299U, 0xF7897292U,
	0x6CE89ABEU, 0x40EB2CAAU, 0x9D45C63BU, 0xF32EB964U,
	0x3A5F0315U, 0xDDE61148U, 0x8C1FDF17U, 0xE9F9DAE6U,
	0x3E890C67U, 0x87CB87DAU, 0x5F6431B2U, 0x13653600U,
	0xF6D4237AU, 0xBEA4C2AEU, 0x7D2EF2B3U, 0x461FCF57U,
	0xFD0FEE51U, 0xAC2EA41BU, 0xD9A4FC49U, 0xC84327ABU,
	0xFB090B56U, 0xA2BDDF19U, 0x41F1F16AU, 0x1FFBD830U,
	0x0C68135CU, 0x65AC2F1BU, 0x138C6B10U, 0x4738D403U,
	0xF26A1DDFU, 0x6A32B094U, 0x7BF38034U, 0xA27C2A33U,
	0x22798C68U, 0x8509EA65U, 0x621DFBD9U, 0x6A503986U,
	0xF927F6A6U, 0xFC2E9E1DU, 0x100846BAU, 0x898AA2CAU,
	0x4C5A671FU, 0x5C20FE84U, 0xE36DC3AAU, 0xD8C026D0U,
	0x53557A6CU, 0xA41A83A0U, 0x51A70443U, 0x39DE2617U,
	0x72D899EAU, 0x6B71528DU, 0x2D99A0ACU, 0xBD5AA85BU,
	0x87D643B8U, 0x0F2B17F0U, 0xB908122BU, 0x83417F0FU,
	0x1EB36D9CU, 0xD2FE08D9U, 0xC8E7E2C2U, 0x89066E4BU,
	0xEF216F5EU, 0xEEB8B7D9U, 0xA88AEEBBU, 0x4FDD51D2U,
	0x2DF092B7U, 0x7BE7422BU, 0xC918AB79U, 0xC19B7D09U,
	0xA64B189CU, 0x6E8CCF0AU, 0x2ED89843U, 0x2497A269U,
	0x9D0712F1U, 0xBA34B38CU, 0x5510F223U, 0x3BD2C72EU,
	0xA374E709U, 0xB61B6569U, 0xDAF9CBD7U, 0x66AD0F6CU,
	0xE10294AEU, 0x0E4C3B96U, 0x357963D7U, 0x6D2A3A3FU,
	0x66BAF0B0U, 0x731D1785U, 0x4C205854U, 0xF2AEB404U,
	0x7AEB8ADAU, 0x93525BFAU, 0x22710BA2U, 0xCD0E79FEU,
	0x9D40C396U, 0xF88661D0U, 0x2313920BU, 0xD698DD4CU,
	0x453F6C75U, 0xB00A94E0U, 0x07BDAED6U, 0x8F64EC45U,
	0x59285EA2U, 0x6CDA2354U, 0xA6755975U, 0xC45E2C10U,
	0x9303105FU, 0xD9B2767BU, 0x56C5A1C2U, 0x68B38C95U,
	0x3D844E8BU, 0x610F6627U, 0x44EB2975U, 0xF1098F69U,
	0x04B73F37U, 0xC6E788BDU, 0x3B2330FCU, 0x20AFABA9U,
	0xC27A00A0U, 0xFD58B0DFU, 0x2B25CB4AU, 0xA80DD28EU,
	0x678ECFE6U, 0x498D3E7AU, 0xB6FA4FF8U, 0xC8AC01E7U,
	0xB3B436F3U, 0x86109B4EU, 0x9EF91337U, 0x02CD5D56U,
	0xAF409552U, 0x5FE1D2DDU, 0x9F30222BU, 0x47476977U,
	0x2F2479A7U, 0xC394B509U, 0x98F120C4U, 0xA670B81DU,
	0x349B5F12U, 0x7619927FU, 0x0B4F8EBAU, 0xAC8DC00CU,
	0x9AF924BEU, 0xF72C1AF9U, 0x0DD2B856U, 0x86ED4060U,
	0xD9FC7C9EU, 0x500A563FU, 0x0F65F52AU, 0xDB12A1EFU,
	0x7DB04144U, 0x323A9634U, 0xB54AB5DEU, 0x579F69B4U,
	0xE2EFBF13U, 0xA963708BU, 0x3FDAB095U, 0x5C6B1F4EU,
	0x34B4BB84U, 0xC5B4421DU, 0xB1114E17U, 0xFD253B2FU,
	0x7509DFE8U, 0x5027040BU, 0x5D703511U, 0xE07B0BDCU,
	0x5EA10CDEU, 0xD809302FU, 0x98475841U, 0x06DE3A8BU,
	0x4B46DF75U, 0xF9B562D1U, 0x39A1CAC5U, 0x92D83C3CU,
	0x803233B4U, 0x74F901C9U, 0xC1545DEBU, 0x23496830U,
	0x07FDF045U, 0x8DB72E5BU, 0xC75F9DF6U, 0x1A89F319U,
	0x76B396B1U, 0xEB78BEA3U, 0xF7F04C19U, 0x9421E72FU,
	0x39BC7F2BU, 0x6B2116A1U, 0xB2F97BE8U, 0x0384551AU,
	0x4933D69AU, 0xBF4F62EEU, 0xAC437567U, 0x0D357A48U,
	0x1A4A12ACU, 0xDCB5D945U, 0xBAC24EBFU, 0xE8E59493U,
	0xD66E3CA5U, 0x753F607DU, 0xE9EE7538U, 0x292055A3U,
	0x5D7DB6F7U, 0xF18579DCU, 0x90654018U, 0x1B63B615U,
	0xF7FA3B50U, 0x1FD0E870U, 0x8A788C75U, 0x00535C45U,
	0xFED46D70U, 0xED1BFC3FU, 0x5458D5EEU, 0x4C7E643FU,
	0x1FD5E9C1U, 0x25C750F4U, 0x6971155CU, 0x23119AC1U,
	0x4CA0F755U, 0x9AA0DC19U, 0x53DD2C35U, 0xF5848953U,
	0xDFF2F0B2U, 0xE3F91A33U, 0xA2E6809DU, 0xCC1D4C8DU,
	0x6F4E6DA5U, 0xF379E4FEU, 0x33B8E3A8U, 0xD4A8BFBAU,
	0xF27033F7U, 0x2184DA8DU, 0x8E26068DU, 0xCD6B5911U,
	0x5CEDE529U, 0xBC688880U, 0x28C39BA1U, 0x5DE19360U,
	0xF18532D4U, 0xA7BCAFF3U, 0xAFEF1FFFU, 0xEB247A53U,
	0xA18CDCD5U, 0xD7C445C6U, 0x62DEF7EDU, 0x6D8C664CU,
	0xF8E6318FU, 0x7C9186ECU, 0x2A14242FU, 0x2DF3EF7DU,
	0xA6D78326U, 0xDF5BAE09U, 0x9D6B98BEU, 0x8228F8B6U,
	0x4C54805CU, 0x405368C3U, 0xA5DA044DU, 0x7FCBC162U,
	0xF256FC31U, 0x039DC7ECU, 0x00D90669U, 0x5388BE58U,
	0xA6ED0354U, 0xA49C2CECU, 0x80F327DDU, 0x5EA79E99U,
	0xAD65AF08U, 0x396CAA7BU, 0x73B7A2DEU, 0x5123F495U,
	0x3CA73A7BU, 0x05CCB0B4U, 0x19A45C00U, 0x308B743DU,
	0xE9A611F8U, 0x73EE2A0FU, 0x52794EBDU, 0x2C896922U,
	0xD17864FFU, 0x87317216U, 0x7545F274U, 0xE2BF5CD6U,
	0x1E1460D0U, 0x97FBC2E7U, 0xD59D145CU, 0xC3810FB3U,
	0x47AF76B2U, 0x74D95B20U, 0xBEBCB68DU, 0xCAF875D9U,
	0x0B6365F2U, 0x83A06318U, 0x9F205471U, 0xBECE5E16U,
	0xE9DD1354U, 0x781D2E36U, 0x05ED206DU, 0xE708FC31U,
	0xCED8451EU, 0x3887C525U, 0x61526D38U, 0x9D43252DU,
	0xD1D2F32BU, 0xE4A3D9B3U, 0x7BB278A3U, 0x9654C8E7U,
	0x205B4971U, 0x4BD71738U, 0xF91C901CU, 0x173A580EU,
	0xE2C1B15AU, 0xAE230781U, 0x3ED0AA7FU, 0x3894BD8DU,
	0x61799465U, 0xBC194851U, 0x0C420172U, 0x8C7A391AU,
	0x2C7E0A87U, 0x0FF2EE62U, 0x800EB921U, 0xBCDDF476U,
	0xC8E8FD33U, 0xC7397511U, 0xEA72B31DU, 0xF4B12AEBU,
	0xB5CABC35U, 0x666CAF6DU, 0x2BACB6C2U, 0x48839306U,
	0x65CC9E5EU, 0xBD319246U, 0x10945EEEU, 0x61524D2CU,
	0x35F6E94BU, 0xDF6AD5A9U, 0xA8686295U, 0x68E4D49EU,
	0x865A2503U, 0xB5062A99U, 0xB82D1543U, 0x4CFDAC91U,
	0x31066637U, 0x3794F8DAU, 0x2D1D56E8U, 0x30876F83U,
	0x870C881EU, 0x79C2A544U, 0x837EE317U, 0xE90E2C19U,
	0xF1E4B835U, 0xDF5D836EU, 0xE6BE31C8U, 0xD7A44DF9U,
	0x31A76AFFU, 0xF6CC38C3U, 0x4A530E4AU, 0x89142765U,
	0x9C55F910U, 0xD4F49D02U, 0x9323F507U, 0x8ADD0D82U,
	0x71AE8BECU, 0x175C4D8BU, 0x8367AE20U, 0x5B6AC21EU
};

/* const ecc struct of p256k1 */
struct str_ecc_param const g_str_p256k1_param = {
	256,
	(uint32_t *)g_p256k1_param,
	(uint32_t *)(g_p256k1_param + 0x08),
	(uint32_t *)(g_p256k1_param + 0x10),
	(uint32_t *)(g_p256k1_param + 0x18),
	(struct str_affpoint *)(g_p256k1_param + 0x20),
	(uint32_t *)g_p256k1_comb_table_g,
	0x01,
	0x00
};

/*
 *       函数名称: ecc_init_p256k1
 * 函数功能: ECC曲线参数初始化为P256k1
 * 输入参数:
 * 			struct str_ecc_key *key, ecc key指针
 * 			struct str_pae *pae_ctx1, pae 结构指针
 * 输出参数:
 * 			RESULT_OK    - 正确返回
 */
uint8_t ecc_init_p256k1(struct str_ecc_key *key, struct str_pae *pae_ctx1)
{
	key->param   = (struct str_ecc_param *)(&g_str_p256k1_param);
	key->pae_ctx = pae_ctx1;
	key->protect_level  = PAE_LEVEL0_F;     /* RFU */
	key->comb_table_q   = (uint32_t *)0x00;
	return RESULT_OK;
}

/* set the big number to 1 */
void bn_setnumberone(uint32_t *bn_buffer, uint32_t lenbyword)
{
	bn_buffer[lenbyword - 1] = 0x01;
}

/*
 * 函数名称: ecc_precomp_rr
 * 函数功能: 预计算RR_N,RRR_N,RRRR_N
 * 输入参数:	struct str_pae *pae_ctx1, pae 结构指针
 *           uint32_t *ptr_m,模的缓冲区
 *           struct str_rr_n *ptr_rr_n,计算结果输出缓冲区指针
 * 			uint32_t lenbyword,模的字长度-
 */
void ecc_precomp_rr(struct str_pae *pae_ctx, uint32_t *ptr_m,
		struct str_rr_n *ptr_rr_n, uint32_t lenbyword)
{
	rsa_calculate_rr_n(pae_ctx, ptr_m, lenbyword * WORD_BITS);

	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK2, ptr_rr_n[0].rr_n, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK02, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, ptr_rr_n[1].rr_n, lenbyword);

	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, ptr_rr_n[2].rr_n, lenbyword);

	dma_wr_allblock(pae_ctx, 0x00, PAE_RAM_BLOCK1, lenbyword);
	rsa_write_ram(pae_ctx, PAE_RAM_BLOCK1, 0x01);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, (uint32_t *)ptr_rr_n[3].rr_n,
		lenbyword);
}

/*
 * 函数名称: ecc_in2modmult
 * 函数功能: 计算2因子模乘
 * 输入参数: struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			uint32_t *out,输出数据的缓冲区
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 			uint32_t *in1,输入数据1的缓冲区
 * 			uint32_t *in2,输入数据2的缓冲区
 * 			uint32_t lenbyword,模的字长度-
 */
void ecc_in2modmult(struct str_pae *pae_ctx, uint32_t *out,
		struct str_rr_n *ptr_rr_n,
		uint32_t *in1, uint32_t *in2, uint32_t lenbyword)
{
	dma_xdata2rsa(pae_ctx, in1, PAE_RAM_BLOCK1, lenbyword);
	dma_xdata2rsa(pae_ctx, ptr_rr_n[0].rr_n, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);

	dma_xdata2rsa(pae_ctx, in2, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, out, lenbyword);
}

/*
 * 函数名称: ECCIn3ModMult
 * 函数功能: 计算3因子模乘
 * 输入参数: struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			uint32_t *out,输出数据的缓冲区
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 			uint32_t *in1,输入数据1的缓冲区
 * 			uint32_t *in2,输入数据2的缓冲区
 * 			uint32_t *in3,输入数据3的缓冲区
 * 			uint32_t lenbyword,模的字长度-
 */
void ecc_in3modmult(struct str_pae *pae_ctx, uint32_t *out,
		struct str_rr_n *ptr_rr_n,
		uint32_t *in1, uint32_t *in2, uint32_t *in3, uint32_t lenbyword)
{
	dma_xdata2rsa(pae_ctx, in1, PAE_RAM_BLOCK1, lenbyword);
	dma_xdata2rsa(pae_ctx, ptr_rr_n[1].rr_n, PAE_RAM_BLOCK2, lenbyword);

	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);

	dma_xdata2rsa(pae_ctx, in2, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);

	dma_xdata2rsa(pae_ctx, in3, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, out, lenbyword);
}

/*
 * 函数名称: ecc_jacob2aff
 * 函数功能: 计算雅克比坐标转化为彷射坐标
 * 输入参数: struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			struct str_affpoint *out_aff,结果点彷射坐标指针
 * 			struct str_jacpoint *jac_p1,数据点的雅克比坐标指针
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 			uint32_t lenbyword,模的字长度
 * 返回值:
 * 			RESULT_OK- 计算结果为正常点
 * 			RESULT_INFINITY - 计算结果无穷远点-
 */
uint8_t ecc_jacob2aff(struct str_pae *pae_ctx, struct str_affpoint *out_aff,
		struct str_jacpoint *jac_p1, struct str_rr_n *ptr_rr_n,
		uint32_t lenbyword)
{
	uint32_t ecc_m[ECCMAXWORDLEN];
	uint32_t ecc_temp[ECCMAXWORDLEN];
	uint32_t ecc_temp2[ECCMAXWORDLEN];

	dma_wr_allxdata(0x00, ecc_temp, lenbyword);
	if (cmp_bn(jac_p1->jac_xyz + 2 * lenbyword, ecc_temp, lenbyword) == 0)
		return RESULT_INFINITY;

	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK0, ecc_m, lenbyword);
	bignum_modinv_soft(pae_ctx, ecc_temp, jac_p1->jac_xyz + 2 * lenbyword,
		ecc_m, lenbyword);
	ecc_in2modmult(pae_ctx, ecc_temp2, ptr_rr_n, ecc_temp,
		ecc_temp, lenbyword);
	ecc_in2modmult(pae_ctx, out_aff->aff_xy, ptr_rr_n, jac_p1->jac_xyz,
		ecc_temp2, lenbyword);
	ecc_in3modmult(pae_ctx, out_aff->aff_xy + lenbyword, ptr_rr_n,
		jac_p1->jac_xyz + lenbyword, ecc_temp2, ecc_temp, lenbyword);

	return RESULT_OK;
}

/*
 * 函数名称: ecc_descar2mont_affine
 * 函数功能: 计算彷射点坐标由笛卡尔域转化为蒙哥马利域
 * 输入参数: struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			struct str_affpoint aff_p1,点彷射坐标指针
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区指针
 * 			uint32_t lenbyword,模的字长度
 * 返回值:   N/A
 */
void ecc_descar2mont_affine(struct str_pae *pae_ctx,
		struct str_affpoint *aff_p1, struct str_rr_n *ptr_rr_n,
		uint32_t lenbyword)
{
	dma_xdata2rsa(pae_ctx, aff_p1->aff_xy, PAE_RAM_BLOCK1, lenbyword);
	dma_xdata2rsa(pae_ctx, (uint32_t *)ptr_rr_n, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, aff_p1->aff_xy, lenbyword);
	dma_xdata2rsa(pae_ctx, aff_p1->aff_xy + lenbyword,
		PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1,
		aff_p1->aff_xy + lenbyword, lenbyword);
}

/*
 * 函数名称: ecc_descar2mont_jacobian
 * 函数功能: 计算雅克比点坐标由笛卡尔域转化为蒙哥马利域
 * 输入参数:	struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			struct str_jacpoint *jac_p1,点的雅克比坐标指针
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 			uint32_t lenbyword,模的字长度
 * 返回值:   N/A
 */
void ecc_descar2mont_jacobian(struct str_pae *pae_ctx,
		struct str_jacpoint *jac_p1, struct str_rr_n *ptr_rr_n,
		uint32_t lenbyword)
{
	dma_xdata2rsa(pae_ctx, (uint32_t *)ptr_rr_n, PAE_RAM_BLOCK2, lenbyword);

	dma_xdata2rsa(pae_ctx, jac_p1->jac_xyz, PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, jac_p1->jac_xyz, lenbyword);

	dma_xdata2rsa(pae_ctx, jac_p1->jac_xyz + lenbyword,
		PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1,
		jac_p1->jac_xyz + lenbyword, lenbyword);

	dma_xdata2rsa(pae_ctx, jac_p1->jac_xyz + 2 * lenbyword,
		PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1,
		jac_p1->jac_xyz + 2 * lenbyword, lenbyword);
}

/*
 * 函数名称: ecc_mont2descar_jacobian
 * 函数功能: 计算雅克比坐标由蒙哥马利域转化为笛卡尔域
 * 			计算
 * 输入参数:	struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			struct str_jacpoint *jac_p1,数据点的雅克比坐标指针
 * 			uint32_t lenbyword,模的字长度
 */
void ecc_mont2descar_jacobian(struct str_pae *pae_ctx,
		struct str_jacpoint *jac_p1, uint32_t lenbyword)
{
	dma_wr_allblock(pae_ctx, 0x00, PAE_RAM_BLOCK2, lenbyword);
	rsa_write_ram(pae_ctx, PAE_RAM_BLOCK2, 0x01);

	dma_xdata2rsa(pae_ctx, jac_p1->jac_xyz, PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1, jac_p1->jac_xyz, lenbyword);

	dma_xdata2rsa(pae_ctx, jac_p1->jac_xyz + lenbyword,
		PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1,
		jac_p1->jac_xyz + lenbyword, lenbyword);

	dma_xdata2rsa(pae_ctx, jac_p1->jac_xyz + 2 * lenbyword,
		PAE_RAM_BLOCK1, lenbyword);
	rsa_command(pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK1,
		jac_p1->jac_xyz + 2 * lenbyword, lenbyword);
}

uint8_t  ecc_dbl_pro(struct str_ecc_key *key, struct str_affpoint *out,
		struct str_affpoint *in1,
		struct str_jacpoint *jac_point, struct str_rr_n *ptr_rr_n,
		uint32_t lenbyword)
{
	dma_xdata2xdata(in1->aff_xy, jac_point->jac_xyz, lenbyword * 2);
	dma_wr_allxdata(0x00, jac_point->jac_xyz + lenbyword * 2, lenbyword);
	bn_setnumberone(jac_point->jac_xyz + lenbyword * 2, lenbyword);

	ecc_descar2mont_jacobian(key->pae_ctx, jac_point,
		ptr_rr_n, lenbyword);
	dma_xdata2rsa(key->pae_ctx, jac_point->jac_xyz,
		PAE_RAM_BLOCK01, lenbyword);
	dma_xdata2rsa(key->pae_ctx, jac_point->jac_xyz + lenbyword,
		PAE_RAM_BLOCK02, lenbyword);
	dma_xdata2rsa(key->pae_ctx, jac_point->jac_xyz + lenbyword * 2,
		PAE_RAM_BLOCK03, lenbyword);
	if (key->param->a_is_minus3 != 0x01) {
		dma_xdata2rsa(key->pae_ctx, (uint32_t *)ptr_rr_n,
			PAE_RAM_BLOCK08, lenbyword);
		dma_xdata2rsa(key->pae_ctx, key->param->a,
			PAE_RAM_BLOCK07, lenbyword);
		rsa_adccommand(key->pae_ctx, RSA_MULT, BLOCK06, BLOCK07, BLOCK08);
	}
	dma_wr_allblock(key->pae_ctx, 0x00, PAE_RAM_BLOCK0E, lenbyword);
	ecc_point_dbl(key->pae_ctx, key->param->a_is_minus3);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK01,
		jac_point->jac_xyz, lenbyword);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK02,
		jac_point->jac_xyz + lenbyword, lenbyword);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK03,
		jac_point->jac_xyz + lenbyword * 2, lenbyword);
	ecc_mont2descar_jacobian(key->pae_ctx, jac_point, lenbyword);
	return ecc_jacob2aff(key->pae_ctx, out, jac_point, ptr_rr_n, lenbyword);
}

/*
 * 函数名称: ecc_pointadd_pro
 * 函数功能: 计算彷射坐标点加, out = in1 + in2
 * 输入参数: struct str_ecc_key *key, ecc key结构体指针
 * 			struct str_affpoint *out,点加后的结果点彷射坐标指针
 * 			struct str_affpoint *in1,数据点1的彷射坐标指针
 * 			struct str_affpoint *in2,数据点2的彷射坐标指针
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 返回值:
 * 			RESULT_OK- 计算结果为正常点
 * 			RESULT_INFINITY - 计算结果无穷远点
 */
uint8_t ecc_pointadd_pro(struct str_ecc_key *key, struct str_affpoint *out,
		struct str_affpoint *in1, struct str_affpoint *in2,
		struct str_rr_n *ptr_rr_n)
{
	uint32_t ecc_temp[ECCMAXWORDLEN];
	uint32_t ecc_temp2[ECCMAXWORDLEN];
	struct str_jacpoint jac_point;
	struct str_affpoint aff_point;
	uint32_t len_w;

	len_w = key->param->bitlen / WORD_BITS;
	dma_xdata2rsa(key->pae_ctx, in2->aff_xy, PAE_RAM_BLOCK1, len_w);
	dma_xdata2rsa(key->pae_ctx, in1->aff_xy, PAE_RAM_BLOCK2, len_w);
	rsa_command(key->pae_ctx, RSA_SUB, BLOCK01, BLOCK01, BLOCK02);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK1, ecc_temp, len_w);
	dma_wr_allxdata(0x00, ecc_temp2, len_w);
	if (cmp_bn(ecc_temp, ecc_temp2, len_w) == 0x00) {
		if (cmp_bn(in1->aff_xy + len_w,
			in2->aff_xy + len_w, len_w) == 0x00)
			return (ecc_dbl_pro(key, out, in1, &jac_point,
				ptr_rr_n, len_w));
		return RESULT_INFINITY;
	}

	dma_xdata2xdata(in1->aff_xy, jac_point.jac_xyz, len_w * 2);
	dma_wr_allxdata(0x00, jac_point.jac_xyz + len_w * 2, len_w);
	bn_setnumberone(jac_point.jac_xyz + len_w * 2, len_w);
	ecc_descar2mont_jacobian(key->pae_ctx, &jac_point, ptr_rr_n, len_w);

	dma_xdata2xdata(in2->aff_xy, aff_point.aff_xy, len_w * 2);
	ecc_descar2mont_affine(key->pae_ctx, &aff_point, ptr_rr_n, len_w);

	dma_xdata2rsa(key->pae_ctx, jac_point.jac_xyz, PAE_RAM_BLOCK01, len_w);
	dma_xdata2rsa(key->pae_ctx, jac_point.jac_xyz + len_w,
		PAE_RAM_BLOCK02, len_w);
	dma_xdata2rsa(key->pae_ctx, jac_point.jac_xyz + len_w * 2,
		PAE_RAM_BLOCK03, len_w);

	dma_xdata2rsa(key->pae_ctx, aff_point.aff_xy, PAE_RAM_BLOCK04, len_w);
	dma_xdata2rsa(key->pae_ctx, aff_point.aff_xy +
		len_w, PAE_RAM_BLOCK05, len_w);
	dma_wr_allblock(key->pae_ctx, 0x00, PAE_RAM_BLOCK0E, len_w);

	ecc_point_mix_add(key->pae_ctx);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK01, jac_point.jac_xyz, len_w);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK02,
		jac_point.jac_xyz + len_w, len_w);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK03,
		jac_point.jac_xyz + len_w * 2, len_w);
	ecc_mont2descar_jacobian(key->pae_ctx, &jac_point, len_w);
	return (ecc_jacob2aff(key->pae_ctx, out, &jac_point,
		ptr_rr_n, len_w));
}

/*
 * 函数名称: ecc_pointadd
 * 函数功能: 计算点加_彷射坐标,out = in1 + in2,给外部函数调用
 * 输入参数:
 * 			struct str_ecc_key *key,曲线参数指针
 * 			struct str_affpoint *In1,输入点1的彷射坐标指针
 * 			struct str_affpoint *In2,输入点2的彷射坐标指针
 * 			struct str_affpoint *Out,输出点坐标
 * 返回值:
 * 			RESULT_OK- 计算结果为正常点
 * 			RESULT_INFINITY - 计算结果无穷远点
 */
uint8_t ecc_pointadd(struct str_ecc_key *key,
		struct str_affpoint *in1, struct str_affpoint *in2,
		struct str_affpoint *out)
{
	struct str_rr_n ptr_rr_n[ECC_RR_N_MAX];
	uint32_t lenbyword;

	lenbyword = key->param->bitlen / WORD_BITS;

	ecc_precomp_rr(key->pae_ctx, key->param->p, ptr_rr_n, lenbyword);
	return ecc_pointadd_pro(key, out, in1, in2, ptr_rr_n);
}

void ecc_calculate_2_n_dp(struct str_pae *pae_ctx,
		struct str_jacpoint *jac_p1,
		uint32_t d, uint8_t a_equ_neg3, uint32_t lenbyword)
{
	uint32_t i;

	for (i = 0; i < d; i++)
		ecc_point_dbl(pae_ctx, a_equ_neg3);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK01, jac_p1->jac_xyz, lenbyword);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK02,
		jac_p1->jac_xyz + lenbyword, lenbyword);
	dma_rsa2xdata(pae_ctx, PAE_RAM_BLOCK03,
		jac_p1->jac_xyz + lenbyword * 2, lenbyword);
	ecc_mont2descar_jacobian(pae_ctx, jac_p1, lenbyword);
}
/*
 * 函数名称: ecc_precomp_dp
 * 函数功能: 固定点点乘的预计算DP
 * 			预计算[2^d]P,[2^2d]P,[2^3d]P
 * 输入参数:	struct str_pae *pae_ctx, pae结构体缓冲区指针
 * 			struct str_affpoint *affp_g,基点P1(InX1,InY1)指针
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 			uint8_t a_equ_neg3,a==-3标志
 * 			struct str_affpoint *ptr_ecc_comb_dp,DP存储点指针
 * 			ptr_ecc_comb_dp[3]存放[2^d]P,[2^2d]P,[2^3d]P
 * 			uint32_t lenbyword,模的字长度
 * 函数返回值:N/A
 */
void ecc_precomp_dp(struct str_pae *pae_ctx, struct str_affpoint *affp_g,
		struct str_rr_n *ptr_rr_n, uint8_t a_equ_neg3,
		struct str_affpoint *ptr_ecc_comb_dp, uint32_t lenbyword)
{
	struct str_affpoint aff_p1;
	struct str_jacpoint jac_p1;
	uint32_t d;

	d = lenbyword * 8;
	dma_xdata2xdata(affp_g->aff_xy, jac_p1.jac_xyz, lenbyword * 2);
	dma_wr_allxdata(0x00, jac_p1.jac_xyz + lenbyword * 2, lenbyword);
	bn_setnumberone(jac_p1.jac_xyz + lenbyword * 2, lenbyword);
	ecc_descar2mont_jacobian(pae_ctx, &jac_p1, ptr_rr_n, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz, PAE_RAM_BLOCK01, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz + lenbyword,
		PAE_RAM_BLOCK02, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz + lenbyword * 2,
		PAE_RAM_BLOCK03, lenbyword);
	dma_wr_allblock(pae_ctx, 0x00, PAE_RAM_BLOCK0E, lenbyword);

	/* calculate [2^d]P  */
	ecc_calculate_2_n_dp(pae_ctx, &jac_p1, d, a_equ_neg3, lenbyword);
	ecc_jacob2aff(pae_ctx, &aff_p1, &jac_p1, ptr_rr_n, lenbyword);
	dma_xdata2xdata(aff_p1.aff_xy, ptr_ecc_comb_dp[0].aff_xy, lenbyword * 2);
	ecc_descar2mont_jacobian(pae_ctx, &jac_p1, ptr_rr_n, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz, PAE_RAM_BLOCK01, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz + lenbyword, PAE_RAM_BLOCK02,
		lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz + lenbyword * 2, PAE_RAM_BLOCK03,
		lenbyword);
	dma_wr_allblock(pae_ctx, 0x00, PAE_RAM_BLOCK0E, lenbyword);

	/* calculate [2^2d]P  */
	ecc_calculate_2_n_dp(pae_ctx, &jac_p1, d, a_equ_neg3, lenbyword);
	ecc_jacob2aff(pae_ctx, &aff_p1, &jac_p1, ptr_rr_n, lenbyword);
	dma_xdata2xdata(aff_p1.aff_xy, ptr_ecc_comb_dp[1].aff_xy, lenbyword * 2);
	ecc_descar2mont_jacobian(pae_ctx, &jac_p1, ptr_rr_n, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz, PAE_RAM_BLOCK01, lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz + lenbyword, PAE_RAM_BLOCK02,
		lenbyword);
	dma_xdata2rsa(pae_ctx, jac_p1.jac_xyz + lenbyword * 2,
		PAE_RAM_BLOCK03, lenbyword);
	dma_wr_allblock(pae_ctx, 0x00, PAE_RAM_BLOCK0E, lenbyword);

	/* calculate [2^3d]P  */
	ecc_calculate_2_n_dp(pae_ctx, &jac_p1, d, a_equ_neg3, lenbyword);
	ecc_jacob2aff(pae_ctx, &aff_p1, &jac_p1, ptr_rr_n, lenbyword);
	dma_xdata2xdata(aff_p1.aff_xy, ptr_ecc_comb_dp[2].aff_xy,
		lenbyword * 2);
}

/*
 * 函数名称: ecc_precomp_np
 * 函数功能: 输入点点乘的预计算NP
 * 			预计算[Ai]P
 * 输入参数:
 * 			struct str_ecc_key *key,ECC曲线参数
 * 			struct str_rr_n *ptr_rr_n,RR_N的缓冲区
 * 			struct str_affpoint ptr_np,计算输出的NP点坐标指针
 * 			struct str_affpoint *ptr_in,输入点坐标
 * 			struct str_affpoint *ptr_comb_dp,[2^d]P,[2^2d]P,[2^3d]P存放指针
 * 			uint32_t n_value,乘数,0x01~0x0F
 */
void ecc_precomp_np(struct str_ecc_key *key, struct str_rr_n *ptr_rr_n,
		struct str_affpoint *ptr_np, struct str_affpoint *ptr_in,
		struct str_affpoint *ptr_comb_dp, uint32_t n_value)
{
	uint32_t temp;
	uint32_t bt_findone;
	uint32_t lenbyword;

	lenbyword = key->param->bitlen / WORD_BITS;
	temp = n_value;

	bt_findone = 0;
	if ((temp & g_bitset_4[0]) == g_bitset_4[0]) {
		if (bt_findone == 1) {
			ecc_pointadd_pro(key, ptr_np, ptr_np, &ptr_comb_dp[2], ptr_rr_n);
		} else {
			bt_findone = 1;
			dma_xdata2xdata(ptr_comb_dp[2].aff_xy,
				(*ptr_np).aff_xy, lenbyword * 2);
		}
	}
	if ((temp & g_bitset_4[1]) == g_bitset_4[1]) {
		if (bt_findone == 1) {
			ecc_pointadd_pro(key, ptr_np, ptr_np, &ptr_comb_dp[1], ptr_rr_n);
		} else {
			bt_findone = 1;
			dma_xdata2xdata(ptr_comb_dp[1].aff_xy,
				(*ptr_np).aff_xy, lenbyword * 2);
		}
	}
	if ((temp & g_bitset_4[2]) == g_bitset_4[2]) {
		if (bt_findone == 1) {
			ecc_pointadd_pro(key, ptr_np, ptr_np, &ptr_comb_dp[0], ptr_rr_n);
		} else {
			bt_findone = 1;
			dma_xdata2xdata(ptr_comb_dp[0].aff_xy,
				(*ptr_np).aff_xy, lenbyword * 2);
		}
	}
	if ((temp & g_bitset_4[3]) == g_bitset_4[3]) {
		if (bt_findone == 1) {
			ecc_pointadd_pro(key, ptr_np, ptr_np, ptr_in, ptr_rr_n);
		} else {
			bt_findone = 1;
			dma_xdata2xdata(ptr_in->aff_xy,
				(*ptr_np).aff_xy, lenbyword * 2);
		}
	}
}

void ecc_precombpoint_loop_pro(struct str_ecc_key *key,
		struct str_rr_n *ptr_rr_n,
		struct str_affpoint *aff_point_np, struct str_jacpoint *jac_point_np,
		uint32_t n_loop, uint32_t lenbyword)
{
	uint32_t j;
	uint32_t e;
	uint32_t *ptr_comb_table_q;
	uint32_t *ptr_comb_table_q_2e;

	e = lenbyword * 4;
	ptr_comb_table_q    = key->comb_table_q;
	ptr_comb_table_q_2e = key->comb_table_q + 30 * lenbyword;

	ecc_descar2mont_affine(key->pae_ctx, aff_point_np, ptr_rr_n,
		lenbyword);
	dma_xdata2xdata(aff_point_np->aff_xy,
		ptr_comb_table_q + n_loop * lenbyword * 2, lenbyword * 2);

	dma_xdata2xdata(aff_point_np->aff_xy,
		jac_point_np->jac_xyz, lenbyword * 2);
	dma_xdata2xdata((uint32_t *)&ptr_rr_n[3],
		jac_point_np->jac_xyz + lenbyword * 2, lenbyword);

	dma_xdata2rsa(key->pae_ctx, jac_point_np->jac_xyz,
		PAE_RAM_BLOCK01, lenbyword);
	dma_xdata2rsa(key->pae_ctx, jac_point_np->jac_xyz + lenbyword,
		PAE_RAM_BLOCK02, lenbyword);
	dma_xdata2rsa(key->pae_ctx, jac_point_np->jac_xyz + lenbyword * 2,
		PAE_RAM_BLOCK03, lenbyword);
	dma_wr_allblock(key->pae_ctx, 0x00, PAE_RAM_BLOCK0E, lenbyword);
	for (j = 0; j < e; j++)
		ecc_point_dbl(key->pae_ctx, key->param->a_is_minus3);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK01,
		jac_point_np->jac_xyz, lenbyword);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK02,
		jac_point_np->jac_xyz + lenbyword, lenbyword);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK03,
		jac_point_np->jac_xyz + lenbyword * 2, lenbyword);

	ecc_mont2descar_jacobian(key->pae_ctx, jac_point_np, lenbyword);
	ecc_jacob2aff(key->pae_ctx, aff_point_np, jac_point_np, ptr_rr_n,
		lenbyword);
	ecc_descar2mont_affine(key->pae_ctx, aff_point_np, ptr_rr_n, lenbyword);

	dma_xdata2xdata(aff_point_np->aff_xy,
		ptr_comb_table_q_2e + n_loop * lenbyword * 2, lenbyword * 2);
}

/*
 * 函数名称: ecc_precombpoint
 * 函数功能: 固定点点乘的预计算[Ai]P,2^e[Ai]P
 * 			预计算数据存入key->comb_table_q,
 * 			窗口固定为4,点为蒙哥马利域的彷射坐标
 * 输入参数:
 * 			struct str_ecc_key *key,ECC密钥数据结构指针
 * 返回值:
 * 			'RESULT_OK   ' - 函数正确返回
 * 		    'RESULT_ERROR' - 计算出错,返回错误
 */
uint8_t ecc_precombpoint(struct str_ecc_key *key)
{
	struct str_affpoint ecc_comb_dp[3];
	struct str_affpoint aff_point_np;
	struct str_jacpoint jac_point_np;
	struct str_rr_n ptr_rr_n[ECC_RR_N_MAX];
	uint32_t i;
	uint32_t n_value;
	uint32_t lenbyword;

	lenbyword = key->param->bitlen / WORD_BITS;

	if (key->comb_table_q == (uint32_t *)0x00)
		return RESULT_ERROR;

	ecc_precomp_rr(key->pae_ctx, key->param->p, ptr_rr_n, lenbyword);

	if (key->param->a_is_minus3 != 1) {
		dma_xdata2rsa(key->pae_ctx, (uint32_t *)ptr_rr_n, PAE_RAM_BLOCK08,
			lenbyword);
		dma_xdata2rsa(key->pae_ctx, key->param->a, PAE_RAM_BLOCK04,
			lenbyword);
		rsa_adccommand(key->pae_ctx, RSA_MULT, BLOCK06, BLOCK04, BLOCK08);
	}

	ecc_precomp_dp(key->pae_ctx, key->q, ptr_rr_n, key->param->a_is_minus3,
		ecc_comb_dp, lenbyword);

	dma_xdata2xdata(key->q->aff_xy, jac_point_np.jac_xyz, lenbyword * 2);
	for (i = 0; i < 15; i++) {
		n_value = i + 1;
		ecc_precomp_np(key, ptr_rr_n, &aff_point_np, key->q,
			ecc_comb_dp, n_value);
		ecc_precombpoint_loop_pro(key, ptr_rr_n,
			&aff_point_np, &jac_point_np, i, lenbyword);
	}
	return RESULT_OK;
}

/*
 * 函数名称: ecc_pointmult_checkn
 * 函数功能: 计算点乘_二进制算法,检查乘数是否为n,
 * 输入参数:
 * 		struct str_ecc_key *key,ECC乘数指针
 * 		uint8_t * k,乘数缓冲区
 * 		struct str_affpoint *out,积点
 * 返回值: 0-乘数=n, 1-乘数!=n
 */
uint8_t ecc_pointmult_checkn(struct str_ecc_key *key,
		uint8_t *k, struct str_affpoint *out)
{
	uint32_t ecc_temp[ECCMAXWORDLEN];
	uint32_t lenbyword;

	lenbyword = key->param->bitlen / WORD_BITS;

	rsa_write_len(key->pae_ctx, lenbyword);
	dma_xdata2rsa(key->pae_ctx, key->param->n, PAE_RAM_BLOCK0, lenbyword);
	dma_xdata_byte2rsa(key->pae_ctx, k, PAE_RAM_BLOCK1, lenbyword);
	rsa_command(key->pae_ctx, RSA_SUB, BLOCK01, BLOCK01, BLOCK00);
	rsa_command(key->pae_ctx, RSA_SUB, BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK1, ecc_temp, lenbyword);

	if (check_allzero(ecc_temp, lenbyword) == 0x01) {
		dma_wr_allxdata(0x00, out->aff_xy, 2 * lenbyword);
		return 0;
	}

	return 1;
}

/*
 * 函数名称: ecc_pre_multbuf
 * 函数功能: 固定点的点乘乘数缓冲区预处理
 * 输入参数:	uint8_t *ptr_k,乘数缓冲区
 * 			uint8_t *ptr_k_pro,乘数处理结果缓冲区,
 * 			缓冲区大小需要为乘数缓冲区2倍
 * 			uint8_t lenbyword,模的字长度
 */
void ecc_pre_multbuf(uint8_t *ptr_k, uint8_t *ptr_k_pro, uint32_t lenbyword)
{
	uint32_t i;
	uint32_t j;
	uint32_t n_table_len;
	uint8_t  bt_char;

	n_table_len = lenbyword / 2;
	for (i = 0; i < n_table_len; i++) {
		for (j = 0; j < 8; j++) {
			bt_char = 0;
			if ((ptr_k[i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x08;
			if ((ptr_k[n_table_len * 2 + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x04;
			if ((ptr_k[n_table_len * 4 + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x02;
			if ((ptr_k[n_table_len * 6 + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x01;
			ptr_k_pro[16 * i + 2 * j] = bt_char;
			bt_char = 0;
			if ((ptr_k[n_table_len + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x08;
			if ((ptr_k[n_table_len * 3 + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x04;
			if ((ptr_k[n_table_len * 5 + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x02;
			if ((ptr_k[n_table_len * 7 + i] & g_bitset_8[j]) == g_bitset_8[j])
				bt_char += 0x01;
			ptr_k_pro[16 * i + 2 * j + 1] = bt_char;
		}
	}
}

void ecc_comb_mix_add_find_one(struct str_pae *pae_ctx, uint8_t k1, uint8_t k2,
		uint32_t *p_comb_table, uint32_t lenbyword)
{
	uint32_t *ptrcombtable;
	uint32_t *ptrcombtable_2e;
	uint8_t wordlen_of_point;
	uint8_t n_index;

	wordlen_of_point  = lenbyword * 2;
	ptrcombtable    = p_comb_table;
	ptrcombtable_2e = p_comb_table + 30 * lenbyword;
	if (k1 != 0) {
		n_index = k1 - 1;
		dma_xdata2rsa(pae_ctx, ptrcombtable_2e + n_index * wordlen_of_point,
			PAE_RAM_BLOCK04, lenbyword);
		dma_xdata2rsa(pae_ctx,
			ptrcombtable_2e + n_index * wordlen_of_point + lenbyword,
			PAE_RAM_BLOCK05, lenbyword);
		ecc_point_mix_add(pae_ctx);
	}
	if (k2 != 0) {
		n_index = k2 - 1;
		dma_xdata2rsa(pae_ctx, ptrcombtable + n_index * wordlen_of_point,
			PAE_RAM_BLOCK04, lenbyword);
		dma_xdata2rsa(pae_ctx,
			ptrcombtable + n_index * wordlen_of_point + lenbyword,
			PAE_RAM_BLOCK05, lenbyword);
		ecc_point_mix_add(pae_ctx);
	}
}

void ecc_comb_mix_add_no_find_one(struct str_pae *pae_ctx, uint8_t *p_find_one,
		uint8_t k1, uint8_t k2,
		uint32_t *p_comb_table, struct str_jacpoint *jac_point1,
		struct str_rr_n *ptr_rr_n, uint32_t lenbyword)
{
	uint32_t *ptrcombtable;
	uint32_t *ptrcombtable_2e;
	uint8_t wordlen_of_point;
	uint8_t n_index;

	wordlen_of_point = lenbyword * 2;
	ptrcombtable    = p_comb_table;
	ptrcombtable_2e = p_comb_table + 30 * lenbyword;

	if (k1 != 0) {
		n_index = k1 - 1;
		*p_find_one = 1;
		dma_xdata2xdata(ptrcombtable_2e + n_index * wordlen_of_point,
			jac_point1->jac_xyz, lenbyword * 2);

		dma_xdata2xdata((uint32_t *)ptr_rr_n,
			jac_point1->jac_xyz + lenbyword * 2, lenbyword);
		dma_xdata2rsa(pae_ctx, jac_point1->jac_xyz, PAE_RAM_BLOCK01,
			lenbyword);
		dma_xdata2rsa(pae_ctx, jac_point1->jac_xyz + lenbyword,
			PAE_RAM_BLOCK02, lenbyword);
		dma_xdata2rsa(pae_ctx, jac_point1->jac_xyz + 2 * lenbyword,
			PAE_RAM_BLOCK03, lenbyword);
	}
	if (k2 != 0) {
		n_index = k2 - 1;
		if ((*p_find_one) == 0) {
			*p_find_one = 1;
			dma_xdata2xdata(ptrcombtable + n_index * wordlen_of_point,
				jac_point1->jac_xyz, lenbyword * 2);

			dma_xdata2xdata((uint32_t *)ptr_rr_n,
				jac_point1->jac_xyz + lenbyword * 2, lenbyword);

			dma_xdata2rsa(pae_ctx, jac_point1->jac_xyz, PAE_RAM_BLOCK01,
				lenbyword);
			dma_xdata2rsa(pae_ctx, jac_point1->jac_xyz + lenbyword,
				PAE_RAM_BLOCK02, lenbyword);
			dma_xdata2rsa(pae_ctx, jac_point1->jac_xyz + 2 * lenbyword,
				PAE_RAM_BLOCK03, lenbyword);
		} else {
			dma_xdata2rsa(pae_ctx, ptrcombtable + n_index * wordlen_of_point,
				PAE_RAM_BLOCK04, lenbyword);
			dma_xdata2rsa(pae_ctx,
				ptrcombtable + n_index * wordlen_of_point + lenbyword,
				PAE_RAM_BLOCK05, lenbyword);
			ecc_point_mix_add(pae_ctx);
		}
	}
}

/*
 * 函数名称: ecc_pointmult_comb
 * 函数功能: 固定点的点乘,双表固定疏状算法(Comb)
 * 输入参数:
 * 			struct str_ecc_key *key,ECC密钥数据结构指针
 * 			uint32_t *pCombTable,
 * 			uint8_t * k,乘数缓冲区
 * 			struct str_affpoint *out,积点坐标指针
 * 返回值:
 * 			RESULT_ERROR-计算结果为无穷远点或者受到攻击
 * 			RESULT_OK- 计算结果为正常点
 * 			RESULT_INFINITY - 计算结果无穷远点
 */
uint8_t ecc_pointmult_comb(struct str_ecc_key *key, uint32_t *p_comb_table,
		uint8_t *k, struct str_affpoint *out)
{
	struct str_jacpoint jac_point1;
	struct str_rr_n ptr_rr_n[ECC_RR_N_MAX];
	uint8_t ptr_k_pro[ECCMAXLEN * 2];
	uint8_t i;
	uint8_t bt_find_one;
	uint8_t bt_ret;
	uint8_t lenbyword;

	lenbyword = key->param->bitlen / WORD_BITS;

	if (p_comb_table == (uint32_t *)0x00)
		return RESULT_ERROR;

	bt_ret = ecc_pointmult_checkn(key, k, out);
	if (bt_ret == 0)
		return RESULT_INFINITY;

	ecc_pre_multbuf(k, ptr_k_pro, lenbyword);
	ecc_precomp_rr(key->pae_ctx, key->param->p, ptr_rr_n, lenbyword);

	dma_xdata2rsa(key->pae_ctx, (uint32_t *)ptr_rr_n, PAE_RAM_BLOCK08,
		lenbyword);
	dma_xdata2rsa(key->pae_ctx, key->param->a, PAE_RAM_BLOCK04, lenbyword);
	rsa_adccommand(key->pae_ctx, RSA_MULT, BLOCK06, BLOCK04, BLOCK08);
	dma_xdata2rsa(key->pae_ctx, key->param->b, PAE_RAM_BLOCK04, lenbyword);
	rsa_adccommand(key->pae_ctx, RSA_MULT, BLOCK0D, BLOCK04, BLOCK08);
	dma_wr_allblock(key->pae_ctx, 0x00, PAE_RAM_BLOCK0E, lenbyword);

	bt_find_one = 0;
	for (i = 0; i < lenbyword * 4; i++) {
		if (bt_find_one == 1) {
			ecc_point_dbl(key->pae_ctx, key->param->a_is_minus3);
			ecc_comb_mix_add_find_one(key->pae_ctx, ptr_k_pro[i * 2],
				ptr_k_pro[i * 2 + 1], p_comb_table, lenbyword);
		} else {
			ecc_comb_mix_add_no_find_one(key->pae_ctx, &bt_find_one,
				ptr_k_pro[i * 2], ptr_k_pro[i * 2 + 1],
				p_comb_table, &jac_point1,
				&(ptr_rr_n[3]), lenbyword);
		}
	}

	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK01, jac_point1.jac_xyz,
		lenbyword);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK02,
		jac_point1.jac_xyz + lenbyword, lenbyword);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK03,
		jac_point1.jac_xyz + lenbyword * 2, lenbyword);

	ecc_mont2descar_jacobian(key->pae_ctx, &jac_point1, lenbyword);
	bt_ret = ecc_jacob2aff(key->pae_ctx, out, &jac_point1, ptr_rr_n,
		lenbyword);
	return bt_ret;
}
