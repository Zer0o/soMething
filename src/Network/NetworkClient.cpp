#include "../MessageEngine/Message.h"
#include "NetworkClient.h"

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

bool NetworkClient::init()
{
    memset(&server_addr, 0, sizeof(server_addr) );
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(getPort());
    inet_aton(getIp().c_str(), &server_addr.sin_addr);

    return true;
}

void NetworkClient::read_cb(struct bufferevent *bev, void *arg)
{

}

void NetworkClient::write_cb(struct bufferevent *bev, void *arg)
{
    NetworkClient *client = (NetworkClient*)arg;
    std::cout << "enable write" << std::endl;

    if (!client->_msgList.empty())
    {
        Message *p = client->_msgList.front();
        client->_msgList.pop_front();
        Message *dst = (Message*)malloc(sizeof(Message)+p->len*sizeof(char));
        *dst = *p;
        memcpy((char*)dst+sizeof(Message), p->data, p->len);

        bufferevent_write(bev, dst, sizeof(Message)+p->len);
    }
}

void NetworkClient::event_cb(struct bufferevent *bev, short event, void *arg)
{
    NetworkClient *client = (NetworkClient*)arg;
    if (event & BEV_EVENT_CONNECTED)
    {
        std::cout << "connected." << std::endl;
        Message *m = new Message(0, MessageType::PING_REQUEST, 0);
        bufferevent_write(bev, m, sizeof(Message));
    }
    else if (event & BEV_EVENT_TIMEOUT)
    {
        std::cout << "conntected timtout." << std::endl;
    }
    else if (event & BEV_EVENT_ERROR)
    {
        std::cout << "error. reconnecting in 5s....." << std::endl;

        struct event_base *base = bufferevent_get_base(bev);
        bufferevent_free(bev);

        sleep(5);
        struct bufferevent* bev = bufferevent_socket_new(base, -1,
                                                     BEV_OPT_CLOSE_ON_FREE);
        bufferevent_socket_connect(bev, (struct sockaddr*)client->getSockaddrin(), sizeof(struct sockaddr));
        bufferevent_setcb(bev, NetworkClient::read_cb, 
        NetworkClient::write_cb, NetworkClient::event_cb, (void*)client);
        bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
    }
    else if (event & BEV_EVENT_EOF)
    {
        std::cout << "eof. reconnecting in 5s....." << std::endl;

        struct event_base *base = bufferevent_get_base(bev);
        bufferevent_free(bev);

        sleep(5);
        struct bufferevent* bev = bufferevent_socket_new(base, -1,
                                                     BEV_OPT_CLOSE_ON_FREE);
        bufferevent_socket_connect(bev, (struct sockaddr*)client->getSockaddrin(), sizeof(struct sockaddr));
        bufferevent_setcb(bev, NetworkClient::read_cb, 
        NetworkClient::write_cb, NetworkClient::event_cb, (void*)client);
        bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
    }
}