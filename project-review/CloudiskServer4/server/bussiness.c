#include "linked_list.h"
#include "thread_pool.h"
#include "user.h"
#include "db.h"
#include "hashtable.h"
#include "timeout_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MAX_LENGTH 1024
//外部变量(userList是在main.c文件中定义的)
extern ListNode * userList;
extern HashTable userTable;
extern ConnectionPool *Mysqlpool;
extern threadpool_t long_task_pool;
extern pthread_mutex_t userTable_mutex;

// O(1)查找用户 - 使用哈希表，互斥锁保护并发访问
static user_info_t* find_user_by_sockfd(int sockfd) {
    char key[32];
    snprintf(key, sizeof(key), "%d", sockfd);
    pthread_mutex_lock(&userTable_mutex);
    user_info_t *user = (user_info_t*)find(&userTable, key);
    pthread_mutex_unlock(&userTable_mutex);
    return user; // 如果找不到会返回NULL
}

//主线程调用:处理客户端发过来的消息
//边缘触发模式下：需要循环读取直到recv返回EAGAIN，表示数据全部读完
void handleMessage(int sockfd, int epfd, task_queue_t * que)
{
    int ret;
    // 循环读取所有可用数据
    while (1) {
        //消息格式：cmd content
        //1.1 获取消息长度
        int length = -1;
        ret = recvn(sockfd, &length, sizeof(length));
        if (ret == 0) {
            // 对端关闭连接
            goto end_close;
        } else if (ret == -1) {
            // 如果是EAGAIN，表示数据已经全部读取完毕，正常退出循环
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            // 其他错误，关闭连接
            goto end_close;
        }
        printf("\n\nrecv length: %d\n", length);

        //1.2 获取消息类型
        int cmdType = -1;
        ret = recvn(sockfd, &cmdType, sizeof(cmdType));
        if (ret == 0) {
            goto end_close;
        } else if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            goto end_close;
        }
        printf("recv cmd type: %d\n", cmdType);

        task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
        if (ptask == NULL) {
            close(sockfd);
            return;
        }
        ptask->peerfd = sockfd;
        ptask->epfd = epfd;
        ptask->type= cmdType;
        if(length > 0) {
            //1.3 获取消息内容
            ret = recvn(sockfd, ptask->data, length);
            if (ret > 0) {
                // 架构设计：短任务直接在主线程执行，长任务交给线程池
                // 短任务：pwd/ls/cd/mkdir/rmdir/login/register - 快速完成
                // 长任务：puts/gets - 传输大文件，交给线程池异步执行
                if (cmdType == CMD_TYPE_PUTS || cmdType == CMD_TYPE_GETS) {
                    // 长任务：需要分配task_long_t以包含originfd字段
                    task_long_t *long_task = (task_long_t*)calloc(1, sizeof(task_long_t));
                    long_task->peerfd = sockfd;
                    long_task->epfd = epfd;
                    long_task->type = cmdType;
                    strncpy(long_task->data, ptask->data, sizeof(long_task->data) - 1);
                    long_task->originfd = sockfd;  // 单端口架构下源fd就是当前连接fd
                    free(ptask);  // 释放原来的小任务分配
                    // 入队到线程池，后台执行。task_long_t*可以安全转为task_t*因为布局兼容
                    taskEnque(que, (task_t*)long_task);
                } else {
                    // 短任务：直接在主线程执行，低延迟
                    doTask(ptask);
                    free(ptask);
                }
            } else if (ret == 0) {
                free(ptask);
                goto end_close;
            } else if (ret == -1) {
                free(ptask);
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                goto end_close;
            }
        } else if(length == 0){
            // length 0 - 依然分类处理
            if (cmdType == CMD_TYPE_PUTS || cmdType == CMD_TYPE_GETS) {
                // 长任务：需要分配task_long_t以包含originfd字段
                task_long_t *long_task = (task_long_t*)calloc(1, sizeof(task_long_t));
                long_task->peerfd = sockfd;
                long_task->epfd = epfd;
                long_task->type = cmdType;
                long_task->originfd = sockfd;
                free(ptask);
                taskEnque(que, (task_t*)long_task);
            } else {
                doTask(ptask);
                free(ptask);
            }
        }
    }
    return;

