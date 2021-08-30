# cpp-custom
知道不难，难在实践。做到知行合一。
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
- [quickjs](./quickjs): `基于libuv的简易运行时、官方仓库中的部分示例`
- [jsGUI](./skia):`简易浏览器(基于 Skia 和 Quickjs)`
    - 支持简易 html 标记语言解析渲染
    - 支持解析 javascript 标签运行
    - 支持标签绑定 onclick 事件
    - 支持 document.getElementById 读取指定dom对象
    - 支持 dom 对象上 width,height,innerHtml 属性以及 setAttribute 重渲染
    - 支持地址栏 file 协议访问本地文件渲染
    - 支持地址栏 http/https 协议访问服务器渲染（同步，基于curl）