#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__
#include <string>
#include <vector>
#include <iostream>
class HTTPRequest {
public:
    std::string method;
    std::string url;
    std::string version;
    std::vector<std::string> headers;
    std::string body;

    // 解析请求行
    void parseRequestLine(const std::string& requestLine);

    // 解析请求头
    void parseHeaders(const std::vector<std::string>& requestHeaders);
    // 解析请求体
    void parseBody(const std::string& requestBody);

    void printHttpRequest();

    void setValid(bool valid) { _valid = valid; }
    bool isValid() const { return _valid; }

private:
    bool _valid = true;
};






#endif