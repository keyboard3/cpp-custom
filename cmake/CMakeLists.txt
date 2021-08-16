cmake_minimum_required(VERSION 3.19)

# 设置项目名和版本号
project(Tutorial VERSION 1.0)

# 让我们首先构建一个 INTERFACE 目标并指定所需的 C++ 标准级别 11，而不是使用 CMAKE_CXX_STANDARD。
add_library(tutorial_compiler_flags INTERFACE)
target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_11)

# 通过BUILD_INTERFACE genex在构建项目时添加编译警告标志
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
# 由于警告标志因编译器而异，使用COMPILE_LANG_AND_ID控制给定语言和一组编译器需要那些标志
# 警告标志被封装在一个 BUILD_INTERFACE 条件中。这样做是为了我们已安装项目的使用者不会继承我们的警告标志。
target_compile_options(tutorial_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

# 指定c++标准库11,确保CMAKE_CXX_STANDARD声明是在add_executable之上声明
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

# 控制静态库和动态库构建的位置，以便在Windows上我们不需要修改运行可执行文件的路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# 配置头文件以及将CMake的设置传递给源代码
configure_file(TutorialConfig.h.in TutorialConfig.h)

# 添加依赖的库
add_subdirectory(MathFunctions)

# 添加可执行程序
add_executable(Tutorial tutorial.cxx)
target_link_libraries(Tutorial PUBLIC MathFunctions)

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
# 启用dashboard脚本 这个模块将会自动调用enable_testing()
include(CTest)

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

# 这个模块将include这个项目在当前platform下的所有运行时库
include(InstallRequiredSystemLibraries)
# 这一些CPack变量存储项目的license以及版本信息
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
# 它将使用这些变量和当前系统的一些其他属性来设置安装程序
include(CPack)
# cpack 生成二进制分发
# cpack -G ZIP -C Debug -G指定生成器 -G指定多配置
# cpack --config CPackSourceConfig.cmake 创建源代码分发