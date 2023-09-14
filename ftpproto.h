//ftpproto.h
#ifndef _FTPPROTO_H_
#define _FTPPROTO_H_
#include"common.h"
#include"session.h"
#include "str.h"
// ftp进程
void handle_child(session_t *sess);
// 回应代码
#ifndef _FTPCODES_H_
#define _FTPCODES_H_
#define FTP_DATACONN 150
#define FTP_NOOPOK 200
#define FTP_TYPEOK 200
#define FTP_PORTOK 200
#define FTP_EPRTOK 200
#define FTP_UMASKOK 200
#define FTP_CHMODOK 200
#define FTP_EPSVALLOK 200
#define FTP_STRUOK 200
#define FTP_MODEOK 200
#define FTP_PBSZOK 200
#define FTP_PROTOK 200
#define FTP_OPTSOK 200
#define FTP_ALLOOK 202
#define FTP_FEAT 211
#define FTP_STATOK 211
#define FTP_SIZEOK 213
#define FTP_MDTMOK 213
#define FTP_STATFILE_OK 213
#define FTP_SITEHELP 214
#define FTP_HELP 214
#define FTP_SYSTOK 215
#define FTP_GREET 220
#define FTP_GOODBYE 221
#define FTP_ABOR_NOCONN 225
#define FTP_TRANSFEROK 226
#define FTP_ABOROK 226
#define FTP_PASVOK 227
#define FTP_EPSVOK 229
#define FTP_LOGINOK 230
#define FTP_AUTHOK 234
#define FTP_CWDOK 250
#define FTP_RMDIROK 250
#define FTP_DELEOK 250
#define FTP_RENAMEOK 250
#define FTP_PWDOK 257
#define FTP_MKDIROK 257
#define FTP_GIVEPWORD 331
#define FTP_RESTOK 350
#define FTP_RNFROK 350
#define FTP_IDLE_TIMEOUT 421
#define FTP_DATA_TIMEOUT 421
#define FTP_TOO_MANY_USERS 421
#define FTP_IP_LIMIT 421
#define FTP_IP_DENY 421
#define FTP_TLS_FAIL 421
#define FTP_BADSENDCONN 425
#define FTP_BADSENDNET 426
#define FTP_BADSENDFILE 451
#define FTP_BADCMD 500
#define FTP_BADOPTS 501
#define FTP_COMMANDNOTIMPL 502
#define FTP_NEEDUSER 503
#define FTP_NEEDRNFR 503
#define FTP_BADPBSZ 503
#define FTP_BADPROT 503
#define FTP_BADSTRU 504
#define FTP_BADMODE 504
#define FTP_BADAUTH 504
#define FTP_NOSUCHPROT 504
#define FTP_NEEDENCRYPT 522
#define FTP_EPSVBAD 522
#define FTP_DATATLSBAD 522
#define FTP_LOGINERR 530
#define FTP_NOHANDLEPROT 536
#define FTP_FILEFAIL 550
#define FTP_NOPERM 550
#define FTP_UPLOADFAIL 553

static void do_user(session_t *sess);   // 定义静态函数，指向session_t结构体的指针sess static只当前源文件可见
static void do_pass(session_t *sess);
static void do_cwd(session_t *sess);
static void do_cdup(session_t *sess);
static void do_quit(session_t *sess);
static void do_port(session_t *sess);
static void do_pasv(session_t *sess);
static void do_type(session_t *sess);
//static void do_stru(session_t *sess);
//static void do_mode(session_t *sess);
static void do_retr(session_t *sess);
static void do_stor(session_t *sess);
static void do_appe(session_t *sess);
static void do_list(session_t *sess);
static void do_nlst(session_t *sess);
static void do_rest(session_t *sess);
static void do_abor(session_t *sess);
static void do_pwd(session_t *sess);
static void do_mkd(session_t *sess);
static void do_rmd(session_t *sess);
static void do_dele(session_t *sess);
static void do_rnfr(session_t *sess);
static void do_rnto(session_t *sess);
static void do_site(session_t *sess);
static void do_syst(session_t *sess);
static void do_feat(session_t *sess);
static void do_size(session_t *sess);
static void do_stat(session_t *sess);
static void do_noop(session_t *sess);
static void do_help(session_t *sess);

void ftp_reply(session_t *sess, int code, const char *msg);
int port_active(session_t *sess);
int pasv_active(session_t *sess);
void limit_rate(session_t *sess, int bytes_transfered, int is_upload);

typedef struct ftpcmd {
    const char *cmd;    // 指向FTP命令名称
    void (*cmd_handler)(session_t *sess);
} ftpcmd_t;
static ftpcmd_t ctrl_cmds[] = {
        /* 访问控制命令 */
        {"USER", do_user },
        {"PASS", do_pass },
        {"CWD", do_cwd },
        {"XCWD", do_cwd },
        {"CDUP", do_cdup },
        {"XCUP", do_cdup },
        {"QUIT", do_quit },
        {"ACCT", NULL },
        {"SMNT", NULL },
        {"REIN", NULL },
        /* 传输参数命令 */
        {"PORT", do_port },
        {"PASV", do_pasv },
        {"TYPE", do_type },
        {"STRU", /*do_stru*/NULL },
        {"MODE", /*do_mode*/NULL },
        /* 服务命令 */
        {"RETR", do_retr },
        {"STOR", do_stor },
        {"APPE", do_appe },
        {"LIST", do_list },
        {"NLST", do_nlst },
        {"REST", do_rest },
        {"ABOR", do_abor },
        {"\377\364\377\362ABOR", do_abor},
        {"PWD", do_pwd },
        {"XPWD", do_pwd },
        {"MKD", do_mkd },
        {"XMKD", do_mkd },
        {"RMD", do_rmd },
        {"XRMD", do_rmd },
        {"DELE", do_dele },
        {"RNFR", do_rnfr },
        {"RNTO", do_rnto },
        {"SITE", do_site },
        {"SYST", do_syst },
        {"FEAT", do_feat },
        {"SIZE", do_size },
        {"STAT", do_stat },
        {"NOOP", do_noop },
        {"HELP", do_help },
        {"STOU", NULL },
        {"ALLO", NULL }
};

#endif
