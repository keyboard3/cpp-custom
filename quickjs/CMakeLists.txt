cmake_minimum_required(VERSION 3.19)

project(QuickjsDemo)
# 当前程序的include路径
include_directories(/usr/local/include "${PROJECT_BINARY_DIR}")

# 创建一个imported的库目标（表示已经存在这个库），这个目标名称的有效范围在当前目录下，可以像其他目标一样被引用。
add_library(quickjs STATIC IMPORTED)
# 通过IMPORTED_*来指定这个库的导入信息。IMPORTED_LOCATION指定了库文件在项目之外的磁盘路径下
set_target_properties(quickjs
        PROPERTIES IMPORTED_LOCATION
        "/usr/local/lib/quickjs/libquickjs.a")
# 设置fib库是否动态加载还是预加载
set(JS_FIB_DYNAMIC_LOAD ON)
# 配置头文件以及将CMake的设置传递给源代码
configure_file(fibConfig.h.in fibConfig.h)

# 添加要编译的对象
add_subdirectory(examples)
add_executable(hello hello.c)
# 因为test_fib.c中有引用到./examples/fib.c的函数定义，所以引入依赖fib.c
add_executable(test_fib test_fib.c ./examples/fib.c)

# 将库链接到指定的目标（这个目标一定是add_executable() or add_library()创建的）
target_link_libraries(hello quickjs)
target_link_libraries(test_fib quickjs)