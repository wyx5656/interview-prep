/**
 * @file red_black_tree.h
 * @brief 红黑树 C语言实现
 *
 * 红黑树是一种自平衡二叉搜索树，满足以下五条性质：
 * 1. 每个节点要么是红色，要么是黑色
 * 2. 根节点是黑色
 * 3. 所有叶子节点（NIL哨兵）都是黑色
 * 4. 如果一个节点是红色，那么它的两个孩子都是黑色
 * 5. 从任意节点到其后代叶子节点的所有路径，包含相同数量的黑色节点
 *
 * 性质保证：树高 ≤ 2 * log2(n + 1)，所以所有操作都是 O(log n)
 *
 * 和AVL树对比：
 * - 平衡要求更宽松 → 插入/删除时旋转次数更少
 * - 对于频繁修改的树，整体性能更好
 * - 这就是为什么C++ STL的map/set使用红黑树
 *
 * 时间复杂度（所有操作）：保证 O(log n)
 * 空间复杂度：O(n)
 */

#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include <stdio.h>
#include <stdlib.h>

// 节点颜色枚举
typedef enum {
    RBT_RED,    // 红色
    RBT_BLACK   // 黑色
} RBTColor;

// 红黑树节点
typedef struct RBTNode {
    int key;                // 键值
    struct RBTNode *left;   // 左孩子
    struct RBTNode *right;  // 右孩子
    struct RBTNode *parent; // 父节点
    RBTColor color;         // 节点颜色
} RBTNode;

// 红黑树结构
typedef struct {
    RBTNode *root;   // 根节点
    RBTNode *nil;    // 哨兵NIL节点（所有叶子都指向这里）
    int size;        // 节点个数
} RedBlackTree;

/**
 * @brief 初始化红黑树
 * @param tree 树结构指针
 */
static void rbt_init(RedBlackTree *tree) {
    // 创建哨兵NIL节点
    tree->nil = (RBTNode*)malloc(sizeof(RBTNode));
    if (tree->nil == NULL) {
        perror("rbt_init 分配NIL节点失败");
        return;
    }
    tree->nil->color = RBT_BLACK;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;
    tree->nil->key = 0;

    tree->root = tree->nil;
    tree->size = 0;
}

/**
 * @brief 创建新节点
 * @param tree 树结构（需要NIL引用）
 * @param key 键值
 * @return 新节点指针
 */
static RBTNode* rbt_create_node(RedBlackTree *tree, int key) {
    RBTNode *node = (RBTNode*)malloc(sizeof(RBTNode));
    if (node == NULL) {
        perror("rbt_create_node 分配内存失败");
        return NULL;
    }
    node->key = key;
    node->left = tree->nil;
    node->right = tree->nil;
    node->parent = tree->nil;
    node->color = RBT_RED;  // 新节点总是红色
    return node;
}

/**
 * @brief 左旋
 */
