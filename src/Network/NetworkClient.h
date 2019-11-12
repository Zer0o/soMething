#pragma once

#include "../File/FileManage.h"
#include "../MessageEngine/Message.h"

#include <iostream>

#include <event.h>
#include <string>
#include <list>

class ServerInfo
{
public:
    ServerInfo(std::string ip, int port, int status)
     : _ip(ip), _port(port), _status(status) {}

    std::string _ip;
    int _port;
    int _status;
};

class NetworkClient
{
public:
    NetworkClient(std::string ip, int port)
    : _connect_ip(ip), _connect_port(port) {}
    virtual ~NetworkClient() {}

    virtual bool init();

    static void new_client(int, short, void *arg);

    static void read_cb(struct bufferevent* bev, void* arg);
    static void write_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);

    std::string getIp() const { return _connect_ip; }
    int getPort() const { return _connect_port; }
    struct sockaddr_in *getSockaddrin() { return &server_addr;}

    void setBase(struct event_base *b) { base = b; }

    void addMessage(Message *m)
    {
        _msgList.push_back(m);
        bufferevent_enable(bev, EV_WRITE);
    }

    ServerInfo getServerInfo() const
    {
        ServerInfo si(_connect_ip, _connect_port, 0);
        return std::move(si);
    }

protected:
    int _fd;
    std::string _connect_ip;
    int _connect_port;
    //std::list<std::string> data;
    std::list<Message *> _msgList;

    struct sockaddr_in server_addr;
    struct event_base *base;
    struct bufferevent* bev;
};