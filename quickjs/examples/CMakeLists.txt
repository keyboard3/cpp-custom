# 这个库只做sqrt,通过将生成的Table.h添加到库的源列表中让CMake知道mysqrt.cxx依赖与Table.h
add_library(fib MODULE fib.c)
# 指定SqrtLibrary的include路径从二进制的目录找table.h
target_include_directories(fib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
# 默认是共享库，需要PIC
set_target_properties(fib PROPERTIES POSITION_INDEPENDENT_CODE ON)
SET_TARGET_PROPERTIES(fib PROPERTIES PREFIX "")

# 创建一个imported的库目标（表示已经存在这个库），这个目标名称的有效范围在当前目录下，可以像其他目标一样被引用。
add_library(quickjs STATIC IMPORTED)
# 通过IMPORTED_*来指定这个库的导入信息。IMPORTED_LOCATION指定了库文件在项目之外的磁盘路径下
set_target_properties(quickjs
        PROPERTIES IMPORTED_LOCATION
        "/usr/local/lib/quickjs/libquickjs.a")

target_link_libraries(fib quickjs)