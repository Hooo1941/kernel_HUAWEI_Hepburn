#include <linux/netfilter/nf_conntrack_common.h>
#include <hooks/reg/iot_hook_pfa.h>
#include "pfa.h"
#include "pfa_dbg.h"
#include "pfa_core.h"
#include "pfa_ip_entry.h"

void mdrv_pfa_enable_pfa_pad(unsigned int en)
{
    struct pfa *pfa = &g_pfa;
    unsigned int value;

    value = en == 0 ? 0 : 1;
    pfa_writel(pfa->regs, PFA_GMAC_PADDING_IDEN_EN, value);
}

void mdrv_pfa_register_flow_cb(struct pfa_flow_ops *ops)
{
    struct pfa *pfa = &g_pfa;

    pfa->ops.pfa_rx_filter = ops->pfa_rx_filter;
    pfa->ops.pfa_macfw_add_filter = ops->pfa_macfw_add_filter;
    pfa->ops.pfa_ip_fw_add_filter = ops->pfa_ip_fw_add_filter;
    pfa->ops.pfa_add_entry_cb = ops->pfa_add_entry_cb;
    pfa->ops.pfa_del_entry_cb = ops->pfa_del_entry_cb;
}

void mdrv_pfa_enable_mac_to_ip(unsigned int en)
{
    int i;

    for (i = 1; i < PFA_CCORE_PORT_BEGIN; i++) {
        bsp_pfa_enable_macfw_to_ip(i, en);
    }
}

void mdrv_pfa_transfer_pause(void)
{
    pfa_transfer_pause();
}

void mdrv_pfa_transfer_restart(void)
{
    pfa_transfer_restart();
}

EXPORT_SYMBOL(mdrv_pfa_enable_mac_to_ip);
EXPORT_SYMBOL(mdrv_pfa_register_flow_cb);
EXPORT_SYMBOL(mdrv_pfa_enable_pfa_pad);
EXPORT_SYMBOL(mdrv_pfa_ipfw_del_by_dev);
EXPORT_SYMBOL(mdrv_pfa_ipfw_del_by_tuple);
EXPORT_SYMBOL(mdrv_pfa_ipfw_find_info_by_tuple);
EXPORT_SYMBOL(mdrv_pfa_transfer_pause);
EXPORT_SYMBOL(mdrv_pfa_transfer_restart);