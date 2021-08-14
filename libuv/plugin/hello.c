#include "plugin.h"
//我们接口定义了所有插件都有一个initialize函数。这个插件被编译成一个共享库
void initialize()
{
    //因为动态链接库，不会连接
    mfp_register("Hello World!");
}
//c++版本的编译动态库nm -gU 没有dyld_stub_binder。目前只能c
//clang hello.c -undefined dynamic_lookup -dynamiclib -fPIC -o libhello.dylib
//-dynamiclib: mac下的shared,表明生成的文件时动态库文件
//-fPIC： 表明生成的动态库是位置无关代码。可以通过GOT表查找符号的相对地址调用
//-undefined dynamic_lookup： 标记所有找不到的符号在运行时找