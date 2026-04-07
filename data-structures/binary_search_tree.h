/**
 * @file binary_search_tree.h
 * @brief 二叉搜索树 (BST) C语言实现
 *
 * 二叉搜索树满足二叉搜索性质：对于任意节点，
 * 左子树所有节点的键值都小于该节点的键值，
 * 右子树所有节点的键值都大于该节点的键值。
 *
 * 时间复杂度：
 * - 查找：平均 O(log n)，最坏 O(n)
 * - 插入：平均 O(log n)，最坏 O(n)
 * - 删除：平均 O(log n)，最坏 O(n)
 *
 * 空间复杂度：O(n) 存储节点
 *
 * 实现功能：
 * - 初始化空树
 * - 销毁树并释放所有内存
 * - 查找指定键值
 * - 插入键值
 * - 删除键值
 * - 查找最小/最大键值
 * - 前序/中序/后序遍历
 * - 层序（广度优先）遍历
 * - 获取树高度
 * - 打印树结构
 */

#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 二叉搜索树节点结构
typedef struct BSTNode {
    int key;                // 存储的键值
    struct BSTNode *left;   // 左孩子指针
    struct BSTNode *right;  // 右孩子指针
} BSTNode;

// 二叉搜索树结构
typedef struct {
    BSTNode *root;  // 根节点指针
    int size;       // 树中节点个数
} BinarySearchTree;

/**
 * @brief 创建新的BST节点
 * @param key 要存储的键值
 * @return 新节点指针，分配失败返回NULL
 */
