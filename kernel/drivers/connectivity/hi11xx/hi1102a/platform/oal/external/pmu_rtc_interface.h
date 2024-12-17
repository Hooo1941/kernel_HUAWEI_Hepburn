/*
 * 版权所有 (c) 华为技术有限公司 2021-2021
 * 功能说明   : pmu_rtc_interface.h
 * 作者       :
 * 创建日期   : 2021年1月24日
 */
#ifndef __PMU_RTC_INTERFACE_H__
#define __PMU_RTC_INTERFACE_H__

#ifdef CONFIG_PMU_RTC_READCOUNT
extern unsigned long pmu_rtc_readcount(void);
#define external_pmu_rtc_readcount pmu_rtc_readcount
#elif defined(CONFIG_HISI_PMU_RTC_READCOUNT)
extern unsigned long hisi_pmu_rtc_readcount(void);
#define external_pmu_rtc_readcount hisi_pmu_rtc_readcount
#endif

#endif /* end for __PMU_RTC_INTERFACE_H__ */