#ifndef _COMMON_H_
#define _COMMON_H_
// 头文件定义

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
//#include<sys/socket.h>
//#include<netinet/in.h>
//#include<arpa/inet.h>
//#include<netdb.h>
#include<pwd.h>
#include<shadow.h>
#include <sys/stat.h>
#include <fcntl.h>

// 程序出现错误时，输出错误信息，终止程序执行
#define ERR_EXIT(m) \
    do{ \
    perror(m);\
    exit(EXIT_FAILURE);\
    }while(0)
#endif /* _COMMOM_H_ */