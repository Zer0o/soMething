#include "NetworkEngine.h"

#include<event2/bufferevent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

Engine::Engine()
{
    base = event_base_new();
}

void Engine::addService(NetworkService *service)
{
    NetworkServiceCallbackArg *arg = new NetworkServiceCallbackArg(base, service);

    struct sockaddr_in sin = service->getSin();
    evconnlistener *listener  
            = evconnlistener_new_bind(base, NetworkService::listener_cb, arg,  
                                      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,  
                                      10, (struct sockaddr *)&sin,  
                                      sizeof(struct sockaddr_in));

    //struct event* ev_listen = event_new(base, service->getFd(), EV_READ | EV_PERSIST,
    //                                    NetworkService::accept_cb, base);
    //event_add(ev_listen, NULL);
}

void Engine::addClient(NetworkClient *client)
{
    struct sockaddr_in server_addr;
 
    memset(&server_addr, 0, sizeof(server_addr) );
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->getPort());
    inet_aton(client->getIp().c_str(), &server_addr.sin_addr);

    struct bufferevent* bev = bufferevent_socket_new(base, -1,
                                                     BEV_OPT_CLOSE_ON_FREE);

    bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr,
                               sizeof(server_addr));
 
    bufferevent_setcb(bev, NetworkClient::read_cb, 
        NetworkClient::write_cb, NetworkClient::event_cb, (void*)client);
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
}

void Engine::run()
{
    event_base_dispatch(base);
}