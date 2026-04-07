/**
 * @file Linux01.cc
 * @brief Linux 后端面试知识点整理 - 进程线程、网络、IO多路复用
 *
 * 包含常见面试问题 + 简要代码实例
 */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. 进程和线程的基本概念
// ============================================================================

/*
进程：资源分配的最小单位，一个进程包含多个线程。
- 进程有独立的地址空间（虚拟地址空间）
- 进程间共享：打开的文件描述符、内存数据（需要通过IPC）、信号处理

线程：CPU调度的最小单位
- 同一进程内所有线程共享进程的地址空间和资源
- 线程私有：自己的栈空间（函数调用、局部变量）、寄存器上下文（切换时需要保存恢复）

进程切换开销大：需要切换页表、切换地址空间、刷新TLB
线程切换开销小：只需要切换寄存器上下文，共享地址空间不需要换页表

面试考点：
Q: 进程和线程区别？
A: 进程是资源分配单位，线程是调度单位。进程有独立地址空间，线程共享。切换开销不同。
*/

// ============================================================================
// 2. 进程间通信(IPC)方式
// ============================================================================

/*
1. 管道 (pipe) - 半双工，只能父子进程通信
   - 其实现在也可以兄弟进程用，因为fork后都保持fd打开
   - 大小有限（PIPE_BUF = 4096）
   - 数据是字节流，没有边界

   代码示例：
*/
static void pipe_example() {
    int pipefd[2];
    pipe(pipefd);  // pipefd[0]读端, pipefd[1]写端

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程 -> 父进程 发送数据
        close(pipefd[0]);  // 关闭不用的读端
        const char *msg = "hello from child";
        write(pipefd[1], msg, strlen(msg));
        close(pipefd[1]);
        exit(0);
    } else {
        close(pipefd[1]);  // 父进程关闭写端
        char buf[100] = {0};
        read(pipefd[0], buf, sizeof(buf));
        printf("父进程收到: %s\n", buf);
        close(pipefd[0]);
        wait(NULL);
    }
}

/*
2. 有名管道 (FIFO) - 可以任意进程通信
   - 磁盘上有文件名，通过mkfifo创建，open打开像普通文件一样读写
   - 可以用于不相关进程之间通信

3. 内存映射 (mmap)
   - 把一个文件映射到进程虚拟地址空间，多个进程映射同一个文件，就能共享这块内存
   - 读写直接操作内存，不需要read/write，少一次拷贝
   - 可以用于进程间共享数据，也可以加快文件访问

   原型：
   void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
   munmap(void *addr, size_t length);
*/

/*
4. 套接字 (socket)
   - 跨网络通信，也可以本机通信（Unix域socket）
   - 最通用，可以跨主机

5. 消息队列、信号量、共享内存（System V IPC）
   - 消息队列：有序链表，发送/接收消息块
   -信号量：计数器，实现进程间同步互斥
*/

// ============================================================================
// 3. TCP/IP 四层模型
// ============================================================================

/*
应用层   HTTP / HTTPS / FTP / DNS / SSH
  （应用层负责处理用户数据，封装请求响应）

传输层   TCP / UDP
  - TCP：面向连接、可靠传输、流量控制、拥塞控制
  - UDP：无连接、不可靠、快（DNS、视频流用UDP）

网络层   IP / ICMP / 路由寻址
  - 负责把数据包从源地址路由到目的地址

链路层   以太网帧传输、物理层硬件传输
  - 处理硬件地址，封装成帧发送

为什么分层？
- 分层把复杂问题拆分，每层只负责一件事，解耦
- 各层可以独立演进，下层变了上层不用改
*/

// ============================================================================
// 4. TCP 三次握手 四次挥手
// ============================================================================

