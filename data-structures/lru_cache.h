/**
 * @file lru_cache.h
 * @brief LRU (Least Recently Used) 最近最少使用缓存置换算法实现
 *
 * LRU缓存原理：
 * 当缓存满了的时候，删除最近最少使用的元素，添加新元素。
 *
 * 实现方法：双向链表 + 哈希表，get和put都是O(1)复杂度。
 * - 双向链表：按照访问顺序维护缓存数据，表头存最近访问，表尾存最久未访问
 * - 哈希表（字典）：根据key快速找到链表节点，O(1)查找
 *
 * 操作：
 * - get(key): 获取key对应的值，如果存在，将节点移到表头（标记为最近使用），返回值
 *          如果不存在，返回-1（表示未命中）
 * - put(key, value):
 *   - 如果key已存在：更新值，移到表头
 *   - 如果key不存在：
 *     1. 如果缓存已满，删除表尾节点（最近最少使用），从哈希表也删除
 *     2. 创建新节点，插入表头，加入哈希表
 *
 * 时间复杂度：get O(1), put O(1)
 * 空间复杂度：O(capacity)
 *
 * 常见面试问题：
 * Q: LRU的链表为什么要用双向链表？
 * A: 因为需要删除中间节点（当节点被命中移动到表头，或者缓存满删除尾节点），
 *    双向链表可以在O(1)时间删除给定节点（已知节点指针的情况下），
 *    单向链表需要O(n)找前驱节点，所以必须双向链表。
 *
 * Q: LRU和LRU-K有什么区别？
 * A: LRU只考虑最近一次访问时间，LRU-K考虑最近K次访问，命中率更高但实现更复杂。
 *
 * Q: 为什么Redis不用LRU而是用近似LRU？
 * A: 近似LRU用随机采样淘汰，不需要额外链表维护顺序，节省CPU和内存，
 *    实际命中率接近精确LRU。
 */

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <stdio.h>
#include <stdlib.h>

// LRU双向链表节点
typedef struct LRUNode {
    int key;             // 键
    int value;           // 值
    struct LRUNode *prev; // 前驱节点
    struct LRUNode *next; // 后继节点
} LRUNode;

// LRU哈希表节点（开链法解决冲突）
typedef struct LRUHashNode {
    int key;             // 键
    LRUNode *node;       // 指向双向链表中的节点
    struct LRUHashNode *next; // 哈希冲突拉链
} LRUHashNode;

// LRU缓存结构
typedef struct {
    int capacity;        // 缓存最大容量
    int size;            // 当前元素个数

    // 双向链表：虚拟头节点和虚拟尾节点（简化边界处理）
    LRUNode *head;
    LRUNode *tail;

    // 哈希表：快速根据key找节点
    LRUHashNode **hash_table;
    int hash_capacity;   // 哈希桶容量
} LRUCache;

// ========== 哈希表部分 ==========

/**
 * @brief 简单哈希函数
 */
static int lru_hash(int key, int capacity) {
    return abs(key) % capacity;
}

/**
 * @brief 在哈希表中查找key对应的节点
 * @return 找到返回LRUNode指针，没找到返回NULL
 */
static LRUNode* lru_hash_search(LRUCache *cache, int key) {
    int idx = lru_hash(key, cache->hash_capacity);
    LRUHashNode *p = cache->hash_table[idx];
    while (p != NULL) {
        if (p->key == key) {
            return p->node;
        }
        p = p->next;
    }
    return NULL;
}

/**
 * @brief 插入哈希表
 */
static void lru_hash_insert(LRUCache *cache, int key, LRUNode *node) {
    int idx = lru_hash(key, cache->hash_capacity);
    LRUHashNode *new_node = (LRUHashNode*)malloc(sizeof(LRUHashNode));
    new_node->key = key;
    new_node->node = node;
    new_node->next = cache->hash_table[idx];
    cache->hash_table[idx] = new_node;
}

/**
 * @brief 从哈希表删除key
 */
static void lru_hash_delete(LRUCache *cache, int key) {
    int idx = lru_hash(key, cache->hash_capacity);
    LRUHashNode *prev = NULL;
    LRUHashNode *p = cache->hash_table[idx];
    while (p != NULL) {
        if (p->key == key) {
            if (prev == NULL) {
                cache->hash_table[idx] = p->next;
            } else {
                prev->next = p->next;
            }
            free(p);
            return;
        }
        prev = p;
        p = p->next;
    }
}

// ========== 双向链表部分 ==========

/**
 * @brief 创建新节点
 */
