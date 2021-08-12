#include "stdio.h"
#include "stdlib.h"
#include "string"
#include "uv.h"

#define PORT 7000
#define DEFAULT_BACKLOG 128
uv_loop_t *loop;
struct sockaddr_in addr;

typedef struct
{
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;
void free_write_req(uv_write_t *req);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void echo_write(uv_write_t *req, int status);
void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void on_new_connection(uv_stream_t *server, int status);

int main()
{
  loop = uv_default_loop();
  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  //这个api比较直观的将ip地址和端口号设置到sock_addr上
  uv_ip4_addr("0.0.0.0", PORT, &addr);
  //给tcp server handle绑定上这sock_addr
  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
  //当客户端新连接建立的时候，就会触发这个回调。DEFAULT_BACKLOG是队列连接最大的长度
  int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
  if (r)
  {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  puts("listen...");
  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);
  free(loop);
  return 0;
}
void on_new_connection(uv_stream_t *server, int status)
{
  if (status < 0)
  {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }
  //为客户端建立监听的tcp handle
  uv_tcp_t *client = new uv_tcp_t();
  uv_tcp_init(loop, client);
  //从server中关联新的连接到client上
  if (uv_accept(server, (uv_stream_t *)client) == 0)
  {
    puts("new connection");
    //从incoming stream中读取数据，alloc_buffer会被调用多次直接读取完全部数据，encho_read会被调用
    //这个是个stream handle，所以会持续的收到这个client连接发送过来的数据
    uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read);
  }
  else
  {
    //遇到错误，关闭释放这个刚建立的handle
    uv_close((uv_handle_t *)client, NULL);
  }
}
void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
  if (nread > 0)
  {
    write_req_t *req = new write_req_t();
    req->buf = uv_buf_init(buf->base, nread);
    printf("from client msg: %s\n", buf->base);
    //向流(client的handle)写入数据(request)
    uv_write((uv_write_t *)req, client, &req->buf, 1, echo_write);
    return;
  }
  if (nread < 0)
  {
    //遇到错误，释放掉client handle
    if (nread != UV_EOF)
      fprintf(stderr, "Read error %zd %s\n", nread, uv_err_name(nread));
    puts("close client");
    uv_close((uv_handle_t *)client, NULL);
  }

  free(buf->base);
}
void free_write_req(uv_write_t *req)
{
  write_req_t *wr = (write_req_t *)req;
  free(wr->buf.base);
  free(wr);
}
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}
void echo_write(uv_write_t *req, int status)
{
  if (status)
  {
    fprintf(stderr, "Write error %s\n", uv_strerror(status));
  }
  printf("wrote complted");
  //释放掉写入请求
  free_write_req(req);
}