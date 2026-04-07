#include "timeout_queue.h"
#include "hashtable.h"
extern ListNode * userList;
extern HashTable userTable;
extern pthread_mutex_t userTable_mutex;

void initQueue(TimeoutQueue* queue) {
    for (int i = 0; i < TIMEOUT; i++) {
        queue->slots[i] = NULL;
    }
    for (int i = 0; i < HASH_SIZE; i++) {
        queue->map[i] = NULL; // NULL indicates no entry
    }
    queue->currentIndex = 0;
    printf("超时队列初始化完成\n");
}

void addUser(TimeoutQueue* queue, user_info_t * user) {
    int timeoutIndex = (queue->currentIndex + TIMEOUT - 1) % TIMEOUT;
    // 添加到时间轮槽位
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->user = user;
    newNode->next = queue->slots[timeoutIndex];
    queue->slots[timeoutIndex] = newNode;

    // 添加到哈希映射，解决碰撞用链表
    int hash = user->sockfd % HASH_SIZE;
    if (hash < 0) hash = -hash; // 处理负数情况
    MapNode* mapNew = (MapNode*)malloc(sizeof(MapNode));
    mapNew->sockfd = user->sockfd;
    mapNew->timeoutIndex = timeoutIndex;
    mapNew->next = queue->map[hash];
    queue->map[hash] = mapNew;
}

// 从超时队列删除用户 - 用于用户正常断开连接，防止内存泄漏和野指针
void removeUser(TimeoutQueue* queue, user_info_t * user) {
    int hash = user->sockfd % HASH_SIZE;
    if (hash < 0) hash = -hash;

    // 从哈希映射中删除
    MapNode* prev_map = NULL;
    MapNode* current_map = queue->map[hash];
    while (current_map != NULL) {
        if (current_map->sockfd == user->sockfd) {
            // 找到，删除节点
            int oldIndex = current_map->timeoutIndex;
            if (prev_map == NULL) {
                queue->map[hash] = current_map->next;
            } else {
                prev_map->next = current_map->next;
            }
            free(current_map);
            break;
        }
        prev_map = current_map;
        current_map = current_map->next;
    }

    // 从时间轮槽位中删除
    int found = 0;
    for (int i = 0; i < TIMEOUT; i++) {
        Node* prev = NULL;
        Node* current = queue->slots[i];
        while (current != NULL) {
            if (current->user == user) {
                if (prev == NULL) {
                    queue->slots[i] = current->next;
                } else {
                    prev->next = current->next;
                }
                free(current);
                found = 1;
                break;
            }
            prev = current;
            current = current->next;
        }
        if (found) break;
    }
}

void updateUser(TimeoutQueue* queue, user_info_t* user) {
    // 先删除旧位置
    removeUser(queue, user);
    // 重新添加到新位置
    addUser(queue, user);
}

void timeoutCheck(int epfd, TimeoutQueue* queue) {
    int index = queue->currentIndex;
    Node* current = queue->slots[index];
    Node* next;

    while (current != NULL) {
        next = current->next;
        printf("User %d (sockfd=%d) timeout.\n", (int)current->user, current->user->sockfd);
        // 从哈希表删除
        char key[32];
        snprintf(key, sizeof(key), "%d", current->user->sockfd);
        pthread_mutex_lock(&userTable_mutex);
        erase(&userTable, key);
        pthread_mutex_unlock(&userTable_mutex);

        delEpollReadfd(epfd, current->user->sockfd);
        close(current->user->sockfd);
        deleteNode2(&userList, current->user->sockfd);

        // 也从哈希映射中删除这个用户
        int hash = current->user->sockfd % HASH_SIZE;
        if (hash < 0) hash = -hash;
        MapNode* prev_map = NULL;
        MapNode* curr_map = queue->map[hash];
        while (curr_map != NULL) {
            if (curr_map->sockfd == current->user->sockfd) {
                if (prev_map == NULL) {
                    queue->map[hash] = curr_map->next;
                } else {
                    prev_map->next = curr_map->next;
                }
                free(curr_map);
                break;
            }
            prev_map = curr_map;
            curr_map = curr_map->next;
        }

        free(current);
        current = next;
    }
    queue->slots[index] = NULL; // 清空当前槽
    queue->currentIndex = (queue->currentIndex + 1) % TIMEOUT; // 移动到下一个槽
}
