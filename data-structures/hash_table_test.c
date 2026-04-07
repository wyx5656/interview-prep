/**
 * @file hash_table_test.c
 * @brief 哈希表测试程序（链地址法）
 */

#include "hash_table.h"
#include <stdio.h>

void print_array(int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (i > 0) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    printf("=== 哈希表（链地址法）测试 ===\n\n");

    HashTable ht;
    hash_init(&ht);

    printf("1. 测试插入:\n");
    int keys[] = {10, 20, 30, 40, 50, 15, 25, 100, 1, 99};
    int n = sizeof(keys) / sizeof(keys[0]);
    for (int i = 0; i < n; i++) {
        int result = hash_insert(&ht, keys[i]);
        printf("插入 %d: %s，当前大小: %d，容量: %d\n",
               keys[i], result ? "成功" : "失败",
               hash_size(&ht), hash_capacity(&ht));
    }
    printf("\n");

    hash_print(&ht);
    printf("\n");

    printf("2. 测试查找:\n");
    int search_tests[] = {20, 99, 100, 42, 1, 50};
    for (int i = 0; i < 6; i++) {
        printf("查找 %d: %s\n", search_tests[i],
               hash_search(&ht, search_tests[i]) ? "找到了" : "没找到");
    }
    printf("\n");

    printf("3. 测试删除:\n");
    int delete_tests[] = {20, 10, 100};
    for (int i = 0; i < 3; i++) {
        int result = hash_delete(&ht, delete_tests[i]);
        printf("删除 %d: %s，当前大小: %d\n",
               delete_tests[i], result ? "成功" : "失败",
               hash_size(&ht));
    }
    printf("\n");

    hash_print(&ht);
    printf("\n");

    printf("4. 测试自动扩容:\n");
    printf("插入更多元素触发扩容:\n");
    int more_keys[] = {101, 202, 303, 404, 505, 606, 707};
    for (int i = 0; i < 7; i++) {
        hash_insert(&ht, more_keys[i]);
        printf("插入 %d，大小: %d，容量: %d，负载因子: %.2f\n",
               more_keys[i], hash_size(&ht), hash_capacity(&ht),
               (double)hash_size(&ht) / hash_capacity(&ht));
    }
    printf("\n扩容后容量从 %d 变为 %d\n\n", HASH_DEFAULT_CAPACITY, hash_capacity(&ht));

    int *output = (int*)malloc(hash_size(&ht) * sizeof(int));
    int count = hash_traverse(&ht, output);
    printf("所有元素: ");
    print_array(output, count);
    printf("\n");

    free(output);
    hash_destroy(&ht);

    printf("5. 测试空哈希表:\n");
    HashTable empty;
    hash_init(&empty);
    printf("是否为空: %s\n", hash_empty(&empty) ? "是" : "否");
    printf("大小: %d，容量: %d\n", hash_size(&empty), hash_capacity(&empty));
    hash_destroy(&empty);
    printf("\n");

    printf("=== 所有哈希表测试完成 ===\n");
    return 0;
}
