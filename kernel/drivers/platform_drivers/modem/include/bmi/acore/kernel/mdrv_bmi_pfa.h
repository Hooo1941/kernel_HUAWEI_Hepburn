/**
 *  @brief   飞龙架构2.0 sne模块对外头文件
 *  @file    mdrv_bmi_pfa.h
 *  @version v1.0
 *  @date    2023.08.16
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.08.16|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_PFA_H_
#define _MDRV_BMI_PFA_H_

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int vlan_id_en : 1;
        unsigned int stream_id_en : 1;
        unsigned int userfiledh_en : 1;
        unsigned int userfiledl_en : 1;
        unsigned int reserved : 29;
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} pfa_entry_info_en;

struct pfa_add_entry_info {
    struct net_device *in;
    struct net_device *out;
    unsigned int vlan_id;
    unsigned int stream_id;
    unsigned int userfiled_higher16 : 16;
    unsigned int userfiled_lower16 : 16;
    pfa_entry_info_en en;
};

struct pfa_entry_flow_info {
    int dir;
    unsigned int hkey;
    unsigned char shost[ETH_ALEN];
    unsigned char dhost[ETH_ALEN];
};

struct pfa_del_entry_info {
    unsigned int stream_id;
};

enum pfa_enc_type {
    PFA_ENC_NONE,
    PFA_ENC_NCM_NTB16,
    PFA_ENC_NCM_NTB32,
    PFA_ENC_RNDIS,
    PFA_ENC_WIFI,
    PFA_ENC_PFA_TFT,
    PFA_ENC_CPU,
    PFA_ENC_ACK,
    PFA_ENC_DIRECT_FW_NCM_NTB16,
    PFA_ENC_DIRECT_FW_NCM_NTB32,
    PFA_ENC_DIRECT_FW_RNDIS,
    PFA_ENC_DIRECT_FW_ECM,
    PFA_ENC_ECM,
    PFA_ENC_WIFI_MULTIPLE,
    PFA_ENC_RELAY,
    PFA_ENC_BOTTOM
};

struct pfa_ops {
    void (* pfa_finish_rd)(struct sk_buff *skb, unsigned int len, void* priv, unsigned int net_id);
    void (* pfa_finish_rd_dra)(unsigned long long dra_orig, unsigned int len, unsigned int l2_hdr_offset, void* priv, unsigned int pkt_id);
    void (* pfa_finish_td)(void *param, void *priv);
    void (* pfa_netif_rx)(struct sk_buff *skb, void *priv);
    void (* pfa_complete_rd)(void *priv);
    struct wan_dev_info_s* (*get_wan_info)(struct net_device* dev);
};

struct pfa_port_comm_attr {
    enum pfa_enc_type enc_type;             /* encap type */
    unsigned int td_depth;
    unsigned int rd_depth;
    unsigned int padding_enable;
    unsigned int bypassport;
    unsigned int bypassport_en;
    unsigned int dpa_en;
    struct pfa_ops ops;
    struct net_device *net;
    void *priv;
    unsigned int td_copy_en;
    unsigned int stick_en;
    unsigned int rsv_port_id;
    unsigned int dfs_speed_multiple_en;
    unsigned int dfs_speed_multiple;
    unsigned int port_multiple_en;
    unsigned int direct_fw_en;
};

/**
 * @brief 设置mac地址
 *
 * @par 描述: 为SNE端口设置mac地址
 *
 * @attention
 * <ul><li> NA </li></ul>
 * @param[out]  mac_addr, 申请的mac地址
 * @param[in] portno,SNE端口号
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 *
 * @see NA
 */
int mdrv_pfa_set_macaddr(int portno, const char *mac_addr);

/**
 * @brief 申请SNE端口
 *
 * @par 描述: 申请SNE端口并使端口生效
 *
 * @attention
 * <ul><li> NA </li></ul>
 * @param[out] int ,申请是否成功的标志
 * @param[in] attr,wifi_hac 地址
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 *
 * @see NA
 */
int mdrv_pfa_port_alloc(struct pfa_port_comm_attr *attr);

