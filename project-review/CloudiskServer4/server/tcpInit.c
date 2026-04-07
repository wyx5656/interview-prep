#include "thread_pool.h"
int tcp(const char* ip, const char* port)
{
    int listenfd = socket(AF_INET, SOL_SOCKET, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);

    
    int res = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    if(res == -1) {
        error(1, errno, "bind");
    }

    res = listen(listenfd, 10);
    if(res == -1) {
        error(1, errno, "listen");
    }
    return listenfd;
}