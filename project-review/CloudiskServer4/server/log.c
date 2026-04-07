#include "thread_pool.h"

const char* cmdTrans(CmdType type){
    switch (type){
    case CMD_TYPE_PWD:             return "pwd";
    case CMD_TYPE_LS:              return "ls";
    case CMD_TYPE_CD:              return "cd";
    case CMD_TYPE_MKDIR:           return "mkdir";
    case CMD_TYPE_RMDIR:           return "rmdir";
    case CMD_TYPE_GETS:            return "gets";
    case CMD_TYPE_PUTS:            return "puts";
    case CMD_TYPE_NOTCMD:          return "notcmd";
    case TASK_LOGIN_SECTION1:      return "login1";
    case TASK_LOGIN_SECTION2:      return "login2";
    default:                       return "cmd";
    }
}

void log_connection(int peerfd){
    time_t now;
    struct tm *info;
    time(&now);
    info = localtime(&now);
    char time_buff[80];
    strftime(time_buff, sizeof(time_buff), "%Y-%m-%d %H:%M:%S", info);
    FILE *log_file = fopen("connection.log", "a");
    if (log_file != NULL) {
        fprintf(log_file, "客户端 %d 请求连接时间 %s\n", peerfd, time_buff);
        fclose(log_file);
    } else {
        perror("无法打开日志文件");
    }




}

void log_write(task_t* task){
    time_t now;
    struct tm *info;
    time(&now);
    info = localtime(&now);
    char time_buff[80];
    strftime(time_buff, sizeof(time_buff), "%Y-%m-%d %H:%M:%S", info);

    FILE *log_file = fopen("connection.log", "a");
    if (log_file != NULL) {
        char cmd[1024] = {0};
        snprintf(cmd, sizeof(cmd), "指令内容：%s %s", cmdTrans(task->type), task->data);
        fprintf(log_file, "来自客户端 %d 的操作: %s at %s\n", task->peerfd, cmd, time_buff);
        fclose(log_file);
    } else {
        perror("无法打开日志文件");
    }
}