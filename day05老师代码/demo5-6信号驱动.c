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
#include <sys/epoll.h>
#include <signal.h>

int udp_fd;

// 自定义信号响应函数
void func(int sig)
{
	printf("当前捕捉的信号=%d\n", sig);
	// 存储数据的buf
	char buf[1024] = {0};
	// 定义存储客户端数据的结构体
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, len);
	// 接收数据
	recvfrom(udp_fd, buf, 1024, 0, (struct sockaddr*)&client_addr, &len);
	printf("当前<%s>:[%hu]:%s\n", inet_ntoa(client_addr.sin_addr), 
								ntohs(client_addr.sin_port),
								buf);
}

int main(int argc, char const *argv[])
{
	// 1.创建udp套接字
	udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_fd == -1)
	{
		perror("sock_fd error\n");
		return -1;
	}

	// 2.准备自身的地址数据
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET; // ipv4地址族
	server_addr.sin_port = htons(atoi(argv[1])); // 端口
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip
	// 3.将套接字与地址绑定
	bind(udp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	// 4.注册信号
	signal(SIGIO, func);
	// 设置信号的属主
	fcntl(udp_fd, F_SETOWN, getpid());
	// 5.设置套接字的信号触发模式
	int state = fcntl(udp_fd, F_GETFL);
	state |= O_ASYNC; // 追加信号触发
	fcntl(udp_fd, F_SETFL, state);

	while(1)
	{
		pause();
	}
	
	return 0;
}
