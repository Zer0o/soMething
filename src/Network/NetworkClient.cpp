#include "Message.h"
#include "NetworkClient.h"

#include <iostream>
#include <string.h>

bool NetworkClient::init()
{
    data.push_back("hello0");
    data.push_back("world");
    data.push_back("enenen2");

    return true;
}

void NetworkClient::read_cb(struct bufferevent *bev, void *arg)
{

}

void NetworkClient::write_cb(struct bufferevent *bev, void *arg)
{
    NetworkClient *client = (NetworkClient*)arg;
    std::cout << "enable write" << std::endl;
    if (!client->data.empty())
    {
        std::string s = std::move(client->data.front());
        client->data.pop_front();
        Message *p = (Message*)malloc(sizeof(Message)+s.size());
        Message m(0, MessageType::DATA_SEND, s.size());
        *p = m;
        strncpy((char*)p+sizeof(Message), s.c_str(), s.size());

        std::cout << "write len = " << p->len << std::endl;

        bufferevent_write(bev, p, sizeof(Message) + s.size());
    }
}

void NetworkClient::event_cb(struct bufferevent *bev, short event, void *arg)
{
    if (event & BEV_EVENT_CONNECTED)
    {
        std::cout << "connected." << std::endl;
        Message m(0, MessageType::PING_REQUEST, 0);
        bufferevent_write(bev, &m, sizeof(m));
    }
}