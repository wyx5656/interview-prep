#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <pthread.h>

#define HOST "127.0.0.1"
#define MYSQL_PORT 3306
#define USER "root"
#define PASSWD "1234"
#define DBNAME "xiangmu"

// 数据库连接池
typedef struct {
    MYSQL **connections;
    int pool_size;         // 当前已初始化连接数
    int max_pool_size;     // 连接池最大容量（支持动态扩容到此最大值）
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int *available;
} ConnectionPool;

// 初始化连接池
// initial_size: 初始连接数
// max_size: 最大连接数（支持动态扩容到这个最大值）
ConnectionPool* init_connection_pool(int initial_size, int max_size);

// 获取连接（阻塞直到有可用连接）
MYSQL* get_connection(ConnectionPool *pool);

// 释放连接回池
void release_connection(ConnectionPool *pool, MYSQL *conn);

// 销毁连接池
void destroy_connection_pool(ConnectionPool *pool);

// 安全工具函数：转义字符串防止SQL注入
// 返回值：转义后字符串长度，-1表示错误
// 调用者必须释放返回的分配内存
int sql_escape_string(MYSQL *conn, const char *input, char **output);

// 兼容旧接口 - 不推荐使用，保留用于兼容
MYSQL * init_mysql();
MYSQL * init_mysql2();

#endif
