/**
 * @file avl_tree.h
 * @brief AVL树（自平衡二叉搜索树）C语言实现
 *
 * AVL树是一种自平衡二叉搜索树，对于任意节点，
 * 左右子树的高度差不超过1。
 *
 * 性质：
 * - 平衡因子 = 左子树高度 - 右子树高度
 * - 所有节点满足 |平衡因子| ≤ 1
 * - 当插入/删除破坏平衡时，通过旋转重新平衡
 * - 树高保证为 O(log n)
 *
 * 时间复杂度（所有操作）：保证 O(log n)
 * 空间复杂度：O(n)
 *
 * 实现功能：
 * - 初始化空树
 * - 销毁树并释放所有内存
 * - 查找键值
 * - 插入键值（自动重新平衡）
 * - 删除键值（自动重新平衡）
 * - 查找最小/最大键值
 * - 前序/中序遍历
 * - 层序遍历
 * - 验证AVL平衡性质
 */

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdio.h>
#include <stdlib.h>

// AVL树节点结构
typedef struct AVLNode {
    int key;                // 键值
    struct AVLNode *left;   // 左孩子
    struct AVLNode *right;  // 右孩子
    int height;             // 以该节点为根的子树高度
} AVLNode;

// AVL树结构
typedef struct {
    AVLNode *root;  // 根节点
    int size;       // 节点个数
} AVLTree;

/**
 * @brief 安全获取节点高度（处理NULL）
 * @param node 节点指针
 * @return 节点高度，NULL返回0
 */
static int avl_get_height(AVLNode *node) {
    if (node == NULL) {
        return 0;
    }
    return node->height;
}

/**
 * @brief 根据孩子更新当前节点高度
 */
static void avl_update_height(AVLNode *node) {
    int left_h = avl_get_height(node->left);
    int right_h = avl_get_height(node->right);
    node->height = (left_h > right_h ? left_h : right_h) + 1;
}

/**
 * @brief 获取节点平衡因子
 * @param node 节点指针
 * @return 平衡因子 = 左高度 - 右高度
 */
static int avl_get_balance(AVLNode *node) {
    if (node == NULL) {
        return 0;
    }
    return avl_get_height(node->left) - avl_get_height(node->right);
}

/**
 * @brief 创建新的AVL节点
 * @param key 键值
 * @return 新节点指针，分配失败返回NULL
 */
static AVLNode* avl_create_node(int key) {
    AVLNode *node = (AVLNode*)malloc(sizeof(AVLNode));
    if (node == NULL) {
        perror("avl_create_node 分配内存失败");
        return NULL;
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;  // 新节点是叶子，高度为1
    return node;
}

/**
 * @brief 右旋，解决左-左不平衡
 *
 * 旋转前:
 *        z                                y
 *       / \                            /    \
 *      y   T4      右旋操作           x      z
 *     / \          ---------->       / \    / \
 *    x   T3                          T1  T2 T3 T4
 *   / \
 *  T1 T2
 */
static AVLNode* avl_rotate_right(AVLNode *z) {
    AVLNode *y = z->left;
    AVLNode *T3 = y->right;

    // 执行旋转
    y->right = z;
    z->left = T3;

    // 更新高度
    avl_update_height(z);
    avl_update_height(y);

    // 返回新的根节点
    return y;
}

/**
 * @brief 左旋，解决右-右不平衡
 *
 * 旋转前:
 *     z                                y
 *    /  \                           /    \
 *   T1   y      左旋操作           z      x
 *       /  \    ---------->       / \    / \
 *      T2   x                    T1  T2 T3 T4
 *          / \
 *         T3 T4
 */
static AVLNode* avl_rotate_left(AVLNode *z) {
    AVLNode *y = z->right;
    AVLNode *T2 = y->left;

    // 执行旋转
    y->left = z;
    z->right = T2;

    // 更新高度
    avl_update_height(z);
    avl_update_height(y);

    // 返回新的根节点
    return y;
}

/**
 * @brief 向AVL子树插入键值，需要时重新平衡
 * @param node 子树根节点
 * @param key 要插入的键值
 * @param success 输出参数，成功设为1
 * @return 插入后的子树根节点
 */
static AVLNode* avl_insert_recursive(AVLNode *node, int key, int *success) {
    // 第一步：执行标准BST插入
    if (node == NULL) {
        *success = 1;
        return avl_create_node(key);
    }

    if (key < node->key) {
        node->left = avl_insert_recursive(node->left, key, success);
    } else if (key > node->key) {
        node->right = avl_insert_recursive(node->right, key, success);
    } else {
        // 不允许重复键值
        *success = 0;
        return node;
    }

    // 第二步：更新祖先节点高度
    avl_update_height(node);

    // 第三步：获取平衡因子，检查是否不平衡
    int balance = avl_get_balance(node);

    // 第四步：如果不平衡，分四种情况处理

    // 情况1：左-左
    // balance > 1 且 key < 左孩子key
    if (balance > 1 && key < node->left->key) {
        return avl_rotate_right(node);
    }

    // 情况2：右-右
    // balance < -1 且 key > 右孩子key
    if (balance < -1 && key > node->right->key) {
        return avl_rotate_left(node);
    }

    // 情况3：左-右
    // balance > 1 且 key > 左孩子key
    if (balance > 1 && key > node->left->key) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }

    // 情况4：右-左
    // balance < -1 且 key < 右孩子key
    if (balance < -1 && key < node->right->key) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }

    // 仍然平衡，直接返回
    return node;
}

