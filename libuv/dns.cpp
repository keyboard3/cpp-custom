#include "stdlib.h"
#include "string"
#include "lib/uv.h"
//示例：http://docs.libuv.org/en/v1.x/guide/networking.html
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void on_connect(uv_connect_t *req, int status);
void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res);
uv_loop_t *loop;
// libuv提供了异步的getaddrinfo和getnameinfo的各种衍生方法
int main() {
  loop = uv_default_loop();

  //需要向dns服务商查询ip地址，所以这个一定是tcp请求
  struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  // uv_getaddrinfo_t是不需要handle直接作用域loop上的request
  uv_getaddrinfo_t resolver;
  fprintf(stderr, "www.baidu.com is... ");
  //如果参数是一个错误的地址信息，会立刻返回0。如果是正取的地址会异步libuv事件通知回调并带上结果
  int r = uv_getaddrinfo(loop, &resolver, on_resolved, "www.baidu.com", "80",
                         &hints);

  if (r) {
    fprintf(stderr, "getaddrinfo call error %s\n", uv_err_name(r));
    return 1;
  }
  return uv_run(loop, UV_RUN_DEFAULT);
}
void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    return;
  }
  //拿到dns的解析ip地址结果
  char addr[17] = {'\0'};
  uv_ip4_name((struct sockaddr_in *)res->ai_addr, addr, 16);
  fprintf(stderr, "%s\n", addr);

  uv_connect_t *connect_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));
  uv_tcp_t *socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);
  fprintf(stderr, "connecting... ip:%s\n", addr);
  uv_tcp_connect(connect_req, socket, (const struct sockaddr *)res->ai_addr,
                 on_connect);
  //释放request
  uv_freeaddrinfo(res);
}
void on_connect(uv_connect_t *req, int status) {
  if (status < 0) {
    //连接百度，如果是错误端口，会报 ETIMEDOUT 
    //UNIX®网络编程: 如果客户端TCP没有收到对其SYN段的响应，则返回ETIMEDOUT
    fprintf(stderr, "connect failed error %s\n", uv_err_name(status));
    free(req);
    return;
  }
  fprintf(stderr,"connected data is ");
  uv_read_start((uv_stream_t *)req->handle, alloc_buffer, on_read);
  free(req);
}
void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    if (nread != UV_EOF)
      fprintf(stderr, "Read error %s\n", uv_err_name(nread));
    uv_close((uv_handle_t *)client, NULL);
    free(buf->base);
    free(client);
    return;
  }

  char *data = (char *)malloc(sizeof(char) * (nread + 1));
  data[nread] = '\0';
  strncpy(data, buf->base, nread);

  fprintf(stderr, "%s", data);
  free(data);
  free(buf->base);
}
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}