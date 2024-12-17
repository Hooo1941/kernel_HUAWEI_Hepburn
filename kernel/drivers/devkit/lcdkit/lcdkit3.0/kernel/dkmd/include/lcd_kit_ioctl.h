
#include "display_engine_interface.h"
#include "display_engine_kernel.h"

#ifndef __LCDKIT_IOCTL_H__
#define __LCDKIT_IOCTL_H__

enum {
	LCDKIT_IOCTL_DISPLAY_ENGINE_PARAM_GET = 0x20,
	LCDKIT_IOCTL_DISPLAY_ENGINE_PARAM_SET,
};
#define LCDKIT_IOCTL_MAGIC 'L'
#define DISPLAY_ENGINE_GET_PARAM _IOW(LCDKIT_IOCTL_MAGIC, LCDKIT_IOCTL_DISPLAY_ENGINE_PARAM_GET, struct display_engine_param)
#define DISPLAY_ENGINE_SET_PARAM _IOW(LCDKIT_IOCTL_MAGIC, LCDKIT_IOCTL_DISPLAY_ENGINE_PARAM_SET, struct display_engine_param)

#endif