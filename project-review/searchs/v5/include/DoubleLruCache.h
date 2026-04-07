#ifndef DOUBLE_LRU_CACHE_H
#define DOUBLE_LRU_CACHE_H

// 按照需求实现：每个线程双缓存 + 全局共享缓冲区，定期批量同步
// - 每个线程：working 缓存（处理业务查询） + sync 缓存（接收同步）
// - 全局：shared_buffer 收集所有线程的新写入
// - 定期同步：每个线程从 shared_buffer 批量拷贝到自己的 sync，然后交换 working 和 sync

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <map>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// 每个线程内的单缓存
struct ThreadLocalCache {
    // LRU 节点
    struct CacheNode {
        json value;
        typename std::list<std::string>::iterator lru_it;
    };

    std::unordered_map<std::string, CacheNode> cache_map;
    std::list<std::string> lru_list;
    size_t max_size;

    ThreadLocalCache(size_t max) : max_size(max) {}

    void evictLru();
    void moveToFront(const std::string& key);
    void insert(const std::string& key, const json& value);
    bool get(const std::string& key, json& out);
};

class DoubleLruCache {
public:
    static DoubleLruCache& getInstance();

    // 初始化：每个缓存最大容量，同步间隔（秒）
    void init(size_t max_per_cache, int sync_interval_seconds = 60);

    // 查询：当前线程 working 缓存查询，无锁
    bool get(const std::string& key, json& out_value);

    // 写入：写入全局共享缓冲区（需要加锁）
    void put(const std::string& key, const json& value);

    // 停止后台同步线程
    void stop();

    // 重置所有缓存（用于单元测试清空状态）
    void reset();

private:
    DoubleLruCache() = default;
    ~DoubleLruCache();

    // 每个线程的双缓存结构
    struct ThreadDoubleCache {
        ThreadLocalCache working;    // 工作缓存：处理业务查询
        ThreadLocalCache sync;       // 同步缓存：接收同步
        ThreadDoubleCache(size_t max) : working(max), sync(max) {}
    };

    // 后台同步线程主循环
    void backgroundSyncLoop();

    // 每个线程初始化自己的双缓存（第一次访问时懒初始化）
    ThreadDoubleCache& getThreadCache();

    // 检查并执行同步（每个线程自己调用）
    void checkSync(ThreadDoubleCache& td);

    DoubleLruCache(const DoubleLruCache&) = delete;
    DoubleLruCache& operator=(const DoubleLruCache&) = delete;

private:
    size_t max_cache_size_;
    int sync_interval_;

    std::mutex global_mutex_;                     // 保护共享缓冲区
    std::map<std::string, json> shared_buffer_;  // 全局共享缓冲区：收集所有新写入
    std::atomic<bool> running_{false};
    std::thread sync_thread_;
    std::condition_variable cv_;
    std::atomic<long long> last_sync_time_;

    // thread_local 每个线程自己的双缓存
    static thread_local std::unique_ptr<ThreadDoubleCache> thread_cache_;
};

#endif // DOUBLE_LRU_CACHE_H
