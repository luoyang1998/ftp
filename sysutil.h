//sysutil.h
#ifndef _SYSUTIL_H_
#define _SYSUTIL_H_

// 公有系统工具函数定义
#include"common.h"

// tcp服务端
int tcp_server(const char *host, unsigned short port);
size_t readn(int fd, void *buf, size_t count);
size_t writen(int fd, const void *buf, size_t count);
size_t recv_peek(int sockfd, void *buf, size_t len);
size_t readline(int sockfd, void *buf, size_t maxline);
#endif
