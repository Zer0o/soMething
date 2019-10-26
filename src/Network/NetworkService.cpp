#include "NetworkService.h"

#include <iostream>

#include <functional>
#include <iostream>
#include <unistd.h>

bool NetworkService::init()
{
    evutil_socket_t _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) return false;

    _sin.sin_family = AF_INET;
    _sin.sin_addr.s_addr = 0;
    _sin.sin_port = htons(_port);

    step = MessageDealStep::START;

    return true;
}

void NetworkService::listener_cb(evconnlistener *listener, evutil_socket_t fd,  
                 struct sockaddr *sock, int socklen, void *arg)
{
    std::cout << "service accepted connect." << std::endl;

    NetworkServiceCallbackArg *cbarg = (NetworkServiceCallbackArg*)arg;
    bufferevent *bev =  bufferevent_socket_new(cbarg->base, fd,  
                                               BEV_OPT_CLOSE_ON_FREE);  
  
    bufferevent_setcb(bev, NetworkService::read_cb, NetworkService::write_cb, NULL, cbarg->service); 
    bufferevent_setwatermark(bev, EV_READ, sizeof(Message), sizeof(Message));
    bufferevent_enable(bev, EV_READ | EV_PERSIST); 
}

void NetworkService::read_cb(bufferevent *bev, void *arg)
{
    NetworkService *service = (NetworkService*)arg;
    switch (service->step)
    {
    case START:
    {
        ssize_t len = bufferevent_read(bev, &service->msgParsed, sizeof(Message));
        if (len < 0 || (unsigned int)len != sizeof(Message) || !service->msgParsed.valid())
        {
            std::cout << "message format valid" << std::endl;
            return;
        }

        std::cout << "recv len = " << service->msgParsed.len << std::endl;

        if (service->msgParsed.len > 0)
        {
            service->step = MESSAGE_HEAD_GET;
            bufferevent_setwatermark(bev, EV_READ, service->msgParsed.len, service->msgParsed.len);
        }

        break;
    }
    case MESSAGE_HEAD_GET:
    {
        char *p = (char*)malloc(service->msgParsed.len+1);
        ssize_t len = bufferevent_read(bev, p, service->msgParsed.len);
        if (len < service->msgParsed.len)
        {
            std::cout << "buffer not enough" << std::endl;
            return;
        }

        p[len] = '\0';

        std::cout << "data = " << p << std::endl;

        service->msgParsed.data = p;

        {
            std::unique_lock<std::mutex> lock(service->request_mutex);
            service->request_messages.push(std::move(service->msgParsed));
        }

        service->step = START;

        bufferevent_setwatermark(bev, EV_READ, sizeof(Message), sizeof(Message));

        break;
    }
    }
}

void NetworkService::write_cb(bufferevent *bev, void *arg)
{
    NetworkService *service = (NetworkService*)arg;
    std::unique_lock<std::mutex> lock(service->response_mutex);
    service->response_cv.wait(lock, [service] { return !service->response_messages.empty(); });

    //send message
}

#if 0
void NetworkService::accept_cb(int fd, short events, void *arg)
{
    evutil_socket_t sockfd;
 
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
 
    sockfd = ::accept(fd, (struct sockaddr*)&client, &len );

    std::cout << "server accepted connect." << std::endl;
 
    struct event_base* base = (event_base*)arg;
 
    struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
    event_assign(ev, base, sockfd, EV_READ | EV_PERSIST,
                 NetworkService::read_cb, (void*)ev);
 
    event_add(ev, NULL);
}

void NetworkService::read_cb(int fd, short events, void *arg)
{
    Message m;
    struct event *ev = (struct event*)arg;
    ssize_t len = read(fd, &m, sizeof(Message));
    if (len < 0 || (unsigned int)len < sizeof(Message) || !m.valid())
    {
        return;
    }

    std::cout << "recv len = " << len << std::endl;
}
#endif