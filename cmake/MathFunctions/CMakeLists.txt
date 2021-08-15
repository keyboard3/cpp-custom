# 库单独编译
add_library(MathFunctions mysqrt.cxx)

# 声明任何链接到 MathFunctions 都需要包含当前的源目录才能找到 MathFunctions.h。
# INTERFACE意味着告知消费者需要，生产者不需要
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

# 对于 MathFunctions 库我们要安装的是library库文件和头文件。对于应用程序安装的是可执行文件和配置头
install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)