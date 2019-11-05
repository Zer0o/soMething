#include "NetworkEngine.h"

#include<event2/bufferevent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>

Engine::Engine()
{
    base = event_base_new();
}

void Engine::addService(NetworkService *service)
{
    if (!service || !service->init())
    {
        return;
    }

    NetworkServiceCallbackArg *arg = new NetworkServiceCallbackArg(base, service);

    struct sockaddr_in *sin = service->getSin();
    evconnlistener *listener  
            = evconnlistener_new_bind(base, NetworkService::listener_cb, arg,  
                                      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,  
                                      10, (struct sockaddr *)sin,  
                                      sizeof(struct sockaddr_in));

    std::cout << "service listening on port " << service->getPort() << std::endl;
}

void Engine::addClient(NetworkClient *client)
{
    if (!client || !client->init())
    {
        return;
    }
    
    struct bufferevent* bev = bufferevent_socket_new(base, -1,
                                                     BEV_OPT_CLOSE_ON_FREE);

    bufferevent_socket_connect(bev, (struct sockaddr*)client->getSockaddrin(),
                               sizeof(struct sockaddr_in));
 
    bufferevent_setcb(bev, NetworkClient::read_cb, 
        NetworkClient::write_cb, NetworkClient::event_cb, (void*)client);
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
}

void Engine::run()
{
    event_base_dispatch(base);
}