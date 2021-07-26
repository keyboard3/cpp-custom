#include "sys/socket.h"
#include "iostream"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "string"
#include "unistd.h"

#define PORT 8887
#define MAXLINE 1024
#define SA struct sockaddr

int main()
{
    //SOCKET_STREAM(TCP) SOCK_DGRAM(UDP)
    //文件描述符是进程中，存储指向内核中打开的文件列表数组的索引
    //Socket文件的inode是保存在内存的。它指向了Scoket在内核中的Socket结构
    struct sockaddr_in server_addr;
    int client_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //初始化server_addr内存空间
    memset(&server_addr, 0, sizeof(server_addr));

    //给scoket指定ip地址和端口。一台机器多个网卡，一个网卡一个ip地址。可以选择监听all/1 ip地址
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //将客户端数据写入到server_addr
    std::string hello = "Hello from client";
    sendto(client_sockfd, hello.c_str(), hello.length(), 0, (SA *)&server_addr, sizeof(server_addr));
    printf("Hello message sent %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    //用来接收远程主机经指定的socket 传来的数据, 并把数据存到由参数buffer 指向的内存空间
    //参数len 为可接收数据的最大长度. 参数flags 一般设0
    char buffer[MAXLINE];
    socklen_t len = sizeof(server_addr);
    int n = recvfrom(client_sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (SA *)&server_addr, &len);
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);

    close(client_sockfd);
    return 0;
}