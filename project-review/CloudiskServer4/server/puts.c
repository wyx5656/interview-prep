#include "thread_pool.h"
#include "linked_list.h"
#include "user.h"
#include "db.h"
#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#define MAX_LENGTH 1024
extern ListNode * userList;
extern ConnectionPool *Mysqlpool;

void putsCommand(task_long_t * task) {

    printf("execute puts command.\n");
    // 1. 接收 SHA256 值
    char sha256[65];
    int ret = recv(task->peerfd, sha256, sizeof(sha256), 0);
    if(ret == 0){
        printf("客户端指令错误连接\n");
        return;
    }
    if (ret == -1) {
        return;
    }
    // 确保NUL终止
    if (ret < 65) {
        sha256[ret] = '\0';
    } else {
        sha256[64] = '\0';
    }
    printf("received SHA256: %s\n", sha256);
    if(strcmp("File open error", sha256) == 0){
        send(task->peerfd, "try again or do others!", 24, 0);
        return;
    }

    // 2. 接收文件大小
    int file_len;
    ret = recvn(task->peerfd, &file_len, sizeof(file_len));
    if (ret == -1) {
        return;
    }
    if(ret == 0){
        printf("客户端断开连接\n");
        return;
    }
    printf("received file size: %d\n", file_len);

    //3、 拿到用户信息 - 使用哈希表O(1)查找
    extern HashTable userTable;
    char key[32];
    snprintf(key, sizeof(key), "%d", task->originfd);
    user_info_t *user = (user_info_t*)find(&userTable, key);
    if (user == NULL) {
        printf("User not found for fd %d\n", task->originfd);
        send(task->peerfd, "Error: User not found", 22, 0);
        return;
    }

    //4、 从连接池获取数据库连接
    MYSQL *pconn = get_connection(Mysqlpool);
    if (pconn == NULL) {
        fprintf(stderr, "Failed to get MySQL connection from pool\n");
        return;
    }
    printf("putsCommand: got connection from pool\n");

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

    printf("Executing SQL query\n");
    if (mysql_query(pconn, sql)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(pconn));
        release_connection(Mysqlpool, pconn);
        return;
    }
    printf("SQL query executed successfully\n");
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
    mysql_free_result(result);

    char filepath[512];
    char filepath1[512];
    strncpy(filepath, user->pwd, sizeof(filepath) - 1);
    filepath[sizeof(filepath) - 1] = '\0';
    strcat(filepath, "/");

    snprintf(filepath1, sizeof(filepath1), "%s%s", filepath, task->data);
    printf("Target filepath: %s\n", filepath1);

    //5、 查找当前目录的 id
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
    if (row == NULL) {
        send(task->peerfd, "Error: Current directory not found", 35, 0);
        mysql_free_result(result);
        release_connection(Mysqlpool, pconn);
        return;
    }
    int parent_id = atoi(row[0]);
    mysql_free_result(result);

    // 6. 检查 SHA256 是否已经存在于 global_file 表
    // SHA256 本身是十六进制，不包含特殊字符，可以不需要转义，但为了安全还是转义
    char *escaped_sha256 = NULL;
    int esc_sha = sql_escape_string(pconn, sha256, &escaped_sha256);
    if (esc_sha < 0) {
        send(task->peerfd, "Error: Database error", 20, 0);
        release_connection(Mysqlpool, pconn);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT id FROM global_file WHERE hash = '%s'", escaped_sha256);
    free(escaped_sha256);

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
    int global_file_id;

    //7、 如果存在
    if(row != NULL){
        mysql_free_result(result);
        global_file_id = atoi(row[0]);
        printf("File content (hash %s) already exists in global store\n", sha256);

        //先查虚拟文件表有没有这个文件
        char *escaped_filepath1 = NULL;
        int esc_fp1 = sql_escape_string(pconn, filepath1, &escaped_filepath1);
        if (esc_fp1 < 0) {
            send(task->peerfd, "Error: Database error", 20, 0);
            free(escaped_filepath1);
            release_connection(Mysqlpool, pconn);
            return;
        }

        snprintf(sql, sizeof(sql), "SELECT id FROM virtual_file WHERE path = '%s'", escaped_filepath1);
        free(escaped_filepath1);

        if (mysql_query(pconn, sql)) {
            fprintf(stderr, "SELECT into virtual_file failed: %s\n", mysql_error(pconn));
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
            mysql_free_result(result);
            // 1. 插入文件信息到虚拟文件表
            char *escaped_filename = NULL;
            escaped_username = NULL;
            escaped_filepath1 = NULL;
            int esc_fn = sql_escape_string(pconn, task->data, &escaped_filename);
            esc_len = sql_escape_string(pconn, user->name, &escaped_username);
            esc_fp1 = sql_escape_string(pconn, filepath1, &escaped_filepath1);

            if (esc_fn < 0 || esc_len < 0 || esc_fp1 < 0) {
                send(task->peerfd, "Error: Database error", 20, 0);
                free(escaped_filename);
                free(escaped_username);
                free(escaped_filepath1);
                release_connection(Mysqlpool, pconn);
                return;
            }

            snprintf(sql, sizeof(sql),
                "INSERT INTO virtual_file (parent_id, filename, owner_id, hash, filesize, type, path) "
                "VALUES (%d, '%s', (SELECT id FROM user WHERE username = '%s'), %d, %d, 'f', '%s')",
                parent_id, escaped_filename, escaped_username, global_file_id, file_len, escaped_filepath1);

            free(escaped_filename);
            free(escaped_username);
            free(escaped_filepath1);

            if (mysql_query(pconn, sql)) {
                fprintf(stderr, "INSERT into virtual_file failed: %s\n", mysql_error(pconn));
            }

            printf("file ref added, content already exists\n");
            send(task->peerfd, "1", 2, 0);
        }else{
            mysql_free_result(result);
            printf("file already exists in virtual file table\n");
            send(task->peerfd, "1", 2, 0);
            send(task->peerfd, "Don't upload duplicate files", 29, 0);
        }
        release_connection(Mysqlpool, pconn);
        return;

    }else{
        mysql_free_result(result);
        send(task->peerfd, "0", 2, 0);

        // 1. 打开或创建文件以写入
        // 使用当前项目正确的基础路径，支持环境变量覆盖默认值
        char server_file_path[256];
        const char *base_path = getenv("CLOUDISK_FILES");
        if (base_path == NULL) {
            base_path = "./files";
        }
        snprintf(server_file_path, sizeof(server_file_path), "%s/%s", base_path, sha256);
        int fd = open(server_file_path, O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            perror("open");
            release_connection(Mysqlpool, pconn);
            return;
        }

        // 2. 设置文件大小
        if (ftruncate(fd, file_len) == -1) {
            perror("ftruncate");
            close(fd);
            release_connection(Mysqlpool, pconn);
            return;
        }

        // 3. 接收客户端发送的已上传偏移量（支持断点续传）
        int offset = 0;
        ret = recv(task->peerfd, &offset, sizeof(offset), 0);
        if (ret <= 0) {
            perror("recv offset");
            close(fd);
            release_connection(Mysqlpool, pconn);
            return;
        }
        // 边界检查
        if (offset < 0) offset = 0;
        if (offset > file_len) offset = file_len;
        printf("Resuming upload from offset: %d/%d\n", offset, file_len);

        // 4. 映射文件到内存，从偏移位置开始
        int remaining = file_len - offset;
        char *pMap = mmap(NULL, file_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (pMap == MAP_FAILED) {
            perror("mmap");
            close(fd);
            release_connection(Mysqlpool, pconn);
            return;
        }
        char *pMapOffset = pMap + offset;

        // 5. 接收文件内容，从偏移位置开始
        ret = recvn(task->peerfd, pMapOffset, remaining);
        if (ret == -1) {
            munmap(pMap, file_len);
            close(fd);
            release_connection(Mysqlpool, pconn);
            return;
        }
        printf("received file content, %d bytes\n", ret);

        // 5. 释放映射并关闭文件
        munmap(pMap, file_len);
        close(fd);

        // 6. 插入文件信息到全局文件表
        escaped_sha256 = NULL;
        int esc_sha256 = sql_escape_string(pconn, sha256, &escaped_sha256);
        if (esc_sha256 < 0) {
            send(task->peerfd, "Error: Database error", 20, 0);
            free(escaped_sha256);
            release_connection(Mysqlpool, pconn);
            return;
        }

        snprintf(sql, sizeof(sql), "INSERT INTO global_file (hash) VALUES ('%s')", escaped_sha256);
        free(escaped_sha256);

        if (mysql_query(pconn, sql)) {
            fprintf(stderr, "INSERT error: %s\n", mysql_error(pconn));
        }
        global_file_id = mysql_insert_id(pconn);

        // 7. 插入文件信息到虚拟文件表
        char *escaped_filename = NULL;
        escaped_username = NULL;
        char *escaped_filepath1 = NULL;
        int esc_fn = sql_escape_string(pconn, task->data, &escaped_filename);
        esc_len = sql_escape_string(pconn, user->name, &escaped_username);
        int esc_fp = sql_escape_string(pconn, filepath1, &escaped_filepath1);

        if (esc_fn < 0 || esc_len < 0 || esc_fp < 0) {
            send(task->peerfd, "Error: Database error", 20, 0);
            free(escaped_filename);
            free(escaped_username);
            free(escaped_filepath1);
            release_connection(Mysqlpool, pconn);
            return;
        }

        snprintf(sql, sizeof(sql),
            "INSERT INTO virtual_file (parent_id, filename, owner_id, hash, filesize, type, path) "
            "VALUES (%d, '%s', (SELECT id FROM user WHERE username = '%s'), %d, %d, 'f', '%s')",
            parent_id, escaped_filename, escaped_username, global_file_id, file_len, escaped_filepath1);

        free(escaped_filename);
        free(escaped_username);
        free(escaped_filepath1);

        if (mysql_query(pconn, sql)) {
            fprintf(stderr, "INSERT into virtual_file failed: %s\n", mysql_error(pconn));
        }

        // 8. 释放连接回池
        release_connection(Mysqlpool, pconn);
        printf("file stored and database records inserted\n");
        send(task->peerfd, "success", 8, 0);
        return;
    }
}
