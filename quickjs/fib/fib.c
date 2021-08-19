/*
 * QuickJS: Example of C module
 *
 * Copyright (c) 2017-2018 Fabrice Bellard
 */
#include "quickjs/quickjs.h"
#include "fibConfig.h"
#define countof(x) (sizeof(x) / sizeof((x)[0]))

static int fib(int n) {
  if (n <= 0)
    return 0;
  else if (n == 1)
    return 1;
  else
    return fib(n - 1) + fib(n - 2);
}

/* 包装c++的fib，将JSValue的数据转型成c++支持的数据类型 */
static JSValue js_fib(JSContext *ctx, JSValueConst this_val, int argc,
                      JSValueConst *argv) {
  int n, res;
  if (JS_ToInt32(ctx, &n, argv[0]))
    return JS_EXCEPTION;
  res = fib(n);
  return JS_NewInt32(ctx, res);
}

/* 向模块的fib项导出包装之后的js_fib函数 */
static const JSCFunctionListEntry js_fib_funcs[] = {
    JS_CFUNC_DEF("fib", 1, js_fib),
};

static int js_fib_init(JSContext *ctx, JSModuleDef *m) {
  return JS_SetModuleExportList(ctx, m, js_fib_funcs, countof(js_fib_funcs));
}

/* 关闭了预加载，使用动态加载该模块 */
#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else 
/* 开启了预加载，在test_fib.c引擎启动的时候就会预加载这个库 */
#define JS_INIT_MODULE js_init_module_fib
#endif

JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name) {
  JSModuleDef *m;
  /* 向ctx中添加这个模块 */
  m = JS_NewCModule(ctx, module_name, js_fib_init);
  if (!m)
    return NULL;
  /* 向模块中添加导出项 */
  JS_AddModuleExportList(ctx, m, js_fib_funcs, countof(js_fib_funcs));
  return m;
}
