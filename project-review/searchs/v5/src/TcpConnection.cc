#include "TcpConnection.h"
#include "EventLoop.h"
#include "HTTPRequest.h"
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;
using std::ostringstream;

TcpConnection::TcpConnection(int fd, EventLoop *loop)
: _loop(loop)
, _sockIO(fd)
, _sock(fd)
, _localAddr(getLocalAddr())
, _peerAddr(getPeerAddr())
{

}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::send(const HTTPResponse msg)
{
    _sockIO.writen(msg);
}

//该函数需要如何实现才能将数据msg发给EventLoop呢？
//真正要做的事情其实就是将msg传递给EventLoop,但是EventLoop没有发送数据的
//能力，并且我们也不能凭空再造一个连接，因为收发数据应该是同一个连接
//所以需要将指向连接的this指针以及要发送的数据msg以及发送数据能力的send
//函数都给EventLoop
void TcpConnection::sendInLoop(const HTTPResponse msg)
{
    if(_loop)
    {
        //void runInLoop(function<void()> &&)
        _loop->runInLoop(bind(&TcpConnection::send, this, msg));
    }
}

HTTPRequest TcpConnection::receive()
{

    HTTPRequest http = _sockIO.receiveHTTPRequest();

    return http;
}

bool TcpConnection::isClosed() const
{
    char buff[10] = {0};
    int ret = ::recv(_sock.fd(), buff, sizeof(buff), MSG_PEEK);

    return (ret == 0);
}

string TcpConnection::toString()
{
    ostringstream oss;
    oss << _localAddr.ip() << ":"
        << _localAddr.port() << "---->"
        << _peerAddr.ip() << ":"
        << _peerAddr.port();

    return oss.str();
}

//获取本端的网络地址信息
InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr );
    //获取本端地址的函数getsockname
    int ret = getsockname(_sock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getsockname");
    }

    return InetAddress(addr);
}

//获取对端的网络地址信息
InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr );
    //获取对端地址的函数getpeername
    int ret = getpeername(_sock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getpeername");
    }

    return InetAddress(addr);
}

//三个事件的注册
void TcpConnection::setNewConnectionCallback(const TcpConnectionCallback &cb)
{
    _onNewConnection = cb;
}

void TcpConnection::setMessageCallback(const TcpConnectionCallback &cb)
{
    _onMessage = cb;
}

void TcpConnection::setCloseCallback(const TcpConnectionCallback &cb)
{
    _onClose = cb;
}

//三个事件的执行
void TcpConnection::handleNewConnectionCallback()
{
    if(_onNewConnection)
    {
        /* _onNewConnection(shared_ptr<TcpConnection>(this));//有风险 */
        _onNewConnection(shared_from_this());
    }
    else
    {
        cout << "_onNewConnection == nullptr" << endl;
    }
}

void TcpConnection::handleMessageCallback()
{
    if(_onMessage)
    {
        _onMessage(shared_from_this());
    }
    else
    {
        cout << "_onMessage == nullptr" << endl;
    }
}

void TcpConnection::handleCloseCallback()
{
    if(_onClose)
    {
        _onClose(shared_from_this());
    }
    else
    {
        cout << "_onClose == nullptr" << endl;
    }
}
