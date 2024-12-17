/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_sms4.c的头文件
 * 作    者 : wifi
 * 创建日期 : 2015年5月20日
 */
#ifndef HMAC_SMS4_H
#define HMAC_SMS4_H

#include "oal_util.h"
#include "wlan_oneimage_define.h"
#include "wlan_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))

static inline uint32_t rotl(uint32_t val, int32_t bits)
{
    return (val << (uint32_t)bits) | (val >> (uint32_t)(NUM_32_BITS - bits)); /* 循环左移 */
}
static inline uint32_t bytesub(uint32_t *sbox, uint32_t mid)
{
    return ((sbox[(((uint32_t)mid) >> BIT_OFFSET_24) & 0xFF] << BIT_OFFSET_24) ^
            (sbox[(((uint32_t)(mid)) >> BIT_OFFSET_16) & 0xFF] << BIT_OFFSET_16) ^
            (sbox[(((uint32_t)mid) >> BIT_OFFSET_8) & 0xFF] << BIT_OFFSET_8) ^ (sbox[((uint32_t)(mid)) & 0xFF]));
}
static inline uint32_t l1(uint32_t value)
{
    return ((value) ^ rotl(value, BIT_OFFSET_2) ^ rotl(value, BIT_OFFSET_10) ^ rotl(value, BIT_OFFSET_18) ^
        rotl(value, BIT_OFFSET_24));
}
static inline uint32_t l2(uint32_t value)
{
    return ((value) ^ rotl(value, BIT_OFFSET_13) ^ rotl(value, BIT_OFFSET_23));
}

void hmac_sms4_crypt(uint8_t *puc_input, uint8_t *puc_output, uint32_t *puc_rk, uint32_t rk_len);
void hmac_sms4_keyext(uint8_t *puc_key, uint32_t *pul_rk, uint32_t rk_len);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_wapi_sms4.h */
