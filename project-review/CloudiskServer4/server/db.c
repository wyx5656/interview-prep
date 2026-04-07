#include "db.h"
#include <stdio.h>
#include <stdlib.h>

static MYSQL conn;
static MYSQL conn2;

// 兼容旧版本的单连接接口 - 保留用于兼容
MYSQL * init_mysql(){
    MYSQL * pconn = mysql_init(&conn);
    pconn = mysql_real_connect(&conn, HOST,USER,PASSWD,DBNAME,MYSQL_PORT,NULL,0);
    if(pconn == NULL) {
        printf("%d, %s\n", mysql_errno(&conn),
               mysql_error(&conn));
        return NULL;
    }
    return pconn;
}

MYSQL * init_mysql2(){
    MYSQL * pconn2 = mysql_init(&conn2);
    if (pconn2 == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return NULL;
    }

    pconn2 = mysql_real_connect(&conn2, HOST, USER, PASSWD, DBNAME, MYSQL_PORT, NULL, 0);
    if (pconn2 == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(&conn2));
        return NULL;
    }
    printf("MySQL connection established successfully\n");
    return pconn2;
}

// 数据库连接池实现
// initial_size: 初始连接数
// max_size: 最大连接数（支持动态扩容到这个最大值）
ConnectionPool* init_connection_pool(int initial_size, int max_size) {
    ConnectionPool *pool = (ConnectionPool*)malloc(sizeof(ConnectionPool));
    if (pool == NULL) {
        fprintf(stderr, "malloc failed for connection pool\n");
        exit(EXIT_FAILURE);
    }

    // 按最大连接数分配空间，预留扩容空间
    pool->connections = (MYSQL**)malloc(max_size * sizeof(MYSQL*));
    if (pool->connections == NULL) {
        fprintf(stderr, "malloc failed for connections array\n");
        free(pool);
        exit(EXIT_FAILURE);
    }

    pool->available = (int*)malloc(max_size * sizeof(int));
    if (pool->available == NULL) {
        fprintf(stderr, "malloc failed for available array\n");
        free(pool->connections);
        free(pool);
        exit(EXIT_FAILURE);
    }

    pool->pool_size = 0;        // 当前已初始化连接数
    pool->max_pool_size = max_size;  // 最大容量
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    // 初始化初始连接
    for (int i = 0; i < initial_size; ++i) {
        pool->connections[i] = mysql_init(NULL);
        if (mysql_real_connect(pool->connections[i], HOST, USER, PASSWD, DBNAME, MYSQL_PORT, NULL, 0) == NULL) {
            fprintf(stderr, "mysql_real_connect() failed for connection %d: %s\n",
                    i, mysql_error(pool->connections[i]));
            // 清理已分配的资源
            for (int j = 0; j < i; ++j) {
                mysql_close(pool->connections[j]);
            }
            free(pool->connections);
            free(pool->available);
            free(pool);
            exit(EXIT_FAILURE);
        }
        pool->available[i] = 1; // 1 表示连接可用
        pool->pool_size++;
    }

    // 将剩余位置标记为未初始化
    for (int i = initial_size; i < max_size; ++i) {
        pool->connections[i] = NULL;
        pool->available[i] = 0;
    }

    printf("Connection pool initialized: initial=%d connections, max=%d connections\n", initial_size, max_size);
    return pool;
}

MYSQL* get_connection(ConnectionPool *pool) {
    MYSQL *conn = NULL;

    pthread_mutex_lock(&pool->lock);

    while (1) {
        conn = NULL;
        // 查找可用连接
        for (int i = 0; i < pool->pool_size; ++i) {
            if (pool->available[i]) {
                conn = pool->connections[i];
                pool->available[i] = 0; // 标记为已用
                break;
            }
        }

        if (conn) {
            break;
        }

        // 没有找到可用连接，尝试动态扩容
        if (pool->pool_size < pool->max_pool_size) {
            int new_idx = pool->pool_size;
            // 新建一个连接
            pool->connections[new_idx] = mysql_init(NULL);
            if (mysql_real_connect(pool->connections[new_idx], HOST, USER, PASSWD, DBNAME, MYSQL_PORT, NULL, 0) != NULL) {
                // 扩容成功，直接使用这个新连接
                conn = pool->connections[new_idx];
                pool->available[new_idx] = 0;
                pool->pool_size++;
                printf("Connection pool expanded to %d connections (max %d)\n", pool->pool_size, pool->max_pool_size);
                break;
            } else {
                // 创建失败，回退，继续等待
                fprintf(stderr, "Failed to expand connection pool: mysql_real_connect() failed: %s\n",
                        mysql_error(pool->connections[new_idx]));
                mysql_close(pool->connections[new_idx]);
                pool->connections[new_idx] = NULL;
            }
        }

        // 仍然没有可用连接（已到最大容量或扩容失败），等待
        pthread_cond_wait(&pool->cond, &pool->lock);
    }

    pthread_mutex_unlock(&pool->lock);
    return conn;
}

void release_connection(ConnectionPool *pool, MYSQL *conn) {
    pthread_mutex_lock(&pool->lock);

    for (int i = 0; i < pool->pool_size; ++i) {
        if (pool->connections[i] == conn) {
            pool->available[i] = 1; // 标记为可用
            break;
        }
    }

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
}

void destroy_connection_pool(ConnectionPool *pool) {
    if (pool == NULL) return;

    for (int i = 0; i < pool->pool_size; ++i) {
        mysql_close(pool->connections[i]);
    }

    free(pool->connections);
    free(pool->available);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
    free(pool);

    printf("Connection pool destroyed\n");
}

// 安全工具函数：转义字符串防止SQL注入
int sql_escape_string(MYSQL *conn, const char *input, char **output) {
    if (conn == NULL || input == NULL || output == NULL) {
        return -1;
    }

    size_t input_len = strlen(input);
    // 分配足够的空间：每个字符最多需要2个字节转义 + 结束符
    *output = (char*)malloc(input_len * 2 + 1);
    if (*output == NULL) {
        return -1;
    }

    unsigned long escaped_len = mysql_real_escape_string(conn, *output, input, input_len);
    return (int)escaped_len;
}
