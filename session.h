//session.h
#ifndef _SESSION_H_
#define _SESSION_H_
#include"common.h"

// 会话层结构定义，实现会话函数，创建子进程，区分ftp和nobody进程，实现ftp与nobody之间的通讯连接，更改nobody进程getpwnam
typedef struct session
{
    /* 控制连接 */
    uid_t uid;  // 用户标识符，表示与该会话相关联的用户
    int ctrl_fd;    // 控制连接文件描述符，处理控制连接的输入输出
    int data_fd;    // 数据传输的文件描述符
    char cmdline[MAX_COMMAND_LINE]; // 存储完整命令行的字符数组，用户在控制连接上输入的完整命令行
    char cmd[MAX_COMMAND];  // 存储命令数组，存储用户在控制连接上输入的命令部分
    char arg[MAX_ARG];  // 存储命令参数的字符数组，存储用户在控制连接上输入的命令参数部分
    char rnfr_name;     // 存储命令参数的文件名信息
    /* 父子进程通道 */
    int parent_fd;
    int child_fd;
    /* 管理pasv模式监听套接字 */
    int pasv_listen_fd;

    int is_ascii;
    int restart_pos;    // 是否需要断点传输
    int bw_transfer_start_sec;
    int bw_transfer_start_usec;
    int bw_upload_rate_max;
    int bw_download_rate_max;


    struct sockaddr_in* port_addr;
}session_t;

void begin_session(session_t *sess);
#endif
