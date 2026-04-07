#include "HTTPRequestHandler.h"
#include "Echoserver.h"
#include "DoubleLruCache.h"

// 静态成员定义
bool HTTPRequestHandler::_stopwords_loaded = false;
std::set<std::string> HTTPRequestHandler::_chinese_stopwords;
std::set<std::string> HTTPRequestHandler::_english_stopwords;

HTTPRequestHandler::HTTPRequestHandler() {
    // 预加载停用词到内存，只加载一次
    if (!_stopwords_loaded) {
        // 加载中文停用词
        std::ifstream fileZh("/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_zh.txt");
        if (fileZh.is_open()) {
            std::string line;
            while (std::getline(fileZh, line)) {
                // 去掉空行
                if (!line.empty()) {
                    _chinese_stopwords.insert(line);
                }
            }
            fileZh.close();
        } else {
            std::cerr << "Warning: cannot open Chinese stopwords file" << std::endl;
        }

        // 加载英文停用词
        std::ifstream fileEn("/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_eng.txt");
        if (fileEn.is_open()) {
            std::string line;
            while (std::getline(fileEn, line)) {
                if (!line.empty()) {
                    _english_stopwords.insert(line);
                }
            }
            fileEn.close();
        } else {
            std::cerr << "Warning: cannot open English stopwords file" << std::endl;
        }

        _stopwords_loaded = true;
    }
}

HTTPResponse HTTPRequestHandler::handleRequest(const HTTPRequest& request) {
    // 检查请求方法是否合法
    if (request.method != "GET" && request.method != "POST") {
        return HTTPResponse(400, "Bad Request", json{{"error", "Unsupported method"}});
    }

    // 检查 URL 是否合法
    if (request.url != "/search" && request.url != "/suggest") {
        return HTTPResponse(404, "Not Found", json{{"error", "Invalid URL"}});
    }

    // GET 请求
    if (request.method == "GET") {
        return HTTPResponse(200, "OK", json{{"response", "No content"}});
    }

    // POST 请求
    if (request.method == "POST") {
        // 解析请求体
        json requestBody = parseRequestBody(request.body);
        if(request.url == "/suggest"){
            return suggestTask(requestBody);
        }else if(request.url == "/search"){
            return searchTask(requestBody);
        }
        else{
            return HTTPResponse(200, "OK", requestBody);
        }
        // 使用解析好的 JSON 构造响应体
    }

    // 如果到达这里，则返回错误响应
    return HTTPResponse(500, "Internal Server Error", json{{"error", "Unknown error"}});
}

json HTTPRequestHandler::parseRequestBody(const std::string& requestBody) {
    try {
        return json::parse(requestBody);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing request body: " << e.what() << std::endl;
        return json{{"error", "Invalid JSON format"}};
    }
}

