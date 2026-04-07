/**
 * 双缓存 LRU 单元测试
 * 测试缓存get/put、LRU淘汰、并发访问
 */

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cassert>
#include "DoubleLruCache.h"

using namespace std;
using json = nlohmann::json;

void test_basic_get_put() {
    DoubleLruCache& cache = DoubleLruCache::getInstance();
    cache.init(5, 60);

    json value;
    // 空缓存get应该返回false
    assert(cache.get("test", value) == false);
    cout << "✓ Test 1 passed: get on empty cache returns false" << endl;

    // put然后get
    json testValue = {{"key", "value"}};
    cache.put("test", testValue);
    // 强制同步
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(cache.get("test", value) == true);
    assert(value["key"] == "value");
    cout << "✓ Test 2 passed: put and get works" << endl;

    cache.stop();
    cache.reset();
    cout << endl << "Basic get put tests passed! 🎉" << endl;
}

void test_lru_eviction() {
    DoubleLruCache& cache = DoubleLruCache::getInstance();
    cache.init(3, 60); // 容量3

    json v;
    cache.put("a", json("a"));
    cache.put("b", json("b"));
    cache.put("c", json("c"));

    // 访问a，a变成最近使用
    cache.get("a", v);

    // 再put d，应该淘汰最久未使用的b
    cache.put("d", json("d"));

    // 等待同步
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // a应该还在，b应该被淘汰
    assert(cache.get("a", v) == true);
    assert(cache.get("b", v) == false);
    assert(cache.get("c", v) == true);
    assert(cache.get("d", v) == true);

    cout << "✓ Test 3 passed: LRU eviction works correctly" << endl;

    cache.stop();
    cache.reset();
    cout << endl << "LRU eviction tests passed! 🎉" << endl;
}

void test_concurrent_access() {
    DoubleLruCache& cache = DoubleLruCache::getInstance();
    cache.init(100, 60);

    const int num_threads = 8;
    const int num_ops = 1000;

    auto worker = [&cache, num_ops](int id) {
        for (int i = 0; i < num_ops; ++i) {
            std::string key = "key_" + std::to_string(id) + "_" + std::to_string(i);
            json value = {{"id", id}, {"i", i}};
            cache.put(key, value);
            json result;
            cache.get(key, result);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    cout << "✓ Test 4 passed: " << num_threads << " threads concurrent access completed without errors" << endl;

    cache.stop();
    cache.reset();
    cout << endl << "Concurrent access tests passed! 🎉" << endl;
}

int main() {
    cout << "Running DoubleLruCache tests..." << endl << endl;

    test_basic_get_put();
    cout << endl;

    test_lru_eviction();
    cout << endl;

    test_concurrent_access();
    cout << endl;

    cout << "All DoubleLruCache tests passed! 🎉" << endl;
    return 0;
}
