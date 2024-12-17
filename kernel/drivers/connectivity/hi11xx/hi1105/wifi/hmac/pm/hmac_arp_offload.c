/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : ARP Offloading相关函数实现
 * 作    者 : wifi
 * 创建日期 : 2015年5月14日
 */
#include "frw_ext_if.h"
#include "hmac_arp_offload.h"
#include "hmac_vap.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ARP_OFFLOAD_C

#ifdef _PRE_WLAN_FEATURE_PWL
void hmac_pwl_set_arp_probe_en(mac_vap_stru *mac_vap, dmac_ip_oper_enum_uint8 oper)
{
    // mac_vap 外层已经做过判空
    if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }
    if (mac_vap->pwl_enable == OAL_FALSE) {
        return;
    }
    if (oper == DMAC_IP_ADDR_ADD) {
        hmac_arp_probe_type_set(mac_vap, OAL_TRUE, HMAC_VAP_ARP_PROBE_TYPE_PWL); // 添加端口时 PWL 开启ARP探测
    } else if (oper == DMAC_IP_ADDR_DEL) {
        hmac_arp_probe_type_set(mac_vap, OAL_FALSE, HMAC_VAP_ARP_PROBE_TYPE_PWL); // 删除端口时 PWL 关闭ARP探测
    }
}
#endif
/*
 * 功能描述  : 网络接口UP/DOWN时，增加/删除
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_arp_offload_set_ip_addr(mac_vap_stru *mac_vap,
    dmac_ip_type_enum_uint8 type, dmac_ip_oper_enum_uint8 oper,
    const void *ip_addr, const void *mask_addr)
{
    dmac_ip_addr_config_stru ip_addr_config;
    uint32_t ret1;
    int32_t ret2 = EOK;

    if (oal_unlikely(oal_any_null_ptr3(mac_vap, ip_addr, mask_addr))) {
        oam_error_log0(0, OAM_SF_PWR,
            "{hmac_arp_offload_set_ip_addr::The mac_vap or ip_addr or mask_addr is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&ip_addr_config, sizeof(dmac_ip_addr_config_stru), 0, sizeof(dmac_ip_addr_config_stru));

    if (type == DMAC_CONFIG_IPV4) {
        ret2 += memcpy_s(ip_addr_config.auc_ip_addr, OAL_IP_ADDR_MAX_SIZE, ip_addr, OAL_IPV4_ADDR_SIZE);
        ret2 += memcpy_s(ip_addr_config.auc_mask_addr, OAL_IP_ADDR_MAX_SIZE, mask_addr, OAL_IPV4_ADDR_SIZE);
    } else if (type == DMAC_CONFIG_IPV6) {
        ret2 += memcpy_s(ip_addr_config.auc_ip_addr, OAL_IP_ADDR_MAX_SIZE, ip_addr, OAL_IPV6_ADDR_SIZE);
    } else {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_arp_offload_set_ip_addr::The type[%d] is error.}", type);
        return OAL_ERR_CODE_MAGIC_NUM_FAIL;
    }
    if (ret2 != EOK) {
        oam_error_log0(0, OAM_SF_PWR, "hmac_arp_offload_set_ip_addr::memcpy fail!");
        return OAL_FAIL;
    }

    ip_addr_config.en_type = type;

    if (oal_unlikely(oper >= DMAC_IP_OPER_BUTT)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_arp_offload_set_ip_addr::The operation[%d] is error.}", oper);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ip_addr_config.en_oper = oper;
#ifdef _PRE_WLAN_FEATURE_PWL
    hmac_pwl_set_arp_probe_en(mac_vap, oper);
#endif

    ret1 = hmac_config_set_ip_addr(mac_vap,
                                   sizeof(dmac_ip_addr_config_stru),
                                   (uint8_t *)&ip_addr_config);
    return ret1;
}
