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
- [libuv](./libuv): `libuv官方部分示例实践`
- [tcp/udp 通信](./socket): `网络通信实践，包括从单 socket 到多进程、多线程、select、kqueue、epoll等多socket的通信`
- [curl](./curl):`curl官方部分示例实践`
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