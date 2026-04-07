#ifndef __SOCKETIO_H__
#define __SOCKETIO_H__
#include "HTTPRequest.h"
#include "HTTPResponse.h"
class SocketIO
{
public:
    explicit SocketIO(int fd);
    ~SocketIO();
    int readn(char *buf, int len);
    // int readLine(char *buf, int len);
    int readline(char *buf, int len);
    int writen(const HTTPResponse msg);
    HTTPRequest receiveHTTPRequest();
    int recvn(void * buff, int len);
    int sendn(const void * buff, int len);
private:
    int _fd;
};

#endif
