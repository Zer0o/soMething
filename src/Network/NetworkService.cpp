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

    msgParsed = new Message;
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
  
    bufferevent_setcb(bev, NetworkService::read_cb, NetworkService::write_cb, NetworkService::event_cb, cbarg->service); 
    bufferevent_setwatermark(bev, EV_READ, sizeof(Message), sizeof(Message));
    bufferevent_enable(bev, EV_READ);
}

void NetworkService::read_cb(bufferevent *bev, void *arg)
{
    NetworkService *service = (NetworkService*)arg;
    switch (service->step)
    {
    case START:
    {
        size_t len = bufferevent_read(bev, &service->msgParsed, sizeof(Message));
        if (len == 0)
        {
            std::cout << "read len = 0, close" << std::endl;
            goto CLOSE;
        }
        if (len != sizeof(Message) || !service->msgParsed->valid())
        {
            std::cout << "message format valid" << std::endl;
            return;
        }

        std::cout << "recv len = " << service->msgParsed->len << std::endl;

        if (service->msgParsed->len > 0)
        {
            service->step = MESSAGE_HEAD_GET;
            bufferevent_setwatermark(bev, EV_READ, service->msgParsed->len, service->msgParsed->len);
        }

        break;
    }
    case MESSAGE_HEAD_GET:
    {
        //use memory pool
        char *p = (char*)malloc(service->msgParsed->len+1);
        size_t len = bufferevent_read(bev, p, service->msgParsed->len);
        if (len == 0)
        {
            std::cout << "read len = 0, close" << std::endl;
            goto CLOSE;
        }

        if (len < (size_t)service->msgParsed->len)
        {
            std::cout << "buffer not enough" << std::endl;
            return;
        }

        p[len] = '\0';

        std::cout << "data = " << p << std::endl;

        service->msgParsed->data = p;

#if 0
        {
            std::unique_lock<std::mutex> lock(service->request_mutex);
            service->request_messages.push(std::move(service->msgParsed));
            service->request_cv.notify_one();
        }
#endif

        service->step = START;

        bufferevent_setwatermark(bev, EV_READ, sizeof(Message), sizeof(Message));

        break;
    }
    }
CLOSE:
    return ;
}

void NetworkService::write_cb(bufferevent *bev, void *arg)
{
    std::cout << "write " << std::endl;

    NetworkService *service = (NetworkService*)arg;
    std::unique_lock<std::mutex> lock(service->response_mutex);
    service->response_cv.wait(lock, [service] { return !service->response_messages.empty(); });

    //send message
}

void NetworkService::event_cb(struct bufferevent *bev, short event, void *arg)
{
    NetworkService *service = (NetworkService*)arg;
    if (event & BEV_EVENT_EOF)
    {
        std::cout << "eof." << std::endl;
        bufferevent_free(bev);
    }
    else if (event & BEV_EVENT_TIMEOUT)
    {
        std::cout << "service timeout." << std::endl;
        bufferevent_free(bev);
    }
    else if (event & BEV_EVENT_ERROR)
    {
        std::cout << "service error." << std::endl;
        bufferevent_free(bev);
    }
}