/**
 * @brief 初始化一棵空的AVL树
 * @param tree 树结构指针
 */
static void avl_init(AVLTree *tree) {
    tree->root = NULL;
    tree->size = 0;
}

/**
 * @brief 向AVL树插入键值
 * @param tree 树指针
 * @param key 要插入的键值
 * @return 成功返回1，重复键或分配失败返回0
 */
static int avl_insert(AVLTree *tree, int key) {
    int success = 0;
    tree->root = avl_insert_recursive(tree->root, key, &success);
    if (success) {
        tree->size++;
    }
    return success;
}

/**
 * @brief 在子树中找到最小键值节点
 */
static AVLNode* avl_find_min(AVLNode *node) {
    AVLNode *current = node;
    while (current->left != NULL) {
        current = current->left;
    }
    return current;
}

/**
 * @brief 从AVL子树删除键值并重新平衡
 * @param node 子树根节点
 * @param key 要删除的键值
 * @param success 输出参数，成功设为1
 * @return 删除后的子树根节点
 */
static AVLNode* avl_delete_recursive(AVLNode *node, int key, int *success) {
    // 第一步：执行标准BST删除
    if (node == NULL) {
        *success = 0;
        return node;
    }

    if (key < node->key) {
        node->left = avl_delete_recursive(node->left, key, success);
    } else if (key > node->key) {
        node->right = avl_delete_recursive(node->right, key, success);
    } else {
        // 找到要删除的节点
        *success = 1;

        // 情况1：节点有0个或1个孩子
        if (node->left == NULL || node->right == NULL) {
            AVLNode *temp = node->left ? node->left : node->right;

            // 没有孩子
            if (temp == NULL) {
                free(node);
                return NULL;
            } else {
                // 一个孩子 → 复制孩子内容
                *node = *temp;  // 复制所有字段包括高度
                free(temp);
            }
        } else {
            // 情况2：两个孩子 → 取中序后继（右子树最小）
            AVLNode *successor = avl_find_min(node->right);

            // 复制后继键值到当前节点
            node->key = successor->key;

            // 删除后继
            node->right = avl_delete_recursive(node->right, successor->key, success);
        }
    }

    // 删除后树为空
    if (node == NULL) {
        return node;
    }

    // 第二步：更新高度
    avl_update_height(node);

    // 第三步：获取平衡因子
    int balance = avl_get_balance(node);

    // 第四步：不平衡则重新平衡（四种情况）

    // 情况1：左-左
    if (balance > 1 && avl_get_balance(node->left) >= 0) {
        return avl_rotate_right(node);
    }

    // 情况2：左-右
    if (balance > 1 && avl_get_balance(node->left) < 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }

    // 情况3：右-右
    if (balance < -1 && avl_get_balance(node->right) <= 0) {
        return avl_rotate_left(node);
    }

    // 情况4：右-左
    if (balance < -1 && avl_get_balance(node->right) > 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }

    return node;
}

/**
 * @brief 从AVL树删除键值
 * @param tree 树指针
 * @param key 要删除的键值
 * @return 成功返回1，键不存在返回0
 */
static int avl_delete(AVLTree *tree, int key) {
    int success = 0;
    tree->root = avl_delete_recursive(tree->root, key, &success);
    if (success) {
        tree->size--;
    }
    return success;
}

/**
 * @brief 在AVL树中查找键值
 * @param tree 树指针
 * @param key 要查找的键值
 * @return 找到返回1，没找到返回0
 */
