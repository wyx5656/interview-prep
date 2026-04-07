#include "config.h"

void readConfig(const char* filename, HashTable * ht)
{
    FILE * fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("open file %s error.\n", filename);
        return;
    }

    char buff[128] = {0};
    while(fgets(buff, sizeof(buff), fp) != NULL) {
        char * strs[3] = {0};
        int cnt = 0;
        splitString(buff, "=", strs, 3, &cnt);
        /* printf("cnt: %d\n", cnt); */
        /* for(int i = 0; i < cnt; ++i) { */
        /*     printf("strs[%d]: %s\n", i, strs[i]); */
        /* } */
        /* printf("\n"); */
        // 需要至少两个token（key和value）才能插入，跳过空行或格式错误行
        if (cnt >= 2 && strs[0] != NULL && strs[1] != NULL) {
            char * value = (char*)calloc(1, strlen(strs[1]) + 1);
            strcpy(value, strs[1]);
            insert(ht, strs[0], value);
        }
        freeStrs(strs, cnt);
    }

    fclose(fp);
}