HTTPResponse HTTPRequestHandler::suggestTask(json requestBody){
    //1、解析json 里面data: 对应的value是要用的数据部分，分字
    if (requestBody.contains("data")) {
        auto data = requestBody["data"];
        std::string inputString = data.get<std::string>();
        std::cout << inputString << std::endl;

        // 1. 一级缓存：双缓存 LRU 内存缓存查询
        DoubleLruCache& cache = DoubleLruCache::getInstance();
        json cachedResponse;
        if (cache.get(inputString, cachedResponse)) {
            cout << "内存缓存命中，回复" << "\n";
            return HTTPResponse(200, "OK", cachedResponse, {{"Content-Type", "application/json"}});
        }

        // 2. 二级缓存：Redis 查询
        RedisSingleton& redis = RedisSingleton::getInstance();
        redisContext* context = redis.getRedisContext();
        std::string redisKey = "suggest:" + inputString; // 构造 Redis key
        redisReply* reply = (redisReply*)redisCommand(context, "GET %s", redisKey.c_str());

        // 如果在 Redis 中找到数据，写入内存缓存并返回结果
        if (reply && reply->type == REDIS_REPLY_STRING) {
            std::string cachedResult = reply->str;
            freeReplyObject(reply);// 释放 Redis 回复

            // 解析缓存的 JSON 数据
            json responseJson = json::parse(cachedResult);
            // 写入一级内存缓存
            cache.put(inputString, responseJson);
            cout << "Redis缓存命中，回复" << "\n";
            return HTTPResponse(200, "OK", responseJson, {{"Content-Type", "application/json"}});
        }

        // 没有找到，执行后续逻辑
        if (reply) freeReplyObject(reply); // 确保释放之前的 Redis 回复

        //2、分字
        size_t pos = 0;
        std::set<std::string> word;
        while (pos < inputString.length()) {
            size_t nBytes = TextProcessor::nBytesCode(inputString[pos]);
            std::string letter(inputString.substr(pos, nBytes));
            word.insert(letter);
            pos += nBytes;
        }
        //2、查找每一个字的索引，对应的候选词，求一个并集，用单例类Dictonary
        std::set<std::string> recommendation;
        std::shared_ptr<Dictonary> d =  Dictonary::getInstance();
        for(auto & w : word){
            //英文
            if(TextProcessor::nBytesCode(w[0]) == 1){
                auto it = d.get()->_Englishindex.find(w);
                if(it != d.get()->_Englishindex.end()){
                    const std::set<int>& indexSet = it->second;
                    for(int i : indexSet){
                        pair<string,int> a = d.get()->_Englishdict[i];
                        recommendation.insert(a.first);
                    }
                }  
            }else{
                auto it = d.get()->_Chineseindex.find(w);
                if(it != d.get()->_Chineseindex.end()){
                    const std::set<int>& indexSet = it->second;
                    for(int i : indexSet){
                        pair<string,int> a = d.get()->_Chinesedict[i];
                        recommendation.insert(a.first);
                    }
                }                        
            }
        }
        //3、候选词进行排序，用关键字和候选词的相似度，最小编辑距离代码，放入一个priority_queue
        struct CompareDistance {
            bool operator()(const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
                return a.first > b.first; // 使编辑距离小的元素优先
            }
        };
        std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, CompareDistance> pq;
        for(auto & candidate : recommendation){
            int distance = TextProcessor::editDistance(inputString,candidate);
            pq.emplace(distance, candidate);
        }
    //4、从队列中依次pop出候选词，放入一个json的array中回复给客户端
            // 构造 JSON 响应
        json responseArray;
        while (!pq.empty()) {
            std::pair<int, std::string> top = pq.top();
            responseArray.push_back(top.second);
            pq.pop();
        }
        json responseJson;
        responseJson["candidates"] = responseArray;

        // 7. 将结果存储到 Redis 和一级缓存中
        std::string responseString = responseJson.dump(); // 将 JSON 转换为字符串
        reply = (redisReply*)redisCommand(context, "SET %s %s", redisKey.c_str(), responseString.c_str());
        if (reply) freeReplyObject(reply); // 释放 Redis 回复
        // 写入一级内存缓存
        cache.put(inputString, responseJson);
        return HTTPResponse(200, "OK", responseJson, {{"Content-Type", "application/json"}});
    }
    return HTTPResponse(400, "Bad Request", json{{"error", "Invalid request data"}});
}

bool HTTPRequestHandler::isEnglish(char c) {
    return c >= 0 && c <= 127;
}

