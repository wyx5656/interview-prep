#include "l8w8jwt/decode.h"
#include <func.h>

#define KEY "niubi"

/* 定义函数用于解码 JWT */
int decode_jwt(const char* jwt, char* verification_key, const char* expected_iss, const char* expected_sub) {
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;
    params.jwt = (char*)jwt;
    params.jwt_length = strlen(jwt);
    params.verification_key = (unsigned char*)verification_key;
    params.verification_key_length = strlen(verification_key);

    params.validate_iss = expected_iss;
    params.validate_sub = expected_sub;

    params.validate_exp = 1; // 验证过期时间
    params.exp_tolerance_seconds = 60; // 容忍 60 秒的过期时间

    params.validate_iat = 1; // 验证签发时间
    params.iat_tolerance_seconds = 60; // 容忍 60 秒的签发时间

    enum l8w8jwt_validation_result validation_result;
    int decode_result = l8w8jwt_decode(&params, &validation_result, NULL, NULL);

    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) {
        printf("\nJWT validation successful!\n");
        return 1; // 成功
    } else {
        printf("\nJWT validation failed!\n");
        return 0; // 失败
    }
}

// static const char KEY[] = "YoUR sUpEr S3krEt 1337 HMAC kEy HeRE";
// static const char JWT[] = "eyJhbGciOiJIUzUxMiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE1ODA5MzczMjksImV4cCI6MTU4MDkzNzkyOSwic3ViIjoiR29yZG9uIEZyZWVtYW4iLCJpc3MiOiJCbGFjayBNZXNhIiwiYXVkIjoiQWRtaW5pc3RyYXRvciJ9.7oNEgWxzs4nCtxOgiyTofP2bxZtL8dS7hgGXRPPDmwQWN1pjcwntsyK4Y5Cr9035Ro6Q16WOLiVAbj7k7TeCDA";

// int main(void)
// {
//     struct l8w8jwt_decoding_params params;
//     l8w8jwt_decoding_params_init(&params);

//     params.alg = L8W8JWT_ALG_HS512;

//     params.jwt = (char*)JWT;
//     params.jwt_length = strlen(JWT);

//     params.verification_key = (unsigned char*)KEY;
//     params.verification_key_length = strlen(KEY);

//     /* 
//      * Not providing params.validate_iss_length makes it use strlen()
//      * Only do this when using properly NUL-terminated C-strings! 
//      */
//     params.validate_iss = "wangyunxiang"; 
//     params.validate_sub = "Gordon Freeman";

//     /* Expiration validation set to false here only because the above example token is already expired! */
//     params.validate_exp = 0; 
//     params.exp_tolerance_seconds = 60;

//     params.validate_iat = 1;
//     params.iat_tolerance_seconds = 60;

//     enum l8w8jwt_validation_result validation_result;

//     int decode_result = l8w8jwt_decode(&params, &validation_result, NULL, NULL);

//     if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) 
//     {
//         printf("\n Example HS512 token validation successful! \n");
//     }
//     else
//     {
//         printf("\n Example HS512 token validation failed! \n");
//     }
    
//     /*
//      * decode_result describes whether decoding/parsing the token succeeded or failed;
//      * the output l8w8jwt_validation_result variable contains actual information about
//      * JWT signature verification status and claims validation (e.g. expiration check).
//      * 
//      * If you need the claims, pass an (ideally stack pre-allocated) array of struct l8w8jwt_claim
//      * instead of NULL,NULL into the corresponding l8w8jwt_decode() function parameters.
//      * If that array is heap-allocated, remember to free it yourself!
//      */

//     return 0;
// }