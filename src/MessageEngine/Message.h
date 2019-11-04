#pragma once

#include <stdio.h>

#define MAGICAL_NUM 0x405A4855

enum MessageType
{
    HEARTBEAT = 0,
    HEARTBEAT_ACK,
    PING_REQUEST,
    PING_RESPONSE,
    DATA_SEND,
    GET_COMMAND,
    ADD_COMMAND,
    EXE_COMMAND,
};

#pragma pack(1)
class Message
{
public:
    Message() : data(NULL) {}
    Message(int id, int type, int l = 0)
     : magical_num(MAGICAL_NUM), message_type(type), message_id(id), len(l), data(NULL) {}
    ~Message() {}

    bool valid() const { return magical_num == MAGICAL_NUM; }

    unsigned int magical_num;
    int message_type;
    int message_id;
    int len;
    void *data;
};
#pragma pack()