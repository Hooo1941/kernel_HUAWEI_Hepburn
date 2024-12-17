
obj-y += cee/
obj-y += basicplatform/
obj-y += see/
obj-y += smart/
obj-y += ai/
obj-y += ffrt/
# maligpu/ comes after char for AGP vs DRM startup and after iommu in driver/
obj-y += maligpu/
obj-y += audio/
obj-y += camera/
obj-y += display/
obj-y += isp/
obj-y += vcodec/
# hisecally
ifeq ($(or $(CONFIG_ATF_FUZZ),$(CONFIG_RDA_SANITIZER)),y)
obj-y += hisecally/
endif
