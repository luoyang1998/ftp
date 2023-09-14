//privsock.c
#include"privsock.h"
// 内部同需要你协议实现

// 初始化内部进程间通信通道 父子进程之间建立一个私有的通信通道
void priv_sock_init(session_t *sess){
    int sockfds[2];     // 保存创建文件描述符
    // 创建UNIX域套接字对，创建成功后，一个保存到父进程，一个保存到子进程
    if(socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
        ERR_EXIT("socketpair");
    sess->child_fd = sockfds[1];
    sess->parent_fd = sockfds[0];
}
// 关闭内部进程通道
void priv_sock_close(session_t *sess){
    if (sess->parent_fd != -1) {
        close(sess->parent_fd);
        sess->parent_fd = -1;
    }
    if (sess->child_fd != -1){
        close(sess->child_fd);
        sess->child_fd = -1;
    }
}
// 初始化父子进程
void priv_sock_set_parent_context(session_t *sess){
    if (sess->child_fd != -1){
        close(sess->child_fd);
        sess->child_fd = -1;
    }
}
void priv_sock_set_child_context(session_t *sess){
    if (sess->parent_fd != -1) {//sess->parent_fd = sockfds[0];
        close(sess->parent_fd); //close(sockfds[0]);
        sess->parent_fd = -1;
    }
}
// 子进程发送命令 父进程接收命令
void priv_sock_send_cmd(int fd, char cmd){
    int ret;
    ret = writen(fd, &cmd, sizeof(cmd));
    if (ret != sizeof(cmd)){
        fprintf(stderr, "priv_sock_send_cmd error\n");
        exit(EXIT_FAILURE);
    }
}
char priv_sock_get_cmd(int fd){
    char res;
    int ret;
    ret = readn(fd, &res, sizeof(res));
    if (ret == 0){
        printf("ftp process exit\n");
        exit(EXIT_SUCCESS);
    }
    if (ret != sizeof(res)){
        fprintf(stderr, "priv_sock_get_cmd error\n");
        exit(EXIT_FAILURE);
    }
    return res;
}
// 父进程发送结果 子进程接收结果
void priv_sock_send_result(int fd, char res){
    int ret;
    ret = writen(fd, &res, sizeof(res));
    if (ret != sizeof(res)){
        fprintf(stderr, "priv_sock_send_result error\n");
        exit(EXIT_FAILURE);
    }
}
char priv_sock_get_result(int fd){
    char res;
    int ret;
    ret = readn(fd, &res, sizeof(res));
    if (ret != sizeof(res)){
        fprintf(stderr, "priv_sock_get_result error\n");
        exit(EXIT_FAILURE);
    }
    return res;
}
// 发送整数，接收整数
void priv_sock_send_int(int fd, int the_int){
    int ret;
    ret = writen(fd, &the_int, sizeof(the_int));
    if (ret != sizeof(the_int)){
        fprintf(stderr, "priv_sock_send_int error\n");
        exit(EXIT_FAILURE);
    }
}
int priv_sock_get_int(int fd){
    int the_int;
    int ret;
    ret = readn(fd, &the_int, sizeof(the_int));
    if (ret != sizeof(the_int)){
        fprintf(stderr, "priv_sock_get_int error\n");
        exit(EXIT_FAILURE);
    }
    return the_int;
}
// 发送字符串，接收字符串
void priv_sock_send_buf(int fd, const char *buf, unsigned int len){
    priv_sock_send_int(fd, (int)len);
    int ret = writen(fd, buf, len);
    if (ret != (int)len){
        fprintf(stderr, "priv_sock_send_buf error\n");
        exit(EXIT_FAILURE);
    }
}
void priv_sock_recv_buf(int fd, char *buf, unsigned int len){
    unsigned int recv_len = (unsigned int)priv_sock_get_int(fd);
    if (recv_len > len){
        fprintf(stderr, "priv_sock_recv_buf error\n");
        exit(EXIT_FAILURE);
    }
    int ret = readn(fd, buf, recv_len);
    if (ret != (int)recv_len){
        fprintf(stderr, "priv_sock_recv_buf error\n");
        exit(EXIT_FAILURE);
    }
}
//发送文件描述符 接收文件描述符
void priv_sock_send_fd(int sock_fd, int fd){
    send_fd(sock_fd, fd);
}
int priv_sock_recv_fd(int sock_fd){
    return recv_fd(sock_fd);
}

// 父进程操作函数 nobody协助进程绑定20口
void handle_parent(session_t *sess){
    minimize_privilege();
    char cmd;
    while(1){
        //读取ftp服务进程的数据并处理
        cmd = priv_sock_get_cmd(sess->parent_fd);
        switch(cmd){
            case PRIV_SOCK_GET_DATA_SOCK:
                privop_pasv_get_data_sock(sess);
                break;
            case PRIV_SOCK_PASV_ACTIVE:
                privop_pasv_active(sess);
                break;
            case PRIV_SOCK_PASV_LISTEN:
                privop_pasv_listen(sess);
                break;
            case PRIV_SOCK_PASV_ACCEPT:
                privop_pasv_accept(sess);
                break;
        }
    }
}

// 提升权限 将进程权限最小化
int capset(cap_user_header_t hdrp, const cap_user_data_t datap){
    return syscall(__NR_capset, hdrp, datap);
}
static void minimize_privilege(){
    struct passwd * pw = getpwnam("nobody");
    if(pw == NULL)
        ERR_EXIT("getpwnam");
    if(setegid(pw->pw_gid) < 0)     // 将有效组ID设置为nobodyID 降低进程权限
        ERR_EXIT("setegid");
    if(seteuid(pw->pw_uid) < 0)     // 将有效用户ID设置为nobodyID 降低进程权限
        ERR_EXIT("seteuid");
    struct __user_cap_header_struct cap_header; // 内核用户能力头部
    struct __user_cap_data_struct cap_data;     // 用户能力数据
    cap_header.version = _LINUX_CAPABILITY_VERSION_2;
    cap_header.pid = 0;
    __u32 mask = 0;
    mask |= (1 << CAP_NET_BIND_SERVICE);    // 允许进程绑定到小于1024的端口号
    cap_data.effective = cap_data.permitted = mask;
    cap_data.inheritable = 0;
    //int capset(cap_user_header_t hdrp, const cap_user_data_t datap);
    capset(&cap_header, &cap_data);
}

// 获取被动模式数据连接套接字
void privop_pasv_get_data_sock(session_t *sess){
    unsigned short port = (unsigned short)priv_sock_get_int(sess->parent_fd);
    char ip[16] = {0};
    priv_sock_recv_buf(sess->parent_fd, ip, sizeof(ip));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int fd = tcp_client(20);        // 创建客户端套接字
    if(connect_timeout(fd, &addr, 0) < 0){      // 连接FTP客户端
        priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_BAD); // 将执行结果发送给父进程
    } else {
        priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_OK);
        priv_sock_send_fd(sess->parent_fd, fd);     // 将数据连接套字发送给父进程
        close(fd);
    }
}
// 判断是否处于被动模式的激活状态
void privop_pasv_active(session_t *sess){
    int active = 0;
    if(sess->pasv_listen_fd != -1)
        active = 1;
    priv_sock_send_int(sess->parent_fd, active);
}
// 获取被动模式下的监听端口
void privop_pasv_listen(session_t *sess){
    char ip[16] = {0};
    getlocalip(ip);     // 获取IP
    sess->pasv_listen_fd = tcp_server(ip, 0);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr);
    // 获取套接字的本地地址信息
    // getsockname(待查询套接字文件描述符，指向保存返回的本地地址结构体指针，存储地址长度)
    if(getsockname(sess->pasv_listen_fd, (struct sockaddr*)&addr, &addrlen) < 0)
        ERR_EXIT("getsockname");
    unsigned short port = ntohs(addr.sin_port);
    priv_sock_send_int(sess->parent_fd, (int)port);
}
// 获取被动模式下的数据连接套接字
void privop_pasv_accept(session_t *sess){
    // 封装函数，在超时时间内接收客户端的连接请求 返回：新的连接套接字文件描述符
    //accept_timeout(监听到接字描述符，存储客户端地址结构体指针，长度指针，超时时间)
    int fd = accept_timeout(sess->pasv_listen_fd, 0, 0);
    if(fd < 0)
        priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_BAD);
    else{
        priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_OK);
        close(sess->pasv_listen_fd);
        sess->pasv_listen_fd = -1;
        priv_sock_send_fd(sess->parent_fd, fd);
        close(fd);
    }
}