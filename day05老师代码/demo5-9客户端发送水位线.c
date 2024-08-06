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

// 获取连接的套接字
int get_connect_fd(char const *port, char const *ip)
{
	// 1.创建套接字      // ipv4   tcp协议   扩展协议
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_fd == -1)
	{
		perror("socket error\n");
		return -1;
	}
	printf("tcp_fd=%d\n", tcp_fd);
	// 2.准备服务端的地址数据
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // ipv4
	server_addr.sin_port = htons(atoi(port)); // 该端口为服务器端口
	// 服务端代码运行的设备的ip地址
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	// 3.发起连接请求
	printf("发起连接请求\n");
	int ret = connect(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret == -1)
		return -1;
	return tcp_fd;
}


int main(int argc, char const *argv[])
{
	if(argc!=3)
	{
		printf("usage: ./demo port ip\n");
		return -1;
	}
	// 获取连接的套接字
	int tcp_fd = get_connect_fd(argv[1], argv[2]);
	
	// 设置水位线
	int low=10;
	setsockopt(tcp_fd, SOL_SOCKET, SO_SNDLOWAT, &low, sizeof(low));

	char buf[1024] = {0};
	int ret;

	while(1)
	{
		memset(buf, 0, 1024);
		printf("请输入需要发送的内容\n");
		scanf("%s", buf);
		// 发送数据
		ret = send(tcp_fd, buf, strlen(buf), 0);
		printf("ret=%d\n", ret);
		if(strcmp(buf, "bye")==0)
			break;
	}
	close(tcp_fd);

	return 0;
}
