# CMakeList.txt: tcp-ip 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# TODO: 如有需要，请添加测试并安装目标。

# 设置工程的名字，生成的可执行文件也将使用该名称，如net.exe
project(tcp-ip)
set(CMAKE_C_STANDARD 99)

LINK_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/npcap/Lib/x64          # win64使用
    #${PROJECT_SOURCE_DIR}/lib/npcap/Lib/             # win32使用
)

# 给visual studio编译器使用的宏，Enable GCC debug
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_compile_options(-g)

# 头文件搜索路径，使得c文件中可以直接使用#include "xxx.h"，而不必加前缀
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/npcap/Include          # pcap路径
        ${PROJECT_SOURCE_DIR}/src/net/net 
        ${PROJECT_SOURCE_DIR}/src/plat
        ${PROJECT_SOURCE_DIR}/src/app
)

# 搜索src下面的所有源文件，生成文件列表
file(GLOB_RECURSE SOURCE_LIST "src/*.cpp" "src/*.h")

# 将源文件列表加入到工程中
add_executable(${PROJECT_NAME} ${SOURCE_LIST} "src/plat/sys_plat.cpp" "src/app/echo/tcp_echo_client.h" "src/app/echo/tcp_echo_client.cpp" "src/app/echo/tcp_echo_server.h" "src/app/echo/tcp_echo_server.cpp" "src/net/net/net.h" "src/net/src/net.cpp" "src/net/net/net_err.h" "src/net/net/exmsg.h" "src/net/src/exmsg.cpp" "src/plat/net_plat.h" "src/plat/net_plat.cpp" "src/plat/netif_pcap.h" "src/plat/netif_pcap.cpp" "src/net/net/dbg.h" "src/net/src/dbg.cpp")

# 与一些库进行编译链接，生成最终的程序
# 针对网络的平台配置
add_definitions(-DNET_DRIVER_PCAP)    # use pcap

message(STATUS "current platform: ${CMAKE_HOST_SYSTEM_NAME}")
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-DSYS_PLAT_WINDOWS)
    target_link_libraries(${PROJECT_NAME} wpcap packet Ws2_32)
else()
    # Linux和Mac上的特定配置
    add_definitions(-DSYS_PLAT_LINUX)
    target_link_libraries(${PROJECT_NAME} pthread pcap)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tcp-ip PROPERTY CXX_STANDARD 20)
endif()