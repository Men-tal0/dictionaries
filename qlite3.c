#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <error.h>
#include <sqlite3.h>
#include "msg.h"

typedef struct {
    int  found;          /* 部分行代表用户存在 */
    char password[128];
} LoginResult;

/* exec 的回调：至少查到一行说明存在 */
int exist_callback(void *data, int argc, char **argv, char **cols)
{
    int *found = (int *)data;
    *found = 1;
    return 0;
}
int login_callback(void *data, int argc, char **argv, char **azColName)
{
    LoginResult *p = (LoginResult *)data;
    p->found = 1;                              /* 查到行 = 用户存在 */
    if (argc > 0 && argv[0])
        strcpy(p->password, argv[0]);          /* 同时存下密码 */
    return 0;
}
static int query_callback(void *data, int argc, char **argv, char **azColName)
{
    char *dest = (char *)data;
    if (argc > 0 && argv[0]) 
    {
        strcpy(dest, argv[0]);   // 把 chinese 字段拷出去
    }
    return 0;
}
void create_tables(void)
{
    sqlite3 *db;
    char buf[512] = {0};
    int rc;

    sqlite3_open("dict.db", &db);

    /* 用户表：id 是 5 位数字账号，password 是密码 */
    sprintf(buf, "create table if not exists users("
                "id integer primary key,"
                "password text not null)");

    rc = sqlite3_exec(db, buf, NULL, NULL, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("建 users 表失败: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    /* 单词表：word_id 自增序号，user_id 是归属的 5 位 ID */
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "create table if not exists words("
                "word_id integer primary key autoincrement,"
                "user_id integer not null,"
                "english text not null,"
                "chinese text not null)");
    rc = sqlite3_exec(db, buf, NULL, NULL, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("建 words 表失败: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    printf("建表成功\n");
    sqlite3_close(db);
}

/* 检查 ID 是否是有效的 5 位数字：返回 1 是，0 否 */
int is_valid_id(int id)
{
    return (id >= 10000 && id <= 99999);
}

/* 检查用户是否已注册：返回 1 已存在，0 不存在 */
int user_exists(sqlite3 *db, int id)
{
    static int found = 0;
    found = 0;
    char buf[256] = {0};
    sprintf(buf, "select id from users where id=%d", id);
    sqlite3_exec(db, buf, exist_callback, &found, NULL);
    return found;
}

/* 注册：成功返回 0,ID非法返回 1,ID已存在返回 2,出错返回 5 */
int db_register(int id, const char *password)
{
    sqlite3 *db;
    char buf[512] = {0};
    int rc;

    /* 1. 校验 5 位数字 */
    if (!is_valid_id(id)) 
    {
        printf("ID 必须是 5 位数字\n");
        return 1;
    }

    sqlite3_open("dict.db", &db);
    create_tables();

    /* 2. 查重 */
    if (user_exists(db, id)) 
    {
        printf("该 ID 已注册: %d\n", id);
        sqlite3_close(db);
        return 2;
    }

    /* 3. 插入用户 */
    sprintf(buf, "insert into users(id,password) values(%d,'%s')", id, password);
    rc = sqlite3_exec(db, buf, NULL, NULL, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("注册插入失败: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 5;
    }

    printf("注册成功: ID=%d\n", id);
    sqlite3_close(db);
    return 0;
}

int db_delete_user(int id)
{
    sqlite3 *db;
    char buf[512] = {0};
    int rc;
    sqlite3_open("dict.db", &db);
    /* 1. 先确认该用户存在 */
    if (!user_exists(db, id)) 
    {
        printf("用户不存在: %d\n", id);
        return 3;                 /* 用户不存在 */
    }

      /* 2. 先删这个用户的所有单词（子表） */
    sprintf(buf, "delete from words where user_id=%d", id);
    rc = sqlite3_exec(db, buf, NULL, NULL, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("删除单词失败: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 5;
    }

    /* 3. 再删用户本身（主表） */
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "delete from users where id=%d", id);
    rc = sqlite3_exec(db, buf, NULL, NULL, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("删除用户失败: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 5;
    }

    printf("已删除用户: %d 及其全部单词\n", id);
    sqlite3_close(db);
    return 0;
}

  int db_login(int id, const char *password)
  {
    sqlite3 *db;
    char sql[256] = {0};
    char *errmsg = NULL;
    LoginResult in = {0, {0}};    /* 初始化 found=0, password="" */
    int ret = RESULT_DB_ERR;

    if (sqlite3_open("dict.db", &db) != SQLITE_OK) 
    {
        printf("login open fail\n");
        return RESULT_DB_ERR;
      
    }
    sprintf(sql, "select password from users where id=%d", id);

    int rc = sqlite3_exec(db, sql, login_callback, &in, &errmsg);
    if (rc != SQLITE_OK) 
    {
        printf("登录查询失败: %s\n", errmsg);
        ret = RESULT_DB_ERR;
    }
    else if (!in.found) 
    {                    /* 没查到 → 用户不存在 */
        ret = RESULT_USER_NONE;
    }
    else if (strcmp(in.password, password) == 0) 
    {   /* 密码比对 */
        ret = RESULT_OK;
    }
    else 
    {
        ret = RESULT_PWD_ERR;
    }

    sqlite3_free(errmsg);
    sqlite3_close(db);
    return ret;
}

int db_query(int user_id, const char *english, char *chinese_out)
{
    sqlite3 *db;
    char sql[512] = {0};
    char *errmsg = NULL;
    int ret = RESULT_NOT_FOUND;
    if(sqlite3_open("dict.db",&db) != SQLITE_OK)
        return ret;
    sprintf(sql,"select chinese from words where user_id = %d and english = '%s' ",user_id,english);
    int rc = sqlite3_exec(db,sql,query_callback,chinese_out,&errmsg);
    if(rc != SQLITE_OK)
    {
        printf("查询单词失败: %s\n", errmsg);
        ret = RESULT_DB_ERR;
    }
    else if(strlen(chinese_out) == 0)
        ret = RESULT_NOT_FOUND; 
    else
        ret = RESULT_FOUND;  
    
    sqlite3_free(errmsg);
    sqlite3_close(db);
    return ret;
}

