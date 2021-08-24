#include "errno.h"
#include "limits.h"
#include "quickjs/quickjs-libc.h"
#include "stdlib.h"
void perror_exit(int errcode, const char *s) {
  fflush(stdout);
  fprintf(stderr, "%s: ", "evalJS");
  perror(s);
  exit(errcode);
}
uint8_t *lib_js_load_file(JSContext *ctx, size_t *pbuf_len, const char *filename) {
  FILE *f;
  uint8_t *buf;
  size_t buf_len;
  long lret;

  f = fopen(filename, "rb");
  if (!f)
    return NULL;
  if (fseek(f, 0, SEEK_END) < 0)
    goto fail;
  lret = ftell(f);
  if (lret < 0)
    goto fail;
  /* XXX: on Linux, ftell() return LONG_MAX for directories */
  if (lret == LONG_MAX) {
    errno = EISDIR;
    goto fail;
  }
  buf_len = lret;
  if (fseek(f, 0, SEEK_SET) < 0)
    goto fail;
  if (ctx)
    buf = (uint8_t *)js_malloc(ctx, buf_len + 1);
  else
    buf = (uint8_t *)malloc(buf_len + 1);
  if (!buf)
    goto fail;
  if (fread(buf, 1, buf_len, f) != buf_len) {
    errno = EIO;
    if (ctx)
      js_free(ctx, buf);
    else
      free(buf);
  fail:
    fclose(f);
    return NULL;
  }
  buf[buf_len] = '\0';
  fclose(f);
  *pbuf_len = buf_len;
  return buf;
}

 char *load_file(const char *filename, size_t *lenp) {
  char *buf;
  size_t buf_len;
  buf = (char *)lib_js_load_file(NULL, &buf_len, filename);
  if (!buf)
    perror_exit(1, filename);
  if (lenp)
    *lenp = buf_len;
  return buf;
}

/* load and evaluate a file */
 JSValue js_loadScript(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  uint8_t *buf;
  const char *filename;
  JSValue ret;
  size_t buf_len;

  filename = JS_ToCString(ctx, argv[0]);
  if (!filename)
    return JS_EXCEPTION;
  buf = lib_js_load_file(ctx, &buf_len, filename);
  if (!buf) {
    JS_ThrowReferenceError(ctx, "could not load '%s'", filename);
    JS_FreeCString(ctx, filename);
    return JS_EXCEPTION;
  }
  ret = JS_Eval(ctx, (char *)buf, buf_len, filename, JS_EVAL_TYPE_GLOBAL);
  js_free(ctx, buf);
  JS_FreeCString(ctx, filename);
  return ret;
}