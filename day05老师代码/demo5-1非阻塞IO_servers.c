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

// 定义结构体存储客户端的数据
typedef struct client{
	int con_fd[20]; // 存储已连接的客户端的套接字
	int count; // 统计连接的人数
}client;

// 初始化
int init_client(client *cli)
{
	for(int i=0; i<20; i++)
		cli->con_fd[i] = -1;
	cli->count = 0; // 初始连接的人数
}

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
	// 3.绑定
	bind(tcp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	// 4.监听
	listen(tcp_fd, 1);
	// 5.返回套接字
	return tcp_fd;
}

// 设置套接字的属性为非阻塞
int set_status(int sock_fd)
{
	// 获取当前套接字的原始属性
	int status = fcntl(sock_fd, F_GETFL);
	// 追加文件的非阻塞属性
	status |= O_NONBLOCK;
	// 设置当前套接字的新属性
	fcntl(sock_fd, F_SETFL, status);
}

int add_client(client *cli, int con_fd)
{
	// 判断是否已满20个用户
	if(cli->count >= 20)
	{
		printf("当前服务端已满\n");
		return -1;
	}
	// 如果不满，添加
	cli->con_fd[cli->count++] = con_fd;
	return 0;
}

int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("usage: ./demo port\n");
		return -1;
	}
	// 申请合法内存空间，存储客户端数据
	client *cli = calloc(1, sizeof(client));
	// 初始化该空间
	init_client(cli);
	// 获取监听的套接字
	int tcp_fd = get_listen_fd(argv[1]);
	// 设置套接字的非阻塞属性
	set_status(tcp_fd);
	// 准备变量存储客户端的地址数据
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, len);
	char buf[1024] = {0};
	int con_fd, ret;
	// 循环获取客户端的连接
	while(1)
	{
		con_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &len);
		// if(con_fd == -1) // 不会一直阻塞等待，不能写判断
		// {
		// 	perror("连接失败\n");
		// 	return -1;
		// }
		if(con_fd > 0) // 连接成功
		{
			printf("连接成功的用户<%s>:[%hu]\n", inet_ntoa(client_addr.sin_addr),
											ntohs(client_addr.sin_port));
			// 给给客户端分配的套接字设置非阻塞属性
			set_status(con_fd);
			// 将客户端添加到结构体中
			ret = add_client(cli, con_fd);
			if(ret == -1)
			{
				send(con_fd, "当前服务器已满，请稍后重试!", 40, 0);
				close(con_fd); // 关闭客户端的套接字
			}
		}
		// 查看数据
		for(int i=0; i<cli->count; i++)
		{
			memset(buf, 0, 1024);
			// 循环检测已连接的客户端的套接字，看是否有数据到达
			ret = recv(cli->con_fd[i], buf, sizeof(1024), 0);
			if(ret > 0)
			{
				printf("client:%d form %s\n", cli->con_fd[i], buf);
			}
		}

	}

	return 0;
}
