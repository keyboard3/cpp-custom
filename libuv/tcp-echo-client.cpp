#include "stdio.h"
#include "stdlib.h"
#include "string"
#include "lib/uv.h"
//示例：http://docs.libuv.org/en/v1.x/guide/networking.html
#define PORT 7000
#define DEFAULT_BACKLOG 128
uv_loop_t *loop;
void on_close(uv_handle_t *handle);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void on_write(uv_write_t *req, int status);
void on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);
static void on_connect(uv_connect_t *connection, int status);
int main()
{
  uv_tcp_t *socket = new uv_tcp_t();
  uv_tcp_init(uv_default_loop(), socket);

  uv_connect_t *connect = new uv_connect_t();
  struct sockaddr_in dest;
  uv_ip4_addr("127.0.0.1", PORT, &dest);

  puts("Connecting...");
  uv_tcp_connect(connect, socket, (const struct sockaddr *)&dest, on_connect);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  uv_loop_close(uv_default_loop());
  free(uv_default_loop());
  return 0;
}
static void on_connect(uv_connect_t *connection, int status)
{
  if (status < 0)
  {
    printf("failed to connect\n");
    return;
  }
  printf("connected. %d\n", status);

  uv_stream_t *stream = connection->handle;
  free(connection);

  uv_buf_t buffer[] = {
      {.base = "hello", .len = 5},
      {.base = "world", .len = 5}};
  uv_write_t *req = new uv_write_t();
  //向流(server的handle)写入数据(request)
  uv_write(req, stream, buffer, 2, on_write);
  //从incoming stream中读取数据，alloc_buffer会被调用多次直接读取完全部数据，encho_read会被调用
  //这个是个stream handle，所以会持续的收到这个client连接发送过来的数据
  uv_read_start(stream, alloc_buffer, on_read);
}
void on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)
{
  printf("on_read.\n");
  if (nread >= 0)
    printf("read: %s\n", buf->base);
  else
    uv_close((uv_handle_t *)tcp, on_close);
  free(buf->base);
}
void on_write(uv_write_t *req, int status)
{
  if (status)
  {
    perror("uv_write error ");
    return;
  }
  printf("wrote.\n");
  free(req);
}
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}
void on_close(uv_handle_t *handle)
{
  printf("closed.");
}