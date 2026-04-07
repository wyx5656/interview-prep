#include "l8w8jwt/encode.h"
#include <func.h>
#include "l8w8jwt/decode.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define KEY "niubi"

// 定义函数，用于生成 JWT 令牌
char* generate_jwt(const char* username) {
    char* jwt = NULL;
    size_t jwt_length;
    
    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);
    
    // 设置 JWT 参数
    params.alg = L8W8JWT_ALG_HS512;

    char* sub = (char*)malloc(strlen(username) + 1);
    if (sub == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    strcpy(sub, username);

    params.sub = sub;   // 用户名

    char* aud = (char*)malloc(strlen(username) + 1);
    if (aud == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(sub);
        return NULL;
    }
    strcpy(aud, username);

    params.aud = aud;   // 使用者填充

    params.iss = "my_service"; // 签发者
    params.iat = l8w8jwt_time(NULL);
    params.exp = l8w8jwt_time(NULL) + 600; // 10分钟后过期

    params.secret_key = (unsigned char*)KEY; // 密码作为密钥
    params.secret_key_length = strlen(params.secret_key);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);

    if (r != L8W8JWT_SUCCESS) {
        fprintf(stderr, "JWT encoding failed\n");
        free(sub);
        free(aud);
        return NULL;
    }

    // 释放临时分配的内存
    free(sub);
    free(aud);

    // 返回生成的 JWT 令牌
    return jwt;
}

/* 定义函数用于解码 JWT */
int decode_jwt(const char* jwt, const char* expected_sub) {
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;
    params.jwt = (char*)jwt;
    params.jwt_length = strlen(jwt);
    params.verification_key = (unsigned char*)KEY;
    params.verification_key_length = strlen(KEY);

    params.validate_iss = "my_service";
    
    // 分配内存并复制 expected_sub
    char* validate_sub = (char*)malloc(strlen(expected_sub) + 1);
    if (validate_sub == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }
    strcpy(validate_sub, expected_sub);
    params.validate_sub = validate_sub;

    params.validate_exp = 1; // 验证过期时间
    params.exp_tolerance_seconds = 60; // 容忍 60 秒的过期时间

    params.validate_iat = 1; // 验证签发时间
    params.iat_tolerance_seconds = 60; // 容忍 60 秒的签发时间

    enum l8w8jwt_validation_result validation_result;
    int decode_result = l8w8jwt_decode(&params, &validation_result, NULL, NULL);

    // 释放临时分配的内存
    free(validate_sub);

    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) {
        printf("\nJWT validation successful!\n");
        return 1; // 成功
    } else {
        printf("\nJWT validation failed!\n");
        return 0; // 失败
    }
}

int main() {
    const char* username = "user_example";

    // 生成 JWT 令牌
    char* token = generate_jwt(username);
    if (token) {
        printf("Generated JWT token: %s\n", token);

        // 测试解码
        int valid = decode_jwt(token, username);
        if (valid) {
            printf("JWT is valid.\n");
        } else {
            printf("JWT is not valid.\n");
        }

        l8w8jwt_free(token);
    } else {
        printf("Failed to generate JWT token.\n");
    }

    return 0;


    return 0;
}
