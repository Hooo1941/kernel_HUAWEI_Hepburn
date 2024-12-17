set(
    tiot_packet_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_packet_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_packet.c
)

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "tiot header list" FORCE)
set(TIOT_SRC_LIST "${TIOT_SRC_LIST}" "${tiot_packet_src_list}" CACHE INTERNAL "tiot src list" FORCE)