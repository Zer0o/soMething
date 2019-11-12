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

void NetworkClient::new_client(int, short, void *arg)
{
    NetworkClient *client = (NetworkClient*)arg;
    client->bev = bufferevent_socket_new(client->base, -1,
                                                     BEV_OPT_CLOSE_ON_FREE);
    bufferevent_socket_connect(client->bev, (struct sockaddr*)client->getSockaddrin(), sizeof(struct sockaddr));
    bufferevent_setcb(client->bev, NetworkClient::read_cb, 
    NetworkClient::write_cb, NetworkClient::event_cb, (void*)client);
    bufferevent_enable(client->bev, EV_READ | EV_WRITE | EV_PERSIST);
}

void NetworkClient::read_cb(struct bufferevent *bev, void *arg)
{

}

void NetworkClient::write_cb(struct bufferevent *bev, void *arg)
{
    NetworkClient *client = (NetworkClient*)arg;

    //std::cout << "write cb" << std::endl;

    if (!client->_msgList.empty())
    {
        Message *p = client->_msgList.front();
        client->_msgList.pop_front();

        if (p->message_type == FILE_UPLOAD)
        {
            struct evbuffer *buffer = evbuffer_new();
            evbuffer_add(buffer, p, sizeof(Message)+p->pbLen);
            uint64_t *fv = (uint64_t*)((char*)p+sizeof(Message)+p->pbLen);
            evbuffer_add_file(buffer, *fv, *(fv+1), *(fv+2));
            bufferevent_write_buffer(bev, buffer);
        }
        else
        {
            //Message *dst = (Message*)malloc(sizeof(Message)+p->len*sizeof(char));
            //*dst = *p;
            //memcpy((char*)dst+sizeof(Message), p->data, p->len);

            bufferevent_write(bev, p, sizeof(Message)+p->len);
        }
    }
}

void NetworkClient::event_cb(struct bufferevent *bev, short event, void *arg)
{
    NetworkClient *client = (NetworkClient*)arg;
    if (event & BEV_EVENT_CONNECTED)
    {
        //std::cout << "connected." << std::endl;
        //Message *m = new Message(0, MessageType::PING_REQUEST, 0);
        //bufferevent_write(bev, m, sizeof(Message));
        //bufferevent_enable(bev, EV_WRITE);
    }
    else if (event & BEV_EVENT_TIMEOUT)
    {
        //std::cout << "conntected timtout." << std::endl;
    }
    else if (event & BEV_EVENT_ERROR)
    {
        //std::cout << "error. reconnecting in 5s....." << std::endl;

        struct event_base *base = bufferevent_get_base(bev);
        bufferevent_free(bev);

        struct event *timer_event = event_new(base, -1, 0, NetworkClient::new_client, arg);
        struct timeval tv = {5, 0};
        evtimer_add(timer_event, &tv);
    }
    else if (event & BEV_EVENT_EOF)
    {
        //std::cout << "eof. reconnecting in 5s....." << std::endl;

        struct event_base *base = bufferevent_get_base(bev);
        bufferevent_free(bev);

        struct event *timer_event = event_new(base, -1, 0, NetworkClient::new_client, arg);
        struct timeval tv = {5, 0};
        evtimer_add(timer_event, &tv);
    }
}