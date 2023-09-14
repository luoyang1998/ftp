//ftpproto.c
#include"ftpproto.h"

// 发送FTP响应信息
// 会话结构体指针、响应码、响应字符串
void ftp_reply(session_t *sess, int code, const char *msg) {
    char reply[512];
    sprintf(reply, "%d %s\r\n", code, msg);
//    writen(sess->ctrl_fd, reply, strlen(reply));
    write(sess->ctrl_fd, reply, strlen(reply));
}

// 处理ftp服务器进程
void handle_child(session_t *sess) {
    //向客户端发送欢迎消息
//    send(sess->ctrl_fd, "220 (miniftpd v1.0)\n\r", strlen("220(miniftpd v1.0)\n\r"), 0);
    ftp_reply(sess, FTP_GREET, "(miniftpd v1.0)");
    int ret;
    int i;
    while(1) {
        //不停的从客户端读取数据并处理

        // 将sess结构体中的命令信息置0
        // memset(要设置的内存块的起始地址，设置的值，设置字节数) 返回：指向被设置内存块的指针
        memset(sess->cmdline, 0, MAX_COMMAND_LINE);
        memset(sess->cmd, 0, MAX_COMMAND);
        memset(sess->arg, 0, MAX_ARG);

        // 从控制连接中读取cmd命令；返回-1 发生错误并退出；返回0 客户端关闭，进程终止
//        ret = readline(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);
        ret = read(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);
        if(ret == -1)
            ERR_EXIT("readline");
        else if(ret == 0)
            exit(EXIT_SUCCESS);

        str_trim_crlf(sess->cmdline);
        str_split(sess->cmdline, sess->cmd, sess->arg, ' ');

        //读取客户端的命令，并调用相应的函数进行处理
        int table_size = sizeof(ctrl_cmds) / sizeof(ftpcmd_t);
        for(i=0; i<table_size; ++i) {
            if(strcmp(ctrl_cmds[i].cmd, sess->cmd) == 0) {
                if(ctrl_cmds[i].cmd_handler != NULL) {
                    ctrl_cmds[i].cmd_handler(sess);
                } else {
                    ftp_reply(sess, FTP_COMMANDNOTIMPL, "Unimplement command.");
                }
                break;
            }
        }
        if(i >= table_size) {
            ftp_reply(sess, FTP_BADCMD, "Unknown command.");
        }
    }
}
// 登录验证
// 验证输入用户名是否有效，有效记录ID并输入密码，无效发送错误信息
void do_user(session_t *sess)
{
    struct passwd *pwd = getpwnam(sess->arg);
    if(pwd == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "0 Login incorrect.");
        return;
    }
    sess->uid = pwd->pw_uid;    // 赋值给sess 记录当前用户身份信息
    ftp_reply(sess, FTP_GIVEPWORD, "Please specify the password.");
}
// 验证登录用户密码 成功切换登录用户目录
void do_pass(session_t *sess)
{
    struct passwd *pwd = getpwuid(sess->uid);
    if(pwd == NULL) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }
    struct spwd *spd = getspnam(pwd->pw_name);
    if(spd == NULL) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }
    char *encrypted_pwd = crypt(sess->arg, spd->sp_pwdp);   // 密码加密，(代价密字符串，用于生成密文salt值)
    if(strcmp(encrypted_pwd, spd->sp_pwdp) != 0) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }
    setegid(pwd->pw_gid);   // 有效组ID设置为
    seteuid(pwd->pw_uid);   // 用户ID设置
    chdir(pwd->pw_dir);     // 切换为用户目录
    ftp_reply(sess, FTP_LOGINOK, "Login successful.");
}
// 实现PORT模式响应
// 通过指定IP地址和端口号告知服务器用于数据传输的地址和端口
void do_port(session_t *sess) {
    unsigned int v[6];  // 存储客户端传来的IP地址和端口号
    sscanf(sess->arg, "%u,%u,%u,%u,%u,%u", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
    sess->port_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    unsigned char *p = (unsigned char*)&sess->port_addr->sin_port; // 端口号存储结构体中
    p[0] = v[4];
    p[1] = v[5];
    p = (unsigned char*)&sess->port_addr->sin_addr; // IP地址存储结构体中
    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
    p[3] = v[3];
    ftp_reply(sess, FTP_PORTOK, "PORT command successful. Consider using PASV.");
}

// 返回特性列表给客户端 供客户端查看
void do_feat(session_t *sess) {
    ftp_reply(sess, FTP_FEAT, "End");
}
// 获取当前目录信息 发送给客户端 并发送响应码
void do_pwd(session_t *sess) {
    char current_dir[256];
    if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
        char reply[512];
        sprintf(reply, FTP_PWDOK + " \"%s\" is the current directory.", current_dir);
    } else {
        ftp_reply(sess, FTP_BADCMD, "Failed to get current directory.");
    }
    ftp_reply(sess, FTP_PWDOK, "Print current directory successfully.");
}

