#include "hashtable.h" 

// hash函数
unsigned int hash(const char *key) {  
    unsigned int hashVal = 0;  
    while (*key != '\0') {  
        hashVal = (hashVal << 5) + hashVal + *key++;  
    }  
    return hashVal % MAX_SIZE;  
}  
  
// 初始化哈希表  
void initHashTable(HashTable *ht) {  
    ht->size = 0;  
    for (int i = 0; i < MAX_SIZE; i++) {  
        strcpy(ht->table[i].key, "");  
        ht->table[i].value = EMPTY;  
    }  
}  
  
// 插入键值对
void insert(HashTable *ht, const char *key, void * value) {
    unsigned int index = hash(key);
    // 线性探测解决冲突，最多探测 MAX_SIZE 次防止死循环
    int probes = 0;
    while (ht->table[index].value != EMPTY && probes < MAX_SIZE) {
        if (strcmp(ht->table[index].key, key) == 0) {
            // 如果键已存在，更新值
            ht->table[index].value = value;
            return;
        }
        index = (index + 1) % MAX_SIZE;
        probes++;
    }
    // 如果探测完整个表都没找到空位，拒绝插入（避免死循环）
    if (probes >= MAX_SIZE) {
        fprintf(stderr, "HashTable full, cannot insert key: %s\n", key);
        return;
    }
    // 插入新键值对
    strcpy(ht->table[index].key, key);
    ht->table[index].value = value;
    ht->size++;
}  
  
// 查找值  
void * find(HashTable *ht, const char *key) {  
    unsigned int index = hash(key);  
    while (ht->table[index].value != EMPTY) {  
        if (strcmp(ht->table[index].key, key) == 0) {  
            return ht->table[index].value;  
        }  
        index = (index + 1) % MAX_SIZE;  
    }  
    return EMPTY; // 键不存在  
}  
  
// 删除键值对
void erase(HashTable *ht, const char *key) {
    unsigned int index = hash(key);
    while (ht->table[index].value != EMPTY) {
        if (strcmp(ht->table[index].key, key) == 0) {
            strcpy(ht->table[index].key, "");
            // NOTE: do NOT free value here, it's freed by deleteNode2 in linked_list
            ht->table[index].value = EMPTY;
            ht->size--;
            return;
        }
        index = (index + 1) % MAX_SIZE;
    }
}  
  
// 打印哈希表内容（仅用于调试）  
void printHashTable(HashTable *ht) {  
    printf("hashtable's content:\n");
    for (int i = 0; i < MAX_SIZE; i++) {  
        if (ht->table[i].value != EMPTY) {  
            printf("Key: %s, Value: %s\n", ht->table[i].key, (const char*)ht->table[i].value);  
        }  
    }  
    printf("\n");
}  

void destroyHashTable(HashTable *ht) {
    for(int i = 0; i < MAX_SIZE; i++) {
        if(ht->table[i].value != EMPTY) {
            strcpy(ht->table[i].key, "");  
            free(ht->table[i].value);
            ht->table[i].value = EMPTY;  
            ht->size--;  
        }
    }
}
