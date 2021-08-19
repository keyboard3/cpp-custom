/* File generated automatically by the QuickJS compiler. */

#include "test_timeout.h"
#include "quickjs/quickjs-libc.h"

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
  /*
    导出os模块，并设置导出项:open,close,read,write,setTimeout,chdir,mkdir...
    调用js_init_module_os会设置os_poll_func以支持os_rw_handlers,port_list,os_timers
    让loop内的os_poll_func支持对这些来自系统任务的处理
  */
  js_init_module_os(ctx, "os");
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

  ctx = JS_NewCustomContext(rt);
  /* 给global对象添加console能力 */
  js_std_add_helpers(ctx, argc, argv);
  /* 解析运行quickjs的二进制指令代码 */
  js_std_eval_binary(ctx, qjsc_test_timeout, qjsc_test_timeout_size, 0);
  /*
    先处理job_list，任务来自promise的resolve,reject，dynamic_import。
    后处理os_poll_func,
    os_poll_func来自于初始化context时，调用js_init_module_os会设置os_poll_func。上面的JS_NewCustomContext没有调用
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