//数据连接获取 返回 是否获取成功
int port_active(session_t *sess) {
    if(sess->port_addr) {
        if(pasv_active(sess)) {
            fprintf(stderr, "both port an pasv are active");
            exit(EXIT_FAILURE);
        }
        return 1;
    }
    return 0;
}
int pasv_active(session_t *sess) {
    if(sess->pasv_listen_fd != -1) {
        if(port_active(sess)) {
            fprintf(stderr, "both port an pasv are active");
            exit(EXIT_FAILURE);
        }
        return 1;
    }
    return 0;
}
// 获取数据传输的文件描述符
int get_transfer_fd(session_t *sess) {
    if(!port_active(sess) && !pasv_active(sess)) { // 判断采用数据传输连接方式
        ftp_reply(sess, FTP_BADSENDCONN, "Use PORT or PASV first.");
        return 0;
    }
    int ret = 1;
    //port
    if(port_active(sess)){
        int fd = tcp_client();  // 创建TCP客户端套接字连接
        if(connect(fd, (struct sockaddr*)sess->port_addr, sizeof(struct sockaddr_in)) < 0) {
            ret = 0;
        }
        else {
            sess->data_fd = fd;
            ret = 1;
        }
    }
    // pasv
    if(pasv_active(sess)){
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(struct sockaddr_in);
        int fd = accept(sess->pasv_listen_fd, (struct sockaddr*)&addr, &addrlen);
        if(fd < 0)
            ret = 0;
        else {
            close(sess->pasv_listen_fd);
            sess->pasv_listen_fd = -1;
            sess->data_fd = fd;
            ret = 1;
        }
    }
    if(sess->port_addr) {
        free(sess->port_addr);
        sess->port_addr = NULL;
    }
    return ret;
}
// 列表显示
void do_list(session_t *sess) {
    if(get_transfer_fd(sess) == 0)  // 是否成功获取数据传输的文件描述符
        return;
    ftp_reply(sess, FTP_DATACONN, "Here comes the directory listing.");
    //显示列表
    list_common(sess, 1);   // 显示目录的列表
    close(sess->data_fd);
    sess->data_fd = -1;
    ftp_reply(sess, FTP_TRANSFEROK, "Directory send OK.");
}
// 激活PASV模式
void do_pasv(session_t *sess){
    //先暂时写死，可以封装getlocalip()函数获取本机IP地址
    char ip[16] = "192.168.232.10";
    //getlocalip(ip);//
    sess->pasv_listen_fd = tcp_server(ip, 0);
    //获取port
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr);
    // 获取监听套接字的IP地址和端口号
    if(getsockname(sess->pasv_listen_fd, (struct sockaddr*)&addr, &addrlen) < 0)
        ERR_EXIT("getsockname");
    unsigned short port = ntohs(addr.sin_port);
    int v[4];
    sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
    char text[1024] = {0};
    //227 Entering Passive Mode (192,168,1,200,187,57).
    sprintf(text, "Entering Passive Mode (%u,%u,%u,%u,%u,%u).", v[0], v[1], v[2], v[3], port>>8, port&0x00ff);
    ftp_reply(sess, FTP_PASVOK, text);
}

