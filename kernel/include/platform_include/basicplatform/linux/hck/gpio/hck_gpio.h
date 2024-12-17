#ifndef __HCK_GPIO__
#define __HCK_GPIO__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
struct list_head;

DECLARE_HCK_VH(gpio_find_base,
	TP_PROTO(struct list_head *gpio_devices, int ngpio, int *base),
	TP_ARGS(gpio_devices, ngpio, base));
DECLARE_HCK_VH(get_gpio_id,
	TP_PROTO(struct ida *gpio_ida, int base, int ngpio, int *id),
	TP_ARGS(gpio_ida, base, ngpio, id));
#endif /* end of hck gpio */
