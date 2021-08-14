#include "stdio.h"
#include "stdlib.h"
#include "lib/uv.h"
// 示例 http://docs.libuv.org/en/v1.x/guide/basics.html#hello-world
int main()
{
  //用户自己给loop分配自定义内存
  uv_loop_t *loop = new uv_loop_t();
  uv_loop_init(loop);
  //因为没有任何事件需要它处理，所以会立刻退出
  printf("Now quitting.\n");
  uv_run(loop, UV_RUN_DEFAULT);
  //调用close释放自己管理的内存
  uv_loop_close(loop);
  free(loop);
  return 0;
}