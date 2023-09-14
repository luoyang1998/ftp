//str.c
#include"str.h"

// 删除\r\n 删除末尾
void str_trim_crlf(char *str){
    char *p = &str[strlen(str)-1];
    while (*p == '\r' || *p == '\n')
        *p-- = '\0';
}
// 字符串分隔
void str_split(const char *str , char *left, char *right, char c){
    char *p = strchr(str, c);
    if (p == NULL)
        strcpy(left, str);
    else {
        strncpy(left, str, p-str);
        strcpy(right, p+1);
    }
}
// 判断字符串是否为空格
int str_all_space(const char *str){
    while (*str) {
        if (!isspace(*str))  // 判断是否为空格
            return 0;
        str++;
    }
    return 1;
}
// 将所有字符转化为大写形式
void str_upper(char *str){
    while (*str) {
        *str = toupper(*str);   // toupper()转化为大写字符
        str++;
    }
}