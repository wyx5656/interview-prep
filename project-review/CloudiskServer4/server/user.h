#ifndef __USER_H__
#define __USER_H__

#include "db.h"

#define ROOT_PATH "/server"

typedef enum {
    STATUS_LOGOFF = 0,
    STATUS_LOGIN
}LoginStatus;

typedef struct {
    int sockfd;//套接字文件描述符
    LoginStatus status;//登录状态
    char name[20];//用户名(客户端传递过来的)
    char encrypted[100];//加密密文
    char pwd[128];//用户当前路径
}user_info_t;

// 函数声明 - 都需要连接池参数
void loginCheck1(user_info_t * user, ConnectionPool *pool);
void loginCheck2(user_info_t * user, const char * encrypted, ConnectionPool *pool);

void RegisterCheck1(user_info_t * user, ConnectionPool *pool);
void RegisterCheck2(user_info_t * user, const char * encrypted, ConnectionPool *pool);

#endif