/*
三次握手（建立连接）：

客户端初始 CLOSED → 发送 SYN (seq=x) → SYN-SENT
服务端 LISTEN → 收到SYN，回复 SYN+ACK (seq=y, ack=x+1) → SYN-RECEIVED
客户端 → 收到后回复 ACK (ack=y+1) → ESTABLISHED
服务端 → 收到ACK → ESTABLISHED

为什么要三次握手？
- 确认双方收发能力都正常：
  客户端发SYN，服务端能收到 → 客户端能发，服务端能收
  服务端回SYN+ACK，客户端能收到 → 服务端能发能收，客户端能发能收
  客户端再发ACK，服务端能收到 → 双方都确认了
- 防止过期重复连接（比如客户端延迟的SYN突然到了，三次握手能避免错误建立）

四次挥手（断开连接）：

主动断开方 FIN-WAIT-1 → 发送 FIN → 等待ACK
被动断开方 CLOSE-WAIT → 收到FIN，回复ACK → 等待应用层关闭
主动断开方 FIN-WAIT-2 → 收到ACK → 等待对方发FIN
被动断开方 LAST-ACK → 应用层关闭了，发送FIN → 等待最后ACK
主动断开方 TIME-WAIT → 收到FIN，回复ACK → 等待 2MSL 超时后进入CLOSED
被动断开方 → 收到ACK → CLOSED

为什么需要 TIME-WAIT？
1. 保证最后一个ACK能到达对方，如果对方没收到，对方会重发FIN，我们还能回复ACK
2. 防止旧连接的报文干扰新连接（旧连接的报文延迟了，TIME-WAIT等2MSL让旧报文消失）

2MSL 是多久？MSL是Maximum Segment Lifetime，一个报文在网络中最大存活时间，通常是1-2分钟。
*/

// ============================================================================
// 5. socket 网络编程流程
// ============================================================================

/*
服务端流程：
socket() → bind() → listen() → accept() → recv/send 或 read/write → close()

客户端流程：
socket() → connect() → recv/send → close()

代码简要框架：
*/

// 服务端基本流程示例
static int tcp_server_example(uint16_t port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定所有网卡
    addr.sin_port = htons(port);       // 主机字节序转网络字节序

    // 地址复用，解决TIME_WAIT导致bind失败问题
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        return -1;
    }

    printf("服务端监听端口 %d\n", port);

    // 阻塞等待客户端连接
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
    printf("新客户端连接: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 读取数据
    char buf[1024];
    int n = read(clientfd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("收到: %s\n", buf);
        // 回显
        write(clientfd, buf, n);
    }

    close(clientfd);
    close(listenfd);
    return 0;
}

// ============================================================================
// 6. IO多路复用 select/poll/epoll 对比
// ============================================================================

/*
---------------- select ----------------
数据结构：fd_set 是位图，每个bit对应一个fd
- 最大限制：默认最多 1024 个fd（受限于内核位图大小）
- 流程：
  1. 用户空间初始化 fd_set，把要监听的fd设置进去
  2. 调用select()，内核遍历所有fd检查是否就绪，把就绪的fd对应的bit置1，返回用户态
  3 用户空间遍历 0~1024 找出哪些bit是1，处理
  4. 下次select需要重新设置fd_set，因为内核修改了它

缺点：
- 并发连接数有限（默认1024）
- 每次都要全量遍历，fd多了性能差
- 每次调用都要从用户空间拷贝fd_set到内核空间

代码示例：
*/

static void select_example(int listenfd) {
    fd_set readfds;
    int maxfd = listenfd;
    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds);

    while (1) {
        // select 会修改readfds，所以每次要重新设置
        fd_set tmp = readfds;
        int ready = select(maxfd + 1, &tmp, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(listenfd, &tmp)) {
            // 有新连接
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
            FD_SET(clientfd, &readfds);
            if (clientfd > maxfd) maxfd = clientfd;
            printf("新连接 %d\n", clientfd);
        }

        // 检查每个已连接fd
        for (int fd = listenfd + 1; fd <= maxfd; fd++) {
            if (FD_ISSET(fd, &tmp)) {
                char buf[1024];
                int n = read(fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    // 断开连接
                    close(fd);
                    FD_CLR(fd, &readfds);
                    continue;
                }
                buf[n] = '\0';
                printf("fd %d 收到: %s\n", fd, buf);
                write(fd, buf, n); // 回显
            }
        }
    }
}

/*
---------------- poll ----------------
和select类似，区别：
- 用数组保存fd，没有1024限制
- 接口更简单，不用每次重新设置fd_set
- 还是需要用户遍历所有fd找就绪的，fd多了还是慢

---------------- epoll ----------------
数据结构：红黑树 + 就绪链表
- epoll_create() 创建epoll实例，内核分配空间
- epoll_ctl() 添加/修改/删除fd（O(log n)）
- epoll_wait() 阻塞等待就绪，直接返回就绪链表，不用全量遍历

优点：
- 没有最大连接数限制（几万个连接没问题）
- 只返回就绪的fd，不用用户全量遍历，性能好
- 只需要在添加/删除fd时拷贝，不需要每次调用都拷贝所有fd

两种触发模式：
1. 水平触发（LT）：默认模式，只要缓冲区还有数据，epoll就会一直触发
   - 好处理，不容易丢数据，只要处理完就行

2. 边缘触发（ET）：只有状态变化（从无数据变有数据）才触发一次
   - 需要把数据读完（用非阻塞+while循环读到EAGAIN）
   - 优点：效率更高，触发次数少
   - 缺点：容易写错，漏数据

面试考点：
Q: epoll 为什么比select快？
A: 1. select有连接数限制，epoll没有；2. select每次全量遍历，epoll只返回就绪fd；3. select每次要拷贝所有fd到内核，epoll只在add/del时拷贝。

代码示例：
*/

