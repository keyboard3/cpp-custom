#include "sys/types.h"
#include "sys/socket.h"
#include "netdb.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/epoll.h" //linux环境
#include "arpa/inet.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"

#define DEFAULT_PORT 8080
#define MAX_CONN 16
#define MAX_EVENTS 32
#define BUF_SIZE 16
#define MAX_LINE 256
#define AF struct sockaddr
/**
 * epoll 可以通过注册 callback 函数的方式，监听某个文件描述符发生变化，就会触发相应的事件
 * epoll_create 创建一个 epoll 对象，它的文件描述符对应了内核中的红黑树的结构
 * epoll_ctl 添加一个 Socket 时，向红黑树添加一个节点，这个节点存储了关心这个 Socket 变化事件的回调函数列表
 * 当 Socket 的事件列表发生变化可以找到挂的 epoll 对象，然后从 epoll 中找到回调并触发它
 **/
//实例来源 https://github.com/onestraw/epoll-example/blob/master/epoll.c
static void set_sockaddr(struct sockaddr_in *addr);
static int setnonblocking(int sockfd);
static void epoll_ctl_add(int epfd, int fd, uint32_t events);

int main()
{
    int i, n, epfd, nfds;
    int listen_sock, conn_sock;
    int socklen;
    char buf[BUF_SIZE];
    struct sockaddr_in srv_addr, cli_addr;
    struct epoll_event events[MAX_EVENTS];

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    set_sockaddr(&srv_addr);
    bind(listen_sock, (AF *)&srv_addr, sizeof(srv_addr));

    setnonblocking(listen_sock);
    listen(listen_sock, MAX_CONN);

    epfd = epoll_create(1);
    epoll_ctl_add(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);

    socklen = sizeof(cli_addr);
    while (1)
    {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listen_sock)
            {
                conn_sock = accept(listen_sock, (AF *)&cli_addr, &socklen);
                inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr), buf, sizeof(cli_addr));
                printf("[+] connected with %s:%d\n", buf, ntohs(cli_addr.sin_port));

                setnonblocking(conn_sock);
                epoll_ctl_add(epfd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
            }
            else if (events[i].events & EPOLLIN)
            {
                while (1)
                {
                    bzero(buf, sizeof(buf));
                    n = read(events[i].data.fd, buf, sizeof(buf));
                    if (n <= 0 || errno == EAGAIN)
                        break;
                    else
                    {
                        printf("[+] data: %s\n", buf);
                        write(events[i].data.fd, buf, strlen(buf));
                    }
                }
            }
            else
                printf("[+] uexpected\n");
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP))
            {
                printf("[+] connection closed\n");
                epoll_ctl(epfd, EPOLL_CTL_DEL,
                          events[i].data.fd, NULL);
                close(events[i].data.fd);
                continue;
            }
        }
    }
}

static void set_sockaddr(struct sockaddr_in *addr)
{
    bzero((char *)addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(DEFAULT_PORT);
}

static int setnonblocking(int sockfd)
{
    int ret = fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
    if (ret == -1)
        return -1;
    return 0;
}

static void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll_ctl()\n");
        exit(1);
    }
}