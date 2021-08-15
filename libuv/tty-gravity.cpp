#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>
/**
 * 示例：http://docs.libuv.org/en/v1.x/guide/utilities.html
 * 这个使用函数和字符位置的转义码做了一个小动画
*/
uv_loop_t *loop;
uv_tty_t tty;
uv_timer_t tick;
uv_write_t write_req;
int width, height;
int pos = 0;
char *message = "  Hello TTY  ";
void update(uv_timer_t *req);

int main()
{
  loop = uv_default_loop();

  uv_tty_init(loop, &tty, STDOUT_FILENO, 0);
  uv_tty_set_mode(&tty, 0);
  //用来得到terminal的宽和高
  if (uv_tty_get_winsize(&tty, &width, &height))
  {
    fprintf(stderr, "Could not get TTY information\n");
    //读取失败恢复terminal状态
    uv_tty_reset_mode();
    return 1;
  }

  fprintf(stderr, "Width %d, height %d\n", width, height);
  uv_timer_init(loop, &tick);
  //每200毫秒更新一下terminal的屏幕
  uv_timer_start(&tick, update, 200, 200);
  return uv_run(loop, UV_RUN_DEFAULT);
}

void update(uv_timer_t *req)
{
  char data[500];

  uv_buf_t buf;
  buf.base = data;
  //2 J 清除部分屏幕，2是整个屏幕
  //H 移动光标到指定位置，默认是左上
  //n B 移动光标到底n行
  //n C 移动光标到右边的底n列
  //m 遵守字符串的显示规则，(40+2)绿色背景，(30+7)白色文本
  buf.len = sprintf(data, "\033[2J\033[H\033[%dB\033[%luC\033[42;37m%s",
                    pos,
                    (unsigned long)(width - strlen(message)) / 2,
                    message);
  uv_write(&write_req, (uv_stream_t *)&tty, &buf, 1, NULL);

  pos++;
  //到输出位置超过高度，就停止输出
  if (pos > height)
  {
    uv_tty_reset_mode();
    uv_timer_stop(&tick);
  }
}