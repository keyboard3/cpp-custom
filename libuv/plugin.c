#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>
#include "plugin/plugin.h"
/**
 * libuv提供了跨平台动态加载共享库的api
 * 可以用来在Node.js层提供模块系统的扩展，通过require可以引入绑定
 * 记在第三方代码时要注意完整性和安全性检查
*/

typedef void (*init_plugin_function)();
//第三方插件可以调用的系统库函数
//C++ 编译后符号会被特殊处理过，目前还不知道为什么？
void mfp_register(const char *name)
{
  fprintf(stderr, "Registered plugin \"%s\"\n", name);
}

int main(int argc, char **argv)
{
  if (argc == 1)
  {
    fprintf(stderr, "Usage: %s [plugin1] [plugin2] ...\n", argv[0]);
    return 0;
  }

  uv_lib_t *lib = (uv_lib_t *)malloc(sizeof(uv_lib_t));
  while (--argc)
  {
    fprintf(stderr, "Loading %s\n", argv[argc]);
    //利用uv_dlopen来打开指定的 ./pulign/libhello.dylib 共享库
    if (uv_dlopen(argv[argc], lib))
    {
      fprintf(stderr, "Error: %s\n", uv_dlerror(lib));
      continue;
    }

    init_plugin_function init_plugin;
    //然后在lib中找到initialize约定的初始化函数的符号
    if (uv_dlsym(lib, "initialize", (void **)&init_plugin))
    {
      fprintf(stderr, "dlsym error: %s\n", uv_dlerror(lib));
      continue;
    }
    //调用通过符号找到函数指针，初始化插件
    init_plugin();
  }

  return 0;
}
