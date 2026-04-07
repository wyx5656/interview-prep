#include "Echoserver.h"
#include "TcpConnection.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPRequestHandler.h"
#include <iostream>
#include <functional>
using std::cout;
using std::endl;
using std::bind;

MyTask::MyTask(const HTTPRequest msg, const TcpConnectionPtr &con)
: _msg(msg)
, _con(con)
{

}
void MyTask::process()
{
    HTTPRequestHandler h;
    HTTPResponse res = h.handleRequest(_msg);
    //所有的业务逻辑全部在该函数中进行处理
    //处理完毕之后需要将数据发出去
    // json body = {
    //     {"message", "Hello, World!"},
    //     {"data", {
    //         {"item1", "value1"},
    //         {"item2", "value2"}
    //     }}
    // };

    // // 创建 HTTP 响应
    // HTTPResponse response(200, "OK", body, {
    //     {"Server", "MyServer"},
    //     {"Date", "Sat, 12 Oct 2024 20:20:00 GMT"}
    // });
    _con->sendInLoop(res);
}

Echoserver::Echoserver(size_t threadNum, size_t queSize
                       , const string &ip
                       , unsigned short port)
: _pool(threadNum, queSize)
, _server(ip, port)
{

}

Echoserver::~Echoserver()
{

}

//服务器的启动与停止
void Echoserver::start()
{
    _pool.start();//线程池对象启动起来

    using namespace std::placeholders;
    _server.setAllCallback(std::bind(&Echoserver::onNewConnection, this, _1)
                           , std::bind(&Echoserver::onMessage, this, _1)
                           , std::bind(&Echoserver::onClose, this, _1));
    _server.start();//TcpServer对象启动起来
}

void Echoserver::stop()
{
    _pool.stop();
    _server.stop();
}

//三个回调
void Echoserver::onNewConnection(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has connected!!!" << endl;
}

void Echoserver::onMessage(const TcpConnectionPtr &con)
{
    //接收客户端的数据
    HTTPRequest httprequest = con->receive();
    httprequest.printHttpRequest();


    //业务逻辑的处理如果比较复杂，可以业务逻辑的处理交个线程池
    MyTask task(httprequest, con);
    _pool.addTask(bind(&MyTask::process, task));
}

void Echoserver::onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!!!" << endl;
}
