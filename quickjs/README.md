# Quickjs
- [hello](./quickjs/hello.c): `打印 'Hello World'`
- [test_fib](./quickjs/test_fib.c): `在 js 代码导入 c++ 模块`
- [test_timeout](./quickjs/test_fib.c): `在 js 代码支持 os 系统任务的调用，如 setTimeout`
- [test_libuv](./quickjs/test_libuv.c): `用 libuv 的 loop 换掉 Quickjs 自身的 loop`
    - 支持 setTimeout, clearTimeout 宏任务
    - 支持微任务及保证和宏任务的执行顺序