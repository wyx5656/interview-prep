/**
 * @file seqlist.h
 * @brief 顺序表的实现 - 动态分配容量
 *
 * 顺序表（Sequential List）：
 * - 线性表的一种，用数组存储，逻辑上相邻，物理存储位置也相邻
 * - 支持动态扩容
 * - 随机访问O(1)，插入删除O(n)
 */

#ifndef SEQLIST_H
#define SEQLIST_H

#include <stdio.h>
#include <stdlib.h>

// 顺序表初始容量
#define INIT_CAPACITY 10

// 顺序表结构定义
typedef struct {
    int *data;      // 存储数据的动态数组指针
    int size;       // 当前元素个数
    int capacity;   // 当前分配的容量
} SeqList;

/**
 * @brief 初始化顺序表
 * @param list 顺序表指针
 * @return 0成功，-1失败
 *
 * 原理：分配初始容量的空间，size设为0
 */
int seqListInit(SeqList *list) {
    list->data = (int *)malloc(INIT_CAPACITY * sizeof(int));
    if (list->data == NULL) {
        perror("malloc failed");
        return -1;
    }
    list->size = 0;
    list->capacity = INIT_CAPACITY;
    return 0;
}

/**
 * @brief 扩容：当size == capacity时，扩容为原来的2倍
 */
void seqListExpand(SeqList *list) {
    int newCapacity = list->capacity * 2;
    int *newData = (int *)realloc(list->data, newCapacity * sizeof(int));
    if (newData == NULL) {
        perror("realloc failed");
        return;
    }
    list->data = newData;
    list->capacity = newCapacity;
    printf("// 顺序表扩容：容量从 %d 扩容到 %d\n", list->capacity / 2, list->capacity);
}

/**
 * @brief 在位置pos插入元素value
 * @param pos 插入位置，从0开始索引
 * @return 0成功，-1失败
 *
 * 插入步骤：
 * 1. 检查位置是否合法，检查是否需要扩容
 * 2. 从最后一个元素开始，向后移动一位，直到pos位置
 * 3. 在pos位置插入新元素
 * 4. size加1
 *
 * 时间复杂度：O(n)，平均移动n/2个元素
 */
int seqListInsert(SeqList *list, int pos, int value) {
    // 检查位置是否合法：0 <= pos <= size
    if (pos < 0 || pos > list->size) {
        printf("插入位置不合法\n");
        return -1;
    }

    // 如果满了就扩容
    if (list->size >= list->capacity) {
        seqListExpand(list);
    }

    // 从后往前移动元素，给新元素腾出位置
    for (int i = list->size; i > pos; i--) {
        list->data[i] = list->data[i - 1];
    }

    // 插入新元素
    list->data[pos] = value;
    list->size++;
    return 0;
}

/**
 * @brief 删除位置pos的元素，并用value保存删除的值
 * @return 0成功，-1失败
 *
 * 删除步骤：
 * 1. 检查位置是否合法
 * 2. 保存要删除的元素
 * 3. 从pos+1开始，所有元素向前移动一位
 * 4. size减1
 *
 * 时间复杂度：O(n)
 */
int seqListDelete(SeqList *list, int pos, int *value) {
    if (pos < 0 || pos >= list->size) {
        printf("删除位置不合法\n");
        return -1;
    }

    *value = list->data[pos];

    // 从pos+1开始，都向前挪一位，覆盖pos位置
    for (int i = pos; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }

    list->size--;
    return 0;
}

/**
 * @brief 按位置查找元素，O(1)随机访问
 * @return 元素值，失败返回-1表示错误
 */
int seqListGet(SeqList *list, int pos) {
    if (pos < 0 || pos >= list->size) {
        printf("查找位置不合法\n");
        return -1;
    }
    return list->data[pos];
}

/**
 * @brief 查找值为value的元素，返回其位置
 * @return 索引，-1表示没找到
 *
 * 时间复杂度：O(n) 顺序查找
 */
int seqListFind(SeqList *list, int value) {
    for (int i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return i;
        }
    }
    return -1; // 没找到
}

/**
 * @brief 返回当前元素个数
 */
int seqListSize(SeqList *list) {
    return list->size;
}

/**
 * @brief 判断是否为空
 */
int seqListEmpty(SeqList *list) {
    return list->size == 0;
}

/**
 * @brief 打印顺序表
 */
void seqListPrint(SeqList *list) {
    printf("顺序表[%d/%d]: ", list->size, list->capacity);
    for (int i = 0; i < list->size; i++) {
        printf("%d ", list->data[i]);
    }
    printf("\n");
}

/**
 * @brief 销毁顺序表，释放内存
 */
void seqListDestroy(SeqList *list) {
    free(list->data);
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
}

#endif // SEQLIST_H
