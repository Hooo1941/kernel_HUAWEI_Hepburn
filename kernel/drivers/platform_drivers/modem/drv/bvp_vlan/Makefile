-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_FEATURE_VLAN)),YES)
obj-y += bvp_vlan.o
else
obj-y += bvp_vlan_adapter.o
endif