#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/uv.h"
#include <curl/curl.h>

/**
 * 示例：http://docs.libuv.org/en/v1.x/guide/utilities.html
 * libuv: 我集成到了库文件中，免的直接安装
 * curl: 系统安装过，
 * 简单的下载管理，使用libcurl下载文件
 * 不会将所有控制权交给libcrul，而是使用libuv事件循环，并使用非阻塞的异步多接口
 * 当IO可读时通知libcurl操作
**/
uv_loop_t *loop;
CURLM *curl_handle;
uv_timer_t timeout;
typedef struct curl_context_s
{
  uv_poll_t poll_handle;
  curl_socket_t sockfd;
} curl_context_t;
curl_context_t *create_curl_context(curl_socket_t sockfd);
void curl_close_cb(uv_handle_t *handle);
void destroy_curl_context(curl_context_t *context);
void add_download(const char *url, int num);
void check_multi_info(void);
void curl_perform(uv_poll_t *req, int status, int events);
void on_timeout(uv_timer_t *req);
void start_timeout(CURLM *multi, long timeout_ms, void *userp);
int socket_callback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);
int main(int argc, char **argv)
{
  loop = uv_default_loop();

  if (argc <= 1)
    return 0;

  if (curl_global_init(CURL_GLOBAL_ALL))
  {
    fprintf(stderr, "Could not init cURL\n");
    return 1;
  }

  uv_timer_init(loop, &timeout);

  //初始化CURLM句柄，可用于监听多个输入。其他文档也称为多句柄
  curl_handle = curl_multi_init();
  //当curl_handle的socket状态发生变化，并且我们开始loop时，就会调用socket_callback
  curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, socket_callback);
  //curl_handle中的curl超时之后通知我们，意味着无法下载了,可以关闭监听了
  curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout);
  while (argc-- > 1)
  {
    //向CURLM句柄中添加单个curl句柄
    add_download(argv[argc], argc);
  }
  uv_run(loop, UV_RUN_DEFAULT);
  //清除CURLM句柄
  curl_multi_cleanup(curl_handle);
  return 0;
}
//start_timeout将在第一次被libcurl立刻调用。只是启动一个libuv定时器
void start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
  if (timeout_ms <= 0)
    timeout_ms = 1; /* 0 means directly call socket_action, but we'll do it in a bit */
  uv_timer_start(&timeout, on_timeout, timeout_ms, 0);
}
void on_timeout(uv_timer_t *req)
{
  int running_handles;
  //easyinterface是阻塞同步接口。curl_multi_perform+select是使用select来做的异步多路发送接口
  //遇到超时，就让curl进行内部的超时处理
  curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0, &running_handles);
  check_multi_info();
}
void add_download(const char *url, int num)
{
  char filename[50];
  sprintf(filename, "%d.download", num);
  FILE *file;

  file = fopen(filename, "w");
  if (file == NULL)
  {
    fprintf(stderr, "Error opening %s\n", filename);
    return;
  }
  //获得请求的curl句柄，指定最终写入磁盘的文件
  CURL *handle = curl_easy_init();
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
  curl_easy_setopt(handle, CURLOPT_URL, url);
  //向curl批处理会话中添加单独的curl句柄
  curl_multi_add_handle(curl_handle, handle);
  fprintf(stderr, "Added download %s -> %s\n", url, filename);
}
int socket_callback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp)
{
  curl_context_t *curl_context;
  //CURL_POLL_IN：等待下载的数据，文件描述符可读
  //CURL_POLL_OUT: 等待发送请求的数据，文件描述符可写
  if (action == CURL_POLL_IN || action == CURL_POLL_OUT)
  {
    if (socketp)
    {
      //将socket指针转成curl_context
      curl_context = (curl_context_t *)socketp;
    }
    else
    {
      //如果不存在socket指针，就创建一个新的
      curl_context = create_curl_context(s);
      //将socket关联到curl_handle内部的socket结构上
      curl_multi_assign(curl_handle, s, (void *)curl_context);
    }
  }

  switch (action)
  {
  case CURL_POLL_IN: //文件描述符可读
    //开始轮询文件描述符，UV_READABLE是二进制掩码的特定位数表示可读，一旦可读就执行curl_perform
    uv_poll_start(&curl_context->poll_handle, UV_READABLE, curl_perform);
    break;
  case CURL_POLL_OUT: //文件描述符可读
    uv_poll_start(&curl_context->poll_handle, UV_WRITABLE, curl_perform);
    break;
  case CURL_POLL_REMOVE: //libcurl不在使用这个文件描述符了
    if (socketp)
    {
      //清除该描述符在libcurl的数据结构
      uv_poll_stop(&((curl_context_t *)socketp)->poll_handle);
      destroy_curl_context((curl_context_t *)socketp);
      curl_multi_assign(curl_handle, s, NULL);
    }
    break;
  default:
    abort();
  }

  return 0;
}
void curl_perform(uv_poll_t *req, int status, int events)
{
  //文件描述符可操作说明已经过了网络阶段，不会超时了
  uv_timer_stop(&timeout);
  int running_handles;
  int flags = 0;
  if (status < 0)
    flags = CURL_CSELECT_ERR;
  if (!status && events & UV_READABLE)
    flags |= CURL_CSELECT_IN;
  if (!status && events & UV_WRITABLE)
    flags |= CURL_CSELECT_OUT;

  curl_context_t *context;

  context = (curl_context_t *)req;
  //让curl内部对指定的socket执行读写动作。操作完毕还会更新socket的状态
  curl_multi_socket_action(curl_handle, context->sockfd, flags, &running_handles);
  check_multi_info();
}
void check_multi_info(void)
{
  char *done_url;
  CURLMsg *message;
  int pending;
  //检查每个句柄的情况
  while ((message = curl_multi_info_read(curl_handle, &pending)))
  {
    switch (message->msg)
    {
    case CURLMSG_DONE:
      curl_easy_getinfo(message->easy_handle, CURLINFO_EFFECTIVE_URL,
                        &done_url);
      printf("%s DONE\n", done_url);
      //清除释放掉请求句柄
      curl_multi_remove_handle(curl_handle, message->easy_handle);
      curl_easy_cleanup(message->easy_handle);
      break;

    default:
      fprintf(stderr, "CURLMSG default\n");
      abort();
    }
  }
}
void destroy_curl_context(curl_context_t *context)
{
  uv_close((uv_handle_t *)&context->poll_handle, curl_close_cb);
}
void curl_close_cb(uv_handle_t *handle)
{
  curl_context_t *context = (curl_context_t *)handle->data;
  free(context);
}
curl_context_t *create_curl_context(curl_socket_t sockfd)
{
  curl_context_t *context;

  context = (curl_context_t *)malloc(sizeof *context);

  context->sockfd = sockfd;

  int r = uv_poll_init_socket(loop, &context->poll_handle, sockfd);
  assert(r == 0);
  context->poll_handle.data = context;

  return context;
}