# cpp-custom
纯手撸一些c++实现

## 运行
例如 libuv
```shell
cd libuv && mkdir build && cd build
cmake .. && cmake --build .
./hello
```

## 例子
- [简单的栈机](./virtual-stack-machine.cpp)
- [字节序验证](./endianness.cpp)
- [JIT原型](./jit-proto.cpp)
- [cmake step1 - step10](./cmake)
- libuv
    - [监听 idle](./libuv/idle.cpp)
    - [读写文件 cat](./libuv/fscat.cpp)
    - [客户端 tcp](./libuv/tcp-echo-client.cpp)
    - [服务端 tcp](./libuv/tcp-echo-server.cpp)
    - [udp-dhcp](./libuv/udp-dhcp.cpp)
    - [dns](./libuv/dns.cpp)
    - [网络 interfaces](./libuv/interfaces.cpp)
    - [ref timer](./libuv/ref-timer.cpp)
    - [idle compute](./libuv/idle-compute.cpp)
    - [plugin](./libuv/plugin.cpp)
    - [tty](./libuv/tty.cpp)
    - [tty Gravity](./libuv/tty-gravity.cpp)
- quickjs
    - [hello](./quickjs/hello.c)
- skia
    - [7角星](./skia/heptagram.cpp)
    - 结论：`发现如果是要绘制在窗口并交互，还是在skia源码下用ninja编译最合适，就不在这里折腾了。`
- 简单 TCP 通信
    - [客户端 socket](./socket/tcp-socket-client.cpp)
    - [服务端 socket](./socket/tcp-socket-server.cpp)
    - [服务端并发 socket：多进程](./socket/tcp-socket-server-process.cpp)
    - [服务端并发 socket：多线程](./socket/tcp-socket-server-thread.cpp)
    - [服务端并发 socket：select](./socket/tcp-socket-server-select.cpp)
    - [服务端并发 socket：kqueue](./socket/tcp-socket-server-kqueue.cpp)
    - [服务端并发 socket：epoll](./socket/tcp-socket-server-epoll.cpp)
    - 总结
        - TCP socket的通信，通信时长看业务逻辑。recv阻塞IO，导致这个socket不处理完，连接不关闭就无法消费队列中的其他连接
        - 因为在TCP不拥塞的情况下，假设业务层处理时间已经优化了，那么一个socket的响应时间是确定的。在等待网络IO时计算机资源理论上还可以处理其他连接的，这样就可以提高程序性能的吞吐量
        - 多进程和多线程使程序可以同时处理多个socket，提升程序的吞吐量，但是维持这些结构非常费内存，有c10k的限制
        - select则可以监听文件描述符集合，当有连接socket可读的时候，程序就可以通过遍历找到socket进行业务处理。没有c10k的限制，但连接太多影响吞吐量的因素就是这个遍历查找时间O(n)
        - epoll和kqueue则进行了优化由内核接管了查找过程，通过红黑树提高查找效率O(logn)。用户进程只需可以监听单个文件描述符事件的变化了。极大的提高了程序性能的吞吐量
    - 推荐参考文章
        - [搞了半天，终于弄懂了TCP Socket数据的接收和发送，太难~](https://cloud.tencent.com/developer/article/1666211)
- 简单 UDP 通信
    - [客户端 socket](./socket/udp-socket-client.cpp)
    - [服务端 socket](./socket/udp-socket-server.cpp)