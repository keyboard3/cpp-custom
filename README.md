# cpp-custom
纯手撸一些c++实现

- [简单的栈机](./virtual-stack-machine.cpp)
- 简单 TCP 通信
    - [客户端 socket](./tcp-socket-client.cpp)
    - [服务端 socket](./tcp-socket-server.cpp)
    - [服务端并发 socket：多进程](./tcp-socket-server-process.cpp)
    - [服务端并发 socket：多线程](./tcp-socket-server-thread.cpp)
    - [服务端并发 socket：select](./tcp-socket-server-select.cpp)
    - [服务端并发 socket：epoll](./tcp-socket-server-epoll.cpp)
    - [服务端并发 socket：kqueue](./tcp-socket-server-kqueue.cpp)
    - 推荐参考文章
        - [搞了半天，终于弄懂了TCP Socket数据的接收和发送，太难~](https://cloud.tencent.com/developer/article/1666211)
- 简单 UDP 通信
    - [客户端 socket](./udp-socket-client.cpp)
    - [服务端 socket](./udp-socket-server.cpp)