# libuv

## 实践案例

- [idle](./idle.cpp): `libuv空闲时回调`
- [cat](./fscat.cpp): `模仿cat命令读文件打印到标准输出中`
- [客户端 tcp](./tcp-echo-client.cpp): `tcp client 与 tcp server端的通信示例`
- [服务端 tcp](./tcp-echo-server.cpp): `tcp server 与 tcp client端的通信示例`
- [udp-dhcp](./udp-dhcp.cpp): `局域网内的dhcp server发送分配请求以获得分配的ip地址`
- [dns](./dns.cpp): `查询域名的ip地址`
- [网络 interfaces](./interfaces.cpp): `类似于ifconfig来查询本机网络设备的信息`
- [ref timer](./ref-timer.cpp): `移除loop中active状态的引用`
- [idle compute](./idle-compute.cpp): `在loop空闲时间做一些低优先级任务`
- [uvwget](./uvwget.cpp): `细粒度的控制curl库的下载过程`
- [plugin](./plugin.cpp): `跨平台加载动态库`
- [tty](./tty.cpp): `终端输出的标准格式化应用`
- [tty Gravity](./tty-gravity.cpp): `终端标准输出的简易动画`