#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int setDaemon()
{
    switch(fork())
    {
    case -1:
        return -1;
    case 0:
        break;
    default:
        exit(0);
    }

    int pid = getpid();

    if (setsid() == -1)
    {
        return -1;
    }

    chdir("/tmp"); 

    umask(0);

    signal(SIGCHLD,SIG_IGN); 

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1)
    {
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        return -1;
    }

    return 0;
}