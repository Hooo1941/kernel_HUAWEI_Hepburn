/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : UAPSD hmac层处理
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#include "oal_ext_if.h"
#include "oal_kernel_file.h"
#include "securec.h"
#include "hmac_thread.h"
#include "hmac_hcc_adapt.h"
#include "hmac_blockack.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SYSFS_STAT_C

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
/* debug sysfs */
OAL_STATIC oal_kobject *g_conn_syfs_hmac_object = NULL;

static int32_t hmac_encap_print_vap_stat_buf(hmac_vap_stru *hmac_vap, char *buf, int32_t buf_len, uint8_t vap_id)
{
    oal_net_device_stru *net_device = NULL;
    uint32_t i;
    int32_t ret;
    int32_t offset = 0;
    struct netdev_queue *txq = NULL;
    ret = snprintf_s(buf + offset, buf_len - offset, (buf_len - offset) - 1, "vap %2u info:\n", vap_id);
    if (ret > 0) {
        offset += ret;
    }
    ret = snprintf_s(buf + offset, buf_len - offset, (buf_len - offset) - 1,
        "vap_state %2u, protocol:%2u, user nums:%2u,init:%u\n",
        hmac_vap->st_vap_base_info.en_vap_state, hmac_vap->st_vap_base_info.en_protocol,
        hmac_vap->st_vap_base_info.us_user_nums, hmac_vap->st_vap_base_info.uc_init_flag);
    if (ret > 0) {
        offset += ret;
    }
    net_device = hmac_vap->pst_net_device;
    if (net_device != NULL) {
        ret = snprintf_s(buf + offset, buf_len - offset, (buf_len - offset) - 1,
            "net name:%s\n", netdev_name(net_device));
        if (ret > 0) {
            offset += ret;
        }
        ret = snprintf_s(buf + offset, buf_len - offset, (buf_len - offset) - 1,
            "tx [%u]queues info, state [bit0:DRV_OFF], [bit1:STACK_OFF], [bit2:FROZEN]\n",
            net_device->num_tx_queues);
        if (ret > 0) {
            offset += ret;
        }
        for (i = 0; i < net_device->num_tx_queues; i++) {
            txq = netdev_get_tx_queue(net_device, i);
            if (!txq->state) {
                continue;
            }
            ret = snprintf_s(buf + offset, buf_len - offset, (buf_len - offset) - 1,
                "net queue[%2u]'s state:0x%lx\n", i, txq->state);
            if (ret > 0) {
                offset += ret;
            }
        }
    }
    ret = snprintf_s(buf + offset, buf_len - offset, (buf_len - offset) - 1, "\n");
    if (ret > 0) {
        offset += ret;
    }
    return offset;
}

int32_t hmac_print_vap_stat(void *data, char *buf, int32_t buf_len)
{
    int32_t offset = 0;
    uint8_t uc_vap_id;
    hmac_vap_stru *pst_hmac_vap = NULL;
    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == NULL) {
            continue;
        }
        offset += hmac_encap_print_vap_stat_buf(pst_hmac_vap, buf + offset, buf_len - offset, uc_vap_id);
    }

    return offset;
}

OAL_STATIC ssize_t hmac_get_vap_stat(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (oal_any_null_ptr3(buf, attr, dev)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += hmac_print_vap_stat(NULL, buf, PAGE_SIZE - ret);
    return ret;
}

OAL_STATIC ssize_t hmac_get_adapt_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (oal_any_null_ptr3(buf, attr, dev)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += hmac_tx_event_pkts_info_print(NULL, buf + ret, PAGE_SIZE - ret);
    return ret;
}

int32_t hmac_wakelock_info_print(char *buf, int32_t buf_len)
{
    int32_t ret = 0;

#ifdef CONFIG_PRINTK
    if (g_st_hmac_wakelock.locked_addr) {
        ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1, "wakelocked by:%pf\n",
                          (void *)g_st_hmac_wakelock.locked_addr);
    }
#endif

    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "hold %lu locks\n", g_st_hmac_wakelock.lock_count);

    return ret;
}

OAL_STATIC ssize_t hmac_get_wakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (oal_any_null_ptr3(buf, attr, dev)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += hmac_wakelock_info_print(buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hmac_show_roam_status(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int32_t ret = 0;
    uint8_t uc_vap_id;
    uint8_t uc_roming_now = 0;
    mac_vap_stru *pst_mac_vap = NULL;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru *pst_hmac_user_multi;
#endif
    hmac_user_stru *pst_hmac_user = NULL;

    if (buf == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{buf is NULL.}");
        return ret;
    }

    if ((dev == NULL) || (attr == NULL)) {
        ret += snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "roam_status=0\n");

        return ret;
    }

    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
        if (pst_mac_vap == NULL) {
            continue;
        }

        if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT)) {
            continue;
        }

        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
            uc_roming_now = 1;
            break;
        }

