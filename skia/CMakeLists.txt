cmake_minimum_required(VERSION 3.19)

# 设置项目名和版本号
project(SkiaDemo VERSION 1.0)

# 指定c++标准库17,确保CMAKE_CXX_STANDARD声明是在add_executable之上声明
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(heptagram heptagram.cpp)

# 将binary dir添加到include files的搜索路径上，所以我们可以找到TutorialConfig.h
include_directories(.)

# 获得libskia文件的绝对路径
get_filename_component(LIB_SKIA_PATH ./lib/libskia.a ABSOLUTE)
# 创建一个imported的库目标（表示已经存在这个库），这个目标名称的有效范围在当前目录下，可以像其他目标一样被引用。
add_library(libskia STATIC IMPORTED)
# 通过IMPORTED_*来指定这个库的导入信息。IMPORTED_LOCATION指定了库文件在项目之外的磁盘路径下
set_target_properties(libskia
        PROPERTIES IMPORTED_LOCATION
        ${LIB_SKIA_PATH})

target_link_libraries(heptagram 
                      libskia
                      "-framework CoreFoundation"
                      "-framework CoreGraphics"
                      "-framework CoreText"
                      "-framework CoreServices"
                      )