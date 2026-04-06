/**
 * @file linkedlist.h
 * @brief 单链表实现 - C语言版本
 *
 * 单链表（Singly Linked List）：
 * - 每个节点包含数据和指向下一个节点的指针
 * - 逻辑上相邻，物理存储不连续，不要求连续空间
 * - 插入删除O(1)只需要修改指针（找到位置后），查找O(n)
 *
 * 本实现包含：
 * - 创建节点
 * - 头插法/尾插法创建链表
 * - 指定位置插入
 * - 指定位置/值删除
 * - 链表逆置（反转）
 * - 判断链表是否有环（快慢指针法）
 * - 遍历打印
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

// 链表节点结构
typedef struct ListNode {
    int data;               // 存储的数据
    struct ListNode *next;  // 指向下一个节点的指针
} ListNode;

/**
 * @brief 创建一个新节点
 * @param value 节点数据
 * @return 返回新节点指针
 */
ListNode* createNode(int value) {
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    if (node == NULL) {
        perror("malloc failed");
        return NULL;
    }
    node->data = value;
    node->next = NULL;
    return node;
}

/**
 * @brief 头插法创建链表
 * @param arr 输入数组
 * @param n 数组长度
 * @return 返回链表头节点指针
 *
 * 头插法：每次新节点插在头部，最终顺序和输入相反
 * 优点：不需要找尾指针，操作简单
 */
ListNode* createListHeadInsert(int arr[], int n) {
    ListNode *head = NULL; // 空链表

    for (int i = 0; i < n; i++) {
        ListNode *newNode = createNode(arr[i]);
        if (newNode == NULL) return head;

        // 新节点的next指向当前head
        newNode->next = head;
        // head更新为新节点
        head = newNode;
    }

    return head;
}

/**
 * @brief 尾插法创建链表
 * @param arr 输入数组
 * @param n 数组长度
 * @return 返回链表头节点指针
 *
 * 尾插法：每次新节点插在尾部，最终顺序和输入相同
 * 需要维护一个tail指针指向当前尾部
 */
ListNode* createListTailInsert(int arr[], int n) {
    if (n <= 0) return NULL;

    // 创建第一个节点作为头和尾
    ListNode *head = createNode(arr[0]);
    if (head == NULL) return NULL;
    ListNode *tail = head;

    for (int i = 1; i < n; i++) {
        ListNode *newNode = createNode(arr[i]);
        if (newNode == NULL) return head;

        tail->next = newNode; // 新节点接在尾部
        tail = newNode;       // 更新尾指针
    }

    return head;
}

/**
 * @brief 在指定位置插入节点（位置从0开始）
 * @param head 链表头
 * @param pos 插入位置，0表示插在头部
 * @param value 插入值
 * @return 返回可能变化的头指针
 *
 * 插入步骤：
 * 1. 如果插在头部，直接修改head返回
 * 2. 否则找到pos-1位置的前驱节点，在其后插入新节点
 *
 * 时间复杂度：O(n) 需要查找前驱
 */
ListNode* insertAtPosition(ListNode *head, int pos, int value) {
    // 插在头部特殊处理
    if (pos <= 0) {
        ListNode *newNode = createNode(value);
        if (newNode == NULL) return head;
        newNode->next = head;
        return newNode; // 新节点成为新的头
    }

    // 找到第pos-1个节点（前驱）
    ListNode *p = head;
    int count = 0;
    while (p != NULL && count < pos - 1) {
        p = p->next;
        count++;
    }

    // pos超出链表长度，插入失败
    if (p == NULL) {
        printf("插入位置超出链表长度\n");
        return head;
    }

    // 在p之后插入新节点
    ListNode *newNode = createNode(value);
    if (newNode == NULL) return head;

    newNode->next = p->next;
    p->next = newNode;

    return head;
}

/**
 * @brief 删除指定位置的节点（位置从0开始）
 * @param head 链表头
 * @param pos 删除位置
 * @param deletedValue 用于保存被删除的值，可以传NULL
 * @return 返回可能变化的头指针
 *
 * 时间复杂度：O(n) 需要查找前驱
 */
ListNode* deleteAtPosition(ListNode *head, int pos, int *deletedValue) {
    if (head == NULL) {
        printf("链表为空\n");
        return NULL;
    }

    // 删除头节点特殊处理
    if (pos == 0) {
        ListNode *newHead = head->next;
        if (deletedValue != NULL) {
            *deletedValue = head->data;
        }
        free(head);
        return newHead;
    }

    // 找到前驱节点
    ListNode *p = head;
    int count = 0;
    while (p->next != NULL && count < pos - 1) {
        p = p->next;
        count++;
    }

    // pos超出范围
    if (p->next == NULL) {
        printf("删除位置超出链表长度\n");
        return head;
    }

    // 删除p->next
    ListNode *toDelete = p->next;
    if (deletedValue != NULL) {
        *deletedValue = toDelete->data;
    }
    p->next = toDelete->next;
    free(toDelete);

    return head;
}

/**
 * @brief 删除第一个值为value的节点
 * @param head 链表头
 * @param value 要删除的值
 * @return 返回可能变化的头指针
 */
ListNode* deleteByValue(ListNode *head, int value) {
    if (head == NULL) return NULL;

    // 如果头节点就是要删的
    if (head->data == value) {
        ListNode *newHead = head->next;
        free(head);
        return newHead;
    }

    // 找值为value的节点的前驱
    ListNode *p = head;
    while (p->next != NULL && p->next->data != value) {
        p = p->next;
    }

    // 没找到
    if (p->next == NULL) {
        printf("未找到值为%d的节点\n", value);
        return head;
    }

    // 删除p->next
    ListNode *toDelete = p->next;
    p->next = toDelete->next;
    free(toDelete);

    return head;
}

