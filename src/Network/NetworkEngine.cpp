#include "NetworkEngine.h"

#include<event2/bufferevent.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream> 

static const char *welcome = 
"Welcome to clish, the client terminal.\n"
"\n"
"Type:  'help' for help with commands\n"
"       'quit or exit' to quit\n";

static const char *clish = "clish>";

struct help
{
    const char *command;
    const char *help;
} helpList[] = 
{
    {"addCommand", 
     "  arg1: command name"},
    {"connnect",
     "  arg1: server ip\n"
     "  arg2: server port"},
    {"exeCommand",
     "  arg1: command name\n"
     "  arg2: command arg"},
    {"list", 
     "  list all commands"},
    {"upload",
     "  arg1: filepath to upload\n"
     "  arg2: server to upload"},
};

std::vector<std::string> splitSpace(std::string input)
{
    std::vector<std::string> v;
    std::size_t off = 0, off2;
    do 
    {
        off2 = input.find_first_of(" ", off);
        if (off2 == std::string::npos)
        {
            v.push_back(std::move(input.substr(off)));
            break;
        }
        else
        {
            v.push_back(std::move(input.substr(off, off2 - off)));
        }

        off = off2 + 1;
    } while(1);

    return std::move(v);
}

void stdin_read_cb(struct bufferevent* bev, void *arg)
{
    Engine *e = (Engine*)arg;

    char a[1024];
    size_t sz = bufferevent_read(bev, a, 1024);
    if (sz < 1) return;
    a[sz-1] = '\0';

    std::vector<std::string> v = splitSpace(a);
    for (auto str : v)
    {
        //std::cout << str << std::endl;
    }

    if (v.empty())
    {
        std::cout << "no such command" << std::endl;
    }
    else if (!strcmp(v[0].c_str(), "quit"))
    {
        std::cout << "Bye." << std::endl;
        exit(0);
    }
    else if (!strcmp(v[0].c_str(), "exit"))
    {
        std::cout << "Bye." << std::endl;
        exit(0);
    }
    else if (!strcmp(v[0].c_str(), "help"))
    {
        for (auto h : helpList)
        {
            std::cout << h.command << std::endl;
            std::cout << h.help << std::endl;
        }
    }
    else if (!strcmp(v[0].c_str(), "connect"))
    {
        if (v.size() < 3)
        {
            std::cout << "[ERROR]connect ip port" << std::endl;
        }
        else
        {
            NetworkClient *client = new NetworkClient(v[1], atoi(v[2].c_str()));
            e->addClient(client);

            std::cout << "connecting..." << std::endl;
        }
    }
    else if (!strcmp(v[0].c_str(), "listserver"))
    {
        std::cout << "index ip         port" << std::endl;
        std::vector<ServerInfo> v = e->getServerInfo();
        for (size_t i = 0; i < v.size(); ++i)
        {
            std::cout << i << "     " << v[i]._ip << " " << v[i]._port << std::endl;
        }
    }
    else if (!strcmp(v[0].c_str(), "upload"))
    {
        if (v.size() < 3)
        {
            std::cout << "[ERROR]upload serverindex filepath" << std::endl;
        }
        else
        {
            struct stat st;
            if (stat(v[2].c_str(), &st) != 0)
            {
                std::cout << "[ERROR]not such file:" << v[2] << std::endl;
            }
            else
            {
                int fd = ::open(v[2].c_str(), O_RDONLY);

                NetworkClient *client = e->getClient(atoi(v[1].c_str()));

                for (int i = 0; i < st.st_size / (1024 * 1024) + 1; ++i)
                {
                    msg::FileUpload *fu = new msg::FileUpload;
                    fu->set_filepath(v[2]);
                    fu->set_filepos(i);
                    msg::Message *msg = new msg::Message;
                    msg->set_allocated_fu(fu);

                    Message *p = (Message*)malloc(sizeof(Message)+msg->ByteSize()+sizeof(uint64_t)*3);
                    Message m(0, FILE_UPLOAD, 0);
                    *p = m;
                    p->pbLen = msg->ByteSize();
                    uint64_t *fv = (uint64_t*)((char*)p + sizeof(Message)+msg->ByteSize());
                    *fv = fd;
                    *(fv+1) = i;
                    if (i == st.st_size / (1024 * 1024))
                    {
                        *(fv+2) = st.st_size % (1024*1024);
                    }
                    else
                    {
                        *(fv+2) = 1024 * 1024;
                    }

                    p->len = p->pbLen + *(fv+2);

                    if (!msg->SerializeToArray((char*)p+sizeof(Message), msg->ByteSize()))
                    {
                        std::cout << "SerializeToArray failed" << std::endl;
                    }
                    else
                    {
                        client->addMessage(p);
                    }
                }
            }
        }
    }
    else if (!strcmp(v[0].c_str(), "add"))
    {
        NetworkClient *client = e->getClient(atoi(v[1].c_str()));

        msg::AddCommandsRequest *ar = new msg::AddCommandsRequest;
                msg::Command *c = ar->add_command();
                c->set_commandname("print");
                c->set_filepath(v[2]);
                c->set_funcname("helloworld");
                c->set_commandhelp("");

                //std::cout << "acr size = " << ar->command_size() << std::endl;

                msg::Message *msg = new msg::Message;
                msg->set_allocated_acr(ar);

                Message *p = (Message*)malloc(sizeof(Message)+msg->ByteSize());
                Message m(0, ADD_COMMANDS_REQUEST, msg->ByteSize());
                //std::cout << msg->ByteSize() << std::endl;
                *p = m;
                p->pbLen = msg->ByteSize();
                if (!msg->SerializeToArray((char*)p+sizeof(Message), msg->ByteSize()))
                {
                        std::cout << "SerializeToArray failed" << std::endl;
                }
                else
                {
                    client->addMessage(p);
                }
    }
    else if (!strcmp(v[0].c_str(), "exe"))
    {
        NetworkClient *client = e->getClient(atoi(v[1].c_str()));

        msg::ExeCommandRequest *ecr = new msg::ExeCommandRequest;
                ecr->set_commandname("print");

                msg::Message *msg = new msg::Message;
                msg->set_allocated_ecr(ecr);

                Message *p = (Message*)malloc(sizeof(Message)+msg->ByteSize());
                Message m(0, EXE_COMMANDS_REQUEST, msg->ByteSize());
                *p = m;
                p->pbLen = msg->ByteSize();
                if (!msg->SerializeToArray((char*)p+sizeof(Message), msg->ByteSize()))
                {
                        std::cout << "SerializeToArray failed" << std::endl;
                }
                else
                {
                    client->addMessage(p);
                }
    }
    else
    {
        std::cout << "no such command" << std::endl;
    }

    std::cout << std::endl;
    std::cout << clish;
    std::cout.flush();
}

