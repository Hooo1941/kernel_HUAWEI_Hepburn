#ifndef DUBAI_DISPLAY_STATS_H
#define DUBAI_DISPLAY_STATS_H

#include <chipset_common/dubai/dubai_display.h>

long dubai_ioctl_display(unsigned int cmd, void __user *argp);
void dubai_display_stats_init(void);

int dubai_display_register_ops(struct dubai_display_hist_ops *op);
int dubai_display_unregister_ops(void);

enum {
	RATE_1HZ = 0,
	RATE_5HZ,
	RATE_10HZ,
	RATE_15HZ,
	RATE_24HZ,
	RATE_30HZ,
	RATE_45HZ,
	RATE_60HZ,
	RATE_90HZ,
	RATE_120HZ,
	RATE_180HZ,
};

struct refresh_rate_data {
	uint32_t count_1hz;
	uint32_t count_5hz;
	uint32_t count_10hz;
	uint32_t count_15hz;
	uint32_t count_24hz;
	uint32_t count_30hz;
	uint32_t count_45hz;
	uint32_t count_60hz;
	uint32_t count_90hz;
	uint32_t count_120hz;
	uint32_t count_180hz;
} __packed;

struct refresh_rate_data_packing {
	uint32_t panel_id;
	struct refresh_rate_data refresh_count;
} __packed;

uint64_t calculate_hist_delta(uint64_t a, uint64_t b);
#endif // DUBAI_DISPLAY_STATS_H
