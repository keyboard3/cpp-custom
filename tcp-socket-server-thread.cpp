#include "sys/socket.h"
#include "iostream"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
#define MYPORT 8884
#define MAX 80
#define BUFFER_SIZE 1024
#define SA struct sockaddr
//实例来源 https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
//因为TCP socket的通信，往往伴随着业务逻辑。accept和recv每次只能阻塞IO，每次处理一个连接(单线程)，导致上层消费连接太慢。rps太低
//C10K限制：一台机器要维护1万个连接，就要创建1万个进程或者线程，操作系统是无法承受的
void communication(int sockfd);
void *threadRun(void *sockfd);
int main()
{
    //SOCKET_STREAM(TCP) SOCK_DGRAM(UDP)
    //文件描述符是进程中，存储指向内核中打开的文件列表数组的索引
    //Socket文件的inode是保存在内存的。它指向了Scoket在内核中的Socket结构
    //这个结构两个队列 发送队列和接收队列。队列保存的是一个个缓存sk_buff。sk_buffer可以看到完整的数据包的结构
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1)
    {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created\n");
    //定义服务端的要绑定的IP地址和端口
    struct sockaddr_in server_sockaddr;
    bzero(&server_sockaddr, sizeof(server_sockaddr));

    //给scoket指定ip地址和端口。一台机器多个网卡，一个网卡一个ip地址。可以选择监听all/1 ip地址
    server_sockaddr.sin_family = AF_INET; //AF_INET(IPv4) AF_INET6(IPv6) AF_LOCAL(UNIX协议) AF_ROUTE(路由socket) AF_KEY(秘钥socket)
    server_sockaddr.sin_port = htons(MYPORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //绑定IP地址和端口
    //网卡中断内核的时候，内核通过TCP头里的端口来索引到相应的内核socket文件
    if (bind(server_sockfd, (SA *)&server_sockaddr, sizeof(server_sockaddr)) != 0)
    {
        perror("socket bind failed\n");
        exit(EXIT_FAILURE);
    }

    //进入TCP状态图中的LISTEN状态，等待客户端发起TCP连接数据包。
    //内核中的那两个队列最大连接数5
    printf("Server listening\n");
    if (listen(server_sockfd, 5) != 0)
    {
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        //内核中还未TCP的Socket结构维护了两个队列。已完成3次握手的队列(established)状态，未完成握手的队列(syn_rcvd)状态。
        //accept会获取一个已经完成握手的连接处理。还没有就阻塞等待。
        printf("process server acccept the client...\n");
        int conn_scokfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
        if (conn_scokfd < 0)
        {
            perror("server acccept failed\n");
            exit(EXIT_FAILURE);
        }
        printf("server acccepted the client %s:%d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //创建一个线程。新的线程会在进程的task列表会新创建一项。但很多资源如文件描述符列表，进程空间都是共享的。
        //无论是线程还是进程在内核中都是任务。线程不是一个完全由内核实现的机制，它是由内核态和用户态合作完成的。
        //pthread_create不是一个系统调用，是Glibc库函数。在用户态给线程创建独立的函数调用栈，然后内核中的新task内容按需求指向进程的task部分内容。
        printf("create thread\n");
        pthread_t thread;
        int ret = pthread_create(&thread, NULL, threadRun, (void *)&conn_scokfd);
        if (ret != 0)
        {
            printf("pthread_create error: error_code=%d\n", ret);
            exit(EXIT_FAILURE);
        }
        //进程就继续等待新的client连接
    }
    close(server_sockfd);
    return 0;
}

void *threadRun(void *sockfd)
{
    printf("new thread begin communication...\n");
    int *serverScokfdAddr = (int *)sockfd;
    communication(*serverScokfdAddr);
    pthread_exit(NULL);
}

void communication(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;)
    {
        bzero(buff, MAX);
        //从client端读取到小心到buff中
        read(sockfd, buff, sizeof(buff));
        std::string fromClientStr = std::string(buff);
        //打印从client端来的消息字符串内容
        printf("From client: %s\t To Server:", fromClientStr.c_str());
        // n = 0;
        //从命令行读取内容到buff中
        // while ((buff[n++] = getchar()) != '\n')
        //     ;
        std::string hello = "auto feedback " + fromClientStr + "";
        //将内容发挥到客户端
        write(sockfd, hello.c_str(), hello.length());
        //如果消息中包含exit内容就退出
        if (fromClientStr.find("exit") != fromClientStr.npos)
        {
            printf("Server Exit\n");
            break;
        }
    }
}