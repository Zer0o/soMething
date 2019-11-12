#pragma once

#include <event2/event.h>
#include <map>

class FileManage
{
public:
    FileManage() {}
    FileManage(struct event_base *base)
      :  _maxActiveFd(50), _base(base) {}

    bool init();

    void setEventBase(struct event_base *base)
    {
        _base = base;
    }

    static void inactive_close(int, short, void *);

    int open(std::string path);
    int close(int fd);

protected:
    class FdInfo
    {
    public:
        FdInfo(int fd, std::string fp, time_t lastActive)
          : _fd(fd), _filepath(fp), _lastActive(lastActive) {}

        int _fd;
        std::string _filepath;
        time_t _lastActive;
    };

    int _maxActiveFd;
    struct event_base *_base;
    std::map<uint64_t, FdInfo*> _fdMap;
    std::map<std::string, FdInfo*> _pathMap;
};