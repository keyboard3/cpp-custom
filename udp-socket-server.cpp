#include "sys/socket.h"
#include "iostream"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "string"
#include "stdlib.h"
#include "unistd.h"

#define PORT 8887
#define MAXLINE 1024
#define SA struct sockaddr

int main()
{
    //SOCKET_STREAM(TCP) SOCK_DGRAM(UDP)
    //文件描述符是进程中，存储指向内核中打开的文件列表数组的索引
    //Socket文件的inode是保存在内存的。它指向了Scoket在内核中的Socket结构
    struct sockaddr_in server_addr, client_addr;
    int server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //初始化server_addr client_addr内存空间
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    //给scoket指定ip地址和端口。一台机器多个网卡，一个网卡一个ip地址。可以选择监听all/1 ip地址
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //绑定IP地址和端口
    //网卡中断内核的时候，内核通过UDP头里的端口来索引到相应的内核socket文件
    if (bind(server_sockfd, (SA *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind faild");
        exit(EXIT_FAILURE);
    }

    char buffer[MAXLINE];
    socklen_t len = sizeof(client_addr);
    //用来接收远程主机经指定的socket 传来的数据, 并把数据存到由参数buffer 指向的内存空间
    //参数len 为可接收数据的最大长度. 参数flags 一般设0
    int n = recvfrom(server_sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (SA *)&client_addr, &len);
    buffer[n] = '\0';
    printf("receive from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    printf("content : %s\n", buffer);

    std::string hello = "Hello from server";
    //将服务端数据写入到client_addr
    sendto(server_sockfd, hello.c_str(), hello.length(), 0, (SA *)&client_addr, len);
    printf("Hello message sent.\n");

    return 0;
}