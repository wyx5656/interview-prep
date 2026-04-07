#include "HTTPRequest.h"
#include <iostream>
// POST /login.php HTTP/1.1
// Host: www.example.com
// User - Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/90.0.4430.212 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q = 0.9,image/avif,image/webp,image/apng,*/*;q = 0.8,application/signed - exchange;v = b3;q = 0.9
// Accept - Language: en - US,en;q = 0.9
// Content - Type: application/x - www - form - urlencoded
// Content - Length: 27
// Connection: keep - alive

// username=john&password=123456

void HTTPRequest::parseRequestLine(const std::string& requestLine){

    size_t pos1 = requestLine.find(' ');
    method = requestLine.substr(0, pos1);
    size_t pos2 = requestLine.find(' ', pos1 + 1);
    url = requestLine.substr(pos1 + 1, pos2 - pos1 - 1);
    version = requestLine.substr(pos2 + 1);

}

void HTTPRequest::parseHeaders(const std::vector<std::string>& requestHeaders){
    headers = requestHeaders;
}


void HTTPRequest::parseBody(const std::string& requestBody){
    body = requestBody;
}


void HTTPRequest::printHttpRequest() {
    std::cout << method << " " << url << " " << version << std::endl;
    for (const auto& header : headers) {
        std::cout << header << std::endl;
    }
    std::cout << std::endl;
    std::cout << body << std::endl;
}
