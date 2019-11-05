#pragma once

#include "../MessageEngine/Message.h"
#include "../MessageEngine/MessageEngine.h"

#include <event.h>
#include <event2/listener.h>

#include <condition_variable>
#include <mutex>
#include <queue>

class NetworkService
{
public:
    NetworkService(int port = -1) : _port(port) {}
    virtual ~NetworkService() {}

    virtual bool init();

    struct sockaddr_in *getSin() { return &_sin; }
    int getPort() const { return _port; }

    static void listener_cb(evconnlistener *listener, evutil_socket_t fd,  
                 struct sockaddr *sock, int socklen, void *arg);
    static void read_cb(bufferevent *bev, void *arg);
    static void write_cb(bufferevent *bev, void *arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);

protected:
    int _port;
    struct sockaddr_in _sin;

    enum MessageDealStep
    {
        START = 0,
        MESSAGE_HEAD_GET = 1,
    };

    enum MessageDealStep step;
    Message *msgParsed;

    MessageEngine *messageEngine;

    std::queue<Message> request_messages;
    std::mutex request_mutex;
    std::condition_variable request_cv;

    std::queue<Message> response_messages;
    std::mutex response_mutex;
    std::condition_variable response_cv;
};

class NetworkServiceCallbackArg
{
public:
    NetworkServiceCallbackArg(event_base *p0, NetworkService *p1)
    : base(p0), service(p1) {}

    event_base *base;
    NetworkService *service;
};