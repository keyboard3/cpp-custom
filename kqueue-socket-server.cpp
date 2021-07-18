#include "sys/socket.h"
#include "iostream"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/event.h"
#define MYPORT 8887
#define QUEUE 20
#define BUFFER_SIZE 1024
#define FD_NUM 100 //要监听多少个文件描述符

int main()
{
    printf("启动3\n");
    //定义sockfd AF_INET(IPv4) AF_INET6(IPv6) AF_LOCAL(UNIX协议) AF_ROUTE(路由socket) AF_KEY(秘钥socket)
    //SOCKET_STREAM(字节流socket) SOCK_DGRAM
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //定义scokaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(MYPORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind 成功返回0 出错返回-1
    if (bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        std::cerr << "bind";
        exit(1);
    }

    printf("监听%d端口\n", MYPORT);
    //linsten 成功返回0 出错返回-1
    if (listen(server_sockfd, QUEUE) == -1)
    {
        std::cerr << "listen";
        exit(1);
    }

    //创建一个消息队列并返回kequeue描述符
    int kq = kqueue();
    struct kevent change_list;        //想要监听的事件列表
    struct kevent event_list[FD_NUM]; //用于kevent返回的事件列表
    char buffer[BUFFER_SIZE];
    int nevents;
    //监听sock读事件 初始化kevent结构的宏
    EV_SET(&change_list, server_sockfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    while (1)
    {
        std::cout << "new loop...\n";
        //已经就绪的文件描述符数量
        nevents = kevent(kq, &change_list, 1, event_list, 2, NULL);
        if (nevents < 0)
        {
            std::cerr << "kevent error.\n";
        }
        else if (nevents > 0)
        {
            printf("get events number: %d\n", nevents);
            for (int i = 0; i < nevents; i++)
            {
                printf("loop index: %d\n", i);
                struct kevent event = event_list[i]; //监听事件的event数据结构
                int clientfd = (int)event.ident;     //监听描述符
                //标识该监听描述符出错
                if (event.flags & EV_ERROR)
                {
                    close(clientfd);
                    std::cout << "EV_ERROR:" << event_list[i].data << std::endl;
                }

                //表示sock有新的连接
                if (clientfd == server_sockfd)
                {
                    std::cout << "new connection\n";
                    struct sockaddr_in client_addr;
                    socklen_t length = sizeof(client_addr);
                    int new_fd = accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
                    long len = recv(new_fd, buffer, sizeof(buffer), 0);
                    char remote[INET_ADDRSTRLEN];
                    printf("connected with ip: %s, port: %d\n",
                           inet_ntop(AF_INET, &client_addr.sin_addr, remote, INET_ADDRSTRLEN),
                           ntohs(client_addr.sin_port));
                    fwrite(buffer, len, 1, stdout);
                    send(new_fd, buffer, len, 0);
                }
            }
        }
    }
    return 0;
}