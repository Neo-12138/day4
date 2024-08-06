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
	if (argc != 3)
	{
		printf("usage: ./demo port ip\n");
		return -1;
	}
	// 1.创建udp套接字
	int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_fd == -1)
	{
		perror("socket error\n");
		return -1;
	}
	// 2.准备服务器的地址结构体
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;			 // 地址族
	server_addr.sin_port = htons(atoi(argv[1])); // 端口
	inet_pton(AF_INET, argv[2], &server_addr.sin_addr);

	// 4.定义变量，准备发送数据
	char buf[1024] = {0}; // 存储数据
	// 5.循环发送数据
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		// 提示用户
		printf("请输入需要发送的数据\n");
		scanf("%s", buf);
		// 发送数据
		int ret = sendto(udp_fd, buf, strlen(buf), 0,
						 (struct sockaddr *)&server_addr, sizeof(server_addr));
	}
	// 关闭套接字
	close(udp_fd);

	return 0;
}