HTTPResponse HTTPRequestHandler::searchTask(json requestBody){
    std::shared_ptr<Dictonary> d = Dictonary::getInstance();
    //1、解析json 里面data: 对应的value是要用的数据部分，分字
    if (requestBody.contains("data")) {
        auto data = requestBody["data"];
        std::string inputString = data.get<std::string>();
        std::cout << inputString << std::endl;

        // 1. 一级缓存：双缓存 LRU 内存缓存查询
        DoubleLruCache& cache = DoubleLruCache::getInstance();
        json cachedResponse;
        if (cache.get(inputString, cachedResponse)) {
            cout << "内存缓存命中，回复" << "\n";
            return HTTPResponse(200, "OK", cachedResponse, {{"Content-Type", "application/json"}});
        }

        // 2. 二级缓存：Redis 查询
        RedisSingleton& redis = RedisSingleton::getInstance();
        redisContext* context = redis.getRedisContext();
        std::string redisKey = "search:" + inputString; // 构造 Redis key
        redisReply* reply = (redisReply*)redisCommand(context, "GET %s", redisKey.c_str());

        // 如果在 Redis 中找到数据，写入内存缓存并返回结果
        if (reply && reply->type == REDIS_REPLY_STRING) {
            std::string cachedResult = reply->str;
            freeReplyObject(reply);// 释放 Redis 回复

            // 解析缓存的 JSON 数据
            json responseJson = json::parse(cachedResult);
            // 写入一级内存缓存
            cache.put(inputString, responseJson);
            cout << "Redis缓存命中，回复" << "\n";
            return HTTPResponse(200, "OK", responseJson, {{"Content-Type", "application/json"}});
        }

        // 没有找到，执行后续逻辑
        if (reply) freeReplyObject(reply); // 确保释放之前的 Redis 回复

        struct ComparePair {
            bool operator()(const std::pair<int, double>& a, const std::pair<int, double>& b) {
                return a.second < b.second;
            }
        };
        //2、如果倒排索引里包含，就是单词，去差对应的网页 返回
        auto it = webPageHandler::newnewindex.find(inputString);
        if(it != webPageHandler::newnewindex.end()){
            std::priority_queue<std::pair<int, double>, std::vector<std::pair<int, double>>, ComparePair> pq;
            for (const auto& pair : it->second) {
                pq.push(pair);
            }
            std::cout << std::endl;
            std::vector<json> responses;
            while (!pq.empty()) {
                std::cout << "Article ID: " << pq.top().first << ", Weight: " << pq.top().second << std::endl;
                webPage wp =  webPageHandler::getwebPage(pq.top().first,"/home/wyx/interview-prep/project-review/searchs/v5/data/new_webpages.dat");
                json responseJson;

                responseJson["id"] = wp._docid;
                responseJson["title"] = wp._docTitle;
                responseJson["url"] = wp._docUrl;
                responseJson["content"] = wp._docContent;
                responses.push_back(responseJson);
                pq.pop();
            }
            json responseJson = responses;
            std::string responseString = responseJson.dump();
            cout << responseString << "\n";
            redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", redisKey.c_str(), responseString.c_str());
            if (reply) freeReplyObject(reply); // 释放 Redis 回复，防止内存泄漏
            // 写入一级内存缓存
            DoubleLruCache& cache = DoubleLruCache::getInstance();
            cache.put(inputString, responseJson);
            return HTTPResponse(200, "OK", responses, {{"Content-Type", "application/json"}});
        }else{
            //2、可能是句子，尝试分词
            std::vector<std::string> words;
            std::map<std::string, int> wordCount;
            std::cout << inputString << "\n";
            d.get()->jieba.Cut(inputString,words,true);
            //去停用词 - 使用预加载到内存的停用词表
            std::vector<std::string> newwords;

            for(auto s : words){
                if(_chinese_stopwords.find(s) == _chinese_stopwords.end()){
                    newwords.push_back(s);
                }
            }

            for (const auto& word : newwords) {
                if (!word.empty()) {
                    ++wordCount[word];
                }
            }
            //如果大小是1，说明没有找到。 如果是多个，计算出每个单词的权重。
            if(wordCount.size() < 1){
                return HTTPResponse(400, "Bad Request", json{{"error", "no valid keywords"}});;
            }
            // TF-IDF 计算:
            // N = 总文档数 = 偏移表大小
            // DF = 该单词出现在多少个文档中 = 倒排索引中该词条目数量
            int N = webPageHandler::offsetlib.size();
            std::map<std::string, double> wordforce;
            double allwordforce = 0;
            for(auto & elem : wordCount){
                int TF = elem.second;
                int DF = webPageHandler::newnewindex[elem.first].size();
                double IDF = log2((double)N / ((double)DF + 1)) + 1;
                double w = TF * IDF;
                wordforce[elem.first] = w;
                allwordforce += (w * w);
            }
            allwordforce = std::sqrt(allwordforce);
            for(auto & elem : wordforce){
                wordforce[elem.first] = elem.second/allwordforce;
            }
            //2、去遍历每一个关键词 找网页
            //2.1 计算交集：找出包含所有查询关键词的文档
            set<int> result;
            bool first = true;
            for (auto& elem : wordforce) {
                auto it = webPageHandler::newnewindex.find(elem.first);
                if (it == webPageHandler::newnewindex.end()) {
                    // 这个词没有在任何文档出现，结果为空
                    result.clear();
                    break;
                }
                // 当前词包含的文档集合
                set<int> current;
                for (auto& pa : it->second) {
                    current.insert(pa.first);
                }
                // 计算交集
                if (first) {
                    // 第一个词，直接取全部文档
                    result.swap(current);
                    first = false;
                } else {
                    // 和已有结果取交集，只保留同时出现在当前词和之前结果中的文档
                    set<int> intersection;
                    set_intersection(result.begin(), result.end(),
                                  current.begin(), current.end(),
                                  inserter(intersection, intersection.begin()));
                    result.swap(intersection);
                    if (result.empty()) {
                        break; // 提前退出，如果已经空了
                    }
                }
            }
            set<int>& reputeInet = result;
            //2.2 存储每个网页 对应每一个关键字的w 得到相似度的simage int 是 id  simpage 是cos相似读
            vector<pair<int,double>> simpage;
            for(auto & id : reputeInet){
                double cup = 0;
                double downa = 0;
                double downb = 0;
                vector<int> w;
                for(auto & keystring : wordforce){
                    set<pair<int,double>> z = webPageHandler::newnewindex[keystring.first];
                    for(auto &e :z){
                        if(e.first == id){
                            cup += e.second * keystring.second;
                            downa += (e.second * e.second);
                            downb += (keystring.second * keystring.second);
                        }
                    }
                }
                downa = std::sqrt(downa);
                downb = std::sqrt(downb);
                double down = downa * downb;
                double cossim = cup / down;
                simpage.push_back({id,cossim});
            }
            if(simpage.size() == 0){
                return HTTPResponse(400, "Bad Request", json{{"error", "can not find word"}});
            }
            std::priority_queue<std::pair<int, double>, std::vector<std::pair<int, double>>, ComparePair> pq1;
            for(auto & elem : simpage){
                pq1.push(elem);
            }
            std::vector<json> responses;
            while (!pq1.empty()) {
                std::cout << "Article ID: " << pq1.top().first << ", Weight: " << pq1.top().second << std::endl;
                webPage wp =  webPageHandler::getwebPage(pq1.top().first,"/home/wyx/interview-prep/project-review/searchs/v5/data/new_webpages.dat");
                json responseJson;
                responseJson["id"] = wp._docid;
                responseJson["title"] = wp._docTitle;
                responseJson["url"] = wp._docUrl;
                responseJson["content"] = wp._docContent;
                responses.push_back(responseJson);
                pq1.pop();
            }
            json responseJson = responses;
            std::string responseString = responseJson.dump();
            cout << responseString << "\n";
            redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", redisKey.c_str(), responseString.c_str());
            if (reply) freeReplyObject(reply); // 释放 Redis 回复，防止内存泄漏
            // 写入一级内存缓存
            DoubleLruCache& cache = DoubleLruCache::getInstance();
            cache.put(inputString, responseJson);

            return HTTPResponse(200, "OK", responses, {{"Content-Type", "application/json"}});
        }

    }
    return HTTPResponse(400, "Bad Request", json{{"error", "can not find word"}});
}