end_close:
    //连接断开的情况
    printf("\nconn %d is closed.\n", sockfd);
    delEpollReadfd(epfd, sockfd);
    close(sockfd);
    // 从哈希表删除，需要加锁保护
    char key[32];
    snprintf(key, sizeof(key), "%d", sockfd);
    // 查找用户信息获取指针，用于从超时队列删除
    pthread_mutex_lock(&userTable_mutex);
    user_info_t *user = (user_info_t*)find(&userTable, key);
    if (user != NULL) {
        // 用户正常断开，从超时队列删除，防止内存泄漏和野指针
        extern void removeUser(TimeoutQueue* queue, user_info_t * user);
        extern TimeoutQueue *TimeoutQueue_global; // 超时队列在main中，需要声明
        removeUser(TimeoutQueue_global, user);
    }
    erase(&userTable, key);
    pthread_mutex_unlock(&userTable_mutex);
    deleteNode2(&userList, sockfd);//删除用户信息
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
//子线程调用
void doTask(task_t * task)
{
    assert(task);
    switch(task->type) {
    case CMD_TYPE_PWD:
        pwdCommand(task);   break;
    case CMD_TYPE_CD:
        cdCommand(task);    break;
    case CMD_TYPE_LS:
        lsCommand(task);    break;
    case CMD_TYPE_MKDIR:
        mkdirCommand(task);  break;
    case CMD_TYPE_RMDIR:
        rmdirCommand(task);  break;
    case CMD_TYPE_NOTCMD:
        notCommand(task);   break;
    case CMD_TYPE_PUTS:
        putsCommand((task_long_t*)task);  break;
    case CMD_TYPE_GETS:
        getsCommand((task_long_t*)task);  break;
    case TASK_LOGIN_SECTION1:
        userLoginCheck1(task); break;
    case TASK_LOGIN_SECTION2:
        userLoginCheck2(task); break;
    case TASK_Register_SECTION1:
        userrRegisterCheck1(task); break;
    case TASK_Register_SECTION2:
        userrRegisterCheck2(task); break;

    }


}

//每一个具体任务的执行，交给一个成员来实现
void normalizePath(char *path) {
    char normalized[MAX_LENGTH];
    char *token;
    char *context;
    char *result[MAX_LENGTH];
    int depth = 0;

    token = strtok_r(path, "/", &context);
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // 跳过 "."
        } else if (strcmp(token, "..") == 0) {
            if (depth > 0) {
                depth--; // 回退到上一级目录
            }
        } else {
            result[depth++] = token;
        }
        token = strtok_r(NULL, "/", &context);
    }

    // 重构路径
    strcpy(normalized, "/");
    for (int i = 0; i < depth; i++) {
        strcat(normalized, result[i]);
        if (i < depth - 1) {
            strcat(normalized, "/");
        }
    }
    strncpy(path, normalized, MAX_LENGTH);
}

