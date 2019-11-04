#pragma once

#include "Message.h"
#include "../ThreadPool/ThreadPool.h"

#include <queue>

typedef Message* (*func)(void *arg);

class MessageEngine
{
public:
    MessageEngine();
    ~MessageEngine();

    void addRequestMsg(Message *);
    Message *getResponseMsg();
    static func getSoFunction(std::string so_name, std::string func_name);

protected:
    void dealMessage();

protected:
    std::queue<Message*> request_messages;
    std::mutex request_mutex;
    std::condition_variable request_cv;

    std::queue<Message> response_messages;
    std::mutex response_mutex;
    std::condition_variable response_cv;
};