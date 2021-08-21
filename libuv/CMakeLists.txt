cmake_minimum_required(VERSION 3.19)

# set the project name and version
project(custom_libuv VERSION 1.0)

include_directories(./lib)
link_directories(./lib)

add_subdirectory(plugin_lib)

# 添加libuv库
add_library(uv STATIC IMPORTED)
set_target_properties(uv
                PROPERTIES IMPORTED_LOCATION
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/libuv.a)
link_libraries(uv)
link_libraries(curl)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile source
add_executable(idle ./idle.cpp)
add_executable(fscat ./fscat.cpp)
add_executable(tcpServer ./tcp-echo-server.cpp)
add_executable(tcpClient ./tcp-echo-client.cpp)
add_executable(dhcp ./udp-dhcp.cpp)
add_executable(dns ./dns.cpp)
add_executable(interfaces ./interfaces.cpp)
add_executable(timer ./ref-timer.cpp)
add_executable(idleCompute ./idle-compute.cpp)
add_executable(uvwget ./uvwget.cpp)
add_executable(plugin ./plugin.cpp)
add_executable(tty ./tty.cpp)
add_executable(ttyGravity ./tty-gravity.cpp)