#ifdef _PRE_WLAN_FEATURE_WAPI
        /* wapi下，将roam标志置为1，防止arp探测 */
        pst_hmac_user_multi = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_multi_user_idx);
        if (pst_hmac_user_multi == NULL) {
            continue;
        }

        if (pst_hmac_user_multi->st_wapi.uc_port_valid == OAL_TRUE) {
            uc_roming_now = 1;
            break;
        }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

        pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_assoc_vap_id);
        if (pst_hmac_user == NULL) {
            continue;
        }

        if (OAL_TRUE == hmac_btcoex_check_by_ba_size(pst_hmac_user)) {
            uc_roming_now = 1;
        }
    }
    /* 先出一个版本强制关闭arp探测，测试下效果 */
    ret += snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "roam_status=%1ud\n", uc_roming_now);

    return ret;
}

OAL_STATIC oal_bool_enum_uint8 hmac_rxthread_input_param_check(struct kobject *dev, struct kobj_attribute *attr,
    const char *buf)
{
    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return OAL_FALSE;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return OAL_FALSE;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return OAL_FALSE;
    }
    return OAL_TRUE;
}
OAL_STATIC ssize_t hmac_set_rxthread_enable(struct kobject *dev, struct kobj_attribute *attr,
                                            const char *buf, size_t count)
{
    uint32_t val;
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();

    if (hmac_rxthread_input_param_check(dev, attr, buf) == OAL_FALSE) {
        return 0;
    }

    if ((sscanf_s(buf, "%u", &val) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }

    rxdata_thread->en_rxthread_enable = (oal_bool_enum_uint8)val;

    return count;
}
OAL_STATIC ssize_t hmac_get_rxthread_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();

    if (hmac_rxthread_input_param_check(dev, attr, buf) == OAL_FALSE) {
        return 0;
    }

    ret += snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
                      "rxthread_enable=%u\nrxthread_queue_len=%u\nrxthread_pkt_loss=%u\n",
                      rxdata_thread->en_rxthread_enable,
                      oal_netbuf_list_len(&rxdata_thread->st_rxdata_netbuf_head),
                      rxdata_thread->pkt_loss_cnt);

    return ret;
}
#ifndef _PRE_WINDOWS_SUPPORT
OAL_STATIC struct kobj_attribute g_dev_attr_vap_info =
    __ATTR(vap_info, S_IRUGO, hmac_get_vap_stat, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_adapt_info =
    __ATTR(adapt_info, S_IRUGO, hmac_get_adapt_info, NULL);
static struct kobj_attribute g_dev_attr_wakelock =
    __ATTR(wakelock, S_IRUGO, hmac_get_wakelock_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_roam_status =
    __ATTR(roam_status, S_IRUGO, hmac_show_roam_status, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_rxdata_info =
    __ATTR(rxdata_info, S_IRUGO | S_IWUSR, hmac_get_rxthread_info, hmac_set_rxthread_enable);
#endif

OAL_STATIC struct attribute *g_hmac_sysfs_entries[] = {
    &g_dev_attr_vap_info.attr,
    &g_dev_attr_adapt_info.attr,
    &g_dev_attr_wakelock.attr,
    &g_dev_attr_roam_status.attr,
    &g_dev_attr_rxdata_info.attr,
    NULL
};

OAL_STATIC struct attribute_group g_hmac_attribute_group = {
    .name = "vap",
    .attrs = g_hmac_sysfs_entries,
};

int32_t hmac_sysfs_entry_init(void)
{
    int32_t ret;
    oal_kobject *pst_root_object = NULL;
    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sysfs_entry_init::get sysfs root object failed!}");
        return -OAL_EFAIL;
    }

    g_conn_syfs_hmac_object = kobject_create_and_add("hmac", pst_root_object);
    if (g_conn_syfs_hmac_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sysfs_entry_init::create hmac object failed!}");
        return -OAL_EFAIL;
    }

    ret = sysfs_create_group(g_conn_syfs_hmac_object, &g_hmac_attribute_group);
    if (ret) {
        kobject_put(g_conn_syfs_hmac_object);
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sysfs_entry_init::sysfs create group failed!}");
        return ret;
    }
    return OAL_SUCC;
}

int32_t hmac_sysfs_entry_exit(void)
{
    if (g_conn_syfs_hmac_object) {
        sysfs_remove_group(g_conn_syfs_hmac_object, &g_hmac_attribute_group);
        kobject_put(g_conn_syfs_hmac_object);
    }
    return OAL_SUCC;
}
#endif
