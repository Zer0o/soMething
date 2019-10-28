#include "ThreadPool.h"

#include <iostream>

void consumer_run(void *arg)
{
    ThreadInfo *info = (ThreadInfo *)arg;
    for (;;)
    {
        info->pool->run(info);
    }
}

ThreadPool::ThreadPool(std::string pool_name, size_t max_thread_num, size_t max_queue_num)
    : _name(pool_name), nextTaskId(0)
{
    consumer_threads.resize(max_thread_num);

    for (size_t i = 0; i < max_thread_num; ++i)
    {
        ThreadInfo *ti = new ThreadInfo(this, i); //todo:delete in thread
        std::thread *t = new std::thread(consumer_run, ti);
        consumer_threads[i] = t;
    }

    _running = true;
    _quit = false;
}

int ThreadPool::addTask(Task t)
{
    std::unique_lock<std::mutex> lock(waiting_mutex);
    t.id = nextTaskId++;
    waiting_list.push_back(t);
    waiting_cv.notify_one();

    return 0;
}

void ThreadPool::run(ThreadInfo *info)
{
    Task t = getTask(); 

    t.run(t.task_arg);
    //t.finish(t.task_arg);
    
    //addDoneTask(t);
}

void thread_exit_task(void *arg)
{
    pthread_exit(0);
}

void ThreadPool::destroy()
{
    _quit = true;
    Task exit_task(thread_exit_task, thread_exit_task, NULL);
    for (auto tp : consumer_threads)
    {
        addTask(exit_task);
    }
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

ThreadWorker::~ThreadWorker()
{
    std::cout << "threadworker free" << std::endl;
    running = false;
    std::unique_lock<std::mutex> lock(mutex);
    for (auto iter : threadPools)
    {
        ThreadPool *pool = iter.second;
        if (pool) pool->destroy();
    }
}

ThreadPool *ThreadWorker::newThreadPool(std::string poolName, size_t max_thread_num, size_t max_queue_num)
{
    if (!running) return NULL;

    std::unique_lock<std::mutex> lock(mutex);
    auto iter = threadPools.find(poolName);
    if (iter != threadPools.end())
    {
        return iter->second;
    }

    ThreadPool *pool = new ThreadPool(poolName, max_thread_num, max_queue_num);
    threadPools[poolName] = pool;
    return pool;
}

int ThreadWorker::freeThreadPool(std::string poolName)
{
    ThreadPool *pool = NULL;
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto iter = threadPools.find(poolName);
        if (iter == threadPools.end())
        {
            return -1;
        }

        pool = iter->second;
    }

    if (pool) pool->destroy();

    return 0;
}