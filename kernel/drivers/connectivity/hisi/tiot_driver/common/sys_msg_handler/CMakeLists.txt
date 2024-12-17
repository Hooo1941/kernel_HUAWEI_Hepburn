set(
    tiot_sys_msg_handler_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_sys_msg_handle.c
)

if(DEFINED CONFIG_DEV_TIOT_SLAVE)
list(
    APPEND tiot_sys_msg_handler_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_sys_msg_slave_handle.c
)
endif()

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "tiot header list" FORCE)
set(TIOT_SRC_LIST "${TIOT_SRC_LIST}" "${tiot_sys_msg_handler_src_list}" CACHE INTERNAL "tiot src list" FORCE)