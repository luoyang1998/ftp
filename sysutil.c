//sysutil.c
#include"sysutil.h"

// 创建tcp套接字，并绑定到指定主机和端口
int tcp_server(const char *host, unsigned short port)
{
    int listenfd;
    if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        ERR_EXIT("tcp_server");
    }
    struct sockaddr_in servaddr;    // 存储IPv4 地址族、端口号、IP地址的结构体类型
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(port);
    int on = 1;
    // 设置套接字选项 (tcp套接字, 协议层级(通用), 设置具体选项 允许地址复用, 缓冲区大小) 确保在程序推出后能够快速重新启动
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))) < 0){
        ERR_EXIT("setsockopt");
    }
    // 将套接字绑定到指定主机和端口
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    // 开始监听连接请求，设置等待连接队列的最大长度 SOMAXCONN=0x7fffffff（2147483647）
    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }
    return listenfd;
}