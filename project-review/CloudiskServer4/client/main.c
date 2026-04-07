#include "client.h"
#include <func.h>
#include <sys/select.h>
char* global_jwt_token = NULL; 
char global[30];
int downloadflag = 0;
void displayMenu() {
    printf("Please select an option:\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("Enter your choice: ");
}



train_t t;
// 长任务传输函数：在已有连接上直接传输，不需要第二个端口
void* transferTask(void* arg) {
    downloadflag = 1;
    int clientfd = (int) arg;
    // 直接使用已连接的clientfd，不需要重新连接9999端口
    // JWT认证已经在主连接完成，不需要重新认证
    if (t.type == CMD_TYPE_GETS) {
        getsCommand(clientfd, &t);
    } else if (t.type == CMD_TYPE_PUTS) {
        putsCommand(clientfd, &t);
    }
    downloadflag = 0;
    return NULL;
}

int main()
{
    int clientfd = tcpConnect("127.0.0.1", 8080);
    int myuserfd;
    recv(clientfd,&myuserfd,sizeof(myuserfd),0);
    printf("dafaff  %d ------------------\n",myuserfd);
    int choice = 0;
    while (choice != 1 && choice != 2) {
        displayMenu();
        scanf("%d", &choice);
        while (getchar() != '\n'); // 清除输入缓冲区
        switch (choice) {
            case 1:
                userLogin(clientfd);
                break;
            case 2:
                userRegister(clientfd);
                choice = 0;
                continue;
            default:
                printf("Invalid choice. Please try again.\n");
                choice = 0;
        }
    }


    char buf[128] = {0};
    //4. 使用select进行监听
    fd_set rdset;
    while(1) {
        FD_ZERO(&rdset);
        FD_SET(STDIN_FILENO, &rdset);
        FD_SET(clientfd, &rdset);
        
        int nready = select(clientfd + 1, &rdset, NULL, NULL, NULL);
        printf("nready:%d\n", nready);
        if(FD_ISSET(STDIN_FILENO, &rdset)) {
            //读取标准输入中的数据
            memset(buf, 0, sizeof(buf));
            int ret = read(STDIN_FILENO, buf, sizeof(buf));
            if(0 == ret) {
                printf("byebye.\n");
                break;
            }
            memset(&t, 0, sizeof(t));
            //解析命令行
            buf[strlen(buf)-1] = '\0';
            parseCommand(buf, strlen(buf), &t);
            sendn(clientfd, &t, 4 + 4 + t.len);

            if(t.type == CMD_TYPE_GETS || t.type == CMD_TYPE_PUTS){
                pthread_t tid;
                pthread_create(&tid, NULL, transferTask, (void*)myuserfd);
                pthread_detach(tid);
            }

        } else if(FD_ISSET(clientfd, &rdset)) {
            int ret = recv(clientfd, buf, sizeof(buf), 0);
            if(ret == 0){
                printf("服务器关闭了连接");
                break;
            }
            printf("recv:%s\n", buf);
        }
    }
    close(clientfd);
    while(downloadflag == 1){
        
    }
    return 0;
}

