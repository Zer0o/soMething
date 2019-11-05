#pragma once

#include <stdio.h>

#define MAGICAL_NUM 0x405A4855

enum MessageType
{
    HEARTBEAT = 0,
    PING_REQUEST,
    PING_RESPONSE,
    GET_COMMANDS_REQUEST,
    GET_COMMANDS_RESPONSE,
    ADD_COMMANDS_REQUEST,
    ADD_COMMANDS_RESPONSE,
    EXE_COMMANDS_REQUEST,
    EXE_COMMANDS_RESPONSE,
    DATA_SEND,
    FILE_UPLOAD,
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
    int pbLen;
    void *data;
};
#pragma pack()