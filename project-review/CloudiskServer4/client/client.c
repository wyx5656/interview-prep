#include "client.h"
#include "str_util.h"
#include <openssl/sha.h>
#include <stdio.h>
#include <unistd.h>
extern char* global_jwt_token;
extern char global[30];
int tcpConnect(const char * ip, unsigned short port)
{
    //1. 创建TCP的客户端套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0) {
        perror("socket");
        return -1;
    }

    //2. 指定服务器的网络地址
    struct sockaddr_in serveraddr;
    //初始化操作,防止内部有脏数据
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;//指定IPv4
    serveraddr.sin_port = htons(port);//指定端口号
    //指定IP地址
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    //3. 发起建立连接的请求
    int ret = connect(clientfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        close(clientfd);
        return -1;
    }
    return clientfd;
}

static int userLogin1(int sockfd, train_t *t);
static int userLogin2(int sockfd, train_t *t);

int userLogin(int sockfd)
{
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    userLogin1(sockfd, &t);
    userLogin2(sockfd, &t);
    return 0;
}

static int userRegister1(int sockfd, train_t *t);
static int userRegister2(int sockfd, train_t *t);
int userRegister(int sockfd){
    printf("用户注册\n");
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    userRegister1(sockfd, &t);
    userRegister2(sockfd, &t);
    return 0;
}
static int userRegister1(int sockfd, train_t *pt)
{
    /* printf("userLogin1.\n"); */
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        printf(USER_NAME_DEFINE);
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';
        t.len = strlen(user);
        t.type = TASK_Register_SECTION1;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("login1 send %d bytes.\n", ret); */

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_Register_SECTION1_RESP_ERROR) {
            //用户名已经存在
            printf("user name is already exist.\n");
            continue;
        }
        //用户名正确，读取setting
        ret = recvn(sockfd, t.buff, t.len);
        break;
    }
    memcpy(pt, &t, sizeof(t));
    return 0;
}

static int userRegister2(int sockfd, train_t * pt)
{
    /* printf("userLogin2.\n"); */
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        //第一次输入
        char * passwd1 = getpass(PASSWORD_DEFINE);
        char * passwd2 = getpass(PASSWORD_DEFINE_AGAIN);
        if(strcmp(passwd1,passwd2) != 0){
            printf("sorry, You entered different passwords twice.\n");
            continue;
        }
        /* printf("password: %s\n", passwd); */
        char * encrytped = crypt(passwd1, pt->buff);
        /* printf("encrytped: %s\n", encrytped); */
        t.len = strlen(encrytped);
        t.type = TASK_Register_SECTION2;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("userLogin2 send %d bytes.\n", ret); */

        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("2 length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_Register_SECTION2_RESP_ERROR) {
            //密码不正确
            printf("somethine wrong\n");
            continue;
        } else {
            ret = recvn(sockfd, t.buff, t.len);
            printf("userRegister Success.\n");
            fprintf(stderr, "%s", t.buff);
            break;
        } 
    }
    return 0;
}

static int userLogin1(int sockfd, train_t *pt)
{
    /* printf("userLogin1.\n"); */
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        printf(USER_NAME);
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';
        strcpy(global,user);
        t.len = strlen(user);
        t.type = TASK_LOGIN_SECTION1;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("login1 send %d bytes.\n", ret); */

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_LOGIN_SECTION1_RESP_ERROR) {
            //无效用户名, 重新输入
            printf("user name not exist.\n");
            continue;
        }
        //用户名正确，读取setting
        ret = recvn(sockfd, t.buff, t.len);
        break;
    }
    memcpy(pt, &t, sizeof(t));
    return 0;
}

