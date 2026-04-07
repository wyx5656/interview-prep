/**
 * @file avl_tree_test.c
 * @brief AVL树测试程序
 */

#include "avl_tree.h"
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
    printf("=== AVL树测试 ===\n\n");

    AVLTree tree;
    avl_init(&tree);

    printf("1. 测试需要旋转的插入:\n");

    // 测试左-左情况
    int keys1[] = {30, 20, 10};  // 应该触发LL旋转
    int n1 = sizeof(keys1) / sizeof(keys1[0]);
    for (int i = 0; i < n1; i++) {
        avl_insert(&tree, keys1[i]);
        printf("插入 %d, 节点数: %d, 高度: %d, 平衡检查: %s\n",
               keys1[i], avl_size(&tree), avl_height(&tree),
               avl_verify(&tree) ? "通过" : "失败");
    }
    avl_print(&tree);
    printf("\n");

    // 继续插入触发其他旋转情况
    int keys2[] = {40, 50};  // 应该触发RR旋转
    for (int i = 0; i < 2; i++) {
        avl_insert(&tree, keys2[i]);
        printf("插入 %d, 节点数: %d, 高度: %d, 平衡检查: %s\n",
               keys2[i], avl_size(&tree), avl_height(&tree),
               avl_verify(&tree) ? "通过" : "失败");
    }
    avl_print(&tree);
    printf("\n");

    printf("2. 测试左-右情况:\n");
    avl_init(&tree);
    int lr_keys[] = {30, 10, 20};  // 左-右不平衡
    for (int i = 0; i < 3; i++) {
        avl_insert(&tree, lr_keys[i]);
    }
    avl_print(&tree);
    printf("平衡验证: %s\n", avl_verify(&tree) ? "通过" : "失败");
    printf("\n");

    printf("3. 测试右-左情况:\n");
    avl_init(&tree);
    int rl_keys[] = {10, 30, 20};  // 右-左不平衡
    for (int i = 0; i < 3; i++) {
        avl_insert(&tree, rl_keys[i]);
    }
    avl_print(&tree);
    printf("平衡验证: %s\n", avl_verify(&tree) ? "通过" : "失败");
    printf("\n");

    printf("4. 批量插入测试（有序序列）:\n");
    avl_init(&tree);
    int sorted_keys[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int n = sizeof(sorted_keys) / sizeof(sorted_keys[0]);
    for (int i = 0; i < n; i++) {
        avl_insert(&tree, sorted_keys[i]);
    }
    printf("插入有序 1..10 之后:\n");
    printf("节点数: %d, 高度: %d\n", avl_size(&tree), avl_height(&tree));
    printf("平衡验证: %s\n", avl_verify(&tree) ? "通过" : "失败");
    avl_print(&tree);

    int *output = (int*)malloc(avl_size(&tree) * sizeof(int));
    avl_inorder(&tree, output);
    printf("\n中序遍历（应该有序）: ");
    print_array(output, avl_size(&tree));
    printf("\n");

    printf("5. 测试查找:\n");
    int search_tests[] = {5, 1, 10, 11, 0};
    for (int i = 0; i < 5; i++) {
        printf("查找 %d: %s\n", search_tests[i],
               avl_search(&tree, search_tests[i]) ? "找到了" : "没找到");
    }
    printf("\n");

    printf("6. 测试删除并重平衡:\n");
    printf("删除 5: %s, 平衡检查: %s\n",
           avl_delete(&tree, 5) ? "成功" : "失败",
           avl_verify(&tree) ? "通过" : "失败");
    printf("删除 1: %s, 平衡检查: %s\n",
           avl_delete(&tree, 1) ? "成功" : "失败",
           avl_verify(&tree) ? "通过" : "失败");
    printf("删除 10: %s, 平衡检查: %s\n",
           avl_delete(&tree, 10) ? "成功" : "失败",
           avl_verify(&tree) ? "通过" : "失败");

    avl_inorder(&tree, output);
    printf("删除后中序遍历: ");
    print_array(output, avl_size(&tree));
    printf("\n");

    printf("删除之后:\n");
    avl_print(&tree);
    printf("\n");

    free(output);
    avl_destroy(&tree);

    printf("7. 测试最小/最大值:\n");
    avl_init(&tree);
    avl_insert(&tree, 50);
    avl_insert(&tree, 30);
    avl_insert(&tree, 70);
    avl_insert(&tree, 10);
    avl_insert(&tree, 90);
    int min, max;
    if (avl_min(&tree, &min)) {
        printf("最小值: %d\n", min);
    }
    if (avl_max(&tree, &max)) {
        printf("最大值: %d\n", max);
    }
    avl_destroy(&tree);
    printf("\n");

    printf("8. 测试空树:\n");
    AVLTree empty;
    avl_init(&empty);
    printf("是否为空: %s\n", avl_empty(&empty) ? "是" : "否");
    printf("节点数: %d\n", avl_size(&empty));
    printf("高度: %d\n", avl_height(&empty));
    avl_verify(&empty);
    avl_print(&empty);
    avl_destroy(&empty);
    printf("\n");

    printf("=== 所有AVL测试完成 ===\n");
    return 0;
}
