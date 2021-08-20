/* File generated automatically by the QuickJS compiler. */
#include "test_libuv.h"
#include "fibConfig.h"
#include "quickjs/quickjs-libc.h"
#include "quickjs/quickjs.h"
#include "uv.h"
#define countof(x) (sizeof(x) / sizeof((x)[0]))
uv_loop_t *loop;
JSContext *ctx;
typedef struct {
  JSValue func;
} UVTimer;
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
static void time_job(uv_timer_t *handle) {
  /* 拿到js函数，解释执行 */
  UVTimer *th = (UVTimer *)handle->data;
  JSValue ret = JS_Call(ctx, th->func, JS_UNDEFINED, 0, NULL);
  /* 执行完毕释放掉函数以及返回结果 */
  JS_FreeValue(ctx, th->func);
  if (JS_IsException(ret))
    js_std_dump_error(ctx);
  JS_FreeValue(ctx, ret);
}
static JSValue js_clearTimeout(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv) {
  /* 将传入的64位整形转成uv_timer_t的指针 */                                
  u_int64_t timerHandle;
  if (JS_ToBigInt64(ctx, &timerHandle, argv[0]))
    return JS_EXCEPTION; 
  uv_timer_t *handle = (uv_timer_t *)timerHandle;
  /* 调用uv停掉这个uv_timer_t */
  if (uv_timer_stop(handle)) {
    return JS_EXCEPTION;
  }
  /* 释放掉指针存的JS函数引用 */
  UVTimer *th = handle->data;
  JS_FreeValue(ctx, th->func);
  return JS_UNDEFINED;
}
static JSValue js_setTimeout(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  int64_t delay;
  JSValueConst func;
  UVTimer *th;
  JSValue obj;

  /* 参数校验，保证参数正常 */
  func = argv[0];
  if (!JS_IsFunction(ctx, func))
    return JS_ThrowTypeError(ctx, "not a function");
  if (JS_ToInt64(ctx, &delay, argv[1]))
    return JS_EXCEPTION;

  uv_timer_t *time_job_req = malloc(sizeof(uv_timer_t));
  obj = JS_NewBigInt64(ctx, (u_int64_t)time_job_req);
  if (JS_IsException(obj))
    return obj;

  th = malloc(sizeof(UVTimer));
  if (!th) {
    /* 分配失败就释放包装指针给上层的JSValue */
    JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
  }
  /* 因为handle的data绑定的是指针，所以将引用的Func数据挂到堆上的结构体内存上 */
  th->func = JS_DupValue(ctx, func);
  time_job_req->data = th;

  /* 启动定时器处理time_job，time_job回调的时候在handle.data上回挂上需要执行的js函数
   */
  uv_timer_init(loop, time_job_req);
  uv_timer_start(time_job_req, time_job, delay, 0);

  return obj;
}
static const JSCFunctionListEntry js_libuv_funcs[] = {
    JS_CFUNC_DEF("setTimeout", 1, js_setTimeout),
    JS_CFUNC_DEF("clearTimeout", 1, js_clearTimeout),
};

/* 给ctx添加模块以及导出项 */
static int js_libuv_init(JSContext *ctx, JSModuleDef *m) {

  return JS_SetModuleExportList(ctx, m, js_libuv_funcs,
                                countof(js_libuv_funcs));
}
static void JS_AddLibuv(JSContext *ctx) {
  JSModuleDef *m = JS_NewCModule(ctx, "libuv.so", js_libuv_init);
  JS_AddModuleExportList(ctx, m, js_libuv_funcs, countof(js_libuv_funcs));
}