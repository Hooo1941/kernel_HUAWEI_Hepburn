#ifndef __HIDPRX_CTRL_H__
#define __HIDPRX_CTRL_H__
#include "hidprx.h"
#include "hidprx_sdp.h"

void dprx_ctrl_interface_init(struct rx_interface *impl);
int dprx_ctrl_get_timinginfo(struct rx_interface *rx, void __user *argp);
int dprx_ctrl_enable_vactive_start(struct rx_interface *rx, void __user *argp);
int dprx_ctrl_wait_vactive_start(struct rx_interface *rx, void __user *argp);
void dprx_phy_set_resistance_high_state(struct dprx_ctrl *dprx);
void dprx_phy_ctrl_reg_resistance_high(struct dprx_ctrl *dprx);
#endif