# 库单独编译
add_library(MathFunctions mysqrt.cxx)

# 声明任何链接到 MathFunctions 都需要包含当前的源目录才能找到 MathFunctions.h。
# INTERFACE意味着告知消费者需要，生产者不需要
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
# 系统是否提供了 log and exp 功能?
# CheckSymbolExists 提供一个宏来检查符号是否作为函数、变量或宏在 C 中存在。
include(CheckSymbolExists)
# 检查sym符号是否存在在math.h头文件中，将结果存到后面的变量中
check_symbol_exists(log "math.h" HAVE_LOG)
check_symbol_exists(exp "math.h" HAVE_EXP)
if(NOT (HAVE_LOG AND HAVE_EXP))
  unset(HAVE_LOG CACHE)
  unset(HAVE_EXP CACHE)
  # 在有些平台需要检查m库
  set(CMAKE_REQUIRED_LIBRARIES "m")
  check_symbol_exists(log "math.h" HAVE_LOG)
  check_symbol_exists(exp "math.h" HAVE_EXP)
  if(HAVE_LOG AND HAVE_EXP)
    # 指定链接给定目标 PUBLIC表示库和目标链接之后，成为接口一部分。PRIVAITE链接之后不是接口一部分。INTERFACE后面的库附加到链接接口，但不会链接到目标
    target_link_libraries(MathFunctions PRIVATE m)
  endif()
endif()

# 会将变量编译成宏定义
if(HAVE_LOG AND HAVE_EXP)
  target_compile_definitions(MathFunctions
                             PRIVATE "HAVE_LOG" "HAVE_EXP")
endif()

# 对于 MathFunctions 库我们要安装的是library库文件和头文件。对于应用程序安装的是可执行文件和配置头
install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)