/* File generated automatically by the QuickJS compiler. */

#include "test_libuv.h"
#include "fibConfig.h"
#include "quickjs/quickjs-libc.h"
#include "quickjs/quickjs.h"
#include "uv.h"
#define countof(x) (sizeof(x) / sizeof((x)[0]))
uv_timer_t time_job_req;
uv_loop_t *loop;
JSContext *ctx;
static JSContext *JS_NewCustomContext(JSRuntime *rt);
static void JS_AddLibuv(JSContext *ctx);
int main(int argc, char **argv) {
  JSRuntime *rt;
  rt = JS_NewRuntime();
  ctx = JS_NewCustomContext(rt);
  loop = uv_default_loop();
  /* 给global对象添加console能力 */
  js_std_add_helpers(ctx, argc, argv);
  /* 解析运行quickjs的二进制指令代码 */
  js_std_eval_binary(ctx, qjsc_test_libuv, qjsc_test_libuv_size, 0);

  /* libuv */
  uv_run(loop, UV_RUN_DEFAULT);

  /* 释放libuv的资源 */
  uv_loop_close(loop);
  /* 释放context和runtime占用的资源 */
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
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
  JS_AddLibuv(ctx);
  return ctx;
}
JSValue timeFun;
static void time_job(uv_timer_t *handle) {
  JS_Call(ctx,timeFun,JS_UNDEFINED,0,NULL);
  JS_FreeValue(ctx, timeFun);
}
static JSValue js_setTimeout(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  int64_t delay;
  JSValueConst func;
  JSValue obj;

  func = argv[0];
  if (!JS_IsFunction(ctx, func))
    return JS_ThrowTypeError(ctx, "not a function");
  if (JS_ToInt64(ctx, &delay, argv[1]))
    return JS_EXCEPTION;

  timeFun = JS_DupValue(ctx, func);
  uv_timer_init(loop, &time_job_req);
  uv_timer_start(&time_job_req, time_job, delay, 0);
  return obj;
}
static const JSCFunctionListEntry js_libuv_funcs[] = {
    JS_CFUNC_DEF("setTimeout", 1, js_setTimeout),
};
static int js_libuv_init(JSContext *ctx, JSModuleDef *m) {

  return JS_SetModuleExportList(ctx, m, js_libuv_funcs,
                                countof(js_libuv_funcs));
}
static void JS_AddLibuv(JSContext *ctx) {
  JSModuleDef *m = JS_NewCModule(ctx, "libuv.so", js_libuv_init);
  JS_AddModuleExportList(ctx, m, js_libuv_funcs, countof(js_libuv_funcs));
}