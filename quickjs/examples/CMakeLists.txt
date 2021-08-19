# MODULE 构建动态装载库，不会进行链接过程，在运行时可以通过dlopen,dlsym通过符号
# SHARED 动态链接库，符号重定位过程在由装载的时候（动态链接器）
add_library(fib MODULE fib.c)

# -fPIC 地址无关代码，实际调用时通过dlsym查找库的GOT表来定位到实际内存地址
# -shared 输出的对象只使用装载时重定位
set_target_properties(fib PROPERTIES POSITION_INDEPENDENT_CODE ON
                      PREFIX "" # 去掉libfib.so前缀lib
                      )
# 编译fib时链接到scope中定义过的quickjs
target_link_libraries(fib quickjs)