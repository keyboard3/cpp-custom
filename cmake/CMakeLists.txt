cmake_minimum_required(VERSION 3.19)

# 设置项目名和版本号
project(Tutorial VERSION 1.0)

# 指定c++标准库11,确保CMAKE_CXX_STANDARD声明是在add_executable之上声明
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 这个配置项将会在ui中显示出来，默认值是ON。可以被用户改变。改变结果会保存到缓存中，不用每次构建指定
# 在terminal中必须手动指定 cmake .. -DUSE_MYMATH=OFF
option(USE_MYMATH "Use tutorial provided math implementation" ON)

# 配置头文件以及将CMake的设置传递给源代码
configure_file(TutorialConfig.h.in TutorialConfig.h)

# 当开启自定义的Math库
if(USE_MYMATH)
  # 添加MathFunctions库，以便构建这个库
  add_subdirectory(MathFunctions)
  # 添加MathFunctions到数组变量EXTRA_LIBS
  list(APPEND EXTRA_LIBS MathFunctions)
endif()

# 添加可执行程序
add_executable(Tutorial tutorial.cxx)

# 指定链接给定目标 PUBLIC表示库和目标链接之后，成为接口一部分。PRIVAITE链接之后不是接口一部分。INTERFACE后面的库附加到链接接口，但不会链接到目标
target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

# 将binary dir添加到include files的搜索路径上，所以我们可以找到TutorialConfig.h
target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

# 添加安装目标 :cmake --install . --prefix "/home/myuser/installdir"
# cmake --install . 会安装头文件，库和可执行文件。prefix会重写安装的根目录
#Installing: /usr/local/lib/libMathFunctions.a
#Installing: /usr/local/include/MathFunctions.h
#Installing: /usr/local/bin/Tutorial
#Installing: /usr/local/include/TutorialConfig.h
install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  DESTINATION include
  )

# 我们可以启用测试，然后添加一些基本测试来验证应用程序是否正常工作
enable_testing()

# 这个测试只是验证程序运行。没有段错误或其他方式崩溃，并且返回值为零。这是 CTest 测试的基本形式。
add_test(NAME Runs COMMAND Tutorial 25)

# 验证输出是否包含某些字符串，当数量不正确的时候会打印使用消息
add_test(NAME Usage COMMAND Tutorial)
set_tests_properties(Usage
  PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*number"
  )
# 定义do_test函数添加测试用例
function(do_test target arg result)
  add_test(NAME Comp${arg} COMMAND ${target} ${arg})
  set_tests_properties(Comp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endfunction(do_test)

# 没掉调用do_test就会添加一个测试项。包括名称、输入和基于传递参数的预期结果
do_test(Tutorial 4 "4 is 2")
do_test(Tutorial 9 "9 is 3")
do_test(Tutorial 5 "5 is 2.236")
do_test(Tutorial 7 "7 is 2.645")
do_test(Tutorial 25 "25 is 5")
do_test(Tutorial -25 "-25 is [-nan|nan|0]")
do_test(Tutorial 0.0001 "0.0001 is 0.01")
# ctest -N 打印总数。ctest -VV打印过程日志。
# 在Debug模式下运行测试: ctest -C Debug -VV