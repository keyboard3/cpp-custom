#include "stdlib.h"
#include "string"
#include "uv.h"
// libuv将系统底层在一个socket文件描述符上的监听，抽象分离出了两个handle,分开监听和发送两个操作
uv_loop_t *loop;
uv_udp_t send_socket;
uv_udp_t recv_socket;
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void on_send(uv_udp_send_t *req, int status);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
uv_buf_t make_discover_msg();
void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags);
int main() {
  loop = uv_default_loop();
  // 0.0.0.0/(INADDR_ANY)好像是监听本机的所有网卡(有待验证)
  //接收socket绑定68端口(DHCP客户端)
  uv_udp_init(loop, &recv_socket);
  struct sockaddr_in recv_addr;
  uv_ip4_addr("0.0.0.0", 68, &recv_addr);
  // UV_UDP_REUSEADDR表示多个线程或进程可以无误的绑定同一个地址
  //但只有最后一个绑定的会收到流量
  uv_udp_bind(&recv_socket, (const struct sockaddr *)&recv_addr,
              UV_UDP_REUSEADDR);
  uv_udp_recv_start(&recv_socket, alloc_buffer, on_read);

  uv_udp_init(loop, &send_socket);
  struct sockaddr_in broadcast_addr;
  //端口0表示操作系统会随机分配一个端口
  uv_ip4_addr("0.0.0.0", 0, &broadcast_addr);
  uv_udp_bind(&send_socket, (const struct sockaddr *)&broadcast_addr, 0);
  //必须设置广播标志，否则会得到EACCES错误（ps: 应该是发送广播请求才需要）
  uv_udp_set_broadcast(&send_socket, 1);

  //准备发送给DHCP server查询分配ip的数据包
  uv_udp_send_t send_req;
  uv_buf_t discover_msg = make_discover_msg();

  // ip地址255.255.255.255是广播地址，这表示数据包将发送到子网上的所有机器上的67端口(DHCP
  // Server)上
  struct sockaddr_in send_addr;
  uv_ip4_addr("255.255.255.255", 67, &send_addr);
  uv_udp_send(&send_req, &send_socket, &discover_msg, 1,
              (const struct sockaddr *)&send_addr, on_send);

  return uv_run(loop, UV_RUN_DEFAULT);
}
void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags) {
  // nread 0表示没有数据读取, <0表示读取出现异常
  if (nread < 0) {
    fprintf(stderr, "Read error %s\n", uv_err_name(nread));
    uv_close((uv_handle_t *)req, NULL);
    free(buf->base);
    return;
  }

  //因为UDP socket没有特定的连接方，所有回调会接收到关于连接方的数据
  char sender[17] = {0};
  uv_ip4_name((const struct sockaddr_in *)addr, sender, 16);
  fprintf(stderr, "Recv from %s\n", sender);

  //如果分配函数没有提供足够的空间，flags会是UV_UDP_PARTIAL，操作系统会丢弃溢出的数据
  unsigned int *as_integer = (unsigned int *)buf->base;
  unsigned int ipbin = ntohl(as_integer[4]);
  unsigned char ip[4] = {0};
  int i;
  for (i = 0; i < 4; i++)
    ip[i] = (ipbin >> i * 8) & 0xff;
  fprintf(stderr, "Offered IP %d.%d.%d.%d\n", ip[3], ip[2], ip[1], ip[0]);

  free(buf->base);
  uv_udp_recv_stop(req);
}
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
};
uv_buf_t make_discover_msg() {
  uv_buf_t buffer;
  alloc_buffer(NULL, 256, &buffer);
  memset(buffer.base, 0, buffer.len);

  // BOOTREQUEST
  buffer.base[0] = 0x1;
  // HTYPE ethernet
  buffer.base[1] = 0x1;
  // HLEN
  buffer.base[2] = 0x6;
  // HOPS
  buffer.base[3] = 0x0;
  // XID 4 bytes
  buffer.base[4] = (unsigned int)random();
  // SECS
  buffer.base[8] = 0x0;
  // FLAGS
  buffer.base[10] = 0x80;
  // CIADDR 12-15 is all zeros
  // YIADDR 16-19 is all zeros
  // SIADDR 20-23 is all zeros
  // GIADDR 24-27 is all zeros
  // CHADDR 28-43 is the MAC address, use your own
  buffer.base[28] = 0xe4;
  buffer.base[29] = 0xce;
  buffer.base[30] = 0x8f;
  buffer.base[31] = 0x13;
  buffer.base[32] = 0xf6;
  buffer.base[33] = 0xd4;
  // SNAME 64 bytes zero
  // FILE 128 bytes zero
  // OPTIONS
  // - magic cookie
  buffer.base[236] = 99;
  buffer.base[237] = 130;
  buffer.base[238] = 83;
  buffer.base[239] = 99;

  // DHCP Message type
  buffer.base[240] = 53;
  buffer.base[241] = 1;
  buffer.base[242] = 1; // DHCPDISCOVER

  // DHCP Parameter request list
  buffer.base[243] = 55;
  buffer.base[244] = 4;
  buffer.base[245] = 1;
  buffer.base[246] = 3;
  buffer.base[247] = 15;
  buffer.base[248] = 6;

  return buffer;
}

void on_send(uv_udp_send_t *req, int status) {
  if (status) {
    fprintf(stderr, "Send error %s\n", uv_strerror(status));
    return;
  }
}