void cdCommand(task_t * task)
{
    //peerfd
    //epfd
    //data
    //需要一个结构体，存储peerfd当前的目录结构
    char solvedPath[MAX_LENGTH];
    char basePath[MAX_LENGTH];

    // O(1)查找用户 - 使用哈希表代替O(n)链表遍历
    user_info_t *user = find_user_by_sockfd(task->peerfd);
    if (user == NULL) {
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }

    //从连接池获取连接
    MYSQL* pconn = get_connection(Mysqlpool);
    printf("cdCommand: got connection from pool\n");

    char sql[256];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT vf.path FROM user u JOIN virtual_file vf ON u.path_id = vf.id WHERE u.username = '%s';", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){// 用户存在的情况下
        send(task->peerfd, "Error:cd error", 15, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    strncpy(user->pwd, row[0], sizeof(user->pwd) - 1);
    user->pwd[sizeof(user->pwd) - 1] = '\0';
    strncpy(basePath, user->pwd, sizeof(basePath) - 1);
    basePath[sizeof(basePath) - 1] = '\0';

    //当第一次cd的时候，是以服务器根目录为基础的(这里先读取peerfd的路径)，以读取到的路径为基础
    char* targetPath = task->data;
    //特殊处理 .  ..  /  其它情况为路径错误   拼接出绝对路径
    if(targetPath[0] == '/'){
        strncpy(solvedPath, targetPath, sizeof(solvedPath) - 1);
        solvedPath[sizeof(solvedPath) - 1] = '\0';
        //绝对路径
    }else{
        //相对路径
        if(strcmp(targetPath, ".") == 0){
            strcpy(solvedPath, basePath);
        }else if(strcmp(targetPath, "..") == 0){
            char* currentChar = basePath;
            char* lastSlash = NULL;
            while(*currentChar != '\0'){
                if(*currentChar == '/'){
                    lastSlash = currentChar;
                }
                currentChar++;
            }
            if (lastSlash != NULL && lastSlash > basePath) {
                *lastSlash = '\0';
                strncpy(solvedPath, basePath, strlen(basePath));
                solvedPath[strlen(basePath)] = '\0';
            } else {
                strcpy(solvedPath, "/");
            }
        }else{
            snprintf(solvedPath, sizeof(solvedPath), "%s/%s", basePath, targetPath);
        }
    }
    //判断路径是否有效，如果有效，将拼接后的路径更新到存储peerfd目录的链表
    normalizePath(solvedPath);

    // 转义参数
    char *escaped_solvedPath = NULL;
    escaped_username = NULL;
    int esc_path = sql_escape_string(pconn, solvedPath, &escaped_solvedPath);
    esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_path < 0 || esc_len < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_solvedPath);
        free(escaped_username);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT id,type FROM virtual_file WHERE path = '%s' AND owner_id = (SELECT id FROM user WHERE username = '%s')", escaped_solvedPath, escaped_username);
    free(escaped_solvedPath);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }

    row = mysql_fetch_row(result);
    if (row == NULL) { // 路径不存在的情况下
        send(task->peerfd, "Error: Path not found", 22, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }

    int path_id = atoi(row[0]);
    if (strcmp(row[1], "d") != 0) { // 检查路径类型是否为目录
        send(task->peerfd, "Error: Not a directory", 23, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    mysql_free_result(result);

    escaped_username = NULL;
    esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_username);
        release_connection(Mysqlpool, pconn);
        return;
    }

    //最后更新user表里的path id字段
    snprintf(sql, sizeof(sql), "UPDATE user SET path_id = %d WHERE username = '%s'", path_id, escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "UPDATE error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }

    release_connection(Mysqlpool, pconn);
    printf("connection released to pool\n");

    strncpy(user->pwd, solvedPath, sizeof(user->pwd) - 1);
    user->pwd[sizeof(user->pwd) - 1] = '\0';
    send(task->peerfd, user->pwd, strlen(user->pwd), 0);
    return;
}

void lsCommand(task_t * task)
{
    printf("execute ls command.\n");
    // 查找当前用户 - O(1)查找
    user_info_t *user = find_user_by_sockfd(task->peerfd);

    if (user == NULL)
    {
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }

    // 从连接池获取连接
    MYSQL *pconn = get_connection(Mysqlpool);
    if (pconn == NULL)
    {
        send(task->peerfd, "Error: Database connection failed", 34, 0);
        return;
    }
    printf("lsCommand: got connection from pool\n");

    // 获取当前用户的 path_id
    char sql[256];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0)
    {
        send(task->peerfd, "Error: Database query failed", 28, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT path_id FROM user WHERE username = '%s'", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Database query failed", 28, 0);
        return;
    }

    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL)
    {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Database query failed", 28, 0);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL)
    {
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Path not found", 22, 0);
        return;
    }

    int path_id = atoi(row[0]);
    mysql_free_result(result);

    // 查询虚拟文件表中当前目录下的所有文件和目录
    escaped_username = NULL;
    esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0)
    {
        send(task->peerfd, "Error: Database query failed", 28, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql),
             "SELECT filename, type FROM virtual_file WHERE parent_id = %d AND owner_id = (SELECT id FROM user WHERE username = '%s')",
             path_id, escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Database query failed", 28, 0);
        return;
    }

    result = mysql_store_result(pconn);
    if (result == NULL)
    {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Database query failed", 28, 0);
        return;
    }

    // 拼接输出结果
    char output[MAX_LENGTH] = "";
    size_t output_len = 0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        size_t fname_len = strlen(row[0]);
        if (output_len + fname_len + 2 < sizeof(output)) {
            strcat(output + output_len, row[0]);
            output_len += fname_len;
            if (strcmp(row[1], "d") == 0) {
                output[output_len++] = '/';
            }
            output[output_len++] = '\n';
            output[output_len] = '\0';
        }
    }

    // 发送结果给客户端
    send(task->peerfd, output, output_len, 0);

    mysql_free_result(result);
    release_connection(Mysqlpool, pconn);
    printf("connection released to pool\n");
}


