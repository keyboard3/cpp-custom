#include "stdio.h"
#include "lib/uv.h"
//示例：http://docs.libuv.org/en/v1.x/guide/networking.html
int main() {
  char buf[512];
  uv_interface_address_t *info;
  int count, i;
  //获取本机系统上网的相关地址信息
  //类似ifconfig：https://www.freebsd.org/cgi/man.cgi?ifconfig
  uv_interface_addresses(&info, &count);
  i = count;

  printf("Number of interfaces: %d\n", count);
  while (i--) {
    uv_interface_address_t interface = info[i];

    //网络设备名称
    printf("Name: %s\n", interface.name);
    // is_internal表示是否是回环接口。注意一个物理接口有多个ipv4和ipv6地址
    // 设备名称会出现多次，地址只出现一次
    printf("Internal? %s\n", interface.is_internal ? "Yes" : "No");

    //网络设备ip地址
    if (interface.address.address4.sin_family == AF_INET) {
      uv_ip4_name(&interface.address.address4, buf, sizeof(buf));
      printf("IPv4 address: %s\n", buf);
    } else if (interface.address.address4.sin_family == AF_INET6) {
      uv_ip6_name(&interface.address.address6, buf, sizeof(buf));
      printf("IPv6 address: %s\n", buf);
    }

    printf("\n");
  }

  uv_free_interface_addresses(info, count);
  return 0;
}