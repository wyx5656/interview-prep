/**
 * @file hash_table.h
 * @brief 哈希表（散列表）C语言实现 - 链地址法解决冲突
 *
 * 哈希表通过哈希函数将键映射到数组下标，平均 O(1) 查找。
 * 冲突解决使用链地址法（这也是STL unordered_map底层使用的方法）。
 *
 * 特点：
 * - 链地址法：每个桶是一个链表，冲突元素挂在链表上
 * - 负载因子 = 元素个数 / 桶个数
 * - 当负载因子 > 0.75 时，自动扩容为原来的2倍，减少冲突
 *
 * 时间复杂度：
 * - 查找：平均 O(1)，最坏 O(n)
 * - 插入：平均 O(1)
 * - 删除：平均 O(1)
 *
 * 空间复杂度：O(n + m)，n是元素数，m是桶数
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 默认初始桶数量
#define HASH_DEFAULT_CAPACITY 8
// 扩容负载因子阈值
#define HASH_LOAD_FACTOR_THRESHOLD 0.75

// 哈希表节点（链表节点）
typedef struct HashNode {
    int key;              // 键值
    struct HashNode *next; // 下一个节点指针
} HashNode;

// 哈希表结构
typedef struct {
    HashNode **buckets;  // 桶数组（每个桶是链表头）
    int capacity;        // 当前桶容量（桶数量）
    int size;            // 当前元素个数
} HashTable;

/**
 * @brief 简单哈希函数（针对整数key）
 * @param key 输入键
 * @param capacity 桶容量
 * @return 映射后的数组下标
 */
static int hash_int(int key, int capacity) {
    // 取模得到桶下标
    // 对于质数容量能更好分散冲突
    return abs(key) % capacity;
}

/**
 * @brief 初始化哈希表
 * @param ht 哈希表指针
 */
static void hash_init(HashTable *ht) {
    ht->capacity = HASH_DEFAULT_CAPACITY;
    ht->size = 0;
    ht->buckets = (HashNode**)calloc(ht->capacity, sizeof(HashNode*));
    if (ht->buckets == NULL) {
        perror("hash_init 分配桶数组失败");
    }
}

/**
 * @brief 计算当前负载因子
 */
static double hash_load_factor(HashTable *ht) {
    return (double)ht->size / (double)ht->capacity;
}

/**
 * @brief 扩容哈希表：容量扩大为两倍
 * @param ht 哈希表指针
 */
static void hash_resize(HashTable *ht) {
    int old_capacity = ht->capacity;
    int new_capacity = old_capacity * 2;
    HashNode **old_buckets = ht->buckets;

    // 分配新的桶数组
    HashNode **new_buckets = (HashNode**)calloc(new_capacity, sizeof(HashNode*));
    if (new_buckets == NULL) {
        perror("hash_resize 分配新桶数组失败");
        return;
    }

    // 重新哈希所有元素到新桶
    for (int i = 0; i < old_capacity; i++) {
        HashNode *node = old_buckets[i];
        while (node != NULL) {
            HashNode *next = node->next;  // 保存下一个，因为node会被移走

            // 计算新桶位置
            int new_idx = hash_int(node->key, new_capacity);

            // 头插法插入新桶
            node->next = new_buckets[new_idx];
            new_buckets[new_idx] = node;

            node = next;
        }
    }

    // 释放旧桶数组
    free(old_buckets);

    // 更新结构
    ht->buckets = new_buckets;
    ht->capacity = new_capacity;
}

/**
 * @brief 查找键是否存在
 * @param ht 哈希表指针
 * @param key 要查找的键
 * @return 找到返回1，没找到返回0
 */
static int hash_search(HashTable *ht, int key) {
    int idx = hash_int(key, ht->capacity);
    HashNode *current = ht->buckets[idx];

    while (current != NULL) {
        if (current->key == key) {
            return 1;  // 找到
        }
        current = current->next;
    }
    return 0;  // 没找到
}

