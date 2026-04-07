/**
 * @file Linux01.cc
 * @brief Linux 后端面试知识点整理 - 进程线程、网络、IO多路复用
 *
 * 包含常见面试问题 + 完整可运行代码实例
 * 编译: g++ -o Linux01 Linux01.cc && ./Linux01
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
#include <errno.h>

using namespace std;

// ============================================================================
// 1. 进程和线程的基本概念 面试考点
// ============================================================================

/*
进程：资源分配的最小单位，一个进程包含多个线程。
- 进程有**独立的虚拟地址空间**
- 进程间共享：打开的文件描述符、信号处理（都在PCB）

线程：CPU调度的最小单位
- 同一进程**所有线程共享地址空间和资源**
- 线程私有：独立栈空间（函数调用、局部变量）、寄存器上下文

### 面试题：进程和线程区别？
答：
- 进程是资源分配单位，线程是CPU调度单位
- 进程有独立地址空间，线程共享地址空间
- 进程切换开销大（换页表、刷新TLB），线程切换开销小（只换寄存器）
- 多线程编程共享资源方便，多进程隔离稳定

### 面试题：fork()之后父子进程区别？
- 父子进程：同一个程序，不同PID，不同地址空间（写时复制）
- 父进程打开的fd，子进程也能访问，共享文件表
*/

// ============================================================================
// 2. 进程间通信(IPC) 代码实例
// ============================================================================

// -------------------------- 管道 pipe --------------------------
/*
管道：半双工，只能在有血缘关系进程间通信
*/
int example_pipe() {
    printf("=== 管道pipe示例 ===\n");

    int pipefd[2];  // pipefd[0]读端, pipefd[1]写端
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // 子进程：写入管道
        close(pipefd[0]);  // 关闭不需要的读端
        const char *msg = "Hello from child process!";
        write(pipefd[1], msg, strlen(msg));
        close(pipefd[1]);
        exit(0);
    } else {
        // 父进程：读取管道
        close(pipefd[1]);  // 关闭不需要的写端
        char buf[100] = {0};
        ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
        printf("父进程收到: %.*s\n", (int)n, buf);
        close(pipefd[0]);
        wait(NULL);
    }

    printf("\n");
    return 0;
}

// -------------------------- 内存映射 mmap --------------------------
/*
mmap：把文件映射到进程地址空间，多个进程映射同一个文件 = 共享内存
优点：读写直接操作内存，不需要read/write，减少一次拷贝
*/
int example_mmap() {
    printf("=== mmap 共享内存示例 ===\n");

    // 打开一个文件用来映射
    int fd = open("/tmp/mmap_test", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    // 扩展文件大小
    ftruncate(fd, 4096);

    // 映射：可读可写，共享映射（修改对其他进程可见）
    char *addr = (char*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }
    close(fd);  // 映射完可以关闭fd

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程写入
        strcpy(addr, "Hello from mmap child!");
        printf("子进程写入: %s\n", addr);
        munmap(addr, 4096);
        exit(0);
    } else {
        wait(NULL);
        // 父进程读取
        printf("父进程读取共享内存: %s\n", addr);
        munmap(addr, 4096);
        unlink("/tmp/mmap_test");
    }

    printf("\n");
    return 0;
}

// ============================================================================
// 3. TCP/IP 四层模型 面试考点
// ============================================================================

/*
应用层   HTTP/HTTPS/FTP/DNS/SSH
  - 处理用户数据，封装请求响应

传输层   TCP/UDP
  - TCP：面向连接、可靠、字节流、流量控制、拥塞控制
  - UDP：无连接、不可靠、快速

网络层   IP/ICMP/路由
  - 寻址路由，把包从源送到目的地

链路层   以太网/ARP
  - 链路层封帧，物理传输

为什么要分层？
- 解耦：每层只做一件事，下层变了上层不用改
- 复用：每层可以有多种实现，互相独立
*/

// ============================================================================
// 4. TCP 三次握手 四次挥手 面试考点
// ============================================================================

/*
三次握手（建立连接）：
1. 客户端 → SYN(seq=x) → 服务器
2. 客户端 ← SYN+ACK(seq=y, ack=x+1) ← 服务器
3. 客户端 → ACK(ack=y+1) → 服务器
双方进入ESTABLISHED

为什么三次握手？
- 确认双方收发功能都正常：客户端发服务器能收，服务器发客户端能收，客户端再确认
- 防止过期重复连接：客户端延迟的SYN突然到了，三次握手能避免错误建立

四次挥手（断开连接）：
1. 主动关闭方 → FIN → 被动关闭方 → FIN-WAIT-1
2. 主动关闭方 ← ACK ← 被动关闭方 → CLOSE-WAIT
3. 主动关闭方 → 收到ACK → FIN-WAIT-2
4. 被动关闭方 → FIN → 主动关闭方 → LAST-ACK
5. 主动关闭方 → ACK → TIME-WAIT (2MSL) → CLOSED
6. 被动关闭方 ← 收到ACK → CLOSED

为什么TIME-WAIT？
1. 保证最后一个ACK能到达对方，如果对方没收得到会重发FIN，我们还能回复ACK
2. 等待2MSL让网络中旧数据包消失，防止旧连接报文干扰新连接

MSL：Maximum Segment Lifetime，报文最大存活时间，一般1-2分钟
*/

