#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "uv.h"
//示例 http://docs.libuv.org/en/v1.x/guide/filesystem.html
/**
 * 执行: cd build && ./fscat ../fscat.cpp
 * libuv的文件操作不同于socket操作，socket操作的非阻塞行为是由操作系统提供的
 * 文件系统操作内部使用的是阻塞函数，但是应用向event loop注册事件通知，让线程池来调用回调
 **/
void on_open(uv_fs_t *req);
void on_read(uv_fs_t *req);
void on_write(uv_fs_t *req);

uv_fs_t open_req, read_req, write_req;
uv_buf_t iov;
char buffer[1024] = {0};
int main(int argc, char **argv)
{

  uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_req);
  uv_fs_req_cleanup(&read_req);
  uv_fs_req_cleanup(&write_req);
  return 0;
}
void on_open(uv_fs_t *req)
{
  // 打开回调的文件请求应该和uv_fs_open传入的是同一个
  assert(req == &open_req);
  if (req->result >= 0)
  {
    iov = uv_buf_init(buffer, sizeof(buffer));
    //在on_read回调之前会将文件的数据填充到iov的buffer上
    uv_fs_read(uv_default_loop(), &read_req, req->result,
               &iov, 1, -1, on_read);
  }
  else
  {
    fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
  }
}
void on_read(uv_fs_t *req)
{
  if (req->result < 0)
  {
    fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
  }
  else if (req->result == 0) //文件末尾EOF=0
  {
    uv_fs_t close_req;
    // 同步执行的。通常单次任务的启动和关闭部分是同步执行的
    uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
  }
  else if (req->result > 0)
  {
    iov.len = req->result;
    //由于文件系统的磁盘驱动的性能考虑，单次写入可能不会立刻提交到磁盘上
    //应该是写到01文件描述符（标准输出上）
    uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
  }
}

void on_write(uv_fs_t *req)
{
  if (req->result < 0)
  {
    fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
  }
  else
  {
    //当写到标准输出上之后，继续读后续内容
    uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
  }
}