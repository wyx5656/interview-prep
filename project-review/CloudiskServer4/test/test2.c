#define STR_LEN 3 //定义随机输出的字符串长度。
#include <func.h>
#define _XOPEN_SOURCE


// char *GenerateStr(char* str) {

//     int i, flag,flag2;
//     flag = rand() % strlen(str);
//     flag2 = rand() % 3;
//     switch (flag2) {
//             case 0: str[flag] = rand() % 26 + 'a'; break;
//             case 1: str[flag] = rand() % 26 + 'A'; break;
//             case 2: str[flag] = rand() % 10 + '0'; break;
//     }
//     return str;
// }

int main() {
    srand(time(NULL)); // 只初始化一次随机数生成器
    char s[31];
    char s1[4] = "j9T";
    char s2[23] = "AmFJBj8YPXD.MidcwyHna1";
    // GenerateStr(s1);
    // GenerateStr(s2);
    // printf("%s %ld\n",s1,strlen(s1));
    // printf("%s %ld\n", s2,strlen(s2));
    int flag1 = rand() % 22;
    int flag2 = rand() % 3;
    int flag3 = rand() % 3;
    int flag4 = rand() % 3;
    switch (flag3) {
        case 0: s2[flag1] = rand() % 26 + 'a'; break;
        case 1: s2[flag1] = rand() % 26 + 'A'; break;
        case 2: s2[flag1] = rand() % 10 + '0'; break;
    }
    switch (flag4) {
        case 0: s1[flag2] = rand() % 26 + 'a'; break;
        case 1: s1[flag2] = rand() % 26 + 'A'; break;
        case 2: s1[flag2] = rand() % 10 + '0'; break;
    }
    snprintf(s, sizeof(s), "$y$%s$%s$", s1, s2);
    printf("%s %ld\n", s,strlen(s));
    //snprintf(s, sizeof(s), "$y$%s$%s$", s2, s3);
    // char* test1 = "j9T";
    // char* test2 = "AmFJBdasdasj8YPXDadsdidcwyH5a1";
    // snprintf(s, sizeof(s), "$y$%s$%s$", test1, test2);
    // char* test = "$y$j9T$AmFJBdasdasj8YPXDadsdidcwyH5a1$";
    char* passwd = "1234";


    // printf("Generated salt: %s\n %ld\n", s,strlen(s));

    char *encrypted = crypt(passwd, s);
    if (encrypted == NULL) {
        perror("crypt");
        exit(EXIT_FAILURE);
    }
    printf("Encrypted password: %s \n%ld\n", encrypted,strlen(encrypted));


    // // 释放生成的字符串
    // free(s2);
    // free(s3);

    // return 0;
    return 0;
}
