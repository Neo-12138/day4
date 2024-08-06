#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned long ip_translation(const char *ip)
{
    int i = 0;
    char buf[20] = {0};
    strcpy(buf, ip);
    char *tmp[10] = {0};
    char *p = strtok(buf, ".");
    if (strlen(p) > 3 || atoi(p) < 0 || atoi(p) > 255)
    {
        printf("输入格式不对！\n");
        exit(-1);
    }
    tmp[i] = p;
    i++;
    while (1)
    {
        p = strtok(NULL, ".");
        if (p == NULL)
            break;
        if (strlen(p) > 3 || atoi(p) < 0 || atoi(p) > 255)
        {
            printf("输入格式不对！\n");
            exit(-1);
        }
        tmp[i] = p;
        i++;
    }
    char dest[32] = {0};
    for (int j = 0; j < i; j++)
        strcat(dest, tmp[j]);

    return atol(dest);
}

// 将点分制的字符串转换成unsigned int类型
int main(int argc, char const *argv[])
{
    unsigned long buf = ip_translation(argv[1]);
    printf("转换的数据为%lu\n", buf);
    return 0;
}
