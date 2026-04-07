// timeout_queue.h
#ifndef TIMEOUT_QUEUE_H
#define TIMEOUT_QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include "user.h"
#include "linked_list.h"
#define TIMEOUT 30
#define HASH_SIZE 1024  // 增大哈希空间减少碰撞

// 时间轮槽位节点 - 存储用户
typedef struct Node {
    user_info_t * user;
    struct Node* next;
} Node;

// 哈希映射节点 - 解决哈希碰撞，链式存储
typedef struct MapNode {
    int sockfd;         // 用户socket fd
    int timeoutIndex;   // 在时间轮中的索引
    struct MapNode* next;
} MapNode;

typedef struct {
    Node* slots[TIMEOUT];           // 时间轮槽位
    MapNode* map[HASH_SIZE];         // 哈希映射：存储(sockfd -> timeoutIndex)链表，解决碰撞
    int currentIndex;
} TimeoutQueue;

void initQueue(TimeoutQueue* queue);
void addUser(TimeoutQueue* queue, user_info_t * user);
void updateUser(TimeoutQueue* queue, user_info_t * user);
void removeUser(TimeoutQueue* queue, user_info_t * user);
void timeoutCheck(int epfd,TimeoutQueue* queue);

#endif // TIMEOUT_QUEUE_H
