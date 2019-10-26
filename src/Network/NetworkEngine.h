#pragma once

#include "NetworkClient.h"
#include "NetworkService.h"

class Engine
{
public:
    Engine();
    virtual ~Engine() {}

    void addService(NetworkService *service);
    void addClient(NetworkClient *client);

    void run();

protected:
    struct event_base *base;
};