static BSTNode* bst_create_node(int key) {
    BSTNode *node = (BSTNode*)malloc(sizeof(BSTNode));
    if (node == NULL) {
        perror("bst_create_node 分配内存失败");
        return NULL;
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/**
 * @brief 初始化一棵空的二叉搜索树
 * @param tree 要初始化的树结构指针
 */
static void bst_init(BinarySearchTree *tree) {
    tree->root = NULL;
    tree->size = 0;
}

/**
 * @brief 递归销毁辅助函数
 */
static void bst_destroy_recursive(BSTNode *node) {
    if (node == NULL) {
        return;
    }
    bst_destroy_recursive(node->left);
    bst_destroy_recursive(node->right);
    free(node);
}

/**
 * @brief 销毁整棵树并释放所有内存
 * @param tree 要销毁的树指针
 */
static void bst_destroy(BinarySearchTree *tree) {
    bst_destroy_recursive(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

/**
 * @brief 在树中查找键值（迭代实现）
 * @param tree 树指针
 * @param key 要查找的键值
 * @return 找到返回1，没找到返回0
 */
static int bst_search(BinarySearchTree *tree, int key) {
    BSTNode *current = tree->root;
    while (current != NULL) {
        if (key == current->key) {
            return 1;  // 找到
        } else if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return 0;  // 没找到
}

/**
 * @brief 向BST插入一个键值
 * @param tree 树指针
 * @param key 要插入的键值
 * @return 成功返回1，键已存在或分配失败返回0
 *
 * 时间复杂度：O(h)，h是树的高度
 */
static int bst_insert(BinarySearchTree *tree, int key) {
    // 如果是空树，直接创建根节点
    if (tree->root == NULL) {
        tree->root = bst_create_node(key);
        if (tree->root == NULL) {
            return 0;
        }
        tree->size = 1;
        return 1;
    }

    // 找到正确的插入位置
    BSTNode *parent = NULL;
    BSTNode *current = tree->root;

    while (current != NULL) {
        if (key == current->key) {
            // 不允许重复键值
            return 0;
        }
        parent = current;
        if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    // 创建新节点并连接到父节点
    BSTNode *new_node = bst_create_node(key);
    if (new_node == NULL) {
        return 0;
    }

    if (key < parent->key) {
        parent->left = new_node;
    } else {
        parent->right = new_node;
    }

    tree->size++;
    return 1;
}

/**
 * @brief 在子树中找到最小键值的节点
 * @param root 子树根节点
 * @return 最小键值节点指针
 */
static BSTNode* bst_min_node(BSTNode *root) {
    BSTNode *current = root;
    while (current->left != NULL) {
        current = current->left;
    }
    return current;
}

/**
 * @brief 在子树中找到最大键值的节点
 * @param root 子树根节点
 * @return 最大键值节点指针
 */
static BSTNode* bst_max_node(BSTNode *root) {
    BSTNode *current = root;
    while (current->right != NULL) {
        current = current->right;
    }
    return current;
}

/**
 * @brief 获取整棵树中的最小键值
 * @param tree 树指针
 * @param out 输出参数，存储最小键值
 * @return 成功返回1，空树返回0
 */
static int bst_min(BinarySearchTree *tree, int *out) {
    if (tree->root == NULL) {
        return 0;
    }
    *out = bst_min_node(tree->root)->key;
    return 1;
}

/**
 * @brief 获取整棵树中的最大键值
 * @param tree 树指针
 * @param out 输出参数，存储最大键值
 * @return 成功返回1，空树返回0
 */
static int bst_max(BinarySearchTree *tree, int *out) {
    if (tree->root == NULL) {
        return 0;
    }
    *out = bst_max_node(tree->root)->key;
    return 1;
}

/**
 * @brief 从BST中删除键值
 * @param tree 树指针
 * @param key 要删除的键值
 * @return 成功返回1，键不存在返回0
 *
 * 处理三种情况：
 * 1. 要删除的节点有0个孩子 → 直接删除
 * 2. 要删除的节点有1个孩子 → 用孩子替换
 * 3. 要删除的节点有2个孩子 → 用中序后继替换
 */
static int bst_delete(BinarySearchTree *tree, int key) {
    if (tree->root == NULL) {
        return 0;
    }

    BSTNode *parent = NULL;
    BSTNode *current = tree->root;

    // 查找要删除的节点
    while (current != NULL && current->key != key) {
        parent = current;
        if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (current == NULL) {
        return 0;  // 未找到键值
    }

    // 情况1：节点有0个或1个孩子
    if (current->left == NULL) {
        BSTNode *child = current->right;
        if (parent == NULL) {
            // 删除根节点
            tree->root = child;
        } else if (current == parent->left) {
            parent->left = child;
        } else {
            parent->right = child;
        }
        free(current);
        tree->size--;
        return 1;
    } else if (current->right == NULL) {
        BSTNode *child = current->left;
        if (parent == NULL) {
            tree->root = child;
        } else if (current == parent->left) {
            parent->left = child;
        } else {
            parent->right = child;
        }
        free(current);
        tree->size--;
        return 1;
    }

    // 情况2：节点有两个孩子
    // 找中序后继（右子树中的最小节点）
    BSTNode *successor_parent = current;
    BSTNode *successor = current->right;
    while (successor->left != NULL) {
        successor_parent = successor;
        successor = successor->left;
    }

    // 把后继的键值复制到当前节点
    current->key = successor->key;

    // 删除后继节点
    if (successor_parent == current) {
        successor_parent->right = successor->right;
    } else {
        successor_parent->left = successor->right;
    }

    free(successor);
    tree->size--;
    return 1;
}

/**
 * @brief 递归计算树高度
 * @param node 当前节点
 * @return 子树高度
 */
static int bst_height_recursive(BSTNode *node) {
    if (node == NULL) {
        return 0;
    }
    int left_height = bst_height_recursive(node->left);
    int right_height = bst_height_recursive(node->right);
    return (left_height > right_height ? left_height : right_height) + 1;
}

/**
 * @brief 获取树的高度
 * @param tree 树指针
 * @return 树高度，空树返回0
 */
static int bst_height(BinarySearchTree *tree) {
    return bst_height_recursive(tree->root);
}

/**
 * @brief 获取节点个数
 * @param tree 树指针
 * @return 节点个数
 */
static int bst_size(BinarySearchTree *tree) {
    return tree->size;
}

/**
 * @brief 判断树是否为空
 * @param tree 树指针
 * @return 空返回1，否则返回0
 */
static int bst_empty(BinarySearchTree *tree) {
    return tree->size == 0;
}

/**
 * @brief 前序遍历递归（访问 → 左 → 右）
 * @param node 当前节点
 * @param output 存储遍历结果的数组
 * @param index 输出数组当前位置指针
 */
static void bst_preorder_recursive(BSTNode *node, int *output, int *index) {
    if (node == NULL) {
        return;
    }
    output[(*index)++] = node->key;
    bst_preorder_recursive(node->left, output, index);
    bst_preorder_recursive(node->right, output, index);
}

/**
 * @brief 中序遍历递归（左 → 访问 → 右）
 *        BST的中序遍历结果是有序的
 * @param node 当前节点
 * @param output 存储遍历结果的数组
 * @param index 输出数组当前位置指针
 */
static void bst_inorder_recursive(BSTNode *node, int *output, int *index) {
    if (node == NULL) {
        return;
    }
    bst_inorder_recursive(node->left, output, index);
    output[(*index)++] = node->key;
    bst_inorder_recursive(node->right, output, index);
}

/**
 * @brief 后序遍历递归（左 → 右 → 访问）
 * @param node 当前节点
 * @param output 存储遍历结果的数组
 * @param index 输出数组当前位置指针
 */
static void bst_postorder_recursive(BSTNode *node, int *output, int *index) {
    if (node == NULL) {
        return;
    }
    bst_postorder_recursive(node->left, output, index);
    bst_postorder_recursive(node->right, output, index);
    output[(*index)++] = node->key;
}

/**
 * @brief 执行前序遍历，结果存入输出数组
 * @param tree 树指针
 * @param output 输出数组（长度至少为节点个数）
 */
static void bst_preorder(BinarySearchTree *tree, int *output) {
    int index = 0;
    bst_preorder_recursive(tree->root, output, &index);
}

/**
 * @brief 执行中序遍历，结果存入输出数组
 * @param tree 树指针
 * @param output 输出数组（长度至少为节点个数）
 */
static void bst_inorder(BinarySearchTree *tree, int *output) {
    int index = 0;
    bst_inorder_recursive(tree->root, output, &index);
}

/**
 * @brief 执行后序遍历，结果存入输出数组
 * @param tree 树指针
 * @param output 输出数组（长度至少为节点个数）
 */
static void bst_postorder(BinarySearchTree *tree, int *output) {
    int index = 0;
    bst_postorder_recursive(tree->root, output, &index);
}

// 层序遍历用的简单队列结构
typedef struct {
    BSTNode **array;
    int front;
    int rear;
    int capacity;
} BSTQueue;

static BSTQueue* bst_queue_create(int capacity) {
    BSTQueue *queue = (BSTQueue*)malloc(sizeof(BSTQueue));
    if (queue == NULL) {
        return NULL;
    }
    queue->array = (BSTNode**)malloc(capacity * sizeof(BSTNode*));
    if (queue->array == NULL) {
        free(queue);
        return NULL;
    }
    queue->front = 0;
    queue->rear = 0;
    queue->capacity = capacity;
    return queue;
}

static void bst_queue_destroy(BSTQueue *queue) {
    free(queue->array);
    free(queue);
}

static int bst_queue_enqueue(BSTQueue *queue, BSTNode *node) {
    if (queue->rear >= queue->capacity) {
        return 0;
    }
    queue->array[queue->rear++] = node;
    return 1;
}

static int bst_queue_dequeue(BSTQueue *queue, BSTNode **out) {
    if (queue->front >= queue->rear) {
        return 0;
    }
    *out = queue->array[queue->front++];
    return 1;
}

static int bst_queue_empty(BSTQueue *queue) {
    return queue->front >= queue->rear;
}

/**
 * @brief 层序遍历（广度优先遍历）
 * @param tree 树指针
 * @param output 输出数组（长度至少为节点个数）
 * @return 遍历的节点个数
 */
static int bst_level_order(BinarySearchTree *tree, int *output) {
    if (tree->root == NULL) {
        return 0;
    }

    BSTQueue *queue = bst_queue_create(tree->size);
    if (queue == NULL) {
        return 0;
    }

    bst_queue_enqueue(queue, tree->root);
    int index = 0;

    while (!bst_queue_empty(queue)) {
        BSTNode *current;
        bst_queue_dequeue(queue, &current);
        output[index++] = current->key;

        if (current->left != NULL) {
            bst_queue_enqueue(queue, current->left);
        }
        if (current->right != NULL) {
            bst_queue_enqueue(queue, current->right);
        }
    }

    bst_queue_destroy(queue);
    return index;
}

/**
 * @brief 递归打印辅助函数
 */
static void bst_print_recursive(BSTNode *node, int depth, char *prefix) {
    if (node == NULL) {
        return;
    }

    // 先打印右子树（这样可视化更直观）
    bst_print_recursive(node->right, depth + 1, "R: ");

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%s%d\n", prefix, node->key);

    // 再打印左子树
    bst_print_recursive(node->left, depth + 1, "L: ");
}

/**
 * @brief 以可读格式打印树结构
 * @param tree 树指针
 */
static void bst_print(BinarySearchTree *tree) {
    if (tree->root == NULL) {
        printf("(空树)\n");
        return;
    }
    printf("二叉搜索树 (节点数: %d, 高度: %d):\n", tree->size, bst_height(tree));
    bst_print_recursive(tree->root, 0, "根: ");
}

#endif // BINARY_SEARCH_TREE_H
