#include "MessageEngine.h"

#include <dlfcn.h>
#include <iostream>
#include <fstream>

class MessageArg
{
public:
    MessageArg(Message *msg, MessageEngine *me)
    {
        m = msg;
        e = me;
    }

    Message *m;
    MessageEngine *e;
};

void deal(void *arg)
{
    if (!arg) return;

    //std::cout << "deal" << std::endl;

    MessageArg *messageArg = (MessageArg*)arg;
    messageArg->e->dealMessage(messageArg->m);
}

MessageEngine::MessageEngine()
{

}

void MessageEngine::addRequestMsg(Message *m)
{
#if 0
    std::unique_lock<std::mutex> lock(request_mutex);
    request_messages.push(m);
    request_cv.notify_one();
#endif

    MessageArg *arg = new MessageArg(m, this);

    Task task(deal, NULL, arg);
    _pool->addTask(task);
}

void MessageEngine::dealMessage(Message *m)
{
    if (!m)
    {
        std::cout << "dealMessage m = NULL" << std::endl;
        return;
    }

    //std::cout << "message type = " << m->message_type << std::endl;

    switch (m->message_type)
    {
    case HEARTBEAT:
    {
        time_t t = time(NULL);
        _lastHeartbeat = t;
        break;
    }
    case PING_REQUEST:
    {
        msg::PingResponse rsp;
        rsp.set_t(time(NULL));
        std::string out;
        rsp.SerializeToString(&out);
        char *data = (char*)malloc(out.size()*sizeof(char));
        strncpy(data, out.c_str(), out.size());
        Message *rspMsg = new Message(0, PING_RESPONSE, out.size());
        rspMsg->data = data;
        break;
    }
    case PING_RESPONSE:
    {
        break;
    }
    case GET_COMMANDS_REQUEST:
    {

        break;
    }
    case ADD_COMMANDS_REQUEST:
    {
        msg::Message msg;
        if (!msg.ParseFromArray(m->data, m->pbLen))
        {
            std::cout << "ParseFromArray failed" << std::endl;
            break;
        }

        const msg::AddCommandsRequest &acr = msg.acr();
        for (int i = 0; i < acr.command_size(); ++i)
        {
            const msg::Command &command = acr.command(i);
            const std::string &commandname = command.commandname();
            _commandMap[commandname] = acr.command(i);
        }
        break;
    }
    case EXE_COMMANDS_REQUEST:
    {
        msg::Message msg;
        if (!msg.ParseFromArray(m->data, m->pbLen))
        {
            std::cout << "ParseFromArray failed" << std::endl;
            break;
        }

        const msg::ExeCommandRequest &ecr = msg.ecr();
        const std::string &commandname = ecr.commandname();

        auto iter = _commandMap.find(commandname);
        if (iter == _commandMap.end())
        {
            std::cout << "command not found " << commandname << std::endl;
            break;
        }

        exeSoFunction(iter->second.filepath(), iter->second.funcname(), (char*)m->data+m->pbLen);

        break;
    }
    case FILE_UPLOAD:
    {
        msg::Message msg;
        if (!msg.ParseFromArray((char*)m->data, m->pbLen))
        {
            std::cout << "msg ParseFromArray failed, pbLen = " << m->pbLen << std::endl;
            break;
        }

        const msg::FileUpload &fu = msg.fu();
        const std::string &filepath = fu.filepath();
        uint64_t filepos = fu.filepos();

        //std::cout << "filepath = " << filepath << ", pos = " << filepos << std::endl;

        std::fstream s(filepath, s.binary | s.out | s.app);
        if (!s.is_open()) {
            std::cout << "failed to open " << filepath << ", pos = " << filepos << std::endl;
            break;
        }
 
        s.seekp(filepos);
        s.write((char*)m->data+m->pbLen, m->len - m->pbLen);
        std::cout << "write size = " << m->len - m->pbLen << std::endl;
        s.close();

        break;
    }
    default:
        break;
    }
}

Message *MessageEngine::exeSoFunction(const std::string &so_name, const std::string &func_name, void *arg)
{
    void *handle = dlopen(so_name.c_str(), RTLD_LAZY);
    if (!handle)
    {
        std::cout << "dlopen failed" << std::endl;
        return NULL;
    }

    func p = (func)dlsym(handle, func_name.c_str());
    Message *msg = p(arg);
    dlclose(handle);

    return msg;
}