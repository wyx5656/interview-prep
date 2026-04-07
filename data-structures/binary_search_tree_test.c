/**
 * @file binary_search_tree_test.c
 * @brief 二叉搜索树测试程序
 */

#include "binary_search_tree.h"
#include <stdio.h>

void print_array(int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    printf("=== 二叉搜索树测试 ===\n\n");

    BinarySearchTree tree;
    bst_init(&tree);

    printf("1. 测试插入:\n");
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    int n = sizeof(keys) / sizeof(keys[0]);
    for (int i = 0; i < n; i++) {
        int result = bst_insert(&tree, keys[i]);
        printf("插入 %d: %s\n", keys[i], result ? "成功" : "失败");
    }
    printf("\n");

    bst_print(&tree);
    printf("\n");

    printf("2. 测试遍历:\n");
    int *output = (int*)malloc(bst_size(&tree) * sizeof(int));

    printf("前序遍历: ");
    bst_preorder(&tree, output);
    print_array(output, bst_size(&tree));

    printf("中序遍历: ");
    bst_inorder(&tree, output);
    print_array(output, bst_size(&tree));

    printf("后序遍历: ");
    bst_postorder(&tree, output);
    print_array(output, bst_size(&tree));

    printf("层序遍历: ");
    bst_level_order(&tree, output);
    print_array(output, bst_size(&tree));
    printf("\n");

    printf("3. 测试查找:\n");
    int search_tests[] = {40, 90, 50, 10};
    for (int i = 0; i < 4; i++) {
        int found = bst_search(&tree, search_tests[i]);
        printf("查找 %d: %s\n", search_tests[i], found ? "找到了" : "没找到");
    }
    printf("\n");

    printf("4. 测试最小/最大值:\n");
    int min_val, max_val;
    if (bst_min(&tree, &min_val)) {
        printf("最小键值: %d\n", min_val);
    }
    if (bst_max(&tree, &max_val)) {
        printf("最大键值: %d\n", max_val);
    }
    printf("\n");

    printf("5. 测试节点数和高度:\n");
    printf("节点数: %d\n", bst_size(&tree));
    printf("高度: %d\n", bst_height(&tree));
    printf("是否为空: %s\n", bst_empty(&tree) ? "是" : "否");
    printf("\n");

    printf("6. 测试删除:\n");

    // 删除叶子节点
    printf("删除叶子节点 20: %s\n", bst_delete(&tree, 20) ? "成功" : "失败");
    printf("删除 20 之后:\n");
    bst_inorder(&tree, output);
    printf("中序遍历: ");
    print_array(output, bst_size(&tree));

    // 删除有一个孩子的节点
    printf("\n删除节点 30: %s\n", bst_delete(&tree, 30) ? "成功" : "失败");
    printf("删除 30 之后:\n");
    bst_inorder(&tree, output);
    printf("中序遍历: ");
    print_array(output, bst_size(&tree));

    // 删除有两个孩子的节点
    printf("\n删除有两个孩子的节点 50: %s\n", bst_delete(&tree, 50) ? "成功" : "失败");
    printf("删除 50 之后:\n");
    bst_print(&tree);
    printf("\n");

    bst_inorder(&tree, output);
    printf("最终中序遍历（应该仍然有序）: ");
    print_array(output, bst_size(&tree));
    printf("\n");

    free(output);
    bst_destroy(&tree);

    printf("7. 测试空树:\n");
    BinarySearchTree empty_tree;
    bst_init(&empty_tree);
    printf("是否为空: %s\n", bst_empty(&empty_tree) ? "是" : "否");
    printf("节点数: %d\n", bst_size(&empty_tree));
    printf("高度: %d\n", bst_height(&empty_tree));
    bst_print(&empty_tree);
    bst_destroy(&empty_tree);
    printf("\n");

    printf("=== 所有测试完成 ===\n");
    return 0;
}
