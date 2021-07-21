#include "arpa/inet.h"
#include "stdlib.h"
#include "string"
#include "unistd.h"
#include "sys/socket.h"
/**
 * 因为TCP socket的通信，往往伴随着业务逻辑。accept和recv每次只能阻塞IO，每次处理一个连接(单线程)，导致上层消费连接太慢。rps太低
 * select 系统调用可以用来监听多个连接的文件描述符。当内核通知进程其中有可以读连接数据了
 * 然后就可以扫描这些连接的 socket，找到那个可读的 socket。读取sk_buff到进程指定的buffer。
 * 进行逻辑处理，然后调用 select 重新监听文件描述符集合
 **/
//示例来源 https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
#define TRUE 1
#define FALSE 0
#define PORT 8884
#define MAXLINE 1024
#define SA struct sockaddr

int main()
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[30] = {0}, max_clients = 30, activity, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[MAXLINE];
    fd_set readfds;

    const char *message = "ECHO Daemon v1.0\r\n";
    //创建监听的TCP socket
    master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    //设置监听socket允许多个连接，没有这个也能用
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    //绑定ip地址和端口到监听的socket上
    if (bind(master_socket, (SA *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d\n", PORT);

    //设置最多3个等待连接
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //接收进来的连接
    addrlen = sizeof(address);
    puts("Waiting for connections...");
    while (1)
    {
        printf("reset select all socket\n");
        //清空文件描述符集合
        FD_ZERO(&readfds);
        //重新添加监听socket
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        //将从监听的socket中获取的已完成连接的socket也加进来
        for (int i = 0; i < max_clients; i++)
        {
            //socket 描述符
            sd = client_socket[i];
            //如果当前socket有效添加到监听的描述符集合中
            if (sd > 0)
                FD_SET(sd, &readfds);

            //保证最高的文件描述符索引
            if (sd > max_sd)
                max_sd = sd;
        }

        //等待一个活跃变动的socket，因为timeout为null，所以会一直等待
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }
        //如果master socket变化，那么一定是进来了新的连接
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (SA *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            //打印新连接的地址
            printf("New connection, socket fd is %d, ip is : %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            //发送消息给新的连接
            if (send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }
            puts("Welcom message sent successfully");

            //添加新连接的socket到文件描述符数组中
            for (int i = 0; i < max_clients; i++)
            {
                //添加到空的位置上
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets ad %d\n", i);
                    break;
                }
            }
        }
        //如果是连接socket的IO操作
        for (int i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds))
            {
                //读取连接socket的进来的消息
                if ((valread = read(sd, buffer, MAXLINE)) == 0)
                {
                    //有些已经断开连接
                    getpeername(sd, (SA *)&address, (socklen_t *)&addrlen);
                    printf("Host disconnected, ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    //关闭这个socket，并标记文件描述符集合为0
                    close(sd);
                    client_socket[i] = 0;
                }
                else
                {
                    //将当前已经写入的socket读取清空，并将消息发送回去
                    buffer[valread] = '\0';
                    printf("from client: %s", buffer);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}