// ============================================================================
// 5. socket 网络编程 完整代码示例
// ============================================================================

// -------------------------- 服务端 --------------------------
int tcp_server_example(uint16_t port) {
    printf("=== TCP服务端示例 监听端口%d ===\n", port);

    // 1. 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return -1;
    }

    // 2. 地址重用：解决TIME_WAIT导致bind失败
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 3. bind绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定所有网卡
    addr.sin_port = htons(port);       // 主机字节序 → 网络字节序

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listenfd);
        return -1;
    }

    // 4. listen开始监听
    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        return -1;
    }

    // 5. accept阻塞等待连接
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
    printf("新客户端连接: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 6. 读取客户端数据
    char buf[1024];
    ssize_t n = read(clientfd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("服务端收到: %s\n", buf);
        // 回显
        write(clientfd, buf, n);
    }

    // 7. 关闭连接
    close(clientfd);
    close(listenfd);
    printf("\n");
    return 0;
}

// -------------------------- 客户端 --------------------------
int tcp_client_example(const char *ip, uint16_t port) {
    printf("=== TCP客户端示例 连接%s:%d ===\n", ip, port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    const char *msg = "Hello from client!";
    write(sockfd, msg, strlen(msg));

    char buf[1024];
    ssize_t n = read(sockfd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("客户端收到回显: %s\n", buf);
    }

    close(sockfd);
    printf("\n");
    return 0;
}

// ============================================================================
// 6. IO多路复用 select/poll/epoll 完整代码示例
// ============================================================================

// -------------------------- select --------------------------
/*
select特点：
- 数据结构：fd_set位图，每个bit对应一个fd
- 最大连接数：默认1024
- 每次调用都要全量遍历，用户也要遍历找出就绪fd
- 每次内核修改fd_set，用户下次要重新设置
*/
int select_example(uint16_t port) {
    printf("=== select IO多路复用示例 ===\n");

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listenfd, 10);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds);
    int maxfd = listenfd;

    printf("select 监听端口%d\n", port);
    printf("提示：用telnet localhost %d 测试连接\n\n", port);

    while (1) {
        // select会修改readfds，每次要重新拷贝
        fd_set tmp = readfds;
        int ready = select(maxfd + 1, &tmp, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(listenfd, &tmp)) {
            // 新连接
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
            printf("新连接 fd=%d\n", clientfd);
            FD_SET(clientfd, &readfds);
            if (clientfd > maxfd) maxfd = clientfd;
        }

        // 检查每个fd是否可读
        for (int fd = listenfd + 1; fd <= maxfd; fd++) {
            if (FD_ISSET(fd, &tmp)) {
                char buf[1024];
                ssize_t n = read(fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    // 断开连接
                    printf("客户端断开 fd=%d\n", fd);
                    close(fd);
                    FD_CLR(fd, &readfds);
                    continue;
                }
                buf[n] = '\0';
                printf("fd=%d 收到: %s\n", fd, buf);
                write(fd, buf, n);  // 回显
            }
        }
    }

    close(listenfd);
    return 0;
}

// -------------------------- epoll 水平触发 --------------------------
/*
epoll特点：
- 数据结构：红黑树存所有fd + 就绪链表存就绪fd
- 没有连接数限制，几万连接没问题
- 只返回就绪fd，不用全量遍历，性能好
- 水平触发LT：只要缓冲区有数据就一直触发（默认，好写）
- 边缘触发ET：只有状态变化才触发一次，需要非阻塞+读完
*/
int epoll_example_lt(uint16_t port) {
    printf("=== epoll 水平触发示例 ===\n");

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listenfd, 10);

    // 创建epoll实例
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        close(listenfd);
        return -1;
    }

    // 添加listenfd到epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;  // 水平触发
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    const int MAX_EVENTS = 10;
    struct epoll_event *events = (struct epoll_event*)malloc(
        sizeof(struct epoll_event) * MAX_EVENTS);

    printf("epoll 监听端口%d (LT水平触发)\n", port);
    printf("提示：用telnet localhost %d 测试\n\n", port);

    while (1) {
        int nready = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == listenfd) {
                // 新连接
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
                printf("新连接 fd=%d\n", clientfd);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                // 可读
                char buf[1024];
                ssize_t n = read(fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    printf("客户端断开 fd=%d\n", fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    continue;
                }
                buf[n] = '\0';
                printf("fd=%d 收到: %s\n", fd, buf);
                write(fd, buf, n);
            }
        }
    }

    free(events);
    close(epfd);
    close(listenfd);
    return 0;
}

// -------------------------- epoll 边缘触发 --------------------------
// 设置非阻塞的辅助函数
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return 0;
}

