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

// 获取监听的套接字
int get_listen_fd(char const *port)
{
	// 1.创建套接字
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_fd == -1)
	{
		perror("socket error\n");
		exit(0);
	}
	// 2.准备自身的地址结构体
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // 地址族
	server_addr.sin_port = htons(atoi(port)); // 端口号
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip
	// 地址可重用
	int on=1;
	setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	// 3.绑定
	bind(tcp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	// 4.监听
	listen(tcp_fd, 1);
	// 5.返回套接字
	return tcp_fd;
}

int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("usage: ./demo port\n");
		return -1;
	}
	// 获取监听的套接字
	int tcp_fd = get_listen_fd(argv[1]);

	// 创建epoll句柄
	int epoll_fd = epoll_create(1);
	if(epoll_fd == -1)
	{
		printf("epoll_create error\n");
		return -1;
	}

	// 创建节点结构体，将tcp_fd上树
	struct epoll_event event;
	event.data.fd = tcp_fd;
	// 设置触发事件
	event.events = EPOLLIN|EPOLLET|EPOLLRDHUP;
	// 上树
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_fd, &event);

	int number; // 记录需要处理的事件数目
	struct epoll_event events[10000]; // 表示可以处理的事件总数
	// 存储内核返回的节点（有事件参数的节点）
	// 循环监听
	while(1)
	{
		// epoll_wait返回需要处理的事件数目
		number = epoll_wait(epoll_fd, events, 10000, -1);
		if(number < 0)
		{
			perror("epoll_wait error\n");
			return -1;
		}
		for(int i=0; i<number; i++) // 循环处理所有的需要处理事件
		{
			int sockfd = events[i].data.fd; // 获取有事件发生的套接字对象
			if(sockfd == tcp_fd) // 监听到客户端的连接
			{
				struct sockaddr_in client_addr; // 准备获取连接的客户端的信息
				socklen_t len =  sizeof(client_addr);
				memset(&client_addr, 0, len);
				int con_fd = accept(tcp_fd, (struct sockaddr*)&client_addr, &len); // 接收客户端的连接
				if(con_fd < 0)
				{
					printf("accept error\n");
					return -1;
				}
				// 如果接受成功：把客户端也上树，进行监听
				struct epoll_event event;
				event.data.fd = con_fd;
				// 设置事件
				event.events = EPOLLIN|EPOLLRDHUP;
				// 上树
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, con_fd, &event);
			}
			else if(events[i].events & EPOLLIN) // 处理客户端套接字上的读事件
			{
				char buf[1024] = {0};
				read(sockfd, buf, 1024);
				printf("当前获取的数据=%s\n", buf);
			}
			
		}
	}

	return 0;
}