/**
 * @brief 反转链表（逆置）- 迭代法
 * @param head 原链表头
 * @return 新链表头
 *
 * 原理：
 * 从头开始遍历，逐步反转指针方向
 * prev记录前一个节点，curr当前，next保存下一个
 *
 * 时间复杂度：O(n)
 * 空间复杂度：O(1) 原地反转
 */
ListNode* reverseList(ListNode *head) {
    ListNode *prev = NULL;  // 前一个节点，初始为空
    ListNode *curr = head;  // 当前节点
    ListNode *next = NULL;  // 保存下一个节点

    while (curr != NULL) {
        next = curr->next;  // 保存下一个，因为接下来要改curr->next
        curr->next = prev;  // 反转指针方向
        prev = curr;        // prev前进
        curr = next;        // curr前进
    }

    // 最终prev就是新的头节点
    return prev;
}

/**
 * @brief 反转链表 - 递归法
 * @param head 当前节点
 * @return 新链表头
 *
 * 递归思路：先反转后面的链表，然后把当前节点放到最后
 */
ListNode* reverseListRecursive(ListNode *head) {
    // 空链表或只有一个节点，直接返回
    if (head == NULL || head->next == NULL) {
        return head;
    }

    // 递归反转后面的部分，newHead最终是原链表的最后一个节点，即新头
    ListNode *newHead = reverseListRecursive(head->next);

    // 反转当前节点和下一个节点的指针
    head->next->next = head;
    head->next = NULL;

    return newHead;
}

/**
 * @brief 判断链表是否有环 - Floyd快慢指针法
 * @param head 链表头
 * @return 1有环，0无环
 *
 * 原理：
 * - 快指针一次走两步，慢指针一次走一步
 * - 如果有环，快指针一定会追上慢指针（相遇）
 * - 如果无环，快指针会先走到NULL
 *
 * 时间复杂度：O(n)
 * 空间复杂度：O(1)
 *
 * 扩展：如果要找环的入口，可以相遇后，慢指针回到头，同速走，再次相遇就是入口
 */
int hasCycle(ListNode *head) {
    if (head == NULL || head->next == NULL) {
        return 0; // 空链表或只有一个节点，无环
    }

    ListNode *slow = head;
    ListNode *fast = head;

    // fast走到终点就停止，说明无环
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;         // 慢指针走一步
        fast = fast->next->next;   // 快指针走两步

        if (slow == fast) { // 相遇，说明有环
            return 1;
        }
    }

    // fast走到了终点，说明无环
    return 0;
}

/**
 * @brief 如果有环，找环的入口节点（扩展功能）
 * @param head 链表头
 * @return 环入口节点指针，无环返回NULL
 *
 * 算法：
 * 1. 快慢指针找到相遇点
 * 2. 慢指针放回head，快慢同速走，再次相遇就是入口
 * （数学证明：从头结点到入口距离 = 相遇点到入口距离）
 */
ListNode* detectCycleEntry(ListNode *head) {
    if (head == NULL || head->next == NULL) {
        return NULL;
    }

    ListNode *slow = head;
    ListNode *fast = head;
    int hasCycleFlag = 0;

    // 第一步：找相遇点
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            hasCycleFlag = 1;
            break;
        }
    }

    if (!hasCycleFlag) {
        return NULL; // 无环
    }

    // 第二步：slow回到头，同速走，再次相遇就是入口
    slow = head;
    while (slow != fast) {
        slow = slow->next;
        fast = fast->next;
    }

    return slow; // 相遇点就是入口
}

/**
 * @brief 获取链表长度
 */
int getListLength(ListNode *head) {
    int len = 0;
    ListNode *p = head;
    while (p != NULL) {
        len++;
        p = p->next;
    }
    return len;
}

/**
 * @brief 按索引查找节点
 * @return 节点指针，NULL表示不存在
 */
ListNode* getNodeAt(ListNode *head, int pos) {
    ListNode *p = head;
    int count = 0;
    while (p != NULL && count < pos) {
        p = p->next;
        count++;
    }
    return p;
}

/**
 * @brief 打印链表
 */
void printList(ListNode *head) {
    ListNode *p = head;
    printf("链表: ");
    // 为了避免有环时无限打印，限制步数
    int steps = 0;
    int maxSteps = 100;

    while (p != NULL && steps < maxSteps) {
        printf("%d -> ", p->data);
        p = p->next;
        steps++;
    }

    if (p != NULL) {
        printf("... (可能有环)\n");
    } else {
        printf("NULL\n");
    }
}

/**
 * @brief 销毁整个链表，释放内存
 */
void destroyList(ListNode *head) {
    ListNode *p;
    while (head != NULL) {
        p = head;
        head = head->next;
        free(p);
    }
}

/**
 * @brief 构造一个有环的链表，用于测试
 * @param head 原链表头
 * @param from 尾节点连到哪个位置（从0开始）
 * @return 原头，现在有环了
 */
ListNode* makeCycle(ListNode *head, int fromPos) {
    ListNode *tail = head;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    ListNode *entry = getNodeAt(head, fromPos);
    tail->next = entry; // 尾节点连到entry位置，形成环
    return head;
}

#endif // LINKEDLIST_H
