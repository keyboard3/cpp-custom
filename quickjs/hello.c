/* File generated automatically by the QuickJS compiler. */

#include "quickjs/quickjs-libc.h"

const uint32_t qjsc_hello_size = 87;

const uint8_t qjsc_hello[87] = {
    0x02, 0x04, 0x0e, 0x63, 0x6f, 0x6e, 0x73, 0x6f, 0x6c, 0x65, 0x06,
    0x6c, 0x6f, 0x67, 0x16, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57,
    0x6f, 0x72, 0x6c, 0x64, 0x22, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c,
    0x65, 0x73, 0x2f, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x6a, 0x73,
    0x0e, 0x00, 0x06, 0x00, 0xa0, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00,
    0x00, 0x14, 0x01, 0xa2, 0x01, 0x00, 0x00, 0x00, 0x38, 0xe1, 0x00,
    0x00, 0x00, 0x42, 0xe2, 0x00, 0x00, 0x00, 0x04, 0xe3, 0x00, 0x00,
    0x00, 0x24, 0x01, 0x00, 0xcd, 0x28, 0xc8, 0x03, 0x01, 0x00,
};

static JSContext *JS_NewCustomContext(JSRuntime *rt) {
  /*
    将runtime和context关联,且context加到runtime的链表中
    还调用了JS_AddIntrinsicBasicObjects
  */
  JSContext *ctx = JS_NewContextRaw(rt);
  if (!ctx)
    return NULL;
  /*
    给Object构造函数对象添加方法(大部分是语言内部的方法，es6建议使用Reflect):
    getPrototypeOf,getOwnPropertyNames...
    给Object构造函数的proto上添加可以被所有对象继承的方法：toString,toLocaleString...
    给Function构造函数的proto上添加可以被所有function继承的方法：call,apply,bind...
    初始化Error构造函数相关内容
    给迭代器的proto对象上添加[Symbol.iterator]方法
    给Array构造函数对象上添加：isArray,from,of...
    给Array构造函数的proto上添加: concat,every,some...
    给Number,Bool,String,Math,Symbol,Generator以及global添加构造函数以及相关的初始化等
  */
  JS_AddIntrinsicBaseObjects(ctx);
  return ctx;
}

int main(int argc, char **argv) {
  /*
    JSRuntime表示一个JavaScript运行时，它是一个object
    heap,多个runtime可以同时存在但是不能exchange object
    在一个runtime中不支持多线程
  */
  JSRuntime *rt;
  /*
    JSContext表示一个JavaScript上下文(or
    Realm).每个JSContext有自己的全局对象和系统对象
    每个JSRuntime可以有多个JSContext，且JSContext之间可以共享对象，类似于浏览器中同源frames共享JS对象
  */
  JSContext *ctx;

  rt = JS_NewRuntime();
  /*
    这个只有启用了os.Worker API才有用
    会在js_worker_ctor内调用系统的pthread_create独立线程内部完成runtime和context的初始化
  */
  js_std_set_worker_new_context_func(JS_NewCustomContext);
  /*
    分配一个JSThreadState，关联os_timers,os_rw_handlers,os_signal_handlers。且将这个ts关联到runtime的user_opaque
    如果启用了os.Worker API，则这个ts依赖POSIX
    threads。设置SharedArrayBuffer内存handlers
  */
  js_std_init_handlers(rt);

  ctx = JS_NewCustomContext(rt);
  /*
    给console对象添加log函数
    给global_obj上添加console对象，以及print函数和__loadScript函数
    并将argv设置到scriptArgs上
  */
  js_std_add_helpers(ctx, argc, argv);
  /*
    将二进制通过JS_ReadObjectRec解析成语言内部的JSValue对象
    JSValue 表示一个 Javascript 值，它可以是原始类型或对象。
    如果是模块会先进行模块的预先解析，然后用evalFunctionInternal
    如果是函数，会传递一个NULL的this对象然后调用CallFree函数。
    如果是内部的调用还会根据提前解析的var_refs和栈帧创建闭包，作为this对象传下去
  */
  js_std_eval_binary(ctx, qjsc_hello, qjsc_hello_size, 0);
  /*
    js任务队列，promise的resolve,reject都会进入队列，dynamic_import也会
    循环读取js队列的任务执行。这里主要是读取上面eval_binary产生的job任务
    没有任务就退出循环
  */
  js_std_loop(ctx);

  /* 释放最开始JS_AddIntrinsicBaseObjects初始化的各种对象 */
  JS_FreeContext(ctx);
  /*
    触发gc释放未引用的对象的堆内存
    释放用于运行时模拟栈的一大块连续堆内存
  */
  JS_FreeRuntime(rt);
  return 0;
}
