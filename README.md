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
- [简单的栈机](./virtual-stack-machine.cpp): 进一步实现见我的 [learn-compiler / super-tiny-virtual-machine](https://github.com/keyboard3/learn-compiler/tree/main/cpp-super-tiny-virtual-machine) 以及 [learn-compiler / tiny-virtual-machine](https://github.com/keyboard3/learn-compiler/tree/main/cpp-tiny-virtual-machine)
- [字节序验证](./endianness.cpp): 同理js可以通过ArrayBuffer来验证,见我的 [js-custom / array-buffer](https://github.com/keyboard3/js-custom/blob/main/array-buffer.mjs)
- [JIT原型](./jit-proto.cpp): 实战见我的 [learn-compiler / kaleidoscope](https://github.com/keyboard3/learn-compiler/tree/main/kaleidoscope) llvm的小型语言编译器
- [Makefile](./cmake): `编译工具Make的简单示例`
- [cmake step1 - step10](./cmake): `元构建系统cmake演变至step10的示例`
- [gn simple example](./gn_example): `元构建系统gn构建示例`
- [xml parser](./parser/xmlParser.cpp): `简易xml解析器`
- [html parser](./parser/htmlParser.cpp): `简易html解析器`
- [libuv](./libuv): `实践多个官方示例`
- quickjs
    - [hello](./quickjs/hello.c): `打印'Hello World'`
    - [test_fib](./quickjs/test_fib.c): `在js代码导入c++模块`
    - [test_timeout](./quickjs/test_fib.c): `在js代码支持os系统任务的调用，如setTimeout`
    - [test_libuv](./quickjs/test_libuv.c): `基于libuv的loop实现js任务的执行`
        - 支持setTimeout,clearTimeout宏任务
        - 支持微任务及保证和宏任务的执行顺序
- skia
    - [jsGUI](./skia/jsGUI): `简易浏览器(基于Quickjs)`
        - 支持简易 html 标记语言解析渲染
        - 支持解析 javascript 标签运行
        - 支持标签绑定 onclick 事件
        - 支持 document.getElementById 读取指定dom对象
        - 支持 dom 对象上 width,height,innerHtml 属性以及 setAttribute 重渲染
        - 支持地址栏 file:// 协议访问重刷新
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