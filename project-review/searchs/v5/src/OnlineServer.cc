#include "Echoserver.h"
#include "Dictonary.h"
#include "webPageHandler.h"
#include "DoubleLruCache.h"

int main(int argc, char **argv)
{
    std::cout << "Starting search engine server..." << std::endl;

    // 加载词典和前缀索引（已经预处理好）
    std::shared_ptr<Dictonary> d = Dictonary::getInstance();
    std::cout << "Dictionary loaded" << std::endl;

    // 加载预生成的文档偏移表
    if (!webPageHandler::loadOffsetlib("/home/wyx/interview-prep/project-review/searchs/v5/data/new_offset.dat")) {
        std::cerr << "Failed to load offset file" << std::endl;
        return 1;
    }

    // 加载预生成的倒排索引
    if (!webPageHandler::loadIndex("/home/wyx/interview-prep/project-review/searchs/v5/data/index_webpages.dat")) {
        std::cerr << "Failed to load index file" << std::endl;
        return 1;
    }

    // 初始化双缓存 LRU 一级缓存：容量 1000，同步间隔 60 秒
    DoubleLruCache& cache = DoubleLruCache::getInstance();
    cache.init(1000, 60);
    std::cout << "Double LRU cache initialized" << std::endl;

    std::cout << "All indexes loaded, starting server..." << std::endl;
    Echoserver server(4, 10, "0.0.0.0", 8888);
    server.start();

    return 0;
}
