/**
 * @file seqlist_test.c
 * @brief 顺序表基本操作测试
 */

#include "seqlist.h"

int main() {
    printf("=== 顺序表(Sequential List)测试 ===\n\n");

    SeqList list;
    seqListInit(&list);
    printf("初始化后：\n");
    seqListPrint(&list);

    // 测试插入
    printf("\n=== 测试插入 ===\n");
    seqListInsert(&list, 0, 10);
    seqListInsert(&list, 1, 20);
    seqListInsert(&list, 2, 30);
    seqListInsert(&list, 1, 15); // 在位置1插入15
    printf("插入后：\n");
    seqListPrint(&list);

    // 测试扩容
    printf("\n=== 测试扩容（插入多个元素） ===\n");
    for (int i = 0; i < 10; i++) {
        seqListInsert(&list, list.size, i * 10);
    }
    printf("插入后：\n");
    seqListPrint(&list);

    // 测试按位置查找
    printf("\n=== 测试查找 ===\n");
    int pos = 2;
    printf("位置%d的元素：%d\n", pos, seqListGet(&list, pos));

    // 测试按值查找
    int value = 15;
    int findPos = seqListFind(&list, value);
    if (findPos != -1) {
        printf("值%d的位置：%d\n", value, findPos);
    } else {
        printf("未找到值%d\n", value);
    }

    // 测试删除
    printf("\n=== 测试删除 ===\n");
    int deleted;
    seqListDelete(&list, 1, &deleted);
    printf("删除位置%d（值%d）后：\n", 1, deleted);
    seqListPrint(&list);

    // 删除头尾
    seqListDelete(&list, 0, &deleted);
    printf("删除头部元素%d后：\n", deleted);
    seqListPrint(&list);

    seqListDelete(&list, list.size - 1, &deleted);
    printf("删除尾部元素%d后：\n", deleted);
    seqListPrint(&list);

    printf("\n当前元素个数：%d\n", seqListSize(&list));
    printf("是否为空：%s\n", seqListEmpty(&list) ? "是" : "否");

    // 销毁释放内存
    seqListDestroy(&list);
    printf("\n顺序表已销毁，内存已释放\n");

    return 0;
}
