#include "FileManage.h"

#include <fcntl.h>
#include <list>
#include <unistd.h>

void FileManage::inactive_close(int, short, void *arg)
{
    FileManage *fm = (FileManage*)arg;
    std::list<FdInfo*> il;
    time_t now = time(NULL);

    for (auto iter : fm->_fdMap)
    {
        if ((iter.second->_lastActive - now) > 300)
        {
            il.push_back(iter.second);
        }
    }

    for (auto iter : il)
    {
        ::close(iter->_fd);
        fm->_fdMap.erase(iter->_fd);
        fm->_pathMap.erase(iter->_filepath);
    }

    //delete
}

bool FileManage::init()
{
    struct event *timer_event = event_new(_base, -1, 0, inactive_close, this);
    struct timeval tv = {300, 0};
    evtimer_add(timer_event, &tv);

    return true;
}

int FileManage::open(std::string path)
{
    time_t now = time(NULL);
    auto iter = _pathMap.find(path);
    if (iter != _pathMap.end())
    {
        iter->second->_lastActive = now;
        return iter->second->_fd;
    }

    int fd = ::open(path.c_str(), O_RDWR | O_CREAT, 0x755);
    if (fd < 0)
    {
        return fd;
    }

    FdInfo *fi = new FdInfo(fd, path, now);
    _fdMap[fd] = fi;
    _pathMap[path] = fi;

    return fd;
}

int FileManage::close(int fd)
{
    auto iter = _fdMap.find(fd);
    if (iter == _fdMap.end())
    {
        return 0;
    }

    iter->second->_lastActive = time(NULL);

    return 0;
}