void mkdirCommand(task_t * task)
{
    printf("execute mkdir command.\n");

    // O(1)查找用户
    user_info_t *user = find_user_by_sockfd(task->peerfd);
    if (user == NULL) {
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }
    char *newDirName = task->data; // 获取要创建的目录名
    char newDirPath[MAX_LENGTH];

    // 从连接池获取连接
    MYSQL* pconn = get_connection(Mysqlpool);
    printf("mkdirCommand: got connection from pool\n");
    char sql[512];

    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT vf.path FROM user u JOIN virtual_file vf ON u.path_id = vf.id WHERE u.username = '%s';", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){
        send(task->peerfd, "Error:mkdir error", 15, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    strncpy(user->pwd, row[0], sizeof(user->pwd) - 1);
    user->pwd[sizeof(user->pwd) - 1] = '\0';

    snprintf(newDirPath, sizeof(newDirPath), "%s/%s", user->pwd, newDirName); // 拼接出新目录的路径
    mysql_free_result(result);

    // 查看目录是否存在
    char *escaped_newDirPath = NULL;
    int esc_path = sql_escape_string(pconn, newDirPath, &escaped_newDirPath);
    if (esc_path < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_newDirPath);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT id FROM virtual_file WHERE path = '%s'", escaped_newDirPath);
    free(escaped_newDirPath);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    row = mysql_fetch_row(result);
    if(row != NULL){// 目录已存在
        send(task->peerfd, "Error: Directory already exists", 30, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    mysql_free_result(result);

    // 查找当前目录的 id
    char *escaped_pwd = NULL;
    escaped_username = NULL;
    int esc_pwd = sql_escape_string(pconn, user->pwd, &escaped_pwd);
    esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_pwd < 0 || esc_len < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_pwd);
        free(escaped_username);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT id FROM virtual_file WHERE path = '%s' AND owner_id = (SELECT id FROM user WHERE username = '%s')", escaped_pwd, escaped_username);
    free(escaped_pwd);
    free(escaped_username);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    result = mysql_store_result(pconn);
    if (result == NULL)
    {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    row = mysql_fetch_row(result);
    if (row == NULL)
    {
        send(task->peerfd, "Error: Current directory not found", 35, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    int parent_id = atoi(row[0]);
    mysql_free_result(result);

    // 插入新目录 - 需要转义所有参数
    char *escaped_newDirName = NULL;
    escaped_username = NULL;
    escaped_newDirPath = NULL;
    int esc_name = sql_escape_string(pconn, newDirName, &escaped_newDirName);
    esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    esc_path = sql_escape_string(pconn, newDirPath, &escaped_newDirPath);

    if (esc_name < 0 || esc_len < 0 || esc_path < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_newDirName);
        free(escaped_username);
        free(escaped_newDirPath);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql),
        "INSERT INTO virtual_file (parent_id, filename, owner_id, hash, filesize, type, path) "
        "VALUES (%d, '%s', (SELECT id FROM user WHERE username = '%s'), 12345, 0, 'd', '%s')",
        parent_id, escaped_newDirName, escaped_username, escaped_newDirPath);

    free(escaped_newDirName);
    free(escaped_username);
    free(escaped_newDirPath);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "INSERT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }

    send(task->peerfd, "Directory created successfully", 30, 0);
    release_connection(Mysqlpool, pconn);
    printf("connection released to pool\n");
}

void pwdCommand(task_t * task)
{
    printf("execute pwd command.\n");

    // 查找当前用户 - O(1)查找
    user_info_t *user = find_user_by_sockfd(task->peerfd);

    if (user == NULL)
    {
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }
    // 从连接池获取连接
    MYSQL *pconn = get_connection(Mysqlpool);
    if (pconn == NULL)
    {
        send(task->peerfd, "Error: Database connection failed", 34, 0);
        return;
    }
    printf("pwdCommand: got connection from pool\n");

    // 转义用户名防止SQL注入
    char sql[256];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0)
    {
        send(task->peerfd, "Error: Database query failed", 28, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    // 合并查询用户的path_id和虚拟文件表中的路径
    snprintf(sql, sizeof(sql),
             "SELECT vf.path FROM user u JOIN virtual_file vf ON u.path_id = vf.id WHERE u.username = '%s'",
             escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Database query failed", 28, 0);
        return;
    }

    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL)
    {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Database query failed", 28, 0);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL)
    {
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        send(task->peerfd, "Error: Path not found", 22, 0);
        return;
    }

    // 发送路径给客户端
    char *path = row[0];
    send(task->peerfd, path, strlen(path), 0);

    mysql_free_result(result);
    release_connection(Mysqlpool, pconn);
    printf("connection released to pool\n");
}

void rmdirCommand(task_t *task)
{
    printf("execute rmdir command.\n");

    // 查找当前用户 - O(1)查找
    user_info_t *user = find_user_by_sockfd(task->peerfd);
    if (user == NULL) {
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }

    char *targetPath = task->data; // 获取要删除的路径
    char fullPath[MAX_LENGTH];

    // 获取当前用户的路径从连接池
    MYSQL *pconn = get_connection(Mysqlpool);
    printf("rmdirCommand: got connection from pool\n");

    char sql[512];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0)
    {
        send(task->peerfd, "Error: Database error", 20, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT vf.path FROM user u JOIN virtual_file vf ON u.path_id = vf.id WHERE u.username = '%s';", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL)
    {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL)
    {
        send(task->peerfd, "Error: rmdir error", 18, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    strncpy(user->pwd, row[0], sizeof(user->pwd) - 1);
    user->pwd[sizeof(user->pwd) - 1] = '\0';

    snprintf(fullPath, sizeof(fullPath), "%s/%s", user->pwd, targetPath); // 拼接出完整路径
    mysql_free_result(result);

    // 检查路径是否存在及其类型
    char *escaped_fullPath = NULL;
    escaped_username = NULL;
    int esc_full = sql_escape_string(pconn, fullPath, &escaped_fullPath);
    esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_full < 0 || esc_len < 0)
    {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_fullPath);
        free(escaped_username);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT id, type FROM virtual_file WHERE path = '%s' AND owner_id = (SELECT id FROM user WHERE username = '%s');", escaped_fullPath, escaped_username);
    free(escaped_fullPath);
    free(escaped_username);

    if (mysql_query(pconn, sql))
    {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    result = mysql_store_result(pconn);
    if (result == NULL)
    {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    row = mysql_fetch_row(result);
    if (row == NULL)
    {
        send(task->peerfd, "Error: Path not found", 21, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    int targetId = atoi(row[0]);
    char type = row[1][0];
    mysql_free_result(result);

    // 递归删除目录及其所有内容
    escaped_fullPath = NULL;
    esc_full = sql_escape_string(pconn, fullPath, &escaped_fullPath);
    if (esc_full < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_fullPath);
        release_connection(Mysqlpool, pconn);
        return;
    }

    if (type == 'd')
    {
        // LIKE 模式匹配：删除目录及其所有子目录/文件
        // 需要匹配 escaped_fullPath 开头的所有路径，所以添加 %
        // 注意：mysql_real_escape_string 不转义 LIKE 通配符 % 和 _
        // 如果原路径包含 % 或 _，它们仍然会被当作通配符，这可能导致不正确匹配
        // 由于这是用户自己目录下的操作，即使出错也只会删除用户自己的文件，风险可控
        char pattern[512];
        snprintf(pattern, sizeof(pattern), "%s%%", escaped_fullPath);
        snprintf(sql, sizeof(sql), "DELETE FROM virtual_file WHERE path LIKE %s;", pattern);
        free(escaped_fullPath);

        if (mysql_query(pconn, sql))
        {
            fprintf(stderr, "DELETE error: %s\n", mysql_error(pconn));
            release_connection(Mysqlpool, pconn);
            return;
        }
    }
    else if (type == 'f' || type == 'd')
    {
        // 对于文件也处理，不管是f还是d都删除具体条目
        snprintf(sql, sizeof(sql), "DELETE FROM virtual_file WHERE id = %d;", targetId);
        free(escaped_fullPath);

        if (mysql_query(pconn, sql))
        {
            fprintf(stderr, "DELETE error: %s\n", mysql_error(pconn));
            release_connection(Mysqlpool, pconn);
            return;
        }
    }
    else
    {
        send(task->peerfd, "Error:Rm Invalid type", 19, 0);
        free(escaped_fullPath);
        release_connection(Mysqlpool, pconn);
        return;
    }

    release_connection(Mysqlpool, pconn);
    printf("connection released to pool\n");
    send(task->peerfd, "Remove succeeded", 17, 0);
    return;
}

void notCommand(task_t * task)
{
    char *path = "Without this command, please re-enter (cd ls pwd puts gets mkdir rm...)";
    send(task->peerfd, path, strlen(path), 0);
}


void getsCommand(task_long_t * task) {
    printf("execute gets command.\n");
    //1、 拿到用户信息 - O(1)查找
    user_info_t *user = find_user_by_sockfd(task->originfd);
    if (user == NULL) {
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }

    //2、 从连接池获取数据库连接
    MYSQL *pconn = get_connection(Mysqlpool);
    printf("getsCommand: got connection from pool\n");

    char sql[256];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT vf.path FROM user u JOIN virtual_file vf ON u.path_id = vf.id WHERE u.username = '%s';", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_RES * result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){
        send(task->peerfd, "Error:path error", 15, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    strncpy(user->pwd, row[0], sizeof(user->pwd) - 1);
    user->pwd[sizeof(user->pwd) - 1] = '\0';

    //文件名+路径
    mysql_free_result(result);
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", user->pwd, task->data);

    char *escaped_filepath = NULL;
    int esc_path = sql_escape_string(pconn, filepath, &escaped_filepath);
    if (esc_path < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        free(escaped_filepath);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql),
        "SELECT gf.hash "
        "FROM virtual_file vf "
        "JOIN global_file gf ON vf.hash = gf.id "
        "WHERE vf.path = '%s'", escaped_filepath);
    free(escaped_filepath);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }

    row = mysql_fetch_row(result);
    if(row == NULL){
        send(task->peerfd, "file not exit", 14, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    // 使用当前项目正确的基础路径，从配置环境中获取
    // 文件存储在当前项目server/files目录下
    char getsfile[512];
    const char *base_path = getenv("CLOUDISK_FILES");
    if (base_path == NULL) {
        base_path = "./files";
    }
    snprintf(getsfile, sizeof(getsfile), "%s/%s", base_path, row[0]);
    printf("File path: %s\n", getsfile);
    mysql_free_result(result);

    int fd = open(getsfile, O_RDONLY);
    if(fd == -1) {
        perror("open");
        release_connection(Mysqlpool, pconn);
        return;
    }

    int fsize_recv = 0;
    //先发送文件的长度
    struct stat st;
    memset(&st, 0, sizeof(st));
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        release_connection(Mysqlpool, pconn);
        return;
    }
    off_t file_len = st.st_size;
    int res = send(task->peerfd, &file_len, sizeof(file_len), 0);
    printf("send %d bytes, file_len:%ld\n", res, (long)file_len);

    //接收客户端传来的已收到文件大小的信息
    res = recv(task->peerfd, &fsize_recv, sizeof(fsize_recv), 0);
    if(res <= 0) {
        perror("recv fsize");
        close(fd);
        release_connection(Mysqlpool, pconn);
        return;
    }
    printf("received fsize:%d\n", fsize_recv);

    if(fsize_recv == (int)file_len) {
        close(fd);
        release_connection(Mysqlpool, pconn);
        printf("File already fully transferred\n");
        return;
    }

    //再发送文件内容
    off_t off = fsize_recv;
    const int MAP_MAX_SIZE = 4096;
    off_t left = file_len - off;
    printf("transfer_file start from offset %ld, %ld bytes remaining\n", (long)off, (long)left);

    while (left > 0) {
        int send_window_size = MAP_MAX_SIZE > (int)left ? (int)left : MAP_MAX_SIZE;
        char* addr = (char*)mmap(NULL, send_window_size, PROT_READ, MAP_PRIVATE, fd, off);
        if(addr == MAP_FAILED) {
            perror("mmap");
            break;
        }

        int ret = send(task->peerfd, addr, send_window_size, 0);
        if(ret == -1) {
            perror("send file");
            munmap(addr, send_window_size);
            break;
        }

        munmap(addr, send_window_size);
        left -= send_window_size;
        off += send_window_size;
    }

    if(left == 0) {
        printf("send file success\n");
    } else {
        printf("send file incomplete, %ld bytes left\n", (long)left);
    }

    close(fd);
    release_connection(Mysqlpool, pconn);
    printf("connection released to pool\n");
}



void userLoginCheck1(task_t * task) {
    printf("userLoginCheck1.\n");
    // O(1)查找用户
    user_info_t * user = find_user_by_sockfd(task->peerfd);
    if(user != NULL) {
        //拷贝用户名
        strncpy(user->name, task->data, sizeof(user->name) - 1);
        user->name[sizeof(user->name) - 1] = '\0';
        loginCheck1(user, Mysqlpool);
    }
}

void userLoginCheck2(task_t * task) {
    printf("userLoginCheck2.\n");
    // O(1)查找用户
    user_info_t * user = find_user_by_sockfd(task->peerfd);
    if(user != NULL) {
        //拷贝加密密文
        loginCheck2(user, task->data, Mysqlpool);
    }
}


void userrRegisterCheck1(task_t * task) {
    printf("userrRegisterCheck1.\n");
    // O(1)查找用户
    user_info_t * user = find_user_by_sockfd(task->peerfd);
    if(user != NULL) {
        memset(user->name, 0, sizeof(user->name));
        memset(user->encrypted, 0, sizeof(user->encrypted));
        strncpy(user->name, task->data, sizeof(user->name) - 1);
        user->name[sizeof(user->name) - 1] = '\0';
        RegisterCheck1(user, Mysqlpool);
    }
}

void userrRegisterCheck2(task_t * task) {
    printf("userrRegisterCheck2.\n");
    // O(1)查找用户
    user_info_t * user = find_user_by_sockfd(task->peerfd);
    if(user != NULL) {
        //拷贝加密密文
        RegisterCheck2(user, task->data, Mysqlpool);
    }
}
