#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// 获取连接的套接字
int get_connect_fd(char const *port, char const *ip)
{
	// 1.创建套接字      // ipv4   tcp协议   扩展协议
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_fd == -1)
	{
		perror("socket error\n");
		return -1;
	}
	printf("tcp_fd=%d\n", tcp_fd);
	// 2.准备服务端的地址数据
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;		  // ipv4
	server_addr.sin_port = htons(atoi(port)); // 该端口为服务器端口
	// 服务端代码运行的设备的ip地址
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	// 3.发起连接请求
	printf("发起连接请求\n");
	int ret = connect(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret == -1)
		return -1;
	return tcp_fd;
}

// 线程任务函数，打印秒数
void *func(void *arg)
{
	int i = 1;
	while (1)
	{
		printf("%d\n", i);
		i++;
		sleep(1);
	}
}

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		printf("usage: ./demo port ip\n");
		return -1;
	}
	// 创建线程，打印秒数
	pthread_t tid;
	pthread_create(&tid, NULL, func, NULL);
	// 获取连接的套接字
	int tcp_fd = get_connect_fd(argv[1], argv[2]);

	// 新建集合
	fd_set set; // 定义集合变量
	// 获取集合中，最大值
	int max_fd = 0 > tcp_fd ? 0 : tcp_fd;
	struct timeval v;	  // 超时时间
	int sel_ret = 0;	  // 记录select的返回值
	char buf[1024] = {0}; // 做数据发送
	// 监听
	while (1)
	{
		FD_ZERO(&set); // 清空集合
		// 把套接字和标准输入加入集合
		FD_SET(STDIN_FILENO, &set);
		FD_SET(tcp_fd, &set);
		v.tv_sec = 5;
		v.tv_usec = 0;
		sel_ret = select(max_fd + 1, &set, NULL, NULL, &v);
		// 5秒内没有数据到达，或者没有发送数据，则超时
		if (sel_ret == 0)
		{
			printf("time out!\n");
		}
		// 有数据，进一步确认是发送还是接收
		if (FD_ISSET(0, &set) == 1) // 来自键盘
		{
			memset(buf, 0, 1024);
			fgets(buf, 1024, stdin);
			send(tcp_fd, buf, strlen(buf), 0);
		}
		if (FD_ISSET(tcp_fd, &set) == 1) // 来自套接字
		{
			memset(buf, 0, 1024);
			recv(tcp_fd, buf, 1024, 0);
			printf("来自客户端的数据：%s\n", buf);
		}
	}

	return 0;
}
