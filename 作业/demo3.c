#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>

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

int main(int argc, char const *argv[])
{
    // 获取监听的套接字
    int tcp_fd = get_listen_fd(argv[1]);

    // 获取客户端的连接
    int con_fd;
    pid_t son;
    char buf[1024] = {0};
    int ret;
    while (1)
    {
        con_fd = accept(tcp_fd, NULL, NULL);
        if (con_fd == -1)
        {
            perror("accept error\n");
            return -1;
        }
        son = fork();
        if (son == 0)
        {
            while (1)
            {
                memset(buf, 0, sizeof(buf));
                ret = recv(con_fd, buf, sizeof(buf), 0);
                printf("收到的数据:%s---[%d字节]\n", buf, ret);
                if (strcmp(buf, "bye") == 0)
                    break;
            }
            close(con_fd);
            exit(1);
        }
    }
    while (1)
    {
        wait(NULL);
    }

    close(tcp_fd);

    return 0;
}