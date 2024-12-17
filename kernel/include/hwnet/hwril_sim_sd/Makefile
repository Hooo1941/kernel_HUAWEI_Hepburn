ifeq (taro, $(findstring taro,$(TARGET_DEVICE)))
obj-y += hwril_sim_sd.o
else
obj-$(CONFIG_SIM_SD_MUX) += hwril_sim_sd.o
endif