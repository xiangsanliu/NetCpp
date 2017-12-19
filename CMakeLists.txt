cmake_minimum_required(VERSION 3.8)
project(NetCpp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp get_device_mac.h crc_check.h tcp_cs.h ping_and_tracert.h tool.h dns_server.h)
set(TARGET_LIB
        "C:\\\\Program Files\\\\MinGW\\\\lib\\\\libnetapi32.a"
        "C:\\\\Program Files\\\\MinGW\\\\lib\\\\libiphlpapi.a"
        "C:\\\\Program Files\\\\MinGW\\\\lib\\\\libws2_32.a")
add_executable(NetCpp ${SOURCE_FILES})
target_link_libraries(NetCpp ${TARGET_LIB})