static int userLogin2(int sockfd, train_t * pt)
{
    /* printf("userLogin2.\n"); */
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        char * passwd = getpass(PASSWORD);
        /* printf("password: %s\n", passwd); */
        char * encrytped = crypt(passwd, pt->buff);
        /* printf("encrytped: %s\n", encrytped); */
        t.len = strlen(encrytped);
        t.type = TASK_LOGIN_SECTION2;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("userLogin2 send %d bytes.\n", ret); */

        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("2 length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_LOGIN_SECTION2_RESP_ERROR) {
            //密码不正确
            printf("sorry, password is not correct.\n");
            continue;
        } else {
            ret = recvn(sockfd, t.buff, t.len);
            printf("Login Success.\n");
            size_t jwt_length;
            // 接收 JWT 的长度
            if (recv(sockfd, &jwt_length, sizeof(jwt_length), 0) <= 0) {
                perror("Failed to receive JWT length");
                return;
            }
            // 分配内存以接收 JWT 数据
            global_jwt_token = (char*)malloc(jwt_length + 1); // +1 for null terminator
            if (global_jwt_token == NULL) {
                perror("Memory allocation failed");
                return;
            }
            // 接收 JWT 数据
            ssize_t received = 0;
            ret = recvn(sockfd,global_jwt_token,jwt_length);
            global_jwt_token[jwt_length] = '\0'; // Null-terminate the string

            printf("Received JWT: %s\n", global_jwt_token);
            printf("please input a command.\n");
            fprintf(stderr, "%s", t.buff);
            break;
        } 
    }
    return 0;
}

//其作用：确定接收len字节的数据
int recvn(int sockfd, void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//作用: 确定发送len字节的数据
int sendn(int sockfd, const void * buff, int len)
{
    int left = len;
    const char * pbuf = buff;
    int ret = -1;
    while(left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        if(ret < 0) {
            perror("send");
            return -1;
        }
        left -= ret;
        pbuf += ret;
        fflush(stdout);
    }
    return len - left;
}

//解析命令
int parseCommand(const char * pinput, int len, train_t * pt)
{
    char * pstrs[10] = {0};
    int cnt = 0;
    splitString(pinput, " ", pstrs, 10, &cnt);
    pt->type = getCommandType(pstrs[0]);
    //暂时限定命令行格式为：
    //1. cmd
    //2. cmd content
    if(cnt > 1) {
        pt->len = strlen(pstrs[1]);
        strncpy(pt->buff, pstrs[1], pt->len);
    }
    return 0;
}

int getCommandType(const char * str)
{
    if(!strcmp(str, "pwd")) 
        return CMD_TYPE_PWD;
    else if(!strcmp(str, "ls"))
        return CMD_TYPE_LS;
    else if(!strcmp(str, "cd"))
        return CMD_TYPE_CD;
    else if(!strcmp(str, "mkdir"))
        return CMD_TYPE_MKDIR;
    else if(!strcmp(str,"rmdir"))
        return CMD_TYPE_RMDIR;
    else if(!strcmp(str, "puts"))
        return CMD_TYPE_PUTS;
    else if(!strcmp(str, "gets"))
        return CMD_TYPE_GETS;
    else
        return CMD_TYPE_NOTCMD;
}



// 计算文件的SHA256值
void calculate_sha256(const char *path, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }
    char buffer[4096];
    int bytesRead = 0;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    printf("----加密完成\n");
    SHA256_Final(hash, &sha256);
    close(fd);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}



