//bitftp.c
#include"bitftp.h"
//void MyBitFtp(int argc, char *argv[])

// 通过监听指定IP地址和端口号的方式实现等待客户连接，创建子进程处理信连接的客户请求
void MyBitFtp() {
    if (getuid() != 0) {  // 获取当前进程的用户ID，判断是否为root用户(=0)
        printf("miniftp : must be started as root.\n");
        exit(EXIT_FAILURE);
    }
    session_t sess = {
            /* 控制连接 */
            -1,
            /* 父子进程通道 */
            -1, -1
    };
    int listenfd = tcp_server("192.168.232.10", 8888);  // 创建TCP服务器套接字，绑定到指定IP地址和端口上
    pid_t pid;  // 用于保护子进程进程ID
    int conn;
    struct sockaddr_in addrcli; // 客户端结构体地址信息
    socklen_t addrlen;  // 结构体大小

    // 等待客户连接
    while (1) {
        // accept(): 接受客户端的连接请求，并创建新的套接字与客户端进行通信 (已经监听套接字，存储客户端地址信息，结构体大小)
        // 阻塞当前进程，直到有客户端连接请求到达，返回新的套接字文件描述符
        if ((conn = accept(listenfd, (struct sockaddr *) &addrcli, &addrlen)) < 0) {
            ERR_EXIT("accept_timeout");
        }

        // fork(): 父进程创建子进程;返回值0: 当前进程执行为子进程; >0: 执行进程父进程，并返回子进程ID; <0: 创建子进程失败
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
        if (pid == 0) {
            close(listenfd);
            sess.ctrl_fd = conn;
            begin_session(&sess);
        } else {
            close(conn);
        }
    }
    close(listenfd);

}