#include "user.h"
#include "thread_pool.h"
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <crypt.h>

char *GenerateStr(int len) {
    char* str = calloc(len + 1, sizeof(char)); // 分配内存并初始化为0
    if (str == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    int i, flag;
    for (i = 0; i < len; i++) {
        flag = rand() % 3;
        switch (flag) {
            case 0: str[i] = rand() % 26 + 'a'; break;
            case 1: str[i] = rand() % 26 + 'A'; break;
            case 2: str[i] = rand() % 10 + '0'; break;
        }
    }
    return str;
}

void loginCheck1(user_info_t * user, ConnectionPool *pool)
{
    if (pool == NULL || user == NULL) {
        return;
    }

    MYSQL* pconn = get_connection(pool);
    printf("loginCheck1: got connection from pool\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));

    char sql[256];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0) {
        t.len = 0;
        t.type = TASK_LOGIN_SECTION1_RESP_ERROR;
        ret = sendn(user->sockfd, &t, 8);
        release_connection(pool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT salt,cryptpasswd FROM user WHERE username = '%s'", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(pool, pconn);
        return;
    }

    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(pool, pconn);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){// 用户不存在的情况下
        t.len = 0;
        t.type = TASK_LOGIN_SECTION1_RESP_ERROR;
        ret = sendn(user->sockfd, &t, 8);
        printf("check1 send %d bytes.\n", ret);
        mysql_free_result(result);
        release_connection(pool, pconn);
        return;
    }

    //用户存在的情况下
    char setting[100] = {0};
    //保存加密密文
    strncpy(user->encrypted, row[1], sizeof(user->encrypted) - 1);
    user->encrypted[sizeof(user->encrypted) - 1] = '\0';
    //提取setting
    strcpy(setting, row[0]);
    t.len = strlen(setting);
    t.type = TASK_LOGIN_SECTION1_RESP_OK;
    strncpy(t.buff, setting, t.len);
    //发送setting
    ret = sendn(user->sockfd, &t, 8 + t.len);
    mysql_free_result(result);
    release_connection(pool, pconn);
    printf("check1 send %d bytes.\n", ret);
}

void loginCheck2(user_info_t * user, const char * encrypted, ConnectionPool *pool)
{
    if (pool == NULL || user == NULL || encrypted == NULL) {
        return;
    }

    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    MYSQL* pconn = get_connection(pool);

    if(strcmp(user->encrypted, encrypted) == 0) {
        //登录成功
        user->status = STATUS_LOGIN;//更新用户登录成功的状态
        t.type = TASK_LOGIN_SECTION2_RESP_OK;

        char sql[256];
        char *escaped_username = NULL;
        int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
        if (esc_len < 0) {
            t.len = 0;
            t.type = TASK_LOGIN_SECTION1_RESP_ERROR;
            ret = sendn(user->sockfd, &t, 8);
            release_connection(pool, pconn);
            return;
        }

        snprintf(sql, sizeof(sql), "SELECT vf.path FROM user u JOIN virtual_file vf ON u.path_id = vf.id WHERE u.username = '%s';", escaped_username);
        free(escaped_username);

        if (mysql_query(pconn, sql)) {
            fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
            release_connection(pool, pconn);
            return;
        }

        MYSQL_RES *result = mysql_store_result(pconn);
        if (result == NULL) {
            fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
            release_connection(pool, pconn);
            return;
        }

        MYSQL_ROW row = mysql_fetch_row(result);
        if(row == NULL){// 用户不存在的情况下
            t.len = 0;
            t.type = TASK_LOGIN_SECTION1_RESP_ERROR;
            ret = sendn(user->sockfd, &t, 8);
            printf("check1 send %d bytes.\n", ret);
            mysql_free_result(result);
            release_connection(pool, pconn);
            return;
        }

        strncpy(user->pwd, row[0], sizeof(user->pwd) - 1);
        user->pwd[sizeof(user->pwd) - 1] = '\0';
        t.len = strlen(user->pwd);
        strcpy(t.buff, user->pwd);
        ret = sendn(user->sockfd, &t, 8 + t.len);
        printf("Login success.\n");
        char* jwt = generate_jwt(user->name);
        size_t jwt_length = strlen(jwt);

        // 发送 JWT 的长度和 JWT 数据
        sendn(user->sockfd, &jwt_length, sizeof(jwt_length));  // 先发送长度
        sendn(user->sockfd, jwt, jwt_length);                  // 再发送数据

        // 释放 JWT 内存
        l8w8jwt_free(jwt);

        mysql_free_result(result);
        release_connection(pool, pconn);
        printf("connection released to pool\n");
    } else {
        //登录失败, 密码错误
        t.type = TASK_LOGIN_SECTION2_RESP_ERROR;
        printf("Login failed.\n");
        ret = sendn(user->sockfd, &t, 8);
        release_connection(pool, pconn);
    }

    printf("check2 send %d bytes.\n", ret);
    return;
}

void RegisterCheck1(user_info_t * user, ConnectionPool *pool)
{
    if (pool == NULL || user == NULL) {
        return;
    }

    printf("RegisterCheck1.\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));
    MYSQL* pconn = get_connection(pool);
    printf("RegisterCheck1: got connection from pool\n");

    char sql[256];
    char *escaped_username = NULL;
    int esc_len = sql_escape_string(pconn, user->name, &escaped_username);
    if (esc_len < 0) {
        t.len = 0;
        t.type = TASK_Register_SECTION1_RESP_ERROR;
        ret = sendn(user->sockfd, &t, 8);
        release_connection(pool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT id FROM user WHERE username = '%s'", escaped_username);
    free(escaped_username);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(pool, pconn);
        return;
    }

    MYSQL_RES *result = mysql_store_result(pconn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(pconn));
        release_connection(pool, pconn);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if(row != NULL){// 用户存在的情况下
        t.len = 0;
        t.type = TASK_Register_SECTION1_RESP_ERROR;
        ret = sendn(user->sockfd, &t, 8);
        printf("check1 send %d bytes.\n", ret);
        mysql_free_result(result);
        release_connection(pool, pconn);
        return;
    }

    //生成一个加密的salt
    const char * prefix = "$y$";
    int flag1 = rand() % 11;
    char s[31];
    strcpy(s, crypt_gensalt(prefix, flag1, NULL, 0));

    t.len = strlen(s);
    t.type = TASK_Register_SECTION1_RESP_OK;
    strncpy(t.buff, s, t.len);
    //发送salt
    strncpy(user->encrypted, s, sizeof(user->encrypted) - 1);
    user->encrypted[sizeof(user->encrypted) - 1] = '\0';
    ret = sendn(user->sockfd, &t, 8 + t.len);
    printf("check1 send %d bytes.\n", ret);
    mysql_free_result(result);
    release_connection(pool, pconn);
    printf("connection released to pool\n");
}

void RegisterCheck2(user_info_t * user, const char * encrypted, ConnectionPool *pool)
{
    if (pool == NULL || user == NULL || encrypted == NULL) {
        return;
    }

    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    MYSQL* pconn = get_connection(pool);
    printf("RegisterCheck2: got connection from pool\n");
    long long int inserted_user_id;
    long long int inserted_vf_id;

    char sql[512];
    char *escaped_username = NULL;
    char *escaped_salt = NULL;
    char *escaped_encrypted = NULL;
    int esc_user = sql_escape_string(pconn, user->name, &escaped_username);
    int esc_salt = sql_escape_string(pconn, user->encrypted, &escaped_salt);
    int esc_enc = sql_escape_string(pconn, encrypted, &escaped_encrypted);

    if (esc_user < 0 || esc_salt < 0 || esc_enc < 0) {
        t.type = TASK_Register_SECTION2_RESP_ERROR;
        printf("Register failed: escape error\n");
        ret = sendn(user->sockfd, &t, 8);
        free(escaped_username);
        free(escaped_salt);
        free(escaped_encrypted);
        release_connection(pool, pconn);
        return;
    }

    //INSERT INTO user (username, salt, cryptpasswd, path_id) VALUES ('newuser', 'randomSalt', 'encryptedPassword', 1);
    snprintf(sql, sizeof(sql),
        "INSERT INTO user (username, salt, cryptpasswd, path_id) VALUES ('%s', '%s', '%s', NULL)",
        escaped_username, escaped_salt, escaped_encrypted);

    free(escaped_username);
    free(escaped_salt);
    free(escaped_encrypted);

    if (mysql_query(pconn, sql)) {
        //注册失败
        t.type = TASK_Register_SECTION2_RESP_ERROR;
        printf("Register failed.\n");
        ret = sendn(user->sockfd, &t, 8);
        fprintf(stderr, "Insert error %d: %s\n", mysql_errno(pconn), mysql_error(pconn));
        release_connection(pool, pconn);
        return;
    }

    printf("Insert user success.\n");
    inserted_user_id = mysql_insert_id(pconn);
    printf("Inserted row ID: %lld\n", inserted_user_id);

    //插入虚拟文件表
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", ROOT_PATH, user->name);

    char *escaped_path = NULL;
    int esc_path = sql_escape_string(pconn, path, &escaped_path);
    if (esc_path < 0) {
        t.type = TASK_Register_SECTION2_RESP_ERROR;
        printf("Register failed: path escape error\n");
        ret = sendn(user->sockfd, &t, 8);
        release_connection(pool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql),
        "INSERT INTO virtual_file (parent_id, filename, owner_id, hash, filesize, type, path) "
        "VALUES (0, '%s', %lld, 12345, 0, 'd', '%s')",
        escaped_username, inserted_user_id, escaped_path);
    free(escaped_path);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "Insert error %d: %s\n", mysql_errno(pconn), mysql_error(pconn));
    } else {
        printf("Insert virtual file success.\n");
        inserted_vf_id = mysql_insert_id(pconn);
        printf("Inserted virtual file ID: %lld\n", inserted_vf_id);
    }

    //更新用户表path_id字段为virtual_file 的 id主键字段
    snprintf(sql, sizeof(sql),
        "UPDATE user SET path_id = %lld WHERE id = %lld",
        inserted_vf_id, inserted_user_id);

    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "Update error %d: %s\n", mysql_errno(pconn), mysql_error(pconn));
    } else {
        printf("RegisterCheck2 update success.\n");
    }

    strncpy(user->encrypted, encrypted, sizeof(user->encrypted) - 1);
    user->encrypted[sizeof(user->encrypted) - 1] = '\0';
    release_connection(pool, pconn);
    printf("connection released to pool\n");

    //告诉用户注册成功
    memset(&t, 0, sizeof(t));
    t.type = TASK_Register_SECTION2_RESP_OK;
    t.len = strlen(path);
    strcpy(t.buff, path);
    ret = sendn(user->sockfd, &t, 8 + t.len);
    printf("Register success.\n");
    printf("check2 send %d bytes.\n", ret);
    return;
}
