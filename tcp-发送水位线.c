#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

// 线程任务函数
void *func(void *arg)
{
    int tcp_fd = *((int *)arg); // 获取套接字
    // 发送数据
    char buf[1024] = {0};
    while (1)
    {
        memset(buf, 0, 1024);
        read(tcp_fd, buf, 1024);
        printf("当前获取的数据=%s\n", buf);
        if (strcmp(buf, "bye") == 0)
            break;
    }
    // 关闭套接字
    close(tcp_fd);
    // 退出线程
    exit(0);
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./demo port ip\n");
        return -1;
    }
    // 1.创建套接字      // ipv4   tcp协议   扩展协议
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1)
    {
        perror("socket error\n");
        return -1;
    }
    // 2.准备服务端的地址数据
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;            // ipv4
    server_addr.sin_port = htons(atoi(argv[1])); // 该端口为服务器端口
    // 服务端代码运行的设备的ip地址
    inet_pton(AF_INET, argv[2], &server_addr.sin_addr);
    // 3.发起连接请求
    printf("发起连接请求\n");
    int ret = connect(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
    {
        perror("连接失败\n");
        return -1;
    }
    printf("连接成功\n");

    // 创建线程
    pthread_t tid;
    pthread_create(&tid, NULL, func, (void *)&tcp_fd);

    // 设置发送水位线
    int low = 10;
    setsockopt(tcp_fd, SOL_SOCKET, SO_SND LOWAT, &low, sizeof(low));

    // 4.发送数据
    char buf[1024] = {0};
    while (1)
    {
        memset(buf, 0, 1024);
        printf("请输入准备发送的数据\n");
        scanf("%s", buf);
        write(tcp_fd, buf, strlen(buf));
        if (strcmp(buf, "bye") == 0)
            break;
    }
    // 关闭套接字
    close(tcp_fd);

    return 0;
}
