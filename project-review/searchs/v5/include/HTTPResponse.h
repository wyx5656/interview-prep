#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__
#include <string>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class HTTPResponse {
public:
    HTTPResponse(int statusCode, const std::string& statusMessage, const json& body, const std::map<std::string, std::string>& headers = {});

    std::string generateResponse() const;

private:
    int statusCode;
    std::string statusMessage;
    json body;
    std::map<std::string, std::string> headers;
};

#endif