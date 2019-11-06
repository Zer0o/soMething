#include "NetworkEngine.h"

#include<event2/bufferevent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream> 

static const char *welcome = 
"Welcome to clish, the client terminal.\n"
"\n"
"Type:  'help' for help with commands\n"
"       'quit or exit' to quit\n";

static const char *clish = "clish>";

struct help
{
    const char *command;
    const char *help;
} helpList[] = 
{
    {"addCommand", 
     "  arg1: command name"},
    {"connnect",
     "  arg1: server ip\n"
     "  arg2: server port"},
    {"exeCommand",
     "  arg1: command name\n"
     "  arg2: command arg"},
    {"list", 
     "  list all commands"},
    {"upload",
     "  arg1: filepath to upload\n"
     "  arg2: server to upload"},
};

void stdin_read_cb(struct bufferevent* bev, void *arg)
{
    char a[1024];
    size_t sz = bufferevent_read(bev, a, 1024);
    if (sz < 1) return;
    a[sz-1] = '\0';

    if (!strcmp(a, "quit"))
    {
        exit(0);
    }
    else if (!strcmp(a, "exit"))
    {
        exit(0);
    }
    else if (!strcmp(a, "help"))
    {
        for (auto h : helpList)
        {
            std::cout << h.command << std::endl;
            std::cout << h.help << std::endl;
        }
    }
    else
    {
        std::cout << "no such command" << std::endl;
    }

    std::cout << std::endl;
    std::cout << clish;
    std::cout.flush();
}

void stdout_write_cb(struct bufferevent* bev, void *arg)
{
    //std::cout << "hello" << std::endl;
}

Engine::Engine()
{
    base = event_base_new();

    struct bufferevent* stdinbev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setfd(stdinbev, 0);
    bufferevent_setcb(stdinbev, stdin_read_cb, 
        stdout_write_cb, NULL, this);
    bufferevent_enable(stdinbev, EV_READ | EV_WRITE | EV_PERSIST);
    bufferevent_write(stdinbev, welcome, strlen(welcome));
    bufferevent_write(stdinbev, clish, strlen(clish));
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

    client->setBase(base);

    NetworkClient::new_client(0, 0, client);
}

void Engine::run()
{
    event_base_dispatch(base);
}