/* File generated automatically by the QuickJS compiler. */

#include "quickjs/quickjs-libc.h"
#include "test_fib.h"
#include "fibConfig.h"

static JSContext *JS_NewCustomContext(JSRuntime *rt) {
  JSContext *ctx = JS_NewContextRaw(rt);
  if (!ctx)
    return NULL;
  //添加对Object,Function,Array,Number,String,Symbol等构造函数proto及自身初始化
  JS_AddIntrinsicBaseObjects(ctx);
  //初始化标准的Date
  JS_AddIntrinsicDate(ctx);
  //给ctx的eval_internal初始化__JS_EvalInternal的能力
  JS_AddIntrinsicEval(ctx);
  //初始化下面的标准对象
  JS_AddIntrinsicStringNormalize(ctx);
  JS_AddIntrinsicRegExp(ctx);
  JS_AddIntrinsicJSON(ctx);
  JS_AddIntrinsicProxy(ctx);
  JS_AddIntrinsicMapSet(ctx);
  JS_AddIntrinsicTypedArrays(ctx);
  JS_AddIntrinsicPromise(ctx);
  JS_AddIntrinsicBigInt(ctx);
#ifndef JS_SHARED_LIBRARY
  {
    /*
      fib.c通过js_init_module_fib向ctx的loaded_modules中预置了examples/fib.so模块
      向模块的导出项中添加fib函数
     */
    extern JSModuleDef *js_init_module_fib(JSContext * ctx, const char *name);
    js_init_module_fib(ctx, "fib/fib.so");
  }
#endif
  return ctx;
}
int main(int argc, char **argv) {
  JSRuntime *rt;
  JSContext *ctx;

  rt = JS_NewRuntime();
  /* 当os.Worker API启用，会在独立的线程中初始化context和runtime */
  js_std_set_worker_new_context_func(JS_NewCustomContext);
  /* 处理os.Worker API相关内容 */
  js_std_init_handlers(rt);
  /*
    给runtime提供模块加载的能力，当js层import * from "./fib.so",会调用
    resolve_imported_module
    先调用js_find_loaded_module处理掉已经加载的模块(JS_SHARED_LIBRARY OFF是预加载)，然后用js_module_loader来处理没有加载的模块
    js_module_loader：
      如果是so库就dlopen打开库，然后通过dlsym调用库中约定js_init_module，约定这个函数向模块设置导出项（JS_SHARED_LIBRARY ON会是动态加载）
      如果非so库就用js_load_file将文件载入成二进制，通过JS_Eval编译这个模块，得到导出项
  */
  JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);

  ctx = JS_NewCustomContext(rt);
  /* 给global对象添加console能力 */
  js_std_add_helpers(ctx, argc, argv);
  /* 解析运行quickjs的二进制指令代码 */
  js_std_eval_binary(ctx, qjsc_test_fib, qjsc_test_fib_size, 0);
  /*
    先处理job_list，任务来自promise的resolve,reject，dynamic_import。
    后处理os_poll_func, os_poll_func来自于初始化context时，调用js_init_module_os会设置os_poll_func。上面的JS_NewCustomContext没有调用
    os_poll_func来自js_os_poll，只要os_rw_handlers,os_timers,port_list这些队列不会空就会执行其中的任务
    poll里面的顺序是先处理timers，然后用select系统等待os_rw_handlers和port_list产生的文件描述符集合
    然后loop过程循环往复的执行，没任务就退出了
  */
  js_std_loop(ctx);
  /* 释放context和runtime占用的资源 */
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}