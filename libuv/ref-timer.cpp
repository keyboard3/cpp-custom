#include <stdio.h>
#include <uv.h>

uv_loop_t *loop;
uv_timer_t gc_req;
uv_timer_t fake_job_req;
void gc(uv_timer_t *handle);
void fake_job(uv_timer_t *handle);
int main()
{
  loop = uv_default_loop();
  // loop的运行依赖active的handles, 每当一个新的handle加入，loop的reference count就会+1。handle stop掉，reference count就-1
  // uv_ref和uv_unref是手动来改变它
  // uv_unref取消引用可以让loop在检查active的时候忽略它，但仍然可以被定时回调
  // 适用于定时进行垃圾回收，定时发送心跳包。当程序退出时，自动关闭
  uv_timer_init(loop, &gc_req);
  uv_unref((uv_handle_t *)&gc_req);

  uv_timer_start(&gc_req, gc, 0, 2000);

  // 可以用于定时处理TCP下载等事情，直到它处理完，loop就会退出
  uv_timer_init(loop, &fake_job_req);
  uv_timer_start(&fake_job_req, fake_job, 9000, 0);
  return uv_run(loop, UV_RUN_DEFAULT);
}

void gc(uv_timer_t *handle)
{
  fprintf(stderr, "Freeing unused objects\n");
}

void fake_job(uv_timer_t *handle)
{
  fprintf(stdout, "Fake job done\n");
}