/**
 * @brief 配置td
 *
 * @par 描述: 检查并配置td
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out] NA
 * @param[in] vir_portno, SNE端口号, dra_l2_addr, MAA中的L2的地址，len,要配置的td的大小，param，用户信息，td_flag，用户的td的标识
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 *
 * @see NA
 */
int mdrv_pfa_config_td(int vir_portno, unsigned long long dra_l2_addr, unsigned int len, void *param, unsigned int td_flags);

/**
 * @brief 释放SNE端口
 *
 * @par 描述: 释放SNE端口
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out] NA
 * @param[in] portno, 要释放的SNE端口号
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 *
 * @see NA
 */
int mdrv_pfa_port_free(int portno);

/**
 * @brief 打开MAC上IP功能
 *
 * @par 描述: 打开MAC上IP功能
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out] NA
 * @param[in] en, 0:关闭；1：打开
 *
 * @retval NA。
 *
 * @see NA
 */
void mdrv_pfa_enable_mac_to_ip(unsigned int en);

int mdrv_pfa_enable_port(int portno);

int mdrv_pfa_disable_port(int portno);

/**
 * @brief 注册回调函数
 *
 * @par 描述: 注册回调函数
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out] NA
 * @param[in] ops, 回调函数
 *
 * @retval NA。
 *
 * @see NA
 */
struct pfa_flow_ops {
    bool (* pfa_rx_filter)(struct sk_buff *skb);
    bool (* pfa_macfw_add_filter)(const unsigned char *mac);
    bool (* pfa_ip_fw_add_filter)(struct sk_buff *skb);
    void (* pfa_add_entry_cb)(struct sk_buff *skb, struct pfa_entry_flow_info *flow_info, struct pfa_add_entry_info *info);
    void (* pfa_del_entry_cb)(struct pfa_del_entry_info *info);
};
void mdrv_pfa_register_flow_cb(struct pfa_flow_ops *ops);

/**
 * @brief 打开padding功能
 *
 * @par
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out] NA
 * @param[in] en, 0:关闭；1：打开
 *
 * @retval NA。
 *
 * @see NA
 */
void mdrv_pfa_enable_pfa_pad(unsigned int en);

void mdrv_pfa_ipfw_del_by_dev(struct net_device *dev);

void mdrv_pfa_ipfw_del_by_tuple(struct nf_conntrack_tuple *tuple);

struct pfa_ipfw_entry_info {
    struct net_device *in;
    struct net_device *out;
};
int mdrv_pfa_ipfw_find_info_by_tuple(struct nf_conntrack_tuple *tuple, struct pfa_ipfw_entry_info *info);

/**
 * @brief 查找mac转发表中是否已存在mac
 *
 * @par
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out] NA
 * @param[in] mac, 需要查找的mac地址
 *
 * @retval true，查找到目的mac; false, 未查到。
 *
 * @see NA
 */
bool mdrv_pfa_macfw_find_mac(const unsigned char *mac);

/**
* @brief 关闭PFA硬转功能 *
* @par 描述:
* 该接口用于关闭PFA硬转功能
* @attention
* <ul><li> NA </li></ul>
* @param[in] void
* @retval void
* @par 依赖:
* <ul><li>mdrv_bmi_pfa.h：该接口声明所在的头文件。</li></ul>
* @see NA
*/
void mdrv_pfa_transfer_pause(void);

/**
 * @brief 打开PFA硬转功能 *
 * @par 描述:
 * 该接口用于打开PFA硬转功能
 * @attention
 * <ul><li> NA </li></ul>
 * @param[in] void
 * @retval void
 * @par 依赖:
 * <ul><li>mdrv_bmi_pfa.h：该接口声明所在的头文件。</li></ul>
 * @see NA
 */
void mdrv_pfa_transfer_restart(void);

/**
 * @brief 更新对应hkey值的表项的stream id *
 * @par 描述:
 * wifi性能优化需求
 * @attention
 * <ul><li> NA </li></ul>
 * @param[in]
 * @retval void
 * @par 依赖:
 * <ul><li>mdrv_bmi_pfa.h：该接口声明所在的头文件。</li></ul>
 * @see NA
 */
int mdrv_pfa_update_streamid_by_hkey(unsigned int hkey, unsigned int stream_id);

#endif
