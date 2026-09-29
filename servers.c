#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <error.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "qlite3.h"
#include "msg.h"


#define SERVE_PORT 8080

int sfd,cfd;
struct sockaddr_in serve_addr,cliect_addr;
int current_user_id = -1;   /* 当前登录用户；-1 未登录 */


void sys_err(const char *str){ perror(str);exit(1);}

/*注册用户*/
void handle_register(const Msg *in, Msg *out)
{
    int ret = db_register(in->user_id, in->password);
    out->type   = MSG_REGISTER;
    out->result = ret;
    switch (ret) 
    {
        case RESULT_OK:         strcpy(out->data, "注册成功"); break;
        case RESULT_INVALID_ID: strcpy(out->data, "ID必须为5位数字"); break;
        case RESULT_USER_EXIST: strcpy(out->data, "该ID已注册"); break;
        default:                strcpy(out->data, "数据库错误"); break;
    }
    printf("[REGISTER] id=%d -> %s\n", in->user_id, out->data);
}
/*删除用户*/
void handle_delete_user(const Msg *in, Msg *out)
{
    int ret = db_delete_user(in->user_id);
    out->type   = MSG_DELETE_USER;
    out->result = ret;
    switch (ret) 
    {
        case RESULT_OK:   strcpy(out->data, "已删除该用户"); break;
        case RESULT_USER_NONE: strcpy(out->data, "用户不存在"); break;
        default:          strcpy(out->data, "数据库错误"); break;
    }
    printf("[DELETE_USER] id=%d -> %s\n", in->user_id, out->data);
}
void handle_login(const Msg *in, Msg *out)
{
    int ret = db_login(in->user_id, in->password);
    out->type   = MSG_LOGIN;
    out->result = ret;
    switch (ret) 
    {
        case RESULT_OK:        strcpy(out->data, "登录成功");
                                 current_user_id = in->user_id;   /* ★记住登录用户 */
                                 break;
        case RESULT_USER_NONE: strcpy(out->data, "用户不存在");  break;
        case RESULT_PWD_ERR:   strcpy(out->data, "密码错误");    break;
        default:               strcpy(out->data, "数据库错误");  break;
    }
    printf("[LOGIN] id=%d -> %s\n", in->user_id, out->data);
}
void handle_query(const Msg *in, Msg *out)
{
    out->type = MSG_REPLY;

    /* 隔离核心：用服务器登录态，不用客户端传的 ID */
    if (current_user_id < 0) 
    {
        out->result = RESULT_USER_NONE;
        strcpy(out->data, "未登录，请先登录");
        return;
    }

    int ret = db_query(current_user_id, in->english, out->chinese);
    out->result = ret;
    if (ret == RESULT_FOUND)
        strcpy(out->data, "查询成功");
    else
    {
        strcpy(out->data, "未找到该单词");
        printf("[QUERY] user=%d word=%s -> %s\n",
        current_user_id, in->english, out->data);
    }

}

/* 读满 sizeof(Msg) 字节（处理半包），返回失败返回 -1 */
int read_full(int fd, void *buf, size_t n)
{
    size_t total = 0;
    while (total < n) 
    {
        ssize_t r = read(fd, (char *)buf + total, n - total);
        if (r <= 0) return -1;
        total += r;
    }
    return 0;
}
/*TCP初始化*/
void TCP_Init()
{
    socklen_t cliect_len;
    serve_addr.sin_family = AF_INET;
    serve_addr.sin_port = htons(SERVE_PORT);
    serve_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd == -1)
        sys_err("socket\n");

    if(bind(sfd,(struct sockaddr *)&serve_addr,sizeof(serve_addr))== -1)
        sys_err("bind\n");
    listen(sfd,128);
    cliect_len = sizeof(cliect_addr);
    cfd = accept(sfd,(struct sockaddr *)&cliect_addr,&cliect_len);
    if(cfd == -1)
        sys_err("accept\n");
}

int main(int argc, char *argv[])
{
    Msg in,out;
    TCP_Init();
    while(1)
    {
        if(read_full(cfd,&in,sizeof(Msg)) != 0)
        {
            printf("Client disconnected\n");
            break;
        }
        memset(&out,0,sizeof(out));
        switch(in.type)
        {
            case MSG_REGISTER :    //注册用户
                handle_register(&in,&out);
            break;
            case MSG_DELETE_USER : //删除用户
                handle_delete_user(&in, &out);
            break;
            case MSG_LOGIN:      //login
                handle_login(&in, &out);
            break;
            case MSG_QUERY:     //search
                handle_query(&in, &out);
            break;
            default:
                out.type   = MSG_REPLY;
                out.result = RESULT_DB_ERR;
                strcpy(out.data, "未知命令");
            break;
        }
        write(cfd,&out,sizeof(Msg));
    }
    return 0;
}


