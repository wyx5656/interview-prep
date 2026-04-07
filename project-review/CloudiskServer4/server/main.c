#include "config.h"
#include "linked_list.h"
#include "thread_pool.h"
#include "user.h"
#include "timeout_queue.h"
#include "db.h"
#include "hashtable.h"
#define EPOLL_ARR_SIZE 100

int exitPipe[2];
// 保持链表用于兼容删除和遍历，主要查找使用哈希表O(1)
ListNode * userList = NULL;
// 用户哈希表：key是字符串化的sockfd，value是user_info_t*
// 提供O(1)查找，比链表O(n)更高效
HashTable userTable;
pthread_mutex_t userTable_mutex; // 保护哈希表并发访问
//全局数据库连接池实例
ConnectionPool *Mysqlpool = NULL;
// 长任务线程池 - 只处理puts/gets上传下载，短任务直接在主线程执行
// 架构设计（按用户建议优化）：
// - 主线程epoll → 直接读取解析所有命令
// - 短任务(pwd/ls/cd/mkdir/rmdir/login/register) → 直接在主线程执行，快速完成不阻塞
// - 长任务(puts/gets) → 扔给线程池异步执行，主线程继续处理新请求
threadpool_t long_task_pool;
// 超时队列 - 全局可访问，用于连接断开时删除节点防止泄漏
TimeoutQueue *TimeoutQueue_global = NULL;
void sigHandler(int num)
{
    printf("\n sig is coming.\n");
    //激活管道, 往管道中写一个1
    int one = 1;
    write(exitPipe[1], &one, sizeof(one));
}



int main(int argc, char ** argv)
{   
    //命令行参数要读取配置文件
    ARGS_CHECK(argc, 2);
    //创建匿名管道
    pipe(exitPipe);
    //fork之后，将创建了子进程
    pid_t pid = fork();

    if(pid > 0) {//父进程
        close(exitPipe[0]);//父进程关闭读端
        signal(SIGUSR1, sigHandler);
        wait(NULL);//等待子进程退出，回收其资源
        close(exitPipe[1]);
        printf("\nparent process exit.\n");
        exit(0);//父进程退出
    }
    //以下都是子进程中的执行流
    //子进程关闭写端
    close(exitPipe[1]);
    //初始化随机数生成器 - 只需要调用一次
    srand(time(NULL));
    //初始化hash表，用来存储配置信息
    HashTable ht;
    initHashTable(&ht);
    //读取服务器配置文件
    readConfig(argv[1], &ht);

    //初始化数据库连接池 - 连接池大小与线程数相同，支持动态扩容
    int thread_num = atoi((const char*)find(&ht, THREAD_NUM));
    // 读取最大连接数，如果配置不存在，使用默认值：thread_num * 4
    int max_thread_num = thread_num * 4;
    char *max_thread_str = (char*)find(&ht, "max_thread_num");
    if (max_thread_str != NULL) {
        max_thread_num = atoi(max_thread_str);
    }
    // 确保最大连接数不小于初始连接数
    if (max_thread_num < thread_num) {
        max_thread_num = thread_num;
    }
    Mysqlpool = init_connection_pool(thread_num, max_thread_num);

    //创建长任务线程池 - 只处理puts/gets，短任务直接在主线程执行
    memset(&long_task_pool, 0, sizeof(long_task_pool));
    //初始化线程池 - 线程数等于初始线程数，和数据库连接池对应
    threadpoolInit(&long_task_pool, thread_num);
    //启动线程池
    threadpoolStart(&long_task_pool);

    //创建监听套接字
    int listenfd = tcpInit(find(&ht, IP), find(&ht, PORT));
    //创建epoll实例
    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");

    //对listenfd进行监听
    addEpollReadfd(epfd, listenfd);
    addEpollReadfd(epfd, exitPipe[0]);


    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));
    TimeoutQueue queue;
    TimeoutQueue_global = &queue;
    // 初始化用户哈希表
    initHashTable(&userTable);
    // 初始化互斥锁保护哈希表并发访问
    pthread_mutex_init(&userTable_mutex, NULL);
    initQueue(&queue);
    while(1) {
        int nready = epoll_wait(epfd, pEventArr, EPOLL_ARR_SIZE,1000);
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            ERROR_CHECK(nready, -1, "epoll_wait");
        } else if(nready == 0){
            printf("1s\n");
            timeoutCheck(epfd,&queue);
        }
        else {
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == listenfd) {//对新连接进行处理
                    int peerfd = accept(listenfd, NULL, NULL);
                    printf("\n conn %d has conneted.\n", peerfd);
                    //userCheck();
                    // 设置socket为非阻塞模式，适配边缘触发
                    int flags = fcntl(peerfd, F_GETFL, 0);
                    fcntl(peerfd, F_SETFL, flags | O_NONBLOCK);
                    //将新连接添加到epoll的监听红黑树上
                    log_connection(peerfd);
                    addEpollReadfd(epfd, peerfd);
                    //添加用户节点
                    user_info_t * user = (user_info_t*)calloc(1, sizeof(user_info_t));
                    user->sockfd = peerfd;
                    int originfd = peerfd;
                    send(peerfd, &originfd, sizeof(originfd), 0);
                    //strncpy(user->pwd,ROOT_PATH,sizeof(ROOT_PATH));
                    appendNode(&userList, user);
                    // 添加到哈希表提供O(1)查找，互斥锁保护并发访问
                    char key[32];
                    snprintf(key, sizeof(key), "%d", peerfd);
                    pthread_mutex_lock(&userTable_mutex);
                    insert(&userTable, key, user);
                    pthread_mutex_unlock(&userTable_mutex);
                    //超时队列
                    addUser(&queue,user);
                } else if(fd == exitPipe[0]) {
                    //线程池要退出
                    int howmany = 0;
                    //对管道进行处理
                    read(exitPipe[0], &howmany, sizeof(howmany));
                    //主线程通知所有的子线程退出
                    threadpoolStop(&long_task_pool);
                    //子进程退出前，回收资源
                    threadpoolDestroy(&long_task_pool);
                    destroy_connection_pool(Mysqlpool);
                    close(listenfd);
                    close(epfd);
                    close(exitPipe[0]);
                    printf("\nchild process exit.\n");
                    destroyHashTable(&ht);
                    exit(0);
                } else {//客户端的连接的处理
                    // O(1)查找用户
                    char key[32];
                    snprintf(key, sizeof(key), "%d", fd);
                    user_info_t *user = (user_info_t*)find(&userTable, key);
                    if (user != NULL) {
                        updateUser(&queue, user);
                    }
                    handleMessage(fd, epfd, &long_task_pool.que);
                }
            }
        }
    }

    return 0;
}

