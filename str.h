//str.h
#ifndef _STR_H_
#define _STR_H_
#include"common.h"

// 处理字符串

// 去除\r\n
void str_trim_crlf(char *str);
// 字符串分隔
void str_split(const char *str , char *left, char *right, char c);
// 判断是否全是空白字符串
int str_all_space(const char *str);
// 字符串转化大写格式
void str_upper(char *str);
#endif
