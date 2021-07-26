cmake_minimum_required(VERSION 3.19)

# set the project name and version
project(custom_libuv VERSION 1.0)

include_directories(/usr/local/include)
link_directories(/usr/local/Cellar/libuv/HEAD-bd7fcf1/lib)
link_libraries(uv)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile source
add_executable(hello ./hello.cpp)
add_executable(idle ./idle.cpp)
add_executable(fs-cat ./fs-cat.cpp)