obj-$(CONFIG_ARCH_PLATFORM) += hck_lpcpu_idle.o
obj-$(CONFIG_ARCH_PLATFORM) += lpcpu_cpuidle_lp_mode.o
obj-$(CONFIG_ARCH_PLATFORM) += lpcpu_cpuidle_menu.o
obj-$(CONFIG_LPCPU_MULTIDRV_CPUIDLE) += lpcpu-multidrv-cpuidle.o
obj-$(CONFIG_LPCPU_IDLE_SLEEP) += lpcpu-idle-sleep.o
obj-y += lpcpu_governors/

ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow
ccflags-y += -I$(srctree)/drivers/cpuidle/
