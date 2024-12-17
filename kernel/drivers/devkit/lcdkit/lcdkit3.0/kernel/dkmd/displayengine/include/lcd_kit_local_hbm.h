

#ifndef __LCD_KIT_LOCAL_HBM__
#define __LCD_KIT_LOCAL_HBM__

#include <linux/kernel.h>

#include "lcd_kit_common.h"
#include "display_engine_kernel.h"

void display_engine_local_hbm_init(void);
int display_engine_brightness_update(unsigned int bl_level);
int display_engine_local_hbm_set(int grayscale);
uint32_t display_engine_local_hbm_get_support(void);
void lcd_kit_local_hbm_cmds_parse(int panel_id, struct device_node *np);
void display_engine_local_hbm_workqueue_handler(struct kthread_work *work);
unsigned int display_engine_get_hbm_bl(int panel_id, unsigned int bl_level);
int display_engine_set_param_aplha_map(int panel_id,
	struct display_engine_alpha_map *input_alpha_map);
int display_engine_ddic_alpha_is_on_for_c08(int panel_id, void *hld, uint32_t alpha, bool is_on);
void display_engine_circle_cmds_set_payload_for_c08(
	int panel_id, int index, int payload_num, char value, bool is_on);
int display_engine_fake_off_handle(int panel_id);
#endif
