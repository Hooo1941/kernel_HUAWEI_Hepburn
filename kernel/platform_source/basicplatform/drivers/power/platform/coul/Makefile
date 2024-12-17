obj-$(CONFIG_COUL_DRV)                  += coul_drv.o coul_event.o
obj-$(CONFIG_COUL_TEST)                 += coul_drv_test.o
ifndef CONFIG_COUL_ALGORITHM_CORE
obj-$(CONFIG_COUL_DRV)                  += coul_core/
endif
obj-$(CONFIG_COUL_ALGORITHM_CORE)       += coul_algo/
obj-$(CONFIG_COUL_ALGORITHM_CORE)       += coul_ic/
