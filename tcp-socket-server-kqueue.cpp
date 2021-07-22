#include "fcntl.h"
#include "string"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "sys/event.h"
#include "stdlib.h"
#include "unistd.h"
//案例来自 https://github.com/jilieryuyi/kqueue-simple

#define PORT 8884
#define exit_if(r, ...)                                                \
    if (r)                                                             \
    {                                                                  \
        printf(__VA_ARGS__);                                           \
        printf("error no: %d error msg %s\n", errno, strerror(errno)); \
        exit(1);                                                       \
    }

const int kReadEvent = 1;
const int kWriteEvent = 2;
void setNoneBlock(int fd);
void updateEvents(int efd, int fd, int events, bool modify);
void handleAccept(int efd, int fd);
void handleRead(int efd, int fd);
void handleWrite(int efd, int fd);
void loop_once(int efd, int lfd, int waitms);

int main()
{
    int kqueue_fd = kqueue();
    exit_if(kqueue_fd < 0, "kqueue_create failed");
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    exit_if(listen_fd < 0, "socket failed");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    int r = bind(listen_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    exit_if(r, "bind to 0.0.0:%d failed %d %s", PORT, errno, strerror(errno));

    r = listen(listen_fd, 20);
    exit_if(r, "listen failed %d %s", errno, strerror(errno));
    printf("fd %d listening at %d\n", listen_fd, PORT);
    setNoneBlock(listen_fd);
    updateEvents(kqueue_fd, listen_fd, kReadEvent, false);
    while (1)
    {
        //实际应用应当注册信号处理函数，退出时清理资源
        loop_once(kqueue_fd, listen_fd, 10000);
    }
    return 0;
}

void loop_once(int efd, int lfd, int waitms)
{
    struct timespec timeout;
    timeout.tv_sec = waitms / 1000;
    timeout.tv_nsec = (waitms % 1000) * 1000 * 1000;
    const int kMaxEvents = 20;
    struct kevent activeEvs[kMaxEvents];
    //获取变动过的事件列表。一个socket一个事件, 等待超过10秒就返回0
    int n = kevent(efd, NULL, 0, activeEvs, kMaxEvents, &timeout);
    printf("change events return %d\n", n);
    for (int i = 0; i < n; i++)
    {
        //从事件中拿到socket文件
        int fd = (int)(intptr_t)activeEvs[i].udata;
        int events = activeEvs[i].filter;
        if (events == EVFILT_READ)
        {
            if (fd == lfd)
                //如果当前的socket是监听的socket。添加新连接socket到kqueue上
                handleAccept(efd, fd);
            else
                //如果是连接socket，就读取它的数据处理
                handleRead(efd, fd);
        }
        else if (events == EVFILT_WRITE)
        {
            //刚进入监听连接，此时状态为可写，kevent直接拿到该可写事件
            //我们这里无处理逻辑，直接删除该监听事件
            updateEvents(efd, fd, kReadEvent, true);
        }
        else
            exit_if(1, "unknow event");
    }
}

/**
 * 从可读的连接文件中读取内容，并直接将内容写入回去。沟通了直接关闭连接
 **/
void handleRead(int efd, int fd)
{
    char buf[4096];
    int n = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        printf("read %d bytes\n", n);
        int r = write(fd, buf, n);
        //实际应用中，写出数据可能会返回EAGAIN，此时应当监听可写事件，当可写时再把数据写出
        exit_if(r <= 0, "write error");
    }
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;
    exit_if(n < 0, "read error"); //实际应用中，n<0应当检查各类错误，如EINTR
    printf("fd %d closed\n", fd);
    close(fd);
}
/**
 * 从监听的socketfd拿到连接的socketfd，监听它的读写事件
 **/
void handleAccept(int efd, int fd)
{
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    //从监听socket中拿到新的已完成握手的连接
    int cfd = accept(fd, (struct sockaddr *)&raddr, &rsz);
    exit_if(cfd < 0, "accept failed");
    sockaddr_in peer, local;
    socklen_t alen = sizeof(peer);
    //检查连接是否已关闭
    int r = getpeername(cfd, (sockaddr *)&peer, &alen);
    exit_if(r < 0, "getpeername failed");
    printf("accpet a connection from %s\n", inet_ntoa(raddr.sin_addr));
    //设置新的连接文件读取时是非阻塞IO
    setNoneBlock(cfd);
    //向kqueue监听连接socket的可读可写事件
    updateEvents(efd, cfd, kReadEvent | kWriteEvent, false);
}
/**
 * 向kqueue注册一个新的监听事件
 **/
void updateEvents(int efd, int fd, int events, bool modify)
{
    struct kevent ev[2];
    int n = 0;
    if (events & kReadEvent)
    {
        //初始化kevent结构
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void *)(intptr_t)fd);
    }
    else if (modify)
    {
        //删除对可读的监听事件
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, (void *)(intptr_t)fd);
    }
    if (events & kWriteEvent)
    {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void *)(intptr_t)fd);
    }
    else if (modify)
    {
        //删除对可写的监听事件
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, (void *)(intptr_t)fd);
    }
    printf("%s fd %d events read %d write %d\n", modify ? "mod" : "add", fd, events & kReadEvent, events & kWriteEvent);
    int r = kevent(efd, ev, n, NULL, 0, NULL);
    exit_if(r, "kevent failed");
}

void setNoneBlock(int fd)
{
    //fcntl针对描述符提供控制
    //cmd:F_GETFL 返回文件访问模式和文件状态
    int flags = fcntl(fd, F_GETFL, 0);
    exit_if(flags < 0, "fcntl failed");
    //F_SETFL 给文件描述符设置
    //O_NONBLOCK 和 O_NDELAY 所产生的结果都是使I/O变成非阻塞模式(non-blocking),
    //在读取不到数据或是写入缓冲区已满会马上return,而不会阻塞等待
    //O_NDELAY读取不到数据会返回0，但读到文件末尾EOF时也会返回0
    //O_NONBLOCK在读取不到数据时会返回-1，并设置errno为EAGAIN
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    exit_if(r < 0, "fcntl failed");
}