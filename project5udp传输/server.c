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

int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("usage: ./demo port\n");
		return -1;
	}
	// 1.创建udp套接字
	int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_fd == -1)
	{
		perror("socket error\n");
		return -1;
	}
	// 2.准备自身的地址结构体
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // 地址族
	server_addr.sin_port = htons(atoi(argv[1])); // 端口
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip
	// 3.绑定
	bind(udp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	// 4.定义地址变量，存储对方地址数据
	struct sockaddr_in client_addr; // 用于存储客户端的地址
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, len);
	char buf[1024] = {0}; // 存储数据
	// 5.循环接收数据
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		int ret = recvfrom(udp_fd, buf, sizeof(buf), 0,
							(struct sockaddr*)&client_addr, &len);
		if(ret == 0)
		{
			printf("对方退出了\n");
			return -1;
		}
		//ip// 端口
		printf("<%s>,[%hu],%s\n", inet_ntoa(client_addr.sin_addr),
									ntohs(client_addr.sin_port),
									buf);
		if(strcmp(buf, "bye")==0)	
			break;
	}
	// 关闭套接字
	close(udp_fd);

	return 0;
}
