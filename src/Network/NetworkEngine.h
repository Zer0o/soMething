#pragma once

#include "NetworkClient.h"
#include "NetworkService.h"

#include <vector>

class Engine
{
public:
    Engine(bool b = false);
    virtual ~Engine() {}

    void addService(NetworkService *service);
    void addClient(NetworkClient *client);

    void run();

    std::vector<ServerInfo> getServerInfo()
    {
        std::vector<ServerInfo> sis;
        for (auto cp : clientList)
        {
            sis.push_back(cp->getServerInfo());
        }

        return std::move(sis);
    }

    NetworkClient *getClient(int index) { return clientList[index]; }

protected:
    struct event_base *base;
    std::vector<NetworkClient*> clientList;
    std::vector<NetworkService*> serviceList;
};