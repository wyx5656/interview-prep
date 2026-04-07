#include "SocketIO.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>
SocketIO::SocketIO(int fd)
: _fd(fd)
{

}

SocketIO::~SocketIO()
{
    /* close(_fd); */
}

//其作用：确定发送len字节的数据
int SocketIO::sendn(const void * buff, int len)
{
    int left = len;
    const char * pbuf = (char*)buff;
    int ret = -1;
    while(left > 0) {
        ret = send(_fd, pbuf, left, 0);
        if(ret < 0) {
            perror("send");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    return len - left;
}

//其作用：确定接收len字节的数据
int SocketIO::recvn(void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = (char*)buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(_fd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//len = 10000    1500/6     1000/1
int SocketIO::readn(char *buf, int len)
{
    int left = len;
    char *pstr = buf;
    int ret = 0;

    while(left > 0)
    {
        ret = read(_fd, pstr, left);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("read error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            pstr += ret;
            left -= ret;
        }
    }

    return len - left;
}

HTTPRequest SocketIO::receiveHTTPRequest() {
    HTTPRequest request;

    // 接收请求行
    char buffer[1024];
    int lineLength = readline(buffer, sizeof(buffer));

    std::string requestLine(buffer, lineLength);
    request.parseRequestLine(requestLine);

    // 接收请求头
    std::vector<std::string> headers;
    while (true) {
        lineLength = readline(buffer, sizeof(buffer));
        if (lineLength == 2) {
            break;
        }
        std::string headerLine(buffer, lineLength);
        headers.push_back(headerLine);
    }
    request.parseHeaders(headers);

    // 如果是POST等有请求体的请求，根据Content - Length解析请求体
    //
    for (const auto& header : request.headers) {
        if (header.find("Content-Length:") == 0) {
            size_t pos = header.find(':');
            int contentLength = std::stoi(header.substr(pos + 1));
            // 安全检查：限制最大请求体大小防止溢出
            const int MAX_CONTENT_LENGTH = 10 * 1024 * 1024; // 10MB
            if (contentLength > MAX_CONTENT_LENGTH) {
                // 过大直接返回，不处理
                request.setValid(false);
                return request;
            }
            if (contentLength <= 0) {
                request.setValid(false);
                return request;
            }
            // 动态分配缓冲区，避免栈溢出
            std::vector<char> buffer(contentLength);
            lineLength = readn(buffer.data(), contentLength);
            std::string requestBody(buffer.data(), lineLength);
            request.parseBody(requestBody);
            break;
        }
    }

    return request;
}


int SocketIO::readline(char *buf, int len)
{
    int left = len - 1;
    char *pstr = buf;
    int ret = 0, total = 0;

    while(left > 0)
    {
        //MSG_PEEK不会将缓冲区中的数据进行清空,只会进行拷贝操作
        ret = recv(_fd, pstr, left, MSG_PEEK);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("readLine error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            for(int idx = 0; idx < ret; ++idx)
            {
                if(pstr[idx] == '\n')
                {
                    int sz = idx + 1;
                    readn(pstr, sz);
                    pstr += sz;
                    *pstr = '\0';//C风格字符串以'\0'结尾

                    return total + sz;
                }
            }

            //readn的底层调用是read，会将数据从内核态拷贝到
            //用户态，并且数据从内核态移除
            readn(pstr, ret);//从内核态拷贝到用户态
            total += ret;
            pstr += ret;
            left -= ret;
        }
    }
    *pstr = '\0';
    return total - left;

}

int SocketIO::writen(const HTTPResponse msg)
{
    std::string responseString = msg.generateResponse();
    size_t totalSent = 0;
    size_t length = responseString.length();
    ssize_t ret;
    const char *pstr = responseString.c_str();

    while (totalSent < length) {
        ret = write(_fd, pstr + totalSent, length - totalSent);
        if (-1 == ret && errno == EINTR) {
            continue;
        } else if (-1 == ret) {
            perror("writen error -1");
            return -1;
        } else if (0 == ret) {
            break;
        } else {
            totalSent += ret;
        }
    }

    return totalSent;
}
