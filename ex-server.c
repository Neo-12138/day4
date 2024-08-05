#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

// 数据结构体
typedef struct client_info
{
    int con_fd;
    struct sockaddr_in addr;
} client_info;

// 统筹结构体
typedef struct client_node
{
    struct client_info data;
    struct client_node *next;
} client_node;

int count = 0;
client_node *head;

// 初始化节点
client_node *node_init(client_info data)
{
    client_node *p = (client_node *)malloc(sizeof(client_node));
    if (p == NULL)
    {
        perror("malloc error\n");
        exit(0);
    }
    p->data = data;
    p->next = NULL;
    return p;
}

// 头插
void add_list_head(client_node *head, client_info data)
{
    client_node *p = node_init(data);
    p->next = head->next;
    head->next = p;
}

// 尾插
void add_list_tail(client_node *head, client_info data)
{
    client_node *pos = head;
    while (pos->next != NULL)
        pos = pos->next;
    add_list_head(head, data);
}

// 遍历链表
void show_list(client_node *head)
{
    if (head->next == NULL)
    {
        printf("链表为空，没有节点\n");
        return;
    }
    client_node *pos = head->next;
    for (; pos != NULL; pos = pos->next)
    {
        printf("client_fd = %d\n", pos->data.con_fd);
    }
    printf("\n");
    printf("总连接客户端为%d\n", count);
}

// 查找节点
client_node *find_list_node(client_node *head, int fd)
{
    if (head->next == NULL)
    {
        printf("链表为空，没有节点\n");
        return NULL;
    }
    client_node *pos = head->next;
    for (; pos != NULL; pos = pos->next)
    {
        if (pos->data.con_fd == fd)
        {
            return pos;
        }
    }
    printf("没找到这个节点\n");
    return NULL;
}

// 删除节点
int delete_list_node(client_node *head, int fd)
{
    if (head->next == NULL)
    {
        printf("链表为空，没有节点\n");
        return -1;
    }
    client_node *tmp = head;
    client_node *pos = head->next;
    for (; pos != NULL; pos = pos->next, tmp = tmp->next)
    {
        if (pos->data.con_fd == fd)
        {
            tmp->next = pos->next;
            free(pos);
            count--;
            return 0;
        }
    }
    printf("没找到这个节点\n");
    return -2;
}

int get_server_fd(const char *port)
{
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1)
    {
        perror("socket error\n");
        exit(0);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 自动获取ip地址
    // 地址可重用
    int on = 1; // 写一，表示启用
    setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    // 绑定
    bind(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // 监听
    listen(tcp_fd, 1);
    return tcp_fd;
}

// 客户端退出
void cli_exit(client_info *p)
{
    printf("<%s>,[%hu]:退出了\n", inet_ntoa(p->addr.sin_addr),
           ntohs(p->addr.sin_port));
    // 如果客户端退出，那么需要将客户端的信息从结构体数组中剔除
    // 遍历链表，通过con_fd，找到节点，删除节点
    delete_list_node(head, p->con_fd);
    count--;
}

// 线程处理函数
void *func(void *arg)
{
    client_info *p = (client_info *)arg;
    // 获取信息
    char buf[100] = {0};
    int ret;
    while (1)
    {
        memset(buf, 0, 100);
        // 读取数据
        ret = recv(p->con_fd, buf, sizeof(buf), 0);
        if (ret == 0 || strcmp(buf, "bye") == 0)
        {
            cli_exit(p);
            break;
        }
        // 打印  ip   端口  内容
        printf("<%s>,[%hu]:%s\n", inet_ntoa(p->addr.sin_addr),
               ntohs(p->addr.sin_port),
               buf);
    }
}

// 等待客户端链接
void wait_client_connect(client_node *head, int tcp_fd)
{
    int con_fd;
    client_info con_info;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    while (1)
    {
        memset(&client_addr, 0, len);
        con_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &len);
        if (con_fd == -1)
        {
            perror("accept error\n");
            exit(0);
        }
        // 给链表节点赋值
        memset(&con_info, 0, sizeof(con_info));
        con_info.con_fd = con_fd;
        con_info.addr = client_addr;
        add_list_tail(head, con_info);
        count++;
        pthread_t tid;
        pthread_create(&tid, NULL, func, (void *)&con_info);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./demo port\n");
        return -1;
    }
    client_info data;
    memset(&data, 0, sizeof(data));
    head = node_init(data); // 创建头节点

    int tcp_fd = get_server_fd(argv[1]);

    // 5.等待客户端连接
    printf("正在循环等待客户端的连接\n");

    wait_client_connect(head, tcp_fd);

    close(tcp_fd);

    return 0;
}
