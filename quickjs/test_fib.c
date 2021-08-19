/* File generated automatically by the QuickJS compiler. */

#include "quickjs/quickjs-libc.h"
#include "fibConfig.h"

const uint32_t qjsc_test_fib_size = 157;
/*
import { fib } from "examples/fib.so";
console.log("Hello World");
console.log("fib(10)=", fib(10));
*/
const uint8_t qjsc_test_fib[157] = {
    0x02, 0x07, 0x28, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x73, 0x2f,
    0x74, 0x65, 0x73, 0x74, 0x5f, 0x66, 0x69, 0x62, 0x2e, 0x6a, 0x73, 0x10,
    0x2e, 0x2f, 0x66, 0x69, 0x62, 0x2e, 0x73, 0x6f, 0x06, 0x66, 0x69, 0x62,
    0x0e, 0x63, 0x6f, 0x6e, 0x73, 0x6f, 0x6c, 0x65, 0x06, 0x6c, 0x6f, 0x67,
    0x16, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64,
    0x10, 0x66, 0x69, 0x62, 0x28, 0x31, 0x30, 0x29, 0x3d, 0x0f, 0xc2, 0x03,
    0x01, 0xc4, 0x03, 0x00, 0x00, 0x01, 0x00, 0xc6, 0x03, 0x00, 0x0e, 0x00,
    0x06, 0x01, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x05, 0x01, 0x00, 0x30, 0x00,
    0xc6, 0x03, 0x00, 0x0c, 0x08, 0xea, 0x02, 0x29, 0x38, 0xe4, 0x00, 0x00,
    0x00, 0x42, 0xe5, 0x00, 0x00, 0x00, 0x04, 0xe6, 0x00, 0x00, 0x00, 0x24,
    0x01, 0x00, 0x0e, 0x38, 0xe4, 0x00, 0x00, 0x00, 0x42, 0xe5, 0x00, 0x00,
    0x00, 0x04, 0xe7, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0xbd, 0x0a, 0xef,
    0x24, 0x02, 0x00, 0x29, 0xc2, 0x03, 0x01, 0x05, 0x01, 0x00, 0x04, 0x0a,
    0x62,
};

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
    js_init_module_fib(ctx, "examples/fib.so");
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
  /* 循环处理上面eval产生的job */
  js_std_loop(ctx);
  /* 释放context和runtime占用的资源 */
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}