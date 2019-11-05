#pragma once

#include "../MessageEngine/Message.h"

#include <event.h>
#include <string>
#include <list>

class NetworkClient
{
public:
    NetworkClient(std::string ip, int port)
    : _connect_ip(ip), _connect_port(port) {}
    virtual ~NetworkClient() {}

    virtual bool init();

    static void read_cb(struct bufferevent* bev, void* arg);
    static void write_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);

    std::string getIp() const { return _connect_ip; }
    int getPort() const { return _connect_port; }
    struct sockaddr_in *getSockaddrin() { return &server_addr;}

    void addMessage(Message *m)
    {
        _msgList.push_back(m);
    }

protected:
    int _fd;
    std::string _connect_ip;
    int _connect_port;
    //std::list<std::string> data;
    std::list<Message *> _msgList;

    struct sockaddr_in server_addr;
};