# how to build

```
mkdir build
cd ../build
cmake ../
make
./mm
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