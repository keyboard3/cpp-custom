#include "stdio.h"
#include "lib/uv.h"
// 示例 http://docs.libuv.org/en/v1.x/guide/basics.html#hello-world
int64_t counter = 0;

//每次事件循环的时候都会调用一次
void wait_for_a_while(uv_idle_t *handle)
{
  counter++;
  if (counter >= 100)
  {
    uv_idle_stop(handle);
    printf("uv_idle_sto\np");
  }
}

int main()
{
  uv_idle_t idler;
  //uv_default_loop是libuv提供的默认loop
  //将handle插入loop->handle_queue队列的队尾
  uv_idle_init(uv_default_loop(), &idler);
  //将handle插入loop->idle_handles队列中
  //给handle->idle_cb=cb
  uv_idle_start(&idler, wait_for_a_while);

  printf("Idling...\n");
  //开始重复循环触发idle callback了
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  printf("uv_loop_close\n");
  uv_loop_close(uv_default_loop());
  return 0;
}