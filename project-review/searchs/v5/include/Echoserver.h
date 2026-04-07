#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__

#include "ThreadPool.h"
#include "TcpServer.h"
#include "HTTPRequest.h"
#include "ProcessFile.h"
#include "Dictonary.h"
#include "RSS.h"
#include "webPageHandler.h"
#include "cppjieba/Jieba.hpp"
class MyTask
{
public:
    MyTask(const HTTPRequest msg, const TcpConnectionPtr &con);
    void process();
private:
    HTTPRequest _msg;
    TcpConnectionPtr _con;
};

class Echoserver
{
public:
    Echoserver(size_t threadNum, size_t queSize
               , const string &ip
               , unsigned short port);
    ~Echoserver();

    //服务器的启动与停止
    void start();
    void stop();

    //三个回调
    void onNewConnection(const TcpConnectionPtr &con);
    void onMessage(const TcpConnectionPtr &con);
    void onClose(const TcpConnectionPtr &con);

private:
    ThreadPool _pool;//线程池对象
    TcpServer _server;//TcpServer服务器对象

};

#endif
