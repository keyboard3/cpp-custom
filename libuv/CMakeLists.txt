cmake_minimum_required(VERSION 3.19)

# set the project name and version
project(custom_libuv VERSION 1.0)

include_directories(/usr/local/include)
link_directories(/usr/local/Cellar/libuv//1.41.1/lib)
link_libraries(uv)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile source
add_executable(hello ./hello.cpp)
add_executable(idle ./idle.cpp)
add_executable(fscat ./fscat.cpp)
add_executable(tcpServer ./tcp-echo-server.cpp)
add_executable(tcpClient ./tcp-echo-client.cpp)
add_executable(dhcp ./udp-dhcp.cpp)
add_executable(dns ./dns.cpp)