static int avl_search(AVLTree *tree, int key) {
    AVLNode *current = tree->root;
    while (current != NULL) {
        if (key == current->key) {
            return 1;
        } else if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return 0;
}

/**
 * @brief 获取最小键值
 * @param tree 树指针
 * @param out 输出参数，存储最小键值
 * @return 成功返回1，空树返回0
 */
static int avl_min(AVLTree *tree, int *out) {
    if (tree->root == NULL) {
        return 0;
    }
    *out = avl_find_min(tree->root)->key;
    return 1;
}

/**
 * @brief 获取最大键值
 * @param tree 树指针
 * @param out 输出参数，存储最大键值
 * @return 成功返回1，空树返回0
 */
static AVLNode* avl_find_max(AVLNode *node) {
    AVLNode *current = node;
    while (current->right != NULL) {
        current = current->right;
    }
    return current;
}

static int avl_max(AVLTree *tree, int *out) {
    if (tree->root == NULL) {
        return 0;
    }
    *out = avl_find_max(tree->root)->key;
    return 1;
}

/**
 * @brief 获取树高度
 */
static int avl_height(AVLTree *tree) {
    return avl_get_height(tree->root);
}

/**
 * @brief 获取节点个数
 */
static int avl_size(AVLTree *tree) {
    return tree->size;
}

/**
 * @brief 判断树是否为空
 */
static int avl_empty(AVLTree *tree) {
    return tree->size == 0;
}

/**
 * @brief 递归销毁树
 */
static void avl_destroy_recursive(AVLNode *node) {
    if (node == NULL) {
        return;
    }
    avl_destroy_recursive(node->left);
    avl_destroy_recursive(node->right);
    free(node);
}

/**
 * @brief 销毁整棵树释放内存
 */
static void avl_destroy(AVLTree *tree) {
    avl_destroy_recursive(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

/**
 * @brief 前序遍历
 */
static void avl_preorder_recursive(AVLNode *node, int *output, int *index) {
    if (node == NULL) {
        return;
    }
    output[(*index)++] = node->key;
    avl_preorder_recursive(node->left, output, index);
    avl_preorder_recursive(node->right, output, index);
}

static void avl_preorder(AVLTree *tree, int *output) {
    int index = 0;
    avl_preorder_recursive(tree->root, output, &index);
}

/**
 * @brief 中序遍历（结果有序）
 */
static void avl_inorder_recursive(AVLNode *node, int *output, int *index) {
    if (node == NULL) {
        return;
    }
    avl_inorder_recursive(node->left, output, index);
    output[(*index)++] = node->key;
    avl_inorder_recursive(node->right, output, index);
}

static void avl_inorder(AVLTree *tree, int *output) {
    int index = 0;
    avl_inorder_recursive(tree->root, output, &index);
}

// 层序遍历用队列
typedef struct {
    AVLNode **array;
    int front;
    int rear;
    int capacity;
} AVLQueue;

static AVLQueue* avl_queue_create(int capacity) {
    AVLQueue *q = (AVLQueue*)malloc(sizeof(AVLQueue));
    if (q == NULL) return NULL;
    q->array = (AVLNode**)malloc(capacity * sizeof(AVLNode*));
    if (q->array == NULL) {
        free(q);
        return NULL;
    }
    q->front = q->rear = 0;
    q->capacity = capacity;
    return q;
}

static void avl_queue_destroy(AVLQueue *q) {
    free(q->array);
    free(q);
}

static int avl_queue_enqueue(AVLQueue *q, AVLNode *node) {
    if (q->rear >= q->capacity) return 0;
    q->array[q->rear++] = node;
    return 1;
}

static int avl_queue_dequeue(AVLQueue *q, AVLNode **out) {
    if (q->front >= q->rear) return 0;
    *out = q->array[q->front++];
    return 1;
}

static int avl_queue_empty(AVLQueue *q) {
    return q->front >= q->rear;
}

/**
 * @brief 层序遍历
 */
static int avl_level_order(AVLTree *tree, int *output) {
    if (tree->root == NULL) return 0;

    AVLQueue *q = avl_queue_create(tree->size);
    if (q == NULL) return 0;

    avl_queue_enqueue(q, tree->root);
    int index = 0;

    while (!avl_queue_empty(q)) {
        AVLNode *current;
        avl_queue_dequeue(q, &current);
        output[index++] = current->key;

        if (current->left != NULL) {
            avl_queue_enqueue(q, current->left);
        }
        if (current->right != NULL) {
            avl_queue_enqueue(q, current->right);
        }
    }

    avl_queue_destroy(q);
    return index;
}

/**
 * @brief 验证AVL不变量：所有节点 |平衡因子| ≤ 1
 * @param node 要验证的子树根
 * @return 满足返回1，否则返回0
 */
static int avl_verify_balance(AVLNode *node) {
    if (node == NULL) {
        return 1;
    }
    int balance = avl_get_balance(node);
    if (balance < -1 || balance > 1) {
        return 0;
    }
    return avl_verify_balance(node->left) && avl_verify_balance(node->right);
}

static int avl_verify(AVLTree *tree) {
    return avl_verify_balance(tree->root);
}

/**
 * @brief 递归打印树，显示平衡信息
 */
static void avl_print_recursive(AVLNode *node, int depth, char *prefix) {
    if (node == NULL) {
        return;
    }

    avl_print_recursive(node->right, depth + 1, "R: ");

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%s%d (bf=%d, h=%d)\n", prefix, node->key, avl_get_balance(node), node->height);

    avl_print_recursive(node->left, depth + 1, "L: ");
}

/**
 * @brief 打印AVL树，显示平衡信息
 */
static void avl_print(AVLTree *tree) {
    if (tree->root == NULL) {
        printf("(空AVL树)\n");
        return;
    }
    printf("AVL树 (节点数: %d, 高度: %d, 平衡验证: %s)\n",
           tree->size, avl_height(tree),
           avl_verify(tree) ? "通过" : "失败");
    avl_print_recursive(tree->root, 0, "根: ");
}

#endif // AVL_TREE_H
