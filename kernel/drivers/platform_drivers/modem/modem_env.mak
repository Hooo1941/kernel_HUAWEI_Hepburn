GBB_CUST_NAME ?= $(gbb_cust_name)

ifeq ($(chip_type),)
    GBB_CHIP_TYPE := cs
else
    GBB_CHIP_TYPE := $(chip_type)
endif

ifeq ($(GBB_CUST_NAME),)
    ifeq ($(TARGET_BUILD_VARIANT)_$(OBB_PRODUCT_NAME),user_charlotte)
        tmp_software_type := $(TARGET_BUILD_VARIANT)
    else ifeq ($(cust_config),)
        tmp_software_type := eng
    else ifeq ($(cust_config),cust_modem_user)
        tmp_software_type := user
    else ifeq ($(cust_config),cust_modem_asan)
        tmp_software_type := asan
    else
        $(error cust_config error)
    endif

    GBB_CUST_NAME := $(GBB_CHIP_TYPE)_$(tmp_software_type)
    tmp_software_type :=
endif

export CUST_NAME := $(GBB_CUST_NAME)
export GBB_CUST_NAME
export GBB_CHIP_TYPE
export GBB_PRODUCT_NAME := $(OBB_PRODUCT_NAME)

$(if $(GBB_PRODUCT_NAME),,$(error OBB_PRODUCT_NAME is NULL))
