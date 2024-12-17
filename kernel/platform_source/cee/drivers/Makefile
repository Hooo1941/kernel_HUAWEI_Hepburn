KVER_5_10 = $(shell if [ $(VERSION) -gt 5 ]; then echo "1"; \
                    elif [ $(VERSION) -eq 5 ] && [ $(PATCHLEVEL) -ge 10 ]; then echo "1"; \
                    else echo "0" ; fi)
ifeq ($(KVER_5_10),1)
obj-$(CONFIG_THERMAL_TSENSOR)  += thermal_platform/
endif
obj-$(CONFIG_FILE_BASED_ENCRYPTO) += file_encry/
obj-y += cpumask/
obj-$(CONFIG_HW_VOTE) += hw_vote/
obj-$(CONFIG_PLATFORM_QOS) += qos/
obj-y += dpm/
obj-$(CONFIG_SR)                      += pm/
obj-y += mpam/
obj-y += hck/
obj-y += lpcpu_gic/
obj-y += lpcpu_pmu/
obj-y += cpuidle/
obj-y += lpcpu_timer/
obj-y += cpufreq/lpcpu/
obj-y += cpuidle/
obj-$(CONFIG_CPUID_MIDR_HIDE) += cpu_hide/
obj-y += edac/lpedac/
obj-$(CONFIG_GUEST_GPUFREQ_VOTE) += gpufreq/guest/
obj-$(CONFIG_GPUFREQ) += gpufreq/
obj-$(CONFIG_NPU_PM) += npu_pm/
obj-$(CONFIG_ONSYS_AVS) += onsys_avs/
obj-$(CONFIG_LPM_HKADC) += adc/
obj-$(CONFIG_DDR_PLATFORM_DDRC)   += ddrc/
obj-$(CONFIG_LPMCU_DEBUG) += lpmcu_debug/
obj-$(CONFIG_HOTPLUG_AGENT) += hotplug_agent/
obj-y += freqdump/
obj-$(CONFIG_BUS_MONITOR)+= bus_monitor/
obj-$(CONFIG_BUS_MONITOR_TEST)+= bus_monitor/
obj-$(CONFIG_PM_DEVFREQ)        += devfreq/
obj-$(CONFIG_DRG) += drg/
obj-$(CONFIG_CORESIGHT) += hwtracing/dfx_coresight/
obj-$(CONFIG_FREQ_QOS_DEBUG) += freq_qos_debug/
obj-$(CONFIG_MIPS_MEM_FREQ) += mips_mem/
obj-$(CONFIG_HECA) += heca/
obj-$(CONFIG_PLATFORM_ARM64_AMU) += amu/
obj-$(CONFIG_RTM_DEBUG) += rtm/
