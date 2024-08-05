#include <sys/epoll.h>  // 包含 epoll 的头文件
#include <stdio.h>      // 包含标准输入输出头文件
#include <stdlib.h>     // 包含标准库头文件
#include <unistd.h>     // 包含 UNIX 标准头文件
#include <fcntl.h>      // 包含文件控制头文件
#include <errno.h>      // 包含错误号头文件
#include <string.h>     // 包含字符串处理头文件
#include <netinet/in.h> // 包含互联网地址族头文件
#include <arpa/inet.h>  // 包含互联网操作头文件

#define MAX_EVENTS 10 // 定义最大事件数为 10
#define PORT 8080     // 定义服务器端口为 8080

int main()
{
    int listen_fd, conn_fd, epoll_fd, nfds, n;
    struct sockaddr_in server_addr;            // 服务器地址结构体
    struct epoll_event ev, events[MAX_EVENTS]; // epoll 事件结构体数组

    // 创建监听套接字
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置监听套接字为非阻塞模式
    fcntl(listen_fd, F_SETFL, O_NONBLOCK);

    // 初始化服务器地址结构体
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;         // 地址族为 AF_INET（IPv4）
    server_addr.sin_addr.s_addr = INADDR_ANY; // 绑定到所有可用接口
    server_addr.sin_port = htons(PORT);       // 设置端口号并转换为网络字节序

    // 绑定监听套接字到服务器地址
    bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // 监听套接字，最大监听队列为 10
    listen(listen_fd, 10);

    // 创建 epoll 实例
    epoll_fd = epoll_create1(0);
    // 设置 epoll 事件为读事件
    ev.events = EPOLLIN;
    // 监听套接字加入 epoll 监控
    ev.data.fd = listen_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    while (1)
    {
        // 等待 epoll 事件的发生
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        // 处理每个发生的事件
        for (n = 0; n < nfds; ++n)
        {
            // 如果是监听套接字事件，表示有新的连接  
            if (events[n].data.fd == listen_fd)
            {
                // 接受新连接
                conn_fd = accept(listen_fd, NULL, NULL);
                // 设置新连接套接字为非阻塞模式
                fcntl(conn_fd, F_SETFL, O_NONBLOCK);
                // 设置 epoll 事件为读事件
                ev.events = EPOLLIN;
                // 新连接加入 epoll 监控
                ev.data.fd = conn_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
            }
            else
            {
                // 处理读事件
                char buffer[512];
                // 从发生读事件的套接字读取数据
                int len = read(events[n].data.fd, buffer, sizeof(buffer));
                // 如果读取到数据
                if (len > 0)
                {
                    // 打印接收到的数据
                    printf("Received: %s\n", buffer);
                }
                // 关闭发生读事件的套接字
                close(events[n].data.fd);
            }
        }
    }

    // 关闭监听套接字
    close(listen_fd);
    // 关闭 epoll 实例
    close(epoll_fd);
    return 0;
}