void stdout_write_cb(struct bufferevent* bev, void *arg)
{
    //std::cout << "hello" << std::endl;
}

Engine::Engine(bool b /* = false */)
{
    base = event_base_new();

    if (b)
    {
        struct bufferevent* stdinbev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setfd(stdinbev, STDIN_FILENO);
        bufferevent_setcb(stdinbev, stdin_read_cb, 
            stdout_write_cb, NULL, this);
        bufferevent_enable(stdinbev, EV_READ | EV_WRITE | EV_PERSIST);
        bufferevent_write(stdinbev, welcome, strlen(welcome));
        bufferevent_write(stdinbev, clish, strlen(clish));
    }
}

void Engine::addService(NetworkService *service)
{
    if (!service || !service->init())
    {
        return;
    }

    NetworkServiceCallbackArg *arg = new NetworkServiceCallbackArg(base, service);

    struct sockaddr_in *sin = service->getSin();
    evconnlistener *listener  
            = evconnlistener_new_bind(base, NetworkService::listener_cb, arg,  
                                      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,  
                                      10, (struct sockaddr *)sin,  
                                      sizeof(struct sockaddr_in));

    serviceList.push_back(service);

    std::cout << "service listening on port " << service->getPort() << std::endl;
}

void Engine::addClient(NetworkClient *client)
{
    if (!client || !client->init())
    {
        return;
    }

    client->setBase(base);

    clientList.push_back(client);

    NetworkClient::new_client(0, 0, client);
}

void Engine::run()
{
    event_base_dispatch(base);
}