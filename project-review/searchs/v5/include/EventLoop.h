#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <sys/epoll.h>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

using std::vector;
using std::map;
using std::shared_ptr;
using std::function;
using std::mutex;
using std::lock_guard;

class Acceptor;//前向声明
class TcpConnection;

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(const TcpConnectionPtr &)>;
using Functor = function<void()>;

class EventLoop
{
public:
    EventLoop(Acceptor &acceptor);
    ~EventLoop();

    //循环或者不循环
    void loop();
    void unloop();

private:
    //封装epoll_wait
    void waitEpollFd();

    //处理新的连接请求
    void handleNewConnection();

    //处理老的连接上的数据收发
    void handleMessage(int fd);

    //封装epoll_create
    int createEpollFd();

    //将文件描述符放在红黑树上进行监听
    void addEpollReadFd(int fd);

    //将文件描述符从红黑树上取消监听
    void delEpollReadFd(int fd);

public:
    //三个事件的注册
    void setNewConnectionCallback(TcpConnectionCallback &&cb);
    void setMessageCallback(TcpConnectionCallback &&cb);
    void setCloseCallback(TcpConnectionCallback &&cb);
public:
    //就是封装了write
    void wakeup();

private:
    //封装的是read
    void handleRead();
    //封装了eventfd
    int createEventFd();
    //执行所有的“任务”
    void doPengdingFunctors();
public:
    void runInLoop(Functor &&cb);
private:
    int _epfd;//epoll_create创建的文件描述符
    vector<struct epoll_event>  _evtList;//存储就绪文件描述符的数据结构
    bool _isLooping;//循环的标志
    Acceptor &_acceptor;//要用该类中的accept函数
    map<int, TcpConnectionPtr> _conns;//存放文件描述符与TcpConnection的键值对

    TcpConnectionCallback _onNewConnection;//连接建立
    TcpConnectionCallback _onMessage;//消息到达
    TcpConnectionCallback _onClose;//连接断开

    int _evtfd;//用于通信的文件描述符
    vector<Functor> _pendings;//存放“任务”的容器
    mutex _mutex;//互斥锁

};

#endif
