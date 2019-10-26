#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
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
    ThreadPool(std::string pool_name, size_t thread_num);
    virtual ~ThreadPool() {}

    void addTask(Task t);

    bool running() const { return _running; }
    bool taskEmpty() const { return waiting_list.empty(); }
    bool doneTaskEmpty() const { return done_list.empty(); }

    void run(ThreadInfo *info);

protected:
    Task getTask();

    void addDoneTask(Task &t);
    Task getDoneTask();

protected:
    std::string name;
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