int epoll_example_et(uint16_t port) {
    printf("=== epoll 边缘触发ET示例 ===\n");

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listenfd);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listenfd, 10);

    int epfd = epoll_create1(0);
    struct epoll_event ev;
    // EPOLLET 表示边缘触发
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    const int MAX_EVENTS = 10;
    struct epoll_event *events = (struct epoll_event*)malloc(
        sizeof(struct epoll_event) * MAX_EVENTS);

    printf("epoll 监听端口%d (ET边缘触发)\n", port);
    printf("提示：所有fd都是非阻塞，必须循环读到EAGAIN\n\n");

    while (1) {
        int nready = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == listenfd) {
                while (1) {  // ET边缘触发要一直accept直到EAGAIN
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
                    if (clientfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        else break;
                    }
                    set_nonblocking(clientfd);
                    printf("新连接 fd=%d\n", clientfd);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = clientfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
                }
            } else if (events[i].events & EPOLLIN) {
                // 边缘触发：必须循环读完所有数据直到EAGAIN
                char buf[1024];
                while (1) {
                    ssize_t n = read(fd, buf, sizeof(buf) - 1);
                    if (n == 0) {
                        // EOF 断开
                        printf("客户端断开 fd=%d\n", fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        break;
                    }
                    if (n < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 读完了
                            break;
                        } else {
                            // 错误断开
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            break;
                        }
                    }
                    // 处理数据
                    buf[n] = '\0';
                    printf("fd=%d 收到: %s\n", fd, buf);
                    write(fd, buf, n);
                }
            }
        }
    }

    free(events);
    close(epfd);
    close(listenfd);
    return 0;
}

/*
select/poll/epoll 总结对比：

| 特性 | select | poll | epoll |
|------|--------|------|-------|
| 最大连接数 | ≤ 1024 | 无限制 | 无限制 |
| 查找就绪fd | 全量遍历 O(n) | 全量遍历 O(n) | 只返回就绪 O(k) + 红黑树O(log n)增删 |
| 拷贝fd到内核 | 每次都要拷 | 每次都要拷 | 只在增删时拷一次 |
| 性能 | 连接少快，连接多很慢 | 连接多还是慢 | 大并发性能好 |

epoll 是Linux高性能网络编程首选（Nginx/Redis都用epoll）
*/

// ============================================================================
// 补充：高频Linux面试题整理
// ============================================================================

/*
### 1. 虚拟内存是什么？有什么好处？
答：
操作系统给每个进程独立虚拟地址空间，每个进程以为自己占有整个内存，
实际上操作系统做虚拟地址 → 物理地址映射。

好处：
- **隔离**：每个进程地址空间独立，互不干扰
- **方便管理**：不连续物理地址可以变连续虚拟地址，内存不够可以换出到磁盘
- **加载方便**：程序从0开始编址，链接加载更简单

### 2. 进程上下文切换做了什么？
答：
1. 保存当前进程寄存器上下文到PCB
2. 切换页表，切换虚拟地址空间
3. 加载新进程寄存器上下文
4. 交给CPU执行

### 3. TCP为什么是面向字节流？UDP为什么是面向报文？
答：
- TCP：不对应用层消息做边界保留，应用层可以随便拆分合并，所以叫字节流
- UDP：保留消息边界，一个send对应一个recv，不拆分不合并，所以叫面向报文

### 4. TCP滑动窗口是什么？
答：
滑动窗口用来做流量控制，发送方维护发送窗口，接收方维护接收窗口，
窗口内的数据才能发送，接收方通过ACK告诉发送方自己还能接收多少，
这样可以流水线发送，不用停等，吞吐量更高。
滑动窗口和拥塞窗口一起决定发送速率。

### 5. 浏览器输入url回车都发生了什么？
答：
1. 浏览器检查缓存，有缓存直接用，没有就DNS解析域名得到IP
2. 浏览器和服务器TCP三次握手建立连接
3. 浏览器发送HTTP请求
4. 服务器处理请求，返回HTTP响应
5. 浏览器解析HTML，构建DOM树，加载资源，渲染页面
6. 连接关闭（如果不用keep-alive）

### 6. 脏读幻读不可重复读是什么？
- 脏读：一个事务读到了另一个事务未提交的数据，如果另一个事务回滚，读到的数据就是无效的
- 不可重复读：同一个事务内，同一行两次读取结果不一样（因为另一个事务修改提交了）
- 幻读：同一个事务内，同一范围查询两次，结果行数不一样（另一个事务插入删除了）

### 7. 什么是死锁？四个必要条件？
- 死锁：两个或多个进程互相等待对方释放资源，都无法继续
- 四个必要条件：互斥、占有并等待、非剥夺、循环等待
*/

// ============================================================================
// 主函数：运行所有示例
// ============================================================================

int main() {
    printf("Linux 后端面试知识点整理 - 带完整代码示例\n");
    printf("================================================\n\n");

    // 运行IPC示例
    example_pipe();
    example_mmap();

    // 如果要测试网络，可以解开注释运行
    // tcp_server_example(8080);
    // tcp_client_example("127.0.0.1", 8080);
    // select_example(8081);
    // epoll_example_lt(8082);
    // epoll_example_et(8083);

    printf("\n=== 所有示例执行完成 ===\n");
    printf("知识点和代码都在注释里，方便复习!\n");
    return 0;
}
