#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

typedef void (*TaskCallback)(void *arg);

class Task
{
public:
    Task() {}
    Task(TaskCallback onRun, TaskCallback onFinish, void *arg)
    : run(onRun), finish(onFinish), task_arg(arg), done(false) {}

    Task(const Task &t)
    {
        run = t.run;
        finish = t.finish;
        task_arg = t.task_arg;
        done = t.done;
    }

    Task& operator=(const Task &t)
    {
        run = t.run;
        finish = t.finish;
        task_arg = t.task_arg;
        done = t.done;

        return *this;
    }

    unsigned int id;
    TaskCallback run;
    TaskCallback finish;
    void *task_arg;
    bool done;
};

class ThreadInfo;

class ThreadPool
{
public:
    ThreadPool(std::string pool_name, size_t max_thread_num, size_t max_queue_num);
    virtual ~ThreadPool() {}

    int addTask(Task t);

    std::string name() const { return _name; }
    bool running() const { return _running; }
    bool quit() const { return _quit; }
    bool taskEmpty() const { return waiting_list.empty(); }
    bool doneTaskEmpty() const { return done_list.empty(); }

    void run(ThreadInfo *info);

    void destroy();

protected:
    Task getTask();

    void addDoneTask(Task &t);
    Task getDoneTask();

protected:
    std::string _name;
    std::vector<std::thread*> consumer_threads;
    std::mutex waiting_mutex;
    std::condition_variable waiting_cv;
    std::list<Task> waiting_list;

    std::mutex done_mutex;
    std::condition_variable done_cv;
    std::list<Task> done_list;

    std::atomic_bool _running;
    std::atomic_bool _quit;

    unsigned int nextTaskId;
};

class ThreadInfo
{
public:
    ThreadInfo(ThreadPool *p, size_t i) : pool(p), busy(false), threadIndex(i) {}

    ThreadPool *pool;
    bool busy;
    size_t threadIndex;
};

class ThreadWorker
{
public:
    ThreadWorker() : running(true) {}
    ~ThreadWorker();

    ThreadPool *newThreadPool(std::string poolName, size_t max_thread_num, size_t max_queue_num);
    int freeThreadPool(std::string poolName);

protected:
    std::map<std::string, ThreadPool*> threadPools;
    std::mutex mutex;
    std::atomic_bool running;
};