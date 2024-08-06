#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	// 定义集合
	fd_set set;
	// 清空集合
	FD_ZERO(&set);
	// 将标准输入加入集合
	FD_SET(0, &set);
	// 判断标准输入是否在集合中
	int ret = FD_ISSET(0, &set);
	if(ret == 1)
	{
		printf("0在集合\n");
	}
	// 判断标准输出是否在集合中
	ret = FD_ISSET(1, &set);
	if(ret == 1)
	{
		printf("1在集合\n");
	}

	return 0;
}
