#include "ThreadPool.h"

void consumer_run(void *arg)
{
    ThreadInfo *info = (ThreadInfo *)arg;
    for (;;)
    {
        info->pool->run(info);
    }
}

ThreadPool::ThreadPool(std::string pool_name, size_t thread_num)
    : name(pool_name), nextTaskId(0)
{
    consumer_threads.resize(thread_num);

    for (size_t i = 0; i < thread_num; ++i)
    {
        ThreadInfo *ti = new ThreadInfo(this, i); //todo:delete in thread
        std::thread *t = new std::thread(consumer_run, ti);
        consumer_threads[i] = t;
    }

    _running = true;
    _quit = false;
}

void ThreadPool::addTask(Task t)
{
    std::unique_lock<std::mutex> lock(waiting_mutex);
    t.id = nextTaskId++;
    waiting_list.push_back(t);
    waiting_cv.notify_one();
}

void ThreadPool::run(ThreadInfo *info)
{
    Task t = getTask(); 

    info->busy = true;
    t.run(t.task_arg);
    t.done = true;
    //t.finish(t.task_arg);
    info->busy = false;
    
    //addDoneTask(t);
}

Task ThreadPool::getTask()
{
    Task t;
    std::unique_lock<std::mutex> lock(waiting_mutex);
    waiting_cv.wait(lock, [this] {return !this->taskEmpty();});
    t = waiting_list.front();
    waiting_list.pop_front();
    return t;
}

void ThreadPool::addDoneTask(Task &t)
{
    std::unique_lock<std::mutex> lock(done_mutex);
    done_list.push_back(t);
}

Task ThreadPool::getDoneTask()
{
    Task t;
    std::unique_lock<std::mutex> lock(done_mutex);
    done_cv.wait(lock, [this] {return !this->taskEmpty();});
    t = done_list.front();
    done_list.pop_front();
    return t;
}