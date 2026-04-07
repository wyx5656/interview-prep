#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define MAX_LENGTH 1024 
void normalizePath(char *path) {
    char normalized[MAX_LENGTH];
    char *token;
    char *context;
    char *result[MAX_LENGTH];
    int depth = 0;

    token = strtok_r(path, "/", &context);
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // 跳过 "."
        } else if (strcmp(token, "..") == 0) {
            if (depth > 0) {
                depth--; // 回退到上一级目录
            }
        } else {
            result[depth++] = token;
        }
        token = strtok_r(NULL, "/", &context);
    }

    // 重构路径
    strcpy(normalized, "/");
    for (int i = 0; i < depth; i++) {
        strcat(normalized, result[i]);
        if (i < depth - 1) {
            strcat(normalized, "/");
        }
    }
    strncpy(path, normalized, MAX_LENGTH);
}

int main() {
    char path[MAX_LENGTH] = "/a/b/./c/../d/";
    normalizePath(path);
    printf("Normalized path: %s\n", path);
    return 0;
}