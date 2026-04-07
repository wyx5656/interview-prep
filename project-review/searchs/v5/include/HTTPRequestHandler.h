#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "ProcessFile.h"
#include "Dictonary.h"
#include "webPageHandler.h"
#include "RedisSingleton.h"
#include <sstream>
#include <map>
#include <set>
#include <cmath>

using json = nlohmann::json;

class HTTPRequestHandler {
public:
    HTTPRequestHandler();
    HTTPResponse handleRequest(const HTTPRequest& request);
private:
    json parseRequestBody(const std::string& requestBody);
    HTTPResponse suggestTask(json requestBody);
    HTTPResponse searchTask(json requestBody);
    bool isEnglish(char c);

    // 停用词预加载到内存，避免每次搜索重新读取文件
    static bool _stopwords_loaded;
    static std::set<std::string> _chinese_stopwords;
    static std::set<std::string> _english_stopwords;
};

#endif // HTTPREQUESTHANDLER_H
