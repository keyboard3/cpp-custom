#include "plugin.h"
//告诉编译器不要用c++的方式混淆函数名，以便通过dlsym找initialize
extern "C"
{
    //我们接口定义了所有插件都有一个initialize函数。这个插件被编译成一个共享库
    void initialize()
    {
        //因为动态链接库，不会连接
        mfp_register("Hello World!");
    }
}