static LRUNode* lru_create_node(int key, int value) {
    LRUNode *node = (LRUNode*)malloc(sizeof(LRUNode));
    node->key = key;
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

/**
 * @brief 将节点移动到表头（标记为最近使用）
 */
static void lru_move_to_head(LRUCache *cache, LRUNode *node) {
    // 先从当前位置移除
    node->prev->next = node->next;
    node->next->prev = node->prev;

    // 插入到head之后（表头位置）
    node->next = cache->head->next;
    node->prev = cache->head;
    cache->head->next->prev = node;
    cache->head->next = node;
}

/**
 * @brief 添加新节点到表头
 */
static void lru_add_to_head(LRUCache *cache, LRUNode *node) {
    node->next = cache->head->next;
    node->prev = cache->head;
    cache->head->next->prev = node;
    cache->head->next = node;
}

/**
 * @brief 删除尾节点（淘汰最近最少使用）
 * @return 返回被删除的节点，方便用户删除哈希表条目
 */
static LRUNode* lru_remove_tail(LRUCache *cache) {
    LRUNode *node = cache->tail->prev;
    node->prev->next = cache->tail;
    cache->tail->prev = node->prev;
    return node;
}

// ========== 对外接口 ==========

/**
 * @brief 初始化LRU缓存
 * @param capacity 缓存最大容量
 * @return 初始化后的LRUCache指针
 */
static LRUCache* lru_cache_create(int capacity) {
    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    if (cache == NULL) {
        perror("lru_cache_create 分配结构失败");
        return NULL;
    }

    cache->capacity = capacity;
    cache->size = 0;

    // 创建虚拟头和虚拟尾（简化边界处理，不用判断NULL）
    cache->head = lru_create_node(0, 0);
    cache->tail = lru_create_node(0, 0);
    cache->head->next = cache->tail;
    cache->tail->prev = cache->head;

    // 哈希表容量取两倍容量，减少冲突
    cache->hash_capacity = capacity * 2;
    cache->hash_table = (LRUHashNode**)calloc(cache->hash_capacity, sizeof(LRUHashNode*));
    if (cache->hash_table == NULL) {
        perror("lru_cache_create 分配哈希表失败");
        free(cache->head);
        free(cache->tail);
        free(cache);
        return NULL;
    }

    return cache;
}

/**
 * @brief 获取缓存中的值
 * @param cache LRU缓存
 * @param key 键
 * @return 命中返回值，未命中返回-1
 *
 * 如果命中，自动将节点移动到表头（标记为最近使用）
 */
static int lru_cache_get(LRUCache *cache, int key) {
    LRUNode *node = lru_hash_search(cache, key);
    if (node == NULL) {
        return -1;  // 未命中
    }
    // 命中，移动到表头标记为最近使用
    lru_move_to_head(cache, node);
    return node->value;
}

/**
 * @brief 放入/更新缓存
 * @param cache LRU缓存
 * @param key 键
 * @param value 值
 */
static void lru_cache_put(LRUCache *cache, int key, int value) {
    // 先检查是否已经存在
    LRUNode *node = lru_hash_search(cache, key);
    if (node != NULL) {
        // 已存在：更新值，移动到表头
        node->value = value;
        lru_move_to_head(cache, node);
        return;
    }

    // 不存在：需要插入新节点
    // 如果容量满了，删除尾节点（最近最少使用）
    if (cache->size >= cache->capacity) {
        LRUNode *removed = lru_remove_tail(cache);
        lru_hash_delete(cache, removed->key);
        free(removed);
        cache->size--;
    }

    // 创建新节点
    LRUNode *new_node = lru_create_node(key, value);
    lru_add_to_head(cache, new_node);
    lru_hash_insert(cache, key, new_node);
    cache->size++;
}

/**
 * @brief 获取当前元素个数
 */
static int lru_cache_size(LRUCache *cache) {
    return cache->size;
}

/**
 * @brief 获取容量
 */
static int lru_cache_capacity(LRUCache *cache) {
    return cache->capacity;
}

/**
 * @brief 销毁LRU缓存释放所有内存
 */
static void lru_cache_destroy(LRUCache *cache) {
    // 释放链表所有节点
    LRUNode *current = cache->head->next;
    while (current != cache->tail) {
        LRUNode *next = current->next;
        free(current);
        current = next;
    }
    free(cache->head);
    free(cache->tail);

    // 释放哈希表所有节点
    for (int i = 0; i < cache->hash_capacity; i++) {
        LRUHashNode *p = cache->hash_table[i];
        while (p != NULL) {
            LRUHashNode *next = p->next;
            free(p);
            p = next;
        }
    }
    free(cache->hash_table);

    free(cache);
}

/**
 * @brief 打印LRU缓存顺序（从最近使用到最少使用）
 */
static void lru_cache_print(LRUCache *cache) {
    printf("LRU Cache: 容量=%d, 大小=%d\n", cache->capacity, cache->size);
    printf("顺序 (最近使用 -> 最少使用): ");
    LRUNode *current = cache->head->next;
    while (current != cache->tail) {
        printf("(%d:%d) ", current->key, current->value);
        current = current->next;
    }
    printf("\n");
}

#endif // LRU_CACHE_H
