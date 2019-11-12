#pragma once

#include <string>

class FileTransfer
{
public:
    FileTransfer(std::string filename);
    ~FileTransfer();

    bool loadMeta();

protected:
    bool createMeta();
};