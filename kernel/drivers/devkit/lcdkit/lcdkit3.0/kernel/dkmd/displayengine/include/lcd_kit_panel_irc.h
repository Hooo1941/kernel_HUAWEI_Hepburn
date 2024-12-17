

#ifndef __LCD_KIT_PANEL_IRC__
#define __LCD_KIT_PANEL_IRC__

#include <linux/kernel.h>

#include "lcd_kit_common.h"

void lcd_kit_parse_ddic_irc(int panel_id, struct device_node *np);
bool display_engine_set_param_irc(int panel_id, bool ddic_irc_enable);

#endif