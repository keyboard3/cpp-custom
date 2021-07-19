#include "sys/socket.h"
#include "iostream"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#define MYPORT 8884
#define MAX 80
#define SA struct sockaddr
//实例来源 https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
void communication(int sockfd);
int main()
{
    //SOCKET_STREAM(TCP) SOCK_DGRAM(UDP)
    //文件描述符是进程中，存储指向内核中打开的文件列表数组的索引
    //Socket文件的inode是保存在内存的。它指向了Scoket在内核中的Socket结构
    //这个结构两个队列 发送队列和接收队列。队列保存的是一个个缓存sk_buff。sk_buffer可以看到完整的数据包的结构
    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket successfully created..\n");
    //定义服务端的要绑定的IP地址和端口
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET; //AF_INET(IPv4) AF_INET6(IPv6) AF_LOCAL(UNIX协议) AF_ROUTE(路由socket) AF_KEY(秘钥socket)
    server_sockaddr.sin_port = htons(MYPORT);
    server_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //服务端已经进入LISTEN状态，客户端可以通过connect函数发起连接
    //指明服务端的IP地址和端口，发起三次握手
    //内核会给客户端分配一个临时的端口。握手成功，服务端accept会返回一个已连接的Socket
    if (connect(client_sockfd, (SA *)&server_sockaddr, sizeof(server_sockaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    //连接成功之后，客户端通过write函数向Socket文件中写数据
    printf("connected to the server..\n");
    communication(client_sockfd);
    //TCP连接 4次挥手关闭
    printf("connect close\n");
    close(client_sockfd);
}

void communication(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;)
    {
        //清空缓存
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        //命令行输入字符串消息发送到服务端
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        //读出来自服务端的字符串消息
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        //如果消息中包含exit就退出
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
}