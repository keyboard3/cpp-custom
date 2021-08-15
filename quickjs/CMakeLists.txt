cmake_minimum_required(VERSION 3.10)
# 约定 runtime 为最终生成的可执行文件
project(runtime)
add_executable(runtime main.cpp)

# 导入 QuickJS 的头文件和库文件
include_directories(/usr/local/include)
add_library(quickjs STATIC IMPORTED)
set_target_properties(quickjs
        PROPERTIES IMPORTED_LOCATION
        "/usr/local/lib/quickjs/libquickjs.a")

# 将 QuickJS 链接到 runtime
target_link_libraries(runtime
        quickjs)