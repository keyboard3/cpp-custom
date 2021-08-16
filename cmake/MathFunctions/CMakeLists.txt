# 添加并构建这个库
add_library(MathFunctions MathFunctions.cxx)

# 声明任何链接到 MathFunctions 都需要包含当前的源目录才能找到 MathFunctions.h。
# INTERFACE意味着告知消费者需要，生产者不需要
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

# 默认值是ON。可以被用户改变。改变结果会保存到缓存中，不用每次构建指定
# 在terminal中必须手动指定 cmake .. -DUSE_MYMATH=OFF
option(USE_MYMATH "Use tutorial provided math implementation" ON)
# 当开启自定义的Math库
if(USE_MYMATH)
  target_compile_definitions(MathFunctions PRIVATE "USE_MYMATH")
  # 首先添加可执行程序来生成table
  add_executable(MakeTable MakeTable.cxx)
  # 添加命令来生成源代码
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    DEPENDS MakeTable
  )
  # 这个库只做sqrt,通过将生成的Table.h添加到库的源列表中让CMake知道mysqrt.cxx依赖与Table.h
  add_library(SqrtLibrary STATIC 
              mysqrt.cxx ${CMAKE_CURRENT_BINARY_DIR}/Table.h
              )
  # 指定SqrtLibrary的include路径从二进制的目录找table.h
  target_include_directories(SqrtLibrary PRIVATE
                             ${CMAKE_CURRENT_BINARY_DIR}
                             )
  # 默认是共享库，需要PIC                          
  set_target_properties(SqrtLibrary PROPERTIES
                        POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
                        )                            
  # 将SqrtLibrary链接到MathFunctions，私有用
  target_link_libraries(MathFunctions PRIVATE SqrtLibrary)
endif()

# 当在Windows上构建时，声明我们正在使用 declspec(dllexport) 的符号
target_compile_definitions(MathFunctions PRIVATE "EXPORTING_MYMATH")

# 对于 MathFunctions 库我们要安装的是library库文件和头文件。对于应用程序安装的是可执行文件和配置头
# install rules
set(installable_libs MathFunctions)
if(TARGET SqrtLibrary)
  list(APPEND installable_libs SqrtLibrary)
endif()
install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)

# 系统是否提供了 log and exp 功能?
# CheckSymbolExists 提供一个宏来检查符号是否作为函数、变量或宏在 C 中存在。
#- include(CheckSymbolExists)
# 检查sym符号是否存在在math.h头文件中，将结果存到后面的变量中
#- check_symbol_exists(log "math.h" HAVE_LOG)
#- check_symbol_exists(exp "math.h" HAVE_EXP)
#- if(NOT (HAVE_LOG AND HAVE_EXP))
#-   unset(HAVE_LOG CACHE)
#-   unset(HAVE_EXP CACHE)
  # 在有些平台需要检查m库
#-   set(CMAKE_REQUIRED_LIBRARIES "m")
#-   check_symbol_exists(log "math.h" HAVE_LOG)
#-   check_symbol_exists(exp "math.h" HAVE_EXP)
#-   if(HAVE_LOG AND HAVE_EXP)
    # 指定链接给定目标 PUBLIC表示库和目标链接之后，成为接口一部分。PRIVAITE链接之后不是接口一部分。INTERFACE后面的库附加到链接接口，但不会链接到目标
#-     target_link_libraries(MathFunctions PRIVATE m)
#-   endif()
#- endif()

# 会将变量编译成宏定义
#- if(HAVE_LOG AND HAVE_EXP)
#-   target_compile_definitions(MathFunctions
#-                              PRIVATE "HAVE_LOG" "HAVE_EXP")
#- endif()