static void epoll_example(int listenfd, int max_events = 10) {
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        return;
    }

    // 添加listenfd到epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    struct epoll_event *events = (struct epoll_event*)malloc(
        sizeof(struct epoll_event) * max_events);

    while (1) {
        int nready = epoll_wait(epfd, events, max_events, -1);
        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == listenfd) {
                // 新连接
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
                printf("新连接 %d\n", clientfd);

                // 设置为非阻塞 如果用边缘触发必须非阻塞
                // fcntl(clientfd, F_SETFL, O_NONBLOCK);

                // 添加到epoll
                ev.events = EPOLLIN;  // LT模式
                // ev.events = EPOLLIN | EPOLLET;  // ET模式
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                // 可读
                char buf[1024];
                int n = read(fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    continue;
                }
                buf[n] = '\0';
                printf("fd %d 收到: %s\n", fd, buf);
                write(fd, buf, n);
            }
        }
    }

    free(events);
    close(epfd);
}

/*
总结对比：

| 特性 | select | poll | epoll |
|------|--------|------|-------|
| 最大连接数 | ≤1024 | 无限制 | 无限制 |
| 遍历方式 | 每次全量遍历 | 每次全量遍历 | 只遍历就绪 |
| 性能 | 连接多了很慢 | 连接多了慢 | 连接多了也快 |
| 复杂度 | O(n) | O(n) | O(1) 就绪返回，O(log n) 添加删除 |

epoll 是目前Linux下高性能网络编程首选（Nginx、Redis都是用epoll）
*/

// ============================================================================
// 补充：常见面试题
// ============================================================================

/*
Q: 什么是脏读脏写？怎么解决？
A: 脏读就是一个线程读了另一个线程未提交的数据，如果另一个线程回滚了，读到的数据就是错的。
   解决：封锁协议，读数据加共享锁，写完释放，没提交不能放锁。

Q: 进程上下文切换做了什么？
A: 1. 保存当前进程寄存器上下文到PCB
   2. 切换页表，更新MMU，切换到新进程地址空间
   3. 加载新进程寄存器上下文
   4. 交给CPU执行

Q: 虚拟内存是什么？有什么好处？
A: 每个进程都认为自己占有整个地址空间，实际是操作系统把虚拟地址映射到物理地址。
   好处：
   1. 隔离：每个进程地址空间独立，不会互相干扰
   2. 方便内存管理：操作系统统一管理，换出到磁盘
   3. 方便程序加载：每个程序从0地址开始，链接方便

Q: TCP为什么是面向字节流？UDP为什么是面向报文？
A: TCP不对应用层消息分割拼接，你发多少它都可能拆分合并，所以叫字节流。
   UDP保留消息边界，你发一个报文它就整体交付，不拆分不合并，所以叫面向报文。

Q: 说说TCP滑动窗口？
A: 滑动窗口用来做流量控制，发送方维护发送窗口，接收方维护接收窗口，
   窗口内的数据才能发送，接收方通过ACK告诉发送方自己还能收多少，
   这样可以一边发送一边ACK，吞吐量更高，不用停等。
   也能解决拥塞控制问题（拥塞窗口+滑动窗口一起控制发送速率）

Q: 浏览器输入url回车都发生了什么？
A: 1. DNS解析域名得到IP地址
   2. 浏览器和服务器建立TCP连接（三次握手）
   3. 浏览器发送HTTP请求
   4. 服务器处理请求返回HTTP响应
   5. 浏览器解析渲染HTML，加载资源
   6. 页面显示完成
   7. 连接关闭（如果不用keep-alive）
*/

void test() {
    // 这里只是知识点整理，上面代码可以单独编译运行
    // pipe_example();
    // tcp_server_example(8080);
}

int main() {
    printf("Linux 面试知识点整理，请看代码注释。\n");
    return 0;
}