static void rbt_rotate_left(RedBlackTree *tree, RBTNode *x) {
    RBTNode *y = x->right;
    x->right = y->left;

    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

/**
 * @brief 右旋
 */
static void rbt_rotate_right(RedBlackTree *tree, RBTNode *y) {
    RBTNode *x = y->left;
    y->left = x->right;

    if (x->right != tree->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

/**
 * @brief 插入后修复红黑树性质
 */
static void rbt_insert_fixup(RedBlackTree *tree, RBTNode *z) {
    while (z->parent->color == RBT_RED) {
        if (z->parent == z->parent->parent->left) {
            RBTNode *y = z->parent->parent->right;  // 叔叔节点

            // 情况1：叔叔是红色 → 只需重新染色
            if (y->color == RBT_RED) {
                z->parent->color = RBT_BLACK;
                y->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                z = z->parent->parent;
            } else {
                // 情况2：叔叔是黑色，z是右孩子
                if (z == z->parent->right) {
                    z = z->parent;
                    rbt_rotate_left(tree, z);
                }
                // 情况3：叔叔是黑色，z是左孩子
                z->parent->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                rbt_rotate_right(tree, z->parent->parent);
            }
        } else {
            // 镜像情况：父节点是右孩子
            RBTNode *y = z->parent->parent->left;  // 叔叔节点

            // 情况1：叔叔是红色
            if (y->color == RBT_RED) {
                z->parent->color = RBT_BLACK;
                y->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                z = z->parent->parent;
            } else {
                // 情况2：叔叔是黑色，z是左孩子
                if (z == z->parent->left) {
                    z = z->parent;
                    rbt_rotate_right(tree, z);
                }
                // 情况3：叔叔是黑色，z是右孩子
                z->parent->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                rbt_rotate_left(tree, z->parent->parent);
            }
        }
    }
    // 保证根总是黑色
    tree->root->color = RBT_BLACK;
}

/**
 * @brief 向红黑树插入键值
 * @param tree 树指针
 * @param key 要插入的键值
 * @return 成功返回1，重复键或分配失败返回0
 */
static int rbt_insert(RedBlackTree *tree, int key) {
    // 标准BST插入
    RBTNode *y = tree->nil;
    RBTNode *x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (key == x->key) {
            // 不允许重复键
            return 0;
        } else if (key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    RBTNode *z = rbt_create_node(tree, key);
    if (z == NULL) {
        return 0;
    }

    z->parent = y;
    if (y == tree->nil) {
        tree->root = z;
    } else if (key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    tree->size++;

    // 修复红黑性质
    rbt_insert_fixup(tree, z);
    return 1;
}

/**
 * @brief 移植 - 删除辅助函数
 */
static void rbt_transplant(RedBlackTree *tree, RBTNode *u, RBTNode *v) {
    if (u->parent == tree->nil) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

/**
 * @brief 在子树中找最小节点
 */
static RBTNode* rbt_minimum(RedBlackTree *tree, RBTNode *node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

/**
 * @brief 找节点的后继
 */
static RBTNode* rbt_successor(RedBlackTree *tree, RBTNode *x) {
    if (x->right != tree->nil) {
        return rbt_minimum(tree, x->right);
    }

    RBTNode *y = x->parent;
    while (y != tree->nil && x == y->right) {
        x = y;
        y = y->parent;
    }
    return y;
}

/**
 * @brief 删除后修复红黑树性质
 */
static void rbt_delete_fixup(RedBlackTree *tree, RBTNode *x) {
    while (x != tree->root && x->color == RBT_BLACK) {
        if (x == x->parent->left) {
            RBTNode *w = x->parent->right;  // 兄弟节点

            // 情况1：兄弟是红色
            if (w->color == RBT_RED) {
                w->color = RBT_BLACK;
                x->parent->color = RBT_RED;
                rbt_rotate_left(tree, x->parent);
                w = x->parent->right;
            }

            // 情况2：兄弟是黑色，两个孩子都是黑色
            if (w->left->color == RBT_BLACK && w->right->color == RBT_BLACK) {
                w->color = RBT_RED;
                x = x->parent;
            } else {
                // 情况3：兄弟黑色，左红右黑
                if (w->right->color == RBT_BLACK) {
                    w->left->color = RBT_BLACK;
                    w->color = RBT_RED;
                    rbt_rotate_right(tree, w);
                    w = x->parent->right;
                }

                // 情况4：兄弟黑色，右孩子红色
                w->color = x->parent->color;
                x->parent->color = RBT_BLACK;
                w->right->color = RBT_BLACK;
                rbt_rotate_left(tree, x->parent);
                x = tree->root;
            }
        } else {
            // 镜像情况：x是右孩子
            RBTNode *w = x->parent->left;  // 兄弟节点

            // 情况1：兄弟是红色
            if (w->color == RBT_RED) {
                w->color = RBT_BLACK;
                x->parent->color = RBT_RED;
                rbt_rotate_right(tree, x->parent);
                w = x->parent->left;
            }

            // 情况2：兄弟是黑色，两个孩子都是黑色
            if (w->right->color == RBT_BLACK && w->left->color == RBT_BLACK) {
                w->color = RBT_RED;
                x = x->parent;
            } else {
                // 情况3：兄弟黑色，右红左黑
                if (w->left->color == RBT_BLACK) {
                    w->right->color = RBT_BLACK;
                    w->color = RBT_RED;
                    rbt_rotate_left(tree, w);
                    w = x->parent->left;
                }

                // 情况4：兄弟黑色，左孩子红色
                w->color = x->parent->color;
                x->parent->color = RBT_BLACK;
                w->left->color = RBT_BLACK;
                rbt_rotate_right(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RBT_BLACK;
}

/**
 * @brief 从红黑树删除节点
 * @param tree 树指针
 * @param key 要删除的键值
 * @return 成功返回1，键不存在返回0
 */
static int rbt_delete(RedBlackTree *tree, int key) {
    // 查找要删除的节点
    RBTNode *z = tree->nil;
    RBTNode *x = tree->root;

    while (x != tree->nil) {
        if (key == x->key) {
            z = x;
            break;
        } else if (key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    if (z == tree->nil) {
        return 0;  // 未找到键
    }

    RBTNode *y = z;
    RBTNode *x_node;
    RBTColor y_original_color = y->color;

    if (z->left == tree->nil) {
        x_node = z->right;
        rbt_transplant(tree, z, z->right);
    } else if (z->right == tree->nil) {
        x_node = z->left;
        rbt_transplant(tree, z, z->left);
    } else {
        y = rbt_minimum(tree, z->right);
        y_original_color = y->color;
        x_node = y->right;

        if (y->parent == z) {
            x_node->parent = y;
        } else {
            rbt_transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        rbt_transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    tree->size--;

    if (y_original_color == RBT_BLACK) {
        rbt_delete_fixup(tree, x_node);
    }

    return 1;
}

/**
 * @brief 查找键值
 * @return 找到返回1，没找到返回0
 */
static int rbt_search(RedBlackTree *tree, int key) {
    RBTNode *current = tree->root;
    while (current != tree->nil) {
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
 */
static int rbt_min(RedBlackTree *tree, int *out) {
    if (tree->root == tree->nil) {
        return 0;
    }
    *out = rbt_minimum(tree, tree->root)->key;
    return 1;
}

/**
 * @brief 获取最大键值
 */
static RBTNode* rbt_maximum(RedBlackTree *tree, RBTNode *node) {
    while (node->right != tree->nil) {
        node = node->right;
    }
    return node;
}

static int rbt_max(RedBlackTree *tree, int *out) {
    if (tree->root == tree->nil) {
        return 0;
    }
    *out = rbt_maximum(tree, tree->root)->key;
    return 1;
}

/**
 * @brief 获取节点个数
 */
static int rbt_size(RedBlackTree *tree) {
    return tree->size;
}

/**
 * @brief 判断是否为空
 */
static int rbt_empty(RedBlackTree *tree) {
    return tree->size == 0;
}

/**
 * @brief 递归销毁树
 */
static void rbt_destroy_recursive(RedBlackTree *tree, RBTNode *node) {
    if (node == tree->nil) {
        return;
    }
    rbt_destroy_recursive(tree, node->left);
    rbt_destroy_recursive(tree, node->right);
    free(node);
}

/**
 * @brief 销毁整棵树释放内存
 */
static void rbt_destroy(RedBlackTree *tree) {
    rbt_destroy_recursive(tree, tree->root);
    free(tree->nil);
    tree->root = tree->nil = NULL;
    tree->size = 0;
}

/**
 * @brief 中序遍历
 */
static void rbt_inorder_recursive(RedBlackTree *tree, RBTNode *node, int *output, int *index) {
    if (node == tree->nil) {
        return;
    }
    rbt_inorder_recursive(tree, node->left, output, index);
    output[(*index)++] = node->key;
    rbt_inorder_recursive(tree, node->right, output, index);
}

static void rbt_inorder(RedBlackTree *tree, int *output) {
    int index = 0;
    rbt_inorder_recursive(tree, tree->root, output, &index);
}

/**
 * @brief 前序遍历
 */
static void rbt_preorder_recursive(RedBlackTree *tree, RBTNode *node, int *output, int *index) {
    if (node == tree->nil) {
        return;
    }
    output[(*index)++] = node->key;
    rbt_preorder_recursive(tree, node->left, output, index);
    rbt_preorder_recursive(tree, node->right, output, index);
}

static void rbt_preorder(RedBlackTree *tree, int *output) {
    int index = 0;
    rbt_preorder_recursive(tree, tree->root, output, &index);
}

// 层序遍历用队列
typedef struct {
    RBTNode **array;
    int front;
    int rear;
    int capacity;
} RBTQueue;

static RBTQueue* rbt_queue_create(int capacity) {
    RBTQueue *q = (RBTQueue*)malloc(sizeof(RBTQueue));
    if (q == NULL) return NULL;
    q->array = (RBTNode**)malloc(capacity * sizeof(RBTNode*));
    if (q->array == NULL) {
        free(q);
        return NULL;
    }
    q->front = q->rear = 0;
    q->capacity = capacity;
    return q;
}

static void rbt_queue_destroy(RBTQueue *q) {
    free(q->array);
    free(q);
}

static int rbt_queue_enqueue(RBTQueue *q, RBTNode *node) {
    if (q->rear >= q->capacity) return 0;
    q->array[q->rear++] = node;
    return 1;
}

static int rbt_queue_dequeue(RBTQueue *q, RBTNode **out) {
    if (q->front >= q->rear) return 0;
    *out = q->array[q->front++];
    return 1;
}

static int rbt_queue_empty(RBTQueue *q) {
    return q->front >= q->rear;
}

/**
 * @brief 层序遍历
 */
static int rbt_level_order(RedBlackTree *tree, int *output) {
    if (tree->root == tree->nil) return 0;

    RBTQueue *q = rbt_queue_create(tree->size);
    if (q == NULL) return 0;

    rbt_queue_enqueue(q, tree->root);
    int index = 0;

    while (!rbt_queue_empty(q)) {
        RBTNode *current;
        rbt_queue_dequeue(q, &current);
        output[index++] = current->key;

        if (current->left != tree->nil) {
            rbt_queue_enqueue(q, current->left);
        }
        if (current->right != tree->nil) {
            rbt_queue_enqueue(q, current->right);
        }
    }

    rbt_queue_destroy(q);
    return index;
}

/**
 * @brief 验证红黑性质5：所有路径黑色节点数相同
 * @return 不一致返回-1，否则返回黑色高度
 */
static int rbt_verify_black_height(RedBlackTree *tree, RBTNode *node) {
    if (node == tree->nil) {
        return 1;  // NIL是黑色
    }

    int left_bh = rbt_verify_black_height(tree, node->left);
    int right_bh = rbt_verify_black_height(tree, node->right);

    if (left_bh == -1 || right_bh == -1 || left_bh != right_bh) {
        return -1;
    }

    return left_bh + (node->color == RBT_BLACK ? 1 : 0);
}

/**
 * @brief 验证所有红黑树性质
 * @return 所有性质满足返回1，否则返回0
 */
static int rbt_verify_recursive(RedBlackTree *tree, RBTNode *node) {
    if (node == tree->nil) {
        return 1;
    }

    // 性质4：如果节点红色，孩子必须黑色
    if (node->color == RBT_RED) {
        if (node->left != tree->nil && node->left->color == RBT_RED) {
            return 0;
        }
        if (node->right != tree->nil && node->right->color == RBT_RED) {
            return 0;
        }
    }

    // 递归检查孩子
    return rbt_verify_recursive(tree, node->left) &&
           rbt_verify_recursive(tree, node->right);
}

static int rbt_verify(RedBlackTree *tree) {
    if (tree->size == 0) {
        return 1;
    }

    // 性质2：根必须黑色
    if (tree->root->color != RBT_BLACK) {
        return 0;
    }

    // 性质3：所有NIL都是黑色（构造保证成立）
    // 性质4：没有连续红色节点
    if (!rbt_verify_recursive(tree, tree->root)) {
        return 0;
    }

    // 性质5：所有路径黑色高度相同
    if (rbt_verify_black_height(tree, tree->root) == -1) {
        return 0;
    }

    return 1;
}

/**
 * @brief 获取树的黑色高度
 */
static int rbt_black_height(RedBlackTree *tree) {
    if (tree->root == tree->nil) {
        return 0;
    }
    return rbt_verify_black_height(tree, tree->root);
}

/**
 * @brief 递归打印树，显示颜色
 */
static void rbt_print_recursive(RedBlackTree *tree, RBTNode *node, int depth, char *prefix) {
    if (node == tree->nil) {
        return;
    }

    rbt_print_recursive(tree, node->right, depth + 1, "R: ");

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%s%d (%s)\n", prefix, node->key,
           node->color == RBT_RED ? "红" : "黑");

    rbt_print_recursive(tree, node->left, depth + 1, "L: ");
}

/**
 * @brief 打印红黑树
 */
static void rbt_print(RedBlackTree *tree) {
    if (tree->root == tree->nil) {
        printf("(空红黑树)\n");
        return;
    }
    printf("红黑树 (节点数: %d, 黑色高度: %d, 验证: %s)\n",
           tree->size, rbt_black_height(tree),
           rbt_verify(tree) ? "通过" : "失败");
    rbt_print_recursive(tree, tree->root, 0, "根: ");
}

#endif // RED_BLACK_TREE_H
