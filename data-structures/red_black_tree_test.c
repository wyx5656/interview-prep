/**
 * @file red_black_tree_test.c
 * @brief 红黑树测试程序
 */

#include "red_black_tree.h"
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
    printf("=== 红黑树测试 ===\n\n");

    RedBlackTree tree;
    rbt_init(&tree);

    printf("1. 基础插入测试:\n");
    int keys[] = {10, 20, 30, 15, 25, 5, 3};
    int n = sizeof(keys) / sizeof(keys[0]);
    for (int i = 0; i < n; i++) {
        int result = rbt_insert(&tree, keys[i]);
        printf("插入 %d: %s, 节点数: %d, 验证: %s\n",
               keys[i], result ? "成功" : "失败",
               rbt_size(&tree), rbt_verify(&tree) ? "通过" : "失败");
    }
    rbt_print(&tree);
    printf("\n");

    int *output = (int*)malloc(rbt_size(&tree) * sizeof(int));
    rbt_inorder(&tree, output);
    printf("中序遍历（应该有序）: ");
    print_array(output, rbt_size(&tree));
    printf("\n");

    printf("2. 测试查找:\n");
    int tests[] = {15, 100, 3, 25, 7};
    for (int i = 0; i < 5; i++) {
        printf("查找 %d: %s\n", tests[i],
               rbt_search(&tree, tests[i]) ? "找到了" : "没找到");
    }
    printf("\n");

    printf("3. 测试最小/最大值:\n");
    int min, max;
    if (rbt_min(&tree, &min)) {
        printf("最小值: %d\n", min);
    }
    if (rbt_max(&tree, &max)) {
        printf("最大值: %d\n", max);
    }
    printf("\n");

    printf("4. 测试删除:\n");
    int delete_tests[] = {10, 20, 3, 25};
    for (int i = 0; i < 4; i++) {
        int result = rbt_delete(&tree, delete_tests[i]);
        printf("删除 %d: %s, 验证: %s\n",
               delete_tests[i], result ? "成功" : "失败",
               rbt_verify(&tree) ? "通过" : "失败");
    }
    printf("\n删除之后:\n");
    rbt_print(&tree);

    rbt_inorder(&tree, output);
    printf("删除后中序遍历: ");
    print_array(output, rbt_size(&tree));
    printf("\n");

    free(output);
    rbt_destroy(&tree);

    printf("5. 插入有序序列 1..15:\n");
    rbt_init(&tree);
    for (int i = 1; i <= 15; i++) {
        rbt_insert(&tree, i);
        // 每次插入后验证
        if (!rbt_verify(&tree)) {
            printf("插入 %d 后验证失败!\n", i);
            break;
        }
    }
    printf("插入 1..15 之后:\n");
    printf("节点数: %d, 黑色高度: %d, 验证: %s\n",
           rbt_size(&tree), rbt_black_height(&tree),
           rbt_verify(&tree) ? "通过" : "失败");
    rbt_print(&tree);

    output = (int*)malloc(rbt_size(&tree) * sizeof(int));
    rbt_inorder(&tree, output);
    printf("\n中序遍历: ");
    print_array(output, rbt_size(&tree));
    printf("\n");

    printf("6. 删除根节点并验证性质:\n");
    int root_key = tree.root->key;
    printf("删除根 (%d): %s, 验证: %s\n",
           root_key, rbt_delete(&tree, root_key) ? "成功" : "失败",
           rbt_verify(&tree) ? "通过" : "失败");

    free(output);
    rbt_destroy(&tree);

    printf("7. 测试空树:\n");
    RedBlackTree empty;
    rbt_init(&empty);
    printf("是否为空: %s\n", rbt_empty(&empty) ? "是" : "否");
    printf("节点数: %d\n", rbt_size(&empty));
    rbt_print(&empty);
    rbt_destroy(&empty);
    printf("\n");

    printf("=== 所有红黑树测试完成 ===\n");
    return 0;
}
