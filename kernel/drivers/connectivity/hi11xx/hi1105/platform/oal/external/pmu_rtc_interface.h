/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pmu_rtc_interface.h header file
 * Author: @CompanyNameTag
 */

#ifndef PMU_RTC_INTERFACE_H
#define PMU_RTC_INTERFACE_H

#ifdef CONFIG_PMU_RTC_READCOUNT
extern unsigned long pmu_rtc_readcount(void);
#define external_pmu_rtc_readcount pmu_rtc_readcount
#elif defined(CONFIG_HISI_PMU_RTC_READCOUNT)
extern unsigned long hisi_pmu_rtc_readcount(void);
#define external_pmu_rtc_readcount hisi_pmu_rtc_readcount
#endif

#endif /* end for __PMU_RTC_INTERFACE_H__ */
