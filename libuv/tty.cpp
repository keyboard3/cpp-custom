#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>
/**
 * 示例：http://docs.libuv.org/en/v1.x/guide/utilities.html
 * 文本终端支持使用标准命令支持基本的格式化。比如grep --colour
 * libuv提供uv_tty_t抽象和相应的方法来实现跨平台的转义码
*/
uv_loop_t *loop;
uv_tty_t tty;
int main() {
    loop = uv_default_loop();
    //tty关联标准输出文件描述符
    uv_tty_init(loop, &tty, STDOUT_FILENO, 0);
    //启用大多数tty格式
    uv_tty_set_mode(&tty, UV_TTY_MODE_NORMAL);
    //检查标准输出文件描述符是否是TTY(terminal)。如果将标准输出重定向到文件，则这个值就不是UV_TTY
    if (uv_guess_handle(1) == UV_TTY) {
        uv_write_t req;
        uv_buf_t buf;
        buf.base = "\033[41;37m";
        buf.len = strlen(buf.base);
        //向流写入数据
        uv_write(&req, (uv_stream_t*) &tty, &buf, 1, NULL);
    }

    uv_write_t req;
    uv_buf_t buf;
    buf.base = "Hello TTY\n";
    buf.len = strlen(buf.base);
    uv_write(&req, (uv_stream_t*) &tty, &buf, 1, NULL);
    //退出程序时，恢复terminal的状态
    uv_tty_reset_mode();
    return uv_run(loop, UV_RUN_DEFAULT);
}