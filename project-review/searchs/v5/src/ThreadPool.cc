#include "ThreadPool.h"
#include <iostream>

using std::cout;
using std::endl;

ThreadPool::ThreadPool(size_t threadNum, size_t queSize)
: _threadNum(threadNum)
, _threads()
, _queSize(queSize)
, _taskQue(_queSize)
, _isExit(false)
{
}

ThreadPool::~ThreadPool()
{
}

//线程池的开始或者结束
void ThreadPool::start()
{
    //创建_threadNum个工作线程，然后将其存放在vector中
    for(size_t i = 0; i < _threadNum; ++i)
    {
        _threads.push_back(thread(&ThreadPool::doTask, this));
    }
}

void ThreadPool::stop()
{
    //只要任务队列不为空，即使主线程进到stop函数中，也不能向下执行,也就是
    //不将标志位设置为true就可以(就是让主线程卡在这里)
    while(!_taskQue.empty())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //线程池要退出了，可以将标志位设置为true
    _isExit = true;

    //将所有等待在_notEmpty条件变量上的线程全部唤醒
    /* _notEmpty.notify_all(); */
    _taskQue.wakeup();

    //回收所有的工作线程
    for(auto &th : _threads)
    {
        th.join();
    }
}

//存放任务或者取出任务
void ThreadPool::addTask(Task &&taskcb)
{
    if(taskcb)
    {
        _taskQue.push(std::move(taskcb));
    }
}

Task ThreadPool::getTask()
{
   return _taskQue.pop();
}

//线程池交给工作线程执行的任务（线程入口函数）
void ThreadPool::doTask()
{
    while(!_isExit)
    {
        //获取任务，并且执行任务
        Task taskcb = getTask();
        if(taskcb)
        {
            /* ptask->process();//执行任务,肯定会体现多态 */
            taskcb();
        }
        else
        {
            cout << "nullptr == ptask" << endl;
        }
    }
}
