/**
 * @file lru_cache_test.c
 * @brief LRU缓存测试程序
 */

#include "lru_cache.h"
#include <stdio.h>

int main() {
    printf("=== LRU缓存测试 ===\n\n");

    // 创建容量为3的LRU缓存
    LRUCache *cache = lru_cache_create(3);
    printf("创建LRU缓存，容量 = %d\n\n", lru_cache_capacity(cache));

    printf("1. 测试插入:\n");
    lru_cache_put(cache, 1, 100);
    lru_cache_put(cache, 2, 200);
    lru_cache_put(cache, 3, 300);
    lru_cache_print(cache);
    printf("\n");

    printf("2. 测试查找命中:\n");
    printf("get key=2: %d\n", lru_cache_get(cache, 2));
    printf("访问key=2之后（2变最近使用）:\n");
    lru_cache_print(cache);
    printf("\n");

    printf("3. 测试插入触发淘汰:\n");
    printf("插入key=4, value=400，容量已满，淘汰最少使用的key=1:\n");
    lru_cache_put(cache, 4, 400);
    lru_cache_print(cache);
    printf("\n");

    printf("验证key=1是否被淘汰: get(1) = %d\n", lru_cache_get(cache, 1));
    printf("验证key=3仍在缓存: get(3) = %d\n", lru_cache_get(cache, 3));
    printf("\n");

    printf("淘汰后当前状态:\n");
    lru_cache_print(cache);
    printf("\n");

    printf("4. 测试更新已有key:\n");
    printf("put key=3, value=3000:\n");
    lru_cache_put(cache, 3, 3000);
    lru_cache_print(cache);
    printf("\n");

    printf("5. 连续访问测试:\n");
    for (int i = 5; i <= 7; i++) {
        lru_cache_put(cache, i, i * 100);
        printf("插入%d后: ", i);
        lru_cache_print(cache);
    }
    printf("\n");

    printf("6. 小容量测试:\n");
    LRUCache *small = lru_cache_create(1);
    lru_cache_put(small, 1, 100);
    lru_cache_put(small, 2, 200);
    printf("容量1，插入两个元素后:\n");
    lru_cache_print(small);
    printf("get(2) = %d\n", lru_cache_get(small, 2));
    lru_cache_destroy(small);
    printf("\n");

    lru_cache_destroy(cache);
    printf("=== 所有LRU测试完成 ===\n");
    return 0;
}