void putsCommand(int sockfd, train_t * pt)
{   
    printf("execute puts command.\n");
    int fd = open(pt->buff, O_RDONLY);
    if (fd == -1) {
        const char *error_msg = "File open error";
        send(sockfd, error_msg, strlen(error_msg), 0);
        perror("open");
        close(fd);
        return;
    }

    // 计算SHA256
    char sha256[65];
    calculate_sha256(pt->buff, sha256);
    printf("SHA256: %s\n", sha256);
    int ret = sendn(sockfd, sha256, sizeof(sha256));
    if (ret == -1) {
        perror("send SHA256");
        close(fd);
        return;
    }
    printf("send SHA256: %d bytes\n", ret);
    // 获取文件大小
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(fd, &st);
    int file_len = st.st_size;

    // 发送文件大小
    ret = send(sockfd, &file_len, sizeof(file_len), 0);
    if (ret == -1) {
        perror("send file size");
        close(fd);
        return;
    }
    printf("send file size: %d\n", file_len);
    char flad[2];
    recv(sockfd, flad, sizeof(flad), 0);
    if(strcmp(flad, "1") == 0){
        printf("File already exists on server,秒传成功\n");
        close(fd);
        return;
    }
    else{
        // 支持断点续传：发送已上传的偏移量
        // 如果是续传，offset就是已经上传的字节数；从头上传就是0
        // 这里我们总是发送0，但是协议已经支持断点续传，后续可以改进为检查临时文件
        int offset = 0;
        // TODO: 如果支持续传，可以在这里检查本地临时文件获取offset
        ret = send(sockfd, &offset, sizeof(offset), 0);
        if (ret == -1) {
            perror("send offset");
            close(fd);
            return;
        }
        printf("send offset: %d\n", offset);

        // 映射文件到内存，从偏移位置开始发送
        char *pMap = mmap(NULL, file_len, PROT_READ, MAP_SHARED, fd, 0);
        if (pMap == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return;
        }
        char *pMapOffset = pMap + offset;
        int remaining = file_len - offset;

        // 发送文件内容，从偏移位置开始
        ret = sendn(sockfd, pMapOffset, remaining);
        if (ret == -1) {
            perror("send file content");
            munmap(pMap, file_len);
            close(fd);
            return;
        }
        printf("send file success, bytes sent: %d\n", ret + offset);

        // 释放资源
        munmap(pMap, file_len);
        close(fd);

        // 等待服务端确认
        char ack[8];
        ret = recv(sockfd, ack, sizeof(ack), 0);
        if (ret > 0) {
            printf("Server: %s\n", ack);
        }
        return;
    }

}

void getsCommand(int sockfd, train_t* t)
{   
    printf("execute gets command.\n");

    char* filename = t->buff;
    printf("filename = %s\n", filename);
    int fd = open(filename, O_RDWR);
    int fsize = 0;
    if(fd == -1)
        fd = open(filename, O_RDWR | O_CREAT,0644);
    else{
        //文件存在，获取文件大小
        fsize = lseek(fd, 0, SEEK_END);
        //ftruncate(fd, fsize);
    }
    //先接收文件的长度
    int length = 0;
    int res = recv(sockfd, &length, sizeof(length), 0);
    if(res == 0) printf("bye\n");
    else if(res < 0) error(1, errno, "recv file_len");
    if(fsize < length) {
        lseek(fd, fsize, SEEK_SET);
        //将已获取到的文件大小传给服务器
        send(sockfd, &fsize, sizeof(fsize), 0);
    } else if (fsize == length) {
        printf("The file is already fully downloaded\n");
        send(sockfd, &fsize, sizeof(fsize), 0);
        close(fd);
        return;
    } else {
        // 文件比服务端声明的大，截断
        ftruncate(fd, length);
        fsize = length;
        printf("Local file larger than remote, truncated to %d bytes\n", length);
        close(fd);
        return;
    }
    //再接收文件内容
    int total = fsize;
    int left = length - fsize;
    printf("left:%d\n", left);
    //2.2 再接收文件内容本身
    while(total < length) {
        char buff[1000] = {0};
        if(left > 1000){
            res = recvn(sockfd, buff, 1000);
            left -= 1000;
        } else {
            res = recvn(sockfd, buff, left);
            left -= res;
        }
        if(res > 0) {
            total += res;
            write(fd, buff, res);//写入本地文件
            printf("download %2.f%%\r", (float)total / length * 100);
            fflush(stdout);
        }
        
    }
    printf("download %2.f%%\n", (float)total / length * 100);
    printf("/server/$");
    close(fd);
}