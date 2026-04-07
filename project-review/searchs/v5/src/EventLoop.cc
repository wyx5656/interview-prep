#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include <unistd.h>
#include <sys/eventfd.h>

#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

EventLoop::EventLoop(Acceptor &acceptor)
: _epfd(createEpollFd())
, _evtList(1024)
, _isLooping(false)
, _acceptor(acceptor)
, _evtfd(createEventFd())
, _pendings()
, _mutex()
{
    //监听文件描述符listenfd
    int listenfd = _acceptor.fd();
    addEpollReadFd(listenfd);

    //需要将用于通信的文件描述符放在红黑树上进行监听
    addEpollReadFd(_evtfd);
}

EventLoop::~EventLoop()
{
    close(_epfd);
    close(_evtfd);
}

//循环或者不循环
void EventLoop::loop()
{
    _isLooping = true;
    while(_isLooping)
    {
        waitEpollFd();
    }
}

void EventLoop::unloop()
{
    _isLooping = false;
}

//封装epoll_wait
void EventLoop::waitEpollFd()
{
    int nready = 0;
    do{
        nready = epoll_wait(_epfd, &*_evtList.begin(), _evtList.size(), 3000);
    }while(-1 == nready && errno == EINTR);

    if(-1 == nready)
    {
        cerr << "-1 == nready" << endl;
        return;
    }
    else if(0 == nready)
    {
        cout << ">>epoll_wait timeout!!!" << endl;
    }
    else
    {
        //如果就绪的文件描述符变多之后，有可能_evtList会越界
        if(nready == (int)_evtList.size())
        {
            _evtList.resize(2 * nready);
        }

        for(int idx = 0; idx < nready; ++idx)
        {
            int fd = _evtList[idx].data.fd;
            int listenfd = _acceptor.fd();

            if(fd == listenfd)
            {
                if(_evtList[idx].events & EPOLLIN)
                {
                    //处理新的连接请求
                    handleNewConnection();
                }
            }
            else if(fd == _evtfd)//表明_evtfd就绪了
            {
                if(_evtList[idx].events & EPOLLIN)
                {
                    handleRead();//清零
                    //需要将vector中的所有任务都遍历执行
                    doPengdingFunctors();
                }
            }
            else
            {
                if(_evtList[idx].events & EPOLLIN)
                {
                    //处理老的连接上的数据的收发
                    handleMessage(fd);
                }
            }
        }
    }
}

//处理新的连接请求
void EventLoop::handleNewConnection()
{
    //重要connfd是正常，就表明三次握手建立成功了
    int connfd = _acceptor.accept();
    if(-1 == connfd)
    {
        perror("handleNewConnection");
        return;
    }

    //需要监听connfd
    addEpollReadFd(connfd);

    //将文件描述符与TcpConnection的键值存储在map中
    //创建了TcpConnection连接的对象
    TcpConnectionPtr con(new TcpConnection(connfd, this));

    con->setNewConnectionCallback(_onNewConnection);//连接建立
    con->setMessageCallback(_onMessage);//消息到达
    con->setCloseCallback(_onClose);//连接断开

    _conns[connfd] = con;

    //连接建立的时机已经到了，需要执行连接建立的事件
    con->handleNewConnectionCallback();
}

//处理老的连接上的数据收发
void EventLoop::handleMessage(int fd)
{
    auto it = _conns.find(fd);
    if(it != _conns.end())
    {
        bool flag = it->second->isClosed();//需要判断连接有没有断开
        if(flag)
        {
            //连接断开
            it->second->handleCloseCallback();//连接断开的事件
            //需要将文件描述符从红黑树上摘除掉
            delEpollReadFd(fd);
            //还需要从map中删除掉
            _conns.erase(it);
        }
        else
        {
            it->second->handleMessageCallback();//消息到达的事件
        }
      
    }
    else
    {
        cout << "该连接不存在" << endl;
        return;
    }
}

//封装epoll_create
int EventLoop::createEpollFd()
{
    int fd = epoll_create1(0);
    if(fd < 0)
    {
        perror("createEpollFd");
        return -1;
    }

    return fd;
}

//将文件描述符放在红黑树上进行监听
void EventLoop::addEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt);
    if(ret < 0)
    {
        perror("addEpollReadFd");
        return;
    }
}

//将文件描述符从红黑树上取消监听
void EventLoop::delEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &evt);
    if(ret < 0)
    {
        perror("delEpollReadFd");
        return;
    }
}

void EventLoop::setNewConnectionCallback(TcpConnectionCallback &&cb)
{
    _onNewConnection = std::move(cb);
}

void EventLoop::setMessageCallback(TcpConnectionCallback &&cb)
{
    _onMessage = std::move(cb);
}

void EventLoop::setCloseCallback(TcpConnectionCallback &&cb)
{
    _onClose = std::move(cb);
}

//就是封装了write
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t ret = write(_evtfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("wakeup");
        return;
    }
}

//封装的是read
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t ret = read(_evtfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("handleRead");
        return;
    }

}
//封装了eventfd
int EventLoop::createEventFd()
{
    int fd = eventfd(10, 0);
    if(fd < 0)
    {
        perror("createEventFd");
        return -1;
    }
    return fd;
}

void EventLoop::doPengdingFunctors()
{
    //2、遍历vector执行所有的任务之后，需要将任务清空
    vector<Functor> tmp;
    {
        lock_guard<mutex> lg(_mutex);
        tmp.swap(_pendings);
    }

    for(auto &cb : tmp)
    {
        //就在执行“任务”:使用同一条连接，
        //调用send函数将处理好之后的数据
        //发送给客户端
        cb();
    }
}
void EventLoop::runInLoop(Functor &&cb)
{
    //将“任务”添加到vector中
    {
        lock_guard<mutex> lg(_mutex);
        _pendings.push_back(std::move(cb));
    }

    //任务添加到vector后还需要做什么？
    wakeup();
}

