#include "RedisSingleton.h"
#include <thread>

RedisSingleton::RedisSingleton()
: _c(nullptr), _reply(nullptr)
{
    int port = 6379;
    const char* hostname = "127.0.0.1";
    std::cout << "redis connecting on thread " << std::this_thread::get_id() << "\n";

    _c = redisConnect(hostname, port);
    if (_c == NULL || _c->err) {
        if (_c) {
            printf("Connection error: %s\n", _c->errstr);
            redisFree(_c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }
    std::cout << "redis connected on thread " << std::this_thread::get_id() << "\n";
}

RedisSingleton::~RedisSingleton(){
    if (_reply) {
        freeReplyObject(_reply);
    }
    if (_c) {
        redisFree(_c);
    }
}

RedisSingleton& RedisSingleton::getInstance(){
    // thread_local: 每个线程一个独立实例，完全无锁并发
    static thread_local RedisSingleton instance;
    return instance;
}
