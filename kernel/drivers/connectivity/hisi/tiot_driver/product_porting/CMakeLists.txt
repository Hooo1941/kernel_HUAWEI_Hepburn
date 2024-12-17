#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
add_subdirectory(porting)  # 替换成对应porting目录

set(
    tiot_product_header_list
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${tiot_product_header_list}" CACHE INTERNAL "tiot header list" FORCE)