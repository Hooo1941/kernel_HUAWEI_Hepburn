#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
add_subdirectory(controller)
add_subdirectory(fw_load)
add_subdirectory(interface)
add_subdirectory(libboundscheck)
add_subdirectory(pm)
add_subdirectory(transfer)
add_subdirectory(sys_msg_handler)
add_subdirectory(utils)

if(DEFINED CONFIG_COMMON_SUPPORT_DEBUG)
add_subdirectory_if_exist(debug)
endif()

if(DEFINED CONFIG_COMMON_SUPPORT_DFR)
add_subdirectory_if_exist(dfr)
endif()
