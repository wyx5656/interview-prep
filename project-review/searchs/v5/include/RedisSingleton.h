#ifndef REDISSINGLETON_H
#define REDISSINGLETON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <iostream>

/**
 * 线程安全的 Redis 单例
 * 使用 thread_local 每个线程一个连接，hiredis 不是线程安全，
 * 每个线程独立连接，完全无锁，不会有并发问题
 */
class RedisSingleton {
public:
    static RedisSingleton& getInstance();
    // 提供接口访问 redisContext 和 redisReply
    redisContext* getRedisContext() { return _c; }
    redisReply* getRedisReply() { return _reply; }
private:
    redisContext *_c;
    redisReply *_reply;
    RedisSingleton(); // 构造函数 - 每个线程调用一次
    ~RedisSingleton(); // 析构函数 - 线程退出释放
    RedisSingleton(const RedisSingleton&) = delete; // 禁止拷贝构造
    RedisSingleton& operator=(const RedisSingleton&) = delete; // 禁止赋值操作
};

#endif // REDISSINGLETON_H
