# MODULE 构建动态装载库，不会进行链接过程，在运行时可以通过dlopen,dlsym通过符号
# SHARED 动态链接库，符号重定位过程在由装载的时候（动态链接器）
add_library(hello MODULE hello.cpp)

# 为了指示 OS X 链接器允许未解析的符号，-undefined dynamic_lookup 添加到链接器选项
# linux 上只需要 -shared
set_target_properties(hello PROPERTIES LINK_FLAGS
"-undefined dynamic_lookup")