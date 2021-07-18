#include "sys/socket.h"
#include "iostream"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#define MYPORT 8887
#define QUEUE 20
#define BUFFER_SIZE 1024

int main()
{
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

    printf("监听%d端口", MYPORT);
    //linsten 成功返回0 出错返回-1
    if (listen(server_sockfd, QUEUE) == -1)
    {
        std::cerr << "listen";
        exit(1);
    }

    //客户端socket
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    std::cout << "等待客户端连接\n";
    //成功返回非负描述符 出错返回-1
    int conn = accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
    if (conn < 0)
    {
        std::cerr << "connect";
        exit(1);
    }
    std::cout << "客户端连接成功\n";

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        long len = recv(conn, buffer, sizeof(buffer), 0);
        //客户端发送exit或者异常结束时，退出
        if (strcmp(buffer, "exit\n") == 0 || len <= 0)
        {
            std::cerr << "出现异常";
            break;
        }
        std::cout << "来自客户端数据\n";
        fwrite(buffer, len, 1, stdout);
        send(conn, buffer, len, 0);
        std::cout << "发送给客户端数据\n";
        fwrite(buffer, len, 1, stdout);
    }
    close(conn);
    close(server_sockfd);
    return 0;
}