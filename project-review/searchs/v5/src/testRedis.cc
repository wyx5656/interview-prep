#include <iostream>
#include "RedisSingleton.h"

void doTest(redisContext *c) {
    if (!c) {
        std::cerr << "No valid Redis connection available." << std::endl;
        return;
    }

    const char* command1 = "set stest1 value1";
    redisReply *reply = (redisReply *)redisCommand(c, command1);

    if (reply == nullptr) {
        std::cerr << "Execute command1 failure." << std::endl;
        return;
    }

    if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str, "OK"))) {
        std::cerr << "Failed to execute command [" << command1 << "]" << std::endl;
    } else {
        std::cout << "Succeed to execute command [" << command1 << "]" << std::endl;
    }

    freeReplyObject(reply);

    const char* command2 = "strlen stest1";
    reply = (redisReply *)redisCommand(c, command2);

    if (reply->type != REDIS_REPLY_INTEGER) {
        std::cerr << "Failed to execute command [" << command2 << "]" << std::endl;
        return;
    }

    int length = reply->integer;
    std::cout << "The length of 'stest1' is " << length << "." << std::endl;
    std::cout << "Succeed to execute command [" << command2 << "]" << std::endl;

    freeReplyObject(reply);

    const char* command3 = "get stest1";
    reply = (redisReply *)redisCommand(c, command3);

    if (reply->type != REDIS_REPLY_STRING) {
        std::cerr << "Failed to execute command [" << command3 << "]" << std::endl;
        return;
    }

    std::cout << "The value of 'stest1' is " << reply->str << std::endl;
    std::cout << "Succeed to execute command [" << command3 << "]" << std::endl;

    freeReplyObject(reply);

    const char* command4 = "get stest2";
    reply = (redisReply *)redisCommand(c, command4);

    if (reply->type != REDIS_REPLY_NIL) {
        std::cerr << "Failed to execute command [" << command4 << "]" << std::endl;
        return;
    }

    std::cout << "Succeed to execute command [" << command4 << "]" << std::endl;

    freeReplyObject(reply);
}

int main() {
    // 获取 Redis 单例对象的上下文
    redisContext *c = RedisSingleton::getContext();

    if (c) {
        doTest(c);
    }

    return 0;
}