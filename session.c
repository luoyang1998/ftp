//session.c
#include"session.h"
#include"ftpproto.h"
#include"priparent.h"

void begin_session(session_t *sess)
{
    int sockfds[2]; // 保存两个套接字描述符
    socketpair(AF_UNIX, SOCK_STREAM, 0, sockfds);   // 创建一对已经连接的套接字，并将文件描述符存储
    pid_t pid;
    pid = fork();
    if(pid == -1)
        ERR_EXIT("fork");
    if(pid == 0) {
        //ftp 服务进程
        close(sockfds[0]);
        handle_child(sess);
    } else {
        //nobody 进程
        close(sockfds[1]);
        handle_parent(sess);
    }

    // 将进程更改为nobody进程
    // struct passwd *getpwnam(const char *name)
    // 指向passwd结构体指针，包含给定用户的 用户名、密码、用户ID、组ID、备注信息、主目录、登录时使用的shell
    struct passwd * pw = getpwnam("nobody");    // 通过用户名获取对应的用户信息
    if(pw == NULL)  // 找不到用户返回空
        ERR_EXIT("getpwnam");
    if(setegid(pw->pw_gid) < 0)
        ERR_EXIT("setegid");
    if(seteuid(pw->pw_uid) < 0)
        ERR_EXIT("seteuid");
}