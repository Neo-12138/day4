#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(0, &set);
    int ret = FD_ISSET(0, &set);
    if (ret == 1)
    {
        printf("0在集合中\n");
    }
    else
    {
        printf("0不再集合中\n");
    }
    ret = FD_ISSET(1, &set);
    if (ret == 1)
    {
        printf("1在集合中\n");
    }
    else
    {
        printf("1不再集合中\n");
    }
    return 0;
}