// 更改目录
void do_cwd(session_t *sess){
    if(chdir(sess->arg) < 0){   // 目录更改为 输入命令
        ftp_reply(sess, FTP_NOPERM, "Failed to change directory.");
        return;
    }
    ftp_reply(sess, FTP_CWDOK, "Directory successfully changed.");
}
// 返回上一级命令
void do_cdup(session_t *sess){
    if(chdir("..") < 0){
        ftp_reply(sess, FTP_NOPERM, "Failed to change directory.");
        return;
    }
    ftp_reply(sess, FTP_CWDOK, "Directory successfully changed.");
}
// 创建目录 并返回创建成功信息
void do_mkd(session_t *sess){
    if(mkdir(sess->arg, 0777) < 0){
        ftp_reply(sess, FTP_NOPERM, "Create directory operation failed.");
        return;
    }
    char text[1024] = {0};
    sprintf(text, "\"%s\" create", sess->arg);
    ftp_reply(sess, FTP_MKDIROK, text);
}
//删除文件
void do_dele(session_t *sess){
    // 文件名称合法性...
    if(unlink(sess->arg) < 0){  // unlink()删除服务器上指定名称文件
        //550 Delete operation failed.
        ftp_reply(sess, FTP_NOPERM, "Delete operation failed.");
        return;
    }
    //250 Delete operation successful.
    ftp_reply(sess, FTP_DELEOK, "Delete operation successful.");
}
void do_rmd(session_t *sess){
    if(rmdir(sess->arg) < 0)
    {
        ftp_reply(sess, FTP_FILEFAIL, "Remove directory operation failed.");
        return;
    }
    //250 Remove directory operation successful.
    ftp_reply(sess, FTP_RMDIROK, "Remove directory operation successful.");
}
//获取文件大小
void do_size(session_t *sess){
    struct stat sbuf;   // 获取指定文件
    // 获取指定文件的统计信息
    if(stat(sess->arg, &sbuf) < 0){
        ftp_reply(sess, FTP_FILEFAIL, "SIZE operation failed.");
        return;
    }
    // 检查文件是否为普通文件
    if(!S_ISREG(sbuf.st_mode)){
        ftp_reply(sess, FTP_FILEFAIL, "Could not get file size.");
        return;
    }
    char text[1024] = {0};
    sprintf(text, "%lld", (long long)sbuf.st_size); // 文件大小转化为字符串
    ftp_reply(sess, FTP_SIZEOK, text);
}
// 保存 文件的源文件名 并向客户端发送响应消息
void do_rnfr(session_t *sess){
    sess->rnfr_name = (char*)malloc(strlen(sess->arg) + 1);
    memset(sess->rnfr_name, 0, strlen(sess->arg)+1);
    strcpy(sess->rnfr_name, sess->arg);
    // 350 Ready for RNTO.
    ftp_reply(sess, FTP_RNFROK, "Ready for RNTO.");
}
//重命名源文件名
void do_rnto(session_t *sess){
    if(sess->rnfr_name == NULL){
        ftp_reply(sess, FTP_NEEDRNFR, "RNFR required first.");
        return;
    }
    if(rename(sess->rnfr_name, sess->arg) < 0){ // 将源文件名修改为目标文件名
        ftp_reply(sess, FTP_NOPERM, "Rename failed.");
        return;
    }
    free(sess->rnfr_name);
    sess->rnfr_name = NULL;
    //250 Rename successful.
    ftp_reply(sess, FTP_RENAMEOK, "Rename successful.");
}
//下载文件
void do_retr(session_t *sess){
    if(get_transfer_fd(sess) == 0)  // 获取数据连接的套接字
        return;
    //打开文件
    int fd = open(sess->arg, O_RDONLY);
    if(fd == -1){
        ftp_reply(sess, FTP_FILEFAIL, "Failed to open file.");
        return;
    }
    struct stat sbuf;
    fstat(fd, &sbuf);   // 获取文件状态信息 文件大小、文件类型
    if(!S_ISREG(sbuf.st_mode)){
        ftp_reply(sess, FTP_FILEFAIL, "Failed to open file.");
        return;
    }
    char text[1024] = {0};
    if(sess->is_ascii){
        sprintf(text, "Opening ASCII mode data connection for %s(%lld bytes).",sess->arg, (long long)sbuf.st_size);
    }else{
        sprintf(text, "Opening BINARY mode data connection for %s(%lld bytes).",sess->arg, (long long)sbuf.st_size);
    }
    ftp_reply(sess, FTP_DATACONN, text);
    //下载文件 分包发送文件
    char buf[1024] = {0};
    int ret = 0;
    int read_total_bytes = sbuf.st_size;
    int read_count;
    int flag;
    while(1){
        read_count = read_total_bytes > 1024 ? 1024 : read_total_bytes;
        ret = read(fd, buf, read_count);
        if(ret == 0) {
            flag = 0; //OK
            break;
        } else if(ret != read_count) {
            flag = 1;
            break;
        } else if(ret == -1) {
            flag = 2;
            break;
        }
        write(sess->data_fd, buf, ret);
        read_total_bytes -= read_count;
    }
    close(sess->data_fd);
    sess->data_fd = -1;
    close(fd);
    if(flag == 0){
        ftp_reply(sess, FTP_TRANSFEROK, "Transfer complete.");
    } else if(flag == 1) {
        ftp_reply(sess, FTP_BADSENDNET, "Failure writting to network stream.");
    } else if(flag == 2) {
        ftp_reply(sess, FTP_BADSENDFILE, "Failure reading from local file.");
    }
}
//上传文件
void do_stor(session_t *sess){
    if(get_transfer_fd(sess) == 0)
        return;
    int fd = open(sess->arg, O_CREAT|O_WRONLY, 0755);
    if(fd == -1){
        ftp_reply(sess, FTP_FILEFAIL, "Failed to open file.");
        return;
    }
    int offset = sess->restart_pos;     // 是否需要断点传输
    sess->restart_pos = 0;
    struct stat sbuf;
    fstat(fd, &sbuf);
    if(!S_ISREG(sbuf.st_mode)){
        ftp_reply(sess, FTP_FILEFAIL, "Failed to open file.");
        return;
    }
    //150 Ok to send data.
    ftp_reply(sess, FTP_DATACONN, "Ok to send data.");
    if(lseek(fd, offset, SEEK_SET) < 0){    // 移动指针到断点位置
        ftp_reply(sess, FTP_UPLOADFAIL, "Could not create file.");
        return;
    }
    sess->bw_transfer_start_sec = get_time_sec();
    sess->bw_transfer_start_usec = get_time_usec();
    char buf[1024] = {0};
    int ret;
    int flag;
    while(1){
        ret = read(sess->data_fd, buf, sizeof(buf));
        if(ret == -1){
            flag = 2;
            break;
        }
        else if(ret == 0){
            flag = 0;
            break;
        }
        if(sess->bw_upload_rate_max != 0)
            limit_rate(sess, ret, 1);       // 限速处理
        if(write(fd, buf, ret) != ret){
            flag = 1;
            break;
        }
    }
    close(fd);
    close(sess->data_fd);
    sess->data_fd = -1;
    if(flag == 0){
        ftp_reply(sess, FTP_TRANSFEROK, "Transfer complete.");
    } else if(flag == 1){
        ftp_reply(sess, FTP_BADSENDNET, "Failure writting to network stream.");
    } else if(flag == 2) {
        ftp_reply(sess, FTP_BADSENDFILE, "Failure reading from local file.");
    }
}
// 限速
void limit_rate(session_t *sess, int bytes_transfered, int is_upload){
    long curr_sec = get_time_sec(); // 获取当前时间 秒数 微秒数
    long curr_usec = get_time_usec();
    double elapsed; //计算从开始传输到当前时间的总耗时
    elapsed = (double)(curr_sec - sess->bw_transfer_start_sec); // 设定最大传输速率
    elapsed += (double)(curr_usec - sess->bw_transfer_start_usec) / (double)1000000;
    unsigned int bw_rate = (unsigned int)((double)bytes_transfered / elapsed);    // 计算限速比例
    double rate_ratio;
    if (is_upload) {
        if (bw_rate <= sess->bw_upload_rate_max) {
            // 不需要限速
            sess->bw_transfer_start_sec = curr_sec;
            sess->bw_transfer_start_usec = curr_usec;
            return;
        }
        rate_ratio = bw_rate / sess->bw_upload_rate_max;
    } else {
        if (bw_rate <= sess->bw_download_rate_max){
            // 不需要限速
            sess->bw_transfer_start_sec = curr_sec;
            sess->bw_transfer_start_usec = curr_usec;
            return;
        }
        rate_ratio = bw_rate / sess->bw_download_rate_max;
    }
    // 睡眠时间 = (当前传输速度 / 最大传输速度 – 1) * 当前传输时间;
    double pause_time;
    pause_time = (rate_ratio - (double)1) * elapsed;
    nano_sleep(pause_time);
    sess->bw_transfer_start_sec = get_time_sec();
    sess->bw_transfer_start_usec = get_time_usec();
}