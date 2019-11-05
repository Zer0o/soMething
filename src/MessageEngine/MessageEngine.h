#pragma once

#include "Message.h"
#include "../Protobuf/Message.pb.h"
#include "../ThreadPool/ThreadPool.h"

#include <map>
#include <queue>

typedef Message* (*func)(void *arg);

class MessageEngine
{
public:
    MessageEngine();
    ~MessageEngine();

    void setThreadPool(ThreadPool *p) { _pool = p; }

    void addRequestMsg(Message *);
    Message *getResponseMsg();
    Message *exeSoFunction(const std::string &so_name, const std::string &func_name, void *arg);

    void dealMessage(Message *m);

protected:
    ThreadPool *_pool;

    time_t _lastHeartbeat;

    std::map<std::string, msg::Command> _commandMap;

#if 0
    std::queue<Message*> request_messages;
    std::mutex request_mutex;
    std::condition_variable request_cv;

    std::queue<Message> response_messages;
    std::mutex response_mutex;
    std::condition_variable response_cv;
#endif
};