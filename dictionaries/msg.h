#ifndef MSG_H
#define MSG_H

#define MSG_REGISTER      1    /* 注册 */
#define MSG_LOGIN         2    /* 登录 */
#define MSG_QUERY         3    /* 查询单词 */
#define MSG_INSERT        4    /* 新增单词 */
#define MSG_DELETE        5    /* 删除单词 */
#define MSG_UPDATE        6    /* 修改单词 */
#define MSG_DELETE_USER   7    /* 删除用户（注销） */

/* ===== 回复类型：服务器回给客户端 ===== */
#define MSG_REPLY     100  /* 通用回复 */

/* ===== 业务结果码 ===== */
#define RESULT_OK         0   /* 成功 */
#define RESULT_INVALID_ID 1   /* ID 不是 5 位数字 */
#define RESULT_USER_EXIST 2   /* ID 已存在(注册) / 已注册 */
#define RESULT_USER_NONE  3   /* 用户不存在(登录) */
#define RESULT_PWD_ERR    4   /* 密码错误 */
#define RESULT_DB_ERR     5   /* 数据库错误 */
#define RESULT_NOT_FOUND  6   /* 没查到单词 */
#define RESULT_FOUND      7   /* 查到单词 */

/* ===== 通信结构体 ===== */
#pragma pack(push, 1)      /* 关对齐，跨端收发长度一致 */
typedef struct {
      int  type;             /* 请求/回复类型，MSG_XXX */
      int  result;           /* 回复结果码，RESULT_XXX */
      int  user_id;          /* 5 位数字用户 ID */
      char password[32];     /* 密码 */
      char english[64];      /* 英文单词 */
      char chinese[64];      /* 中文释义 */
      char data[256];        /* 附加数据(错误信息/提示) */
} Msg;
#pragma pack(pop)

#endif

