#include "HTTPResponse.h"

HTTPResponse::HTTPResponse(int statusCode, const std::string& statusMessage, const json& body, const std::map<std::string, std::string>& headers)
    : statusCode(statusCode)
    , statusMessage(statusMessage)
    , body(body)
    , headers(headers) {}

std::string HTTPResponse::generateResponse() const {
    std::ostringstream responseStream;
    // 响应行
    responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    // 响应头
    for (const auto& header : headers) {
        responseStream << header.first << ": " << header.second << "\r\n";
    }
    // 添加 Content-Type 头
    responseStream << "Content-Type: application/json\r\n";
    // 添加 Content-Length 头
    std::string bodyString = body.dump();
    responseStream << "Content-Length: " << bodyString.length() << "\r\n";
    // 空行，表示头部结束
    responseStream << "\r\n";
    // 响应体
    responseStream << bodyString;
    return responseStream.str();
}