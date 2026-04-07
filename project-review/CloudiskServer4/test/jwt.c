#include "l8w8jwt/encode.h"
#include <func.h>
#define KEY "niubi"
// 定义函数，用于生成 JWT 令牌
char* generate_jwt(const char* username, const char* password) {
    char* jwt = NULL;
    size_t jwt_length;
    
    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);
    
    // 设置 JWT 参数
    params.alg = L8W8JWT_ALG_HS512;
    params.sub = username;   // 用户名
    params.iss = "my_service"; // 签发者
    params.aud = username;   // 使用者填充

    params.iat = l8w8jwt_time(NULL);
    params.exp = l8w8jwt_time(NULL) + 600; // 10分钟后过期

    params.secret_key = (unsigned char*)password; // 密码作为密钥
    params.secret_key_length = strlen(params.secret_key);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);

    if (r != L8W8JWT_SUCCESS) {
        fprintf(stderr, "JWT encoding failed\n");
        return NULL;
    }

    // 返回生成的 JWT 令牌
    return jwt;
}