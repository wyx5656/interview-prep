#include "TaskQueue.h"

TaskQueue::TaskQueue(size_t capa)
: _capacity(capa)
, _que()
, _mutex()
, _notFull()
, _notEmpty()
, _flag(true)
{

}

TaskQueue::~TaskQueue()
{

}

//生产数据与消费数据
void TaskQueue::push(ElemType &&taskcb)
{
    //1、先上锁
    unique_lock<mutex> ul(_mutex);
    //2、判满
    while(full())
    {
        //2.1、如果任务队列是满的，生产者睡眠
        _notFull.wait(ul);
    }
    //2.2、如果任务队列不满，将数据入队列,将消费者唤醒
    _que.push(std::move(taskcb));
    _notEmpty.notify_one();
    //3、最后解锁
}

ElemType TaskQueue::pop()
{
    //1、先上锁
    unique_lock<mutex> ul(_mutex);
    //2、判空
    while(empty() && _flag)
    {
        //2.1、如果任务队列是空的，消费者睡眠
        _notEmpty.wait(ul);
    }

    if(_flag)
    {
        //2.2、如果任务队列不空，将数据出队列,将生产者唤醒
        ElemType tmp = _que.front();
        _que.pop();
        _notFull.notify_one();
        //3、最后解锁
        return tmp;
    }
    else
    {
        return nullptr;
    }
}

//判空与判满
bool TaskQueue::empty() const
{
    return  0 == _que.size();
}

bool TaskQueue::full() const
{
    return _capacity == _que.size();
}

//将所有等待在_notEmpty条件变量上的线程全部唤醒
void TaskQueue::wakeup()
{
    _flag = false;
    _notEmpty.notify_all();
}
