/**
 * @file linkedlist_test.c
 * @brief 单链表测试，测试所有功能：创建、插入、删除、逆置、判断环
 */

#include "linkedlist.h"

int main() {
    printf("=== 单链表(Singly Linked List)测试 ===\n\n");

    // 1. 测试尾插法创建链表
    int arr[] = {1, 2, 3, 4, 5};
    int n = sizeof(arr) / sizeof(arr[0]);
    ListNode *head = createListTailInsert(arr, n);
    printf("1. 尾插法创建链表：\n");
    printList(head);
    printf("   长度：%d\n\n", getListLength(head));

    // 2. 测试头插法
    int arr2[] = {10, 20, 30};
    ListNode *head2 = createListHeadInsert(arr2, 3);
    printf("2. 头插法创建链表（顺序和输入相反）：\n");
    printList(head2);
    printf("   长度：%d\n\n", getListLength(head2));
    destroyList(head2);

    // 3. 测试插入
    printf("3. 测试插入操作：\n");
    head = insertAtPosition(head, 0, 0);   // 插头部
    head = insertAtPosition(head, 3, 100); // 插中间
    head = insertAtPosition(head, 7, 50);  // 插尾部
    printf("   插入后：\n   ");
    printList(head);
    printf("   长度：%d\n\n", getListLength(head));

    // 4. 测试删除
    printf("4. 测试删除操作：\n");
    int deleted;
    head = deleteAtPosition(head, 0, &deleted);
    printf("   删除头部元素%d后：\n   ", deleted);
    printList(head);

    head = deleteAtPosition(head, 3, &deleted);
    printf("   删除位置3元素%d后：\n   ", deleted);
    printList(head);

    head = deleteByValue(head, 50);
    printf("   删除值为50后：\n   ");
    printList(head);
    printf("\n");

    // 5. 测试链表反转（逆置）迭代法
    printf("5. 测试链表反转（逆置）- 迭代法：\n");
    printf("   原链表：");
    printList(head);
    head = reverseList(head);
    printf("   反转后：");
    printList(head);
    printf("\n");

    // 6. 测试反转递归法
    printf("6. 测试链表反转 - 递归法：\n");
    head = reverseListRecursive(head);
    printf("   再次反转回去：");
    printList(head);
    printf("\n");

    // 7. 测试判断环 - 无环情况
    printf("7. 测试判断环：\n");
    printf("   当前无环链表，hasCycle = %d\n", hasCycle(head));

    // 8. 测试有环情况
    printf("\n8. 构造一个环测试判断：\n");
    ListNode *cycleHead = createListTailInsert(arr, n);
    printList(cycleHead);
    makeCycle(cycleHead, 2); // 尾节点连到位置2
    printf("   构造环（尾 -> 位置2）后，hasCycle = %d\n", hasCycle(cycleHead));
    ListNode *entry = detectCycleEntry(cycleHead);
    if (entry != NULL) {
        printf("   环入口位置的值：%d\n", entry->data);
    }

    // 清理
    // 注意：有环的链表不能直接destroyList，会无限循环释放
    // 这里我们不释放它，程序结束后OS会回收，测试没问题

    destroyList(head);
    printf("\n链表已销毁\n");

    return 0;
}