/**
 * @brief 插入键到哈希表
 * @param ht 哈希表指针
 * @param key 要插入的键
 * @return 成功返回1，键已存在或分配失败返回0
 *
 * 如果负载因子超过阈值，会自动扩容。
 */
static int hash_insert(HashTable *ht, int key) {
    // 先检查是否已经存在
    if (hash_search(ht, key)) {
        return 0;  // 重复键不插入
    }

    // 检查是否需要扩容
    if (hash_load_factor(ht) > HASH_LOAD_FACTOR_THRESHOLD) {
        hash_resize(ht);
    }

    int idx = hash_int(key, ht->capacity);

    // 创建新节点
    HashNode *new_node = (HashNode*)malloc(sizeof(HashNode));
    if (new_node == NULL) {
        perror("hash_insert 分配节点失败");
        return 0;
    }
    new_node->key = key;

    // 头插法插入链表
    new_node->next = ht->buckets[idx];
    ht->buckets[idx] = new_node;

    ht->size++;
    return 1;
}

/**
 * @brief 删除键
 * @param ht 哈希表指针
 * @param key 要删除的键
 * @return 成功返回1，键不存在返回0
 */
static int hash_delete(HashTable *ht, int key) {
    int idx = hash_int(key, ht->capacity);
    HashNode *current = ht->buckets[idx];
    HashNode *prev = NULL;

    while (current != NULL) {
        if (current->key == key) {
            // 找到要删除的节点
            if (prev == NULL) {
                // 删除头节点
                ht->buckets[idx] = current->next;
            } else {
                // 删除中间节点
                prev->next = current->next;
            }
            free(current);
            ht->size--;
            return 1;
        }
        prev = current;
        current = current->next;
    }

    return 0;  // 没找到
}

/**
 * @brief 获取元素个数
 */
static int hash_size(HashTable *ht) {
    return ht->size;
}

/**
 * @brief 获取当前桶容量
 */
static int hash_capacity(HashTable *ht) {
    return ht->capacity;
}

/**
 * @brief 判断是否为空
 */
static int hash_empty(HashTable *ht) {
    return ht->size == 0;
}

/**
 * @brief 销毁哈希表释放所有内存
 * @param ht 哈希表指针
 */
static void hash_destroy(HashTable *ht) {
    // 遍历每个桶，释放链表节点
    for (int i = 0; i < ht->capacity; i++) {
        HashNode *current = ht->buckets[i];
        while (current != NULL) {
            HashNode *next = current->next;
            free(current);
            current = next;
        }
    }
    // 释放桶数组
    free(ht->buckets);
    ht->buckets = NULL;
    ht->capacity = 0;
    ht->size = 0;
}

/**
 * @brief 打印哈希表结构（用于调试）
 * @param ht 哈希表指针
 */
static void hash_print(HashTable *ht) {
    printf("哈希表: 容量=%d, 元素数=%d, 负载因子=%.2f\n",
           ht->capacity, ht->size, hash_load_factor(ht));
    for (int i = 0; i < ht->capacity; i++) {
        if (ht->buckets[i] != NULL) {
            printf("  bucket[%d]: ", i);
            HashNode *node = ht->buckets[i];
            while (node != NULL) {
                printf("%d -> ", node->key);
                node = node->next;
            }
            printf("NULL\n");
        }
    }
}

/**
 * @brief 遍历所有元素，存入输出数组
 * @param ht 哈希表指针
 * @param output 输出数组（长度至少为size）
 * @return 遍历的元素个数
 */
static int hash_traverse(HashTable *ht, int *output) {
    int count = 0;
    for (int i = 0; i < ht->capacity; i++) {
        HashNode *node = ht->buckets[i];
        while (node != NULL) {
            output[count++] = node->key;
            node = node->next;
        }
    }
    return count;
}

#endif // HASH_TABLE_H
