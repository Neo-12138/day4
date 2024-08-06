#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

// 获取监听的套接字
int get_listen_fd(char const *port)
{
    // 1.创建套接字
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1)
    {
        perror("socket error\n");
        exit(0);
    }
    // 2.准备自身的地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                // 地址族
    server_addr.sin_port = htons(atoi(port));        // 端口号
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip
    // 3.绑定
    bind(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // 4.监听
    listen(tcp_fd, 1);
    // 5.返回套接字
    return tcp_fd;
}
int i = 1;
// 线程任务函数，打印秒数
void *func(void *arg)
{
    while (1)
    {
        printf("%d\n", i);
        i++;
        sleep(1);
    }
}

int main(int argc, char const *argv[])
{
    // 创建线程，打印秒数
    pthread_t tid;
    pthread_create(&tid, NULL, func, NULL);
    // 获取监听的套接字
    int tcp_fd = get_listen_fd(argv[1]);
    // 等待对方的连接
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));

    // 等待客户端的连接
    int con_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &len);
    if (con_fd == -1)
    {
        perror("连接失败\n");
        return -1;
    }
    // 新建集合
    fd_set set; // 定义集合变量

    // 获取集合中，最大值
    int max_fd = 0 > con_fd ? 0 : con_fd;
    struct timeval v;     // 超时时间
    int sel_ret = 0;      // 记录select的返回值
    char buf[1024] = {0}; // 做数据发送
    // 监听
    while (1)
    {
        printf("==================\n");
        FD_ZERO(&set); // 清空集合
        // 把套接字和标准输入加入集合
        FD_SET(STDIN_FILENO, &set);
        FD_SET(con_fd, &set);
        v.tv_sec = 3;
        v.tv_usec = 0;
        sel_ret = select(max_fd + 1, &set, NULL, NULL, &v);
        // 5秒内没有数据到达，或者没有发送数据，则超时
        if (sel_ret == 0)
        {
            // printf("hello\n");
            send(con_fd, "hello\n", 7, 0);
        }
        // 有数据，进一步确认是发送还是接收
        if (FD_ISSET(0, &set) == 1) // 来自键盘
        {
            memset(buf, 0, 1024);
            fgets(buf, 1024, stdin);
            send(con_fd, buf, strlen(buf), 0);
            i = 1;
        }
        if (FD_ISSET(con_fd, &set) == 1) // 来自套接字
        {
            memset(buf, 0, 1024);
            recv(con_fd, buf, 1024, 0);
            printf("来自客户端的数据：%s\n", buf);
        }
        if (i == 10)
        {
            kill(getpid(), 2);
        }
    }
    return 0;
}
