cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(custom_libuv VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile source
add_executable(tcp-client ./tcp-socket-client.cpp)
add_executable(tcp-server ./tcp-socket-server.cpp)
add_executable(tcp-server-process ./tcp-socket-server-process.cpp)
add_executable(tcp-server-thread ./tcp-socket-server-thread.cpp)
add_executable(tcp-server-select ./tcp-socket-server-select.cpp)
add_executable(tcp-server-kqueue ./tcp-socket-server-kqueue.cpp)
add_executable(udp-client ./udp-socket-client.cpp)
add_executable(udp-server ./udp-socket-server.cpp)

