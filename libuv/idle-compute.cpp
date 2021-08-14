#include <stdio.h>
#include "lib/uv.h"

uv_loop_t *loop;
uv_fs_t stdin_watcher;
uv_idle_t idler;
char buffer[1024];
void crunch_away(uv_idle_t *handle);
void on_type(uv_fs_t *req);

int main()
{
  loop = uv_default_loop();
  // 监听cpu空闲
  uv_idle_init(loop, &idler);
  uv_idle_start(&idler, crunch_away);
  // 监听标准输入
  uv_buf_t buf = uv_buf_init(buffer, 1024);
  uv_fs_read(loop, &stdin_watcher, 0, &buf, 1, -1, on_type);
  return uv_run(loop, UV_RUN_DEFAULT);
}

void crunch_away(uv_idle_t *handle)
{
  //做一些低优先级的任务，如每日应用性能的的摘要发送给开发人员进行分析
  fprintf(stderr, "Computing PI...\n");
  // 为了避免疯狂的向控制台输出
  // 监听一次cpu空闲就退出了，等待下次输入处理完成之后的监听
  uv_idle_stop(handle);
}

void on_type(uv_fs_t *req)
{
  //如果读取的标准输入有内容
  if (stdin_watcher.result > 0)
  {
    buffer[stdin_watcher.result] = '\0';
    //打印出输入的结果
    printf("Typed %s\n", buffer);
    //因为读取requst是一次性的，所以需要再次监听输入
    uv_buf_t buf = uv_buf_init(buffer, 1024);
    uv_fs_read(loop, &stdin_watcher, 0, &buf, 1, -1, on_type);
    //同时再次监听cpu空闲
    uv_idle_start(&idler, crunch_away);
  }
  else if (stdin_watcher.result < 0)
  {
    fprintf(stderr, "error opening file: %s\n", uv_strerror(req->result));
  }
}