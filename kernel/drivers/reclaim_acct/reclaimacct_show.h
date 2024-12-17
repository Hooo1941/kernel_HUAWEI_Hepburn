 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
  * Description: Show memory reclaim delay and reclaim efficiency.
  * Author: Gong Chen <gongchen4@huawei.com>
  * Create: 2021-12-12
  */
#ifndef _RECLAIMACCT_SHOW_
#define _RECLAIMACCT_SHOW_

#include <linux/types.h>

bool reclaimacct_initialize_show_data(void);
void reclaimacct_reinitialize_show_data(void);
void reclaimacct_destroy_show_data(void);

void reclaimacct_collect_data(void);
void reclaimacct_collect_reclaim_efficiency(void);

#endif