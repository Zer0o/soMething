# how to build

```
mkdir build
cd ../build
cmake ../
make
./mm
```

## Command

```
Welcome to clish, the client terminal.

Type:  'help' for help with commands
       'quit or exit' to quit
clish>
```

connnect $ip $port
```
clish>connect 127.0.0.1 9999
```

getservercommand $serverindex
```
clish>getservercommand 0
index ip           port
0     192.168.1.2  6666
------------------------
commands
```

listserver
```
clish>listserver
index  ip           port  status
0      192.168.1.2  6666  CONNECTED
1      192.168.1.2  7777  RECONNECTING(2)
2      192.168.1.3  6666  CLOSE
3      192.168.1.4  6666  FAILED
```

upload $filepath $serverindex
```
clish>upload libaaa.so 0
```

## ThreadPool

example:
```
#include "ThreadPool/ThreadPool.h"

void p0(void *arg)
{
    int *count = (int*)arg;
    for (int i = 0; i < *count; ++i)
    {
        std::cout << i << std::endl;
    }
}

void testThreadPool()
{
    int c0 = 2000, c1 = 1000;
    Task t0(p0, p0, &c0);
    Task t1(p0, p0, &c1);
    Task t2(p0, p0, &c1);
    ThreadWorker tw;
    ThreadPool *pool = tw.newThreadPool("count", 2, 10);
    pool->addTask(t0);
    pool->addTask(t1);
    pool->addTask(t2);

    for (;;);
}
```