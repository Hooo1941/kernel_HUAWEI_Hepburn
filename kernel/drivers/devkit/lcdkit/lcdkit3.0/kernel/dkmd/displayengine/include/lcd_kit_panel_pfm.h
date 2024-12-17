

#ifndef __LCD_KIT_PANEL_PFM__
#define __LCD_KIT_PANEL_PFM__

#include <linux/kernel.h>

#include "lcd_kit_common.h"

void lcd_kit_parse_panel_pfm(int panel_id, struct device_node *np);
void lcd_kit_bl_pfm_ctrl(int panel_id, unsigned int bl_level, unsigned int current_fps);
void lcd_kit_fps_pfm_ctrl(int panel_id, unsigned int current_fps);

#endif