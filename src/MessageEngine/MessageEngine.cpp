#include "MessageEngine.h"

#include <dlfcn.h>
#include <iostream>

MessageEngine::MessageEngine()
{

}

void MessageEngine::addRequestMsg(Message *m)
{
    std::unique_lock<std::mutex> lock(request_mutex);
    request_messages.push(m);
    request_cv.notify_one();
}

void MessageEngine::dealMessage()
{
    Message *m = NULL;
    {
        std::unique_lock<std::mutex> lock(request_mutex);
        request_cv.wait(lock, [this] { return !this->request_messages.empty(); });
        
        m = request_messages.front();
        request_messages.pop();
    }

    switch (m->message_type)
    {
    case HEARTBEAT:
        break;
    case HEARTBEAT_ACK:
        break;
    case PING_REQUEST:
        break;
    case PING_RESPONSE:
        break;
    case DATA_SEND:
        break;
    case ADD_COMMAND:
        break;
    case EXE_COMMAND:
        break;
    default:
        break;
    }
}

func MessageEngine::getSoFunction(std::string so_name, std::string func_name)
{
    void *handle = dlopen(so_name.c_str(), RTLD_LAZY);
    if (!handle)
    {
        std::cout << "dlopen failed" << std::endl;
    }

    func p = (func)dlsym(handle, func_name.c_str());
    //dlclose(handle);

    return p;
}