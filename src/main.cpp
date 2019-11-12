#include "MessageEngine/MessageEngine.h"
#include "Network/NetworkEngine.h"

void setDaemon();

void run(int argc, char **argv)
{
    if (argc < 2)
    {
        goto OUTPUT_USAGE;
    }

    if (!strcmp(argv[1], "-c"))
    {
        Engine *e = new Engine(true);
        e->run();
    }
    else if (!strcmp(argv[1], "-s") && argc > 2)
    {
        setDaemon();

        Engine *e = new Engine();
        NetworkService *service = new NetworkService(atoi(argv[2]));
        e->addService(service);
        e->run();
    }

OUTPUT_USAGE:
    std::cout << "-c: run as client" << std::endl;
    std::cout << "-s port: run as server " << std::endl;
}

int main(int argc, char **argv)
{
    run(argc, argv);

    return 0;
}